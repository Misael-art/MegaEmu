#include "../../src/gui/sdl2_backend.h"
#include "../../src/gui/shaders.h"
#include "../../src/gui/retro_effects.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura para simulação da aplicação
typedef struct
{
    gui_sdl2_backend_t backend;
    gui_shader_system_t shaders;
    retro_effect_system_t effects;
    SDL_Texture *game_screen;
    const char *current_rom;
    retro_effect_type_t current_effect;
    gui_bool_t running;
} app_state_t;

// Função para criar uma textura de simulação de jogo
SDL_Texture *create_game_texture(gui_sdl2_backend_t *backend)
{
    int width = 256;
    int height = 224;

    SDL_Texture *texture = SDL_CreateTexture(
        backend->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    if (!texture)
        return NULL;

    // Desenhar uma cena de teste
    SDL_SetRenderTarget(backend->renderer, texture);
    SDL_SetRenderDrawColor(backend->renderer, 0, 0, 0, 255);
    SDL_RenderClear(backend->renderer);

    // Grade de cores
    int cell_size = 16;
    int rows = height / cell_size;
    int cols = width / cell_size;

    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            SDL_Rect rect = {
                x * cell_size,
                y * cell_size,
                cell_size,
                cell_size};

            Uint8 r = (x * 255) / cols;
            Uint8 g = (y * 255) / rows;
            Uint8 b = ((x + y) * 127) / (cols + rows);

            SDL_SetRenderDrawColor(backend->renderer, r, g, b, 255);
            SDL_RenderFillRect(backend->renderer, &rect);
        }
    }

    // Desenhar alguns sprites
    for (int i = 0; i < 5; i++)
    {
        int x = rand() % (width - 32);
        int y = rand() % (height - 32);

        SDL_Rect sprite = {x, y, 32, 32};
        SDL_SetRenderDrawColor(backend->renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(backend->renderer, &sprite);

        SDL_Rect inner = {x + 4, y + 4, 24, 24};
        SDL_SetRenderDrawColor(backend->renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(backend->renderer, &inner);
    }

    // Restaurar render target
    SDL_SetRenderTarget(backend->renderer, NULL);

    return texture;
}

// Inicializar aplicação
gui_bool_t app_init(app_state_t *app)
{
    srand(time(NULL));

    // Inicializar backend
    gui_size_t window_size = {800, 600};
    if (gui_sdl2_init(&app->backend, "Retro Effects Demo", window_size, GUI_TRUE) != GUI_SUCCESS)
    {
        fprintf(stderr, "Falha ao inicializar backend\n");
        return GUI_FALSE;
    }

    // Inicializar sistema de shaders
    if (gui_shaders_init(&app->shaders, &app->backend) != GUI_SUCCESS)
    {
        fprintf(stderr, "Falha ao inicializar shaders\n");
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    // Criar textura de jogo
    app->game_screen = create_game_texture(&app->backend);
    if (!app->game_screen)
    {
        fprintf(stderr, "Falha ao criar textura do jogo\n");
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    // Inicializar sistema de efeitos
    if (retro_effects_init(&app->effects, &app->shaders, 256, 224) != GUI_SUCCESS)
    {
        fprintf(stderr, "Falha ao inicializar efeitos\n");
        SDL_DestroyTexture(app->game_screen);
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    app->current_rom = "test.rom";
    app->current_effect = RETRO_EFFECT_NONE;
    app->running = GUI_TRUE;

    return GUI_TRUE;
}

// Finalizar aplicação
void app_shutdown(app_state_t *app)
{
    if (app->game_screen)
        SDL_DestroyTexture(app->game_screen);

    retro_effects_shutdown(&app->effects);
    gui_shaders_shutdown(&app->shaders);
    gui_sdl2_shutdown(&app->backend);
}

// Atualizar efeito
void app_change_effect(app_state_t *app, retro_effect_type_t effect)
{
    app->current_effect = effect;

    // Configurar parâmetros específicos para cada efeito
    retro_effect_params_t params = app->effects.params;

    switch (effect)
    {
    case RETRO_EFFECT_PIXEL_PERFECT:
        params.scale.mode = RETRO_SCALE_INTEGER;
        params.scale.letterbox = GUI_TRUE;
        params.scale.sharpness = 1.0f;
        break;

    case RETRO_EFFECT_CRT:
        params.crt.curvature = 0.1f;
        params.crt.scanline_intensity = 0.3f;
        params.crt.mask_intensity = 0.2f;
        params.crt.bleed = 0.1f;
        params.crt.phosphor = GUI_TRUE;
        break;

    case RETRO_EFFECT_DITHERING:
        params.dither.type = RETRO_DITHER_ORDERED;
        params.dither.strength = 0.5f;
        params.dither.pattern_size = 8;
        params.dither.color_dither = GUI_TRUE;
        break;

    case RETRO_EFFECT_COLOR_GRADING:
        params.color.color_depth = 16;
        params.color.gamma = 2.2f;
        retro_effects_generate_palette(&app->effects, 32);
        break;

    case RETRO_EFFECT_BLOOM:
        params.bloom.intensity = 0.5f;
        params.bloom.threshold = 0.7f;
        params.bloom.radius = 5.0f;
        params.bloom.tint = (gui_color_t){255, 220, 180, 255};
        break;

    default:
        break;
    }

    retro_effects_set_params(&app->effects, &params);
}

// Personalizar efeitos
void app_customize_effects(app_state_t *app)
{
    retro_effect_params_t params = app->effects.params;

    // Exemplo de personalização baseada no efeito atual
    switch (app->current_effect)
    {
    case RETRO_EFFECT_CRT:
        params.crt.scanline_intensity += 0.1f;
        if (params.crt.scanline_intensity > 1.0f)
            params.crt.scanline_intensity = 0.0f;
        break;

    case RETRO_EFFECT_DITHERING:
        params.dither.type = (params.dither.type + 1) % 4;
        break;

    case RETRO_EFFECT_COLOR_GRADING:
        params.color.color_depth = (params.color.color_depth == 32) ? 8 : 32;
        break;

    case RETRO_EFFECT_BLOOM:
        params.bloom.intensity += 0.2f;
        if (params.bloom.intensity > 1.0f)
            params.bloom.intensity = 0.2f;
        break;

    default:
        break;
    }

    retro_effects_set_params(&app->effects, &params);
}

// Renderização
void app_render(app_state_t *app)
{
    // Textura para o resultado final
    SDL_Texture *result = SDL_CreateTexture(
        app->backend.renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        app->backend.window_size.width,
        app->backend.window_size.height);

    // Aplicar efeitos
    retro_effects_begin_frame(&app->effects, result);
    retro_effects_apply(&app->effects, app->game_screen, result);
    retro_effects_end_frame(&app->effects);

    // Renderizar para a tela
    SDL_SetRenderDrawColor(app->backend.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->backend.renderer);

    // Calcular viewport
    SDL_Rect viewport;
    retro_effects_calculate_viewport(&app->effects, &viewport);
    SDL_RenderCopy(app->backend.renderer, result, NULL, &viewport);

    // Mostrar informações na tela
    char info[128];
    const char *effect_name = "None";
    switch (app->current_effect)
    {
    case RETRO_EFFECT_PIXEL_PERFECT:
        effect_name = "Pixel Perfect";
        break;
    case RETRO_EFFECT_CRT:
        effect_name = "CRT";
        break;
    case RETRO_EFFECT_DITHERING:
        effect_name = "Dithering";
        break;
    case RETRO_EFFECT_COLOR_GRADING:
        effect_name = "Color Grading";
        break;
    case RETRO_EFFECT_BLOOM:
        effect_name = "Bloom";
        break;
    default:
        break;
    }

    snprintf(info, sizeof(info),
             "Effect: %s | Press 1-5 to change effect, C to customize",
             effect_name);

    // Mostrar texto (modo simples)
    SDL_SetRenderDrawColor(app->backend.renderer, 0, 0, 0, 200);
    SDL_Rect info_rect = {0, 0, app->backend.window_size.width, 30};
    SDL_RenderFillRect(app->backend.renderer, &info_rect);

    SDL_RenderPresent(app->backend.renderer);

    // Limpar
    SDL_DestroyTexture(result);
}

// Loop principal
void app_run(app_state_t *app)
{
    SDL_Event event;

    while (app->running)
    {
        // Processar eventos
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                app->running = GUI_FALSE;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    app->running = GUI_FALSE;
                    break;

                case SDLK_1:
                    app_change_effect(app, RETRO_EFFECT_PIXEL_PERFECT);
                    break;

                case SDLK_2:
                    app_change_effect(app, RETRO_EFFECT_CRT);
                    break;

                case SDLK_3:
                    app_change_effect(app, RETRO_EFFECT_DITHERING);
                    break;

                case SDLK_4:
                    app_change_effect(app, RETRO_EFFECT_COLOR_GRADING);
                    break;

                case SDLK_5:
                    app_change_effect(app, RETRO_EFFECT_BLOOM);
                    break;

                case SDLK_c:
                    app_customize_effects(app);
                    break;
                }
            }
        }

        // Renderizar
        app_render(app);

        // Limitar framerate
        SDL_Delay(16);
    }
}

int main(int argc, char *argv[])
{
    app_state_t app = {0};

    if (!app_init(&app))
    {
        return 1;
    }

    app_run(&app);
    app_shutdown(&app);

    return 0;
}
