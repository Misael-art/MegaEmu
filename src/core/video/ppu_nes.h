#ifndef PPU_NES_H
#define PPU_NES_H

#include <stdbool.h>
#include <stdint.h>

// Constantes do PPU
#define PPU_VRAM_SIZE 0x4000  // 16KB VRAM
#define PPU_OAM_SIZE 0x100    // 256 bytes OAM
#define PPU_PALETTE_SIZE 0x20 // 32 bytes paleta
#define PPU_SCREEN_WIDTH 256
#define PPU_SCREEN_HEIGHT 240

// Estrutura de registradores do PPU
typedef struct {
  // Registradores de controle ($2000-$2007)
  uint8_t ctrl;     // $2000 PPUCTRL
  uint8_t mask;     // $2001 PPUMASK
  uint8_t status;   // $2002 PPUSTATUS
  uint8_t oam_addr; // $2003 OAMADDR
  uint8_t oam_data; // $2004 OAMDATA
  uint8_t scroll;   // $2005 PPUSCROLL
  uint8_t addr;     // $2006 PPUADDR
  uint8_t data;     // $2007 PPUDATA

  // Registradores internos
  uint16_t v;     // Endereço VRAM atual (15 bits)
  uint16_t t;     // Endereço VRAM temporário (15 bits)
  uint8_t x;      // Fine X scroll (3 bits)
  bool w;         // Write toggle
  uint8_t buffer; // Read buffer
} ppu_registers_t;

// Estrutura do PPU
typedef struct {
  ppu_registers_t regs;
  uint8_t *vram;
  uint8_t *oam;
  uint8_t *palette;
  uint32_t *framebuffer;
  bool frame_ready;
  int scanline;
  int cycle;
  bool nmi_occurred;
  bool sprite_zero_hit;
  bool sprite_overflow;
  void *cart_ctx;
  uint8_t (*read_chr)(void *ctx, uint16_t addr);
  void (*write_chr)(void *ctx, uint16_t addr, uint8_t value);
} ppu_nes_t;

// Funções de inicialização e controle
void ppu_nes_init(ppu_nes_t *ppu);
void ppu_nes_reset(ppu_nes_t *ppu);
void ppu_nes_run(ppu_nes_t *ppu, int cycles);

// Funções de acesso aos registradores
uint8_t ppu_nes_read_reg(ppu_nes_t *ppu, uint16_t addr);
void ppu_nes_write_reg(ppu_nes_t *ppu, uint16_t addr, uint8_t value);

// Funções de acesso à memória
uint8_t ppu_nes_read_vram(ppu_nes_t *ppu, uint16_t addr);
void ppu_nes_write_vram(ppu_nes_t *ppu, uint16_t addr, uint8_t value);
uint8_t ppu_nes_read_oam(ppu_nes_t *ppu, uint8_t addr);
void ppu_nes_write_oam(ppu_nes_t *ppu, uint8_t addr, uint8_t value);
uint8_t ppu_nes_read_palette(ppu_nes_t *ppu, uint8_t addr);
void ppu_nes_write_palette(ppu_nes_t *ppu, uint8_t addr, uint8_t value);

// Funções de renderização
void ppu_nes_render_pixel(ppu_nes_t *ppu);
void ppu_nes_evaluate_sprites(ppu_nes_t *ppu);
void ppu_nes_update_registers(ppu_nes_t *ppu);

#endif // PPU_NES_H
