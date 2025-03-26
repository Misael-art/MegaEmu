#include "ppu_2c02_adapter.h"
#include <stdlib.h>
#include <string.h>

// Funções de adaptação
static int32_t adapter_init(void *ctx, const ppu_config_t *config) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_config_t nes_config = {
      .read_mem = (nes_ppu_read_func_t)config->read_mem,
      .write_mem = (nes_ppu_write_func_t)config->write_mem,
      .context = config->context,
      .log_level = config->log_level};

  if (!nes_ppu_init(&nes_config)) {
    return PPU_ERROR_NONE;
  }
  return -1;
}

static void adapter_shutdown(void *ctx) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_shutdown(ppu);
}

static void adapter_reset(void *ctx) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_reset(ppu);
}

static int32_t adapter_execute(void *ctx, int32_t cycles) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  return nes_ppu_execute(ppu, cycles);
}

static void adapter_get_state(void *ctx, ppu_state_t *state) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_state_t nes_state;
  nes_ppu_get_state(ppu, &nes_state);

  state->scanline = nes_state.scanline;
  state->cycle = nes_state.dot;
  state->frame = nes_state.frame;

  // Converte flags específicas do NES para flags genéricas
  state->flags = 0;
  if (nes_state.in_vblank)
    state->flags |= PPU_FLAG_VBLANK;
  if (nes_state.sprite0_hit)
    state->flags |= PPU_FLAG_SPRITE0_HIT;
  if (nes_state.rendering_enabled)
    state->flags |= PPU_FLAG_RENDERING;
  if (nes_state.nmi_enabled)
    state->flags |= PPU_FLAG_NMI_ENABLED;
}

static void adapter_set_state(void *ctx, const ppu_state_t *state) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_state_t nes_state = {
      .scanline = state->scanline,
      .dot = state->cycle,
      .frame = state->frame,
      .in_vblank = (state->flags & PPU_FLAG_VBLANK) != 0,
      .sprite0_hit = (state->flags & PPU_FLAG_SPRITE0_HIT) != 0,
      .rendering_enabled = (state->flags & PPU_FLAG_RENDERING) != 0,
      .nmi_enabled = (state->flags & PPU_FLAG_NMI_ENABLED) != 0};

  nes_ppu_set_state(ppu, &nes_state);
}

static uint8_t adapter_read_register(void *ctx, uint32_t reg) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  return nes_ppu_read_register(ppu, reg);
}

static void adapter_write_register(void *ctx, uint32_t reg, uint8_t value) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_write_register(ppu, reg, value);
}

static uint8_t adapter_read_vram(void *ctx, uint32_t addr) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  return nes_ppu_read_vram(ppu, addr);
}

static void adapter_write_vram(void *ctx, uint32_t addr, uint8_t value) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_write_vram(ppu, addr, value);
}

static uint8_t adapter_read_palette(void *ctx, uint32_t addr) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  return nes_ppu_read_palette(ppu, addr);
}

static void adapter_write_palette(void *ctx, uint32_t addr, uint8_t value) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_write_palette(ppu, addr, value);
}

static uint8_t adapter_read_oam(void *ctx, uint32_t addr) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  return nes_ppu_read_oam(ppu, addr);
}

static void adapter_write_oam(void *ctx, uint32_t addr, uint8_t value) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_write_oam(ppu, addr, value);
}

static void adapter_dma_write(void *ctx, const uint8_t *data) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_dma_write(ppu, data);
}

static void adapter_end_frame(void *ctx) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  nes_ppu_end_frame(ppu);
}

static const uint32_t *adapter_get_frame_buffer(void *ctx) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  return nes_ppu_get_frame_buffer(ppu);
}

static int32_t adapter_dump_state(void *ctx, char *buffer,
                                  int32_t buffer_size) {
  nes_ppu_t *ppu = (nes_ppu_t *)ctx;
  return nes_ppu_dump_state(ppu, buffer, buffer_size);
}

ppu_interface_t *ppu_2c02_create_interface(void) {
  ppu_interface_t *interface =
      (ppu_interface_t *)malloc(sizeof(ppu_interface_t));
  if (!interface) {
    return NULL;
  }

  // Aloca contexto para a PPU
  interface->context = malloc(sizeof(nes_ppu_t));
  if (!interface->context) {
    free(interface);
    return NULL;
  }

  // Inicializa funções da interface
  interface->init = adapter_init;
  interface->shutdown = adapter_shutdown;
  interface->reset = adapter_reset;
  interface->execute = adapter_execute;
  interface->get_state = adapter_get_state;
  interface->set_state = adapter_set_state;
  interface->read_register = adapter_read_register;
  interface->write_register = adapter_write_register;
  interface->read_vram = adapter_read_vram;
  interface->write_vram = adapter_write_vram;
  interface->read_palette = adapter_read_palette;
  interface->write_palette = adapter_write_palette;
  interface->read_oam = adapter_read_oam;
  interface->write_oam = adapter_write_oam;
  interface->dma_write = adapter_dma_write;
  interface->end_frame = adapter_end_frame;
  interface->get_frame_buffer = adapter_get_frame_buffer;
  interface->dump_state = adapter_dump_state;

  return interface;
}
