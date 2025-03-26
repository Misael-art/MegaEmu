/**
 * @file apu_mixer.c
 * @brief Implementação do mixer de áudio otimizado para o APU do NES
 */

#include "apu_mixer.h"
#include "../../../utils/logging.h"
#include <math.h>
#include <string.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

// Constantes para geração das tabelas de lookup
#define PULSE_LEVEL 95.52
#define TND_LEVEL 163.67
#define DMC_LEVEL 159.79

// Coeficientes do filtro passa-baixa
#define LPF_BETA_DEFAULT 0.6f
#define LPF_BETA_HQ 0.8f

/**
 * @brief Gera as tabelas de lookup para mistura não-linear
 */
static void generate_lookup_tables(nes_apu_mixer_t *mixer) {
  if (!mixer)
    return;

  // Tabela para canais pulse (95.52 / (8128.0 / x + 100))
  for (int i = 0; i < APU_PULSE_TABLE_SIZE; i++) {
    mixer->pulse_table[i] = PULSE_LEVEL / (8128.0f / (float)i + 100.0f);
  }

  // Tabela para canais TND (159.79 / (22638.0 / x + 100))
  for (int i = 0; i < APU_TND_TABLE_SIZE; i++) {
    mixer->tnd_table[i] = TND_LEVEL / (22638.0f / (float)i + 100.0f);
  }
}

void apu_mixer_init(nes_apu_mixer_t *mixer, uint32_t sample_rate) {
  if (!mixer) {
    LOG_ERROR("APU Mixer: Ponteiro nulo passado para inicialização");
    return;
  }

  // Limpar estrutura
  memset(mixer, 0, sizeof(nes_apu_mixer_t));

  // Configurar volumes padrão
  mixer->pulse1_volume = 1.0f;
  mixer->pulse2_volume = 1.0f;
  mixer->triangle_volume = 1.0f;
  mixer->noise_volume = 1.0f;
  mixer->dmc_volume = 1.0f;
  mixer->master_volume = 1.0f;

  // Configurar filtro
  mixer->lpf_beta = LPF_BETA_DEFAULT;
  mixer->filter_enabled = true;
  mixer->high_quality_mode = false;
  mixer->sample_rate = sample_rate;

  // Gerar tabelas de lookup
  generate_lookup_tables(mixer);

  LOG_INFO("APU Mixer: Inicializado com sample_rate=%u Hz", sample_rate);
}

void apu_mixer_reset(nes_apu_mixer_t *mixer) {
  if (!mixer)
    return;

  // Resetar estado do filtro
  mixer->lpf_acc = 0.0f;
  mixer->lpf_prev = 0.0f;
}

void apu_mixer_set_volumes(nes_apu_mixer_t *mixer, float pulse1, float pulse2,
                           float triangle, float noise, float dmc,
                           float master) {
  if (!mixer)
    return;

  mixer->pulse1_volume = pulse1;
  mixer->pulse2_volume = pulse2;
  mixer->triangle_volume = triangle;
  mixer->noise_volume = noise;
  mixer->dmc_volume = dmc;
  mixer->master_volume = master;
}

void apu_mixer_enable_filter(nes_apu_mixer_t *mixer, bool enabled) {
  if (!mixer)
    return;
  mixer->filter_enabled = enabled;
}

void apu_mixer_set_quality(nes_apu_mixer_t *mixer, bool high_quality) {
  if (!mixer)
    return;
  mixer->high_quality_mode = high_quality;
  mixer->lpf_beta = high_quality ? LPF_BETA_HQ : LPF_BETA_DEFAULT;
}

float apu_mixer_mix(nes_apu_mixer_t *mixer, uint8_t pulse1, uint8_t pulse2,
                    uint8_t triangle, uint8_t noise, uint8_t dmc) {
  if (!mixer)
    return 0.0f;

  float output = 0.0f;

#ifdef __SSE2__
  if (mixer->high_quality_mode) {
    // Versão SSE2 otimizada para modo de alta qualidade
    __m128 pulse_vec = _mm_set_ps(
        mixer->pulse_table[pulse1] * mixer->pulse1_volume,
        mixer->pulse_table[pulse2] * mixer->pulse2_volume, 0.0f, 0.0f);

    __m128 tnd_vec = _mm_set_ps(
        mixer->tnd_table[3 * triangle + 2 * noise + dmc] *
            (mixer->triangle_volume + mixer->noise_volume + mixer->dmc_volume) /
            3.0f,
        0.0f, 0.0f, 0.0f);

    // Somar componentes
    __m128 sum = _mm_add_ps(pulse_vec, tnd_vec);

    // Aplicar volume master
    sum = _mm_mul_ps(sum, _mm_set1_ps(mixer->master_volume));

    // Extrair resultado
    float result[4];
    _mm_store_ps(result, sum);
    output = result[3] + result[2] + result[1] + result[0];
  } else
#endif
  {
    // Versão escalar para modo normal
    float pulse_out = mixer->pulse_table[pulse1 + pulse2] *
                      (mixer->pulse1_volume + mixer->pulse2_volume) / 2.0f;

    float tnd_out =
        mixer->tnd_table[3 * triangle + 2 * noise + dmc] *
        (mixer->triangle_volume + mixer->noise_volume + mixer->dmc_volume) /
        3.0f;

    output = (pulse_out + tnd_out) * mixer->master_volume;
  }

  // Aplicar filtro passa-baixa se habilitado
  if (mixer->filter_enabled) {
    mixer->lpf_acc =
        mixer->lpf_beta * output + (1.0f - mixer->lpf_beta) * mixer->lpf_prev;
    mixer->lpf_prev = mixer->lpf_acc;
    output = mixer->lpf_acc;
  }

  return output;
}
