/**
 * @file vdp.c
 * @brief Implementação do VDP do Mega Drive
 */

#include "vdp.h"
#include "vdp_dma.h"
#include "vdp_scroll.h"
#include "vdp_sprites.h"
#include <string.h>

// Estado do VDP
static struct {
  // Memórias
  uint8_t vram[VDP_VRAM_SIZE];        // VRAM
  uint16_t cram[VDP_CRAM_SIZE / 2];   // CRAM (cores)
  uint16_t vsram[VDP_VSRAM_SIZE / 2]; // VSRAM (scroll vertical)

  // Registradores
  uint8_t registers[24]; // Registradores de controle
  uint16_t status;       // Registrador de status

  // Estado de acesso
  vdp_access_mode_t access_mode; // Modo de acesso atual
  uint32_t access_addr;          // Endereço de acesso
  uint8_t first_write;           // Flag de primeiro write no controle
  uint16_t command_word;         // Primeira palavra do comando
  uint8_t auto_increment;        // Valor de auto-incremento

  // Padrões de tiles
  vdp_pattern_t patterns[2048]; // 2048 padrões de 8x8

  // Buffer de frame
  uint8_t frame_buffer[320 * 240 * 4]; // RGBA

  // Contadores
  uint16_t v_counter; // Contador vertical (0-261 NTSC, 0-312 PAL)
  uint16_t h_counter; // Contador horizontal (0-341)
  uint8_t in_vblank;  // Flag de V-blank
  uint8_t in_hblank;  // Flag de H-blank

  // Interrupções
  uint8_t hint_counter; // Contador de interrupção H
  uint8_t hint_value;   // Valor para interrupção H
  uint8_t hint_enabled; // Flag de habilitação de interrupção H
  uint8_t vint_enabled; // Flag de habilitação de interrupção V
  uint8_t ext_enabled;  // Flag de habilitação de interrupção externa

  // Modo de vídeo
  uint8_t mode_5;    // Flag de modo 5 (320x224)
  uint8_t interlace; // Modo entrelaçado
  uint8_t pal_mode;  // Flag de modo PAL

} g_vdp_state;

// Constantes de temporização
#define NTSC_LINES 262
#define PAL_LINES 313
#define ACTIVE_LINES 224
#define H_PIXELS 320
#define H_TOTAL 342

// Funções auxiliares internas
static void process_control_port(uint16_t value) {
  if (!g_vdp_state.first_write) {
    // Primeira palavra do comando
    g_vdp_state.command_word = value;
    g_vdp_state.first_write = 1;
  } else {
    // Segunda palavra - processa comando
    uint32_t addr = ((value & 0x3) << 14) | g_vdp_state.command_word;
    uint8_t code = (value >> 14) & 0x3;

    switch (code) {
    case 0: // VRAM read
      g_vdp_state.access_mode = VDP_ACCESS_VRAM_READ;
      g_vdp_state.access_addr = addr & 0xFFFF;
      break;

    case 1: // VRAM write
      g_vdp_state.access_mode = VDP_ACCESS_VRAM_WRITE;
      g_vdp_state.access_addr = addr & 0xFFFF;
      break;

    case 2: // Register write
      vdp_write_register(value & 0x1F, g_vdp_state.command_word & 0xFF);
      break;

    case 3: // CRAM/VSRAM write
      if (value & 0x80) {
        g_vdp_state.access_mode = VDP_ACCESS_VSRAM_WRITE;
        g_vdp_state.access_addr = addr & 0x3F;
      } else {
        g_vdp_state.access_mode = VDP_ACCESS_CRAM_WRITE;
        g_vdp_state.access_addr = addr & 0x7F;
      }
      break;
    }

    g_vdp_state.first_write = 0;
  }
}

static void increment_addr(void) {
  g_vdp_state.access_addr =
      (g_vdp_state.access_addr + g_vdp_state.auto_increment) & 0xFFFF;
}

// Funções de acesso à memória
uint16_t vdp_read_vram(uint32_t addr) {
  addr &= 0xFFFF;
  return (g_vdp_state.vram[addr] << 8) | g_vdp_state.vram[addr ^ 1];
}

void vdp_write_vram(uint32_t addr, uint16_t data) {
  addr &= 0xFFFF;
  g_vdp_state.vram[addr] = data >> 8;
  g_vdp_state.vram[addr ^ 1] = data & 0xFF;
}

uint16_t vdp_read_cram(uint16_t addr) {
  addr &= 0x7F;
  return g_vdp_state.cram[addr / 2];
}

void vdp_write_cram(uint16_t addr, uint16_t data) {
  addr &= 0x7F;
  g_vdp_state.cram[addr / 2] = data & 0xEEE;
}

uint16_t vdp_read_vsram(uint16_t addr) {
  addr &= 0x3F;
  return g_vdp_state.vsram[addr / 2];
}

void vdp_write_vsram(uint16_t addr, uint16_t data) {
  addr &= 0x3F;
  g_vdp_state.vsram[addr / 2] = data & 0x03FF;
}

// Funções de acesso a padrões
uint8_t *vdp_get_pattern_data(uint16_t pattern_index) {
  pattern_index &= 0x7FF;
  return g_vdp_state.patterns[pattern_index].data;
}

void vdp_write_pattern_data(uint16_t pattern_index, const uint8_t *data) {
  pattern_index &= 0x7FF;
  memcpy(g_vdp_state.patterns[pattern_index].data, data, 32);
}

// Funções de renderização
void vdp_write_line_buffer(uint16_t line, const uint8_t *buffer) {
  if (line < 240) {
    memcpy(&g_vdp_state.frame_buffer[line * 320 * 4], buffer, 320 * 4);
  }
}

void vdp_render_pattern(uint16_t pattern_index, int16_t x, int16_t y,
                        uint8_t palette, uint8_t priority, uint8_t flip_h,
                        uint8_t flip_v) {
  if (x < 0 || x >= 320 || y < 0 || y >= 240) {
    return;
  }

  pattern_index &= 0x7FF;
  const uint8_t *pattern = g_vdp_state.patterns[pattern_index].data;
  uint8_t *dest = &g_vdp_state.frame_buffer[(y * 320 + x) * 4];

  for (int py = 0; py < 8; py++) {
    int src_y = flip_v ? 7 - py : py;
    if (y + py >= 240)
      break;

    const uint8_t *src_line = &pattern[src_y * 4];
    uint8_t *dst_line = &dest[py * 320 * 4];

    for (int px = 0; px < 8; px++) {
      if (x + px >= 320)
        break;

      int src_x = flip_h ? 7 - px : px;
      uint8_t pixel = (src_line[src_x / 2] >> ((1 - (src_x & 1)) * 4)) & 0x0F;

      if (pixel) {
        dst_line[px * 4 + 0] = pixel;
        dst_line[px * 4 + 1] = palette;
        dst_line[px * 4 + 2] = priority;
        dst_line[px * 4 + 3] = 1;
      }
    }
  }
}

// Funções de controle
void vdp_init(void) {
  memset(&g_vdp_state, 0, sizeof(g_vdp_state));
  g_vdp_state.status = 0x3400; // V-blank, FIFO empty

  // Configura modo padrão
  g_vdp_state.mode_5 = 1;
  g_vdp_state.pal_mode = 0; // NTSC por padrão

  // Inicializa sistemas relacionados
  emu_vdp_scroll_init();
  emu_vdp_sprites_init();
  emu_vdp_dma_init();
}

void vdp_reset(void) {
  memset(g_vdp_state.registers, 0, sizeof(g_vdp_state.registers));
  g_vdp_state.first_write = 0;
  g_vdp_state.auto_increment = 1;
  g_vdp_state.status = 0x3400;

  // Reseta contadores
  g_vdp_state.v_counter = 0;
  g_vdp_state.h_counter = 0;
  g_vdp_state.in_vblank = 1;
  g_vdp_state.in_hblank = 0;

  // Reseta interrupções
  g_vdp_state.hint_counter = 0;
  g_vdp_state.hint_value = 0;
  g_vdp_state.hint_enabled = 0;
  g_vdp_state.vint_enabled = 0;
  g_vdp_state.ext_enabled = 0;

  // Reseta sistemas relacionados
  emu_vdp_scroll_reset();
  emu_vdp_sprites_reset();
  emu_vdp_dma_reset();
}

void vdp_write_register(uint8_t reg, uint8_t value) {
  if (reg < 24) {
    g_vdp_state.registers[reg] = value;

    // Processa registradores especiais
    switch (reg) {
    case VDP_REG_MODE1:
      g_vdp_state.hint_enabled = (value & 0x10) != 0;
      break;

    case VDP_REG_MODE2:
      g_vdp_state.vint_enabled = (value & 0x20) != 0;
      g_vdp_state.mode_5 = (value & 0x04) != 0;
      break;

    case VDP_REG_HINT:
      g_vdp_state.hint_value = value;
      break;

    case VDP_REG_MODE4:
      g_vdp_state.interlace = (value & 0x06) >> 1;
      break;

    case VDP_REG_AUTO_INC:
      g_vdp_state.auto_increment = value;
      break;
    }
  }
}

uint8_t vdp_read_register(uint8_t reg) {
  return (reg < 24) ? g_vdp_state.registers[reg] : 0;
}

void vdp_write_control(uint16_t value) { process_control_port(value); }

uint16_t vdp_read_status(void) {
  uint16_t status = g_vdp_state.status;
  g_vdp_state.status &= ~0x0200; // Clear sprite collision flag
  return status;
}

void vdp_write_data(uint16_t value) {
  switch (g_vdp_state.access_mode) {
  case VDP_ACCESS_VRAM_WRITE:
    vdp_write_vram(g_vdp_state.access_addr, value);
    break;

  case VDP_ACCESS_CRAM_WRITE:
    vdp_write_cram(g_vdp_state.access_addr, value);
    break;

  case VDP_ACCESS_VSRAM_WRITE:
    vdp_write_vsram(g_vdp_state.access_addr, value);
    break;
  }

  increment_addr();
}

uint16_t vdp_read_data(void) {
  uint16_t value = 0;

  switch (g_vdp_state.access_mode) {
  case VDP_ACCESS_VRAM_READ:
    value = vdp_read_vram(g_vdp_state.access_addr);
    break;

  case VDP_ACCESS_CRAM_READ:
    value = vdp_read_cram(g_vdp_state.access_addr);
    break;

  case VDP_ACCESS_VSRAM_READ:
    value = vdp_read_vsram(g_vdp_state.access_addr);
    break;
  }

  increment_addr();
  return value;
}

// Funções de processamento
static void update_h_interrupt(void) {
  if (g_vdp_state.hint_enabled) {
    if (g_vdp_state.hint_counter == 0) {
      // Gera interrupção H
      g_vdp_state.status |= 0x0004;
      g_vdp_state.hint_counter = g_vdp_state.hint_value;
    } else {
      g_vdp_state.hint_counter--;
    }
  }
}

static void update_v_interrupt(void) {
  if (g_vdp_state.vint_enabled) {
    // Gera interrupção V
    g_vdp_state.status |= 0x0008;
  }
}

void vdp_run_scanline(void) {
  // Atualiza contador vertical
  uint16_t max_lines = g_vdp_state.pal_mode ? PAL_LINES : NTSC_LINES;

  // Processa linha ativa
  if (g_vdp_state.v_counter < ACTIVE_LINES) {
    // Renderiza linha
    emu_vdp_render_line(g_vdp_state.v_counter);

    // Processa DMA se necessário
    emu_vdp_dma_run();

    // Atualiza interrupção H
    update_h_interrupt();
  }
  // Processa V-blank
  else if (g_vdp_state.v_counter == ACTIVE_LINES) {
    g_vdp_state.in_vblank = 1;
    g_vdp_state.status |= 0x0008; // Set V-blank flag
    update_v_interrupt();
  }

  // Atualiza contador vertical
  g_vdp_state.v_counter++;
  if (g_vdp_state.v_counter >= max_lines) {
    g_vdp_state.v_counter = 0;
    g_vdp_state.in_vblank = 0;
    g_vdp_state.status &= ~0x0008; // Clear V-blank flag
  }

  // Atualiza contador horizontal
  g_vdp_state.h_counter = (g_vdp_state.h_counter + 1) % H_TOTAL;

  // Atualiza H-blank
  if (g_vdp_state.h_counter == H_PIXELS) {
    g_vdp_state.in_hblank = 1;
    g_vdp_state.status |= 0x0004; // Set H-blank flag
  } else if (g_vdp_state.h_counter == 0) {
    g_vdp_state.in_hblank = 0;
    g_vdp_state.status &= ~0x0004; // Clear H-blank flag
  }
}

void vdp_end_frame(void) {
  // Processa sprites para o próximo frame
  emu_vdp_sprites_end_frame();

  // Limpa flags de interrupção
  g_vdp_state.status &= ~0x000C; // Clear H-int and V-int flags

  // Reseta contadores
  g_vdp_state.hint_counter = g_vdp_state.hint_value;

  // Atualiza modo de vídeo se necessário
  if (g_vdp_state.registers[VDP_REG_MODE2] & 0x04) {
    g_vdp_state.mode_5 = 1;
  }
}

// Funções de consulta de estado
uint8_t vdp_in_vblank(void) { return g_vdp_state.in_vblank; }

uint8_t vdp_in_hblank(void) { return g_vdp_state.in_hblank; }

uint16_t vdp_get_line(void) { return g_vdp_state.v_counter; }

uint8_t vdp_get_mode(void) {
  uint8_t mode = 0;

  // Verifica bits de modo nos registradores
  if (g_vdp_state.registers[VDP_REG_MODE1] & 0x04)
    mode |= 0x01; // H40
  if (g_vdp_state.registers[VDP_REG_MODE2] & 0x04)
    mode |= 0x02; // Mode 5
  if (g_vdp_state.registers[VDP_REG_MODE4] & 0x01)
    mode |= 0x04; // H80
  if (g_vdp_state.registers[VDP_REG_MODE4] & 0x06)
    mode |= 0x08; // Interlace
  if (g_vdp_state.registers[VDP_REG_MODE4] & 0x08)
    mode |= 0x10; // Shadow/Highlight

  return mode;
}

uint8_t vdp_is_pal(void) { return g_vdp_state.pal_mode; }

uint8_t vdp_is_mode5(void) { return g_vdp_state.mode_5; }

uint8_t vdp_get_interlace(void) { return g_vdp_state.interlace; }
