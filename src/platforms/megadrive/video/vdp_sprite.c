/**
 * @file vdp_sprite.c
 * @brief Implementação da renderização de sprites do VDP do Mega Drive com
 * suporte a Shadow/Highlight
 * @version 1.0
 * @date 2024-03-28
 */

#include "vdp_sprite.h"
#include "vdp_adapter.h"
#include "vdp_color.h"
#include "vdp_memory.h"
#include "vdp_registers.h"
#include <stdlib.h>
#include <string.h>

// Constantes para renderização de sprites
#define SPRITE_MAX_PER_LINE 20    // Máximo de sprites por linha no hardware
#define SPRITE_MAX_PER_FRAME 80   // Máximo de sprites no frame
#define SPRITE_PIXEL_OVERFLOW 320 // Máximo de pixels de sprite por linha
#define SPRITE_ATTR_SIZE 8        // Tamanho de um atributo de sprite em bytes

// Estrutura para informação de sprite
typedef struct {
  uint16_t y;       // Posição Y
  uint8_t width;    // Largura em tiles (1-4)
  uint8_t height;   // Altura em tiles (1-4)
  uint8_t link;     // Link para próximo sprite
  uint16_t pattern; // Índice do padrão base
  uint8_t palette;  // Paleta a utilizar (0-3)
  bool priority;    // Flag de prioridade
  bool h_flip;      // Flag de flip horizontal
  bool v_flip;      // Flag de flip vertical
  uint16_t x;       // Posição X
} sprite_info_t;

// Estrutura para sprite em linha
typedef struct {
  uint16_t x;       // Posição X na linha
  uint16_t pattern; // Índice de padrão para esta parte do sprite
  uint8_t palette;  // Paleta (0-3)
  bool priority;    // Flag de prioridade
  bool h_flip;      // Flag de flip horizontal
  bool v_flip;      // Flag de flip vertical
} line_sprite_t;

// Buffer temporário para sprites em uma linha
static line_sprite_t line_sprites[SPRITE_MAX_PER_LINE];
static int sprite_count = 0;
static bool sprite_overflow = false;
static bool sprite_collision = false;
static uint8_t tile_line_buffer[8]; // Buffer para linha de um tile

/**
 * @brief Decodifica informações de um sprite a partir de seus atributos
 *
 * @param vdp_ctx Contexto do VDP
 * @param sprite_idx Índice do sprite na tabela de atributos
 * @param info Estrutura para armazenar as informações decodificadas
 * @return true se o sprite é válido, false se não é
 */
static bool decode_sprite_info(vdp_context_t *vdp_ctx, int sprite_idx,
                               sprite_info_t *info) {
  if (sprite_idx >= SPRITE_MAX_PER_FRAME) {
    return false;
  }

  // Calcula endereço do sprite na tabela de atributos
  uint16_t sprite_addr = (vdp_ctx->registers[VDP_REG_SPRITE] << 9) & 0xFC00;
  sprite_addr += sprite_idx * SPRITE_ATTR_SIZE;

  // Lê os atributos do sprite
  uint8_t *sprite_data = &vdp_ctx->vram[sprite_addr];

  // Y position (byte 0-1) - nota: Y = 0 é a primeira linha visível
  info->y = ((sprite_data[0] << 8) | sprite_data[1]) & 0x3FF;

  // Size (byte 2) - bits 0-1: width, bits 2-3: height
  uint8_t size = sprite_data[2] & 0x0F;
  info->width = ((size >> 2) & 0x3) + 1; // 1-4 tiles
  info->height = (size & 0x3) + 1;       // 1-4 tiles

  // Link (byte 3)
  info->link = sprite_data[3] & 0x7F; // 0-127 (0 = fim da lista)

  // Attribute word (byte 4-5)
  uint16_t attr = (sprite_data[4] << 8) | sprite_data[5];
  info->pattern = attr & 0x07FF;         // Bits 0-10: Índice do padrão
  info->h_flip = (attr & 0x0800) != 0;   // Bit 11: Flip H
  info->v_flip = (attr & 0x1000) != 0;   // Bit 12: Flip V
  info->palette = (attr >> 13) & 0x3;    // Bits 13-14: Paleta (0-3)
  info->priority = (attr & 0x8000) != 0; // Bit 15: Prioridade

  // X position (byte 6-7)
  info->x = ((sprite_data[6] << 8) | sprite_data[7]) & 0x1FF;

  // Verificações de validade
  // No hardware real, Y=0 é válido, mas para nosso código usamos convenção
  // 0-based
  if (info->y >= 0x200) {
    // Para Y >= 0x200, o sprite não é renderizado (convenção 0-based)
    return false;
  }

  return true;
}

/**
 * @brief Decodifica uma linha de um tile de sprite
 *
 * @param vdp_ctx Contexto do VDP
 * @param pattern_idx Índice do padrão do tile
 * @param line Linha dentro do tile (0-7)
 * @param flip_h Flag de flip horizontal
 * @param flip_v Flag de flip vertical
 * @param output Buffer para linha decodificada
 */
static void decode_sprite_tile_line(vdp_context_t *vdp_ctx,
                                    uint16_t pattern_idx, int line, bool flip_h,
                                    bool flip_v, uint8_t *output) {
  // Se flip vertical, inverte a linha
  if (flip_v) {
    line = 7 - line;
  }

  // Calcula endereço do tile
  uint32_t addr = (pattern_idx * 32) + (line * 4); // 4 bytes por linha
  const uint8_t *src = &vdp_ctx->vram[addr];

  // Decodifica pixels
  for (int i = 0; i < 4; i++) {
    int byte_idx = flip_h ? (3 - i) : i;
    uint8_t data = src[byte_idx];

    int pixel_idx = i * 2;
    if (flip_h) {
      output[7 - pixel_idx - 0] = (data >> 4) & 0x0F; // Pixel superior
      output[7 - pixel_idx - 1] = data & 0x0F;        // Pixel inferior
    } else {
      output[pixel_idx + 0] = (data >> 4) & 0x0F; // Pixel superior
      output[pixel_idx + 1] = data & 0x0F;        // Pixel inferior
    }
  }
}

/**
 * @brief Renderiza sprites em uma linha
 *
 * Esta função processa os sprites visíveis em uma linha específica e os
 * renderiza no buffer de saída, considerando prioridades e efeitos de
 * Shadow/Highlight.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha de sprites
 */
void vdp_render_sprites_line(vdp_context_t *vdp_ctx, int line,
                             uint16_t *line_buffer) {
  // Limpa buffer de linha
  memset(line_buffer, 0, vdp_ctx->screen_width * sizeof(uint16_t));

  // Reseta contadores e flags para esta linha
  sprite_count = 0;
  sprite_overflow = false;
  sprite_collision = false;

  // Obtém endereço base da tabela de sprites
  uint16_t sprite_table = (vdp_ctx->registers[VDP_REG_SPRITE] << 9) & 0xFC00;
  int screen_width = vdp_ctx->screen_width;

  // Primeiro passo: Encontrar todos os sprites visíveis nesta linha
  int sprite_index = 0;
  int total_sprites = 0;

  // Percorre a lista linkada de sprites, com limite para evitar loops infinitos
  while (sprite_index < SPRITE_MAX_PER_FRAME &&
         total_sprites < SPRITE_MAX_PER_FRAME) {
    sprite_info_t info;
    if (!decode_sprite_info(vdp_ctx, sprite_index, &info)) {
      break;
    }

    total_sprites++;

    // Verifica se o sprite está visível nesta linha
    if (line >= info.y && line < (info.y + info.height * 8)) {
      // Este sprite está visível na linha atual
      if (sprite_count < SPRITE_MAX_PER_LINE) {
        // Calcula qual tile do sprite corresponde a esta linha
        int sprite_line = line - info.y;
        int tile_row = sprite_line / 8;
        int tile_y = sprite_line % 8;

        // Para cada tile na largura do sprite
        for (int tile_col = 0;
             tile_col < info.width && sprite_count < SPRITE_MAX_PER_LINE;
             tile_col++) {
          // Calcula o padrão efetivo com base no tile atual
          uint16_t pattern = info.pattern;

          // Ajuste do padrão baseado na posição do tile no sprite
          if (info.h_flip) {
            pattern += tile_row * 32 + (info.width - 1 - tile_col);
          } else {
            pattern += tile_row * 32 + tile_col;
          }

          // Preenche informação do sprite para esta linha
          line_sprites[sprite_count].x = info.x + tile_col * 8;
          line_sprites[sprite_count].pattern = pattern;
          line_sprites[sprite_count].palette = info.palette;
          line_sprites[sprite_count].priority = info.priority;
          line_sprites[sprite_count].h_flip = info.h_flip;
          line_sprites[sprite_count].v_flip = info.v_flip;

          sprite_count++;
        }
      } else {
        // Overflow: mais de 20 sprites na linha
        sprite_overflow = true;
        vdp_ctx->sprite_overflow = true;
        break;
      }
    }

    // Vai para o próximo sprite na lista
    sprite_index = info.link;

    // Se o link é 0 ou retorna a um sprite já processado, termina
    if (sprite_index == 0 || sprite_index >= SPRITE_MAX_PER_FRAME) {
      break;
    }
  }

  // Atualiza contagem de sprites na linha
  vdp_ctx->sprite_count = sprite_count;

  // Segundo passo: Renderizar sprites na ordem correta (último para primeiro)
  int pixel_count = 0;

  // Processa sprites em ordem inversa (último desenhado tem maior prioridade)
  for (int s = sprite_count - 1; s >= 0; s--) {
    line_sprite_t *sprite = &line_sprites[s];

    // Decodifica linha de tile do sprite
    decode_sprite_tile_line(vdp_ctx, sprite->pattern,
                            sprite->v_flip ? 7 - (line % 8) : line % 8,
                            sprite->h_flip, sprite->v_flip, tile_line_buffer);

    // Renderiza os pixels do sprite
    for (int i = 0; i < 8; i++) {
      int x = sprite->x + i;

      // Verifica se o pixel está dentro da tela
      if (x >= 0 && x < screen_width) {
        uint8_t pixel = tile_line_buffer[i];

        // Processa apenas pixels não transparentes
        if (pixel != 0) {
          // Verifica colisão com outros sprites
          if (line_buffer[x] != 0) {
            // Colisão detectada
            sprite_collision = true;
            vdp_ctx->sprite_collision = true;
          }

          // Cria pixel com metadados usando função do vdp_color.h
          // Nota: sprites especiais podem ser configurados para aplicar shadow
          bool is_shadow_sprite = false;
          bool is_highlight_sprite = false;

          // No hardware real, sprites com determinadas configurações podem
          // aplicar efeitos shadow/highlight em vez de renderizar pixels
          if (IS_SHADOW_HIGHLIGHT_ENABLED(vdp_ctx)) {
            // Verificação para sprite de shadow/highlight
            // Sprites de paleta 3, cor 14 (índice 0x3E) são especiais
            if (sprite->palette == 3 && pixel == 14) {
              is_shadow_sprite = true;
            }
            // Sprites de paleta 3, cor 15 (índice 0x3F) são para highlight
            else if (sprite->palette == 3 && pixel == 15) {
              is_highlight_sprite = true;
            }
          }

          // Armazena no buffer com tipo completo
          line_buffer[x] =
              vdp_create_pixel_type(pixel,                    // Índice de cor
                                    sprite->priority ? 1 : 0, // Prioridade
                                    sprite->palette,          // Paleta
                                    is_shadow_sprite,         // Flag de shadow
                                    is_highlight_sprite // Flag de highlight
              );

          pixel_count++;
        }
      }
    }
  }

  // Verifica overflow de pixels
  if (pixel_count > SPRITE_PIXEL_OVERFLOW) {
    sprite_overflow = true;
    vdp_ctx->sprite_overflow = true;
  }
}

/**
 * @brief Verifica se houve colisão de sprites
 *
 * @param vdp_ctx Contexto do VDP
 * @return true se houve colisão, false caso contrário
 */
bool vdp_sprite_collision_occurred(vdp_context_t *vdp_ctx) {
  return vdp_ctx->sprite_collision;
}

/**
 * @brief Verifica se houve overflow de sprites
 *
 * @param vdp_ctx Contexto do VDP
 * @return true se houve overflow, false caso contrário
 */
bool vdp_sprite_overflow_occurred(vdp_context_t *vdp_ctx) {
  return vdp_ctx->sprite_overflow;
}

/**
 * @brief Reseta flags de sprite
 *
 * @param vdp_ctx Contexto do VDP
 */
void vdp_sprite_reset_flags(vdp_context_t *vdp_ctx) {
  vdp_ctx->sprite_collision = false;
  vdp_ctx->sprite_overflow = false;
}

/**
 * @brief Obtém o número de sprites processados na última linha
 *
 * @param vdp_ctx Contexto do VDP
 * @return Número de sprites
 */
int vdp_sprite_get_count(vdp_context_t *vdp_ctx) {
  return vdp_ctx->sprite_count;
}
