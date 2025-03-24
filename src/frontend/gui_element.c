/**
 * @file gui_element.c
 * @brief Implementação das funções de gerenciamento de elementos da GUI
 */

#include "gui/gui_element.h"
#include "gui/gui_common.h"
#include <stdlib.h>
#include <string.h>

// Estrutura interna do elemento
typedef struct gui_element_s
{
    gui_element_id_t id;
    gui_element_type_t type;
    gui_rect_t rect;
    bool visible;
    bool enabled;
    char text[GUI_MAX_TEXT];

    // Hierarquia
    struct gui_element_s *parent;
    struct gui_element_s *children[GUI_MAX_CHILDREN];
    size_t num_children;

    // Propriedades
    gui_property_t *properties[GUI_MAX_PROPERTIES];
    size_t num_properties;

    // Callbacks
    gui_element_callback_t on_create;
    gui_element_callback_t on_destroy;
    gui_event_callback_t on_event;
    gui_update_callback_t on_update;
    gui_render_callback_t on_render;
} gui_element_t;

// Array global de elementos
static gui_element_t *g_elements[GUI_MAX_ELEMENTS] = {NULL};
static size_t g_num_elements = 0;
static gui_element_id_t g_next_id = 1;

// Funções auxiliares internas
static gui_element_t *get_element(gui_element_id_t id)
{
    if (id == GUI_INVALID_ID || id >= GUI_MAX_ELEMENTS)
        return NULL;
    return g_elements[id];
}

static gui_element_id_t allocate_element(void)
{
    if (g_num_elements >= GUI_MAX_ELEMENTS)
        return GUI_INVALID_ID;

    while (g_elements[g_next_id] != NULL)
    {
        g_next_id = (g_next_id + 1) % GUI_MAX_ELEMENTS;
        if (g_next_id == 0)
            g_next_id = 1; // Skip invalid ID 0
    }

    return g_next_id++;
}

// Implementação das funções públicas
bool gui_element_init(void)
{
    memset(g_elements, 0, sizeof(g_elements));
    g_num_elements = 0;
    g_next_id = 1;
    return true;
}

void gui_element_shutdown(void)
{
    for (size_t i = 0; i < GUI_MAX_ELEMENTS; i++)
    {
        if (g_elements[i])
        {
            gui_element_destroy(i);
        }
    }
}

gui_element_id_t gui_element_create(gui_element_type_t type)
{
    gui_element_id_t id = allocate_element();
    if (id == GUI_INVALID_ID)
        return GUI_INVALID_ID;

    gui_element_t *element = (gui_element_t *)calloc(1, sizeof(gui_element_t));
    if (!element)
        return GUI_INVALID_ID;

    element->id = id;
    element->type = type;
    element->visible = true;
    element->enabled = true;
    element->text[0] = '\0';

    g_elements[id] = element;
    g_num_elements++;

    if (element->on_create)
        element->on_create(element);

    return id;
}

void gui_element_destroy(gui_element_id_t id)
{
    gui_element_t *element = get_element(id);
    if (!element)
        return;

    if (element->on_destroy)
        element->on_destroy(element);

    // Destruir filhos
    for (size_t i = 0; i < element->num_children; i++)
    {
        if (element->children[i])
        {
            gui_element_destroy(element->children[i]->id);
        }
    }

    // Remover do pai
    if (element->parent)
    {
        gui_element_remove_child(element->parent->id, id);
    }

    // Liberar propriedades
    for (size_t i = 0; i < element->num_properties; i++)
    {
        if (element->properties[i])
        {
            if (element->properties[i]->type == GUI_PROPERTY_STRING &&
                element->properties[i]->value.string)
            {
                free(element->properties[i]->value.string);
            }
            free(element->properties[i]);
        }
    }

    g_elements[id] = NULL;
    free(element);
    g_num_elements--;
}

bool gui_element_add_child(gui_element_id_t parent_id, gui_element_id_t child_id)
{
    gui_element_t *parent = get_element(parent_id);
    gui_element_t *child = get_element(child_id);

    if (!parent || !child || parent->num_children >= GUI_MAX_CHILDREN)
        return false;

    // Remover do pai anterior
    if (child->parent)
    {
        gui_element_remove_child(child->parent->id, child_id);
    }

    parent->children[parent->num_children++] = child;
    child->parent = parent;
    return true;
}

bool gui_element_remove_child(gui_element_id_t parent_id, gui_element_id_t child_id)
{
    gui_element_t *parent = get_element(parent_id);
    if (!parent)
        return false;

    for (size_t i = 0; i < parent->num_children; i++)
    {
        if (parent->children[i] && parent->children[i]->id == child_id)
        {
            parent->children[i]->parent = NULL;

            // Mover os elementos restantes
            for (size_t j = i; j < parent->num_children - 1; j++)
            {
                parent->children[j] = parent->children[j + 1];
            }

            parent->children[parent->num_children - 1] = NULL;
            parent->num_children--;
            return true;
        }
    }

    return false;
}

bool gui_element_set_text(gui_element_id_t id, const char *text)
{
    gui_element_t *element = get_element(id);
    if (!element || !text)
        return false;

    strncpy(element->text, text, GUI_MAX_TEXT - 1);
    element->text[GUI_MAX_TEXT - 1] = '\0';
    return true;
}

bool gui_element_set_position(gui_element_id_t id, int32_t x, int32_t y)
{
    gui_element_t *element = get_element(id);
    if (!element)
        return false;

    element->rect.x = x;
    element->rect.y = y;
    return true;
}

bool gui_element_set_size(gui_element_id_t id, int32_t width, int32_t height)
{
    gui_element_t *element = get_element(id);
    if (!element)
        return false;

    element->rect.w = width;
    element->rect.h = height;
    return true;
}

bool gui_element_set_visible(gui_element_id_t id, bool visible)
{
    gui_element_t *element = get_element(id);
    if (!element)
        return false;

    element->visible = visible;
    return true;
}

bool gui_element_set_enabled(gui_element_id_t id, bool enabled)
{
    gui_element_t *element = get_element(id);
    if (!element)
        return false;

    element->enabled = enabled;
    return true;
}

bool gui_element_process_event(const gui_event_t *event)
{
    if (!event)
        return false;

    // Processar do elemento mais recente para o mais antigo
    for (int i = GUI_MAX_ELEMENTS - 1; i >= 0; i--)
    {
        gui_element_t *element = g_elements[i];
        if (!element || !element->visible || !element->enabled)
            continue;

        // Verificar se o evento é relevante para este elemento
        if (event->type == GUI_EVENT_MOUSE_BUTTON ||
            event->type == GUI_EVENT_MOUSE_MOTION)
        {
            if (!gui_point_in_rect(&element->rect,
                                   event->type == GUI_EVENT_MOUSE_BUTTON ? event->mouse_button.point.x : event->mouse_motion.point.x,
                                   event->type == GUI_EVENT_MOUSE_BUTTON ? event->mouse_button.point.y : event->mouse_motion.point.y))
            {
                continue;
            }
        }

        // Processar o evento
        if (element->on_event && element->on_event(element, event))
            return true;

        // Processar filhos
        for (size_t j = 0; j < element->num_children; j++)
        {
            if (element->children[j] &&
                gui_element_process_event(event))
                return true;
        }
    }

    return false;
}

void gui_element_update(void)
{
    for (size_t i = 0; i < GUI_MAX_ELEMENTS; i++)
    {
        gui_element_t *element = g_elements[i];
        if (!element || !element->visible)
            continue;

        if (element->on_update)
            element->on_update(element, 0.0f); // TODO: Passar delta time
    }
}

void gui_element_render(SDL_Renderer *renderer)
{
    if (!renderer)
        return;

    for (size_t i = 0; i < GUI_MAX_ELEMENTS; i++)
    {
        gui_element_t *element = g_elements[i];
        if (!element || !element->visible)
            continue;

        if (element->on_render)
            element->on_render(element, renderer);

        // Renderizar filhos
        for (size_t j = 0; j < element->num_children; j++)
        {
            if (element->children[j])
                gui_element_render(renderer);
        }
    }
}
