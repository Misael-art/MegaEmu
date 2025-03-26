/**
 * @file test_waveform_cli.c
 * @brief Testes unitários para a interface de linha de comando
 */

#include "unity.h"
#include "waveform_cli.h"
#include "waveform_viewer.h"
#include <string.h>

static waveform_cli_t *cli = NULL;
static waveform_viewer_t *viewer = NULL;

// Mock do visualizador para testes
static waveform_config_t mock_config = {
    .sample_rate = 44100,
    .window_size = 1024,
    .zoom_level = 1,
    .auto_scroll = true,
    .show_grid = true,
    .show_peaks = true,
    .update_rate = 60,
    .channel_enabled = {true, true, true, true, true, true}};

waveform_config_t waveform_get_config(waveform_viewer_t *viewer) {
  return mock_config;
}

void waveform_set_config(waveform_viewer_t *viewer,
                         const waveform_config_t *config) {
  mock_config = *config;
}

void waveform_set_zoom(waveform_viewer_t *viewer, int zoom) {
  mock_config.zoom_level = zoom;
}

void waveform_set_auto_scroll(waveform_viewer_t *viewer, bool enabled) {
  mock_config.auto_scroll = enabled;
}

void setUp(void) {
  viewer = (waveform_viewer_t *)1; // Mock do visualizador
  cli = waveform_cli_create(viewer);
  TEST_ASSERT_NOT_NULL(cli);
}

void tearDown(void) {
  if (cli) {
    waveform_cli_destroy(cli);
    cli = NULL;
  }
  viewer = NULL;
}

void test_cli_creation(void) {
  TEST_ASSERT_NOT_NULL(cli);
  TEST_ASSERT_TRUE(cli->running);
  TEST_ASSERT_TRUE(cli->echo_enabled);
  TEST_ASSERT_EQUAL_UINT32(0, cli->buffer_pos);
  TEST_ASSERT_EQUAL_STRING("", cli->command_buffer);
}

void test_cli_process_char(void) {
  // Testa entrada de caracteres
  TEST_ASSERT_TRUE(waveform_cli_process_char(cli, 'h'));
  TEST_ASSERT_TRUE(waveform_cli_process_char(cli, 'e'));
  TEST_ASSERT_TRUE(waveform_cli_process_char(cli, 'l'));
  TEST_ASSERT_TRUE(waveform_cli_process_char(cli, 'p'));
  TEST_ASSERT_EQUAL_UINT32(4, cli->buffer_pos);
  TEST_ASSERT_EQUAL_STRING_LEN("help", cli->command_buffer, 4);

  // Testa backspace
  TEST_ASSERT_TRUE(waveform_cli_process_char(cli, '\b'));
  TEST_ASSERT_EQUAL_UINT32(3, cli->buffer_pos);
  TEST_ASSERT_EQUAL_STRING_LEN("hel", cli->command_buffer, 3);

  // Testa execução do comando
  TEST_ASSERT_TRUE(waveform_cli_process_char(cli, '\n'));
  TEST_ASSERT_EQUAL_UINT32(0, cli->buffer_pos);
}

void test_cli_execute_command(void) {
  // Testa comando help
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "help"));

  // Testa comando quit
  TEST_ASSERT_FALSE(waveform_cli_execute_command(cli, "quit"));
  TEST_ASSERT_FALSE(cli->running);

  // Testa comando inválido
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "invalid"));
}

void test_cli_channel_command(void) {
  // Testa ativação de canal
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "channel pulse1 on"));
  TEST_ASSERT_TRUE(mock_config.channel_enabled[WAVEFORM_CHANNEL_PULSE1]);

  // Testa desativação de canal
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "channel pulse1 off"));
  TEST_ASSERT_FALSE(mock_config.channel_enabled[WAVEFORM_CHANNEL_PULSE1]);

  // Testa canal inválido
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "channel invalid on"));
}

void test_cli_zoom_command(void) {
  // Testa zoom válido
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "zoom 5"));
  TEST_ASSERT_EQUAL_INT(5, mock_config.zoom_level);

  // Testa zoom inválido
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "zoom 11"));
  TEST_ASSERT_EQUAL_INT(5, mock_config.zoom_level);
}

void test_cli_scroll_command(void) {
  // Testa ativação de rolagem
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "scroll on"));
  TEST_ASSERT_TRUE(mock_config.auto_scroll);

  // Testa desativação de rolagem
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "scroll off"));
  TEST_ASSERT_FALSE(mock_config.auto_scroll);
}

void test_cli_grid_command(void) {
  // Testa ativação da grade
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "grid on"));
  TEST_ASSERT_TRUE(mock_config.show_grid);

  // Testa desativação da grade
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "grid off"));
  TEST_ASSERT_FALSE(mock_config.show_grid);
}

void test_cli_peaks_command(void) {
  // Testa ativação dos picos
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "peaks on"));
  TEST_ASSERT_TRUE(mock_config.show_peaks);

  // Testa desativação dos picos
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "peaks off"));
  TEST_ASSERT_FALSE(mock_config.show_peaks);
}

void test_cli_echo_command(void) {
  // Testa ativação do eco
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "echo on"));
  TEST_ASSERT_TRUE(cli->echo_enabled);

  // Testa desativação do eco
  TEST_ASSERT_TRUE(waveform_cli_execute_command(cli, "echo off"));
  TEST_ASSERT_FALSE(cli->echo_enabled);
}

void test_cli_buffer_overflow(void) {
  char long_command[300];
  memset(long_command, 'a', sizeof(long_command) - 1);
  long_command[sizeof(long_command) - 1] = '\0';

  // Tenta processar um comando muito longo
  for (size_t i = 0; i < strlen(long_command); i++) {
    TEST_ASSERT_TRUE(waveform_cli_process_char(cli, long_command[i]));
  }

  // Verifica se o buffer não estourou
  TEST_ASSERT_LESS_OR_EQUAL_UINT32(sizeof(cli->command_buffer) - 1,
                                   cli->buffer_pos);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_cli_creation);
  RUN_TEST(test_cli_process_char);
  RUN_TEST(test_cli_execute_command);
  RUN_TEST(test_cli_channel_command);
  RUN_TEST(test_cli_zoom_command);
  RUN_TEST(test_cli_scroll_command);
  RUN_TEST(test_cli_grid_command);
  RUN_TEST(test_cli_peaks_command);
  RUN_TEST(test_cli_echo_command);
  RUN_TEST(test_cli_buffer_overflow);

  return UNITY_END();
}
