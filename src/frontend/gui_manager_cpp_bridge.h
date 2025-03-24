/**
 * @file gui_manager_cpp_bridge.h
 * @brief Interface C++ para o gerenciador de GUI
 */
#ifndef GUI_MANAGER_CPP_BRIDGE_H
#define GUI_MANAGER_CPP_BRIDGE_H

#include <SDL2/SDL.h>
#include "gui/gui_types.h"
#include "renderer/text_renderer.h"

// Estrutura de evento de mouse para C++
typedef struct
{
    int x;
    int y;
    int button;
} cpp_gui_mouse_event_t;

// Estrutura de evento GUI para C++
typedef struct
{
    int type;
    cpp_gui_mouse_event_t mouse;
} cpp_gui_event_t;

// Definições de eventos de mouse para C++
#define GUI_EVENT_MOUSE_MOVE 1
#define GUI_EVENT_MOUSE_DOWN 2
#define GUI_EVENT_MOUSE_UP 3
#define GUI_EVENT_KEY_DOWN 4
#define GUI_EVENT_KEY_UP 5

// Definições de tamanhos de texto para C++
#define TEXT_SIZE_SMALL TEXT_SIZE_SMALL
#define TEXT_SIZE_MEDIUM TEXT_SIZE_NORMAL
#define TEXT_SIZE_LARGE TEXT_SIZE_LARGE
#define TEXT_SIZE_XLARGE TEXT_SIZE_LARGE

#ifdef __cplusplus
extern "C"
{
#endif

    // Funções de inicialização/finalização
    bool gui_manager_init(void);
    void gui_manager_shutdown(void);

    // Funções de janela
    SDL_Window *gui_manager_create_window(const char *title, int width, int height);
    void gui_manager_destroy_window(void);
    SDL_Renderer *gui_manager_get_renderer(void);
    void gui_manager_set_window_size(int width, int height);
    void gui_manager_get_window_size(int *width, int *height);

    // Funções de evento
    bool gui_manager_process_events(void);
    void gui_manager_handle_event(SDL_Event *event);

    // Funções de renderização
    void gui_manager_begin_frame(void);
    void gui_manager_end_frame(void);
    void gui_manager_render(void);

    // Funções de estado
    void gui_manager_set_running(bool running);
    bool gui_manager_is_running(void);

    // Funções para salvar estados
    bool gui_save_state_get_state();
    void gui_save_state_set_state(bool state);
    void render_save_state();

    // Callback para salvar estado
    typedef void (*on_save_state_callback_t)(bool success);
    void on_save_state_complete(bool success);

#ifdef __cplusplus
}
#endif

#endif // GUI_MANAGER_CPP_BRIDGE_H
