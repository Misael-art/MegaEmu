#ifndef EMU_PPU_INTERFACE_H
#define EMU_PPU_INTERFACE_H

#include "core_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flags de status da PPU
 */
typedef enum {
  EMU_PPU_FLAG_NONE = 0x00,
  EMU_PPU_FLAG_VBLANK = 0x01,
  EMU_PPU_FLAG_SPRITE0_HIT = 0x02,
  EMU_PPU_FLAG_SPRITE_OVERFLOW = 0x04,
  EMU_PPU_FLAG_RENDERING = 0x08
} emu_ppu_flags_t;

/**
 * @brief Estado da PPU
 */
typedef struct {
  uint32_t cycles;       // Ciclos executados
  uint32_t scanline;     // Linha atual
  uint32_t frame;        // Frame atual
  emu_ppu_flags_t flags; // Flags de status
  void *context;         // Contexto específico da PPU
} emu_ppu_state_t;

/**
 * @brief Interface padrão para PPUs
 */
typedef struct {
  void *context; // Contexto da implementação

  // Funções de ciclo de vida
  int (*init)(void *ctx);
  void (*reset)(void *ctx);
  void (*shutdown)(void *ctx);

  // Funções de execução
  int (*execute)(void *ctx, int cycles);
  void (*start_frame)(void *ctx);
  void (*end_frame)(void *ctx);

  // Funções de memória
  uint8_t (*read_register)(void *ctx, uint32_t addr);
  void (*write_register)(void *ctx, uint32_t addr, uint8_t val);
  uint8_t (*read_vram)(void *ctx, uint32_t addr);
  void (*write_vram)(void *ctx, uint32_t addr, uint8_t val);

  // Funções de estado
  void (*get_state)(void *ctx, emu_ppu_state_t *state);
  void (*set_state)(void *ctx, const emu_ppu_state_t *state);

  // Funções de renderização
  void (*render_scanline)(void *ctx);
  void (*update_screen)(void *ctx, void *framebuffer);
  void (*set_pixel)(void *ctx, int x, int y, uint32_t color);

  // Funções de debug
  uint32_t (*get_register)(void *ctx, int reg);
  void (*set_register)(void *ctx, int reg, uint32_t value);
  const char *(*get_register_name)(void *ctx, int reg);
  void (*dump_pattern_table)(void *ctx, int table, void *buffer);
  void (*dump_nametable)(void *ctx, int table, void *buffer);
} emu_ppu_interface_t;

/**
 * @brief Cria uma nova instância da interface PPU
 * @param type Tipo de PPU a ser criada
 * @return Ponteiro para a interface ou NULL em caso de erro
 */
emu_ppu_interface_t *emu_ppu_create(int type);

/**
 * @brief Destrói uma instância da interface PPU
 * @param ppu Ponteiro para a interface
 */
void emu_ppu_destroy(emu_ppu_interface_t *ppu);

#ifdef __cplusplus
}
#endif

#endif // EMU_PPU_INTERFACE_H
