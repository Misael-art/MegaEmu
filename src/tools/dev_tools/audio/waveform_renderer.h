/**
 * @file waveform_renderer.h
 * @brief Renderizador de forma de onda para debug de áudio
 */

#ifndef WAVEFORM_RENDERER_H
#define WAVEFORM_RENDERER_H

#include "waveform_viewer.h"
#include <stdint.h>

// Cores para renderização
typedef struct {
  uint8_t r, g, b, a;
} waveform_color_t;

// Configurações de renderização
typedef struct {
  uint32_t width;              // Largura da área de renderização
  uint32_t height;             // Altura da área de renderização
  uint32_t grid_size;          // Tamanho da grade
  float line_thickness;        // Espessura das linhas
  waveform_color_t bg_color;   // Cor de fundo
  waveform_color_t grid_color; // Cor da grade
  waveform_color_t wave_colors[WAVE_CHANNEL_COUNT]; // Cores dos canais
} waveform_render_config_t;

// Contexto de renderização
typedef struct {
  waveform_render_config_t config;
  uint8_t *framebuffer;      // Buffer de pixels RGBA
  uint32_t framebuffer_size; // Tamanho do framebuffer em bytes
  float *temp_buffer;        // Buffer temporário para processamento
  uint32_t temp_buffer_size; // Tamanho do buffer temporário
} waveform_render_context_t;

// Funções de interface
waveform_render_context_t *
waveform_renderer_create(const waveform_render_config_t *config);
void waveform_renderer_destroy(waveform_render_context_t *context);
void waveform_renderer_resize(waveform_render_context_t *context,
                              uint32_t width, uint32_t height);
void waveform_renderer_set_config(waveform_render_context_t *context,
                                  const waveform_render_config_t *config);

// Funções de renderização
void waveform_renderer_begin_frame(waveform_render_context_t *context);
void waveform_renderer_draw_grid(waveform_render_context_t *context);
void waveform_renderer_draw_channel(waveform_render_context_t *context,
                                    const waveform_viewer_t *viewer,
                                    waveform_channel_t channel);
void waveform_renderer_draw_peaks(waveform_render_context_t *context,
                                  const waveform_viewer_t *viewer);
void waveform_renderer_end_frame(waveform_render_context_t *context);

// Funções auxiliares
void waveform_renderer_set_pixel(waveform_render_context_t *context, uint32_t x,
                                 uint32_t y, const waveform_color_t *color);
void waveform_renderer_draw_line(waveform_render_context_t *context, float x1,
                                 float y1, float x2, float y2,
                                 const waveform_color_t *color);
void waveform_renderer_fill_rect(waveform_render_context_t *context, float x,
                                 float y, float w, float h,
                                 const waveform_color_t *color);

#endif // WAVEFORM_RENDERER_H
