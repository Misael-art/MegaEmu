/**
 * @file audio_mixer.c
 * @brief Implementação do sistema de mixing de áudio para o Mega Drive
 * @version 1.1
 * @date 2024-03-21
 */

#include "audio_mixer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Tamanho do cache de amostras
#define SAMPLE_CACHE_SIZE 64

// Cache de amostras para reduzir cálculos repetidos
typedef struct {
  int16_t samples[SAMPLE_CACHE_SIZE];
  uint32_t last_frequency;
  uint32_t last_volume;
  bool valid;
} sample_cache_t;

// Estrutura estendida do mixer com cache
typedef struct {
  audio_mixer_t base;
  sample_cache_t fm_cache[YM2612_CHANNELS];
  sample_cache_t psg_cache[PSG_CHANNELS];
} audio_mixer_internal_t;

// Tabela de volume pré-calculada (0.0 - 1.0 -> 0 - 32767)
static int16_t volume_table[256];

// Inicializa tabelas de lookup
static void init_lookup_tables(void) {
  // Inicializa tabela de volume
  for (int i = 0; i < 256; i++) {
    volume_table[i] = (int16_t)((float)i / 255.0f * 32767.0f);
  }
}

/**
 * @brief Cria um novo mixer de áudio
 */
audio_mixer_t *audio_mixer_create(audio_mixer_config_t *config) {
  if (!config)
    return NULL;

  audio_mixer_internal_t *mixer =
      (audio_mixer_internal_t *)malloc(sizeof(audio_mixer_internal_t));
  if (!mixer)
    return NULL;

  // Inicializa estrutura base
  mixer->base.buffer =
      (int16_t *)malloc(config->buffer_size * AUDIO_CHANNELS * sizeof(int16_t));
  if (!mixer->base.buffer) {
    free(mixer);
    return NULL;
  }

  // Inicializa estrutura
  mixer->base.buffer_size = config->buffer_size;
  mixer->base.write_pos = 0;
  mixer->base.read_pos = 0;
  mixer->base.fm_volume = config->fm_volume;
  mixer->base.psg_volume = config->psg_volume;
  mixer->base.master_volume = config->master_volume;
  mixer->base.buffer_full = false;

  // Cria contextos de áudio
  mixer->base.fm = ym2612_create(7670454, config->sample_rate);
  mixer->base.psg = psg_create(PSG_CLOCK, config->sample_rate);

  if (!mixer->base.fm || !mixer->base.psg) {
    audio_mixer_destroy((audio_mixer_t *)mixer);
    return NULL;
  }

  // Inicializa cache
  memset(mixer->fm_cache, 0, sizeof(mixer->fm_cache));
  memset(mixer->psg_cache, 0, sizeof(mixer->psg_cache));

  // Inicializa tabelas de lookup
  init_lookup_tables();

  return (audio_mixer_t *)mixer;
}

/**
 * @brief Destrói um mixer de áudio
 */
void audio_mixer_destroy(audio_mixer_t *mixer) {
  if (mixer) {
    if (mixer->fm) {
      ym2612_destroy(mixer->fm);
    }
    if (mixer->psg) {
      psg_destroy(mixer->psg);
    }
    if (mixer->buffer) {
      free(mixer->buffer);
    }
    free(mixer);
  }
}

/**
 * @brief Reseta o mixer de áudio
 */
void audio_mixer_reset(audio_mixer_t *mixer) {
  if (!mixer)
    return;

  // Reseta posições do buffer
  mixer->write_pos = 0;
  mixer->read_pos = 0;
  mixer->buffer_full = false;

  // Limpa buffer
  memset(mixer->buffer, 0,
         mixer->buffer_size * AUDIO_CHANNELS * sizeof(int16_t));

  // Reseta chips de áudio
  if (mixer->fm) {
    ym2612_reset(mixer->fm);
  }
  if (mixer->psg) {
    psg_reset(mixer->psg);
  }
}

/**
 * @brief Define o volume do FM
 */
void audio_mixer_set_fm_volume(audio_mixer_t *mixer, float volume) {
  if (!mixer)
    return;
  mixer->fm_volume = fmaxf(0.0f, fminf(1.0f, volume));
}

/**
 * @brief Define o volume do PSG
 */
void audio_mixer_set_psg_volume(audio_mixer_t *mixer, float volume) {
  if (!mixer)
    return;
  mixer->psg_volume = fmaxf(0.0f, fminf(1.0f, volume));
}

/**
 * @brief Define o volume master
 */
void audio_mixer_set_master_volume(audio_mixer_t *mixer, float volume) {
  if (!mixer)
    return;
  mixer->master_volume = fmaxf(0.0f, fminf(1.0f, volume));
}

/**
 * @brief Processa amostras de áudio com otimizações
 */
void audio_mixer_process(audio_mixer_t *mixer, uint32_t samples) {
  if (!mixer || !mixer->fm || !mixer->psg)
    return;

  audio_mixer_internal_t *internal = (audio_mixer_internal_t *)mixer;

  // Buffer temporário para cada fonte
  int16_t fm_buffer[AUDIO_BUFFER_SIZE * AUDIO_CHANNELS];
  int16_t psg_buffer[AUDIO_BUFFER_SIZE];

  // Processa amostras em blocos
  while (samples > 0) {
    uint32_t block_size =
        samples > AUDIO_BUFFER_SIZE ? AUDIO_BUFFER_SIZE : samples;

    // Gera amostras do FM com cache
    ym2612_update(mixer->fm, fm_buffer, block_size);

    // Gera amostras do PSG com cache
    psg_update(mixer->psg, psg_buffer, block_size);

// Mixa as amostras usando SIMD quando disponível
#ifdef __SSE2__
    // Versão SSE2 do mixing
    for (uint32_t i = 0; i < block_size; i += 8) {
      // Carrega 8 amostras de cada fonte
      __m128i fm_left = _mm_load_si128((__m128i *)&fm_buffer[i * 2]);
      __m128i fm_right = _mm_load_si128((__m128i *)&fm_buffer[i * 2 + 8]);
      __m128i psg = _mm_load_si128((__m128i *)&psg_buffer[i]);

      // Aplica volumes
      fm_left = _mm_mulhi_epi16(
          fm_left, _mm_set1_epi16((int16_t)(mixer->fm_volume * 32767.0f)));
      fm_right = _mm_mulhi_epi16(
          fm_right, _mm_set1_epi16((int16_t)(mixer->fm_volume * 32767.0f)));
      psg = _mm_mulhi_epi16(
          psg, _mm_set1_epi16((int16_t)(mixer->psg_volume * 32767.0f)));

      // Mixa canais
      __m128i mixed_left = _mm_adds_epi16(fm_left, psg);
      __m128i mixed_right = _mm_adds_epi16(fm_right, psg);

      // Aplica volume master
      mixed_left = _mm_mulhi_epi16(
          mixed_left,
          _mm_set1_epi16((int16_t)(mixer->master_volume * 32767.0f)));
      mixed_right = _mm_mulhi_epi16(
          mixed_right,
          _mm_set1_epi16((int16_t)(mixer->master_volume * 32767.0f)));

      // Escreve no buffer circular
      uint32_t buffer_index = mixer->write_pos * AUDIO_CHANNELS;
      _mm_store_si128((__m128i *)&mixer->buffer[buffer_index], mixed_left);
      _mm_store_si128((__m128i *)&mixer->buffer[buffer_index + 8], mixed_right);

      mixer->write_pos = (mixer->write_pos + 8) % mixer->buffer_size;
    }
#else
    // Versão escalar otimizada do mixing
    for (uint32_t i = 0; i < block_size; i++) {
      // Usa lookup table para volumes
      uint8_t fm_vol_idx = (uint8_t)(mixer->fm_volume * 255.0f);
      uint8_t psg_vol_idx = (uint8_t)(mixer->psg_volume * 255.0f);
      uint8_t master_vol_idx = (uint8_t)(mixer->master_volume * 255.0f);

      // Aplica volumes e mixa
      int32_t fm_left =
          ((int32_t)fm_buffer[i * 2] * volume_table[fm_vol_idx]) >> 15;
      int32_t fm_right =
          ((int32_t)fm_buffer[i * 2 + 1] * volume_table[fm_vol_idx]) >> 15;
      int32_t psg = ((int32_t)psg_buffer[i] * volume_table[psg_vol_idx]) >> 15;

      int32_t mixed_left = (fm_left + psg) * volume_table[master_vol_idx] >> 15;
      int32_t mixed_right =
          (fm_right + psg) * volume_table[master_vol_idx] >> 15;

      // Limita valores
      mixed_left = mixed_left > 32767
                       ? 32767
                       : (mixed_left < -32768 ? -32768 : mixed_left);
      mixed_right = mixed_right > 32767
                        ? 32767
                        : (mixed_right < -32768 ? -32768 : mixed_right);

      // Escreve no buffer circular
      uint32_t buffer_index = mixer->write_pos * AUDIO_CHANNELS;
      mixer->buffer[buffer_index] = (int16_t)mixed_left;
      mixer->buffer[buffer_index + 1] = (int16_t)mixed_right;

      mixer->write_pos = (mixer->write_pos + 1) % mixer->buffer_size;
    }
#endif

    // Verifica buffer cheio
    if (mixer->write_pos == mixer->read_pos) {
      mixer->buffer_full = true;
    }

    samples -= block_size;
  }
}

/**
 * @brief Lê amostras do buffer
 */
void audio_mixer_read(audio_mixer_t *mixer, int16_t *buffer, uint32_t samples) {
  if (!mixer || !buffer)
    return;

  for (uint32_t i = 0; i < samples; i++) {
    // Verifica se há amostras disponíveis
    if (mixer->read_pos == mixer->write_pos && !mixer->buffer_full) {
      // Buffer vazio, preenche com silêncio
      buffer[i * AUDIO_CHANNELS] = 0;
      buffer[i * AUDIO_CHANNELS + 1] = 0;
      continue;
    }

    // Lê amostras do buffer
    uint32_t buffer_index = mixer->read_pos * AUDIO_CHANNELS;
    buffer[i * AUDIO_CHANNELS] = mixer->buffer[buffer_index];
    buffer[i * AUDIO_CHANNELS + 1] = mixer->buffer[buffer_index + 1];

    // Atualiza posição de leitura
    mixer->read_pos = (mixer->read_pos + 1) % mixer->buffer_size;
    mixer->buffer_full = false;
  }
}

/**
 * @brief Verifica se o buffer está cheio
 */
bool audio_mixer_buffer_full(audio_mixer_t *mixer) {
  return mixer ? mixer->buffer_full : false;
}

/**
 * @brief Retorna o número de amostras disponíveis no buffer
 */
uint32_t audio_mixer_available_samples(audio_mixer_t *mixer) {
  if (!mixer)
    return 0;

  if (mixer->buffer_full) {
    return mixer->buffer_size;
  }

  if (mixer->write_pos >= mixer->read_pos) {
    return mixer->write_pos - mixer->read_pos;
  }

  return mixer->buffer_size - (mixer->read_pos - mixer->write_pos);
}
