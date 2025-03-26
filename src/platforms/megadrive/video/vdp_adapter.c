/**
 * @file vdp_adapter.c
 * @brief Implementação do adaptador de vídeo VDP para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "vdp_adapter.h"
#include <stdlib.h>
#include <string.h>

// Constantes do VDP
#define SCREEN_WIDTH_H32 256
#define SCREEN_WIDTH_H40 320
#define SCREEN_HEIGHT_V28 224
#define SCREEN_HEIGHT_V30 240

#define SPRITE_ATTRIBUTE_TABLE_MASK 0x7F
#define SPRITE_SIZE_MAX 4

// Macros úteis
#define IS_H40_MODE(ctx) ((ctx)->regs[0x0C] & 0x81)
#define IS_V30_MODE(ctx) ((ctx)->regs[0x01] & 0x08)

// Funções estáticas do adaptador
static int adapter_init(void *ctx, const emu_video_config_t *config) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context || !config)
    return -1;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));

  // Configura modo inicial
  context->mode = MD_VDP_MODE_H32_V28;
  context->access_mode = MD_VDP_ACCESS_VRAM_READ;
  context->first_byte = true;

  // Inicializa registradores com valores padrão
  context->regs[0x00] = 0x04; // Modo normal
  context->regs[0x01] = 0x04; // Modo normal, display desligado
  context->regs[0x02] = 0x30; // Plano A em 0xC000
  context->regs[0x03] = 0x3C; // Window em 0xF000
  context->regs[0x04] = 0x07; // Plano B em 0xE000
  context->regs[0x05] = 0x6C; // Sprites em 0xD800
  context->regs[0x0A] = 0xFF; // Valor de interrupção H
  context->regs[0x0B] = 0x00; // Modo normal
  context->regs[0x0C] = 0x81; // H40, shadow/highlight off
  context->regs[0x0D] = 0x3F; // HScroll em 0xFC00
  context->regs[0x0F] = 0x02; // Auto-increment 2

  return 0;
}

static void adapter_reset(void *ctx) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context)
    return;

  // Preserva memórias mas reseta estado
  context->status = 0x3400; // V-Blank, PAL/NTSC
  context->address = 0;
  context->code = 0;
  context->first_byte = true;
  context->hcounter = 0;
  context->vcounter = 0;
  context->frame_count = 0;
  context->dma_enabled = false;
  context->sprite_count = 0;
  context->sprite_collision = false;
  context->sprite_overflow = false;
  context->vint_pending = false;
  context->hint_pending = false;
  context->hint_counter = 0;
}

static void adapter_shutdown(void *ctx) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context)
    return;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));
}

static void adapter_begin_frame(void *ctx) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context)
    return;

  // Reseta contadores para novo frame
  context->vcounter = 0;
  context->sprite_count = 0;
  context->sprite_collision = false;
  context->sprite_overflow = false;

  // Processa sprites para o próximo frame
  // TODO: Implementar processamento de sprites
}

static void adapter_end_frame(void *ctx) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context)
    return;

  // Incrementa contador de frames
  context->frame_count++;

  // Atualiza status
  context->status |= 0x0008; // VBLANK

  // Gera interrupção vertical se habilitada
  if (context->regs[0x01] & 0x20) {
    context->vint_pending = true;
    if (context->vint_callback) {
      context->vint_callback(context->callback_data);
    }
  }
}

static void adapter_render_line(void *ctx, int line) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context)
    return;

  context->vcounter = line;

  // Processa interrupção horizontal
  if (--context->hint_counter <= 0) {
    context->hint_counter = context->hint_value;
    if (context->regs[0x00] & 0x10) {
      context->hint_pending = true;
      if (context->hint_callback) {
        context->hint_callback(context->callback_data);
      }
    }
  }

  // Renderiza a linha atual
  if (line >= 0 && line < (IS_V30_MODE(context) ? 240 : 224)) {
    uint8_t *line_buffer =
        &context->frame_buffer[line * (IS_H40_MODE(context) ? 320 : 256)];
    vdp_render_line(context, line, line_buffer);
  }
}

static void adapter_update(void *ctx, int cycles) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context)
    return;

  // Atualiza contadores
  context->hcounter =
      (context->hcounter + cycles) % (IS_H40_MODE(context) ? 420 : 342);

  // Processa DMA se ativo
  if (context->dma_enabled && context->dma_length > 0) {
    vdp_dma_execute(context);
  }
}

static void adapter_write_register(void *ctx, uint16_t reg, uint8_t val) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context || reg >= MD_VDP_REG_COUNT)
    return;

  vdp_write_register(context, reg, val);
}

static uint8_t adapter_read_register(void *ctx, uint16_t reg) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context || reg >= MD_VDP_REG_COUNT)
    return 0xFF;

  return vdp_read_register(context, reg);
}

static void adapter_write_vram(void *ctx, uint32_t addr, uint8_t val) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context || addr >= MD_VDP_VRAM_SIZE)
    return;

  context->vram[addr] = val;
}

static uint8_t adapter_read_vram(void *ctx, uint32_t addr) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context || addr >= MD_VDP_VRAM_SIZE)
    return 0xFF;

  return context->vram[addr];
}

static void adapter_get_state(void *ctx, emu_video_state_t *state) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context || !state)
    return;

  state->line = context->vcounter;
  state->cycle = context->hcounter;
  state->flags = 0;
  if (context->vint_pending)
    state->flags |= EMU_VIDEO_FLAG_VBLANK;
  if (context->hint_pending)
    state->flags |= EMU_VIDEO_FLAG_HBLANK;
  if (context->sprite_overflow)
    state->flags |= EMU_VIDEO_FLAG_SPRITE_OVF;
  if (context->sprite_collision)
    state->flags |= EMU_VIDEO_FLAG_COLLISION;
  state->context = context;
}

static void adapter_set_state(void *ctx, const emu_video_state_t *state) {
  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)ctx;
  if (!context || !state)
    return;

  context->vcounter = state->line;
  context->hcounter = state->cycle;
  context->vint_pending = (state->flags & EMU_VIDEO_FLAG_VBLANK) != 0;
  context->hint_pending = (state->flags & EMU_VIDEO_FLAG_HBLANK) != 0;
  context->sprite_overflow = (state->flags & EMU_VIDEO_FLAG_SPRITE_OVF) != 0;
  context->sprite_collision = (state->flags & EMU_VIDEO_FLAG_COLLISION) != 0;
}

// Funções públicas
emu_video_interface_t *megadrive_vdp_adapter_create(void) {
  emu_video_interface_t *interface = calloc(1, sizeof(emu_video_interface_t));
  megadrive_vdp_context_t *context = calloc(1, sizeof(megadrive_vdp_context_t));

  if (!interface || !context) {
    free(interface);
    free(context);
    return NULL;
  }

  // Configura a interface
  interface->context = context;
  interface->init = adapter_init;
  interface->reset = adapter_reset;
  interface->shutdown = adapter_shutdown;
  interface->begin_frame = adapter_begin_frame;
  interface->end_frame = adapter_end_frame;
  interface->render_line = adapter_render_line;
  interface->update = adapter_update;
  interface->write_register = adapter_write_register;
  interface->read_register = adapter_read_register;
  interface->write_vram = adapter_write_vram;
  interface->read_vram = adapter_read_vram;
  interface->get_state = adapter_get_state;
  interface->set_state = adapter_set_state;

  return interface;
}

void megadrive_vdp_adapter_destroy(emu_video_interface_t *video) {
  if (!video)
    return;

  if (video->context) {
    adapter_shutdown(video->context);
    free(video->context);
  }

  free(video);
}

megadrive_vdp_context_t *
megadrive_vdp_get_context(emu_video_interface_t *video) {
  if (!video || !video->context)
    return NULL;
  return (megadrive_vdp_context_t *)video->context;
}

int megadrive_vdp_set_context(emu_video_interface_t *video,
                              const megadrive_vdp_context_t *context) {
  if (!video || !video->context || !context)
    return -1;

  memcpy(video->context, context, sizeof(megadrive_vdp_context_t));
  return 0;
}

void megadrive_vdp_set_interrupt_callbacks(emu_video_interface_t *video,
                                           void (*vint_cb)(void *user_data),
                                           void (*hint_cb)(void *user_data),
                                           void *user_data) {
  if (!video || !video->context)
    return;

  megadrive_vdp_context_t *context = (megadrive_vdp_context_t *)video->context;
  context->vint_callback = vint_cb;
  context->hint_callback = hint_cb;
  context->callback_data = user_data;
}
