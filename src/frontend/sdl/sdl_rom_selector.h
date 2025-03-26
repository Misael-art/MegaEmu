/**
 * @file sdl_rom_selector.h
 * @brief Seletor de ROMs para o frontend SDL
 */
#ifndef SDL_ROM_SELECTOR_H
#define SDL_ROM_SELECTOR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "frontend/console_types.h"
#include "sdl_game_renderer.h"

// Limites do seletor de ROMs
#define SDL_ROM_MAX_PATH 1024
#define SDL_ROM_MAX_NAME 256
#define SDL_ROM_MAX_ITEMS 1000
#define SDL_ROM_MAX_EXTENSIONS 10
#define SDL_ROM_ITEMS_PER_PAGE 12
#define SDL_ROM_THUMBNAIL_SIZE 96

// Tipos de organização
typedef enum {
  SDL_ROM_VIEW_ALL,
  SDL_ROM_VIEW_FAVORITES,
  SDL_ROM_VIEW_RECENT,
  SDL_ROM_VIEW_BY_SYSTEM
} sdl_rom_view_type_t;

// Metadados de ROM
typedef struct {
  char path[SDL_ROM_MAX_PATH];           // Caminho completo para a ROM
  char name[SDL_ROM_MAX_NAME];           // Nome da ROM (sem extensão)
  char display_name[SDL_ROM_MAX_NAME];   // Nome para exibição (pode incluir
                                         // metadados)
  console_type_t system;                 // Sistema desta ROM
  uint64_t size;                         // Tamanho em bytes
  time_t last_played;                    // Última vez jogada
  time_t added_date;                     // Data em que foi adicionada
  bool favorite;                         // Se é favorito
  SDL_Texture *thumbnail;                // Thumbnail da ROM
  char thumbnail_path[SDL_ROM_MAX_PATH]; // Caminho para o thumbnail
  int play_count;                        // Número de vezes jogada
} sdl_rom_item_t;

// Filtro de ROMs
typedef struct {
  char text[SDL_ROM_MAX_NAME]; // Texto de busca
  console_type_t system;       // Filtrar por sistema
  sdl_rom_view_type_t view;    // Tipo de visualização
  bool show_all_systems;       // Mostrar todos os sistemas
} sdl_rom_filter_t;

// Estado do seletor de ROMs
typedef struct {
  bool visible;                                 // Se está visível
  sdl_rom_item_t items[SDL_ROM_MAX_ITEMS];      // Itens (ROMs)
  uint32_t item_count;                          // Total de itens
  uint32_t filtered_indices[SDL_ROM_MAX_ITEMS]; // Índices após filtragem
  uint32_t filtered_count;                      // Total após filtragem
  int selected_index;                           // Índice selecionado atual
  int scroll_position;                          // Posição de scroll
  sdl_rom_filter_t filter;                      // Filtro atual
  SDL_Rect viewport;                            // Retângulo de visualização

  // Recursos visuais
  TTF_Font *title_font;                     // Fonte para títulos
  TTF_Font *item_font;                      // Fonte para itens
  SDL_Texture *background_texture;          // Textura de fundo
  SDL_Texture *highlight_texture;           // Textura de destaque
  SDL_Texture *thumbnail_placeholder;       // Textura padrão para thumbnails
  SDL_Texture *system_icons[CONSOLE_COUNT]; // Ícones para cada sistema

  // Renderizador
  sdl_game_renderer_t *renderer; // Renderizador

  // Dados de sistema
  char rom_dirs[CONSOLE_COUNT]
               [SDL_ROM_MAX_PATH]; // Diretórios de ROMs por sistema
  char extensions[CONSOLE_COUNT][SDL_ROM_MAX_EXTENSIONS]
                 [16];                    // Extensões por sistema
  uint8_t extension_count[CONSOLE_COUNT]; // Número de extensões por sistema

  // Callbacks
  void (*on_rom_selected)(const char *path, console_type_t system,
                          void *userdata);
  void (*on_cancel)(void *userdata);
  void *userdata;
} sdl_rom_selector_t;

// Funções de inicialização e finalização
bool sdl_rom_selector_init(sdl_rom_selector_t *selector,
                           sdl_game_renderer_t *renderer);
void sdl_rom_selector_shutdown(sdl_rom_selector_t *selector);

// Funções de gerenciamento de ROMs
bool sdl_rom_selector_scan_directories(sdl_rom_selector_t *selector);
bool sdl_rom_selector_add_rom(sdl_rom_selector_t *selector, const char *path,
                              console_type_t system);
bool sdl_rom_selector_remove_rom(sdl_rom_selector_t *selector,
                                 const char *path);
void sdl_rom_selector_clear_roms(sdl_rom_selector_t *selector);
bool sdl_rom_selector_load_thumbnails(sdl_rom_selector_t *selector);

// Funções de favoritos e recentes
bool sdl_rom_selector_toggle_favorite(sdl_rom_selector_t *selector, int index);
bool sdl_rom_selector_update_last_played(sdl_rom_selector_t *selector,
                                         int index);
bool sdl_rom_selector_save_metadata(sdl_rom_selector_t *selector);
bool sdl_rom_selector_load_metadata(sdl_rom_selector_t *selector);

// Funções de filtragem e ordenação
void sdl_rom_selector_apply_filter(sdl_rom_selector_t *selector);
void sdl_rom_selector_set_view(sdl_rom_selector_t *selector,
                               sdl_rom_view_type_t view);
void sdl_rom_selector_set_system_filter(sdl_rom_selector_t *selector,
                                        console_type_t system);
void sdl_rom_selector_set_search_text(sdl_rom_selector_t *selector,
                                      const char *text);

// Funções de navegação e seleção
void sdl_rom_selector_select_item(sdl_rom_selector_t *selector, int index);
void sdl_rom_selector_select_next(sdl_rom_selector_t *selector);
void sdl_rom_selector_select_prev(sdl_rom_selector_t *selector);
void sdl_rom_selector_page_down(sdl_rom_selector_t *selector);
void sdl_rom_selector_page_up(sdl_rom_selector_t *selector);
const sdl_rom_item_t *
sdl_rom_selector_get_selected(const sdl_rom_selector_t *selector);

// Funções de interface
void sdl_rom_selector_show(sdl_rom_selector_t *selector);
void sdl_rom_selector_hide(sdl_rom_selector_t *selector);
bool sdl_rom_selector_is_visible(const sdl_rom_selector_t *selector);
void sdl_rom_selector_set_viewport(sdl_rom_selector_t *selector,
                                   SDL_Rect viewport);
void sdl_rom_selector_set_callbacks(
    sdl_rom_selector_t *selector,
    void (*on_rom_selected)(const char *path, console_type_t system,
                            void *userdata),
    void (*on_cancel)(void *userdata), void *userdata);

// Funções de renderização e eventos
void sdl_rom_selector_render(sdl_rom_selector_t *selector);
bool sdl_rom_selector_handle_event(sdl_rom_selector_t *selector,
                                   SDL_Event *event);

#endif /* SDL_ROM_SELECTOR_H */
