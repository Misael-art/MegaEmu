#ifndef SDL_MENU_H
#define SDL_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sdl_game_renderer.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

// Tamanho máximo para textos e IDs
#define SDL_MENU_MAX_TEXT_LENGTH 64
#define SDL_MENU_MAX_ITEMS 32
#define SDL_MENU_MAX_DEPTH 8

// Tipos de itens de menu
typedef enum {
  SDL_MENU_ITEM_ACTION,   // Item que executa uma ação
  SDL_MENU_ITEM_TOGGLE,   // Item de alternância (on/off)
  SDL_MENU_ITEM_SLIDER,   // Item com valor ajustável (barra deslizante)
  SDL_MENU_ITEM_CHOICE,   // Item de escolha entre opções
  SDL_MENU_ITEM_SUBMENU,  // Item que abre um submenu
  SDL_MENU_ITEM_SEPARATOR // Separador visual (não selecionável)
} sdl_menu_item_type_t;

// Estrutura para opções de escolha
typedef struct {
  char text[SDL_MENU_MAX_TEXT_LENGTH];
  int value;
} sdl_menu_choice_option_t;

// Estrutura para um item de menu
typedef struct sdl_menu_item {
  char id[SDL_MENU_MAX_TEXT_LENGTH];   // ID único do item
  char text[SDL_MENU_MAX_TEXT_LENGTH]; // Texto exibido
  sdl_menu_item_type_t type;           // Tipo do item
  bool enabled;                        // Se o item está habilitado
  bool visible;                        // Se o item está visível

  // União para dados específicos do tipo
  union {
    // Para SDL_MENU_ITEM_ACTION
    struct {
      void (*callback)(void *userdata);
    } action;

    // Para SDL_MENU_ITEM_TOGGLE
    struct {
      bool value;
      void (*callback)(bool value, void *userdata);
    } toggle;

    // Para SDL_MENU_ITEM_SLIDER
    struct {
      int min_value;
      int max_value;
      int value;
      int step;
      void (*callback)(int value, void *userdata);
    } slider;

    // Para SDL_MENU_ITEM_CHOICE
    struct {
      sdl_menu_choice_option_t options[SDL_MENU_MAX_ITEMS];
      int option_count;
      int selected_index;
      void (*callback)(int value, void *userdata);
    } choice;

    // Para SDL_MENU_ITEM_SUBMENU
    struct {
      struct sdl_menu *submenu;
    } submenu;
  };
} sdl_menu_item_t;

// Estrutura para um menu
typedef struct sdl_menu {
  char title[SDL_MENU_MAX_TEXT_LENGTH];      // Título do menu
  sdl_menu_item_t items[SDL_MENU_MAX_ITEMS]; // Itens do menu
  int item_count;                            // Número de itens
  int selected_index;                        // Índice do item selecionado
  struct sdl_menu *parent;                   // Menu pai (NULL se for menu raiz)
  void *userdata;                            // Dados do usuário
  bool visible;                              // Se o menu está visível

  // Aparência
  SDL_Color bg_color;        // Cor de fundo
  SDL_Color text_color;      // Cor do texto
  SDL_Color highlight_color; // Cor do item destacado
  SDL_Color disabled_color;  // Cor de itens desabilitados
  int padding;               // Espaçamento interno
  int item_height;           // Altura de cada item
  int width;                 // Largura do menu

  // Posição
  SDL_Rect rect; // Retângulo do menu na tela
} sdl_menu_t;

// Contexto global do menu
typedef struct {
  sdl_menu_t *active_menu;                    // Menu atualmente ativo
  sdl_menu_t *menu_stack[SDL_MENU_MAX_DEPTH]; // Pilha de menus (para navegação)
  int menu_stack_depth;                       // Profundidade atual da pilha
  sdl_game_renderer_t *renderer;              // Renderizador para desenho
  SDL_Texture *menu_texture; // Textura para renderização do menu
  bool initialized;          // Se o sistema de menu está inicializado
  TTF_Font *font;            // Fonte para o texto do menu
  void *userdata;            // Dados do usuário para callbacks
} sdl_menu_context_t;

// Funções de inicialização e finalização
bool sdl_menu_init(sdl_menu_context_t *context, sdl_game_renderer_t *renderer);
void sdl_menu_shutdown(sdl_menu_context_t *context);

// Funções de criação e manipulação de menus
sdl_menu_t *sdl_menu_create(const char *title, sdl_menu_t *parent);
void sdl_menu_destroy(sdl_menu_t *menu);
int sdl_menu_add_item(sdl_menu_t *menu, const char *id, const char *text,
                      sdl_menu_item_type_t type);
int sdl_menu_add_action(sdl_menu_t *menu, const char *id, const char *text,
                        void (*callback)(void *userdata));
int sdl_menu_add_toggle(sdl_menu_t *menu, const char *id, const char *text,
                        bool initial_value,
                        void (*callback)(bool value, void *userdata));
int sdl_menu_add_slider(sdl_menu_t *menu, const char *id, const char *text,
                        int min_value, int max_value, int initial_value,
                        int step, void (*callback)(int value, void *userdata));
int sdl_menu_add_choice(sdl_menu_t *menu, const char *id, const char *text,
                        sdl_menu_choice_option_t *options, int option_count,
                        int initial_index,
                        void (*callback)(int value, void *userdata));
int sdl_menu_add_submenu(sdl_menu_t *menu, const char *id, const char *text,
                         sdl_menu_t *submenu);
int sdl_menu_add_separator(sdl_menu_t *menu);

// Funções de acesso e modificação
bool sdl_menu_set_item_enabled(sdl_menu_t *menu, const char *id, bool enabled);
bool sdl_menu_set_item_visible(sdl_menu_t *menu, const char *id, bool visible);
bool sdl_menu_set_toggle_value(sdl_menu_t *menu, const char *id, bool value);
bool sdl_menu_set_slider_value(sdl_menu_t *menu, const char *id, int value);
bool sdl_menu_set_choice_index(sdl_menu_t *menu, const char *id, int index);
sdl_menu_item_t *sdl_menu_get_item(sdl_menu_t *menu, const char *id);

// Funções de entrada e navegação
bool sdl_menu_process_event(sdl_menu_context_t *context, SDL_Event *event);
bool sdl_menu_navigate_to(sdl_menu_context_t *context, sdl_menu_t *menu);
bool sdl_menu_navigate_back(sdl_menu_context_t *context);
bool sdl_menu_select_item(sdl_menu_context_t *context);

// Funções de renderização
bool sdl_menu_render(sdl_menu_context_t *context);
void sdl_menu_set_visible(sdl_menu_context_t *context, bool visible);
bool sdl_menu_is_visible(const sdl_menu_context_t *context);

#ifdef __cplusplus
}
#endif

#endif // SDL_MENU_H
