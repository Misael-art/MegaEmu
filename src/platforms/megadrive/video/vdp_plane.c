/**
 * @file vdp_plane.c
 * @brief Implementação das funções de renderização de planos (A, B e Window) do
 * VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-28
 */

#include "vdp_plane.h"
#include "vdp_adapter.h"
#include "vdp_color.h"
#include "vdp_memory.h"
#include "vdp_registers.h"
#include <stdlib.h>
#include <string.h>

// Constantes de renderização
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define CELL_WIDTH_MAX 64
#define CELL_HEIGHT_MAX 64

// Estrutura para informação de tile
typedef struct {
  uint16_t pattern_idx;
  uint8_t palette;
  bool priority;
  bool flip_h;
  bool flip_v;
} tile_info_t;

// Buffer para linha de tile
static uint8_t tile_line_buffer[TILE_WIDTH];

/**
 * @brief Decodifica dados de um tile para obter suas propriedades
 *
 * @param pattern_word Palavra de padrão do tile
 * @param info Estrutura para armazenar informações decodificadas
 */
static void decode_tile_info(uint16_t pattern_word, tile_info_t *info) {
  info->pattern_idx = pattern_word & 0x07FF;     // Bits 0-10: índice do padrão
  info->palette = (pattern_word >> 13) & 0x03;   // Bits 13-14: paleta (0-3)
  info->priority = (pattern_word & 0x8000) != 0; // Bit 15: prioridade
  info->flip_h = (pattern_word & 0x0800) != 0;   // Bit 11: flip horizontal
  info->flip_v = (pattern_word & 0x1000) != 0;   // Bit 12: flip vertical
}

/**
 * @brief Decodifica uma linha de um tile
 *
 * @param vdp_ctx Contexto do VDP
 * @param pattern_idx Índice do padrão de tile
 * @param line Número da linha dentro do tile (0-7)
 * @param flip_h Flag de flip horizontal
 * @param flip_v Flag de flip vertical
 * @param output Buffer de saída para a linha do tile
 */
static void decode_tile_line(vdp_context_t *vdp_ctx, uint16_t pattern_idx,
                             int line, bool flip_h, bool flip_v,
                             uint8_t *output) {
  // Se flip vertical, inverte a linha
  if (flip_v) {
    line = 7 - line;
  }

  // Calcula endereço na VRAM para o tile e linha específicos
  uint32_t addr = (pattern_idx * 32) + (line * 4); // 4 bytes por linha
  const uint8_t *src = &vdp_ctx->vram[addr];

  // Processa cada byte na linha (4 bytes = 8 pixels de 4 bits cada)
  for (int i = 0; i < 4; i++) {
    int byte_idx = flip_h ? (3 - i) : i; // Inverte bytes se flip horizontal
    uint8_t data = src[byte_idx];

    // Extrai os dois pixels deste byte
    int pixel_idx = i * 2;
    if (flip_h) {
      // Se flip horizontal, inverte a ordem dos pixels no buffer
      output[7 - pixel_idx - 0] =
          (data >> 4) & 0x0F;                  // Pixel superior (bits 4-7)
      output[7 - pixel_idx - 1] = data & 0x0F; // Pixel inferior (bits 0-3)
    } else {
      output[pixel_idx + 0] = (data >> 4) & 0x0F; // Pixel superior
      output[pixel_idx + 1] = data & 0x0F;        // Pixel inferior
    }
  }
}

/**
 * @brief Renderiza plano A em uma linha
 *
 * Processa e renderiza uma linha do plano A, considerando scroll e prioridade.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha
 */
void vdp_render_plane_a_line(vdp_context_t *vdp_ctx, int line,
                             uint16_t *line_buffer) {
  // Limpa o buffer de linha
  memset(line_buffer, 0, vdp_ctx->screen_width * sizeof(uint16_t));

  // Obtém parâmetros do plano A
  uint16_t plane_addr = (vdp_ctx->registers[VDP_REG_PLANE_A] << 10) & 0xF000;

  // Obtém valores de scroll
  uint16_t scroll_x = ((vdp_ctx->vsram[0] << 8) | vdp_ctx->vsram[1]) & 0x3FF;
  uint16_t scroll_y = ((vdp_ctx->vsram[2] << 8) | vdp_ctx->vsram[3]) & 0x3FF;

  // Calcula a linha efetiva após aplicar scroll
  int effective_y = (line + scroll_y) % (vdp_ctx->plane_a_height * 8);
  int row = effective_y / 8;
  int tile_y = effective_y % 8;

  // Renderiza cada coluna na linha
  int screen_width = IS_H40_MODE(vdp_ctx) ? 320 : 256;
  int plane_width = vdp_ctx->plane_a_width;

  for (int x = 0; x < screen_width; x += 8) {
    // Calcular posição X efetiva após aplicar scroll
    int effective_x = (x + scroll_x) % (plane_width * 8);
    int col = effective_x / 8;

    // Calcula endereço da entrada do tile
    uint16_t tile_addr = plane_addr + ((row * plane_width + col) * 2);

    // Lê a palavra do padrão de tile
    uint16_t pattern_word =
        (vdp_ctx->vram[tile_addr] << 8) | vdp_ctx->vram[tile_addr + 1];

    // Decodifica informações do tile
    tile_info_t info;
    decode_tile_info(pattern_word, &info);

    // Se o tile tem um padrão válido, renderiza
    if (info.pattern_idx != 0 || info.palette != 0) {
      // Decodifica a linha do tile
      decode_tile_line(vdp_ctx, info.pattern_idx, tile_y, info.flip_h,
                       info.flip_v, tile_line_buffer);

      // Copia pixels para o buffer de linha
      for (int i = 0; i < 8; i++) {
        int out_x = x + i;
        if (out_x < screen_width) {
          uint8_t pixel = tile_line_buffer[i];
          if (pixel != 0) { // Se não é transparente
            // Cria pixel com metadados usando função de vdp_color.h
            line_buffer[out_x] =
                vdp_create_pixel_type(pixel,                 // Índice de cor
                                      info.priority ? 1 : 0, // Prioridade
                                      info.palette,          // Paleta
                                      false,                 // Não é shadow
                                      false                  // Não é highlight
                );
          }
        }
      }
    }
  }
}

/**
 * @brief Renderiza plano B em uma linha
 *
 * Processa e renderiza uma linha do plano B, considerando scroll e prioridade.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha
 */
void vdp_render_plane_b_line(vdp_context_t *vdp_ctx, int line,
                             uint16_t *line_buffer) {
  // Limpa o buffer de linha
  memset(line_buffer, 0, vdp_ctx->screen_width * sizeof(uint16_t));

  // Obtém parâmetros do plano B
  uint16_t plane_addr = (vdp_ctx->registers[VDP_REG_PLANE_B] << 13) & 0xF000;

  // Obtém valores de scroll
  uint16_t scroll_x = ((vdp_ctx->vsram[4] << 8) | vdp_ctx->vsram[5]) & 0x3FF;
  uint16_t scroll_y = ((vdp_ctx->vsram[6] << 8) | vdp_ctx->vsram[7]) & 0x3FF;

  // Calcula a linha efetiva após aplicar scroll
  int effective_y = (line + scroll_y) % (vdp_ctx->plane_b_height * 8);
  int row = effective_y / 8;
  int tile_y = effective_y % 8;

  // Renderiza cada coluna na linha
  int screen_width = IS_H40_MODE(vdp_ctx) ? 320 : 256;
  int plane_width = vdp_ctx->plane_b_width;

  for (int x = 0; x < screen_width; x += 8) {
    // Calcular posição X efetiva após aplicar scroll
    int effective_x = (x + scroll_x) % (plane_width * 8);
    int col = effective_x / 8;

    // Calcula endereço da entrada do tile
    uint16_t tile_addr = plane_addr + ((row * plane_width + col) * 2);

    // Lê a palavra do padrão de tile
    uint16_t pattern_word =
        (vdp_ctx->vram[tile_addr] << 8) | vdp_ctx->vram[tile_addr + 1];

    // Decodifica informações do tile
    tile_info_t info;
    decode_tile_info(pattern_word, &info);

    // Se o tile tem um padrão válido, renderiza
    if (info.pattern_idx != 0 || info.palette != 0) {
      // Decodifica a linha do tile
      decode_tile_line(vdp_ctx, info.pattern_idx, tile_y, info.flip_h,
                       info.flip_v, tile_line_buffer);

      // Copia pixels para o buffer de linha
      for (int i = 0; i < 8; i++) {
        int out_x = x + i;
        if (out_x < screen_width) {
          uint8_t pixel = tile_line_buffer[i];
          if (pixel != 0) { // Se não é transparente
            // Cria pixel com metadados usando função de vdp_color.h
            line_buffer[out_x] =
                vdp_create_pixel_type(pixel,                 // Índice de cor
                                      info.priority ? 1 : 0, // Prioridade
                                      info.palette,          // Paleta
                                      false,                 // Não é shadow
                                      false                  // Não é highlight
                );
          }
        }
      }
    }
  }
}

/**
 * @brief Renderiza janela em uma linha
 *
 * Processa e renderiza uma linha da janela, verificando se a linha está na área
 * da janela.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha
 */
void vdp_render_window_line(vdp_context_t *vdp_ctx, int line,
                            uint16_t *line_buffer) {
  // Limpa o buffer de linha
  memset(line_buffer, 0, vdp_ctx->screen_width * sizeof(uint16_t));

  // Se a janela não está habilitada, retorna
  if (!vdp_ctx->window_enabled) {
    return;
  }

  // Obtém base da janela
  uint16_t window_addr = (vdp_ctx->registers[VDP_REG_WINDOW] << 10) & 0xF000;

  // Verifica se a linha está dentro da área da janela
  int window_v_pos = (vdp_ctx->registers[0x12] & 0x1F) * 8;
  int window_v_mode = (vdp_ctx->registers[0x12] & 0x80) != 0;

  // Determina se a linha está na área da janela (baseado no modo vertical)
  bool in_window;
  if (window_v_mode) {
    // Modo down: janela na parte inferior da tela
    in_window = line >= window_v_pos;
  } else {
    // Modo up: janela na parte superior da tela
    in_window = line < window_v_pos;
  }

  if (!in_window) {
    return;
  }

  // Calcula área horizontal da janela
  int window_h_pos = (vdp_ctx->registers[0x11] & 0x1F) * 16;
  int window_h_mode = (vdp_ctx->registers[0x11] & 0x80) != 0;
  int screen_width = IS_H40_MODE(vdp_ctx) ? 320 : 256;

  // Calcula a linha dentro da janela (sem scroll)
  int window_line = line;
  int row = window_line / 8;
  int tile_y = window_line % 8;

  // Determina a largura da área da janela
  int start_x, end_x;
  if (window_h_mode) {
    // Modo right: janela do lado direito
    start_x = window_h_pos;
    end_x = screen_width;
  } else {
    // Modo left: janela do lado esquerdo
    start_x = 0;
    end_x = window_h_pos;
  }

  // Renderiza a linha da janela
  for (int x = start_x; x < end_x; x += 8) {
    // Calcula a coluna na janela
    int col = (x - start_x) / 8;

    // Calcula endereço da entrada do tile
    uint16_t tile_addr =
        window_addr + ((row * vdp_ctx->window_width + col) * 2);

    // Lê a palavra do padrão de tile
    uint16_t pattern_word =
        (vdp_ctx->vram[tile_addr] << 8) | vdp_ctx->vram[tile_addr + 1];

    // Decodifica informações do tile
    tile_info_t info;
    decode_tile_info(pattern_word, &info);

    // Se o tile tem um padrão válido, renderiza
    if (info.pattern_idx != 0 || info.palette != 0) {
      // Decodifica a linha do tile
      decode_tile_line(vdp_ctx, info.pattern_idx, tile_y, info.flip_h,
                       info.flip_v, tile_line_buffer);

      // Copia pixels para o buffer de linha
      for (int i = 0; i < 8; i++) {
        int out_x = x + i;
        if (out_x < end_x) {
          uint8_t pixel = tile_line_buffer[i];
          if (pixel != 0) { // Se não é transparente
            // Cria pixel com metadados usando função de vdp_color.h
            line_buffer[out_x] =
                vdp_create_pixel_type(pixel,                 // Índice de cor
                                      info.priority ? 1 : 0, // Prioridade
                                      info.palette,          // Paleta
                                      false,                 // Não é shadow
                                      false                  // Não é highlight
                );
          }
        }
      }
    }
  }
}

/**
 * @brief Verifica se um ponto está dentro da área da janela
 *
 * @param vdp_ctx Contexto do VDP
 * @param x Coordenada X
 * @param y Coordenada Y
 * @return true se o ponto está na janela, false caso contrário
 */
bool vdp_is_point_in_window(vdp_context_t *vdp_ctx, int x, int y) {
  if (!vdp_ctx->window_enabled) {
    return false;
  }

  // Obtém parâmetros da janela
  int window_v_pos = (vdp_ctx->registers[0x12] & 0x1F) * 8;
  int window_v_mode = (vdp_ctx->registers[0x12] & 0x80) != 0;
  int window_h_pos = (vdp_ctx->registers[0x11] & 0x1F) * 16;
  int window_h_mode = (vdp_ctx->registers[0x11] & 0x80) != 0;

  // Verifica posição vertical
  bool in_v_window;
  if (window_v_mode) {
    // Modo down: janela na parte inferior
    in_v_window = y >= window_v_pos;
  } else {
    // Modo up: janela na parte superior
    in_v_window = y < window_v_pos;
  }

  // Verifica posição horizontal
  bool in_h_window;
  if (window_h_mode) {
    // Modo right: janela do lado direito
    in_h_window = x >= window_h_pos;
  } else {
    // Modo left: janela do lado esquerdo
    in_h_window = x < window_h_pos;
  }

  // O ponto está na janela se estiver tanto na área vertical quanto horizontal
  return in_v_window && in_h_window;
}
