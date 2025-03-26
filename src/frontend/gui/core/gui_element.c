/**
 * @file gui_element.c
 * @brief Implementação das funções de manipulação de elementos da GUI
 */
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui_common.h"
#include "gui_element.h"
#include "gui_types.h"

// Estrutura interna para elementos
typedef struct {
    gui_element_s base;
    void *renderer_data;
} gui_element_internal_t;

// Array de elementos
static gui_element_internal_t g_elements[GUI_MAX_ELEMENTS];
static uint32_t g_element_count = 0;
static gui_element_id_t g_next_id = 1;
static bool g_initialized = false;

// Funções auxiliares para manipulação de propriedades
static gui_property_t* find_property(gui_element_s *element, const char *name) {
    if (!element || !name) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < element->property_count; i++) {
        if (strcmp(element->properties[i].name, name) == 0) {
            return &element->properties[i];
        }
    }
    
    return NULL;
}

static bool add_property(gui_element_s *element, const char *name, gui_property_type_t type, const void *value) {
    if (!element || !name || element->property_count >= GUI_MAX_PROPERTIES) {
        return false;
    }
    
    // Verificar se a propriedade já existe
    gui_property_t *prop = find_property(element, name);
    if (prop) {
        // Limpar valor anterior se for string
        if (prop->type == GUI_PROPERTY_STRING && prop->value.string) {
            free(prop->value.string);
            prop->value.string = NULL;
        }
        
        // Atualizar tipo e valor
        prop->type = type;
    } else {
        // Adicionar nova propriedade
        prop = &element->properties[element->property_count++];
        strncpy(prop->name, name, GUI_MAX_PROPERTY_NAME - 1);
        prop->name[GUI_MAX_PROPERTY_NAME - 1] = '\0';
        prop->type = type;
    }
    
    // Definir valor
    switch (type) {
        case GUI_PROPERTY_INT:
            prop->value.integer = *(const int*)value;
            break;
        case GUI_PROPERTY_FLOAT:
            prop->value.floating = *(const float*)value;
            break;
        case GUI_PROPERTY_BOOL:
            prop->value.boolean = *(const bool*)value;
            break;
        case GUI_PROPERTY_STRING:
            prop->value.string = gui_strdup((const char*)value);
            if (!prop->value.string) {
                return false;
            }
            break;
        case GUI_PROPERTY_COLOR:
            prop->value.color = *(const gui_color_t*)value;
            break;
        case GUI_PROPERTY_POINTER:
            prop->value.pointer = *(void**)value;
            break;
        default:
            return false;
    }
    
    return true;
}

// Implementação das funções públicas

bool gui_element_init(void) {
    if (g_initialized) {
        return true;
    }
    
    memset(g_elements, 0, sizeof(g_elements));
    g_element_count = 0;
    g_next_id = 1;
    g_initialized = true;
    
    GUI_LOG_INFO("GUI Element system initialized");
    return true;
}

void gui_element_shutdown(void) {
    if (!g_initialized) {
        return;
    }
    
    for (uint32_t i = 0; i < g_element_count; i++) {
        if (g_elements[i].base.id != GUI_INVALID_ID) {
            gui_element_destroy(g_elements[i].base.id);
        }
    }
    
    memset(g_elements, 0, sizeof(g_elements));
    g_element_count = 0;
    g_initialized = false;
    
    GUI_LOG_INFO("GUI Element system shutdown");
}

gui_element_id_t gui_element_create(gui_element_type_t type) {
    if (!g_initialized) {
        GUI_LOG_ERROR("GUI Element system not initialized");
        return GUI_INVALID_ID;
    }
    
    if (g_element_count >= GUI_MAX_ELEMENTS) {
        GUI_LOG_ERROR("Maximum number of GUI elements reached");
        return GUI_INVALID_ID;
    }
    
    // Encontrar slot livre
    uint32_t index = g_element_count;
    for (uint32_t i = 0; i < g_element_count; i++) {
        if (g_elements[i].base.id == GUI_INVALID_ID) {
            index = i;
            break;
        }
    }
    
    // Inicializar elemento
    gui_element_internal_t *element = &g_elements[index];
    memset(element, 0, sizeof(gui_element_internal_t));
    
    element->base.id = g_next_id++;
    element->base.type = type;
    element->base.visible = true;
    element->base.enabled = true;
    element->base.parent = GUI_INVALID_ID;
    
    // Configurar funções virtuais com base no tipo
    switch (type) {
        case GUI_ELEMENT_BUTTON:
            // Implementação específica para botões seria definida aqui
            break;
        case GUI_ELEMENT_LABEL:
            // Implementação específica para labels seria definida aqui
            break;
        // Outros tipos...
        default:
            break;
    }
    
    if (index == g_element_count) {
        g_element_count++;
    }
    
    GUI_LOG_DEBUG("Created GUI element %u of type %d", element->base.id, type);
    return element->base.id;
}

void gui_element_destroy(gui_element_id_t id) {
    if (!g_initialized || id == GUI_INVALID_ID) {
        return;
    }
    
    gui_element_internal_t *element = NULL;
    uint32_t index = UINT32_MAX;
    
    // Encontrar o elemento
    for (uint32_t i = 0; i < g_element_count; i++) {
        if (g_elements[i].base.id == id) {
            element = &g_elements[i];
            index = i;
            break;
        }
    }
    
    if (!element) {
        GUI_LOG_ERROR("GUI element %u not found", id);
        return;
    }
    
    // Remover de qualquer pai
    if (element->base.parent != GUI_INVALID_ID) {
        gui_element_remove_child(element->base.parent, id);
    }
    
    // Destruir filhos
    for (uint32_t i = 0; i < element->base.child_count; i++) {
        gui_element_destroy(element->base.children[i]);
    }
    
    // Limpar propriedades
    for (uint32_t i = 0; i < element->base.property_count; i++) {
        if (element->base.properties[i].type == GUI_PROPERTY_STRING && 
            element->base.properties[i].value.string) {
            free(element->base.properties[i].value.string);
        }
    }
    
    // Chamar função de destruição específica do tipo
    if (element->base.destroy) {
        element->base.destroy(&element->base);
    }
    
    // Liberar dados do renderizador
    if (element->renderer_data) {
        free(element->renderer_data);
    }
    
    // Limpar o elemento
    memset(element, 0, sizeof(gui_element_internal_t));
    
    GUI_LOG_DEBUG("Destroyed GUI element %u", id);
}

gui_element_s* gui_element_get(gui_element_id_t id) {
    if (!g_initialized || id == GUI_INVALID_ID) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < g_element_count; i++) {
        if (g_elements[i].base.id == id) {
            return &g_elements[i].base;
        }
    }
    
    return NULL;
}

bool gui_element_add_child(gui_element_id_t parent_id, gui_element_id_t child_id) {
    if (!g_initialized || parent_id == GUI_INVALID_ID || child_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_s *parent = gui_element_get(parent_id);
    gui_element_s *child = gui_element_get(child_id);
    
    if (!parent || !child) {
        GUI_LOG_ERROR("Parent or child element not found");
        return false;
    }
    
    if (parent->child_count >= GUI_MAX_CHILDREN) {
        GUI_LOG_ERROR("Maximum number of children reached for element %u", parent_id);
        return false;
    }
    
    // Verificar se o filho já está associado a outro pai
    if (child->parent != GUI_INVALID_ID) {
        gui_element_remove_child(child->parent, child_id);
    }
    
    // Adicionar filho ao pai
    parent->children[parent->child_count++] = child_id;
    child->parent = parent_id;
    
    GUI_LOG_DEBUG("Added element %u as child of element %u", child_id, parent_id);
    return true;
}

bool gui_element_remove_child(gui_element_id_t parent_id, gui_element_id_t child_id) {
    if (!g_initialized || parent_id == GUI_INVALID_ID || child_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_s *parent = gui_element_get(parent_id);
    gui_element_s *child = gui_element_get(child_id);
    
    if (!parent || !child) {
        GUI_LOG_ERROR("Parent or child element not found");
        return false;
    }
    
    // Verificar se o filho pertence ao pai
    bool found = false;
    uint32_t index = 0;
    
    for (uint32_t i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child_id) {
            found = true;
            index = i;
            break;
        }
    }
    
    if (!found) {
        GUI_LOG_ERROR("Element %u is not a child of element %u", child_id, parent_id);
        return false;
    }
    
    // Remover filho do pai
    for (uint32_t i = index; i < parent->child_count - 1; i++) {
        parent->children[i] = parent->children[i + 1];
    }
    
    parent->child_count--;
    child->parent = GUI_INVALID_ID;
    
    GUI_LOG_DEBUG("Removed element %u as child of element %u", child_id, parent_id);
    return true;
}

bool gui_element_set_text(gui_element_id_t id, const char* text) {
    if (!g_initialized || id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_s *element = gui_element_get(id);
    
    if (!element) {
        GUI_LOG_ERROR("Element %u not found", id);
        return false;
    }
    
    if (!text) {
        element->text[0] = '\0';
    } else {
        strncpy(element->text, text, GUI_MAX_TEXT - 1);
        element->text[GUI_MAX_TEXT - 1] = '\0';
    }
    
    return true;
}

bool gui_element_set_position(gui_element_id_t id, int32_t x, int32_t y) {
    if (!g_initialized || id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_s *element = gui_element_get(id);
    
    if (!element) {
        GUI_LOG_ERROR("Element %u not found", id);
        return false;
    }
    
    element->rect.x = x;
    element->rect.y = y;
    
    return true;
}

bool gui_element_set_size(gui_element_id_t id, int32_t width, int32_t height) {
    if (!g_initialized || id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_s *element = gui_element_get(id);
    
    if (!element) {
        GUI_LOG_ERROR("Element %u not found", id);
        return false;
    }
    
    element->rect.w = width;
    element->rect.h = height;
    
    return true;
}

bool gui_element_set_visible(gui_element_id_t id, bool visible) {
    if (!g_initialized || id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_s *element = gui_element_get(id);
    
    if (!element) {
        GUI_LOG_ERROR("Element %u not found", id);
        return false;
    }
    
    element->visible = visible;
    
    return true;
}

bool gui_element_set_enabled(gui_element_id_t id, bool enabled) {
    if (!g_initialized || id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_s *element = gui_element_get(id);
    
    if (!element) {
        GUI_LOG_ERROR("Element %u not found", id);
        return false;
    }
    
    element->enabled = enabled;
    
    return true;
}

bool gui_element_set_property(gui_element_id_t id, const char* name, gui_property_type_t type, const void* value) {
    if (!g_initialized || id == GUI_INVALID_ID || !name || !value) {
        return false;
    }
    
    gui_element_s *element = gui_element_get(id);
    
    if (!element) {
        GUI_LOG_ERROR("Element %u not found", id);
        return false;
    }
    
    return add_property(element, name, type, value);
}

bool gui_element_get_property(gui_element_id_t id, const char* name, gui_property_type_t* type, void* value) {
    if (!g_initialized || id == GUI_INVALID_ID || !name || !type || !value) {
        return false;
    }
    
    gui_element_s *element = gui_element_get(id);
    
    if (!element) {
        GUI_LOG_ERROR("Element %u not found", id);
        return false;
    }
    
    gui_property_t *prop = find_property(element, name);
    
    if (!prop) {
        return false;
    }
    
    *type = prop->type;
    
    switch (prop->type) {
        case GUI_PROPERTY_INT:
            *(int*)value = prop->value.integer;
            break;
        case GUI_PROPERTY_FLOAT:
            *(float*)value = prop->value.floating;
            break;
        case GUI_PROPERTY_BOOL:
            *(bool*)value = prop->value.boolean;
            break;
        case GUI_PROPERTY_STRING:
            *(char**)value = prop->value.string;
            break;
        case GUI_PROPERTY_COLOR:
            *(gui_color_t*)value = prop->value.color;
            break;
        case GUI_PROPERTY_POINTER:
            *(void**)value = prop->value.pointer;
            break;
        default:
            return false;
    }
    
    return true;
}

bool gui_element_process_event(const gui_event_t* event) {
    if (!g_initialized || !event) {
        return false;
    }
    
    // Processar evento para o elemento alvo, se especificado
    if (event->target != GUI_INVALID_ID) {
        gui_element_s *element = gui_element_get(event->target);
        
        if (element && element->process_event) {
            return element->process_event(element, event);
        }
    }
    
    // Processar evento para todos os elementos de nível superior
    bool handled = false;
    
    for (uint32_t i = 0; i < g_element_count; i++) {
        gui_element_s *element = &g_elements[i].base;
        
        if (element->id != GUI_INVALID_ID && element->parent == GUI_INVALID_ID && element->process_event) {
            if (element->process_event(element, event)) {
                handled = true;
                break;
            }
        }
    }
    
    return handled;
}

void gui_element_update(void) {
    if (!g_initialized) {
        return;
    }
    
    for (uint32_t i = 0; i < g_element_count; i++) {
        gui_element_s *element = &g_elements[i].base;
        
        if (element->id != GUI_INVALID_ID && element->update) {
            element->update(element);
        }
    }
}

void gui_element_render(SDL_Renderer* renderer) {
    if (!g_initialized || !renderer) {
        return;
    }
    
    // Renderizar apenas elementos de nível superior
    for (uint32_t i = 0; i < g_element_count; i++) {
        gui_element_s *element = &g_elements[i].base;
        
        if (element->id != GUI_INVALID_ID && element->parent == GUI_INVALID_ID && element->visible && element->render) {
            element->render(element, renderer);
        }
    }
}
