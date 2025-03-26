/**
 * @file psg_adapter.c
 * @brief Implementação do adaptador PSG (SN76489) para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "psg_adapter.h"
#include <stdlib.h>
#include <string.h>

// Tabela de volume (atenuação em dB convertida para amplitude linear)
static const int16_t volume_table[16] = {
    32767, 26028, 20675, 16422, 13045, 10362, 8231, 6568,
    5193,  4125,  3277,  2603,  2067,  1642,  1304, 0};

/**
 * @brief Cria um novo contexto PSG
 */
psg_context_t *psg_create(uint32_t clock_rate, uint32_t sample_rate) {
  psg_context_t *ctx = (psg_context_t *)malloc(sizeof(psg_context_t));
  if (!ctx)
    return NULL;

  // Inicializa o contexto
  memset(ctx, 0, sizeof(psg_context_t));
  ctx->clock_rate = clock_rate;
  ctx->sample_rate = sample_rate;
  ctx->cycles_per_sample = clock_rate / sample_rate;

  // Reset inicial
  psg_reset(ctx);

  return ctx;
}

/**
 * @brief Destrói um contexto PSG
 */
void psg_destroy(psg_context_t *ctx) {
  if (ctx) {
    free(ctx);
  }
}

/**
 * @brief Reseta o PSG para seu estado inicial
 */
void psg_reset(psg_context_t *ctx) {
  if (!ctx)
    return;

  // Reseta canais de tom
  for (int i = 0; i < 3; i++) {
    psg_channel_t *channel = &ctx->channels[i];
    channel->frequency = 0;
    channel->volume = 0x0F; // Mudo
    channel->counter = 0;
    channel->output = false;
  }

  // Reseta canal de ruído
  ctx->noise.mode = 0;
  ctx->noise.shift_rate = 0;
  ctx->noise.volume = 0x0F; // Mudo
  ctx->noise.counter = 0;
  ctx->noise.shift_reg = 0x8000; // Estado inicial do registrador de ruído

  // Reseta estado do latch
  ctx->latch = 0;
  ctx->is_latched = false;
}

/**
 * @brief Processa um byte de dados escrito no PSG
 */
void psg_write(psg_context_t *ctx, uint8_t value) {
  if (!ctx)
    return;

  // Verifica se é um comando de latch
  if (value & 0x80) {
    ctx->latch = value;
    ctx->is_latched = true;

    // Extrai informações do latch
    uint8_t channel = (value >> 5) & 0x03;
    uint8_t type = (value >> 4) & 0x01;
    uint8_t data = value & 0x0F;

    // Processa o comando
    if (channel < 3) { // Canais de tom
      if (type == 0) { // Frequência (bits baixos)
        ctx->channels[channel].frequency =
            (ctx->channels[channel].frequency & 0x3F0) | data;
      } else { // Volume
        ctx->channels[channel].volume = data;
      }
    } else {           // Canal de ruído
      if (type == 0) { // Configuração de ruído
        ctx->noise.mode = (data >> 2) & 0x01;
        ctx->noise.shift_rate = data & 0x03;
      } else { // Volume
        ctx->noise.volume = data;
      }
    }
  }
  // Dados para comando anterior
  else if (ctx->is_latched) {
    uint8_t channel = (ctx->latch >> 5) & 0x03;
    uint8_t type = (ctx->latch >> 4) & 0x01;

    if (type == 0 && channel < 3) { // Frequência (bits altos)
      ctx->channels[channel].frequency =
          (ctx->channels[channel].frequency & 0x0F) | ((value & 0x3F) << 4);
    }
  }
}

/**
 * @brief Atualiza o estado do PSG e gera amostras de áudio
 */
void psg_run(psg_context_t *ctx, uint32_t cycles) {
  if (!ctx)
    return;

  // Atualiza contadores dos canais de tom
  for (int i = 0; i < 3; i++) {
    psg_channel_t *channel = &ctx->channels[i];

    if (channel->frequency > 0) {
      channel->counter += cycles;

      // Período completo
      while (channel->counter >= channel->frequency) {
        channel->counter -= channel->frequency;
        channel->output = !channel->output;
      }
    }
  }

  // Atualiza contador do canal de ruído
  if (ctx->noise.shift_rate > 0) {
    ctx->noise.counter += cycles;

    // Atualiza registrador de deslocamento
    while (ctx->noise.counter >= ctx->noise.shift_rate) {
      ctx->noise.counter -= ctx->noise.shift_rate;

      // Calcula novo bit
      uint16_t new_bit;
      if (ctx->noise.mode == 0) { // Ruído periódico
        new_bit = (ctx->noise.shift_reg & 0x0001) ^
                  ((ctx->noise.shift_reg >> 3) & 0x0001);
      } else { // Ruído branco
        new_bit = ctx->noise.shift_reg & 0x0001;
      }

      // Desloca registrador
      ctx->noise.shift_reg = (ctx->noise.shift_reg >> 1) | (new_bit << 15);
    }
  }
}

/**
 * @brief Gera amostras de áudio para o buffer
 */
void psg_update(psg_context_t *ctx, int16_t *buffer, int length) {
  if (!ctx || !buffer)
    return;

  for (int i = 0; i < length; i++) {
    int32_t sample = 0;

    // Soma saída dos canais de tom
    for (int j = 0; j < 3; j++) {
      psg_channel_t *channel = &ctx->channels[j];
      if (channel->output) {
        sample += volume_table[channel->volume];
      }
    }

    // Soma saída do canal de ruído
    if (ctx->noise.shift_reg & 0x0001) {
      sample += volume_table[ctx->noise.volume];
    }

    // Limita e escreve no buffer
    if (sample > 32767)
      sample = 32767;
    if (sample < -32768)
      sample = -32768;
    buffer[i] = (int16_t)sample;

    // Avança a simulação
    psg_run(ctx, ctx->cycles_per_sample);
  }
}
