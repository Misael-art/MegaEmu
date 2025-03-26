/**
 * @file vdp_types.h
 * @brief Definições de tipos para o VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-28
 */

#ifndef MEGA_EMU_VDP_TYPES_H
#define MEGA_EMU_VDP_TYPES_H

#include <stdbool.h>
#include <stdint.h>

// Constantes de memória
#define VDP_VRAM_SIZE 0x10000 // 64KB
#define VDP_CRAM_SIZE 0x80    // 64 entradas de cores (128 bytes)
#define VDP_VSRAM_SIZE 0x80   // 40 entradas de scroll (80 bytes)

// Constantes de renderização
#define VDP_SCREEN_WIDTH_H32 256
#define VDP_SCREEN_WIDTH_H40 320
#define VDP_SCREEN_HEIGHT_V28 224
#define VDP_SCREEN_HEIGHT_V30 240

// Modos do VDP
typedef enum {
  VDP_MODE_H32_V28 = 0, // 256x224
  VDP_MODE_H32_V30,     // 256x240
  VDP_MODE_H40_V28,     // 320x224
  VDP_MODE_H40_V30      // 320x240
} vdp_mode_t;

// Estrutura para o contexto do VDP
typedef struct {
  // Memórias
  uint8_t vram[VDP_VRAM_SIZE];        // VRAM
  uint16_t cram[VDP_CRAM_SIZE / 2];   // CRAM (Color RAM)
  uint16_t vsram[VDP_VSRAM_SIZE / 2]; // VSRAM (Vertical Scroll RAM)
  uint8_t registers[0x20];            // Registradores (0x00-0x1F)

  // Estado de vídeo
  vdp_mode_t mode;        // Modo atual (H32/H40, V28/V30)
  int screen_width;       // Largura de tela (256 ou 320)
  int screen_height;      // Altura de tela (224 ou 240)
  int framebuffer_stride; // Stride do framebuffer
  uint32_t *framebuffer;  // Ponteiro para framebuffer RGB

  // Estado de planos
  int plane_a_width;   // Largura do plano A em tiles (32, 64)
  int plane_a_height;  // Altura do plano A em tiles (32, 64)
  int plane_b_width;   // Largura do plano B em tiles (32, 64)
  int plane_b_height;  // Altura do plano B em tiles (32, 64)
  int window_width;    // Largura da janela em tiles (32)
  int window_height;   // Altura da janela em tiles (32)
  bool window_enabled; // Flag indicando se a janela está habilitada

  // Estado de sprites
  int sprite_count;      // Número de sprites na linha atual
  bool sprite_overflow;  // Flag de overflow de sprites
  bool sprite_collision; // Flag de colisão de sprites

  // Controle de interrupções
  bool vint_pending; // Flag de interrupção vertical pendente
  bool hint_pending; // Flag de interrupção horizontal pendente
  int hint_counter;  // Contador para interrupção horizontal

  // Estado de DMA
  bool dma_active;     // Flag indicando se DMA está ativo
  uint32_t dma_source; // Endereço fonte para DMA
  uint16_t dma_length; // Comprimento de dados para DMA
  uint8_t dma_type;    // Tipo de transferência DMA

  // Callbacks
  void (*line_rendered_callback)(
      void *, int);    // Callback chamado após renderização de linha
  void *callback_data; // Dados para callbacks

} vdp_context_t;

#endif // MEGA_EMU_VDP_TYPES_H
