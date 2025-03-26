/**
 * @file vdp_render.c
 * @brief Implementação das funções de renderização do VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "vdp_adapter.h"
#include <stdlib.h>
#include <string.h>
#include "vdp_render.h"
#include "vdp_color.h"
#include "vdp_sprite.h"
#include "vdp_plane.h"
#include "vdp_registers.h"
#include "vdp_memory.h"

// Constantes de renderização
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)
#define TILE_ROW_SIZE (TILE_WIDTH / 2) // 4 pixels por byte

#define PLANE_CELL_WIDTH 64
#define PLANE_CELL_HEIGHT 32

#define SPRITE_MAX_PER_LINE 20
#define SPRITE_MAX_PIXELS_LINE 320

// Estruturas auxiliares
typedef struct {
  uint16_t pattern_idx;
  uint8_t palette;
  bool priority;
  bool flip_h;
  bool flip_v;
} tile_info_t;

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  uint16_t pattern;
  uint8_t palette;
  bool priority;
} sprite_info_t;

// Cache de tiles para otimização
static uint8_t tile_cache[TILE_SIZE];
static uint8_t line_buffer[SPRITE_MAX_PIXELS_LINE];
static uint8_t priority_buffer[SPRITE_MAX_PIXELS_LINE];

// Buffer de linha temporário para composição de pixels
static uint16_t line_buffer_a[320];      // Plano A
static uint16_t line_buffer_b[320];      // Plano B
static uint16_t line_buffer_s[320];      // Sprites
static uint16_t line_buffer_w[320];      // Window
static uint16_t line_buffer_final[320];  // Buffer final

// Flags para controle de renderização
static bool shadow_highlight_enabled = false;

// Funções auxiliares estáticas
static void decode_tile_info(uint16_t pattern_word, tile_info_t *info) {
  info->pattern_idx = pattern_word & 0x07FF;
  info->palette = (pattern_word >> 13) & 0x03;
  info->priority = (pattern_word & 0x8000) != 0;
  info->flip_h = (pattern_word & 0x0800) != 0;
  info->flip_v = (pattern_word & 0x1000) != 0;
}

static void decode_sprite_info(const uint8_t *sprite_data,
                               sprite_info_t *info) {
  // Formato do sprite: y (2 bytes), size (2 bytes), link (1 byte), attr (1
  // byte), x (2 bytes)
  info->y = ((uint16_t)sprite_data[0] << 8) | sprite_data[1];
  uint8_t size = sprite_data[2];
  info->width = ((size >> 2) & 0x03) + 1;
  info->height = (size & 0x03) + 1;
  info->width *= 8;
  info->height *= 8;

  uint16_t attr = ((uint16_t)sprite_data[5] << 8) | sprite_data[4];
  info->pattern = attr & 0x07FF;
  info->palette = (attr >> 13) & 0x03;
  info->priority = (attr & 0x8000) != 0;

  info->x = ((uint16_t)sprite_data[6] << 8) | sprite_data[7];
  if (info->x >= 0x80)
    info->x = -(0x100 - info->x);
}

static void decode_tile_line(const uint8_t *tile_data, int line, bool flip_h,
                             bool flip_v, uint8_t *output) {
  // Cada linha tem 4 bytes (32 bits = 8 pixels de 4 bits)
  if (flip_v)
    line = 7 - line;
  const uint8_t *src = tile_data + (line * 4);

  // Processa 2 pixels por byte
  for (int i = 0; i < 4; i++) {
    uint8_t pixel_pair = src[flip_h ? (3 - i) : i];
    int out_idx = i * 2;
    if (flip_h)
      out_idx = 6 - out_idx;

    output[out_idx] = pixel_pair >> 4;
    output[out_idx + 1] = pixel_pair & 0x0F;
  }
}

static void render_plane_line(megadrive_vdp_context_t *ctx, int line,
                              uint16_t plane_addr, int scroll_x, int scroll_y,
                              uint8_t *output, uint8_t *priority) {
  int effective_y = (line + scroll_y) & 0xFF;
  int row = effective_y >> 3;
  int tile_y = effective_y & 7;

  // Calcula endereço base da linha de tiles
  uint16_t row_addr = plane_addr + (row * PLANE_CELL_WIDTH * 2);

  // Para cada coluna visível
  for (int col = 0; col < (IS_H40_MODE(ctx) ? 40 : 32); col++) {
    int effective_x = (col * 8 - (scroll_x & 0x3FF));
    if (effective_x < 0)
      effective_x += IS_H40_MODE(ctx) ? 320 : 256;

    // Lê dados do tile
    uint16_t pattern_word = (ctx->vram[row_addr + col * 2] << 8) |
                            ctx->vram[row_addr + col * 2 + 1];

    tile_info_t info;
    decode_tile_info(pattern_word, &info);

    // Decodifica linha do tile
    const uint8_t *tile_data = &ctx->vram[info.pattern_idx * 32];
    decode_tile_line(tile_data, tile_y, info.flip_h, info.flip_v, tile_cache);

    // Copia pixels para o buffer de saída
    for (int x = 0; x < 8; x++) {
      int out_x = effective_x + x;
      if (out_x >= 0 && out_x < (IS_H40_MODE(ctx) ? 320 : 256)) {
        uint8_t pixel = tile_cache[x];
        if (pixel != 0) { // Pixel não transparente
          output[out_x] = pixel | (info.palette << 4);
          priority[out_x] = info.priority ? 1 : 0;
        }
      }
    }
  }
}

static void render_sprites_line(megadrive_vdp_context_t *ctx, int line,
                                uint8_t *output, uint8_t *priority) {
  int sprite_count = 0;
  sprite_info_t sprites[SPRITE_MAX_PER_LINE];

  // Encontra sprites visíveis nesta linha
  uint16_t sprite_table = (ctx->regs[5] << 9) & 0xFF00;
  for (int i = 0; i < 80 && sprite_count < SPRITE_MAX_PER_LINE; i++) {
    const uint8_t *sprite_data = &ctx->vram[sprite_table + i * 8];

    sprite_info_t info;
    decode_sprite_info(sprite_data, &info);

    // Verifica se o sprite está visível nesta linha
    if (line >= info.y && line < (info.y + info.height)) {
      sprites[sprite_count++] = info;
    }
  }

  // Renderiza sprites em ordem reversa (maior prioridade por último)
  for (int i = sprite_count - 1; i >= 0; i--) {
    sprite_info_t *sprite = &sprites[i];
    int sprite_line = line - sprite->y;

    // Decodifica linha do sprite
    const uint8_t *sprite_data = &ctx->vram[sprite->pattern * 32];
    for (int tile_row = 0; tile_row < sprite->height / 8; tile_row++) {
      if ((sprite_line / 8) == tile_row) {
        decode_tile_line(sprite_data + (tile_row * 32), sprite_line % 8, false,
                         false, tile_cache);

        // Copia pixels para o buffer de saída
        for (int x = 0; x < sprite->width; x++) {
          int out_x = sprite->x + x;
          if (out_x >= 0 && out_x < (IS_H40_MODE(ctx) ? 320 : 256)) {
            uint8_t pixel = tile_cache[x % 8];
            if (pixel != 0) { // Pixel não transparente
              output[out_x] = pixel | (sprite->palette << 4);
              priority[out_x] = sprite->priority ? 2 : 1;
            }
          }
        }
        break;
      }
    }
  }
}

// Funções públicas
void vdp_render_line(megadrive_vdp_context_t *ctx, int line, uint8_t *output) {
  if (!ctx || !output || line < 0 || line >= (IS_V30_MODE(ctx) ? 240 : 224))
    return;

  // Limpa buffers
  memset(line_buffer, 0, sizeof(line_buffer));
  memset(priority_buffer, 0, sizeof(priority_buffer));
  memset(output, 0, IS_H40_MODE(ctx) ? 320 : 256);

  // Obtém endereços dos planos
  uint16_t plane_a = (ctx->regs[2] << 10) & 0xF000;
  uint16_t plane_b = (ctx->regs[4] << 13) & 0xF000;
  uint16_t window = (ctx->regs[3] << 10) & 0xF000;

  // Obtém valores de scroll
  int scroll_a_x = ((ctx->regs[8] << 8) | ctx->regs[9]) & 0x3FF;
  int scroll_a_y = ((ctx->regs[10] << 8) | ctx->regs[11]) & 0x3FF;
  int scroll_b_x = ((ctx->regs[12] << 8) | ctx->regs[13]) & 0x3FF;
  int scroll_b_y = ((ctx->regs[14] << 8) | ctx->regs[15]) & 0x3FF;

  // Renderiza plano B (background)
  render_plane_line(ctx, line, plane_b, scroll_b_x, scroll_b_y, line_buffer,
                    priority_buffer);

  // Renderiza plano A
  render_plane_line(ctx, line, plane_a, scroll_a_x, scroll_a_y, line_buffer,
                    priority_buffer);

  // Renderiza sprites
  render_sprites_line(ctx, line, line_buffer, priority_buffer);

  // Aplica shadow/highlight se habilitado
  bool shadow_highlight = (ctx->regs[0x0C] & 0x08) != 0;
  if (shadow_highlight) {
    for (int x = 0; x < (IS_H40_MODE(ctx) ? 320 : 256); x++) {
      uint8_t pixel = line_buffer[x];
      uint8_t prio = priority_buffer[x];

      // Aplica efeito baseado na prioridade
      if (prio == 2) { // Highlight
        pixel |= 0x40;
      } else if (prio == 0) { // Shadow
        pixel |= 0x80;
      }
      output[x] = pixel;
    }
  } else {
    // Copia buffer final sem efeitos
    memcpy(output, line_buffer, IS_H40_MODE(ctx) ? 320 : 256);
  }
}

/**
 * @brief Inicializa o sistema de renderização
 *
 * Configura buffers e estados iniciais para renderização
 *
 * @param vdp_ctx Contexto do VDP
 */
void vdp_render_init(vdp_context_t* vdp_ctx) {
    if (!vdp_ctx) {
        return;
    }

    // Inicializa a tabela de cores
    vdp_color_init();

    // Inicializa shadow/highlight com base nos registros iniciais
    vdp_update_shadow_highlight_mode(vdp_ctx->registers[VDP_REG_MODE4]);

    // Limpa todos os buffers de renderização
    memset(line_buffer_a, 0, sizeof(line_buffer_a));
    memset(line_buffer_b, 0, sizeof(line_buffer_b));
    memset(line_buffer_w, 0, sizeof(line_buffer_w));
    memset(line_buffer_s, 0, sizeof(line_buffer_s));
    memset(line_buffer_final, 0, sizeof(line_buffer_final));

    // Inicializa buffers auxiliares
    memset(line_buffer, 0, sizeof(line_buffer));
    memset(priority_buffer, 0, sizeof(priority_buffer));
    memset(tile_cache, 0, sizeof(tile_cache));

    // Configura tamanho da tela com base no modo
    vdp_ctx->screen_width = IS_H40_MODE(vdp_ctx) ? 320 : 256;
    vdp_ctx->screen_height = IS_PAL_MODE(vdp_ctx) ? 240 : 224;
    vdp_ctx->window_enabled = false;

    // Atualiza configurações com base nos registros
    vdp_render_update_config(vdp_ctx);
}

/**
 * @brief Atualiza configurações de renderização baseado em registros
 *
 * @param vdp_ctx Contexto do VDP
 */
void vdp_render_update_config(vdp_context_t* vdp_ctx) {
    if (!vdp_ctx) {
        return;
    }

    // Atualiza tamanho da tela com base no modo
    vdp_ctx->screen_width = IS_H40_MODE(vdp_ctx) ? 320 : 256;

    // Dimensões dependem do modo PAL ou NTSC
    vdp_ctx->screen_height = IS_PAL_MODE(vdp_ctx) ? 240 : 224;

    // Atualiza flag de Shadow/Highlight
    vdp_update_shadow_highlight_mode(vdp_ctx->registers[VDP_REG_MODE4]);

    // Atualiza status de janela
    vdp_ctx->window_enabled = (vdp_ctx->registers[0x12] & 0x1F) > 0 ||
                             (vdp_ctx->registers[0x11] & 0x1F) > 0;
}

/**
 * @brief Processa um scanline completo para renderização
 *
 * Esta função orquestra a renderização de um scanline, incluindo sprites,
 * planos A e B, janela, e aplicando efeitos de shadow/highlight.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a ser processada
 */
void vdp_process_scanline(vdp_context_t* vdp_ctx, int line) {
    // Verificação de contexto válido
    if (!vdp_ctx || line >= vdp_ctx->screen_height) {
        return;
    }

    // Se a tela está desabilitada, preenche com cor de fundo e retorna
    if (!(vdp_ctx->registers[VDP_REG_MODE2] & VDP_REG2_DISPLAY_ENABLE)) {
        uint16_t bg_color_index = vdp_ctx->registers[VDP_REG_BGCOLOR] & 0x3F;
        uint32_t bg_color = vdp_calculate_color(bg_color_index, vdp_ctx->cram, false, false);
        uint32_t* fb_line = vdp_ctx->framebuffer + (line * vdp_ctx->framebuffer_stride);

        for (int x = 0; x < vdp_ctx->screen_width; x++) {
            fb_line[x] = bg_color;
        }

        // Notifica callback, se existir
        if (vdp_ctx->line_rendered_callback) {
            vdp_ctx->line_rendered_callback(vdp_ctx->callback_data, line);
        }

        return;
    }

    // Verifica e atualiza a configuração do VDP, se necessário
    vdp_render_update_config(vdp_ctx);

    // Limpa os buffers de linha para nova renderização
    memset(line_buffer_b, 0, vdp_ctx->screen_width * sizeof(uint16_t));
    memset(line_buffer_a, 0, vdp_ctx->screen_width * sizeof(uint16_t));
    memset(line_buffer_w, 0, vdp_ctx->screen_width * sizeof(uint16_t));
    memset(line_buffer_s, 0, vdp_ctx->screen_width * sizeof(uint16_t));
    memset(line_buffer_final, 0, vdp_ctx->screen_width * sizeof(uint16_t));

    // 1. Renderiza cada componente em seu próprio buffer

    // Renderiza sprites em seu buffer
    vdp_render_sprites_line(vdp_ctx, line, line_buffer_s);

    // Renderiza plano B em seu buffer
    vdp_render_plane_b_line(vdp_ctx, line, line_buffer_b);

    // Renderiza plano A em seu buffer
    vdp_render_plane_a_line(vdp_ctx, line, line_buffer_a);

    // Renderiza janela em seu buffer, se habilitada
    if (vdp_ctx->window_enabled) {
        vdp_render_window_line(vdp_ctx, line, line_buffer_w);
    }

    // 2. Combina todos os elementos seguindo as regras de prioridade e S/H
    vdp_combine_line(vdp_ctx, line);

    // 3. Converte o buffer combinado para o framebuffer final
    vdp_render_line_to_framebuffer(vdp_ctx, line);
}

/**
 * @brief Combina todos os elementos de um scanline respeitando prioridades e Shadow/Highlight
 *
 * Esta função implementa as regras complexas de composição de pixels do VDP, incluindo
 * o processamento correto dos efeitos Shadow/Highlight.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 */
void vdp_combine_line(vdp_context_t* vdp_ctx, int line) {
    // Verifica se o contexto é válido
    if (!vdp_ctx) {
        return;
    }

    uint16_t* cram = vdp_ctx->cram;
    int width = vdp_ctx->screen_width;
    bool is_shadow_enabled = vdp_is_shadow_highlight_enabled();

    // Obtém cor de fundo da linha (background)
    uint16_t bg_color_index = vdp_ctx->registers[VDP_REG_BGCOLOR] & 0x3F;
    uint8_t bg_palette = bg_color_index >> 4;
    uint8_t bg_color = bg_color_index & 0xF;

    // Configura buffer final com cor de fundo (início da composição)
    for (int x = 0; x < width; x++) {
        // Cor de fundo sem nenhum efeito para começar
        line_buffer_final[x] = vdp_create_pixel_type(
            bg_color,       // Índice de cor
            0,              // Sem prioridade
            bg_palette,     // Paleta da cor de fundo
            false,          // Não é shadow
            false           // Não é highlight
        );
    }

    // FASE 1: Componentes de baixa prioridade
    // Ordem da renderização (do fundo para frente):
    // 1. Background (já configurado acima)
    // 2. Plano B com baixa prioridade
    // 3. Sprites com baixa prioridade (incluindo efeitos shadow)
    // 4. Plano A/Window com baixa prioridade

    // Primeira passada: Renderizar plano B com baixa prioridade
    for (int x = 0; x < width; x++) {
        uint16_t plane_b_pixel = line_buffer_b[x];
        uint8_t color_index = vdp_get_color_index(plane_b_pixel);

        // Plano B com baixa prioridade
        if (color_index != 0 && !vdp_has_priority(plane_b_pixel)) {
            line_buffer_final[x] = plane_b_pixel;
        }
    }

    // Segunda passada: Renderizar sprites com baixa prioridade
    for (int x = 0; x < width; x++) {
        uint16_t sprite_pixel = line_buffer_s[x];
        uint8_t color_index = vdp_get_color_index(sprite_pixel);

        if (color_index != 0 && !vdp_has_priority(sprite_pixel)) {
            // Lógica para shadow/highlight de sprites especiais
            if (is_shadow_enabled) {
                if (vdp_is_shadow_effect_sprite(sprite_pixel)) {
                    // Sprite shadow especial - não renderiza o sprite, mas aplica efeito shadow
                    line_buffer_final[x] = vdp_apply_shadow_to_pixel(line_buffer_final[x]);
                    continue;
                }
                else if (vdp_is_highlight_effect_sprite(sprite_pixel)) {
                    // Sprite highlight especial - não renderiza o sprite, mas aplica efeito highlight
                    line_buffer_final[x] = vdp_apply_highlight_to_pixel(line_buffer_final[x]);
                    continue;
                }
            }

            // Sprite normal - sobrepõe completamente
            line_buffer_final[x] = sprite_pixel;
        }
    }

    // Terceira passada: Renderizar plano A ou window com baixa prioridade
    for (int x = 0; x < width; x++) {
        // Determinar se usamos pixels do plano A ou da janela
        uint16_t plane_a_pixel;

        // Verificar se o ponto está dentro da área da janela
        if (vdp_ctx->window_enabled && vdp_is_point_in_window(vdp_ctx, x, line)) {
            plane_a_pixel = line_buffer_w[x]; // Usar window
        } else {
            plane_a_pixel = line_buffer_a[x]; // Usar plano A
        }

        // Renderizar apenas se visível e com baixa prioridade
        uint8_t color_index = vdp_get_color_index(plane_a_pixel);
        if (color_index != 0 && !vdp_has_priority(plane_a_pixel)) {
            line_buffer_final[x] = plane_a_pixel;
        }
    }

    // FASE 2: Componentes de alta prioridade
    // Ordem da renderização:
    // 1. Plano B com alta prioridade
    // 2. Plano A/Window com alta prioridade
    // 3. Sprites com alta prioridade (incluindo efeitos shadow)

    // Quarta passada: Renderizar plano B com alta prioridade
    for (int x = 0; x < width; x++) {
        uint16_t plane_b_pixel = line_buffer_b[x];
        uint8_t color_index = vdp_get_color_index(plane_b_pixel);

        // Plano B com alta prioridade
        if (color_index != 0 && vdp_has_priority(plane_b_pixel)) {
            line_buffer_final[x] = plane_b_pixel;
        }
    }

    // Quinta passada: Renderizar plano A ou window com alta prioridade
    for (int x = 0; x < width; x++) {
        // Determinar se usamos pixels do plano A ou da janela
        uint16_t plane_a_pixel;

        // Verificar se o ponto está dentro da área da janela
        if (vdp_ctx->window_enabled && vdp_is_point_in_window(vdp_ctx, x, line)) {
            plane_a_pixel = line_buffer_w[x]; // Usar window
        } else {
            plane_a_pixel = line_buffer_a[x]; // Usar plano A
        }

        // Renderizar apenas se visível e com alta prioridade
        uint8_t color_index = vdp_get_color_index(plane_a_pixel);
        if (color_index != 0 && vdp_has_priority(plane_a_pixel)) {
            line_buffer_final[x] = plane_a_pixel;
        }
    }

    // Sexta passada: Renderizar sprites com alta prioridade
    for (int x = 0; x < width; x++) {
        uint16_t sprite_pixel = line_buffer_s[x];
        uint8_t color_index = vdp_get_color_index(sprite_pixel);

        if (color_index != 0 && vdp_has_priority(sprite_pixel)) {
            // Lógica para shadow/highlight de sprites especiais
            if (is_shadow_enabled) {
                if (vdp_is_shadow_effect_sprite(sprite_pixel)) {
                    // Sprite shadow especial - não renderiza o sprite, mas aplica efeito shadow
                    line_buffer_final[x] = vdp_apply_shadow_to_pixel(line_buffer_final[x]);
                    continue;
                }
                else if (vdp_is_highlight_effect_sprite(sprite_pixel)) {
                    // Sprite highlight especial - não renderiza o sprite, mas aplica efeito highlight
                    line_buffer_final[x] = vdp_apply_highlight_to_pixel(line_buffer_final[x]);
                    continue;
                }
            }

            // Sprite normal - sobrepõe completamente
            line_buffer_final[x] = sprite_pixel;
        }
    }
}

/**
 * @brief Renderiza a linha final para o framebuffer
 *
 * Converte os pixels combinados com metadados para cores RGB finais no framebuffer.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 */
void vdp_render_line_to_framebuffer(vdp_context_t* vdp_ctx, int line) {
    if (!vdp_ctx || !vdp_ctx->framebuffer || line >= vdp_ctx->screen_height) {
        return;
    }

    // Offset no framebuffer para esta linha
    uint32_t* fb_line = vdp_ctx->framebuffer + (line * vdp_ctx->framebuffer_stride);
    int width = vdp_ctx->screen_width;

    // Converter cada pixel para RGB final
    for (int x = 0; x < width; x++) {
        uint16_t pixel_type = line_buffer_final[x];
        fb_line[x] = vdp_pixel_to_rgb(pixel_type, vdp_ctx->cram);
    }

    // Chamar callback de linha renderizada, se existir
    if (vdp_ctx->line_rendered_callback) {
        vdp_ctx->line_rendered_callback(vdp_ctx->callback_data, line);
    }
}
