#ifndef MEGA_EMU_GUI_ELEMENT_H
#define MEGA_EMU_GUI_ELEMENT_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "gui_common.h"
#include "gui_types.h"

// Funções de gerenciamento de elementos
bool gui_element_init(void);
void gui_element_shutdown(void);

// Funções de manipulação de elementos
gui_element_id_t gui_element_create(gui_element_type_t type);
void gui_element_destroy(gui_element_id_t id);

// Funções de hierarquia
bool gui_element_add_child(gui_element_id_t parent_id, gui_element_id_t child_id);
bool gui_element_remove_child(gui_element_id_t parent_id, gui_element_id_t child_id);

// Funções de configuração
bool gui_element_set_text(gui_element_id_t id, const char *text);
bool gui_element_set_position(gui_element_id_t id, int32_t x, int32_t y);
bool gui_element_set_size(gui_element_id_t id, int32_t width, int32_t height);
bool gui_element_set_visible(gui_element_id_t id, bool visible);
bool gui_element_set_enabled(gui_element_id_t id, bool enabled);

// Funções de processamento de eventos e renderização
bool gui_element_process_event(const gui_event_t *event);
void gui_element_update(void);
void gui_element_render(SDL_Renderer *renderer);

#endif /* MEGA_EMU_GUI_ELEMENT_H */
