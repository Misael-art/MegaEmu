#ifndef GUI_SDL2_BACKEND_H
#define GUI_SDL2_BACKEND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "gui_types.h"

// Estrutura de tamanho
typedef struct gui_size_s
{
    int width;
    int height;
} gui_size_t;

// Estrutura do backend SDL2
typedef struct gui_sdl2_backend_s
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *target;
    gui_size_t window_size;
    bool vsync_enabled;
    float scale_factor;
} gui_sdl2_backend_t;

// Resultado das operações
typedef gui_error_t gui_result_t;

// Inicialização e finalização
gui_result_t gui_sdl2_init(gui_sdl2_backend_t *backend, const char *title,
                           gui_size_t size, bool vsync);
void gui_sdl2_shutdown(gui_sdl2_backend_t *backend);

// Funções de renderização
gui_result_t gui_sdl2_begin_frame(gui_sdl2_backend_t *backend);
gui_result_t gui_sdl2_end_frame(gui_sdl2_backend_t *backend);
gui_result_t gui_sdl2_draw_rect(gui_sdl2_backend_t *backend,
                                gui_rect_t rect, gui_color_t color);
gui_result_t gui_sdl2_draw_text(gui_sdl2_backend_t *backend,
                                const char *text, gui_point_t pos,
                                gui_color_t color);

// Processamento de eventos
typedef gui_result_t (*gui_event_handler_t)(const gui_event_t *event, void *user_data);
gui_result_t gui_sdl2_process_events(gui_sdl2_backend_t *backend,
                                     gui_event_handler_t handler);

#endif // GUI_SDL2_BACKEND_H
