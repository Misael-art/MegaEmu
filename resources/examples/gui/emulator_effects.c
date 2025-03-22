#include "../../src/gui/sdl2_backend.h"
#include "../../src/gui/shaders.h"
#include "../../src/gui/emu_effects.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura para simulação da aplicação
typedef struct
{
    gui_sdl2_backend_t backend;
    gui_shader_system_t shaders;
    emu_effect_system_t effects;
    SDL_Texture *game_screen;
    const char *current_rom;
    emu_console_type_t current_console;
    gui_bool_t running;
} app_state_t;

// Função para criar uma textura de simulação de jogo
SDL_Texture *create_game_texture(gui_sdl2_backend_t *backend, emu_console_type_t console)
{
    int width, height;
    gui_color_t bg_color = {0, 0, 0, 255};

    // Simular diferentes resoluções por console
    switch (console)
    {
    case EMU_CONSOLE_NES:
        width = 256;
        height = 240;
        bg_color.r = 100;
        break;

    case EMU_CONSOLE_SNES:
        width = 256;
        height = 224;
        bg_color.g = 100;
        break;

    case EMU_CONSOLE_GAMEBOY:
        width = 160;
        height = 144;
        bg_color.r = 15;
        bg_color.g = 56;
        bg_color.b = 15;
        break;

    case EMU_CONSOLE_MEGADRIVE:
        width = 320;
        height = 224;
        bg_color.b = 100;
        break;

    default:
        width = 320;
        height = 240;
        break;
    }

    // Criar textura
    SDL_Texture *texture = SDL_CreateTexture(
        backend->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    if (!texture)
    {
        return NULL;
    }

    // Desenhar algo nela
    SDL_SetRenderTarget(backend->renderer, texture);
    SDL_SetRenderDrawColor(backend->renderer, bg_color.r, bg_color.g, bg_color.b, 255);
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

    // Desenhar alguns sprites simulados
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

    // Desenhar linhas horizontais para simular tela de jogo
    for (int y = 0; y < height; y += height / 8)
    {
        SDL_SetRenderDrawColor(backend->renderer, 128, 128, 128, 255);
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
    if (gui_sdl2_init(&app->backend, "Emulator Effects Demo", window_size, GUI_TRUE) != GUI_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize backend\n");
        return GUI_FALSE;
    }

    // Inicializar sistema de shaders
    if (gui_shaders_init(&app->shaders, &app->backend) != GUI_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize shaders\n");
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    // Inicializar sistema de efeitos
    if (emu_effects_init(&app->effects, &app->shaders) != GUI_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize effects\n");
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    // Configurar NES como console inicial
    app->current_console = EMU_CONSOLE_NES;
    emu_effects_set_console(&app->effects, app->current_console);

    // Criar textura de jogo
    app->game_screen = create_game_texture(&app->backend, app->current_console);
    if (!app->game_screen)
    {
        fprintf(stderr, "Failed to create game texture\n");
        emu_effects_shutdown(&app->effects);
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    app->current_rom = "mario.nes";
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

    emu_effects_shutdown(&app->effects);
    gui_shaders_shutdown(&app->shaders);
    gui_sdl2_shutdown(&app->backend);
}

// Atualizar console
void app_change_console(app_state_t *app, emu_console_type_t console)
{
    // Atualizar console
    app->current_console = console;
    emu_effects_set_console(&app->effects, console);

    // Recriar textura de jogo
    if (app->game_screen)
    {
        SDL_DestroyTexture(app->game_screen);
    }

    app->game_screen = create_game_texture(&app->backend, console);

    // Simular ROM correspondente
    switch (console)
    {
    case EMU_CONSOLE_NES:
        app->current_rom = "mario.nes";
        break;
    case EMU_CONSOLE_SNES:
        app->current_rom = "zelda.smc";
        break;
    case EMU_CONSOLE_GAMEBOY:
        app->current_rom = "tetris.gb";
        break;
    case EMU_CONSOLE_GBC:
        app->current_rom = "pokemon.gbc";
        break;
    case EMU_CONSOLE_GBA:
        app->current_rom = "advance.gba";
        break;
    case EMU_CONSOLE_MEGADRIVE:
        app->current_rom = "sonic.md";
        break;
    case EMU_CONSOLE_MASTERSYSTEM:
        app->current_rom = "alexkidd.sms";
        break;
    case EMU_CONSOLE_ARCADE_CRT:
        app->current_rom = "pacman.zip";
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
    emu_effect_params_t params = app->effects.current_preset.params;

    // Aumentar intensidade de alguns efeitos
    params.phosphor_persistence += 0.1f;
    if (params.phosphor_persistence > 1.0f)
        params.phosphor_persistence = 0.0f;

    params.scanline_intensity += 0.1f;
    if (params.scanline_intensity > 1.0f)
        params.scanline_intensity = 0.0f;

    // Aplicar personalização
    emu_effects_customize(&app->effects, params);

    printf("Efeitos personalizados:\n");
    printf("- Phosphor: %.1f\n", params.phosphor_persistence);
    printf("- Scanlines: %.1f\n", params.scanline_intensity);
}

// Exemplo de salvamento de preset
void app_save_preset(app_state_t *app)
{
    // Salvar preset personalizado para o jogo atual
    emu_effects_save_game_preset(app->current_rom, &app->effects.current_preset);
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

    // Aplicar efeitos
    emu_effects_begin_frame(&app->effects, result);
    emu_effects_apply(&app->effects, app->game_screen, result);
    emu_effects_end_frame(&app->effects);

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
    snprintf(info, sizeof(info), "Console: %s | ROM: %s | Press 1-8 to change console, C to customize, S to save",
             app->effects.current_preset.name, app->current_rom);

    // Modo simples, na prática usaríamos o sistema de texto apropriado
    SDL_SetRenderDrawColor(app->backend.renderer, 0, 0, 0, 200);
    SDL_Rect info_rect = {0, 0, app->backend.window_size.width, 30};
    SDL_RenderFillRect(app->backend.renderer, &info_rect);

    // Na implementação real, usaríamos gui_sdl2_draw_text

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
                    app_change_console(app, EMU_CONSOLE_NES);
                    break;

                case SDLK_2:
                    app_change_console(app, EMU_CONSOLE_SNES);
                    break;

                case SDLK_3:
                    app_change_console(app, EMU_CONSOLE_GAMEBOY);
                    break;

                case SDLK_4:
                    app_change_console(app, EMU_CONSOLE_GBC);
                    break;

                case SDLK_5:
                    app_change_console(app, EMU_CONSOLE_GBA);
                    break;

                case SDLK_6:
                    app_change_console(app, EMU_CONSOLE_MEGADRIVE);
                    break;

                case SDLK_7:
                    app_change_console(app, EMU_CONSOLE_MASTERSYSTEM);
                    break;

                case SDLK_8:
                    app_change_console(app, EMU_CONSOLE_ARCADE_CRT);
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
