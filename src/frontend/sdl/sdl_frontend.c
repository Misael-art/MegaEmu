/**
 * @file sdl_frontend.c
 * @brief Implementação do frontend baseado em SDL (compatível com SDL2 e SDL3)
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "core/core.h"
#include "frontend/common/frontend.h"
#include "frontend/common/frontend_config.h"
#include "frontend/sdl/sdl_frontend.h"
#include "frontend/sdl/sdl_frontend_state.h"
#include "frontend/sdl/sdl_game_renderer.h"
#include "frontend/sdl/sdl_menu.h"
#include "utils/enhanced_log.h"
#include "utils/log_categories.h"
#include "utils/log_modules.h"
#include "utils/log_utils.h"

// Definições para compatibilidade SDL2/SDL3
#define SDL_MAIN_HANDLED

// Estado global do frontend
static sdl_frontend_state_t g_state;

// Tentar carregar SDL3 primeiro, se não disponível, tenta SDL2
#ifdef _WIN32
#define SDL3_DLL_NAME "SDL3.dll"
#define SDL2_DLL_NAME "SDL2.dll"

// Flag global para indicar qual versão do SDL está sendo usada
static int g_using_sdl3 = 0;

/**
 * @brief Verifica se a DLL do SDL está disponível
 *
 * @return 1 se disponível, 0 caso contrário
 */
int check_sdl_dll(void) {
  HMODULE hModule = LoadLibraryA(SDL3_DLL_NAME);
  if (hModule != NULL) {
    printf("SDL3.dll carregada com sucesso: %p\n", hModule);
    FreeLibrary(hModule);
    g_using_sdl3 = 1;
    return 1;
  }

  // Tentar carregar SDL2 como fallback
  hModule = LoadLibraryA(SDL2_DLL_NAME);
  if (hModule != NULL) {
    printf("SDL2.dll carregada com sucesso: %p\n", hModule);
    FreeLibrary(hModule);
    g_using_sdl3 = 0;
    return 1;
  }

  DWORD error = GetLastError();
  printf("Erro ao carregar SDL.dll: %lu\n", error);

  // Verificar se os arquivos existem
  FILE *f = fopen(SDL3_DLL_NAME, "rb");
  if (f) {
    printf("Arquivo SDL3.dll existe, mas não pode ser carregado\n");
    fclose(f);
  } else {
    printf("Arquivo SDL3.dll não encontrado\n");
  }

  f = fopen(SDL2_DLL_NAME, "rb");
  if (f) {
    printf("Arquivo SDL2.dll existe, mas não pode ser carregado\n");
    fclose(f);
  } else {
    printf("Arquivo SDL2.dll não encontrado\n");
  }

  // Verificar diretório atual
  char current_dir[MAX_PATH];
  GetCurrentDirectoryA(MAX_PATH, current_dir);
  printf("Diretório atual: %s\n", current_dir);
  return 0;
}
#else
int check_sdl_dll(void) {
  return 1; // Assume success on non-Windows platforms
}
#endif

// Definir macro UNUSED se não estiver definida
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

// Macros de log para o frontend
#define FRONTEND_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define FRONTEND_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define FRONTEND_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define FRONTEND_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)

// Referência para a configuração padrão do SDL
extern const emu_frontend_config_t SDL_DEFAULT_FRONTEND_CONFIG;

// Menus do sistema
static sdl_menu_t *g_main_menu = NULL;
static sdl_menu_t *g_video_menu = NULL;
static sdl_menu_t *g_audio_menu = NULL;
static sdl_menu_t *g_input_menu = NULL;

// Callbacks para os itens de menu
static void toggle_fullscreen_callback(bool value, void *userdata) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;
  state->config.fullscreen = value;
  sdl_game_renderer_toggle_fullscreen(&state->renderer);
}

static void set_scale_factor_callback(int value, void *userdata) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;
  state->config.scale_factor = (float)value / 10.0f;
  sdl_game_renderer_set_scale(&state->renderer, state->config.scale_factor);
}

static void set_integer_scaling_callback(bool value, void *userdata) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;
  state->config.integer_scaling = value;
  sdl_game_renderer_set_integer_scaling(&state->renderer, value);
}

static void set_smooth_scaling_callback(bool value, void *userdata) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;
  state->config.smooth_scaling = value;
  sdl_game_renderer_set_smooth_scaling(&state->renderer, value);
}

static void set_audio_enabled_callback(bool value, void *userdata) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;
  state->config.audio_enabled = value;

  if (state->audio_device) {
    SDL_PauseAudioDevice(state->audio_device, !value);
  }
}

static void set_show_fps_callback(bool value, void *userdata) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;
  state->show_fps = value;
}

static void exit_callback(void *userdata) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;
  state->running = false;
}

// Função para criar os menus
static bool create_menus(void) {
  // Criar contexto de menu
  if (!sdl_menu_init(&g_state.menu_context, &g_state.renderer)) {
    FRONTEND_LOG_ERROR("Erro ao inicializar contexto de menu");
    return false;
  }

  // Definir userdata para callbacks
  g_state.menu_context.userdata = &g_state;

  // Menu principal
  g_main_menu = sdl_menu_create("Menu Principal", NULL);
  if (!g_main_menu) {
    FRONTEND_LOG_ERROR("Erro ao criar menu principal");
    return false;
  }

  // Menu de vídeo
  g_video_menu = sdl_menu_create("Configurações de Vídeo", g_main_menu);
  if (!g_video_menu) {
    FRONTEND_LOG_ERROR("Erro ao criar menu de vídeo");
    sdl_menu_destroy(g_main_menu);
    return false;
  }

  // Menu de áudio
  g_audio_menu = sdl_menu_create("Configurações de Áudio", g_main_menu);
  if (!g_audio_menu) {
    FRONTEND_LOG_ERROR("Erro ao criar menu de áudio");
    sdl_menu_destroy(g_video_menu);
    sdl_menu_destroy(g_main_menu);
    return false;
  }

  // Menu de controles
  g_input_menu = sdl_menu_create("Configurações de Controle", g_main_menu);
  if (!g_input_menu) {
    FRONTEND_LOG_ERROR("Erro ao criar menu de controle");
    sdl_menu_destroy(g_audio_menu);
    sdl_menu_destroy(g_video_menu);
    sdl_menu_destroy(g_main_menu);
    return false;
  }

  // Adicionar itens ao menu principal
  sdl_menu_add_submenu(g_main_menu, "video_menu", "Vídeo", g_video_menu);
  sdl_menu_add_submenu(g_main_menu, "audio_menu", "Áudio", g_audio_menu);
  sdl_menu_add_submenu(g_main_menu, "input_menu", "Controles", g_input_menu);
  sdl_menu_add_separator(g_main_menu);
  sdl_menu_add_toggle(g_main_menu, "show_fps", "Mostrar FPS", g_state.show_fps,
                      set_show_fps_callback);
  sdl_menu_add_separator(g_main_menu);
  sdl_menu_add_action(g_main_menu, "exit", "Sair", exit_callback);

  // Adicionar itens ao menu de vídeo
  sdl_menu_add_toggle(g_video_menu, "fullscreen", "Tela Cheia",
                      g_state.config.fullscreen, toggle_fullscreen_callback);
  sdl_menu_add_toggle(g_video_menu, "integer_scaling", "Escala Inteira",
                      g_state.config.integer_scaling,
                      set_integer_scaling_callback);
  sdl_menu_add_toggle(g_video_menu, "smooth_scaling", "Escala Suave",
                      g_state.config.smooth_scaling,
                      set_smooth_scaling_callback);

  // Adicionar slider para fator de escala
  int scale_value = (int)(g_state.config.scale_factor * 10.0f);
  sdl_menu_add_slider(g_video_menu, "scale_factor", "Fator de Escala", 10, 50,
                      scale_value, 5, set_scale_factor_callback);

  // Adicionar itens ao menu de áudio
  sdl_menu_add_toggle(g_audio_menu, "audio_enabled", "Áudio Ativado",
                      g_state.config.audio_enabled, set_audio_enabled_callback);

  // Definir menu principal como ativo
  sdl_menu_navigate_to(&g_state.menu_context, g_main_menu);

  return true;
}

// Função para destruir os menus
static void destroy_menus(void) {
  // Destruir menus em ordem inversa à criação
  if (g_input_menu) {
    sdl_menu_destroy(g_input_menu);
    g_input_menu = NULL;
  }

  if (g_audio_menu) {
    sdl_menu_destroy(g_audio_menu);
    g_audio_menu = NULL;
  }

  if (g_video_menu) {
    sdl_menu_destroy(g_video_menu);
    g_video_menu = NULL;
  }

  if (g_main_menu) {
    sdl_menu_destroy(g_main_menu);
    g_main_menu = NULL;
  }

  // Finalizar contexto de menu
  sdl_menu_shutdown(&g_state.menu_context);
}

// Callback de áudio
static void audio_callback(void *userdata, Uint8 *stream, int len) {
  sdl_frontend_state_t *state = (sdl_frontend_state_t *)userdata;

  // Limpar buffer de áudio com silêncio
  SDL_memset(stream, 0, len);

  // Verificar se há amostras disponíveis
  if (state->audio_buffer_size > 0) {
    // Calcular quantos bytes copiar (não mais que o disponível)
    int bytes_to_copy =
        (state->audio_buffer_size < len) ? state->audio_buffer_size : len;

    // Copiar dados do buffer para o stream
    SDL_MixAudioFormat(stream, state->audio_buffer, AUDIO_S16, bytes_to_copy,
                       SDL_MIX_MAXVOLUME);

    // Se ainda há dados no buffer, mover para o início
    if (state->audio_buffer_size > bytes_to_copy) {
      memmove(state->audio_buffer, state->audio_buffer + bytes_to_copy,
              state->audio_buffer_size - bytes_to_copy);
    }

    state->audio_buffer_size -= bytes_to_copy;
  }
}

/**
 * @brief Mostra uma mensagem de erro na linha de comando quando SDL não for
 * encontrado
 */
void show_sdl_missing_error(void) {
  printf("\n\n");
  printf("*** ERRO: Biblioteca SDL não encontrada ou não está acessível ***\n");
  printf("Para corrigir este problema:\n");
  printf(
      "1. Certifique-se de que o SDL3 ou SDL2 está instalado no seu sistema\n");
  printf("2. Copie o arquivo SDL3.dll ou SDL2.dll para o diretório do "
         "executável\n");
  printf("\n");
  printf("O emulador continuará em modo de texto sem interface gráfica.\n");
  printf("\n\n");
}

/**
 * @brief Sistema de log robusto para o SDL
 *
 * @param userdata Dados do usuário (não utilizado)
 * @param categoria Categoria da mensagem
 * @param prioridade Prioridade da mensagem (debug, info, warning, error)
 * @param mensagem A mensagem de log
 */
void sdl_log_callback(void *userdata, int categoria, SDL_LogPriority prioridade,
                      const char *mensagem) {
  UNUSED(userdata);
  emu_log_level_t level;
  emu_log_category_t cat;
  // Mapear prioridade SDL para nível de log
  switch (prioridade) {
  case SDL_LOG_PRIORITY_VERBOSE:
    level = EMU_LOG_LEVEL_DEBUG;
    break;
  case SDL_LOG_PRIORITY_DEBUG:
    level = EMU_LOG_LEVEL_DEBUG;
    break;
  case SDL_LOG_PRIORITY_INFO:
    level = EMU_LOG_LEVEL_INFO;
    break;
  case SDL_LOG_PRIORITY_WARN:
    level = EMU_LOG_LEVEL_WARNING;
    break;
  case SDL_LOG_PRIORITY_ERROR:
    level = EMU_LOG_LEVEL_ERROR;
    break;
  case SDL_LOG_PRIORITY_CRITICAL:
    level = EMU_LOG_LEVEL_ERROR;
    break;
  default:
    level = EMU_LOG_LEVEL_INFO;
  }
  // Mapear categoria SDL para categoria de log
  switch (categoria) {
  case SDL_LOG_CATEGORY_APPLICATION:
    cat = EMU_LOG_CAT_CORE;
    break;
  case SDL_LOG_CATEGORY_ERROR:
    cat = EMU_LOG_CAT_CORE;
    break;
  case SDL_LOG_CATEGORY_ASSERT:
    cat = EMU_LOG_CAT_CORE;
    break;
  case SDL_LOG_CATEGORY_SYSTEM:
    cat = EMU_LOG_CAT_CORE;
    break;
  case SDL_LOG_CATEGORY_AUDIO:
    cat = EMU_LOG_CAT_AUDIO;
    break;
  case SDL_LOG_CATEGORY_VIDEO:
    cat = EMU_LOG_CAT_VIDEO;
    break;
  case SDL_LOG_CATEGORY_RENDER:
    cat = EMU_LOG_CAT_VIDEO;
    break;
  case SDL_LOG_CATEGORY_INPUT:
    cat = EMU_LOG_CAT_INPUT;
    break;
  case SDL_LOG_CATEGORY_TEST:
    cat = EMU_LOG_CAT_CORE;
    break;
  default:
    cat = EMU_LOG_CAT_CORE;
  }
  emu_log_message(level, cat, __FILE__, __LINE__, "%s", mensagem);
}

// Funções de inicialização e finalização
bool sdl_frontend_init(const emu_frontend_config_t *config) {
  // Inicializar o estado global
  memset(&g_state, 0, sizeof(sdl_frontend_state_t));
  FRONTEND_LOG_INFO("Inicializando frontend SDL");
  printf("Inicializando frontend SDL...\n");
  // Verificar DLL do SDL
  if (!check_sdl_dll()) {
    show_sdl_missing_error();
    return false;
  }
  // Usar configuração padrão se não fornecida
  if (config) {
    memcpy(&g_state.config, config, sizeof(emu_frontend_config_t));
  } else {
    memcpy(&g_state.config, &SDL_DEFAULT_FRONTEND_CONFIG,
           sizeof(emu_frontend_config_t));
  }
  // Configurar callback de log do SDL
  SDL_LogSetOutputFunction(sdl_log_callback, NULL);
  // Inicializar SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
    FRONTEND_LOG_ERROR("Erro ao inicializar SDL: %s", SDL_GetError());
    return false;
  }
  // Inicializar renderizador
  sdl_renderer_config_t renderer_config = {
      .window_width = g_state.config.window_width,
      .window_height = g_state.config.window_height,
      .game_width = g_state.config.game_width,
      .game_height = g_state.config.game_height,
      .scale_factor = g_state.config.scale_factor,
      .vsync_enabled = g_state.config.vsync_enabled,
      .fullscreen = g_state.config.fullscreen,
      .smooth_scaling = g_state.config.smooth_scaling,
      .integer_scaling = g_state.config.integer_scaling};
  if (!sdl_game_renderer_init(&g_state.renderer, &renderer_config)) {
    FRONTEND_LOG_ERROR("Erro ao inicializar renderizador");
    SDL_Quit();
    return false;
  }
  // Alocar buffer de áudio com tamanho adequado
  g_state.audio_buffer =
      (Uint8 *)malloc(g_state.config.audio_buffer_size *
                      4); // Buffer maior para evitar underruns
  if (!g_state.audio_buffer) {
    FRONTEND_LOG_ERROR("Erro ao alocar buffer de áudio");
    sdl_game_renderer_shutdown(&g_state.renderer);
    SDL_Quit();
    return false;
  }
  g_state.audio_buffer_size = 0; // Inicialmente vazio
  g_state.audio_buffer_capacity = g_state.config.audio_buffer_size * 4;
  // Configurar áudio
  SDL_AudioSpec want, have;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = g_state.config.audio_sample_rate;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = g_state.config.audio_buffer_size;
  want.callback = audio_callback;
  want.userdata = &g_state;
  g_state.audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (g_state.audio_device == 0) {
    FRONTEND_LOG_ERROR("Erro ao abrir dispositivo de áudio: %s",
                       SDL_GetError());
    free(g_state.audio_buffer);
    sdl_game_renderer_shutdown(&g_state.renderer);
    SDL_Quit();
    return false;
  }
  // Armazenar as especificações de áudio obtidas
  g_state.audio_spec = have;
  g_state.audio_conversion_needed =
      (have.format != want.format || have.freq != want.freq ||
       have.channels != want.channels);
  FRONTEND_LOG_INFO(
      "Áudio inicializado: %d Hz, %d canais, formato %d, buffer %d amostras",
      have.freq, have.channels, have.format, have.samples);
  SDL_PauseAudioDevice(g_state.audio_device, 0);
  // Inicializar controles
  SDL_GameControllerEventState(SDL_ENABLE);
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController(i)) {
      g_state.gamepad = SDL_GameControllerOpen(i);
      if (g_state.gamepad) {
        FRONTEND_LOG_INFO("Controle conectado: %s",
                          SDL_GameControllerName(g_state.gamepad));
        break;
      }
    }
  }
  // Inicializar sistema de menus
  if (!create_menus()) {
    FRONTEND_LOG_ERROR("Erro ao criar menus");
    // Continuar inicialização mesmo sem menus
  }

  g_state.running = true;
  g_state.show_fps = true;
  g_state.show_menu = false;

  return true;
}

void sdl_frontend_shutdown(void) {
  // Destruir menus
  destroy_menus();

  if (g_state.gamepad) {
    SDL_GameControllerClose(g_state.gamepad);
    g_state.gamepad = NULL;
  }
  if (g_state.audio_device) {
    SDL_CloseAudioDevice(g_state.audio_device);
    g_state.audio_device = 0;
  }
  if (g_state.audio_buffer) {
    free(g_state.audio_buffer);
    g_state.audio_buffer = NULL;
  }
  sdl_game_renderer_shutdown(&g_state.renderer);
  SDL_Quit();
}

// Funções de controle
bool sdl_frontend_process_events(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    // Processar eventos do menu se estiver visível
    if (g_state.show_menu && sdl_menu_is_visible(&g_state.menu_context)) {
      if (sdl_menu_process_event(&g_state.menu_context, &event)) {
        // Evento processado pelo menu
        continue;
      }
    }

    switch (event.type) {
    case SDL_QUIT:
      g_state.running = false;
      break;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_ESCAPE) {
        if (g_state.show_menu) {
          // Se o menu estiver visível, esconder
          g_state.show_menu = false;
        } else {
          // Se o menu não estiver visível, mostrar ou sair
          g_state.show_menu = true;
        }
      } else if (event.key.keysym.sym == SDLK_F11) {
        g_state.show_fps = !g_state.show_fps;
      } else if (event.key.keysym.sym == SDLK_F12) {
        g_state.show_menu = !g_state.show_menu;
        // Atualizar visibilidade do menu
        sdl_menu_set_visible(&g_state.menu_context, g_state.show_menu);
      }
      break;
    case SDL_CONTROLLERDEVICEADDED:
      if (!g_state.gamepad) {
        g_state.gamepad = SDL_GameControllerOpen(event.cdevice.which);
        if (g_state.gamepad) {
          FRONTEND_LOG_INFO("Controle conectado: %s",
                            SDL_GameControllerName(g_state.gamepad));
        }
      }
      break;
    case SDL_CONTROLLERDEVICEREMOVED:
      if (g_state.gamepad &&
          event.cdevice.which ==
              SDL_JoystickInstanceID(
                  SDL_GameControllerGetJoystick(g_state.gamepad))) {
        SDL_GameControllerClose(g_state.gamepad);
        g_state.gamepad = NULL;
        FRONTEND_LOG_INFO("Controle desconectado");
      }
      break;
    }
  }

  return g_state.running;
}

bool sdl_frontend_process_audio(const int16_t *audio_samples, int num_samples) {
  if (!g_state.running || !g_state.config.audio_enabled || !audio_samples ||
      num_samples <= 0)
    return false;

  SDL_LockAudioDevice(g_state.audio_device);

  // Verificar se há espaço suficiente no buffer
  int bytes_needed = num_samples * sizeof(int16_t) * 2; // Stereo, 16 bits

  if (g_state.audio_buffer_size + bytes_needed >
      g_state.audio_buffer_capacity) {
    // Buffer cheio, redimensionar ou descartar amostras antigas
    if (bytes_needed > g_state.audio_buffer_capacity) {
      // Caso extremo: amostra maior que o buffer total
      FRONTEND_LOG_WARN(
          "Amostra de áudio muito grande (%d bytes), redimensionando buffer",
          bytes_needed);
      Uint8 *new_buffer =
          (Uint8 *)realloc(g_state.audio_buffer, bytes_needed * 2);
      if (new_buffer) {
        g_state.audio_buffer = new_buffer;
        g_state.audio_buffer_capacity = bytes_needed * 2;
        g_state.audio_buffer_size = 0; // Descartar amostras antigas
      } else {
        // Falha ao redimensionar, descartar amostras antigas
        g_state.audio_buffer_size = 0;
      }
    } else {
      // Descartar amostras antigas para abrir espaço
      int bytes_to_keep = g_state.audio_buffer_capacity - bytes_needed;
      memmove(g_state.audio_buffer,
              g_state.audio_buffer +
                  (g_state.audio_buffer_size - bytes_to_keep),
              bytes_to_keep);
      g_state.audio_buffer_size = bytes_to_keep;
    }
  }

  // Agora copiar as novas amostras para o buffer
  if (g_state.audio_conversion_needed) {
    // Aplicar conversão se necessário (simplificado - aqui você expandiria com
    // a conversão real) Para uma implementação completa, usaria SDL_AudioCVT
    memcpy(g_state.audio_buffer + g_state.audio_buffer_size, audio_samples,
           bytes_needed);
  } else {
    // Cópia direta
    memcpy(g_state.audio_buffer + g_state.audio_buffer_size, audio_samples,
           bytes_needed);
  }

  g_state.audio_buffer_size += bytes_needed;

  SDL_UnlockAudioDevice(g_state.audio_device);

  return true;
}

void sdl_frontend_render_frame(const uint32_t *framebuffer,
                               const int16_t *audio_samples, int num_samples) {
  if (!g_state.running)
    return;

  // Atualizar áudio
  if (audio_samples && num_samples > 0) {
    sdl_frontend_process_audio(audio_samples, num_samples);
  }

  // Renderizar frame
  sdl_game_renderer_begin_frame(&g_state.renderer);

  if (framebuffer) {
    sdl_game_renderer_update_game_texture(&g_state.renderer, framebuffer);
    sdl_game_renderer_draw_frame(&g_state.renderer);
  }

  // Renderizar menu se estiver visível
  if (g_state.show_menu) {
    sdl_menu_render(&g_state.menu_context);
  }

  sdl_game_renderer_end_frame(&g_state.renderer);

  // Calcular FPS
  Uint32 current_time = SDL_GetTicks();
  if (current_time > g_state.last_fps_update + 1000) {
    g_state.fps = g_state.frames_since_last_fps;
    g_state.frames_since_last_fps = 0;
    g_state.last_fps_update = current_time;
  }
  g_state.frames_since_last_fps++;
}

bool sdl_frontend_is_running(void) { return g_state.running; }

uint8_t sdl_frontend_get_controller_state(int controller) {
  if (controller < 0 || controller >= 4)
    return 0;
  return g_state.controller_states[controller];
}

void sdl_frontend_toggle_fullscreen(void) {
  sdl_game_renderer_toggle_fullscreen(&g_state.renderer);
}

void sdl_frontend_set_title(const char *title) {
  SDL_SetWindowTitle(SDL_RenderGetWindow(g_state.renderer.renderer), title);
}

float sdl_frontend_get_fps(void) { return g_state.fps; }
