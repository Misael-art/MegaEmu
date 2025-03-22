/**
 * @file vdp_sprites.c
 * @brief Implementação da renderização de sprites para o VDP do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "vdp.h"
#include "utils/log_utils.h"
#include "utils/validation_utils.h"

// Definições para sprites
#define SPRITE_TABLE_SIZE 80
#define SPRITE_WIDTH_NORMAL 8
#define SPRITE_HEIGHT_NORMAL 8
#define SPRITE_MAX_PER_LINE 20
#define SPRITE_OVERFLOW_FLAG 0x01
#define SPRITE_COLLISION_FLAG 0x02

// Estrutura para manter o estado de um sprite
typedef struct {
    int16_t x;            // Posição X do sprite
    int16_t y;            // Posição Y do sprite
    uint16_t width;       // Largura em pixels
    uint16_t height;      // Altura em pixels
    uint16_t tile_index;  // Índice do tile no pattern
    uint8_t palette;      // Paleta de cores (0-3)
    uint8_t priority;     // Prioridade (0-3)
    uint8_t h_flip;       // Flag de flip horizontal
    uint8_t v_flip;       // Flag de flip vertical
    uint8_t link;         // Link para o próximo sprite
} md_sprite_t;

// Estrutura para manter o estado de processamento de sprites
typedef struct {
    md_sprite_t sprites[SPRITE_TABLE_SIZE];  // Array de sprites
    uint32_t sprite_count;                   // Número de sprites ativos
    uint8_t sprite_overflow;                 // Flag de overflow
    uint8_t sprite_collision;                // Flag de colisão
    
    // Cache para sprites por linha
    uint8_t sprites_per_line[240];           // Número de sprites por linha
    uint16_t sprite_line_indices[240][SPRITE_MAX_PER_LINE]; // Índices de sprites por linha
    
    // Flags para otimização
    uint8_t cache_valid;                     // Flag para validar cache
    uint32_t frame_counter;                  // Contador de frames
} md_sprite_system_t;

// Estado global
static md_sprite_system_t sprite_system;

// Declarações de funções
static void _parse_sprite_table(const uint8_t *vram, uint16_t table_addr);
static void _build_sprite_line_cache(void);
static void _render_sprite_line(uint32_t *framebuffer, int line, int width, const uint16_t *cram);
static uint8_t _check_sprite_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

/**
 * @brief Inicializa o sistema de sprites
 */
void md_vdp_sprite_init(void)
{
    // Limpar estado do sistema de sprites
    memset(&sprite_system, 0, sizeof(md_sprite_system_t));
    
    LOG_INFO("Subsistema de sprites do VDP inicializado");
}

/**
 * @brief Reseta o sistema de sprites
 */
void md_vdp_sprite_reset(void)
{
    // Limpar apenas os sprites e flags, mas manter estruturas
    for (int i = 0; i < SPRITE_TABLE_SIZE; i++) {
        sprite_system.sprites[i].x = 0;
        sprite_system.sprites[i].y = 0;
        sprite_system.sprites[i].width = SPRITE_WIDTH_NORMAL;
        sprite_system.sprites[i].height = SPRITE_HEIGHT_NORMAL;
        sprite_system.sprites[i].tile_index = 0;
        sprite_system.sprites[i].palette = 0;
        sprite_system.sprites[i].priority = 0;
        sprite_system.sprites[i].h_flip = 0;
        sprite_system.sprites[i].v_flip = 0;
        sprite_system.sprites[i].link = 0;
    }
    
    sprite_system.sprite_count = 0;
    sprite_system.sprite_overflow = 0;
    sprite_system.sprite_collision = 0;
    sprite_system.cache_valid = 0;
    
    LOG_INFO("Subsistema de sprites do VDP resetado");
}

/**
 * @brief Processa a tabela de sprites da VRAM
 * @param vram Ponteiro para a VRAM
 * @param table_addr Endereço da tabela de sprites na VRAM
 */
static void _parse_sprite_table(const uint8_t *vram, uint16_t table_addr)
{
    CHECK_NULL_RETURN_VOID(vram, "VRAM nula passada para processamento de sprites");
    
    // Limpar contagem de sprites
    sprite_system.sprite_count = 0;
    
    // Processar sprites
    for (int i = 0; i < SPRITE_TABLE_SIZE && sprite_system.sprite_count < SPRITE_TABLE_SIZE; i++) {
        uint16_t offset = table_addr + (i * 8); // Cada sprite ocupa 8 bytes
        
        // Garantir que o offset está dentro dos limites da VRAM
        if (offset >= VRAM_SIZE - 8) {
            LOG_ERROR("Offset de tabela de sprites fora dos limites: 0x%04X", offset);
            break;
        }
        
        // Ler dados do sprite
        int16_t y = ((vram[offset] << 8) | vram[offset + 1]) & 0x3FF;
        uint8_t size_link = vram[offset + 2];
        uint8_t attr = vram[offset + 3];
        int16_t x = ((vram[offset + 4] << 8) | vram[offset + 5]) & 0x3FF;
        uint16_t pattern_index = ((vram[offset + 6] << 8) | vram[offset + 7]) & 0x7FF;
        
        // Processar Y
        if (y == 0) {
            continue; // Sprite desativado
        }
        
        // Ajustar Y (considerar 128 como origem)
        y -= 128;
        
        // Ajustar X (considerar 128 como origem)
        x -= 128;
        
        // Calcular tamanho do sprite
        uint8_t size_h = (size_link >> 2) & 0x03;
        uint8_t size_v = size_link & 0x03;
        
        uint16_t width = SPRITE_WIDTH_NORMAL * (1 << size_h);
        uint16_t height = SPRITE_HEIGHT_NORMAL * (1 << size_v);
        
        // Obter link
        uint8_t link = size_link >> 4;
        
        // Processar atributos
        uint8_t palette = (attr >> 5) & 0x03;
        uint8_t priority = (attr >> 7) & 0x01;
        uint8_t h_flip = (attr >> 4) & 0x01;
        uint8_t v_flip = (attr >> 5) & 0x01;
        
        // Armazenar informações do sprite
        md_sprite_t *sprite = &sprite_system.sprites[sprite_system.sprite_count];
        sprite->x = x;
        sprite->y = y;
        sprite->width = width;
        sprite->height = height;
        sprite->tile_index = pattern_index;
        sprite->palette = palette;
        sprite->priority = priority;
        sprite->h_flip = h_flip;
        sprite->v_flip = v_flip;
        sprite->link = link;
        
        sprite_system.sprite_count++;
        
        // Verificar link
        if (link == 0) {
            break; // Fim da cadeia de sprites
        }
        
        // Ir para o próximo sprite (seguindo o link)
        i = link - 1;
    }
    
    // Verificar overflow
    if (sprite_system.sprite_count >= SPRITE_TABLE_SIZE) {
        sprite_system.sprite_overflow = 1;
        LOG_WARNING("Overflow de sprites detectado: %d sprites", sprite_system.sprite_count);
    } else {
        sprite_system.sprite_overflow = 0;
    }
    
    LOG_DEBUG("Processados %d sprites", sprite_system.sprite_count);
}

/**
 * @brief Constrói o cache de sprites por linha para otimizar a renderização
 */
static void _build_sprite_line_cache(void)
{
    // Limpar cache
    memset(sprite_system.sprites_per_line, 0, sizeof(sprite_system.sprites_per_line));
    memset(sprite_system.sprite_line_indices, 0, sizeof(sprite_system.sprite_line_indices));
    
    // Processar cada sprite
    for (uint32_t i = 0; i < sprite_system.sprite_count; i++) {
        md_sprite_t *sprite = &sprite_system.sprites[i];
        
        // Determinar as linhas afetadas pelo sprite
        int16_t start_y = sprite->y;
        int16_t end_y = start_y + sprite->height;
        
        // Limitar às linhas visíveis
        if (start_y < 0) start_y = 0;
        if (end_y > 240) end_y = 240;
        
        // Adicionar sprite a cada linha
        for (int16_t y = start_y; y < end_y; y++) {
            if (sprite_system.sprites_per_line[y] < SPRITE_MAX_PER_LINE) {
                sprite_system.sprite_line_indices[y][sprite_system.sprites_per_line[y]] = i;
                sprite_system.sprites_per_line[y]++;
            } else {
                // Mais sprites por linha do que o permitido
                sprite_system.sprite_overflow = 1;
                break;
            }
        }
    }
    
    // Marcar cache como válido
    sprite_system.cache_valid = 1;
}

/**
 * @brief Renderiza todos os sprites em uma linha específica
 * @param framebuffer Buffer para renderização
 * @param line Número da linha a renderizar
 * @param width Largura do framebuffer
 * @param cram Ponteiro para a CRAM (paleta)
 */
static void _render_sprite_line(uint32_t *framebuffer, int line, int width, const uint16_t *cram)
{
    CHECK_NULL_RETURN_VOID(framebuffer, "Framebuffer nulo passado para renderização de sprites");
    CHECK_NULL_RETURN_VOID(cram, "CRAM nula passada para renderização de sprites");
    
    // Verificar limites
    if (line < 0 || line >= 240 || width <= 0) {
        return;
    }
    
    // Garantir que o cache está atualizado
    if (!sprite_system.cache_valid) {
        _build_sprite_line_cache();
    }
    
    // Obter número de sprites na linha
    int sprites_in_line = sprite_system.sprites_per_line[line];
    
    // Renderizar cada sprite na linha (do último para o primeiro para respeitar prioridade)
    for (int i = sprites_in_line - 1; i >= 0; i--) {
        uint16_t sprite_idx = sprite_system.sprite_line_indices[line][i];
        md_sprite_t *sprite = &sprite_system.sprites[sprite_idx];
        
        // Calcular offsetY dentro do sprite
        int16_t sprite_y = line - sprite->y;
        
        // Aplicar flip vertical se necessário
        if (sprite->v_flip) {
            sprite_y = sprite->height - 1 - sprite_y;
        }
        
        // Processar todos os pixels do sprite na linha atual
        for (int16_t x = 0; x < sprite->width; x++) {
            // Calcular posição na tela
            int16_t screen_x = sprite->x + x;
            
            // Verificar se o pixel está na tela
            if (screen_x < 0 || screen_x >= width) {
                continue;
            }
            
            // Aplicar flip horizontal se necessário
            int16_t sprite_x = x;
            if (sprite->h_flip) {
                sprite_x = sprite->width - 1 - x;
            }
            
            // Calcular tile e posição dentro do tile
            uint16_t tile = sprite->tile_index + 
                          ((sprite_x / SPRITE_WIDTH_NORMAL) * (sprite->height / SPRITE_HEIGHT_NORMAL)) +
                          ((sprite_y / SPRITE_HEIGHT_NORMAL) * (sprite->width / SPRITE_WIDTH_NORMAL));
            
            uint8_t tile_x = sprite_x % SPRITE_WIDTH_NORMAL;
            uint8_t tile_y = sprite_y % SPRITE_HEIGHT_NORMAL;
            
            // NOTA: A função abaixo é apenas um protótipo e não implementa a lógica 
            // real de busca de pixels na VRAM, que seria complexa e dependeria da 
            // implementação completa do VDP
            
            // Obter cor do pixel no sprite (4bpp)
            uint8_t pixel = (tile_x & 1) ? 0x05 : 0x02; // Valor de demonstração
            
            // Ignorar pixels transparentes (cor 0)
            if (pixel == 0) {
                continue;
            }
            
            // Calcular índice na CRAM
            uint16_t cram_idx = (sprite->palette * 16) + pixel;
            
            // Obter cor da CRAM
            uint16_t color_raw = cram[cram_idx];
            
            // Converter para RGB32
            uint32_t color = ((color_raw & 0x0F00) << 12) | // R 
                             ((color_raw & 0x00F0) << 8)  | // G
                             ((color_raw & 0x000F) << 4);   // B
            
            // Verificar colisão com outros sprites
            uint32_t fb_index = line * width + screen_x;
            if ((framebuffer[fb_index] & 0xFF000000) == 0x01000000) {
                // Pixel já desenhado por outro sprite, registrar colisão
                sprite_system.sprite_collision = 1;
            }
            
            // Desenhar pixel com marcador de sprite (0x01000000)
            framebuffer[fb_index] = color | 0x01000000;
        }
    }
}

/**
 * @brief Renderiza todos os sprites em um framebuffer
 * @param framebuffer Buffer para renderização
 * @param width Largura do framebuffer
 * @param height Altura do framebuffer
 * @param vram Ponteiro para a VRAM
 * @param cram Ponteiro para a CRAM (paleta)
 * @param sprite_table_addr Endereço da tabela de sprites
 */
void md_vdp_render_sprites(uint32_t *framebuffer, int width, int height, 
                         const uint8_t *vram, const uint16_t *cram, 
                         uint16_t sprite_table_addr)
{
    CHECK_NULL_RETURN_VOID(framebuffer, "Framebuffer nulo passado para renderização de sprites");
    CHECK_NULL_RETURN_VOID(vram, "VRAM nula passada para renderização de sprites");
    CHECK_NULL_RETURN_VOID(cram, "CRAM nula passada para renderização de sprites");
    
    // Verificar limites
    if (width <= 0 || height <= 0) {
        LOG_ERROR("Dimensões inválidas para renderização de sprites: %dx%d", width, height);
        return;
    }
    
    // Processar tabela de sprites
    _parse_sprite_table(vram, sprite_table_addr);
    
    // Invalidar cache para reconstruir
    sprite_system.cache_valid = 0;
    
    // Construir cache de sprites por linha
    _build_sprite_line_cache();
    
    // Renderizar sprites linha por linha
    for (int y = 0; y < height && y < 240; y++) {
        _render_sprite_line(framebuffer, y, width, cram);
    }
    
    // Incrementar contador de frames
    sprite_system.frame_counter++;
    
    LOG_DEBUG("Sprites renderizados: %d, overflow=%d, colisão=%d", 
            sprite_system.sprite_count, 
            sprite_system.sprite_overflow, 
            sprite_system.sprite_collision);
}

/**
 * @brief Verifica a colisão entre dois sprites
 * @param x1 Posição X do primeiro sprite
 * @param y1 Posição Y do primeiro sprite
 * @param w1 Largura do primeiro sprite
 * @param h1 Altura do primeiro sprite
 * @param x2 Posição X do segundo sprite
 * @param y2 Posição Y do segundo sprite
 * @param w2 Largura do segundo sprite
 * @param h2 Altura do segundo sprite
 * @return 1 se há colisão, 0 caso contrário
 */
static uint8_t _check_sprite_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    // Verificar se os retângulos se sobrepõem
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2) ? 1 : 0;
}

/**
 * @brief Verifica colisões entre todos os sprites
 * @return 1 se alguma colisão foi detectada, 0 caso contrário
 */
uint8_t md_vdp_check_sprite_collisions(void)
{
    // Resetar flag de colisão
    sprite_system.sprite_collision = 0;
    
    // Verificar colisões entre todos os pares de sprites
    for (uint32_t i = 0; i < sprite_system.sprite_count; i++) {
        md_sprite_t *sprite1 = &sprite_system.sprites[i];
        
        for (uint32_t j = i + 1; j < sprite_system.sprite_count; j++) {
            md_sprite_t *sprite2 = &sprite_system.sprites[j];
            
            // Verificar colisão
            if (_check_sprite_collision(sprite1->x, sprite1->y, sprite1->width, sprite1->height,
                                      sprite2->x, sprite2->y, sprite2->width, sprite2->height)) {
                sprite_system.sprite_collision = 1;
                return 1;
            }
        }
    }
    
    return 0;
}

/**
 * @brief Obtém o status de overflow de sprites
 * @return 1 se ocorreu overflow, 0 caso contrário
 */
uint8_t md_vdp_get_sprite_overflow(void)
{
    return sprite_system.sprite_overflow;
}

/**
 * @brief Obtém o status de colisão de sprites
 * @return 1 se ocorreu colisão, 0 caso contrário
 */
uint8_t md_vdp_get_sprite_collision(void)
{
    return sprite_system.sprite_collision;
}

/**
 * @brief Obtém o número de sprites atualmente ativos
 * @return Número de sprites ativos
 */
uint32_t md_vdp_get_sprite_count(void)
{
    return sprite_system.sprite_count;
}

/**
 * @brief Gera uma visualização de diagnóstico dos sprites
 * @param framebuffer Buffer para renderização
 * @param width Largura do framebuffer
 * @param height Altura do framebuffer
 */
void md_vdp_generate_sprite_debug_view(uint32_t *framebuffer, int width, int height)
{
    CHECK_NULL_RETURN_VOID(framebuffer, "Framebuffer nulo passado para debug de sprites");
    
    // Verificar limites
    if (width <= 0 || height <= 0) {
        return;
    }
    
    // Limpar área
    for (int i = 0; i < width * height; i++) {
        framebuffer[i] = 0x000000; // Preto
    }
    
    // Desenhar grade
    for (int y = 0; y < height; y += 8) {
        for (int x = 0; x < width; x++) {
            framebuffer[y * width + x] = 0x303030; // Cinza escuro
        }
    }
    
    for (int x = 0; x < width; x += 8) {
        for (int y = 0; y < height; y++) {
            framebuffer[y * width + x] = 0x303030; // Cinza escuro
        }
    }
    
    // Desenhar representação de cada sprite
    for (uint32_t i = 0; i < sprite_system.sprite_count; i++) {
        md_sprite_t *sprite = &sprite_system.sprites[i];
        
        // Calcular cores para diferentes prioridades e paletas
        uint32_t colors[4] = {
            0xFF0000, // Vermelho (paleta 0)
            0x00FF00, // Verde (paleta 1)
            0x0000FF, // Azul (paleta 2)
            0xFFFF00  // Amarelo (paleta 3)
        };
        
        uint32_t color = colors[sprite->palette & 0x03];
        
        // Tornar mais escuro se for prioridade baixa
        if (sprite->priority == 0) {
            color = (color >> 1) & 0x7F7F7F; // Metade do brilho
        }
        
        // Calcular posição na tela (ajustada para visualização)
        int16_t x = sprite->x + 128;
        int16_t y = sprite->y + 128;
        
        // Desenhar retângulo do sprite
        for (int16_t dy = 0; dy < sprite->height; dy++) {
            int16_t py = y + dy;
            if (py < 0 || py >= height) continue;
            
            for (int16_t dx = 0; dx < sprite->width; dx++) {
                int16_t px = x + dx;
                if (px < 0 || px >= width) continue;
                
                // Borda do sprite
                if (dx == 0 || dx == sprite->width - 1 || 
                    dy == 0 || dy == sprite->height - 1) {
                    framebuffer[py * width + px] = 0xFFFFFF; // Branco
                } else {
                    framebuffer[py * width + px] = color;
                }
            }
        }
        
        // Desenhar identificador do sprite
        int16_t text_x = x + 2;
        int16_t text_y = y + 2;
        if (text_x >= 0 && text_x < width - 8 && text_y >= 0 && text_y < height - 8) {
            // Número simplificado (primeiro dígito)
            int digit = i % 10;
            for (int ty = 0; ty < 5; ty++) {
                for (int tx = 0; tx < 3; tx++) {
                    // Usar matriz simples 3x5 para representar dígitos
                    static const uint8_t digits[10][5][3] = {
                        {{1,1,1}, {1,0,1}, {1,0,1}, {1,0,1}, {1,1,1}}, // 0
                        {{0,1,0}, {1,1,0}, {0,1,0}, {0,1,0}, {1,1,1}}, // 1
                        {{1,1,1}, {0,0,1}, {1,1,1}, {1,0,0}, {1,1,1}}, // 2
                        {{1,1,1}, {0,0,1}, {0,1,1}, {0,0,1}, {1,1,1}}, // 3
                        {{1,0,1}, {1,0,1}, {1,1,1}, {0,0,1}, {0,0,1}}, // 4
                        {{1,1,1}, {1,0,0}, {1,1,1}, {0,0,1}, {1,1,1}}, // 5
                        {{1,1,1}, {1,0,0}, {1,1,1}, {1,0,1}, {1,1,1}}, // 6
                        {{1,1,1}, {0,0,1}, {0,1,0}, {1,0,0}, {1,0,0}}, // 7
                        {{1,1,1}, {1,0,1}, {1,1,1}, {1,0,1}, {1,1,1}}, // 8
                        {{1,1,1}, {1,0,1}, {1,1,1}, {0,0,1}, {1,1,1}}  // 9
                    };
                    
                    if (digits[digit][ty][tx]) {
                        framebuffer[(text_y + ty) * width + (text_x + tx)] = 0xFFFFFF;
                    }
                }
            }
        }
    }
    
    // Desenhar informações de status
    char status[64];
    sprintf(status, "Sprites: %d  Overflow: %d  Collision: %d", 
           sprite_system.sprite_count, 
           sprite_system.sprite_overflow, 
           sprite_system.sprite_collision);
    
    // Renderizar texto (simplificado para demonstração)
    int text_x = 10;
    int text_y = height - 20;
    
    // Desenhando fundo para o texto
    for (int y = text_y - 2; y < text_y + 10; y++) {
        if (y < 0 || y >= height) continue;
        
        for (int x = text_x - 2; x < text_x + 300; x++) {
            if (x < 0 || x >= width) continue;
            framebuffer[y * width + x] = 0x000000; // Fundo preto
        }
    }
    
    // Esta é apenas uma visualização rudimentar de texto
    // Uma implementação real usaria uma fonte bitmap
    for (int i = 0; status[i] != '\0' && i < 63; i++) {
        int cx = text_x + i * 8;
        if (cx >= width) break;
        
        // Desenhar apenas um retângulo para cada caractere
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 6; x++) {
                if (text_y + y >= 0 && text_y + y < height && 
                    cx + x >= 0 && cx + x < width) {
                    framebuffer[(text_y + y) * width + (cx + x)] = 0xFFFFFF;
                }
            }
        }
    }
}
