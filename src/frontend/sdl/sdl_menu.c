#include "sdl_menu.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configurações padrão
#define DEFAULT_MENU_WIDTH 320
#define DEFAULT_ITEM_HEIGHT 30
#define DEFAULT_MENU_PADDING 10
#define DEFAULT_FONT_SIZE 16
#define DEFAULT_FONT_PATH "assets/fonts/OpenSans-Regular.ttf"

// Cores padrão
static const SDL_Color DEFAULT_BG_COLOR = {
    32, 32, 32, 225}; // Cinza escuro com transparência
static const SDL_Color DEFAULT_TEXT_COLOR = {240, 240, 240,
                                             255}; // Branco quase puro
static const SDL_Color DEFAULT_HIGHLIGHT_COLOR = {64, 128, 255, 255}; // Azul
static const SDL_Color DEFAULT_DISABLED_COLOR = {128, 128, 128, 200}; // Cinza

// Função de log simples
static void menu_log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("[SDL_MENU] ");
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

// Função auxiliar para encontrar um item por ID
static int find_item_index(const sdl_menu_t *menu, const char *id) {
  if (!menu || !id)
    return -1;

  for (int i = 0; i < menu->item_count; i++) {
    if (strcmp(menu->items[i].id, id) == 0) {
      return i;
    }
  }

  return -1;
}

// Inicializar o contexto de menu
bool sdl_menu_init(sdl_menu_context_t *context, sdl_game_renderer_t *renderer) {
  if (!context || !renderer)
    return false;

  // Limpar contexto
  memset(context, 0, sizeof(sdl_menu_context_t));
  context->renderer = renderer;

  // Inicializar SDL_ttf se necessário
  if (!TTF_WasInit() && TTF_Init() < 0) {
    menu_log("Erro ao inicializar SDL_ttf: %s", TTF_GetError());
    return false;
  }

  // Carregar fonte
  context->font = TTF_OpenFont(DEFAULT_FONT_PATH, DEFAULT_FONT_SIZE);
  if (!context->font) {
    // Tentar caminho alternativo
    context->font =
        TTF_OpenFont("../assets/fonts/OpenSans-Regular.ttf", DEFAULT_FONT_SIZE);
  }

  if (!context->font) {
    menu_log("Erro ao carregar fonte: %s", TTF_GetError());
    menu_log("Usando fonte padrão do sistema");
    // Continuar mesmo sem fonte, renderizaremos texto de forma básica
  }

  // Criar textura para o menu
  int window_width, window_height;
  sdl_game_renderer_get_output_size(renderer, &window_width, &window_height);

  context->menu_texture =
      SDL_CreateTexture(renderer->renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_TARGET, window_width, window_height);

  if (!context->menu_texture) {
    menu_log("Erro ao criar textura do menu: %s", SDL_GetError());
    if (context->font) {
      TTF_CloseFont(context->font);
      context->font = NULL;
    }
    return false;
  }

  // Configurar modo de blending
  SDL_SetTextureBlendMode(context->menu_texture, SDL_BLENDMODE_BLEND);

  context->initialized = true;
  return true;
}

// Finalizar o contexto de menu
void sdl_menu_shutdown(sdl_menu_context_t *context) {
  if (!context || !context->initialized)
    return;

  // Liberar menus ativos
  for (int i = 0; i < context->menu_stack_depth; i++) {
    // Não destruímos os menus aqui, apenas limpamos a pilha
    context->menu_stack[i] = NULL;
  }

  // Liberar textura
  if (context->menu_texture) {
    SDL_DestroyTexture(context->menu_texture);
    context->menu_texture = NULL;
  }

  // Liberar fonte
  if (context->font) {
    TTF_CloseFont(context->font);
    context->font = NULL;
  }

  context->initialized = false;
}

// Criar um novo menu
sdl_menu_t *sdl_menu_create(const char *title, sdl_menu_t *parent) {
  if (!title)
    return NULL;

  sdl_menu_t *menu = (sdl_menu_t *)calloc(1, sizeof(sdl_menu_t));
  if (!menu)
    return NULL;

  // Configurar título e estado padrão
  strncpy(menu->title, title, SDL_MENU_MAX_TEXT_LENGTH - 1);
  menu->parent = parent;
  menu->item_count = 0;
  menu->selected_index = 0;
  menu->visible = true;

  // Configurar aparência padrão
  menu->bg_color = DEFAULT_BG_COLOR;
  menu->text_color = DEFAULT_TEXT_COLOR;
  menu->highlight_color = DEFAULT_HIGHLIGHT_COLOR;
  menu->disabled_color = DEFAULT_DISABLED_COLOR;
  menu->padding = DEFAULT_MENU_PADDING;
  menu->item_height = DEFAULT_ITEM_HEIGHT;
  menu->width = DEFAULT_MENU_WIDTH;

  // Calcular posição padrão (centro da tela)
  menu->rect.w = menu->width;
  menu->rect.h = DEFAULT_ITEM_HEIGHT; // Será ajustado quando adicionarmos itens
  menu->rect.x = 0; // Será calculado no momento da renderização
  menu->rect.y = 0; // Será calculado no momento da renderização

  return menu;
}

// Destruir um menu e seus submenus
void sdl_menu_destroy(sdl_menu_t *menu) {
  if (!menu)
    return;

  // Destruir submenus
  for (int i = 0; i < menu->item_count; i++) {
    if (menu->items[i].type == SDL_MENU_ITEM_SUBMENU &&
        menu->items[i].submenu.submenu) {
      sdl_menu_destroy(menu->items[i].submenu.submenu);
      menu->items[i].submenu.submenu = NULL;
    }
  }

  free(menu);
}

// Adicionar um item genérico
int sdl_menu_add_item(sdl_menu_t *menu, const char *id, const char *text,
                      sdl_menu_item_type_t type) {
  if (!menu || !id || !text || menu->item_count >= SDL_MENU_MAX_ITEMS)
    return -1;

  // Verificar se já existe um item com o mesmo ID
  if (find_item_index(menu, id) >= 0) {
    menu_log("Item com ID '%s' já existe", id);
    return -1;
  }

  // Adicionar novo item
  sdl_menu_item_t *item = &menu->items[menu->item_count];
  strncpy(item->id, id, SDL_MENU_MAX_TEXT_LENGTH - 1);
  strncpy(item->text, text, SDL_MENU_MAX_TEXT_LENGTH - 1);
  item->type = type;
  item->enabled = true;
  item->visible = true;

  // Incrementar contador e atualizar altura do menu
  int index = menu->item_count;
  menu->item_count++;
  menu->rect.h = menu->padding * 2 + menu->item_count * menu->item_height;

  return index;
}

// Adicionar item de ação
int sdl_menu_add_action(sdl_menu_t *menu, const char *id, const char *text,
                        void (*callback)(void *userdata)) {
  int index = sdl_menu_add_item(menu, id, text, SDL_MENU_ITEM_ACTION);
  if (index >= 0) {
    menu->items[index].action.callback = callback;
  }
  return index;
}

// Adicionar item de alternância
int sdl_menu_add_toggle(sdl_menu_t *menu, const char *id, const char *text,
                        bool initial_value,
                        void (*callback)(bool value, void *userdata)) {
  int index = sdl_menu_add_item(menu, id, text, SDL_MENU_ITEM_TOGGLE);
  if (index >= 0) {
    menu->items[index].toggle.value = initial_value;
    menu->items[index].toggle.callback = callback;
  }
  return index;
}

// Adicionar item de slider
int sdl_menu_add_slider(sdl_menu_t *menu, const char *id, const char *text,
                        int min_value, int max_value, int initial_value,
                        int step, void (*callback)(int value, void *userdata)) {
  int index = sdl_menu_add_item(menu, id, text, SDL_MENU_ITEM_SLIDER);
  if (index >= 0) {
    menu->items[index].slider.min_value = min_value;
    menu->items[index].slider.max_value = max_value;
    menu->items[index].slider.value = initial_value;
    menu->items[index].slider.step = step;
    menu->items[index].slider.callback = callback;
  }
  return index;
}

// Adicionar item de escolha
int sdl_menu_add_choice(sdl_menu_t *menu, const char *id, const char *text,
                        sdl_menu_choice_option_t *options, int option_count,
                        int initial_index,
                        void (*callback)(int value, void *userdata)) {
  int index = sdl_menu_add_item(menu, id, text, SDL_MENU_ITEM_CHOICE);
  if (index >= 0) {
    // Limitar número de opções
    if (option_count > SDL_MENU_MAX_ITEMS)
      option_count = SDL_MENU_MAX_ITEMS;

    // Copiar opções
    for (int i = 0; i < option_count; i++) {
      menu->items[index].choice.options[i] = options[i];
    }

    menu->items[index].choice.option_count = option_count;

    // Validar índice inicial
    if (initial_index >= 0 && initial_index < option_count) {
      menu->items[index].choice.selected_index = initial_index;
    } else {
      menu->items[index].choice.selected_index = 0;
    }

    menu->items[index].choice.callback = callback;
  }
  return index;
}

// Adicionar submenu
int sdl_menu_add_submenu(sdl_menu_t *menu, const char *id, const char *text,
                         sdl_menu_t *submenu) {
  int index = sdl_menu_add_item(menu, id, text, SDL_MENU_ITEM_SUBMENU);
  if (index >= 0) {
    menu->items[index].submenu.submenu = submenu;
  }
  return index;
}

// Adicionar separador
int sdl_menu_add_separator(sdl_menu_t *menu) {
  static int separator_count = 0;
  char id[SDL_MENU_MAX_TEXT_LENGTH];
  snprintf(id, SDL_MENU_MAX_TEXT_LENGTH, "separator_%d", separator_count++);

  return sdl_menu_add_item(menu, id, "", SDL_MENU_ITEM_SEPARATOR);
}

// Definir ativação de um item
bool sdl_menu_set_item_enabled(sdl_menu_t *menu, const char *id, bool enabled) {
  if (!menu || !id)
    return false;

  int index = find_item_index(menu, id);
  if (index < 0)
    return false;

  menu->items[index].enabled = enabled;
  return true;
}

// Definir visibilidade de um item
bool sdl_menu_set_item_visible(sdl_menu_t *menu, const char *id, bool visible) {
  if (!menu || !id)
    return false;

  int index = find_item_index(menu, id);
  if (index < 0)
    return false;

  menu->items[index].visible = visible;
  return true;
}

// Definir valor de item de alternância
bool sdl_menu_set_toggle_value(sdl_menu_t *menu, const char *id, bool value) {
  if (!menu || !id)
    return false;

  int index = find_item_index(menu, id);
  if (index < 0 || menu->items[index].type != SDL_MENU_ITEM_TOGGLE)
    return false;

  menu->items[index].toggle.value = value;
  return true;
}

// Definir valor de item slider
bool sdl_menu_set_slider_value(sdl_menu_t *menu, const char *id, int value) {
  if (!menu || !id)
    return false;

  int index = find_item_index(menu, id);
  if (index < 0 || menu->items[index].type != SDL_MENU_ITEM_SLIDER)
    return false;

  // Limitar ao intervalo válido
  if (value < menu->items[index].slider.min_value)
    value = menu->items[index].slider.min_value;
  else if (value > menu->items[index].slider.max_value)
    value = menu->items[index].slider.max_value;

  menu->items[index].slider.value = value;
  return true;
}

// Definir índice de item de escolha
bool sdl_menu_set_choice_index(sdl_menu_t *menu, const char *id, int index) {
  if (!menu || !id)
    return false;

  int item_index = find_item_index(menu, id);
  if (item_index < 0 || menu->items[item_index].type != SDL_MENU_ITEM_CHOICE)
    return false;

  // Validar índice
  if (index < 0 || index >= menu->items[item_index].choice.option_count)
    return false;

  menu->items[item_index].choice.selected_index = index;
  return true;
}

// Obter item por ID
sdl_menu_item_t *sdl_menu_get_item(sdl_menu_t *menu, const char *id) {
  if (!menu || !id)
    return NULL;

  int index = find_item_index(menu, id);
  if (index < 0)
    return NULL;

  return &menu->items[index];
}

// Processar evento SDL
bool sdl_menu_process_event(sdl_menu_context_t *context, SDL_Event *event) {
  if (!context || !context->initialized || !event || !context->active_menu)
    return false;

  sdl_menu_t *menu = context->active_menu;

  // Processar apenas se o menu estiver visível
  if (!menu->visible)
    return false;

  switch (event->type) {
  case SDL_KEYDOWN:
    switch (event->key.keysym.sym) {
    case SDLK_UP:
      // Navegar para cima
      do {
        menu->selected_index--;
        if (menu->selected_index < 0)
          menu->selected_index = menu->item_count - 1;
      } while (
          menu->selected_index >= 0 &&
          menu->selected_index < menu->item_count &&
          (!menu->items[menu->selected_index].visible ||
           !menu->items[menu->selected_index].enabled ||
           menu->items[menu->selected_index].type == SDL_MENU_ITEM_SEPARATOR));
      return true;

    case SDLK_DOWN:
      // Navegar para baixo
      do {
        menu->selected_index++;
        if (menu->selected_index >= menu->item_count)
          menu->selected_index = 0;
      } while (
          menu->selected_index >= 0 &&
          menu->selected_index < menu->item_count &&
          (!menu->items[menu->selected_index].visible ||
           !menu->items[menu->selected_index].enabled ||
           menu->items[menu->selected_index].type == SDL_MENU_ITEM_SEPARATOR));
      return true;

    case SDLK_LEFT:
      // Ajustar valor para a esquerda
      if (menu->selected_index >= 0 &&
          menu->selected_index < menu->item_count) {
        sdl_menu_item_t *item = &menu->items[menu->selected_index];

        if (item->visible && item->enabled) {
          switch (item->type) {
          case SDL_MENU_ITEM_TOGGLE:
            item->toggle.value = !item->toggle.value;
            if (item->toggle.callback)
              item->toggle.callback(item->toggle.value, context->userdata);
            return true;

          case SDL_MENU_ITEM_SLIDER:
            item->slider.value -= item->slider.step;
            if (item->slider.value < item->slider.min_value)
              item->slider.value = item->slider.min_value;
            if (item->slider.callback)
              item->slider.callback(item->slider.value, context->userdata);
            return true;

          case SDL_MENU_ITEM_CHOICE:
            item->choice.selected_index--;
            if (item->choice.selected_index < 0)
              item->choice.selected_index = item->choice.option_count - 1;
            if (item->choice.callback)
              item->choice.callback(
                  item->choice.options[item->choice.selected_index].value,
                  context->userdata);
            return true;

          default:
            break;
          }
        }
      }
      break;

    case SDLK_RIGHT:
      // Ajustar valor para a direita
      if (menu->selected_index >= 0 &&
          menu->selected_index < menu->item_count) {
        sdl_menu_item_t *item = &menu->items[menu->selected_index];

        if (item->visible && item->enabled) {
          switch (item->type) {
          case SDL_MENU_ITEM_TOGGLE:
            item->toggle.value = !item->toggle.value;
            if (item->toggle.callback)
              item->toggle.callback(item->toggle.value, context->userdata);
            return true;

          case SDL_MENU_ITEM_SLIDER:
            item->slider.value += item->slider.step;
            if (item->slider.value > item->slider.max_value)
              item->slider.value = item->slider.max_value;
            if (item->slider.callback)
              item->slider.callback(item->slider.value, context->userdata);
            return true;

          case SDL_MENU_ITEM_CHOICE:
            item->choice.selected_index++;
            if (item->choice.selected_index >= item->choice.option_count)
              item->choice.selected_index = 0;
            if (item->choice.callback)
              item->choice.callback(
                  item->choice.options[item->choice.selected_index].value,
                  context->userdata);
            return true;

          default:
            break;
          }
        }
      }
      break;

    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      // Selecionar item
      return sdl_menu_select_item(context);

    case SDLK_ESCAPE:
      // Voltar ao menu anterior
      return sdl_menu_navigate_back(context);

    default:
      break;
    }
    break;

    // Adicionar suporte a mouse e outros eventos aqui

  default:
    break;
  }

  return false;
}

// Navegar para um menu
bool sdl_menu_navigate_to(sdl_menu_context_t *context, sdl_menu_t *menu) {
  if (!context || !context->initialized || !menu)
    return false;

  // Verificar espaço na pilha
  if (context->menu_stack_depth >= SDL_MENU_MAX_DEPTH - 1) {
    menu_log("Pilha de menu cheia, não é possível navegar para mais menus");
    return false;
  }

  // Adicionar menu atual à pilha
  if (context->active_menu) {
    context->menu_stack[context->menu_stack_depth++] = context->active_menu;
  }

  // Ativar novo menu
  context->active_menu = menu;

  return true;
}

// Voltar ao menu anterior
bool sdl_menu_navigate_back(sdl_menu_context_t *context) {
  if (!context || !context->initialized)
    return false;

  // Verificar se há menu anterior
  if (context->menu_stack_depth <= 0) {
    // Esconder menu atual
    if (context->active_menu) {
      context->active_menu->visible = false;
      context->active_menu = NULL;
    }
    return false;
  }

  // Voltar ao menu anterior
  context->active_menu = context->menu_stack[--context->menu_stack_depth];

  return true;
}

// Selecionar item atual
bool sdl_menu_select_item(sdl_menu_context_t *context) {
  if (!context || !context->initialized || !context->active_menu)
    return false;

  sdl_menu_t *menu = context->active_menu;

  // Verificar se há item selecionado válido
  if (menu->selected_index < 0 || menu->selected_index >= menu->item_count)
    return false;

  sdl_menu_item_t *item = &menu->items[menu->selected_index];

  // Verificar se item está habilitado e visível
  if (!item->visible || !item->enabled)
    return false;

  // Processar ação com base no tipo
  switch (item->type) {
  case SDL_MENU_ITEM_ACTION:
    if (item->action.callback)
      item->action.callback(context->userdata);
    return true;

  case SDL_MENU_ITEM_TOGGLE:
    item->toggle.value = !item->toggle.value;
    if (item->toggle.callback)
      item->toggle.callback(item->toggle.value, context->userdata);
    return true;

  case SDL_MENU_ITEM_SUBMENU:
    if (item->submenu.submenu)
      return sdl_menu_navigate_to(context, item->submenu.submenu);
    return false;

  default:
    return false;
  }
}

// Renderizar texto usando SDL_ttf se disponível
static void render_text(sdl_menu_context_t *context, const char *text,
                        SDL_Color color, SDL_Rect *rect) {
  SDL_Renderer *renderer = context->renderer->renderer;

  if (context->font) {
    // Renderizar com SDL_ttf
    SDL_Surface *surface = TTF_RenderText_Blended(context->font, text, color);
    if (surface) {
      SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
      if (texture) {
        // Ajustar retângulo para centralizar o texto verticalmente
        SDL_Rect dest = *rect;
        dest.y = rect->y + (rect->h - surface->h) / 2;
        dest.h = surface->h;

        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_DestroyTexture(texture);
      }
      SDL_FreeSurface(surface);
    }
  } else {
    // Renderização básica de caracteres sem SDL_ttf
    // Apenas desenha um retângulo com a cor do texto para indicar sua presença
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, rect);

    // Desenhar pontos para representar o texto
    int text_len = strlen(text);
    int dot_width = 2;
    int dot_space = 3;

    for (int i = 0; i < text_len && i < 20; i++) {
      SDL_Rect dot = {rect->x + 5 + i * (dot_width + dot_space),
                      rect->y + (rect->h / 2) - (dot_width / 2), dot_width,
                      dot_width};
      SDL_RenderFillRect(renderer, &dot);
    }
  }
}

// Renderizar menu ativo
bool sdl_menu_render(sdl_menu_context_t *context) {
  if (!context || !context->initialized || !context->active_menu)
    return false;

  sdl_menu_t *menu = context->active_menu;

  // Se menu não está visível, não renderizar
  if (!menu->visible)
    return false;

  SDL_Renderer *renderer = context->renderer->renderer;

  // Salvar renderizador alvo atual
  SDL_Texture *current_target = SDL_GetRenderTarget(renderer);

  // Definir textura de menu como alvo
  SDL_SetRenderTarget(renderer, context->menu_texture);

  // Limpar textura com transparência
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  // Calcular dimensões do menu
  int window_width, window_height;
  sdl_game_renderer_get_output_size(context->renderer, &window_width,
                                    &window_height);

  // Calcular altura real do menu (apenas itens visíveis)
  int visible_items = 0;
  for (int i = 0; i < menu->item_count; i++) {
    if (menu->items[i].visible)
      visible_items++;
  }

  int menu_height = menu->padding * 2 + visible_items * menu->item_height;

  // Centralizar menu na tela
  menu->rect.w = menu->width;
  menu->rect.h = menu_height;
  menu->rect.x = (window_width - menu->width) / 2;
  menu->rect.y = (window_height - menu_height) / 2;

  // Desenhar fundo do menu
  SDL_SetRenderDrawColor(renderer, menu->bg_color.r, menu->bg_color.g,
                         menu->bg_color.b, menu->bg_color.a);
  SDL_RenderFillRect(renderer, &menu->rect);

  // Adicionar borda
  SDL_SetRenderDrawColor(renderer, menu->text_color.r, menu->text_color.g,
                         menu->text_color.b, menu->text_color.a);
  SDL_RenderDrawRect(renderer, &menu->rect);

  // Renderizar título
  SDL_Rect title_rect = {menu->rect.x + menu->padding,
                         menu->rect.y + menu->padding,
                         menu->rect.w - menu->padding * 2, menu->item_height};

  render_text(context, menu->title, menu->text_color, &title_rect);

  // Desenhar linha separadora do título
  SDL_Rect title_separator = {menu->rect.x + menu->padding,
                              title_rect.y + title_rect.h,
                              menu->rect.w - menu->padding * 2, 1};

  SDL_RenderFillRect(renderer, &title_separator);

  // Renderizar itens
  int y_pos = title_rect.y + title_rect.h + menu->padding;

  for (int i = 0; i < menu->item_count; i++) {
    sdl_menu_item_t *item = &menu->items[i];

    // Pular itens não visíveis
    if (!item->visible)
      continue;

    // Retângulo para o item
    SDL_Rect item_rect = {menu->rect.x + menu->padding, y_pos,
                          menu->rect.w - menu->padding * 2, menu->item_height};

    // Desenhar fundo do item selecionado
    if (i == menu->selected_index) {
      SDL_SetRenderDrawColor(renderer, menu->highlight_color.r,
                             menu->highlight_color.g, menu->highlight_color.b,
                             128); // Semi-transparente
      SDL_RenderFillRect(renderer, &item_rect);
    }

    // Escolher cor com base no estado do item
    SDL_Color text_color;
    if (!item->enabled) {
      text_color = menu->disabled_color;
    } else if (i == menu->selected_index) {
      text_color = menu->text_color;
    } else {
      text_color = menu->text_color;
      // Reduzir alpha para itens não selecionados
      text_color.a = 200;
    }

    // Renderizar texto do item
    if (item->type != SDL_MENU_ITEM_SEPARATOR) {
      render_text(context, item->text, text_color, &item_rect);
    } else {
      // Desenhar linha separadora
      SDL_Rect separator = {item_rect.x, item_rect.y + item_rect.h / 2,
                            item_rect.w, 1};

      SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b,
                             text_color.a);
      SDL_RenderFillRect(renderer, &separator);
    }

    // Renderizar elementos adicionais com base no tipo
    SDL_Rect value_rect = {item_rect.x + item_rect.w - 100, item_rect.y, 100,
                           item_rect.h};

    char value_text[SDL_MENU_MAX_TEXT_LENGTH];

    switch (item->type) {
    case SDL_MENU_ITEM_TOGGLE:
      snprintf(value_text, SDL_MENU_MAX_TEXT_LENGTH, "%s",
               item->toggle.value ? "ON" : "OFF");
      render_text(context, value_text, text_color, &value_rect);
      break;

    case SDL_MENU_ITEM_SLIDER:
      snprintf(value_text, SDL_MENU_MAX_TEXT_LENGTH, "%d", item->slider.value);
      render_text(context, value_text, text_color, &value_rect);
      break;

    case SDL_MENU_ITEM_CHOICE:
      if (item->choice.selected_index >= 0 &&
          item->choice.selected_index < item->choice.option_count) {
        render_text(context,
                    item->choice.options[item->choice.selected_index].text,
                    text_color, &value_rect);
      }
      break;

    case SDL_MENU_ITEM_SUBMENU:
      render_text(context, "►", text_color, &value_rect);
      break;

    default:
      break;
    }

    y_pos += menu->item_height;
  }

  // Restaurar renderizador alvo
  SDL_SetRenderTarget(renderer, current_target);

  // Renderizar textura do menu na tela
  SDL_RenderCopy(renderer, context->menu_texture, NULL, NULL);

  return true;
}

// Definir visibilidade do menu
void sdl_menu_set_visible(sdl_menu_context_t *context, bool visible) {
  if (!context || !context->initialized || !context->active_menu)
    return;

  context->active_menu->visible = visible;
}

// Verificar se o menu está visível
bool sdl_menu_is_visible(const sdl_menu_context_t *context) {
  if (!context || !context->initialized || !context->active_menu)
    return false;

  return context->active_menu->visible;
}
