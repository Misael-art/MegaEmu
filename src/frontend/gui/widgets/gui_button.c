/**
 * @file gui_button.c
 * @brief Implementação do widget de botão da GUI
 */
#include "gui_button.h"
#include "../core/gui_common.h"
#include "../core/gui_element.h"
#include "../core/gui_manager.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Cores padrão do botão
static const gui_color_t DEFAULT_TEXT_COLOR = {255, 255, 255, 255};
static const gui_color_t DEFAULT_BG_COLOR = {80, 80, 80, 255};
static const gui_color_t DEFAULT_HOVER_COLOR = {100, 100, 100, 255};
static const gui_color_t DEFAULT_PRESSED_COLOR = {60, 60, 60, 255};

// Propriedades específicas do botão
typedef struct {
    void (*click_callback)(gui_element_id_t, void*);
    void* user_data;
    gui_color_t text_color;
    gui_color_t bg_color;
    gui_color_t hover_color;
    gui_color_t pressed_color;
    bool is_hover;
    bool is_pressed;
} gui_button_props_t;

// Função de renderização do botão
static void button_render(gui_element_id_t id, SDL_Renderer* renderer, const gui_rect_t* rect, void* props) {
    gui_button_props_t* button_props = (gui_button_props_t*)props;
    
    if (!button_props || !renderer || !rect) {
        return;
    }
    
    // Selecionar cor de fundo com base no estado
    const gui_color_t* bg_color = &button_props->bg_color;
    
    if (button_props->is_pressed) {
        bg_color = &button_props->pressed_color;
    } else if (button_props->is_hover) {
        bg_color = &button_props->hover_color;
    }
    
    // Renderizar fundo
    SDL_SetRenderDrawColor(renderer, bg_color->r, bg_color->g, bg_color->b, bg_color->a);
    
    SDL_Rect sdl_rect = {
        rect->x,
        rect->y,
        rect->width,
        rect->height
    };
    
    SDL_RenderFillRect(renderer, &sdl_rect);
    
    // Renderizar borda
    SDL_SetRenderDrawColor(renderer, 
                          button_props->text_color.r, 
                          button_props->text_color.g, 
                          button_props->text_color.b, 
                          button_props->text_color.a);
    
    SDL_RenderDrawRect(renderer, &sdl_rect);
    
    // Renderizar texto (simplificado - em uma implementação real, usaríamos SDL_ttf)
    // Aqui apenas simulamos o texto com um pequeno retângulo no centro
    int text_width = rect->width / 3;
    int text_height = rect->height / 4;
    
    SDL_Rect text_rect = {
        rect->x + (rect->width - text_width) / 2,
        rect->y + (rect->height - text_height) / 2,
        text_width,
        text_height
    };
    
    // Desenhar apenas se o botão tiver texto
    const char* text = gui_element_get_text(id);
    if (text && strlen(text) > 0) {
        SDL_RenderDrawRect(renderer, &text_rect);
    }
}

// Função de processamento de eventos do botão
static bool button_process_event(gui_element_id_t id, const gui_event_t* event, void* props) {
    gui_button_props_t* button_props = (gui_button_props_t*)props;
    
    if (!button_props || !event) {
        return false;
    }
    
    bool handled = false;
    
    switch (event->type) {
        case GUI_EVENT_MOUSE_MOVE:
            if (event->target == id) {
                button_props->is_hover = true;
                handled = true;
            } else {
                button_props->is_hover = false;
            }
            break;
            
        case GUI_EVENT_MOUSE_DOWN:
            if (event->target == id && event->mouse.button == GUI_MOUSE_BUTTON_LEFT) {
                button_props->is_pressed = true;
                handled = true;
            }
            break;
            
        case GUI_EVENT_MOUSE_UP:
            if (button_props->is_pressed && event->mouse.button == GUI_MOUSE_BUTTON_LEFT) {
                // Verificar se o mouse ainda está sobre o botão
                if (event->target == id) {
                    // Chamar callback de clique
                    if (button_props->click_callback) {
                        button_props->click_callback(id, button_props->user_data);
                    }
                }
                
                button_props->is_pressed = false;
                handled = true;
            }
            break;
            
        case GUI_EVENT_MOUSE_LEAVE:
            button_props->is_hover = false;
            button_props->is_pressed = false;
            handled = true;
            break;
            
        default:
            break;
    }
    
    return handled;
}

// Função de destruição das propriedades do botão
static void button_destroy_props(void* props) {
    if (props) {
        free(props);
    }
}

gui_element_id_t gui_button_create(const gui_rect_t* rect, const char* text) {
    if (!rect) {
        GUI_LOG_ERROR("Invalid button rectangle");
        return GUI_INVALID_ID;
    }
    
    // Criar propriedades do botão
    gui_button_props_t* props = (gui_button_props_t*)malloc(sizeof(gui_button_props_t));
    
    if (!props) {
        GUI_LOG_ERROR("Failed to allocate button properties");
        return GUI_INVALID_ID;
    }
    
    // Inicializar propriedades
    memset(props, 0, sizeof(gui_button_props_t));
    props->text_color = DEFAULT_TEXT_COLOR;
    props->bg_color = DEFAULT_BG_COLOR;
    props->hover_color = DEFAULT_HOVER_COLOR;
    props->pressed_color = DEFAULT_PRESSED_COLOR;
    props->is_hover = false;
    props->is_pressed = false;
    
    // Criar elemento
    gui_element_id_t id = gui_manager_add_element(
        gui_manager_get_instance(),
        GUI_ELEMENT_BUTTON,
        rect,
        text
    );
    
    if (id == GUI_INVALID_ID) {
        free(props);
        return GUI_INVALID_ID;
    }
    
    // Configurar callbacks
    gui_element_set_render_callback(id, button_render);
    gui_element_set_event_callback(id, button_process_event);
    gui_element_set_destroy_callback(id, button_destroy_props);
    gui_element_set_props(id, props);
    
    return id;
}

bool gui_button_set_click_callback(gui_element_id_t button_id, void (*callback)(gui_element_id_t, void*), void* user_data) {
    if (button_id == GUI_INVALID_ID) {
        return false;
    }
    
    // Obter propriedades do botão
    gui_button_props_t* props = (gui_button_props_t*)gui_element_get_props(button_id);
    
    if (!props) {
        return false;
    }
    
    // Configurar callback
    props->click_callback = callback;
    props->user_data = user_data;
    
    return true;
}

bool gui_button_set_text_color(gui_element_id_t button_id, const gui_color_t* color) {
    if (button_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    // Obter propriedades do botão
    gui_button_props_t* props = (gui_button_props_t*)gui_element_get_props(button_id);
    
    if (!props) {
        return false;
    }
    
    // Configurar cor do texto
    props->text_color = *color;
    
    return true;
}

bool gui_button_set_background_color(gui_element_id_t button_id, const gui_color_t* color) {
    if (button_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    // Obter propriedades do botão
    gui_button_props_t* props = (gui_button_props_t*)gui_element_get_props(button_id);
    
    if (!props) {
        return false;
    }
    
    // Configurar cor de fundo
    props->bg_color = *color;
    
    return true;
}

bool gui_button_set_hover_color(gui_element_id_t button_id, const gui_color_t* color) {
    if (button_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    // Obter propriedades do botão
    gui_button_props_t* props = (gui_button_props_t*)gui_element_get_props(button_id);
    
    if (!props) {
        return false;
    }
    
    // Configurar cor de hover
    props->hover_color = *color;
    
    return true;
}

bool gui_button_set_pressed_color(gui_element_id_t button_id, const gui_color_t* color) {
    if (button_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    // Obter propriedades do botão
    gui_button_props_t* props = (gui_button_props_t*)gui_element_get_props(button_id);
    
    if (!props) {
        return false;
    }
    
    // Configurar cor de pressionado
    props->pressed_color = *color;
    
    return true;
}
