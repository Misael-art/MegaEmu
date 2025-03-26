#include "ppu_2c02.h"
#include <stdlib.h>
#include <string.h>

// Funções estáticas de suporte
static void update_vram_addr(emu_2c02_context_t *ctx) {
  ctx->vram_addr += (ctx->ctrl & EMU_2C02_CTRL_INCREMENT) ? 32 : 1;
}

// Implementações das funções da interface

static int ppu_2c02_init(void *ctx) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu)
    return -1;

  // Inicialização dos registradores
  ppu->ctrl = 0;
  ppu->mask = 0;
  ppu->status = 0;
  ppu->oam_addr = 0;
  ppu->scroll_x = 0;
  ppu->scroll_y = 0;
  ppu->vram_addr = 0;
  ppu->temp_addr = 0;
  ppu->fine_x = 0;
  ppu->write_toggle = 0;

  // Limpa memórias
  memset(ppu->vram, 0, sizeof(ppu->vram));
  memset(ppu->palette, 0, sizeof(ppu->palette));
  memset(ppu->oam, 0, sizeof(ppu->oam));
  memset(ppu->secondary_oam, 0, sizeof(ppu->secondary_oam));
  memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));

  // Estado inicial
  ppu->cycles = 0;
  ppu->scanline = 0;
  ppu->frame = 0;
  ppu->nmi_occurred = 0;
  ppu->sprite_zero_hit = 0;

  return 0;
}

static void ppu_2c02_reset(void *ctx) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu)
    return;

  ppu->ctrl = 0;
  ppu->mask = 0;
  ppu->status &= 0x80; // Mantém apenas o bit 7 (vblank)
  ppu->write_toggle = 0;
  ppu->cycles = 0;
  ppu->scanline = 0;
  ppu->frame = 0;
}

static void ppu_2c02_shutdown(void *ctx) {
  if (ctx) {
    free(ctx);
  }
}

static uint8_t ppu_2c02_read_register(void *ctx, uint32_t addr) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu)
    return 0;

  uint8_t data = 0;

  switch (addr & 7) {
  case EMU_2C02_REG_PPUSTATUS:
    data = ppu->status;
    ppu->status &= ~0x80;  // Limpa o bit de vblank
    ppu->write_toggle = 0; // Reset do write toggle
    break;

  case EMU_2C02_REG_OAMDATA:
    data = ppu->oam[ppu->oam_addr];
    break;

  case EMU_2C02_REG_PPUDATA:
    data = ppu->vram[ppu->vram_addr & 0x3FFF];
    update_vram_addr(ppu);
    break;

  default:
    // Outros registradores são write-only
    break;
  }

  return data;
}

static void ppu_2c02_write_register(void *ctx, uint32_t addr, uint8_t val) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu)
    return;

  switch (addr & 7) {
  case EMU_2C02_REG_PPUCTRL:
    ppu->ctrl = val;
    ppu->temp_addr = (ppu->temp_addr & 0xF3FF) | ((val & 0x03) << 10);
    break;

  case EMU_2C02_REG_PPUMASK:
    ppu->mask = val;
    break;

  case EMU_2C02_REG_OAMADDR:
    ppu->oam_addr = val;
    break;

  case EMU_2C02_REG_OAMDATA:
    ppu->oam[ppu->oam_addr++] = val;
    break;

  case EMU_2C02_REG_PPUSCROLL:
    if (ppu->write_toggle == 0) {
      ppu->scroll_x = val;
      ppu->fine_x = val & 0x07;
      ppu->temp_addr = (ppu->temp_addr & 0xFFE0) | (val >> 3);
    } else {
      ppu->scroll_y = val;
      ppu->temp_addr = (ppu->temp_addr & 0x8FFF) | ((val & 0x07) << 12);
      ppu->temp_addr = (ppu->temp_addr & 0xFC1F) | ((val >> 3) << 5);
    }
    ppu->write_toggle ^= 1;
    break;

  case EMU_2C02_REG_PPUADDR:
    if (ppu->write_toggle == 0) {
      ppu->temp_addr = (ppu->temp_addr & 0x00FF) | ((val & 0x3F) << 8);
    } else {
      ppu->temp_addr = (ppu->temp_addr & 0xFF00) | val;
      ppu->vram_addr = ppu->temp_addr;
    }
    ppu->write_toggle ^= 1;
    break;

  case EMU_2C02_REG_PPUDATA:
    ppu->vram[ppu->vram_addr & 0x3FFF] = val;
    update_vram_addr(ppu);
    break;
  }
}

static int ppu_2c02_execute(void *ctx, int cycles) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu)
    return 0;

  int executed_cycles = 0;
  while (executed_cycles < cycles) {
    // Atualiza ciclos e scanlines
    ppu->cycles++;
    if (ppu->cycles >= 341) {
      ppu->cycles = 0;
      ppu->scanline++;
      if (ppu->scanline >= 262) {
        ppu->scanline = 0;
        ppu->frame++;
      }
    }

    // Verifica vblank
    if (ppu->scanline == 241 && ppu->cycles == 1) {
      ppu->status |= 0x80; // Set vblank flag
      if (ppu->ctrl & EMU_2C02_CTRL_NMI) {
        ppu->nmi_occurred = 1;
      }
    }

    executed_cycles++;
  }

  return executed_cycles;
}

static void ppu_2c02_get_state(void *ctx, emu_ppu_state_t *state) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu || !state)
    return;

  state->cycles = ppu->cycles;
  state->scanline = ppu->scanline;
  state->frame = ppu->frame;
  state->flags = EMU_PPU_FLAG_NONE;
  if (ppu->status & 0x80)
    state->flags |= EMU_PPU_FLAG_VBLANK;
  if (ppu->sprite_zero_hit)
    state->flags |= EMU_PPU_FLAG_SPRITE0_HIT;
  state->context = ppu;
}

static void ppu_2c02_set_state(void *ctx, const emu_ppu_state_t *state) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu || !state)
    return;

  ppu->cycles = state->cycles;
  ppu->scanline = state->scanline;
  ppu->frame = state->frame;
  if (state->flags & EMU_PPU_FLAG_VBLANK)
    ppu->status |= 0x80;
  if (state->flags & EMU_PPU_FLAG_SPRITE0_HIT)
    ppu->sprite_zero_hit = 1;
}

static void ppu_2c02_render_scanline(void *ctx) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu)
    return;

  // TODO: Implementar renderização de scanline
  // Por enquanto, apenas limpa a linha
  if (ppu->scanline < 240) {
    memset(&ppu->framebuffer[ppu->scanline * 256], 0, 256 * sizeof(uint32_t));
  }
}

static void ppu_2c02_update_screen(void *ctx, void *framebuffer) {
  emu_2c02_context_t *ppu = (emu_2c02_context_t *)ctx;
  if (!ppu || !framebuffer)
    return;

  // Copia o framebuffer interno para o framebuffer fornecido
  memcpy(framebuffer, ppu->framebuffer, 256 * 240 * sizeof(uint32_t));
}

// Criação da interface
emu_ppu_interface_t *emu_ppu_2c02_create(void) {
  emu_ppu_interface_t *interface = calloc(1, sizeof(emu_ppu_interface_t));
  if (!interface)
    return NULL;

  emu_2c02_context_t *context = calloc(1, sizeof(emu_2c02_context_t));
  if (!context) {
    free(interface);
    return NULL;
  }

  interface->context = context;
  interface->init = ppu_2c02_init;
  interface->reset = ppu_2c02_reset;
  interface->shutdown = ppu_2c02_shutdown;
  interface->execute = ppu_2c02_execute;
  interface->read_register = ppu_2c02_read_register;
  interface->write_register = ppu_2c02_write_register;
  interface->get_state = ppu_2c02_get_state;
  interface->set_state = ppu_2c02_set_state;
  interface->render_scanline = ppu_2c02_render_scanline;
  interface->update_screen = ppu_2c02_update_screen;

  return interface;
}
