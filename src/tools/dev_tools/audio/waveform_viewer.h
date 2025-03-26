/**
 * @file waveform_viewer.h
 * @brief Visualizador de forma de onda para debug de áudio
 */

#ifndef WAVEFORM_VIEWER_H
#define WAVEFORM_VIEWER_H

#include <stdbool.h>
#include <stdint.h>

// Tamanho do buffer circular para amostras
#define WAVEFORM_BUFFER_SIZE 4096

// Canais disponíveis para visualização
typedef enum {
  WAVE_CHANNEL_PULSE1,
  WAVE_CHANNEL_PULSE2,
  WAVE_CHANNEL_TRIANGLE,
  WAVE_CHANNEL_NOISE,
  WAVE_CHANNEL_DMC,
  WAVE_CHANNEL_MIXED,
  WAVE_CHANNEL_COUNT
} waveform_channel_t;

// Configurações de visualização
typedef struct {
  uint32_t sample_rate; // Taxa de amostragem
  uint32_t window_size; // Tamanho da janela em amostras
  float zoom_level;     // Nível de zoom (1.0 = normal)
  bool auto_scroll;     // Rolagem automática
  bool show_grid;       // Mostrar grade
  bool show_peaks;      // Mostrar picos
  uint32_t update_rate; // Taxa de atualização em ms
} waveform_config_t;

// Estrutura principal do visualizador
typedef struct {
  // Configurações
  waveform_config_t config;

  // Buffers circulares por canal
  float *channel_buffers[WAVE_CHANNEL_COUNT];
  uint32_t buffer_pos[WAVE_CHANNEL_COUNT];

  // Estatísticas
  float peak_values[WAVE_CHANNEL_COUNT];
  float rms_values[WAVE_CHANNEL_COUNT];

  // Estado
  bool active;
  uint64_t last_update;
  void *render_context; // Contexto de renderização específico da plataforma
} waveform_viewer_t;

// Funções de interface
waveform_viewer_t *waveform_create(const waveform_config_t *config);
void waveform_destroy(waveform_viewer_t *viewer);
void waveform_reset(waveform_viewer_t *viewer);

// Funções de configuração
void waveform_set_config(waveform_viewer_t *viewer,
                         const waveform_config_t *config);
void waveform_set_zoom(waveform_viewer_t *viewer, float zoom_level);
void waveform_set_auto_scroll(waveform_viewer_t *viewer, bool enabled);

// Funções de amostragem
void waveform_add_sample(waveform_viewer_t *viewer, waveform_channel_t channel,
                         float sample);
void waveform_add_mixed_sample(waveform_viewer_t *viewer, float *samples);

// Funções de renderização
void waveform_render(waveform_viewer_t *viewer);
void waveform_render_channel(waveform_viewer_t *viewer,
                             waveform_channel_t channel);
void waveform_render_all(waveform_viewer_t *viewer);

// Funções de análise
float waveform_get_peak(waveform_viewer_t *viewer, waveform_channel_t channel);
float waveform_get_rms(waveform_viewer_t *viewer, waveform_channel_t channel);
void waveform_get_spectrum(waveform_viewer_t *viewer,
                           waveform_channel_t channel, float *spectrum,
                           uint32_t bins);

#endif // WAVEFORM_VIEWER_H
