#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "gui_common.h"
#include "gui_element.h"
#include "gui_manager.h"
#include "gui_save_state.h"
#include "gui_types.h"

#define GUI_MAX_SAVE_SLOTS 10
#define GUI_SAVE_DIR "saves"
#define GUI_SAVE_EXT ".sav"

typedef struct
{
    char path[256];
    char name[32];
    bool used;
    uint32_t timestamp;
} save_slot_s;

typedef struct
{
    save_slot_s slots[GUI_MAX_SAVE_SLOTS];
    int32_t selected_slot;
    SDL_Rect viewport;
    SDL_Color text_color;
    gui_element_id_t window_id;
    bool visible;
    void (*callback)(bool success, void *user_data);
    void *user_data;
} gui_save_state_s;

static gui_save_state_s g_save_state = {0};

static bool create_save_directory(void)
{
#ifdef _WIN32
    return (mkdir(GUI_SAVE_DIR) == 0 || errno == EEXIST);
#else
    return (mkdir(GUI_SAVE_DIR, 0755) == 0 || errno == EEXIST);
#endif
}

static void update_slot_info(int32_t slot)
{
    if (slot < 0 || slot >= GUI_MAX_SAVE_SLOTS)
    {
        return;
    }
    save_slot_s *save_slot = &g_save_state.slots[slot];
    snprintf(save_slot->path, sizeof(save_slot->path), "%s/slot%d%s",
             GUI_SAVE_DIR, slot, GUI_SAVE_EXT);
    FILE *file = fopen(save_slot->path, "rb");
    if (file)
    {
        struct stat st;
        if (stat(save_slot->path, &st) == 0)
        {
            save_slot->used = true;
            save_slot->timestamp = (uint32_t)st.st_mtime;
            time_t t = st.st_mtime;
            strftime(save_slot->name, sizeof(save_slot->name),
                     "Slot %d - %Y-%m-%d %H:%M:%S", localtime(&t));
        }
        fclose(file);
    }
    else
    {
        save_slot->used = false;
        snprintf(save_slot->name, sizeof(save_slot->name), "Slot %d - Empty", slot);
    }
    gui_element_s *button = gui_manager_get_element(g_save_state.window_id);
    if (button)
    {
        gui_element_set_text(button, save_slot->name);
    }
}

bool gui_save_state_init(void)
{
    if (!create_save_directory())
    {
        GUI_LOG_ERROR("Failed to create save directory");
        return false;
    }
    memset(&g_save_state, 0, sizeof(gui_save_state_s));
    g_save_state.selected_slot = -1;
    g_save_state.text_color = (SDL_Color){255, 255, 255, 255};
    // Criar janela principal
    g_save_state.window_id = gui_manager_add_element(GUI_ELEMENT_WINDOW);
    if (g_save_state.window_id == GUI_INVALID_ID)
    {
        GUI_LOG_ERROR("Failed to create save state window");
        return false;
    }
    gui_element_s *window = gui_manager_get_element(g_save_state.window_id);
    gui_element_set_position(window, 100, 100);
    gui_element_set_size(window, 400, 300);
    gui_element_set_visible(window, false);
    // Criar slots
    for (int32_t i = 0; i < GUI_MAX_SAVE_SLOTS; i++)
    {
        gui_element_id_t button_id = gui_manager_add_element(GUI_ELEMENT_BUTTON);
        if (button_id == GUI_INVALID_ID)
        {
            GUI_LOG_ERROR("Failed to create save slot button");
            continue;
        }
        gui_element_s *button = gui_manager_get_element(button_id);
        gui_element_set_position(button, 10, 10 + i * 30);
        gui_element_set_size(button, 380, 25);
        gui_element_set_text(button, "Empty");
        gui_manager_add_child(window, button);
        update_slot_info(i);
    }
    return true;
}

void gui_save_state_shutdown(void)
{
    if (g_save_state.window_id != GUI_INVALID_ID)
    {
        gui_manager_remove_element(g_save_state.window_id);
        g_save_state.window_id = GUI_INVALID_ID;
    }
    memset(&g_save_state, 0, sizeof(gui_save_state_s));
}

void gui_save_state_show(bool show)
{
    if (g_save_state.window_id == GUI_INVALID_ID)
    {
        return;
    }
    gui_element_s *window = gui_manager_get_element(g_save_state.window_id);
    if (window)
    {
        gui_element_set_visible(window, show);
        g_save_state.visible = show;
        if (show)
        {
            for (int32_t i = 0; i < GUI_MAX_SAVE_SLOTS; i++)
            {
                update_slot_info(i);
            }
        }
    }
}

bool gui_save_state_save(const void *data, size_t size)
{
    if (!data || size == 0 || g_save_state.selected_slot < 0 ||
        g_save_state.selected_slot >= GUI_MAX_SAVE_SLOTS)
    {
        return false;
    }
    save_slot_s *slot = &g_save_state.slots[g_save_state.selected_slot];
    FILE *file = fopen(slot->path, "wb");
    if (!file)
    {
        return false;
    }
    bool success = (fwrite(data, 1, size, file) == size);
    fclose(file);
    if (success)
    {
        update_slot_info(g_save_state.selected_slot);
    }
    return success;
}

bool gui_save_state_load(void *data, size_t size)
{
    if (!data || size == 0 || g_save_state.selected_slot < 0 ||
        g_save_state.selected_slot >= GUI_MAX_SAVE_SLOTS)
    {
        return false;
    }
    save_slot_s *slot = &g_save_state.slots[g_save_state.selected_slot];
    if (!slot->used)
    {
        return false;
    }
    FILE *file = fopen(slot->path, "rb");
    if (!file)
    {
        return false;
    }
    bool success = (fread(data, 1, size, file) == size);
    fclose(file);
    return success;
}

bool gui_save_state_process_event(const gui_event_s *event)
{
    if (!g_save_state.visible || !event)
    {
        return false;
    }
    gui_element_s *window = gui_manager_get_element(g_save_state.window_id);
    if (!window)
    {
        return false;
    }
    if (gui_element_process_event(window, event))
    {
        return true;
    }
    if (event->type == GUI_EVENT_MOUSE_BUTTON &&
        event->data.mouse_button.state == GUI_BUTTON_PRESSED)
    {
        for (int32_t i = 0; i < GUI_MAX_SAVE_SLOTS; i++)
        {
            gui_element_s *button = gui_manager_get_element(g_save_state.slots[i].id);
            if (button && gui_point_in_rect(&event->data.mouse_button.point, &button->rect))
            {
                g_save_state.selected_slot = i;
                if (g_save_state.callback)
                {
                    g_save_state.callback(true, g_save_state.user_data);
                }
                return true;
            }
        }
    }
    return false;
}

void gui_save_state_update(void)
{
    if (!g_save_state.visible)
    {
        return;
    }
    gui_element_s *window = gui_manager_get_element(g_save_state.window_id);
    if (window)
    {
        gui_element_update(window);
    }
}

void gui_save_state_render(SDL_Renderer *renderer)
{
    if (!g_save_state.visible || !renderer)
    {
        return;
    }
    gui_element_s *window = gui_manager_get_element(g_save_state.window_id);
    if (window)
    {
        gui_element_render(window, renderer);
    }
}
