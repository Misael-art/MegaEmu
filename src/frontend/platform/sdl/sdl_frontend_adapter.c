/**
 * @file sdl_frontend_adapter.c
 * @brief Implementação do adaptador SDL para o frontend do Mega_Emu
 */
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../gui/core/gui_common.h"
#include "../../gui/core/gui_manager.h"
#include "../../gui/core/gui_types.h"
#include "sdl_frontend_adapter.h"

// Estrutura para armazenar o estado do adaptador SDL
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    gui_manager_t gui_manager;
    bool initialized;
    SDL_Color background_color;
    uint32_t last_update_time;
    uint32_t frame_delay;
} sdl_frontend_state_t;

// Estado global do adaptador
static sdl_frontend_state_t g_state = {0};

// Taxa de atualização alvo (60 FPS)
#define TARGET_FPS 60

bool sdl_frontend_init(const char* title, int32_t width, int32_t height) {
    if (g_state.initialized) {
        GUI_LOG_WARN("SDL frontend already initialized");
        return true;
    }
    
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        GUI_LOG_ERROR("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    
    // Criar janela
    g_state.window = SDL_CreateWindow(
        title ? title : "Mega_Emu",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width > 0 ? width : 800,
        height > 0 ? height : 600,
        SDL_WINDOW_SHOWN
    );
    
    if (!g_state.window) {
        GUI_LOG_ERROR("Failed to create SDL window: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    // Criar renderizador
    g_state.renderer = SDL_CreateRenderer(
        g_state.window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!g_state.renderer) {
        GUI_LOG_ERROR("Failed to create SDL renderer: %s", SDL_GetError());
        SDL_DestroyWindow(g_state.window);
        SDL_Quit();
        return false;
    }
    
    // Inicializar gerenciador de GUI
    g_state.gui_manager = gui_manager_init();
    
    if (!g_state.gui_manager) {
        GUI_LOG_ERROR("Failed to initialize GUI manager");
        SDL_DestroyRenderer(g_state.renderer);
        SDL_DestroyWindow(g_state.window);
        SDL_Quit();
        return false;
    }
    
    // Configurar estado
    g_state.initialized = true;
    g_state.background_color.r = 0;
    g_state.background_color.g = 0;
    g_state.background_color.b = 0;
    g_state.background_color.a = 255;
    g_state.last_update_time = SDL_GetTicks();
    g_state.frame_delay = 1000 / TARGET_FPS;
    
    GUI_LOG_INFO("SDL frontend initialized");
    return true;
}

void sdl_frontend_shutdown(void) {
    if (!g_state.initialized) {
        return;
    }
    
    // Finalizar gerenciador de GUI
    if (g_state.gui_manager) {
        gui_manager_shutdown(g_state.gui_manager);
        g_state.gui_manager = NULL;
    }
    
    // Liberar recursos SDL
    if (g_state.renderer) {
        SDL_DestroyRenderer(g_state.renderer);
        g_state.renderer = NULL;
    }
    
    if (g_state.window) {
        SDL_DestroyWindow(g_state.window);
        g_state.window = NULL;
    }
    
    SDL_Quit();
    
    // Limpar estado
    g_state.initialized = false;
    
    GUI_LOG_INFO("SDL frontend shutdown");
}

bool sdl_frontend_process_events(void) {
    if (!g_state.initialized) {
        return false;
    }
    
    SDL_Event sdl_event;
    gui_event_t gui_event;
    bool quit = false;
    
    while (SDL_PollEvent(&sdl_event)) {
        // Verificar eventos de saída
        if (sdl_event.type == SDL_QUIT) {
            quit = true;
            break;
        }
        
        // Converter evento SDL para evento GUI
        if (gui_manager_convert_sdl_event(&sdl_event, &gui_event)) {
            // Encontrar elemento alvo para eventos de mouse
            if (gui_event.type == GUI_EVENT_MOUSE_MOVE ||
                gui_event.type == GUI_EVENT_MOUSE_DOWN ||
                gui_event.type == GUI_EVENT_MOUSE_UP) {
                
                gui_event.target = gui_manager_find_element_at(
                    g_state.gui_manager,
                    gui_event.mouse.position.x,
                    gui_event.mouse.position.y
                );
            }
            
            // Processar evento
            gui_manager_process_event(g_state.gui_manager, &gui_event);
        }
    }
    
    return !quit;
}

void sdl_frontend_update(void) {
    if (!g_state.initialized) {
        return;
    }
    
    // Limitar taxa de atualização
    uint32_t current_time = SDL_GetTicks();
    uint32_t elapsed_time = current_time - g_state.last_update_time;
    
    if (elapsed_time < g_state.frame_delay) {
        SDL_Delay(g_state.frame_delay - elapsed_time);
    }
    
    g_state.last_update_time = SDL_GetTicks();
    
    // Atualizar GUI
    gui_manager_update(g_state.gui_manager);
}

void sdl_frontend_render(void) {
    if (!g_state.initialized) {
        return;
    }
    
    // Limpar tela
    SDL_SetRenderDrawColor(
        g_state.renderer,
        g_state.background_color.r,
        g_state.background_color.g,
        g_state.background_color.b,
        g_state.background_color.a
    );
    
    SDL_RenderClear(g_state.renderer);
    
    // Renderizar GUI
    gui_manager_render(g_state.gui_manager, g_state.renderer);
    
    // Apresentar renderização
    SDL_RenderPresent(g_state.renderer);
}

gui_manager_t sdl_frontend_get_gui_manager(void) {
    return g_state.gui_manager;
}

SDL_Renderer* sdl_frontend_get_renderer(void) {
    return g_state.renderer;
}

SDL_Window* sdl_frontend_get_window(void) {
    return g_state.window;
}

void sdl_frontend_set_background_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!g_state.initialized) {
        return;
    }
    
    g_state.background_color.r = r;
    g_state.background_color.g = g;
    g_state.background_color.b = b;
    g_state.background_color.a = a;
}
