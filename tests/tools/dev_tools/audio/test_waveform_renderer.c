/**
 * @file test_waveform_renderer.c
 * @brief Testes unitários para o renderizador de forma de onda
 */

#include "tools/dev_tools/audio/waveform_renderer.h"
#include "tools/dev_tools/audio/waveform_viewer.h"
#include "unity.h"
#include <math.h>
#include <string.h>

static waveform_render_context_t *context = NULL;
static waveform_render_config_t config;
static waveform_viewer_t *viewer = NULL;

void setUp(void) {
  // Configurar renderizador
  config.width = 800;
  config.height = 600;
  config.grid_size = 50;
  config.show_grid = true;
  config.show_peaks = true;
  config.bg_color = (waveform_color_t){32, 32, 32, 255};
  config.grid_color = (waveform_color_t){64, 64, 64, 255};

  // Cores dos canais
  config.wave_colors[0] = (waveform_color_t){255, 100, 100, 255}; // Pulse 1
  config.wave_colors[1] = (waveform_color_t){100, 255, 100, 255}; // Pulse 2
  config.wave_colors[2] = (waveform_color_t){100, 100, 255, 255}; // Triangle
  config.wave_colors[3] = (waveform_color_t){255, 255, 100, 255}; // Noise
  config.wave_colors[4] = (waveform_color_t){255, 100, 255, 255}; // DMC
  config.wave_colors[5] = (waveform_color_t){255, 255, 255, 255}; // Mixed

  context = waveform_renderer_create(&config);
  TEST_ASSERT_NOT_NULL(context);

  // Criar visualizador para testes
  waveform_config_t viewer_config = {.sample_rate = 44100,
                                     .window_size = 4096,
                                     .zoom_level = 1.0f,
                                     .auto_scroll = true,
                                     .show_grid = true,
                                     .show_peaks = true,
                                     .update_rate = 60};
  viewer = waveform_create(&viewer_config);
  TEST_ASSERT_NOT_NULL(viewer);
}

void tearDown(void) {
  if (context) {
    waveform_renderer_destroy(context);
    context = NULL;
  }
  if (viewer) {
    waveform_destroy(viewer);
    viewer = NULL;
  }
}

void test_renderer_initialization(void) {
  TEST_ASSERT_EQUAL_UINT32(800, context->config.width);
  TEST_ASSERT_EQUAL_UINT32(600, context->config.height);
  TEST_ASSERT_EQUAL_UINT32(50, context->config.grid_size);
  TEST_ASSERT_TRUE(context->config.show_grid);
  TEST_ASSERT_TRUE(context->config.show_peaks);
  TEST_ASSERT_NOT_NULL(context->framebuffer);
  TEST_ASSERT_NOT_NULL(context->temp_buffer);
}

void test_renderer_resize(void) {
  uint32_t new_width = 1024;
  uint32_t new_height = 768;

  waveform_renderer_resize(context, new_width, new_height);

  TEST_ASSERT_EQUAL_UINT32(new_width, context->config.width);
  TEST_ASSERT_EQUAL_UINT32(new_height, context->config.height);
  TEST_ASSERT_EQUAL_UINT32(new_width * new_height * 4,
                           context->framebuffer_size);
  TEST_ASSERT_EQUAL_UINT32(new_width * sizeof(float),
                           context->temp_buffer_size);
}

void test_renderer_clear_frame(void) {
  waveform_renderer_begin_frame(context);

  // Verificar se o framebuffer foi limpo com a cor de fundo
  for (uint32_t i = 0; i < context->framebuffer_size; i += 4) {
    TEST_ASSERT_EQUAL_UINT8(context->config.bg_color.r,
                            context->framebuffer[i]);
    TEST_ASSERT_EQUAL_UINT8(context->config.bg_color.g,
                            context->framebuffer[i + 1]);
    TEST_ASSERT_EQUAL_UINT8(context->config.bg_color.b,
                            context->framebuffer[i + 2]);
    TEST_ASSERT_EQUAL_UINT8(context->config.bg_color.a,
                            context->framebuffer[i + 3]);
  }
}

void test_renderer_draw_grid(void) {
  waveform_renderer_begin_frame(context);
  waveform_renderer_draw_grid(context);

  // Verificar alguns pixels da grade
  uint32_t grid_size = context->config.grid_size;
  for (uint32_t x = 0; x < context->config.width; x += grid_size) {
    uint32_t offset = (0 * context->config.width + x) * 4;
    TEST_ASSERT_EQUAL_UINT8(context->config.grid_color.r,
                            context->framebuffer[offset]);
    TEST_ASSERT_EQUAL_UINT8(context->config.grid_color.g,
                            context->framebuffer[offset + 1]);
    TEST_ASSERT_EQUAL_UINT8(context->config.grid_color.b,
                            context->framebuffer[offset + 2]);
  }
}

void test_renderer_draw_channel(void) {
  // Gerar onda senoidal de teste
  float freq = 440.0f; // Hz
  float sample_rate = 44100.0f;
  float amplitude = 0.5f;

  for (uint32_t i = 0; i < WAVEFORM_BUFFER_SIZE; i++) {
    float t = (float)i / sample_rate;
    viewer->channel_buffers[0][i] = amplitude * sinf(2.0f * M_PI * freq * t);
  }

  waveform_renderer_begin_frame(context);
  waveform_renderer_draw_channel(context, viewer, 0);

  // Verificar se alguns pixels foram alterados
  bool found_non_bg = false;
  for (uint32_t i = 0; i < context->framebuffer_size; i += 4) {
    if (context->framebuffer[i] != context->config.bg_color.r ||
        context->framebuffer[i + 1] != context->config.bg_color.g ||
        context->framebuffer[i + 2] != context->config.bg_color.b) {
      found_non_bg = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(found_non_bg);
}

void test_renderer_draw_peaks(void) {
  // Configurar valores de pico e RMS
  for (int i = 0; i < WAVE_CHANNEL_COUNT; i++) {
    viewer->peak_values[i] = 0.8f - (float)i * 0.1f;
    viewer->rms_values[i] = 0.4f - (float)i * 0.05f;
  }

  waveform_renderer_begin_frame(context);
  waveform_renderer_draw_peaks(context, viewer);

  // Verificar se as barras de pico foram desenhadas
  bool found_peak_colors = false;
  for (int i = 0; i < WAVE_CHANNEL_COUNT; i++) {
    uint32_t x = context->config.width - 20 * (WAVE_CHANNEL_COUNT - i);
    uint32_t y = context->config.height - 1;
    uint32_t offset = (y * context->config.width + x) * 4;

    if (context->framebuffer[offset] == config.wave_colors[i].r &&
        context->framebuffer[offset + 1] == config.wave_colors[i].g &&
        context->framebuffer[offset + 2] == config.wave_colors[i].b) {
      found_peak_colors = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(found_peak_colors);
}

void test_renderer_pixel_blending(void) {
  waveform_color_t test_color = {255, 0, 0, 128}; // Vermelho semi-transparente
  uint32_t x = 100;
  uint32_t y = 100;

  waveform_renderer_begin_frame(context);
  waveform_renderer_set_pixel(context, x, y, &test_color);

  uint32_t offset = (y * context->config.width + x) * 4;
  uint8_t expected_r =
      (uint8_t)(context->config.bg_color.r * 0.5f + 255 * 0.5f);

  TEST_ASSERT_EQUAL_UINT8(expected_r, context->framebuffer[offset]);
}

void test_renderer_line_drawing(void) {
  waveform_color_t line_color = {255, 255, 255, 255};
  float x1 = 100.0f;
  float y1 = 100.0f;
  float x2 = 200.0f;
  float y2 = 200.0f;

  waveform_renderer_begin_frame(context);
  waveform_renderer_draw_line(context, x1, y1, x2, y2, &line_color);

  // Verificar alguns pontos ao longo da linha
  bool found_line = false;
  for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
    uint32_t x = (uint32_t)(x1 + (x2 - x1) * t);
    uint32_t y = (uint32_t)(y1 + (y2 - y1) * t);
    uint32_t offset = (y * context->config.width + x) * 4;

    if (context->framebuffer[offset] == line_color.r &&
        context->framebuffer[offset + 1] == line_color.g &&
        context->framebuffer[offset + 2] == line_color.b) {
      found_line = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(found_line);
}

void test_renderer_rect_drawing(void) {
  waveform_color_t rect_color = {0, 255, 0, 255};
  float x = 100.0f;
  float y = 100.0f;
  float w = 50.0f;
  float h = 50.0f;

  waveform_renderer_begin_frame(context);
  waveform_renderer_fill_rect(context, x, y, w, h, &rect_color);

  // Verificar pixels dentro do retângulo
  bool found_rect = true;
  for (uint32_t py = (uint32_t)y; py < (uint32_t)(y + h); py++) {
    for (uint32_t px = (uint32_t)x; px < (uint32_t)(x + w); px++) {
      uint32_t offset = (py * context->config.width + px) * 4;
      if (context->framebuffer[offset] != rect_color.r ||
          context->framebuffer[offset + 1] != rect_color.g ||
          context->framebuffer[offset + 2] != rect_color.b) {
        found_rect = false;
        break;
      }
    }
    if (!found_rect)
      break;
  }
  TEST_ASSERT_TRUE(found_rect);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_renderer_initialization);
  RUN_TEST(test_renderer_resize);
  RUN_TEST(test_renderer_clear_frame);
  RUN_TEST(test_renderer_draw_grid);
  RUN_TEST(test_renderer_draw_channel);
  RUN_TEST(test_renderer_draw_peaks);
  RUN_TEST(test_renderer_pixel_blending);
  RUN_TEST(test_renderer_line_drawing);
  RUN_TEST(test_renderer_rect_drawing);

  return UNITY_END();
}
