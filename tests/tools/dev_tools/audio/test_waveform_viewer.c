/**
 * @file test_waveform_viewer.c
 * @brief Testes unitários para o visualizador de forma de onda
 */

#include "../../../../src/tools/dev_tools/audio/waveform_viewer.h"
#include <math.h>
#include <unity.h>

static waveform_viewer_t *viewer;
static waveform_config_t config;

void setUp(void) {
  config.sample_rate = 44100;
  config.window_size = 1024;
  config.zoom_level = 1.0f;
  config.auto_scroll = true;
  config.show_grid = true;
  config.show_peaks = true;
  config.update_rate = 16; // ~60 Hz

  viewer = waveform_create(&config);
}

void tearDown(void) {
  if (viewer) {
    waveform_destroy(viewer);
    viewer = NULL;
  }
}

void test_viewer_initialization(void) {
  TEST_ASSERT_NOT_NULL(viewer);
  TEST_ASSERT_EQUAL_UINT32(44100, viewer->config.sample_rate);
  TEST_ASSERT_EQUAL_UINT32(1024, viewer->config.window_size);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, viewer->config.zoom_level);
  TEST_ASSERT_TRUE(viewer->config.auto_scroll);
  TEST_ASSERT_TRUE(viewer->config.show_grid);
  TEST_ASSERT_TRUE(viewer->config.show_peaks);
  TEST_ASSERT_TRUE(viewer->active);
}

void test_viewer_sample_addition(void) {
  // Testar adição de amostras individuais
  float test_samples[] = {0.5f, -0.5f, 0.25f, -0.25f};

  for (int i = 0; i < 4; i++) {
    waveform_add_sample(viewer, WAVE_CHANNEL_PULSE1, test_samples[i]);
  }

  // Verificar se as amostras foram armazenadas corretamente
  TEST_ASSERT_EQUAL_FLOAT(test_samples[3],
                          viewer->channel_buffers[WAVE_CHANNEL_PULSE1][3]);
}

void test_viewer_mixed_samples(void) {
  float samples[WAVE_CHANNEL_COUNT - 1] = {
      0.5f,  // PULSE1
      0.5f,  // PULSE2
      0.25f, // TRIANGLE
      0.25f, // NOISE
      0.125f // DMC
  };

  waveform_add_mixed_sample(viewer, samples);

  // Verificar se a mistura foi calculada corretamente
  float expected = (0.5f + 0.5f + 0.25f + 0.25f + 0.125f) / 5.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, expected,
                           viewer->channel_buffers[WAVE_CHANNEL_MIXED][0]);
}

void test_viewer_statistics(void) {
  // Adicionar uma onda senoidal de teste
  for (int i = 0; i < 100; i++) {
    float sample = sinf(2.0f * M_PI * (float)i / 100.0f);
    waveform_add_sample(viewer, WAVE_CHANNEL_PULSE1, sample);
  }

  // Verificar pico (deve ser próximo de 1.0)
  float peak = waveform_get_peak(viewer, WAVE_CHANNEL_PULSE1);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, peak);

  // Verificar RMS (deve ser próximo de 0.707 para uma senoide)
  float rms = waveform_get_rms(viewer, WAVE_CHANNEL_PULSE1);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.707f, rms);
}

void test_viewer_config_changes(void) {
  // Testar mudança de zoom
  waveform_set_zoom(viewer, 2.0f);
  TEST_ASSERT_EQUAL_FLOAT(2.0f, viewer->config.zoom_level);

  // Testar limite inferior de zoom
  waveform_set_zoom(viewer, 0.05f);
  TEST_ASSERT_EQUAL_FLOAT(0.1f, viewer->config.zoom_level);

  // Testar limite superior de zoom
  waveform_set_zoom(viewer, 20.0f);
  TEST_ASSERT_EQUAL_FLOAT(10.0f, viewer->config.zoom_level);

  // Testar mudança de auto-scroll
  waveform_set_auto_scroll(viewer, false);
  TEST_ASSERT_FALSE(viewer->config.auto_scroll);
}

void test_viewer_reset(void) {
  // Adicionar algumas amostras
  for (int i = 0; i < 10; i++) {
    waveform_add_sample(viewer, WAVE_CHANNEL_PULSE1, 1.0f);
  }

  // Resetar o visualizador
  waveform_reset(viewer);

  // Verificar se os buffers foram limpos
  for (int i = 0; i < WAVE_CHANNEL_COUNT; i++) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, viewer->channel_buffers[i][0]);
    TEST_ASSERT_EQUAL_UINT32(0, viewer->buffer_pos[i]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, viewer->peak_values[i]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, viewer->rms_values[i]);
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_viewer_initialization);
  RUN_TEST(test_viewer_sample_addition);
  RUN_TEST(test_viewer_mixed_samples);
  RUN_TEST(test_viewer_statistics);
  RUN_TEST(test_viewer_config_changes);
  RUN_TEST(test_viewer_reset);

  return UNITY_END();
}
