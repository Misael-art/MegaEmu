/**
 * @file waveform_interface.c
 * @brief Implementação da interface principal do visualizador
 */

#include "waveform_interface.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Obtém o timestamp atual em milissegundos
static uint32_t get_timestamp_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

waveform_interface_t *waveform_interface_create(uint32_t width, uint32_t height,
                                                const char *title) {
  // Aloca a estrutura da interface
  waveform_interface_t *interface = calloc(1, sizeof(waveform_interface_t));
  if (!interface)
    return NULL;

  // Inicializa o subsistema de exibição
  if (!waveform_display_init()) {
    free(interface);
    return NULL;
  }

  // Cria o visualizador
  interface->viewer = waveform_create();
  if (!interface->viewer) {
    free(interface);
    waveform_display_quit();
    return NULL;
  }

  // Cria a janela de exibição
  interface->display = waveform_display_create(width, height, title);
  if (!interface->display) {
    waveform_destroy(interface->viewer);
    free(interface);
    waveform_display_quit();
    return NULL;
  }

  // Cria a interface de linha de comando
  interface->cli = waveform_cli_create(interface->viewer);
  if (!interface->cli) {
    waveform_display_destroy(interface->display);
    waveform_destroy(interface->viewer);
    free(interface);
    waveform_display_quit();
    return NULL;
  }

  // Inicializa os demais campos
  interface->running = true;
  interface->paused = false;
  interface->frame_count = 0;
  interface->last_update_time = get_timestamp_ms();

  return interface;
}

void waveform_interface_destroy(waveform_interface_t *interface) {
  if (!interface)
    return;

  if (interface->cli) {
    waveform_cli_destroy(interface->cli);
  }
  if (interface->display) {
    waveform_display_destroy(interface->display);
  }
  if (interface->viewer) {
    waveform_destroy(interface->viewer);
  }
  waveform_display_quit();
  free(interface);
}

bool waveform_interface_update(waveform_interface_t *interface) {
  if (!interface)
    return false;

  // Processa eventos da janela
  if (!waveform_display_process_events(interface->display)) {
    interface->running = false;
    return false;
  }

  // Se estiver pausado, não atualiza o visualizador
  if (!interface->paused) {
    // Atualiza o visualizador
    waveform_update(interface->viewer);

    // Renderiza o frame
    const uint8_t *framebuffer = waveform_render(interface->viewer);
    if (framebuffer) {
      waveform_config_t config = waveform_get_config(interface->viewer);
      waveform_display_update(interface->display, framebuffer,
                              config.window_size, config.window_size);
    }

    // Atualiza contadores
    interface->frame_count++;
    interface->last_update_time = get_timestamp_ms();
  }

  return interface->running;
}

void waveform_interface_process_char(waveform_interface_t *interface, char c) {
  if (!interface)
    return;
  if (!waveform_cli_process_char(interface->cli, c)) {
    interface->running = false;
  }
}

void waveform_interface_set_paused(waveform_interface_t *interface,
                                   bool paused) {
  if (interface) {
    interface->paused = paused;
  }
}

bool waveform_interface_is_paused(const waveform_interface_t *interface) {
  return interface ? interface->paused : false;
}

uint32_t
waveform_interface_get_frame_count(const waveform_interface_t *interface) {
  return interface ? interface->frame_count : 0;
}

uint32_t
waveform_interface_get_last_update_time(const waveform_interface_t *interface) {
  return interface ? interface->last_update_time : 0;
}
