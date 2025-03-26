/**
 * @file waveform_renderer.c
 * @brief Implementação do renderizador de forma de onda para debug de áudio
 */

#include "waveform_renderer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

// Funções auxiliares internas
static void blend_pixel(uint8_t *dst, const waveform_color_t *src);
static void interpolate_samples(float *dst, const float *src, uint32_t src_size,
                                uint32_t dst_size);

waveform_render_context_t *
waveform_renderer_create(const waveform_render_config_t *config) {
  if (!config || config->width == 0 || config->height == 0)
    return NULL;

  waveform_render_context_t *context =
      (waveform_render_context_t *)calloc(1, sizeof(waveform_render_context_t));
  if (!context)
    return NULL;

  // Copiar configurações
  memcpy(&context->config, config, sizeof(waveform_render_config_t));

  // Alocar framebuffer
  context->framebuffer_size = config->width * config->height * 4;
  context->framebuffer = (uint8_t *)calloc(1, context->framebuffer_size);
  if (!context->framebuffer) {
    free(context);
    return NULL;
  }

  // Alocar buffer temporário
  context->temp_buffer_size = config->width * sizeof(float);
  context->temp_buffer = (float *)calloc(1, context->temp_buffer_size);
  if (!context->temp_buffer) {
    free(context->framebuffer);
    free(context);
    return NULL;
  }

  return context;
}

void waveform_renderer_destroy(waveform_render_context_t *context) {
  if (!context)
    return;

  if (context->framebuffer) {
    free(context->framebuffer);
  }
  if (context->temp_buffer) {
    free(context->temp_buffer);
  }
  free(context);
}

void waveform_renderer_resize(waveform_render_context_t *context,
                              uint32_t width, uint32_t height) {
  if (!context || width == 0 || height == 0)
    return;

  // Realocar framebuffer
  uint32_t new_size = width * height * 4;
  uint8_t *new_buffer = (uint8_t *)realloc(context->framebuffer, new_size);
  if (!new_buffer)
    return;

  context->framebuffer = new_buffer;
  context->framebuffer_size = new_size;

  // Realocar buffer temporário
  uint32_t new_temp_size = width * sizeof(float);
  float *new_temp = (float *)realloc(context->temp_buffer, new_temp_size);
  if (!new_temp)
    return;

  context->temp_buffer = new_temp;
  context->temp_buffer_size = new_temp_size;

  // Atualizar configurações
  context->config.width = width;
  context->config.height = height;
}

void waveform_renderer_set_config(waveform_render_context_t *context,
                                  const waveform_render_config_t *config) {
  if (!context || !config)
    return;
  memcpy(&context->config, config, sizeof(waveform_render_config_t));
}

void waveform_renderer_begin_frame(waveform_render_context_t *context) {
  if (!context)
    return;

  // Limpar framebuffer com cor de fundo
#ifdef __SSE2__
  __m128i bg_color = _mm_set1_epi32(
      (context->config.bg_color.r << 24) | (context->config.bg_color.g << 16) |
      (context->config.bg_color.b << 8) | context->config.bg_color.a);

  for (uint32_t i = 0; i < context->framebuffer_size; i += 16) {
    _mm_store_si128((__m128i *)&context->framebuffer[i], bg_color);
  }
#else
  for (uint32_t i = 0; i < context->framebuffer_size; i += 4) {
    context->framebuffer[i] = context->config.bg_color.r;
    context->framebuffer[i + 1] = context->config.bg_color.g;
    context->framebuffer[i + 2] = context->config.bg_color.b;
    context->framebuffer[i + 3] = context->config.bg_color.a;
  }
#endif
}

void waveform_renderer_draw_grid(waveform_render_context_t *context) {
  if (!context || !context->config.show_grid)
    return;

  uint32_t grid_size = context->config.grid_size;
  if (grid_size == 0)
    return;

  // Desenhar linhas verticais
  for (uint32_t x = 0; x < context->config.width; x += grid_size) {
    waveform_renderer_draw_line(context, x, 0, x, context->config.height,
                                &context->config.grid_color);
  }

  // Desenhar linhas horizontais
  for (uint32_t y = 0; y < context->config.height; y += grid_size) {
    waveform_renderer_draw_line(context, 0, y, context->config.width, y,
                                &context->config.grid_color);
  }
}

void waveform_renderer_draw_channel(waveform_render_context_t *context,
                                    const waveform_viewer_t *viewer,
                                    waveform_channel_t channel) {
  if (!context || !viewer || channel >= WAVE_CHANNEL_COUNT)
    return;

  // Interpolar amostras para a largura da tela
  interpolate_samples(context->temp_buffer, viewer->channel_buffers[channel],
                      WAVEFORM_BUFFER_SIZE, context->config.width);

  // Desenhar forma de onda
  float mid_y = context->config.height / 2.0f;
  float scale_y = context->config.height / 2.0f;

  for (uint32_t x = 1; x < context->config.width; x++) {
    float y1 = mid_y - context->temp_buffer[x - 1] * scale_y;
    float y2 = mid_y - context->temp_buffer[x] * scale_y;

    waveform_renderer_draw_line(context, x - 1, y1, x, y2,
                                &context->config.wave_colors[channel]);
  }
}

void waveform_renderer_draw_peaks(waveform_render_context_t *context,
                                  const waveform_viewer_t *viewer) {
  if (!context || !viewer || !context->config.show_peaks)
    return;

  float bar_width = 20.0f;
  float spacing = 5.0f;
  float x = context->config.width - (bar_width + spacing) * WAVE_CHANNEL_COUNT;

  for (int i = 0; i < WAVE_CHANNEL_COUNT; i++) {
    float peak = viewer->peak_values[i];
    float rms = viewer->rms_values[i];

    // Desenhar barra de pico
    float peak_height = peak * context->config.height;
    waveform_renderer_fill_rect(context, x,
                                context->config.height - peak_height, bar_width,
                                peak_height, &context->config.wave_colors[i]);

    // Desenhar indicador RMS
    float rms_height = rms * context->config.height;
    waveform_renderer_fill_rect(context, x, context->config.height - rms_height,
                                bar_width, 2.0f,
                                &context->config.wave_colors[i]);

    x += bar_width + spacing;
  }
}

void waveform_renderer_end_frame(waveform_render_context_t *context) {
  if (!context)
    return;
  // Nada a fazer por enquanto
}

void waveform_renderer_set_pixel(waveform_render_context_t *context, uint32_t x,
                                 uint32_t y, const waveform_color_t *color) {
  if (!context || !color || x >= context->config.width ||
      y >= context->config.height)
    return;

  uint32_t offset = (y * context->config.width + x) * 4;
  blend_pixel(&context->framebuffer[offset], color);
}

void waveform_renderer_draw_line(waveform_render_context_t *context, float x1,
                                 float y1, float x2, float y2,
                                 const waveform_color_t *color) {
  if (!context || !color)
    return;

  // Algoritmo de Bresenham modificado para anti-aliasing
  float dx = x2 - x1;
  float dy = y2 - y1;
  float steps = fmaxf(fabsf(dx), fabsf(dy));

  if (steps < 1) {
    waveform_renderer_set_pixel(context, x1, y1, color);
    return;
  }

  float x_inc = dx / steps;
  float y_inc = dy / steps;
  float x = x1;
  float y = y1;

  for (int i = 0; i <= steps; i++) {
    waveform_renderer_set_pixel(context, x, y, color);
    x += x_inc;
    y += y_inc;
  }
}

void waveform_renderer_fill_rect(waveform_render_context_t *context, float x,
                                 float y, float w, float h,
                                 const waveform_color_t *color) {
  if (!context || !color)
    return;

  int32_t x1 = (int32_t)x;
  int32_t y1 = (int32_t)y;
  int32_t x2 = (int32_t)(x + w);
  int32_t y2 = (int32_t)(y + h);

  x1 = fmaxf(0, fminf(x1, context->config.width - 1));
  y1 = fmaxf(0, fminf(y1, context->config.height - 1));
  x2 = fmaxf(0, fminf(x2, context->config.width));
  y2 = fmaxf(0, fminf(y2, context->config.height));

  for (int32_t py = y1; py < y2; py++) {
    for (int32_t px = x1; px < x2; px++) {
      waveform_renderer_set_pixel(context, px, py, color);
    }
  }
}

static void blend_pixel(uint8_t *dst, const waveform_color_t *src) {
  if (!dst || !src)
    return;

  float alpha = src->a / 255.0f;
  dst[0] = (uint8_t)(dst[0] * (1.0f - alpha) + src->r * alpha);
  dst[1] = (uint8_t)(dst[1] * (1.0f - alpha) + src->g * alpha);
  dst[2] = (uint8_t)(dst[2] * (1.0f - alpha) + src->b * alpha);
  dst[3] = (uint8_t)(255 - ((255 - dst[3]) * (255 - src->a)) / 255);
}

static void interpolate_samples(float *dst, const float *src, uint32_t src_size,
                                uint32_t dst_size) {
  if (!dst || !src || src_size == 0 || dst_size == 0)
    return;

  for (uint32_t i = 0; i < dst_size; i++) {
    float pos = (float)i * src_size / dst_size;
    uint32_t idx = (uint32_t)pos;
    float frac = pos - idx;

    if (idx >= src_size - 1) {
      dst[i] = src[src_size - 1];
    } else {
      dst[i] = src[idx] * (1.0f - frac) + src[idx + 1] * frac;
    }
  }
}
