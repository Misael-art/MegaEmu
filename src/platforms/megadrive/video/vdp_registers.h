#ifndef MEGA_EMU_VDP_REGISTERS_H
#define MEGA_EMU_VDP_REGISTERS_H

// Números dos registros VDP
#define VDP_REG_MODE1 0x00
#define VDP_REG_MODE2 0x01
#define VDP_REG_PLANE_A 0x02
#define VDP_REG_WINDOW 0x03
#define VDP_REG_PLANE_B 0x04
#define VDP_REG_SPRITE 0x05
#define VDP_REG_BGCOLOR 0x07
#define VDP_REG_HSCROLL 0x0D
#define VDP_REG_MODE4 0x0C
#define VDP_REG_AUTOINCREMENT 0x0F
#define VDP_REG_SCROLL_SIZE 0x10

// Máscaras de bits para o registro de modo 1
#define VDP_REG1_HSCROLL_INHIBIT 0x01
#define VDP_REG1_VSCROLL_INHIBIT 0x02
#define VDP_REG1_HBLANK_ENABLE 0x10
#define VDP_REG1_DISPLAY_ENABLE 0x40

// Máscaras de bits para o registro de modo 2
#define VDP_REG2_DISPLAY_ENABLE 0x40
#define VDP_REG2_VBLANK_ENABLE 0x20
#define VDP_REG2_DMA_ENABLE 0x10
#define VDP_REG2_PAL_MODE 0x08

// Máscaras de bits para o registro de modo 4
#define VDP_REG4_H40 0x01
#define VDP_REG4_INTERLACE 0x02
#define VDP_REG4_SHADOW_HIGHLIGHT_ENABLE 0x08
#define VDP_REG4_H40_CELL 0x81

// Máscaras para controle de tamanho dos planos
#define VDP_SCROLL_SIZE_H32 0x00
#define VDP_SCROLL_SIZE_H64 0x01
#define VDP_SCROLL_SIZE_V32 0x00
#define VDP_SCROLL_SIZE_V64 0x10

// Macros para verificação de modos
#define IS_H40_MODE(ctx) ((ctx->registers[VDP_REG_MODE4] & VDP_REG4_H40) != 0)
#define IS_INTERLACE_MODE(ctx)                                                 \
  ((ctx->registers[VDP_REG_MODE4] & VDP_REG4_INTERLACE) != 0)
#define IS_SHADOW_HIGHLIGHT_ENABLED(ctx)                                       \
  ((ctx->registers[VDP_REG_MODE4] & VDP_REG4_SHADOW_HIGHLIGHT_ENABLE) != 0)
#define IS_PAL_MODE(ctx)                                                       \
  ((ctx->registers[VDP_REG_MODE2] & VDP_REG2_PAL_MODE) != 0)

// Endereços de memória no VDP (deslocamentos)
#define VDP_VRAM_SIZE 0x10000 // 64KB
#define VDP_CRAM_SIZE 0x80    // 64 entradas de cores (128 bytes)
#define VDP_VSRAM_SIZE 0x80   // 40 entradas de scroll (128 bytes)

// Tipos de acesso ao VDP
#define VDP_ACCESS_VRAM_READ 0x00
#define VDP_ACCESS_VRAM_WRITE 0x01
#define VDP_ACCESS_CRAM_WRITE 0x03
#define VDP_ACCESS_VSRAM_READ 0x04
#define VDP_ACCESS_VSRAM_WRITE 0x05
#define VDP_ACCESS_CRAM_READ 0x08

// Máscaras para cálculo de cores
#define VDP_CRAM_COLOR_MASK 0x0EEE // Máscara de cor válida (12 bits)

#endif // MEGA_EMU_VDP_REGISTERS_H
