#include &ltSDL.h&gt
#include &ltSDL_gamecontroller.h&gt
#include &ltstdbool.h&gt
#include &ltstdint.h&gt
#include &ltstdio.h&gt
#include &ltstring.h&gt
#include "../common/frontend.h"
#include "../common/frontend_internal.h"
#include "../../utils/enhanced_log.h"
#include "../../utils/log_categories.h"
#include "gui/widgets/gui_config.h" // Incluir o header do widget de configuração

#ifndef EMU_FRONTEND_MOCK

// Estrutura para dados específicos do SDL
typedef struct sdl_platform_data {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    bool quit_requested;
} sdl_platform_data_t;

// Widget de configuração
gui_config_t *config_widget;

emu_frontend_t emu_frontend_init(void) {
    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Inicializando frontend SDL");

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao inicializar SDL: %s", SDL_GetError());
        return NULL;
    }

    // Criar estrutura do frontend
    emu_frontend_t frontend = calloc(1, sizeof(struct emu_frontend));
    if (!frontend) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao alocar memória para o frontend");
        SDL_Quit();
        return NULL;
    }

    // Alocar dados específicos do SDL
    sdl_platform_data_t *sdl_data = calloc(1, sizeof(sdl_platform_data_t));
    if (!sdl_data) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao alocar memória para dados SDL");
        free(frontend);
        SDL_Quit();
        return NULL;
    }

    // Criar janela SDL
    sdl_data->window = SDL_CreateWindow(
        "Mega Emu",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!sdl_data->window) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao criar janela SDL: %s", SDL_GetError());
        free(sdl_data);
        free(frontend);
        SDL_Quit();
        return NULL;
    }

    // Criar renderer
    sdl_data->renderer = SDL_CreateRenderer(sdl_data->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl_data->renderer) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao criar renderer SDL: %s", SDL_GetError());
        SDL_DestroyWindow(sdl_data->window);
        free(sdl_data);
        free(frontend);
        SDL_Quit();
        return NULL;
    }

    // Configurar frontend
    frontend->width = 320;
    frontend->height = 240;
    frontend->framebuffer = calloc(frontend->width * frontend->height, sizeof(uint32_t));
    if (!frontend->framebuffer) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao alocar framebuffer");
        SDL_DestroyRenderer(sdl_data->renderer);
        SDL_DestroyWindow(sdl_data->window);
        free(sdl_data);
        free(frontend);
        SDL_Quit();
        return NULL;
    }

    // Criar textura
    sdl_data->texture = SDL_CreateTexture(
        sdl_data->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        frontend->width,
        frontend->height);
    if (!sdl_data->texture) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao criar textura SDL: %s", SDL_GetError());
        free(frontend->framebuffer);
        SDL_DestroyRenderer(sdl_data->renderer);
        SDL_DestroyWindow(sdl_data->window);
        free(sdl_data);
        free(frontend);
        SDL_Quit();
        return NULL;
    }

    // Configurar dados específicos do SDL
    frontend->platform_data = sdl_data;
    frontend->initialized = true;

    // Criar o widget de configuração
    config_widget = gui_config_create();
    if (!config_widget) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao criar o widget de configuração");
        emu_frontend_shutdown(frontend);
        return NULL;
    }

    return frontend;
}

void emu_frontend_shutdown(emu_frontend_t frontend) {
    if (!frontend)
        return;

    if (frontend->platform_data) {
        sdl_platform_data_t *sdl_data = (sdl_platform_data_t *)frontend->platform_data;
        if (sdl_data->texture)
            SDL_DestroyTexture(sdl_data->texture);
        if (sdl_data->renderer)
            SDL_DestroyRenderer(sdl_data->renderer);
        if (sdl_data->window)
            SDL_DestroyWindow(sdl_data->window);
        free(sdl_data);
    }

    if (frontend->framebuffer)
        free(frontend->framebuffer);

    // Destruir o widget de configuração
    if (config_widget)
        gui_config_destroy(config_widget);

    free(frontend);
    SDL_Quit();
}

bool emu_frontend_process_events(emu_frontend_t frontend) {
    if (!frontend || !frontend->initialized)
        return false;

    sdl_platform_data_t *sdl_data = (sdl_platform_data_t *)frontend->platform_data;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                sdl_data->quit_requested = true;
                break;
        }
    }

    return !sdl_data->quit_requested;
}

int emu_frontend_render_frame(emu_frontend_t frontend, const uint32_t *framebuffer, int width, int height) {
    if (!frontend || !frontend->initialized || !framebuffer)
        return -1;

    if (width <= 0 || height <= 0)
        return -1;

    if (width != frontend->width || height != frontend->height) {
        uint32_t *new_buffer = realloc(frontend->framebuffer, width * height * sizeof(uint32_t));
        if (!new_buffer)
            return -1;

        frontend->framebuffer = new_buffer;
        frontend->width = width;
        frontend->height = height;

        sdl_platform_data_t *sdl_data = (sdl_platform_data_t *)frontend->platform_data;
        SDL_DestroyTexture(sdl_data->texture);
        sdl_data->texture = SDL_CreateTexture(
            sdl_data->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height);
        if (!sdl_data->texture) {
            EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao recriar textura SDL: %s", SDL_GetError());
            return -1;
        }
    }

    memcpy(frontend->framebuffer, framebuffer, width * height * sizeof(uint32_t));
    return 0;
}

int emu_frontend_update_window(emu_frontend_t frontend) {
    if (!frontend || !frontend->initialized)
        return -1;

    sdl_platform_data_t *sdl_data = (sdl_platform_data_t *)frontend->platform_data;
    SDL_UpdateTexture(sdl_data->texture, NULL, frontend->framebuffer, frontend->width * sizeof(uint32_t));
    SDL_RenderClear(sdl_data->renderer);
    SDL_RenderCopy(sdl_data->renderer, sdl_data->texture, NULL, NULL);
    SDL_RenderPresent(sdl_data->renderer);
    return 0;
}

#endif /* EMU_FRONTEND_MOCK */

uint8_t emu_frontend_get_controller_state(void) {
    return 0; // Implementação básica - sem controle
}
