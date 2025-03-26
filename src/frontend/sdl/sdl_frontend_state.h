#ifndef SDL_FRONTEND_STATE_H
#define SDL_FRONTEND_STATE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <stdbool.h>
#include <stdint.h>

#include "frontend/common/frontend_config.h"
#include "frontend/sdl/sdl_game_renderer.h"
#include "frontend/sdl/sdl_menu.h"

typedef struct {
  // Configuração
  emu_frontend_config_t config;

  // Renderização
  sdl_game_renderer_t renderer;

  // Áudio
  SDL_AudioDeviceID audio_device;
  SDL_AudioSpec audio_spec;     // Especificações do áudio obtidas
  Uint8 *audio_buffer;          // Buffer de áudio dinâmico
  int audio_buffer_size;        // Tamanho atual do buffer (bytes usados)
  int audio_buffer_capacity;    // Capacidade total do buffer em bytes
  bool audio_conversion_needed; // Indica se conversão é necessária
  float audio_volume;           // Volume do áudio (0.0f - 1.0f)

  // Estado do jogo
  bool running;
  bool paused;
  bool show_menu;
  bool show_fps;

  // Métricas
  float fps;
  uint32_t frames_since_last_fps;
  uint32_t last_fps_update;

  // Controles
  SDL_GameController *gamepad;
  uint8_t controller_states[4];

  // Sistema de menu
  sdl_menu_context_t menu_context;
} sdl_frontend_state_t;

#endif // SDL_FRONTEND_STATE_H
