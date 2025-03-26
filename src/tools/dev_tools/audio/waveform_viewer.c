/**
 * @file waveform_viewer.c
 * @brief Implementação do visualizador de forma de onda para debug de áudio
 */

#include "waveform_viewer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

// Funções auxiliares internas
static void update_statistics(waveform_viewer_t *viewer,
                              waveform_channel_t channel);
static uint64_t get_timestamp_ms(void);

waveform_viewer_t *waveform_create(const waveform_config_t *config) {
  if (!config)
    return NULL;

  waveform_viewer_t *viewer =
      (waveform_viewer_t *)calloc(1, sizeof(waveform_viewer_t));
  if (!viewer)
    return NULL;

  // Copiar configurações
  memcpy(&viewer->config, config, sizeof(waveform_config_t));

  // Alocar buffers para cada canal
  for (int i = 0; i < WAVE_CHANNEL_COUNT; i++) {
    viewer->channel_buffers[i] =
        (float *)calloc(WAVEFORM_BUFFER_SIZE, sizeof(float));
    if (!viewer->channel_buffers[i]) {
      waveform_destroy(viewer);
      return NULL;
    }
  }

  viewer->active = true;
  viewer->last_update = get_timestamp_ms();

  return viewer;
}

void waveform_destroy(waveform_viewer_t *viewer) {
  if (!viewer)
    return;

  // Liberar buffers
  for (int i = 0; i < WAVE_CHANNEL_COUNT; i++) {
    if (viewer->channel_buffers[i]) {
      free(viewer->channel_buffers[i]);
    }
  }

  free(viewer);
}

void waveform_reset(waveform_viewer_t *viewer) {
  if (!viewer)
    return;

  // Limpar buffers
  for (int i = 0; i < WAVE_CHANNEL_COUNT; i++) {
    memset(viewer->channel_buffers[i], 0, WAVEFORM_BUFFER_SIZE * sizeof(float));
    viewer->buffer_pos[i] = 0;
    viewer->peak_values[i] = 0.0f;
    viewer->rms_values[i] = 0.0f;
  }

  viewer->last_update = get_timestamp_ms();
}

void waveform_set_config(waveform_viewer_t *viewer,
                         const waveform_config_t *config) {
  if (!viewer || !config)
    return;
  memcpy(&viewer->config, config, sizeof(waveform_config_t));
}

void waveform_set_zoom(waveform_viewer_t *viewer, float zoom_level) {
  if (!viewer)
    return;
  viewer->config.zoom_level = fmaxf(0.1f, fminf(zoom_level, 10.0f));
}

void waveform_set_auto_scroll(waveform_viewer_t *viewer, bool enabled) {
  if (!viewer)
    return;
  viewer->config.auto_scroll = enabled;
}

void waveform_add_sample(waveform_viewer_t *viewer, waveform_channel_t channel,
                         float sample) {
  if (!viewer || channel >= WAVE_CHANNEL_COUNT)
    return;

  // Adicionar amostra ao buffer circular
  viewer->channel_buffers[channel][viewer->buffer_pos[channel]] = sample;
  viewer->buffer_pos[channel] =
      (viewer->buffer_pos[channel] + 1) % WAVEFORM_BUFFER_SIZE;

  // Atualizar estatísticas se necessário
  uint64_t current_time = get_timestamp_ms();
  if (current_time - viewer->last_update >= viewer->config.update_rate) {
    update_statistics(viewer, channel);
    viewer->last_update = current_time;
  }
}

void waveform_add_mixed_sample(waveform_viewer_t *viewer, float *samples) {
  if (!viewer || !samples)
    return;

  float mixed = 0.0f;

#ifdef __SSE2__
  // Versão SIMD otimizada para mistura
  __m128 sum = _mm_setzero_ps();
  for (int i = 0; i < WAVE_CHANNEL_COUNT - 1; i += 4) {
    __m128 samples_vec = _mm_loadu_ps(&samples[i]);
    sum = _mm_add_ps(sum, samples_vec);
  }

  // Reduzir soma
  float result[4];
  _mm_store_ps(result, sum);
  mixed = (result[0] + result[1] + result[2] + result[3]) /
          (float)(WAVE_CHANNEL_COUNT - 1);
#else
  // Versão escalar
  for (int i = 0; i < WAVE_CHANNEL_COUNT - 1; i++) {
    mixed += samples[i];
  }
  mixed /= (float)(WAVE_CHANNEL_COUNT - 1);
#endif

  // Adicionar amostra misturada
  waveform_add_sample(viewer, WAVE_CHANNEL_MIXED, mixed);
}

float waveform_get_peak(waveform_viewer_t *viewer, waveform_channel_t channel) {
  if (!viewer || channel >= WAVE_CHANNEL_COUNT)
    return 0.0f;
  return viewer->peak_values[channel];
}

float waveform_get_rms(waveform_viewer_t *viewer, waveform_channel_t channel) {
  if (!viewer || channel >= WAVE_CHANNEL_COUNT)
    return 0.0f;
  return viewer->rms_values[channel];
}

void waveform_get_spectrum(waveform_viewer_t *viewer,
                           waveform_channel_t channel, float *spectrum,
                           uint32_t bins) {
  if (!viewer || !spectrum || channel >= WAVE_CHANNEL_COUNT || bins == 0)
    return;

  // TODO: Implementar FFT para análise espectral
  memset(spectrum, 0, bins * sizeof(float));
}

static void update_statistics(waveform_viewer_t *viewer,
                              waveform_channel_t channel) {
  if (!viewer || channel >= WAVE_CHANNEL_COUNT)
    return;

  float peak = 0.0f;
  float rms_sum = 0.0f;

#ifdef __SSE2__
  // Versão SIMD otimizada para cálculo de estatísticas
  __m128 peak_vec = _mm_setzero_ps();
  __m128 rms_vec = _mm_setzero_ps();

  for (int i = 0; i < WAVEFORM_BUFFER_SIZE; i += 4) {
    __m128 samples = _mm_loadu_ps(&viewer->channel_buffers[channel][i]);

    // Atualizar pico
    peak_vec = _mm_max_ps(peak_vec, _mm_andnot_ps(_mm_set1_ps(-0.0f), samples));

    // Atualizar RMS
    rms_vec = _mm_add_ps(rms_vec, _mm_mul_ps(samples, samples));
  }

  // Reduzir resultados
  float peak_array[4], rms_array[4];
  _mm_store_ps(peak_array, peak_vec);
  _mm_store_ps(rms_array, rms_vec);

  peak = fmaxf(fmaxf(peak_array[0], peak_array[1]),
               fmaxf(peak_array[2], peak_array[3]));

  rms_sum = rms_array[0] + rms_array[1] + rms_array[2] + rms_array[3];
#else
  // Versão escalar
  for (int i = 0; i < WAVEFORM_BUFFER_SIZE; i++) {
    float sample = fabsf(viewer->channel_buffers[channel][i]);
    peak = fmaxf(peak, sample);
    rms_sum += sample * sample;
  }
#endif

  viewer->peak_values[channel] = peak;
  viewer->rms_values[channel] = sqrtf(rms_sum / WAVEFORM_BUFFER_SIZE);
}

static uint64_t get_timestamp_ms(void) {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}
