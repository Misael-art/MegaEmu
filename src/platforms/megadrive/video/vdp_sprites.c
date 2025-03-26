/**
 * @file vdp_sprites.c
 * @brief Implementação da renderização de sprites do VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-25
 */

#include "vdp_adapter.h"
#include <stdbool.h>
#include <string.h>

// Constantes para sprites
#define SPRITE_TABLE_SIZE 80 // 80 sprites
#define SPRITE_MAX_PER_LINE                                                    \
  20 // Máximo de sprites por linha (limitação de hardware)
#define SPRITE_MAX_LINK 80 // Valor máximo para link (0 = link final)

// Estrutura para sprite
typedef struct {
  uint16_t y;       // Posição Y
  uint8_t size;     // Tamanho (bits 0-1: largura, bits 2-3: altura)
  uint8_t link;     // Link para próximo sprite (0 = fim da lista)
  uint16_t attr;    // Atributos (bits 11-15: paleta, bit 10: prioridade, bit 9:
                    // V-flip, bit 8: H-flip)
  uint16_t x;       // Posição X
  uint16_t pattern; // Índice do padrão de base
} vdp_sprite_t;

// Estrutura para sprite na linha
typedef struct {
  uint16_t x;       // Posição X
  uint16_t pattern; // Índice do padrão
  uint8_t width;    // Largura em tiles (1-4)
  uint8_t height;   // Altura em tiles (1-4)
  uint8_t palette;  // Índice da paleta (0-3)
  uint8_t priority; // Prioridade (0-1)
  uint8_t h_flip;   // Flip horizontal (0-1)
  uint8_t v_flip;   // Flip vertical (0-1)
  uint8_t offset_y; // Offset vertical dentro do sprite
} vdp_line_sprite_t;

// Buffer de sprites para uma linha
static vdp_line_sprite_t line_sprites[SPRITE_MAX_PER_LINE];
static uint8_t line_sprite_count;
static uint8_t line_sprite_overflow;
static uint8_t sprite_collision;

/**
 * @brief Lê um sprite da tabela de atributos
 * @param ctx Contexto do VDP
 * @param index Índice do sprite (0-79)
 * @param sprite Estrutura para receber os dados
 */
static void read_sprite_attributes(megadrive_vdp_context_t *ctx, uint8_t index,
                                   vdp_sprite_t *sprite) {
  uint16_t table_addr = (ctx->registers[VDP_REG_SPRITE] & 0x7F) << 9;
  uint16_t sprite_addr = table_addr + (index * 8);

  // Lê os 8 bytes da entrada de sprite
  sprite->y =
      ((ctx->vram[sprite_addr] << 8) | ctx->vram[sprite_addr + 1]) & 0x3FF;
  sprite->size = ctx->vram[sprite_addr + 2] & 0x0F;
  sprite->link = ctx->vram[sprite_addr + 3] & 0x7F;
  sprite->attr = (ctx->vram[sprite_addr + 4] << 8) | ctx->vram[sprite_addr + 5];
  sprite->x =
      ((ctx->vram[sprite_addr + 6] << 8) | ctx->vram[sprite_addr + 7]) & 0x3FF;

  // Extrair índice de padrão do atributo
  sprite->pattern = sprite->attr & 0x07FF;
}

/**
 * @brief Calcula as dimensões de um sprite com base no campo de tamanho
 * @param size Campo de tamanho do sprite
 * @param width Ponteiro para receber a largura em tiles (1-4)
 * @param height Ponteiro para receber a altura em tiles (1-4)
 */
static void calculate_sprite_dimensions(uint8_t size, uint8_t *width,
                                        uint8_t *height) {
  static const uint8_t sprite_widths[] = {1, 2, 3, 4};
  static const uint8_t sprite_heights[] = {1, 2, 3, 4};

  *width = sprite_widths[size & 0x03];
  *height = sprite_heights[(size >> 2) & 0x03];
}

/**
 * @brief Coleta sprites visíveis para a linha atual
 * @param ctx Contexto do VDP
 * @param line Linha atual sendo renderizada
 */
void vdp_collect_line_sprites(megadrive_vdp_context_t *ctx, uint16_t line) {
  if (!ctx || line >= 240)
    return;

  // Reset dos buffers de linha
  line_sprite_count = 0;
  line_sprite_overflow = 0;
  sprite_collision = 0;

  // Verificar se sprites estão desabilitados
  if (!(ctx->registers[VDP_REG_MODE2] & 0x40))
    return;

  // Offset de 128 pixels para coordenadas Y
  int16_t line_offset = line + 128;

  // Tamanho do sprite em modo H40/H32
  bool h40_mode = (ctx->registers[VDP_REG_MODE4] & 0x01) != 0;
  uint16_t sprite_screen_width = h40_mode ? 320 : 256;

  // Iniciar pelo primeiro sprite da tabela
  uint8_t link = 0;
  uint8_t count = 0;

  // Percorrer a lista encadeada de sprites
  while (link < SPRITE_TABLE_SIZE && count < SPRITE_TABLE_SIZE) {
    vdp_sprite_t sprite;
    read_sprite_attributes(ctx, link, &sprite);

    // Calcular dimensões do sprite
    uint8_t width, height;
    calculate_sprite_dimensions(sprite.size, &width, &height);

    // Verificar se o sprite é visível nesta linha
    int16_t sprite_y = sprite.y - 128;
    if (line >= sprite_y && line < sprite_y + (height * 8)) {
      // Sprite está visível nesta linha
      if (line_sprite_count < SPRITE_MAX_PER_LINE) {
        // Calcular offset vertical dentro do sprite
        uint8_t offset_y = line - sprite_y;
        if (sprite.attr & 0x1000) { // V-flip
          offset_y = (height * 8) - 1 - offset_y;
        }

        // Adicionar sprite à lista de linha
        vdp_line_sprite_t *line_sprite = &line_sprites[line_sprite_count];
        line_sprite->x = sprite.x - 128;
        line_sprite->pattern = sprite.pattern;
        line_sprite->width = width;
        line_sprite->height = height;
        line_sprite->palette = (sprite.attr >> 13) & 0x03;
        line_sprite->priority = (sprite.attr >> 15) & 0x01;
        line_sprite->h_flip = (sprite.attr >> 11) & 0x01;
        line_sprite->v_flip = (sprite.attr >> 12) & 0x01;
        line_sprite->offset_y = offset_y;

        line_sprite_count++;
      } else {
        // Overflow de sprites
        line_sprite_overflow = 1;
        ctx->status |= VDP_STATUS_SOVR;
        break;
      }
    }

    // Seguir a lista encadeada
    link = sprite.link;
    if (link == 0 || link >= SPRITE_TABLE_SIZE)
      break;

    count++;
  }
}

/**
 * @brief Renderiza os sprites coletados para a linha atual no buffer de linha
 * @param ctx Contexto do VDP
 * @param line Linha atual sendo renderizada
 * @param line_buffer Buffer para a linha (320 pixels)
 * @param priority_buffer Buffer de prioridade (usado para detectar colisões)
 */
void vdp_render_line_sprites(megadrive_vdp_context_t *ctx, uint16_t line,
                             uint8_t *line_buffer, uint8_t *priority_buffer) {
  if (!ctx || !line_buffer || !priority_buffer || line >= 240)
    return;

  // Tamanho do sprite em modo H40/H32
  bool h40_mode = (ctx->registers[VDP_REG_MODE4] & 0x01) != 0;
  uint16_t sprite_screen_width = h40_mode ? 320 : 256;

  // Para cada sprite na linha (começando do último para o primeiro para
  // prioridade)
  for (int16_t s = line_sprite_count - 1; s >= 0; s--) {
    vdp_line_sprite_t *sprite = &line_sprites[s];

    // Posições de início e fim do sprite na linha
    int16_t start_x = sprite->x;
    int16_t end_x = start_x + (sprite->width * 8);

    // Clipping de tela
    if (start_x < 0)
      start_x = 0;
    if (end_x > sprite_screen_width)
      end_x = sprite_screen_width;

    // Calcular índice de padrão base para esta linha
    uint16_t pattern_line = sprite->pattern;
    pattern_line += (sprite->offset_y / 8) * sprite->width;

    // Offset de pixel dentro do tile
    uint8_t pixel_y = sprite->offset_y % 8;

    // Renderizar o sprite na linha
    for (int16_t x = start_x; x < end_x; x++) {
      // Calcular qual tile do sprite estamos renderizando
      int16_t sprite_x = x - sprite->x;
      if (sprite->h_flip) {
        sprite_x = (sprite->width * 8) - 1 - sprite_x;
      }

      uint16_t tile_x = sprite_x / 8;
      uint8_t pixel_x = sprite_x % 8;
      if (tile_x >= sprite->width)
        continue;

      // Calcular índice do padrão para este tile
      uint16_t pattern_index =
          pattern_line +
          (sprite->h_flip ? (sprite->width - 1 - tile_x) : tile_x);

      // Calcular endereço do padrão na VRAM (cada padrão tem 32 bytes)
      uint32_t pattern_addr = pattern_index * 32;

      // Calcular endereço para a linha específica dentro do padrão
      pattern_addr += pixel_y * 4;

      // Ler os 4 bytes que representam a linha de pixels do padrão
      uint32_t pixel_data = 0;
      for (int i = 0; i < 4; i++) {
        pixel_data = (pixel_data << 8) |
                     ctx->vram[(pattern_addr + i) & (MD_VDP_VRAM_SIZE - 1)];
      }

      // Calcular o pixel específico
      uint8_t pixel_shift = 28 - (pixel_x * 4);
      uint8_t color_index = (pixel_data >> pixel_shift) & 0x0F;

      // Se o pixel não é transparente
      if (color_index != 0) {
        // Calcular o índice de cor completo (palette + color_index)
        uint8_t full_color = (sprite->palette << 4) | color_index;

        // Verificar colisão com outros sprites
        if (priority_buffer[x] & 0x10) {
          // Colisão com outro sprite
          sprite_collision = 1;
          ctx->status |= VDP_STATUS_COLLISION;
        } else {
          // Marcar pixel como sprite
          priority_buffer[x] |= 0x10;
        }

        // Verificar prioridade com planos
        bool can_draw = false;

        if (sprite->priority) {
          // Sprite de alta prioridade só perde para pixels de alta prioridade
          // dos planos
          can_draw = !(priority_buffer[x] & 0x80);
        } else {
          // Sprite de baixa prioridade perde para qualquer pixel
          // não-transparente dos planos
          can_draw = !(priority_buffer[x] & 0x40);
        }

        // Desenhar pixel se tiver prioridade
        if (can_draw) {
          line_buffer[x] = full_color;

          // Atualizar buffer de prioridade (marcar como sprite e definir
          // prioridade)
          priority_buffer[x] =
              (priority_buffer[x] & 0x10) | (sprite->priority ? 0x20 : 0x00);
        }
      }
    }
  }
}

/**
 * @brief Verifica se ocorreu overflow de sprites na linha atual
 * @return 1 se ocorreu overflow, 0 caso contrário
 */
uint8_t vdp_get_sprite_overflow(void) { return line_sprite_overflow; }

/**
 * @brief Verifica se ocorreu colisão de sprites na linha atual
 * @return 1 se ocorreu colisão, 0 caso contrário
 */
uint8_t vdp_get_sprite_collision(void) { return sprite_collision; }

/**
 * @brief Limpa flags de colisão e overflow
 */
void vdp_clear_sprite_flags(void) {
  line_sprite_overflow = 0;
  sprite_collision = 0;
}

/**
 * @brief Obtém a contagem de sprites na linha atual
 * @return Número de sprites na linha
 */
uint8_t vdp_get_sprite_count(void) { return line_sprite_count; }
