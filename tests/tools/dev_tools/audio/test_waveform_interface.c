/**
 * @file test_waveform_interface.c
 * @brief Testes unitários para a interface principal
 */

#include "unity.h"
#include "waveform_interface.h"
#include <string.h>

static waveform_interface_t *interface = NULL;
static const uint32_t TEST_WIDTH = 800;
static const uint32_t TEST_HEIGHT = 600;
static const char *TEST_TITLE = "Teste do Visualizador";

// Mock do visualizador
static waveform_config_t mock_config = {
    .sample_rate = 44100,
    .window_size = 1024,
    .zoom_level = 1,
    .auto_scroll = true,
    .show_grid = true,
    .show_peaks = true,
    .update_rate = 60,
    .channel_enabled = {true, true, true, true, true, true}};

waveform_viewer_t *waveform_create(void) { return (waveform_viewer_t *)1; }

void waveform_destroy(waveform_viewer_t *viewer) {}

void waveform_update(waveform_viewer_t *viewer) {}

const uint8_t *waveform_render(waveform_viewer_t *viewer) {
  static uint8_t mock_framebuffer[1024 * 1024 * 4];
  memset(mock_framebuffer, 0, sizeof(mock_framebuffer));
  return mock_framebuffer;
}

waveform_config_t waveform_get_config(waveform_viewer_t *viewer) {
  return mock_config;
}

void waveform_set_config(waveform_viewer_t *viewer,
                         const waveform_config_t *config) {
  mock_config = *config;
}

// Mock da interface de exibição
bool waveform_display_init(void) { return true; }

void waveform_display_quit(void) {}

waveform_display_t *waveform_display_create(uint32_t width, uint32_t height,
                                            const char *title) {
  return (waveform_display_t *)1;
}

void waveform_display_destroy(waveform_display_t *display) {}

bool waveform_display_process_events(waveform_display_t *display) {
  return true;
}

void waveform_display_update(waveform_display_t *display,
                             const uint8_t *framebuffer, uint32_t width,
                             uint32_t height) {}

// Mock da interface de linha de comando
waveform_cli_t *waveform_cli_create(waveform_viewer_t *viewer) {
  return (waveform_cli_t *)1;
}

void waveform_cli_destroy(waveform_cli_t *cli) {}

bool waveform_cli_process_char(waveform_cli_t *cli, char c) { return c != 'q'; }

void setUp(void) {
  interface = waveform_interface_create(TEST_WIDTH, TEST_HEIGHT, TEST_TITLE);
  TEST_ASSERT_NOT_NULL(interface);
}

void tearDown(void) {
  if (interface) {
    waveform_interface_destroy(interface);
    interface = NULL;
  }
}

void test_interface_creation(void) {
  TEST_ASSERT_NOT_NULL(interface->viewer);
  TEST_ASSERT_NOT_NULL(interface->display);
  TEST_ASSERT_NOT_NULL(interface->cli);
  TEST_ASSERT_TRUE(interface->running);
  TEST_ASSERT_FALSE(interface->paused);
  TEST_ASSERT_EQUAL_UINT32(0, interface->frame_count);
  TEST_ASSERT_NOT_EQUAL_UINT32(0, interface->last_update_time);
}

void test_interface_update(void) {
  uint32_t initial_frame_count = interface->frame_count;
  uint32_t initial_time = interface->last_update_time;

  // Testa atualização normal
  TEST_ASSERT_TRUE(waveform_interface_update(interface));
  TEST_ASSERT_EQUAL_UINT32(initial_frame_count + 1, interface->frame_count);
  TEST_ASSERT_GREATER_THAN_UINT32(initial_time, interface->last_update_time);

  // Testa atualização com visualização pausada
  waveform_interface_set_paused(interface, true);
  initial_frame_count = interface->frame_count;
  initial_time = interface->last_update_time;

  TEST_ASSERT_TRUE(waveform_interface_update(interface));
  TEST_ASSERT_EQUAL_UINT32(initial_frame_count, interface->frame_count);
  TEST_ASSERT_EQUAL_UINT32(initial_time, interface->last_update_time);
}

void test_interface_process_char(void) {
  // Testa caractere normal
  waveform_interface_process_char(interface, 'a');
  TEST_ASSERT_TRUE(interface->running);

  // Testa caractere de saída
  waveform_interface_process_char(interface, 'q');
  TEST_ASSERT_FALSE(interface->running);
}

void test_interface_pause(void) {
  TEST_ASSERT_FALSE(waveform_interface_is_paused(interface));

  waveform_interface_set_paused(interface, true);
  TEST_ASSERT_TRUE(waveform_interface_is_paused(interface));

  waveform_interface_set_paused(interface, false);
  TEST_ASSERT_FALSE(waveform_interface_is_paused(interface));
}

void test_interface_frame_count(void) {
  TEST_ASSERT_EQUAL_UINT32(0, waveform_interface_get_frame_count(interface));

  waveform_interface_update(interface);
  TEST_ASSERT_EQUAL_UINT32(1, waveform_interface_get_frame_count(interface));

  waveform_interface_update(interface);
  TEST_ASSERT_EQUAL_UINT32(2, waveform_interface_get_frame_count(interface));
}

void test_interface_update_time(void) {
  uint32_t initial_time = waveform_interface_get_last_update_time(interface);
  TEST_ASSERT_NOT_EQUAL_UINT32(0, initial_time);

  waveform_interface_update(interface);
  TEST_ASSERT_GREATER_THAN_UINT32(
      initial_time, waveform_interface_get_last_update_time(interface));
}

void test_interface_null_handling(void) {
  TEST_ASSERT_FALSE(waveform_interface_update(NULL));
  waveform_interface_process_char(NULL, 'a');
  waveform_interface_set_paused(NULL, true);
  TEST_ASSERT_FALSE(waveform_interface_is_paused(NULL));
  TEST_ASSERT_EQUAL_UINT32(0, waveform_interface_get_frame_count(NULL));
  TEST_ASSERT_EQUAL_UINT32(0, waveform_interface_get_last_update_time(NULL));
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_interface_creation);
  RUN_TEST(test_interface_update);
  RUN_TEST(test_interface_process_char);
  RUN_TEST(test_interface_pause);
  RUN_TEST(test_interface_frame_count);
  RUN_TEST(test_interface_update_time);
  RUN_TEST(test_interface_null_handling);

  return UNITY_END();
}
