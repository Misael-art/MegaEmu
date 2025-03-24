#include "sdl2_backend.h"
#include "batch_renderer.h"
#include "utils/enhanced_log.h"
#include <SDL2/SDL_ttf.h>

// Macros de log
#define BACKEND_LOG_ERROR(...) LOG_ERROR(EMU_LOG_CAT_GUI, __VA_ARGS__)
#define BACKEND_LOG_INFO(...) LOG_INFO(EMU_LOG_CAT_GUI, __VA_ARGS__)
#define BACKEND_LOG_WARN(...) LOG_WARN(EMU_LOG_CAT_GUI, __VA_ARGS__)
#define BACKEND_LOG_DEBUG(...) LOG_DEBUG(EMU_LOG_CAT_GUI, __VA_ARGS__)

// Estrutura interna do backend
typedef struct
{
    gui_batch_renderer_t batch_renderer;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *render_target;
    TTF_Font *default_font;
    gui_bool_t vsync_enabled;
    gui_rect_t viewport;
    float dpi_scale;
} sdl2_backend_internal_t;

// Instância global do backend
static sdl2_backend_internal_t *g_backend = NULL;

// Inicialização do backend
gui_result_t gui_sdl2_init(gui_sdl2_backend_t *backend, const char *title,
                           gui_size_t size, gui_bool_t vsync)
{
    if (!backend)
        return GUI_ERROR_INVALID_PARAM;
    // Inicializar SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        BACKEND_LOG_ERROR("Falha ao inicializar SDL2: %s", SDL_GetError());
        return GUI_ERROR_INIT_FAILED;
    }
    // Inicializar SDL2_ttf
    if (TTF_Init() < 0)
    {
        SDL_Quit();
        return GUI_ERROR_INIT_FAILED;
    }
    // Alocar estrutura interna
    g_backend = (sdl2_backend_internal_t *)malloc(sizeof(sdl2_backend_internal_t));
    if (!g_backend)
    {
        TTF_Quit();
        SDL_Quit();
        return GUI_ERROR_MEMORY;
    }
    // Criar janela
    Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    g_backend->window = SDL_CreateWindow(title,
                                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                         size.width, size.height, window_flags);
    if (!g_backend->window)
    {
        free(g_backend);
        TTF_Quit();
        SDL_Quit();
        return GUI_ERROR_INIT_FAILED;
    }
    // Criar renderer
    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED;
    if (vsync)
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    g_backend->renderer = SDL_CreateRenderer(g_backend->window, -1, renderer_flags);
    if (!g_backend->renderer)
    {
        SDL_DestroyWindow(g_backend->window);
        free(g_backend);
        TTF_Quit();
        SDL_Quit();
        return GUI_ERROR_INIT_FAILED;
    }
    // Configurar renderização
    SDL_SetRenderDrawBlendMode(g_backend->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetIntegerScale(g_backend->renderer, SDL_TRUE);
    // Criar render target
    g_backend->render_target = SDL_CreateTexture(g_backend->renderer,
                                                 SDL_PIXELFORMAT_RGBA8888,
                                                 SDL_TEXTUREACCESS_TARGET,
                                                 size.width, size.height);
    if (!g_backend->render_target)
    {
        SDL_DestroyRenderer(g_backend->renderer);
        SDL_DestroyWindow(g_backend->window);
        free(g_backend);
        TTF_Quit();
        SDL_Quit();
        return GUI_ERROR_INIT_FAILED;
    }
    // Carregar fonte padrão
    g_backend->default_font = TTF_OpenFont("assets/fonts/default.ttf", 16);
    if (!g_backend->default_font)
    {
        // Fallback para fonte do sistema
        g_backend->default_font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16);
    }
    // Inicializar batch renderer
    if (gui_batch_renderer_init(&g_backend->batch_renderer) != GUI_SUCCESS)
    {
        if (g_backend->default_font)
            TTF_CloseFont(g_backend->default_font);
        SDL_DestroyTexture(g_backend->render_target);
        SDL_DestroyRenderer(g_backend->renderer);
        SDL_DestroyWindow(g_backend->window);
        free(g_backend);
        TTF_Quit();
        SDL_Quit();
        return GUI_ERROR_INIT_FAILED;
    }
    // Configurar viewport e escala
    float ddpi;
    SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(g_backend->window), &ddpi, NULL, NULL);
    g_backend->dpi_scale = ddpi / 96.0f;
    g_backend->viewport = (gui_rect_t){0, 0, size.width, size.height};
    g_backend->vsync_enabled = vsync;
    // Configurar backend público
    backend->window = g_backend->window;
    backend->renderer = g_backend->renderer;
    backend->target = g_backend->render_target;
    backend->window_size = size;
    backend->vsync_enabled = vsync;
    backend->scale_factor = g_backend->dpi_scale;
    BACKEND_LOG_INFO("Backend SDL2 inicializado com sucesso");
    return GUI_SUCCESS;
}

// Finalização do backend
void gui_sdl2_shutdown(gui_sdl2_backend_t *backend)
{
    if (!g_backend)
        return;
    gui_batch_renderer_shutdown(&g_backend->batch_renderer);
    if (g_backend->default_font)
    {
        TTF_CloseFont(g_backend->default_font);
    }
    if (g_backend->render_target)
    {
        SDL_DestroyTexture(g_backend->render_target);
    }
    if (g_backend->renderer)
    {
        SDL_DestroyRenderer(g_backend->renderer);
    }
    if (g_backend->window)
    {
        SDL_DestroyWindow(g_backend->window);
    }
    free(g_backend);
    g_backend = NULL;
    TTF_Quit();
    SDL_Quit();
    BACKEND_LOG_INFO("Backend SDL2 finalizado com sucesso");
}

// Início do frame
gui_result_t gui_sdl2_begin_frame(gui_sdl2_backend_t *backend)
{
    if (!g_backend)
        return GUI_ERROR_NOT_INITIALIZED;
    // Limpar render target
    SDL_SetRenderTarget(g_backend->renderer, g_backend->render_target);
    SDL_SetRenderDrawColor(g_backend->renderer, 0, 0, 0, 0);
    SDL_RenderClear(g_backend->renderer);
    // Iniciar batch renderer
    gui_batch_renderer_begin(&g_backend->batch_renderer);
    return GUI_SUCCESS;
}

// Fim do frame
gui_result_t gui_sdl2_end_frame(gui_sdl2_backend_t *backend)
{
    if (!g_backend)
        return GUI_ERROR_NOT_INITIALIZED;
    // Finalizar batch renderer
    gui_batch_renderer_end(&g_backend->batch_renderer, backend);
    // Renderizar para a tela
    SDL_SetRenderTarget(g_backend->renderer, NULL);
    SDL_RenderCopy(g_backend->renderer, g_backend->render_target, NULL, NULL);
    SDL_RenderPresent(g_backend->renderer);
    return GUI_SUCCESS;
}

// Renderização de retângulo
gui_result_t gui_sdl2_draw_rect(gui_sdl2_backend_t *backend,
                                gui_rect_t rect, gui_color_t color)
{
    if (!g_backend)
        return GUI_ERROR_NOT_INITIALIZED;
    // Aplicar escala DPI
    rect.x *= g_backend->dpi_scale;
    rect.y *= g_backend->dpi_scale;
    rect.width *= g_backend->dpi_scale;
    rect.height *= g_backend->dpi_scale;
    // Usar batch renderer
    gui_batch_renderer_draw_rect(&g_backend->batch_renderer,
                                 rect, color, GUI_BLEND_ALPHA);
    return GUI_SUCCESS;
}

// Renderização de texto
gui_result_t gui_sdl2_draw_text(gui_sdl2_backend_t *backend,
                                const char *text, gui_point_t pos,
                                gui_color_t color)
{
    if (!g_backend || !g_backend->default_font)
        return GUI_ERROR_NOT_INITIALIZED;
    // Aplicar escala DPI
    pos.x *= g_backend->dpi_scale;
    pos.y *= g_backend->dpi_scale;
    // Renderizar texto usando batch renderer
    gui_batch_renderer_draw_text(&g_backend->batch_renderer,
                                 text, pos, g_backend->default_font,
                                 color, GUI_BLEND_ALPHA);
    return GUI_SUCCESS;
}

// Processamento de eventos
gui_result_t gui_sdl2_process_events(gui_sdl2_backend_t *backend,
                                     gui_event_handler_t handler)
{
    if (!g_backend)
        return GUI_ERROR_NOT_INITIALIZED;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            return GUI_EVENT_QUIT;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                int width = event.window.data1;
                int height = event.window.data2;
                // Recriar render target
                if (g_backend->render_target)
                {
                    SDL_DestroyTexture(g_backend->render_target);
                }
                g_backend->render_target = SDL_CreateTexture(g_backend->renderer,
                                                             SDL_PIXELFORMAT_RGBA8888,
                                                             SDL_TEXTUREACCESS_TARGET,
                                                             width, height);
                backend->target = g_backend->render_target;
                backend->window_size.width = width;
                backend->window_size.height = height;
                g_backend->viewport.width = width;
                g_backend->viewport.height = height;
                if (handler)
                {
                    gui_event_t gui_event = {
                        .type = GUI_EVENT_RESIZE,
                        .resize = {
                            .width = width,
                            .height = height}};
                    handler(&gui_event);
                }
            }
            break;
        }
    }
    return GUI_SUCCESS;
}

// Funções auxiliares
gui_result_t gui_sdl2_set_viewport(gui_sdl2_backend_t *backend, gui_rect_t viewport)
{
    if (!g_backend)
        return GUI_ERROR_NOT_INITIALIZED;
    g_backend->viewport = viewport;
    SDL_Rect sdl_viewport = {
        viewport.x, viewport.y,
        viewport.width, viewport.height};
    SDL_RenderSetViewport(g_backend->renderer, &sdl_viewport);
    return GUI_SUCCESS;
}

gui_result_t gui_sdl2_get_window_size(gui_sdl2_backend_t *backend,
                                      int *width, int *height)
{
    if (!g_backend)
        return GUI_ERROR_NOT_INITIALIZED;
    SDL_GetWindowSize(g_backend->window, width, height);
    return GUI_SUCCESS;
}

gui_result_t gui_sdl2_set_vsync(gui_sdl2_backend_t *backend, gui_bool_t enable)
{
    if (!g_backend)
        return GUI_ERROR_NOT_INITIALIZED;
    // SDL2 não permite mudar vsync em tempo de execução
    // Precisamos recriar o renderer
    SDL_DestroyRenderer(g_backend->renderer);
    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED;
    if (enable)
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    g_backend->renderer = SDL_CreateRenderer(g_backend->window, -1, renderer_flags);
    if (!g_backend->renderer)
    {
        return GUI_ERROR_INIT_FAILED;
    }
    backend->renderer = g_backend->renderer;
    backend->vsync_enabled = enable;
    g_backend->vsync_enabled = enable;
    return GUI_SUCCESS;
}
