/**
 * @file audio_adapter.c
 * @brief Implementação do adaptador de áudio para o Mega Drive (YM2612 + PSG)
 * @version 1.0
 * @date 2024-03-21
 */

#include "audio_adapter.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Constantes do YM2612
#define YM2612_CLOCK 7670454
#define YM2612_BUFFER_SIZE 4096

// Constantes do PSG
#define PSG_CLOCK 3579545
#define PSG_VOLUME_TABLE_SIZE 16

// Tabelas pré-calculadas
static const float psg_volume_table[PSG_VOLUME_TABLE_SIZE] = {
    1.0f,   0.794f, 0.631f, 0.501f, 0.398f, 0.316f, 0.251f, 0.200f,
    0.158f, 0.126f, 0.100f, 0.079f, 0.063f, 0.050f, 0.040f, 0.000f};

// Funções auxiliares
static inline float clamp(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

// Funções estáticas do adaptador
static int adapter_init(void *ctx, const emu_audio_config_t *config) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context || !config)
    return -1;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));

  // Configura clocks e taxas
  context->fm_clock = YM2612_CLOCK;
  context->fm_rate = config->sample_rate;
  context->psg_clock = PSG_CLOCK;
  context->psg_rate = config->sample_rate;
  context->sample_rate = config->sample_rate;

  // Aloca buffer de mixagem
  context->mix_buffer_size = YM2612_BUFFER_SIZE;
  context->mix_buffer = calloc(context->mix_buffer_size, sizeof(int16_t));
  if (!context->mix_buffer)
    return -1;

  // Configura volumes iniciais
  context->fm_volume = 1.0f;
  context->psg_volume = 0.5f;

  // Habilita o sistema
  context->enabled = true;

  return 0;
}

static void adapter_reset(void *ctx) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context)
    return;

  // Reseta YM2612
  memset(context->fm_channels, 0, sizeof(context->fm_channels));
  memset(context->fm_registers, 0, sizeof(context->fm_registers));
  context->fm_busy = false;
  context->fm_irq = false;

  // Reseta PSG
  memset(context->psg_channels, 0, sizeof(context->psg_channels));
  memset(context->psg_registers, 0, sizeof(context->psg_registers));
  context->psg_noise_shift = 0x8000;
  context->psg_noise_tap = 0x0009;
  context->psg_noise_type = 0;

  // Reseta mixer
  memset(context->mix_buffer, 0, context->mix_buffer_size * sizeof(int16_t));
  context->mix_position = 0;
  context->samples_played = 0;
}

static void adapter_shutdown(void *ctx) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context)
    return;

  // Libera buffer de mixagem
  free(context->mix_buffer);
  context->mix_buffer = NULL;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));
}

static void adapter_start(void *ctx) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context)
    return;
  context->enabled = true;
}

static void adapter_stop(void *ctx) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context)
    return;
  context->enabled = false;
}

static void adapter_pause(void *ctx) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context)
    return;
  context->enabled = false;
}

static void adapter_resume(void *ctx) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context)
    return;
  context->enabled = true;
}

static int adapter_write_samples(void *ctx, const void *buffer,
                                 uint32_t num_samples) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context || !buffer || !num_samples)
    return 0;

  // TODO: Implementar síntese FM
  // TODO: Implementar síntese PSG
  // TODO: Implementar mixagem

  return num_samples;
}

static int adapter_read_samples(void *ctx, void *buffer, uint32_t num_samples) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context || !buffer || !num_samples)
    return 0;

  int16_t *out = (int16_t *)buffer;
  uint32_t samples_read = 0;

  while (samples_read < num_samples &&
         context->mix_position < context->mix_buffer_size) {
    out[samples_read++] = context->mix_buffer[context->mix_position++];
  }

  if (context->mix_position >= context->mix_buffer_size) {
    context->mix_position = 0;
  }

  return samples_read;
}

static void adapter_clear_buffer(void *ctx) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context)
    return;

  memset(context->mix_buffer, 0, context->mix_buffer_size * sizeof(int16_t));
  context->mix_position = 0;
}

static void adapter_get_state(void *ctx, emu_audio_state_t *state) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context || !state)
    return;

  state->samples_played = context->samples_played;
  state->buffer_level = context->mix_buffer_size - context->mix_position;
  state->flags = 0;
  if (context->enabled)
    state->flags |= EMU_AUDIO_FLAG_PLAYING;
  if (context->mix_position >= context->mix_buffer_size)
    state->flags |= EMU_AUDIO_FLAG_BUFFERING;
  state->context = context;
}

static void adapter_set_state(void *ctx, const emu_audio_state_t *state) {
  megadrive_audio_context_t *context = (megadrive_audio_context_t *)ctx;
  if (!context || !state)
    return;

  context->samples_played = state->samples_played;
  context->enabled = (state->flags & EMU_AUDIO_FLAG_PLAYING) != 0;
}

// Funções públicas
emu_audio_interface_t *megadrive_audio_adapter_create(void) {
  emu_audio_interface_t *interface = calloc(1, sizeof(emu_audio_interface_t));
  megadrive_audio_context_t *context =
      calloc(1, sizeof(megadrive_audio_context_t));

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
  interface->start = adapter_start;
  interface->stop = adapter_stop;
  interface->pause = adapter_pause;
  interface->resume = adapter_resume;
  interface->write_samples = adapter_write_samples;
  interface->read_samples = adapter_read_samples;
  interface->clear_buffer = adapter_clear_buffer;
  interface->get_state = adapter_get_state;
  interface->set_state = adapter_set_state;

  return interface;
}

void megadrive_audio_adapter_destroy(emu_audio_interface_t *audio) {
  if (!audio)
    return;

  if (audio->context) {
    adapter_shutdown(audio->context);
    free(audio->context);
  }

  free(audio);
}

megadrive_audio_context_t *
megadrive_audio_get_context(emu_audio_interface_t *audio) {
  if (!audio || !audio->context)
    return NULL;
  return (megadrive_audio_context_t *)audio->context;
}

int megadrive_audio_set_context(emu_audio_interface_t *audio,
                                const megadrive_audio_context_t *context) {
  if (!audio || !audio->context || !context)
    return -1;

  memcpy(audio->context, context, sizeof(megadrive_audio_context_t));
  return 0;
}

void megadrive_audio_write_ym2612(emu_audio_interface_t *audio, uint8_t port,
                                  uint8_t reg, uint8_t value) {
  megadrive_audio_context_t *context = megadrive_audio_get_context(audio);
  if (!context)
    return;

  uint16_t addr = (port << 8) | reg;
  if (addr >= MD_YM2612_REGISTERS)
    return;

  context->fm_registers[addr] = value;

  // TODO: Implementar processamento de registradores FM
}

void megadrive_audio_write_psg(emu_audio_interface_t *audio, uint8_t value) {
  megadrive_audio_context_t *context = megadrive_audio_get_context(audio);
  if (!context)
    return;

  // TODO: Implementar processamento de registradores PSG
}

void megadrive_audio_set_fm_volume(emu_audio_interface_t *audio, float volume) {
  megadrive_audio_context_t *context = megadrive_audio_get_context(audio);
  if (!context)
    return;

  context->fm_volume = clamp(volume, 0.0f, 1.0f);
}

void megadrive_audio_set_psg_volume(emu_audio_interface_t *audio,
                                    float volume) {
  megadrive_audio_context_t *context = megadrive_audio_get_context(audio);
  if (!context)
    return;

  context->psg_volume = clamp(volume, 0.0f, 1.0f);
}
