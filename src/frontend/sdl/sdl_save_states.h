/**
 * @file sdl_save_states.h
 * @brief Gerenciamento de save states para o frontend SDL
 */
#ifndef SDL_SAVE_STATES_H
#define SDL_SAVE_STATES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sdl_game_renderer.h"
#include "utils/save_state.h"

// Constantes para gerenciamento de save states
#define SDL_SAVESTATE_MAX_SLOTS 10
#define SDL_SAVESTATE_MAX_PATH 1024
#define SDL_SAVESTATE_THUMBNAIL_WIDTH 160
#define SDL_SAVESTATE_THUMBNAIL_HEIGHT 120

// Estrutura para um slot de save state
typedef struct {
  bool occupied;                         // Se o slot está ocupado
  int slot_number;                       // Número do slot (0-9)
  char game_id[64];                      // Identificador único do jogo
  char filepath[SDL_SAVESTATE_MAX_PATH]; // Caminho para o arquivo de save state
  time_t timestamp;                      // Timestamp de quando foi criado
  char formatted_time[32];               // Timestamp formatado legível
  SDL_Texture *thumbnail;                // Thumbnail do estado
  char description[128];                 // Descrição opcional
} sdl_save_slot_t;

// Estrutura para gerenciar save states
typedef struct {
  bool visible;                                   // Se está visível
  sdl_save_slot_t slots[SDL_SAVESTATE_MAX_SLOTS]; // Slots disponíveis
  int selected_slot;                              // Slot atualmente selecionado
  int active_slots;                               // Número de slots ocupados
  char current_game_id[64];                       // ID do jogo atual

  // Recursos visuais
  SDL_Rect viewport;               // Área de exibição
  TTF_Font *title_font;            // Fonte para títulos
  TTF_Font *info_font;             // Fonte para informações
  SDL_Texture *background_texture; // Textura de fundo
  SDL_Texture *selected_texture;   // Textura para slot selecionado
  SDL_Texture *empty_slot_texture; // Textura para slot vazio

  // Renderizador
  sdl_game_renderer_t *renderer; // Renderizador SDL

  // Funções de callback
  void (*on_load)(int slot, const char *filepath, void *userdata);
  void (*on_save)(int slot, const char *filepath, void *userdata);
  void (*on_delete)(int slot, const char *filepath, void *userdata);
  void (*on_cancel)(void *userdata);
  void *userdata;
} sdl_save_states_t;

// Funções de inicialização e finalização
bool sdl_save_states_init(sdl_save_states_t *states,
                          sdl_game_renderer_t *renderer);
void sdl_save_states_shutdown(sdl_save_states_t *states);

// Funções para manipulação de slots
bool sdl_save_states_save_to_slot(sdl_save_states_t *states, int slot);
bool sdl_save_states_load_from_slot(sdl_save_states_t *states, int slot);
bool sdl_save_states_delete_slot(sdl_save_states_t *states, int slot);
bool sdl_save_states_create_thumbnail(sdl_save_states_t *states, int slot,
                                      const uint32_t *framebuffer, int width,
                                      int height);
void sdl_save_states_select_slot(sdl_save_states_t *states, int slot);
void sdl_save_states_select_next_slot(sdl_save_states_t *states);
void sdl_save_states_select_prev_slot(sdl_save_states_t *states);
bool sdl_save_states_is_slot_occupied(const sdl_save_states_t *states,
                                      int slot);
const sdl_save_slot_t *sdl_save_states_get_slot(const sdl_save_states_t *states,
                                                int slot);
bool sdl_save_states_set_description(sdl_save_states_t *states, int slot,
                                     const char *description);

// Funções para interface de usuário
void sdl_save_states_show(sdl_save_states_t *states);
void sdl_save_states_hide(sdl_save_states_t *states);
bool sdl_save_states_is_visible(const sdl_save_states_t *states);
void sdl_save_states_set_viewport(sdl_save_states_t *states, SDL_Rect viewport);
void sdl_save_states_set_current_game(sdl_save_states_t *states,
                                      const char *game_id);
void sdl_save_states_set_callbacks(
    sdl_save_states_t *states,
    void (*on_load)(int slot, const char *filepath, void *userdata),
    void (*on_save)(int slot, const char *filepath, void *userdata),
    void (*on_delete)(int slot, const char *filepath, void *userdata),
    void (*on_cancel)(void *userdata), void *userdata);

// Funções de renderização e eventos
void sdl_save_states_render(sdl_save_states_t *states);
bool sdl_save_states_handle_event(sdl_save_states_t *states, SDL_Event *event);

// Funções utilitárias
bool sdl_save_states_refresh_slots(sdl_save_states_t *states);
void sdl_save_states_get_auto_save_path(const char *game_id, char *path,
                                        size_t max_length);

#endif /* SDL_SAVE_STATES_H */
