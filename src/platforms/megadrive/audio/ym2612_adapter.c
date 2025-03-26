/**
 * @file ym2612_adapter.c
 * @brief Implementação do adaptador YM2612
 * @version 1.0
 * @date 2024-03-21
 */

#include "ym2612_adapter.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Tabelas de lookup para síntese FM
static const uint8_t multiple_table[16] = {1,  2,  4,  6,  8,  10, 12, 14,
                                           16, 18, 20, 22, 24, 26, 28, 30};

static const int8_t detune_table[32] = {
    0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
    -4, -4, -4, -4, -3, -3, -3, -3, -2, -2, -2, -2, -1, -1, -1, -1};

// Tabela de atenuação para envelope ADSR
static const uint16_t eg_rate_table[64] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

// Tabela de algoritmos FM (conexões entre operadores)
static const uint8_t algorithm_table[8][4] = {
    {0, 1, 1, 1}, // Algoritmo 0: Op1 -> Op2 -> Op3 -> Op4
    {0, 0, 1, 1}, // Algoritmo 1: (Op1 + Op2) -> Op3 -> Op4
    {0, 0, 0, 1}, // Algoritmo 2: (Op1 + Op2 + Op3) -> Op4
    {0, 1, 2, 1}, // Algoritmo 3: (Op1 -> Op2) + (Op3 -> Op4)
    {0, 1, 2, 2}, // Algoritmo 4: (Op1 -> Op2) + Op3 + Op4
    {0, 1, 1, 0}, // Algoritmo 5: (Op1 -> Op2) + (Op3 -> Op4)
    {0, 1, 0, 0}, // Algoritmo 6: (Op1 -> Op2) + Op3 + Op4
    {0, 0, 0, 0}  // Algoritmo 7: Op1 + Op2 + Op3 + Op4
};

// Tabelas de lookup otimizadas
static int16_t sine_table[4096]; // Tabela de seno com maior resolução
static int16_t exp_table[4096];  // Tabela exponencial para envelope
static int32_t phase_scale[8];   // Fatores de escala de fase por bloco
static int16_t lfo_table[256];   // Tabela para LFO

/**
 * @brief Inicializa todas as tabelas de lookup
 */
static void init_lookup_tables(void) {
  // Inicializa tabela de seno com 4096 entradas para maior precisão
  for (int i = 0; i < 4096; i++) {
    sine_table[i] = (int16_t)(sin(2.0 * M_PI * i / 4096.0) * 32767.0);
  }

  // Inicializa tabela exponencial para envelope ADSR
  for (int i = 0; i < 4096; i++) {
    exp_table[i] = (int16_t)(pow(2.0, -i / 256.0) * 32767.0);
  }

  // Inicializa fatores de escala de fase
  for (int i = 0; i < 8; i++) {
    phase_scale[i] = 1 << i;
  }

  // Inicializa tabela LFO
  for (int i = 0; i < 256; i++) {
    lfo_table[i] = (int16_t)(sin(2.0 * M_PI * i / 256.0) * 32767.0);
  }
}

/**
 * @brief Calcula a fase do operador FM com otimizações
 */
static uint32_t calculate_phase(ym2612_operator_t *op, uint32_t freq,
                                uint8_t block) {
  // Usa lookup tables e operações bit-a-bit
  uint32_t multiple = multiple_table[op->multiple & 0x0F];
  int32_t detune = detune_table[op->detune & 0x1F];

  // Aplica detune usando shift
  uint32_t base_freq = (freq + (detune << (block + 1))) & 0xFFFFF;

  // Aplica multiplicador e block usando lookup
  return (base_freq * multiple * phase_scale[block & 0x07]) & 0xFFFFF;
}

/**
 * @brief Calcula o nível de saída do operador FM com otimizações
 */
static int16_t calculate_operator_output(ym2612_operator_t *op, uint32_t phase,
                                         uint32_t modulation) {
  // Aplica modulação à fase
  phase = (phase + modulation) & 0xFFF;

  // Usa lookup tables otimizadas
  int16_t sine = sine_table[phase];
  int16_t env = exp_table[op->envelope_level & 0xFFF];

  // Aplica envelope usando multiplicação de ponto fixo
  return (sine * env) >> 15;
}

/**
 * @brief Atualiza o envelope ADSR do operador com otimizações
 */
static void update_envelope(ym2612_operator_t *op) {
  if (!op->key_on) {
    // Release phase otimizada
    if (op->envelope_level > 0) {
      op->envelope_level -= eg_rate_table[op->release_rate] << 2;
      if (op->envelope_level < 0)
        op->envelope_level = 0;
    }
    return;
  }

  // Usa lookup tables e shifts para cálculos
  switch (op->envelope_state) {
  case ATTACK:
    op->envelope_level += eg_rate_table[op->attack_rate] << 2;
    if (op->envelope_level >= 0xFFF) {
      op->envelope_level = 0xFFF;
      op->envelope_state = DECAY;
    }
    break;

  case DECAY:
    op->envelope_level -= eg_rate_table[op->decay_rate];
    if (op->envelope_level <= (op->sustain_level << 7)) {
      op->envelope_level = op->sustain_level << 7;
      op->envelope_state = SUSTAIN;
    }
    break;

  case SUSTAIN:
    if (op->envelope_level > (op->sustain_level << 7)) {
      op->envelope_level -= eg_rate_table[op->sustain_rate];
      if (op->envelope_level < (op->sustain_level << 7)) {
        op->envelope_level = op->sustain_level << 7;
      }
    }
    break;

  case RELEASE:
    op->envelope_level -= eg_rate_table[op->release_rate] << 1;
    if (op->envelope_level < 0)
      op->envelope_level = 0;
    break;
  }
}

/**
 * @brief Gera amostras de áudio para um canal FM com otimizações
 */
static void generate_channel_samples(ym2612_context_t *ctx,
                                     ym2612_channel_t *channel, int16_t *buffer,
                                     uint32_t num_samples) {
  uint8_t algorithm = channel->algorithm & 0x07;
  uint8_t feedback = channel->feedback & 0x07;

  // Pré-calcula valores comuns
  int32_t feedback_shift = feedback > 0 ? 9 - feedback : 0;

  // Alinha buffer para SIMD
  int16_t *aligned_buffer = (int16_t *)__builtin_assume_aligned(buffer, 16);

  for (uint32_t i = 0; i < num_samples; i++) {
    int16_t op_output[4] = {0, 0, 0, 0};
    int16_t feedback_mod = 0;

    // Calcula feedback do operador 1
    if (feedback > 0) {
      feedback_mod =
          (channel->feedback_buffer[0] + channel->feedback_buffer[1]) >>
          feedback_shift;
      channel->feedback_buffer[1] = channel->feedback_buffer[0];
    }

// Atualiza envelopes - pode ser paralelizado
#pragma omp parallel for
    for (int op = 0; op < 4; op++) {
      update_envelope(&channel->operators[op]);
    }

    // Calcula saída de cada operador
    for (int op = 0; op < 4; op++) {
      uint32_t phase = calculate_phase(&channel->operators[op],
                                       channel->frequency, channel->block);
      int16_t modulation = 0;

      // Aplica modulação usando lookup da tabela de algoritmos
      if (op > 0) {
        modulation = op_output[algorithm_table[algorithm][op]];
      } else {
        modulation = feedback_mod;
      }

      op_output[op] =
          calculate_operator_output(&channel->operators[op], phase, modulation);
    }

    // Armazena valor para feedback
    if (feedback > 0) {
      channel->feedback_buffer[0] = op_output[0];
    }

// Mixa saída dos operadores usando SIMD quando possível
#ifdef __SSE2__
    __m128i output_vec = _mm_setzero_si128();

    switch (algorithm) {
    case 0:
    case 1:
    case 2:
      output_vec = _mm_set1_epi16(op_output[3]);
      break;
    case 3:
      output_vec = _mm_add_epi16(_mm_set1_epi16(op_output[1]),
                                 _mm_set1_epi16(op_output[3]));
      break;
    case 4:
    case 6:
      output_vec = _mm_add_epi16(_mm_add_epi16(_mm_set1_epi16(op_output[1]),
                                               _mm_set1_epi16(op_output[2])),
                                 _mm_set1_epi16(op_output[3]));
      break;
    case 5:
      output_vec = _mm_add_epi16(_mm_set1_epi16(op_output[1]),
                                 _mm_set1_epi16(op_output[3]));
      break;
    case 7:
      output_vec = _mm_add_epi16(
          _mm_add_epi16(_mm_add_epi16(_mm_set1_epi16(op_output[0]),
                                      _mm_set1_epi16(op_output[1])),
                        _mm_set1_epi16(op_output[2])),
          _mm_set1_epi16(op_output[3]));
      break;
    }

    // Aplica pan L/R
    __m128i pan_mask = _mm_set_epi16(
        channel->right_enable ? -1 : 0, channel->left_enable ? -1 : 0,
        channel->right_enable ? -1 : 0, channel->left_enable ? -1 : 0,
        channel->right_enable ? -1 : 0, channel->left_enable ? -1 : 0,
        channel->right_enable ? -1 : 0, channel->left_enable ? -1 : 0);
    output_vec = _mm_and_si128(output_vec, pan_mask);

    // Escreve no buffer
    _mm_store_si128((__m128i *)&aligned_buffer[i * 2], output_vec);
#else
    // Versão escalar otimizada
    int32_t output = 0;
    switch (algorithm) {
    case 0:
    case 1:
    case 2:
      output = op_output[3];
      break;
    case 3:
      output = op_output[1] + op_output[3];
      break;
    case 4:
    case 6:
      output = op_output[1] + op_output[2] + op_output[3];
      break;
    case 5:
      output = op_output[1] + op_output[3];
      break;
    case 7:
      output = op_output[0] + op_output[1] + op_output[2] + op_output[3];
      break;
    }

    // Aplica pan e escreve no buffer
    if (channel->left_enable)
      aligned_buffer[i * 2] = output;
    if (channel->right_enable)
      aligned_buffer[i * 2 + 1] = output;
#endif
  }
}

/**
 * @brief Cria um novo contexto YM2612
 */
ym2612_context_t *ym2612_create(void) {
  ym2612_context_t *ctx = (ym2612_context_t *)malloc(sizeof(ym2612_context_t));
  if (!ctx)
    return NULL;

  memset(ctx, 0, sizeof(ym2612_context_t));
  init_lookup_tables();
  ym2612_reset(ctx);

  return ctx;
}

/**
 * @brief Destrói um contexto YM2612
 */
void ym2612_destroy(ym2612_context_t *ctx) {
  if (ctx) {
    free(ctx);
  }
}

/**
 * @brief Reseta o YM2612 para seu estado inicial
 */
void ym2612_reset(ym2612_context_t *ctx) {
  if (!ctx)
    return;

  memset(ctx->registers, 0, sizeof(ctx->registers));

  for (int i = 0; i < YM2612_NUM_CHANNELS; i++) {
    ctx->channels[i].algorithm = 0;
    ctx->channels[i].feedback = 0;
    ctx->channels[i].frequency = 0;
    ctx->channels[i].block = 0;
    ctx->channels[i].left_enable = true;
    ctx->channels[i].right_enable = true;
    ctx->channels[i].key_on = false;

    for (int j = 0; j < 4; j++) {
      ctx->channels[i].operators[j].multiple = 1;
      ctx->channels[i].operators[j].detune = 0;
      ctx->channels[i].operators[j].total_level = 127;
      ctx->channels[i].operators[j].rate_scaling = 0;
      ctx->channels[i].operators[j].attack_rate = 0;
      ctx->channels[i].operators[j].decay_rate = 0;
      ctx->channels[i].operators[j].sustain_level = 0;
      ctx->channels[i].operators[j].release_rate = 0;
      ctx->channels[i].operators[j].envelope_level = 0;
      ctx->channels[i].operators[j].envelope_state = RELEASE;
      ctx->channels[i].operators[j].key_on = false;
    }
  }

  ctx->timer_a = 0;
  ctx->timer_b = 0;
  ctx->timer_a_counter = 0;
  ctx->timer_b_counter = 0;
  ctx->timer_a_enabled = false;
  ctx->timer_b_enabled = false;
  ctx->timer_a_overflow = false;
  ctx->timer_b_overflow = false;
}

/**
 * @brief Escreve um valor em um registrador
 */
void ym2612_write_reg(ym2612_context_t *ctx, uint8_t bank, uint8_t reg,
                      uint8_t value) {
  if (!ctx)
    return;

  uint16_t reg_offset = bank * 0x100 + reg;
  ctx->registers[reg_offset] = value;

  // Processa registradores especiais
  if (reg >= 0x30 && reg <= 0x9F) {
    // Registradores de operadores
    uint8_t channel = (reg & 0x03);
    uint8_t operator_index = (reg >> 2) & 0x03;

    switch (reg & 0xF0) {
    case 0x30: // DT1/MUL
      ctx->channels[channel].operators[operator_index].detune =
          (value >> 4) & 0x07;
      ctx->channels[channel].operators[operator_index].multiple = value & 0x0F;
      break;

    case 0x40: // TL
      ctx->channels[channel].operators[operator_index].total_level =
          value & 0x7F;
      break;

    case 0x50: // RS/AR
      ctx->channels[channel].operators[operator_index].rate_scaling =
          (value >> 6) & 0x03;
      ctx->channels[channel].operators[operator_index].attack_rate =
          value & 0x1F;
      break;

    case 0x60: // AM/D1R
      ctx->channels[channel].operators[operator_index].decay_rate =
          value & 0x1F;
      break;

    case 0x70: // D2R
      ctx->channels[channel].operators[operator_index].sustain_rate =
          value & 0x1F;
      break;

    case 0x80: // D1L/RR
      ctx->channels[channel].operators[operator_index].sustain_level =
          (value >> 4) & 0x0F;
      ctx->channels[channel].operators[operator_index].release_rate =
          value & 0x0F;
      break;
    }
  } else if (reg >= 0xA0 && reg <= 0xB7) {
    // Registradores de canal
    uint8_t channel = reg & 0x03;

    switch (reg & 0xFC) {
    case 0xA0: // Frequência LSB
      ctx->channels[channel].frequency =
          (ctx->channels[channel].frequency & 0xFF00) | value;
      break;

    case 0xA4: // Block/Frequência MSB
      ctx->channels[channel].block = (value >> 3) & 0x07;
      ctx->channels[channel].frequency =
          (ctx->channels[channel].frequency & 0x00FF) | ((value & 0x07) << 8);
      break;

    case 0xB0: // Feedback/Algorithm
      ctx->channels[channel].algorithm = value & 0x07;
      ctx->channels[channel].feedback = (value >> 3) & 0x07;
      break;

    case 0xB4: // LR/AMS/FMS
      ctx->channels[channel].left_enable = (value & 0x80) != 0;
      ctx->channels[channel].right_enable = (value & 0x40) != 0;
      break;
    }
  } else if (reg == 0x28) {
    // Key On/Off
    uint8_t channel = value & 0x07;
    if (channel < YM2612_NUM_CHANNELS) {
      for (int i = 0; i < 4; i++) {
        ctx->channels[channel].operators[i].key_on = (value & (0x10 << i)) != 0;
        if (ctx->channels[channel].operators[i].key_on) {
          ctx->channels[channel].operators[i].envelope_state = ATTACK;
          ctx->channels[channel].operators[i].envelope_level = 0;
        }
      }
    }
  }
}

/**
 * @brief Lê um valor de um registrador
 */
uint8_t ym2612_read_reg(ym2612_context_t *ctx, uint8_t bank, uint8_t reg) {
  if (!ctx)
    return 0;
  return ctx->registers[bank * 0x100 + reg];
}

/**
 * @brief Atualiza os timers
 */
void ym2612_timer_tick(ym2612_context_t *ctx) {
  if (!ctx)
    return;

  // Timer A
  if (ctx->timer_a_enabled) {
    ctx->timer_a_counter++;
    if (ctx->timer_a_counter >= ctx->timer_a) {
      ctx->timer_a_counter = 0;
      ctx->timer_a_overflow = true;
      if (ctx->timer_a_callback) {
        ctx->timer_a_callback(ctx->timer_a_callback_data);
      }
    }
  }

  // Timer B
  if (ctx->timer_b_enabled) {
    ctx->timer_b_counter++;
    if (ctx->timer_b_counter >= ctx->timer_b) {
      ctx->timer_b_counter = 0;
      ctx->timer_b_overflow = true;
      if (ctx->timer_b_callback) {
        ctx->timer_b_callback(ctx->timer_b_callback_data);
      }
    }
  }
}

/**
 * @brief Define o callback para o timer A
 */
void ym2612_set_timer_a_callback(ym2612_context_t *ctx,
                                 ym2612_timer_callback_t callback, void *data) {
  if (!ctx)
    return;
  ctx->timer_a_callback = callback;
  ctx->timer_a_callback_data = data;
}

/**
 * @brief Define o callback para o timer B
 */
void ym2612_set_timer_b_callback(ym2612_context_t *ctx,
                                 ym2612_timer_callback_t callback, void *data) {
  if (!ctx)
    return;
  ctx->timer_b_callback = callback;
  ctx->timer_b_callback_data = data;
}

/**
 * @brief Gera amostras de áudio
 */
void ym2612_generate_samples(ym2612_context_t *ctx, int16_t *buffer,
                             uint32_t num_samples) {
  if (!ctx || !buffer)
    return;

  // Limpa o buffer
  memset(buffer, 0, num_samples * 2 * sizeof(int16_t));

  // Gera amostras para cada canal
  for (int i = 0; i < YM2612_NUM_CHANNELS; i++) {
    if (ctx->channels[i].key_on) {
      generate_channel_samples(ctx, &ctx->channels[i], buffer, num_samples);
    }
  }
}
