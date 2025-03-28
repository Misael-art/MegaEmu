#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gui_common.h"
#include "gui_element.h"
#include "gui_manager.h"
#include "gui_types.h"

#define GUI_MAX_ELEMENTS 256

typedef struct
{
    bool initialized;
    uint32_t element_count;
    gui_element_id_t next_id;
    gui_element_s *elements[GUI_MAX_ELEMENTS];
} gui_manager_t;

static gui_manager_t g_manager = {0};

bool gui_manager_init(void)
{
    if (g_manager.initialized)
    {
        return true;
    }
    memset(&g_manager, 0, sizeof(gui_manager_t));
    g_manager.initialized = true;
    g_manager.next_id = 1;
    GUI_LOG_INFO("GUI Manager initialized");
    return true;
}

void gui_manager_shutdown(void)
{
    if (!g_manager.initialized)
    {
        return;
    }
    for (uint32_t i = 0; i < g_manager.element_count; i++)
    {
        if (g_manager.elements[i])
        {
            gui_element_destroy(g_manager.elements[i]->id);
        }
    }
    memset(&g_manager, 0, sizeof(gui_manager_t));
    GUI_LOG_INFO("GUI Manager shutdown");
}

gui_element_id_t gui_manager_add_element(gui_element_type_t type)
{
    if (!g_manager.initialized)
    {
        GUI_LOG_ERROR("GUI Manager not initialized");
        return GUI_INVALID_ID;
    }
    if (g_manager.element_count >= GUI_MAX_ELEMENTS)
    {
        GUI_LOG_ERROR("Maximum number of GUI elements reached");
        return GUI_INVALID_ID;
    }
    gui_element_id_t id = g_manager.next_id++;
    gui_element_s *element = gui_element_create(type);
    if (!element)
    {
        GUI_LOG_ERROR("Failed to create GUI element");
        return GUI_INVALID_ID;
    }
    element->id = id;
    g_manager.elements[g_manager.element_count++] = element;
    GUI_LOG_DEBUG("Added GUI element %u of type %d", id, type);
    return id;
}

void gui_manager_remove_element(gui_element_id_t id)
{
    if (!g_manager.initialized || id == GUI_INVALID_ID)
    {
        return;
    }
    for (uint32_t i = 0; i < g_manager.element_count; i++)
    {
        if (g_manager.elements[i] && g_manager.elements[i]->id == id)
        {
            gui_element_destroy(id);
            g_manager.elements[i] = NULL;
            // Compactar o array
            for (uint32_t j = i; j < g_manager.element_count - 1; j++)
            {
                g_manager.elements[j] = g_manager.elements[j + 1];
            }
            g_manager.element_count--;
            GUI_LOG_DEBUG("Removed GUI element %u", id);
            return;
        }
    }
}

gui_element_s *gui_manager_get_element(gui_element_id_t id)
{
    if (!g_manager.initialized || id == GUI_INVALID_ID)
    {
        return NULL;
    }
    for (uint32_t i = 0; i < g_manager.element_count; i++)
    {
        if (g_manager.elements[i] && g_manager.elements[i]->id == id)
        {
            return g_manager.elements[i];
        }
    }
    return NULL;
}

bool gui_manager_add_child(gui_element_id_t parent_id, gui_element_id_t child_id)
{
    gui_element_s *parent = gui_manager_get_element(parent_id);
    gui_element_s *child = gui_manager_get_element(child_id);
    if (!parent || !child)
    {
        GUI_LOG_ERROR("Invalid parent or child element");
        return false;
    }
    return gui_element_add_child(parent_id, child_id);
}

bool gui_manager_process_events(const gui_event_s *event)
{
    if (!g_manager.initialized || !event)
    {
        return false;
    }
    // Processar eventos de trás para frente para que os elementos superiores
    // tenham prioridade
    for (int32_t i = g_manager.element_count - 1; i >= 0; i--)
    {
        if (g_manager.elements[i] && gui_element_process_event(event))
        {
            return true;
        }
    }
    return false;
}

void gui_manager_update(void)
{
    if (!g_manager.initialized)
    {
        return;
    }
    for (uint32_t i = 0; i < g_manager.element_count; i++)
    {
        if (g_manager.elements[i])
        {
            gui_element_update();
        }
    }
}

void gui_manager_render(SDL_Renderer *renderer)
{
    if (!g_manager.initialized || !renderer)
    {
        return;
    }
    for (uint32_t i = 0; i < g_manager.element_count; i++)
    {
        if (g_manager.elements[i])
        {
            gui_element_render(renderer);
        }
    }
}
