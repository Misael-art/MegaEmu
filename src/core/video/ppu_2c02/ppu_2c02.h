#ifndef EMU_PPU_2C02_H
#define EMU_PPU_2C02_H

#include "core/interfaces/ppu_interface.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registradores da PPU 2C02
 */
typedef enum {
  EMU_2C02_REG_PPUCTRL = 0, // $2000 Control
  EMU_2C02_REG_PPUMASK,     // $2001 Mask
  EMU_2C02_REG_PPUSTATUS,   // $2002 Status
  EMU_2C02_REG_OAMADDR,     // $2003 OAM Address
  EMU_2C02_REG_OAMDATA,     // $2004 OAM Data
  EMU_2C02_REG_PPUSCROLL,   // $2005 Scroll
  EMU_2C02_REG_PPUADDR,     // $2006 Address
  EMU_2C02_REG_PPUDATA,     // $2007 Data
  EMU_2C02_REG_COUNT
} emu_2c02_registers_t;

/**
 * @brief Flags do registrador PPUCTRL
 */
typedef enum {
  EMU_2C02_CTRL_NAMETABLE =
      0x03, // Base nametable address (0=2000,1=2400,2=2800,3=2C00)
  EMU_2C02_CTRL_INCREMENT = 0x04,    // VRAM address increment (0=+1,1=+32)
  EMU_2C02_CTRL_SPRITE_TABLE = 0x08, // Sprite pattern table (0=0000,1=1000)
  EMU_2C02_CTRL_BACK_TABLE = 0x10,   // Background pattern table (0=0000,1=1000)
  EMU_2C02_CTRL_SPRITE_SIZE = 0x20,  // Sprite size (0=8x8,1=8x16)
  EMU_2C02_CTRL_MASTER_SLAVE = 0x40, // PPU master/slave select
  EMU_2C02_CTRL_NMI = 0x80           // Generate NMI at VBlank
} emu_2c02_ctrl_flags_t;

/**
 * @brief Flags do registrador PPUMASK
 */
typedef enum {
  EMU_2C02_MASK_GRAYSCALE = 0x01,      // Grayscale display
  EMU_2C02_MASK_SHOW_LEFT_BACK = 0x02, // Show background in leftmost 8 pixels
  EMU_2C02_MASK_SHOW_LEFT_SPR = 0x04,  // Show sprites in leftmost 8 pixels
  EMU_2C02_MASK_SHOW_BACK = 0x08,      // Show background
  EMU_2C02_MASK_SHOW_SPR = 0x10,       // Show sprites
  EMU_2C02_MASK_EMPH_RED = 0x20,       // Emphasize red
  EMU_2C02_MASK_EMPH_GREEN = 0x40,     // Emphasize green
  EMU_2C02_MASK_EMPH_BLUE = 0x80       // Emphasize blue
} emu_2c02_mask_flags_t;

/**
 * @brief Contexto específico da PPU 2C02
 */
typedef struct {
  // Registradores
  uint8_t ctrl;         // $2000
  uint8_t mask;         // $2001
  uint8_t status;       // $2002
  uint8_t oam_addr;     // $2003
  uint8_t scroll_x;     // Fine X scroll
  uint8_t scroll_y;     // Fine Y scroll
  uint16_t vram_addr;   // Current VRAM address
  uint16_t temp_addr;   // Temporary VRAM address
  uint8_t fine_x;       // Fine X scroll (3 bits)
  uint8_t write_toggle; // Address/scroll write toggle

  // Memória
  uint8_t vram[0x4000];      // VRAM (16KB)
  uint8_t palette[32];       // Palette RAM (32 bytes)
  uint8_t oam[256];          // OAM (Object Attribute Memory)
  uint8_t secondary_oam[32]; // Secondary OAM

  // Estado de renderização
  uint32_t cycles;         // Ciclos na scanline atual
  uint32_t scanline;       // Linha atual
  uint32_t frame;          // Frame atual
  uint8_t nmi_occurred;    // Flag de NMI
  uint8_t sprite_zero_hit; // Flag de sprite zero hit

  // Buffer de frame
  uint32_t framebuffer[256 * 240];

  // Interface de memória
  void *memory;
} emu_2c02_context_t;

/**
 * @brief Cria uma nova instância da PPU 2C02
 * @return Ponteiro para a interface da PPU ou NULL em caso de erro
 */
emu_ppu_interface_t *emu_ppu_2c02_create(void);

#ifdef __cplusplus
}
#endif

#endif // EMU_PPU_2C02_H
