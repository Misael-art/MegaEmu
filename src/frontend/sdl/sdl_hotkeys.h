/**
 * @file sdl_hotkeys.h
 * @brief Sistema de teclas de atalho configuráveis para o frontend SDL
 */
#ifndef SDL_HOTKEYS_H
#define SDL_HOTKEYS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

// Tipos de ações para hotkeys
typedef enum {
  SDL_HOTKEY_ACTION_NONE = 0,
  SDL_HOTKEY_ACTION_TOGGLE_FULLSCREEN,
  SDL_HOTKEY_ACTION_SAVE_STATE,
  SDL_HOTKEY_ACTION_LOAD_STATE,
  SDL_HOTKEY_ACTION_RESET,
  SDL_HOTKEY_ACTION_QUIT,
  SDL_HOTKEY_ACTION_PAUSE,
  SDL_HOTKEY_ACTION_FAST_FORWARD,
  SDL_HOTKEY_ACTION_SLOW_MOTION,
  SDL_HOTKEY_ACTION_SCREENSHOT,
  SDL_HOTKEY_ACTION_RECORD_VIDEO,
  SDL_HOTKEY_ACTION_REWIND,
  SDL_HOTKEY_ACTION_NEXT_SLOT,
  SDL_HOTKEY_ACTION_PREV_SLOT,
  SDL_HOTKEY_ACTION_TOGGLE_SCANLINES,
  SDL_HOTKEY_ACTION_TOGGLE_CRT,
  SDL_HOTKEY_ACTION_MUTE,
  SDL_HOTKEY_ACTION_VOLUME_UP,
  SDL_HOTKEY_ACTION_VOLUME_DOWN,
  SDL_HOTKEY_ACTION_TOGGLE_MENU,
  SDL_HOTKEY_ACTION_TOGGLE_DEBUG_INFO,
  SDL_HOTKEY_ACTION_COUNT
} sdl_hotkey_action_t;

// Estrutura que define uma hotkey
typedef struct {
  SDL_Keycode key;            // Tecla primária
  SDL_Keymod modifiers;       // Modificadores (Ctrl, Alt, Shift)
  sdl_hotkey_action_t action; // Ação associada
  int param;                  // Parâmetro opcional (ex: número do slot)
  bool enabled;               // Se esta hotkey está ativa
  char description[64];       // Descrição da hotkey
} sdl_hotkey_t;

// Estrutura principal para gerenciar hotkeys
typedef struct {
  sdl_hotkey_t hotkeys[SDL_HOTKEY_ACTION_COUNT]; // Lista de hotkeys por ação
  int count;                                     // Número de hotkeys definidas
  bool initialized; // Se o sistema foi inicializado

  // Callbacks de ações
  struct {
    void (*toggle_fullscreen)(void *userdata);
    void (*save_state)(int slot, void *userdata);
    void (*load_state)(int slot, void *userdata);
    void (*reset)(void *userdata);
    void (*quit)(void *userdata);
    void (*pause)(bool state, void *userdata);
    void (*fast_forward)(bool state, void *userdata);
    void (*slow_motion)(bool state, void *userdata);
    void (*screenshot)(void *userdata);
    void (*record_video)(bool state, void *userdata);
    void (*rewind)(bool state, void *userdata);
    void (*next_slot)(void *userdata);
    void (*prev_slot)(void *userdata);
    void (*toggle_scanlines)(void *userdata);
    void (*toggle_crt)(void *userdata);
    void (*mute)(bool state, void *userdata);
    void (*volume_up)(void *userdata);
    void (*volume_down)(void *userdata);
    void (*toggle_menu)(void *userdata);
    void (*toggle_debug_info)(void *userdata);
  } callbacks;

  void *userdata; // Dados do usuário para callbacks
} sdl_hotkeys_t;

// Funções de inicialização e finalização
bool sdl_hotkeys_init(sdl_hotkeys_t *hotkeys);
void sdl_hotkeys_shutdown(sdl_hotkeys_t *hotkeys);

// Funções para configuração de hotkeys
bool sdl_hotkeys_set(sdl_hotkeys_t *hotkeys, sdl_hotkey_action_t action,
                     SDL_Keycode key, SDL_Keymod modifiers, int param);
bool sdl_hotkeys_remove(sdl_hotkeys_t *hotkeys, sdl_hotkey_action_t action);
bool sdl_hotkeys_clear_all(sdl_hotkeys_t *hotkeys);
const sdl_hotkey_t *sdl_hotkeys_get(const sdl_hotkeys_t *hotkeys,
                                    sdl_hotkey_action_t action);

// Funções para gerenciamento de eventos
bool sdl_hotkeys_process_event(sdl_hotkeys_t *hotkeys, SDL_Event *event);
bool sdl_hotkeys_is_action_key(const sdl_hotkeys_t *hotkeys, SDL_Event *event,
                               sdl_hotkey_action_t action);

// Funções para salvar/carregar configurações
bool sdl_hotkeys_save_config(const sdl_hotkeys_t *hotkeys,
                             const char *filepath);
bool sdl_hotkeys_load_config(sdl_hotkeys_t *hotkeys, const char *filepath);
void sdl_hotkeys_reset_to_defaults(sdl_hotkeys_t *hotkeys);

// Funções para gerenciamento de callbacks
void sdl_hotkeys_set_callbacks(
    sdl_hotkeys_t *hotkeys, void (*toggle_fullscreen)(void *userdata),
    void (*save_state)(int slot, void *userdata),
    void (*load_state)(int slot, void *userdata), void (*reset)(void *userdata),
    void (*quit)(void *userdata), void (*pause)(bool state, void *userdata),
    void (*fast_forward)(bool state, void *userdata),
    void (*slow_motion)(bool state, void *userdata),
    void (*screenshot)(void *userdata),
    void (*record_video)(bool state, void *userdata),
    void (*rewind)(bool state, void *userdata),
    void (*next_slot)(void *userdata), void (*prev_slot)(void *userdata),
    void (*toggle_scanlines)(void *userdata),
    void (*toggle_crt)(void *userdata),
    void (*mute)(bool state, void *userdata), void (*volume_up)(void *userdata),
    void (*volume_down)(void *userdata), void (*toggle_menu)(void *userdata),
    void (*toggle_debug_info)(void *userdata), void *userdata);

// Utilitários
const char *sdl_hotkeys_get_action_name(sdl_hotkey_action_t action);
bool sdl_hotkeys_get_key_name(SDL_Keycode key, SDL_Keymod modifiers,
                              char *output, size_t max_length);

#endif /* SDL_HOTKEYS_H */
