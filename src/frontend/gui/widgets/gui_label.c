/**
 * @file gui_label.c
 * @brief Implementação do widget de label (rótulo) da GUI
 */
#include "gui_label.h"
#include "../core/gui_element.h"
#include "../core/gui_common.h"
#include <stdlib.h>
#include <string.h>

// Definição de propriedades específicas do label
#define GUI_PROP_LABEL_TEXT_COLOR      "text_color"
#define GUI_PROP_LABEL_BG_COLOR        "bg_color"
#define GUI_PROP_LABEL_H_ALIGNMENT     "h_alignment"
#define GUI_PROP_LABEL_V_ALIGNMENT     "v_alignment"
#define GUI_PROP_LABEL_TRANSPARENT     "transparent"

// Valores padrão
static const gui_color_t DEFAULT_TEXT_COLOR = {255, 255, 255, 255};
static const gui_color_t DEFAULT_BG_COLOR = {0, 0, 0, 0};
static const int DEFAULT_H_ALIGNMENT = 0; // Esquerda
static const int DEFAULT_V_ALIGNMENT = 1; // Centro
static const bool DEFAULT_TRANSPARENT = true;

// Função de renderização do label
static void label_render(gui_element_t element, SDL_Renderer* renderer) {
    if (!element || !renderer) {
        return;
    }

    // Obter retângulo do label
    gui_rect_t rect;
    gui_element_get_rect(element, &rect);
    
    // Converter para SDL_Rect
    SDL_Rect sdl_rect = {
        rect.x, rect.y, rect.width, rect.height
    };
    
    // Verificar se o label é transparente
    bool transparent = DEFAULT_TRANSPARENT;
    gui_element_get_property_bool(element, GUI_PROP_LABEL_TRANSPARENT, &transparent);
    
    // Renderizar fundo se não for transparente
    if (!transparent) {
        gui_color_t bg_color;
        if (gui_element_get_property_color(element, GUI_PROP_LABEL_BG_COLOR, &bg_color)) {
            SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
            SDL_RenderFillRect(renderer, &sdl_rect);
        }
    }
    
    // Obter texto do label
    const char* text = gui_element_get_text(element);
    if (!text || strlen(text) == 0) {
        return;
    }
    
    // Obter cor do texto
    gui_color_t text_color;
    if (!gui_element_get_property_color(element, GUI_PROP_LABEL_TEXT_COLOR, &text_color)) {
        text_color = DEFAULT_TEXT_COLOR;
    }
    
    // Obter alinhamentos
    int h_alignment = DEFAULT_H_ALIGNMENT;
    int v_alignment = DEFAULT_V_ALIGNMENT;
    gui_element_get_property_int(element, GUI_PROP_LABEL_H_ALIGNMENT, &h_alignment);
    gui_element_get_property_int(element, GUI_PROP_LABEL_V_ALIGNMENT, &v_alignment);
    
    // Renderizar texto com alinhamento
    gui_render_text(renderer, text, &sdl_rect, &text_color, h_alignment, v_alignment);
}

// Função de processamento de eventos do label
static bool label_process_event(gui_element_t element, const gui_event_t* event) {
    // Labels geralmente não processam eventos, apenas exibem informações
    return false;
}

// Função de atualização do label
static void label_update(gui_element_t element) {
    // Labels geralmente não precisam de atualizações lógicas
}

// Função de destruição do label
static void label_destroy(gui_element_t element) {
    // Não há recursos específicos do label para liberar
}

gui_element_id_t gui_label_create(const gui_rect_t* rect, const char* text) {
    if (!rect) {
        GUI_LOG_ERROR("Invalid rectangle for label");
        return GUI_INVALID_ID;
    }
    
    // Criar elemento base
    gui_element_t element = gui_element_create(GUI_ELEMENT_LABEL, rect, text);
    if (!element) {
        GUI_LOG_ERROR("Failed to create base element for label");
        return GUI_INVALID_ID;
    }
    
    // Configurar funções do elemento
    gui_element_set_render_func(element, label_render);
    gui_element_set_process_event_func(element, label_process_event);
    gui_element_set_update_func(element, label_update);
    gui_element_set_destroy_func(element, label_destroy);
    
    // Definir propriedades padrão
    gui_element_set_property_color(element, GUI_PROP_LABEL_TEXT_COLOR, &DEFAULT_TEXT_COLOR);
    gui_element_set_property_color(element, GUI_PROP_LABEL_BG_COLOR, &DEFAULT_BG_COLOR);
    gui_element_set_property_int(element, GUI_PROP_LABEL_H_ALIGNMENT, DEFAULT_H_ALIGNMENT);
    gui_element_set_property_int(element, GUI_PROP_LABEL_V_ALIGNMENT, DEFAULT_V_ALIGNMENT);
    gui_element_set_property_bool(element, GUI_PROP_LABEL_TRANSPARENT, DEFAULT_TRANSPARENT);
    
    return gui_element_get_id(element);
}

bool gui_label_set_text_color(gui_element_id_t label_id, const gui_color_t* color) {
    if (label_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(label_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_LABEL) {
        return false;
    }
    
    return gui_element_set_property_color(element, GUI_PROP_LABEL_TEXT_COLOR, color);
}

bool gui_label_set_background_color(gui_element_id_t label_id, const gui_color_t* color) {
    if (label_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(label_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_LABEL) {
        return false;
    }
    
    return gui_element_set_property_color(element, GUI_PROP_LABEL_BG_COLOR, color);
}

bool gui_label_set_h_alignment(gui_element_id_t label_id, int alignment) {
    if (label_id == GUI_INVALID_ID || alignment < 0 || alignment > 2) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(label_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_LABEL) {
        return false;
    }
    
    return gui_element_set_property_int(element, GUI_PROP_LABEL_H_ALIGNMENT, alignment);
}

bool gui_label_set_v_alignment(gui_element_id_t label_id, int alignment) {
    if (label_id == GUI_INVALID_ID || alignment < 0 || alignment > 2) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(label_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_LABEL) {
        return false;
    }
    
    return gui_element_set_property_int(element, GUI_PROP_LABEL_V_ALIGNMENT, alignment);
}

bool gui_label_set_transparent(gui_element_id_t label_id, bool transparent) {
    if (label_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(label_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_LABEL) {
        return false;
    }
    
    return gui_element_set_property_bool(element, GUI_PROP_LABEL_TRANSPARENT, transparent);
}
