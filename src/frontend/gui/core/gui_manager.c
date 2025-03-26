/**
 * @file gui_manager.c
 * @brief Implementação do gerenciador de GUI do Mega_Emu
 */
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui_common.h"
#include "gui_element.h"
#include "gui_manager.h"
#include "gui_types.h"

// Estrutura para callback
typedef struct {
    uint32_t id;
    gui_event_type_t event_type;
    gui_event_callback_t callback;
    void* user_data;
    bool active;
} gui_callback_s;

// Estrutura do gerenciador
struct gui_manager_s {
    gui_element_id_t elements[GUI_MANAGER_MAX_ELEMENTS];
    uint32_t element_count;
    gui_callback_s callbacks[GUI_MANAGER_MAX_CALLBACKS];
    uint32_t callback_count;
    uint32_t next_callback_id;
    gui_element_id_t focused_element;
    gui_element_id_t hovered_element;
    bool initialized;
};

gui_manager_t gui_manager_init(void) {
    // Inicializar sistema de elementos
    if (!gui_element_init()) {
        GUI_LOG_ERROR("Failed to initialize GUI element system");
        return NULL;
    }
    
    // Alocar gerenciador
    gui_manager_t manager = (gui_manager_t)malloc(sizeof(struct gui_manager_s));
    
    if (!manager) {
        GUI_LOG_ERROR("Failed to allocate GUI manager");
        gui_element_shutdown();
        return NULL;
    }
    
    // Inicializar gerenciador
    memset(manager, 0, sizeof(struct gui_manager_s));
    manager->next_callback_id = 1;
    manager->focused_element = GUI_INVALID_ID;
    manager->hovered_element = GUI_INVALID_ID;
    manager->initialized = true;
    
    GUI_LOG_INFO("GUI manager initialized");
    return manager;
}

void gui_manager_shutdown(gui_manager_t manager) {
    if (!manager || !manager->initialized) {
        return;
    }
    
    // Destruir todos os elementos
    for (uint32_t i = 0; i < manager->element_count; i++) {
        if (manager->elements[i] != GUI_INVALID_ID) {
            gui_element_destroy(manager->elements[i]);
        }
    }
    
    // Finalizar sistema de elementos
    gui_element_shutdown();
    
    // Limpar e liberar gerenciador
    memset(manager, 0, sizeof(struct gui_manager_s));
    free(manager);
    
    GUI_LOG_INFO("GUI manager shutdown");
}

gui_element_id_t gui_manager_add_element(gui_manager_t manager, gui_element_type_t type, const gui_rect_t* rect, const char* text) {
    if (!manager || !manager->initialized) {
        GUI_LOG_ERROR("Invalid GUI manager");
        return GUI_INVALID_ID;
    }
    
    if (manager->element_count >= GUI_MANAGER_MAX_ELEMENTS) {
        GUI_LOG_ERROR("Maximum number of GUI elements reached");
        return GUI_INVALID_ID;
    }
    
    // Criar elemento
    gui_element_id_t element_id = gui_element_create(type);
    
    if (element_id == GUI_INVALID_ID) {
        GUI_LOG_ERROR("Failed to create GUI element");
        return GUI_INVALID_ID;
    }
    
    // Configurar elemento
    gui_element_s* element = gui_element_get(element_id);
    
    if (!element) {
        GUI_LOG_ERROR("Failed to get GUI element");
        gui_element_destroy(element_id);
        return GUI_INVALID_ID;
    }
    
    if (rect) {
        element->rect = *rect;
    }
    
    if (text) {
        gui_element_set_text(element_id, text);
    }
    
    // Adicionar ao gerenciador
    manager->elements[manager->element_count++] = element_id;
    
    GUI_LOG_DEBUG("Added element %u to GUI manager", element_id);
    return element_id;
}

void gui_manager_remove_element(gui_manager_t manager, gui_element_id_t element_id) {
    if (!manager || !manager->initialized || element_id == GUI_INVALID_ID) {
        return;
    }
    
    // Encontrar o elemento
    bool found = false;
    uint32_t index = 0;
    
    for (uint32_t i = 0; i < manager->element_count; i++) {
        if (manager->elements[i] == element_id) {
            found = true;
            index = i;
            break;
        }
    }
    
    if (!found) {
        GUI_LOG_ERROR("Element %u not found in GUI manager", element_id);
        return;
    }
    
    // Atualizar elementos focados/hover
    if (manager->focused_element == element_id) {
        manager->focused_element = GUI_INVALID_ID;
    }
    
    if (manager->hovered_element == element_id) {
        manager->hovered_element = GUI_INVALID_ID;
    }
    
    // Destruir elemento
    gui_element_destroy(element_id);
    
    // Remover do gerenciador
    for (uint32_t i = index; i < manager->element_count - 1; i++) {
        manager->elements[i] = manager->elements[i + 1];
    }
    
    manager->element_count--;
    
    GUI_LOG_DEBUG("Removed element %u from GUI manager", element_id);
}

gui_element_s* gui_manager_get_element(gui_manager_t manager, gui_element_id_t element_id) {
    if (!manager || !manager->initialized || element_id == GUI_INVALID_ID) {
        return NULL;
    }
    
    return gui_element_get(element_id);
}

uint32_t gui_manager_register_callback(gui_manager_t manager, gui_event_type_t event_type, gui_event_callback_t callback, void* user_data) {
    if (!manager || !manager->initialized || !callback || event_type >= GUI_EVENT_MAX) {
        return 0;
    }
    
    if (manager->callback_count >= GUI_MANAGER_MAX_CALLBACKS) {
        GUI_LOG_ERROR("Maximum number of GUI callbacks reached");
        return 0;
    }
    
    // Encontrar slot livre
    uint32_t index = manager->callback_count;
    for (uint32_t i = 0; i < manager->callback_count; i++) {
        if (!manager->callbacks[i].active) {
            index = i;
            break;
        }
    }
    
    // Configurar callback
    gui_callback_s* cb = &manager->callbacks[index];
    cb->id = manager->next_callback_id++;
    cb->event_type = event_type;
    cb->callback = callback;
    cb->user_data = user_data;
    cb->active = true;
    
    if (index == manager->callback_count) {
        manager->callback_count++;
    }
    
    GUI_LOG_DEBUG("Registered callback %u for event type %d", cb->id, event_type);
    return cb->id;
}

bool gui_manager_unregister_callback(gui_manager_t manager, uint32_t callback_id) {
    if (!manager || !manager->initialized || callback_id == 0) {
        return false;
    }
    
    // Encontrar callback
    for (uint32_t i = 0; i < manager->callback_count; i++) {
        if (manager->callbacks[i].active && manager->callbacks[i].id == callback_id) {
            manager->callbacks[i].active = false;
            GUI_LOG_DEBUG("Unregistered callback %u", callback_id);
            return true;
        }
    }
    
    GUI_LOG_ERROR("Callback %u not found", callback_id);
    return false;
}

bool gui_manager_process_event(gui_manager_t manager, const gui_event_t* event) {
    if (!manager || !manager->initialized || !event) {
        return false;
    }
    
    bool handled = false;
    
    // Processar evento para elementos
    handled = gui_element_process_event(event);
    
    // Processar callbacks
    for (uint32_t i = 0; i < manager->callback_count; i++) {
        gui_callback_s* cb = &manager->callbacks[i];
        
        if (cb->active && (cb->event_type == event->type || cb->event_type == GUI_EVENT_NONE)) {
            if (cb->callback(manager, event, cb->user_data)) {
                handled = true;
                break;
            }
        }
    }
    
    return handled;
}

void gui_manager_update(gui_manager_t manager) {
    if (!manager || !manager->initialized) {
        return;
    }
    
    gui_element_update();
}

void gui_manager_render(gui_manager_t manager, SDL_Renderer* renderer) {
    if (!manager || !manager->initialized || !renderer) {
        return;
    }
    
    gui_element_render(renderer);
}

bool gui_manager_convert_sdl_event(const SDL_Event* sdl_event, gui_event_t* gui_event) {
    if (!sdl_event || !gui_event) {
        return false;
    }
    
    memset(gui_event, 0, sizeof(gui_event_t));
    gui_event->target = GUI_INVALID_ID;
    
    switch (sdl_event->type) {
        case SDL_MOUSEMOTION:
            gui_event->type = GUI_EVENT_MOUSE_MOVE;
            gui_event->mouse.position.x = sdl_event->motion.x;
            gui_event->mouse.position.y = sdl_event->motion.y;
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            gui_event->type = GUI_EVENT_MOUSE_DOWN;
            gui_event->mouse.position.x = sdl_event->button.x;
            gui_event->mouse.position.y = sdl_event->button.y;
            gui_event->mouse.button = sdl_event->button.button;
            gui_event->mouse.clicks = sdl_event->button.clicks;
            break;
            
        case SDL_MOUSEBUTTONUP:
            gui_event->type = GUI_EVENT_MOUSE_UP;
            gui_event->mouse.position.x = sdl_event->button.x;
            gui_event->mouse.position.y = sdl_event->button.y;
            gui_event->mouse.button = sdl_event->button.button;
            gui_event->mouse.clicks = sdl_event->button.clicks;
            break;
            
        case SDL_KEYDOWN:
            gui_event->type = GUI_EVENT_KEY_DOWN;
            gui_event->key.key_code = sdl_event->key.keysym.sym;
            gui_event->key.scan_code = sdl_event->key.keysym.scancode;
            gui_event->key.modifiers = sdl_event->key.keysym.mod;
            gui_event->key.repeat = sdl_event->key.repeat;
            break;
            
        case SDL_KEYUP:
            gui_event->type = GUI_EVENT_KEY_UP;
            gui_event->key.key_code = sdl_event->key.keysym.sym;
            gui_event->key.scan_code = sdl_event->key.keysym.scancode;
            gui_event->key.modifiers = sdl_event->key.keysym.mod;
            gui_event->key.repeat = sdl_event->key.repeat;
            break;
            
        default:
            return false;
    }
    
    return true;
}

gui_element_id_t gui_manager_find_element_at(gui_manager_t manager, int32_t x, int32_t y) {
    if (!manager || !manager->initialized) {
        return GUI_INVALID_ID;
    }
    
    // Percorrer elementos em ordem inversa (de cima para baixo na pilha de renderização)
    for (int32_t i = manager->element_count - 1; i >= 0; i--) {
        gui_element_id_t element_id = manager->elements[i];
        gui_element_s* element = gui_element_get(element_id);
        
        if (element && element->visible && element->enabled) {
            if (gui_point_in_rect(x, y, &element->rect)) {
                return element_id;
            }
        }
    }
    
    return GUI_INVALID_ID;
}
