#include "../../src/gui/sdl2_backend.h"
#include "../../src/gui/shaders.h"
#include "../../src/gui/anime_effects.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura para simulação da aplicação
typedef struct
{
    gui_sdl2_backend_t backend;
    gui_shader_system_t shaders;
    anime_effect_system_t effects;
    SDL_Texture *game_screen;
    const char *current_rom;
    anime_style_t current_style;
    gui_bool_t running;
} app_state_t;

// Função para criar uma textura de simulação de jogo
SDL_Texture *create_game_texture(gui_sdl2_backend_t *backend)
{
    int width = 320;
    int height = 240;

    SDL_Texture *texture = SDL_CreateTexture(
        backend->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    if (!texture)
    {
        return NULL;
    }

    // Desenhar uma cena de teste
    SDL_SetRenderTarget(backend->renderer, texture);
    SDL_SetRenderDrawColor(backend->renderer, 0, 0, 0, 255);
    SDL_RenderClear(backend->renderer);

    // Desenhar um personagem simples
    SDL_Rect character = {width / 2 - 20, height / 2 - 30, 40, 60};
    SDL_SetRenderDrawColor(backend->renderer, 200, 150, 150, 255);
    SDL_RenderFillRect(backend->renderer, &character);

    // Desenhar cabelo
    SDL_Rect hair = {width / 2 - 15, height / 2 - 40, 30, 20};
    SDL_SetRenderDrawColor(backend->renderer, 50, 50, 200, 255);
    SDL_RenderFillRect(backend->renderer, &hair);

    // Desenhar olhos
    SDL_Rect eye1 = {width / 2 - 10, height / 2 - 25, 5, 8};
    SDL_Rect eye2 = {width / 2 + 5, height / 2 - 25, 5, 8};
    SDL_SetRenderDrawColor(backend->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(backend->renderer, &eye1);
    SDL_RenderFillRect(backend->renderer, &eye2);

    // Desenhar fundo com gradiente
    for (int y = 0; y < height; y++)
    {
        SDL_SetRenderDrawColor(backend->renderer,
                               100 + y / 2,
                               150 - y / 3,
                               200 - y / 2,
                               255);
        SDL_RenderDrawLine(backend->renderer, 0, y, width, y);
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
    if (gui_sdl2_init(&app->backend, "Anime Effects Demo", window_size, GUI_TRUE) != GUI_SUCCESS)
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

    // Inicializar sistema de efeitos anime
    if (anime_effects_init(&app->effects, &app->shaders) != GUI_SUCCESS)
    {
        fprintf(stderr, "Falha ao inicializar efeitos anime\n");
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    // Configurar estilo inicial
    app->current_style = ANIME_STYLE_MODERN;
    anime_effects_set_style(&app->effects, app->current_style);

    // Criar textura de jogo
    app->game_screen = create_game_texture(&app->backend);
    if (!app->game_screen)
    {
        fprintf(stderr, "Falha ao criar textura do jogo\n");
        anime_effects_shutdown(&app->effects);
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    app->current_rom = "demo.rom";
    app->running = GUI_TRUE;

    return GUI_TRUE;
}

// Finalizar aplicação
void app_shutdown(app_state_t *app)
{
    if (app->game_screen)
    {
        SDL_DestroyTexture(app->game_screen);
    }

    anime_effects_shutdown(&app->effects);
    gui_shaders_shutdown(&app->shaders);
    gui_sdl2_shutdown(&app->backend);
}

// Atualizar estilo
void app_change_style(app_state_t *app, anime_style_t style)
{
    app->current_style = style;
    anime_effects_set_style(&app->effects, style);

    // Simular ROM correspondente ao estilo
    switch (style)
    {
    case ANIME_STYLE_MODERN:
        app->current_rom = "demon_slayer.rom";
        break;
    case ANIME_STYLE_CLASSIC:
        app->current_rom = "dragon_ball.rom";
        break;
    case ANIME_STYLE_SHONEN:
        app->current_rom = "naruto.rom";
        break;
    case ANIME_STYLE_SHOJO:
        app->current_rom = "sailor_moon.rom";
        break;
    case ANIME_STYLE_SEINEN:
        app->current_rom = "berserk.rom";
        break;
    case ANIME_STYLE_CHIBI:
        app->current_rom = "super_deformed.rom";
        break;
    case ANIME_STYLE_CYBERPUNK:
        app->current_rom = "ghost_shell.rom";
        break;
    case ANIME_STYLE_WATERCOLOR:
        app->current_rom = "ghibli.rom";
        break;
    default:
        app->current_rom = "unknown.rom";
        break;
    }
}

// Exemplo de personalização de efeitos
void app_customize_effects(app_state_t *app)
{
    // Obter parâmetros atuais
    anime_effect_params_t params = app->effects.current_preset.params;

    // Aumentar intensidade de alguns efeitos
    params.outline.thickness += 0.5f;
    if (params.outline.thickness > 5.0f)
        params.outline.thickness = 0.5f;

    params.cel.shade_levels = (params.cel.shade_levels % 5) + 2;

    params.color.saturation += 0.2f;
    if (params.color.saturation > 2.0f)
        params.color.saturation = 0.8f;

    // Aplicar personalização
    anime_effects_customize(&app->effects, params);

    printf("Efeitos personalizados:\n");
    printf("- Contorno: %.1f\n", params.outline.thickness);
    printf("- Níveis de cel: %d\n", params.cel.shade_levels);
    printf("- Saturação: %.1f\n", params.color.saturation);
}

// Exemplo de salvamento de preset
void app_save_preset(app_state_t *app)
{
    // Salvar preset personalizado para o jogo atual
    anime_effects_save_game_preset(app->current_rom, &app->effects.current_preset);
    printf("Preset salvo para %s\n", app->current_rom);
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

    // Aplicar efeitos anime
    anime_effects_begin_frame(&app->effects, result);
    anime_effects_apply(&app->effects, app->game_screen, result, NULL);
    anime_effects_end_frame(&app->effects);

    // Renderizar para a tela
    SDL_SetRenderDrawColor(app->backend.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->backend.renderer);

    // Calcular dimensões para manter aspect ratio
    int game_width, game_height;
    SDL_QueryTexture(app->game_screen, NULL, NULL, &game_width, &game_height);

    float scale = 2.0f;
    SDL_Rect dest = {
        (app->backend.window_size.width - game_width * scale) / 2,
        (app->backend.window_size.height - game_height * scale) / 2,
        game_width * scale,
        game_height * scale};

    SDL_RenderCopy(app->backend.renderer, result, NULL, &dest);

    // Mostrar informações na tela
    char info[128];
    snprintf(info, sizeof(info), "Estilo: %s | ROM: %s | Press 1-8 para mudar estilo, C para customizar, S para salvar",
             app->effects.current_preset.name, app->current_rom);

    // Modo simples, na prática usaríamos o sistema de texto apropriado
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
                    app_change_style(app, ANIME_STYLE_MODERN);
                    break;

                case SDLK_2:
                    app_change_style(app, ANIME_STYLE_CLASSIC);
                    break;

                case SDLK_3:
                    app_change_style(app, ANIME_STYLE_SHONEN);
                    break;

                case SDLK_4:
                    app_change_style(app, ANIME_STYLE_SHOJO);
                    break;

                case SDLK_5:
                    app_change_style(app, ANIME_STYLE_SEINEN);
                    break;

                case SDLK_6:
                    app_change_style(app, ANIME_STYLE_CHIBI);
                    break;

                case SDLK_7:
                    app_change_style(app, ANIME_STYLE_CYBERPUNK);
                    break;

                case SDLK_8:
                    app_change_style(app, ANIME_STYLE_WATERCOLOR);
                    break;

                case SDLK_c:
                    app_customize_effects(app);
                    break;

                case SDLK_s:
                    app_save_preset(app);
                    break;
                }
            }
        }

        // Atualizar e renderizar
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
