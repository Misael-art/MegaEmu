#include "../../src/gui/sdl2_backend.h"
#include "../../src/gui/shaders.h"
#include "../../src/gui/modern_effects.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura para simulação da aplicação
typedef struct
{
    gui_sdl2_backend_t backend;
    gui_shader_system_t shaders;
    modern_effect_system_t effects;
    SDL_Texture *game_screen;
    SDL_Texture *depth_buffer;
    SDL_Texture *normal_buffer;
    const char *current_rom;
    modern_effect_type_t current_effect;
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

    // Desenhar fundo com gradiente
    for (int y = 0; y < height; y++)
    {
        SDL_SetRenderDrawColor(backend->renderer,
                               50 + y / 2,
                               100 - y / 3,
                               150 - y / 2,
                               255);
        SDL_RenderDrawLine(backend->renderer, 0, y, width, y);
    }

    // Desenhar alguns elementos de teste
    SDL_Rect ground = {0, height - 40, width, 40};
    SDL_SetRenderDrawColor(backend->renderer, 100, 150, 50, 255);
    SDL_RenderFillRect(backend->renderer, &ground);

    // Desenhar árvores
    for (int i = 0; i < 5; i++)
    {
        int x = 20 + i * 50;
        SDL_Rect trunk = {x, height - 60, 10, 20};
        SDL_Rect leaves = {x - 15, height - 100, 40, 40};

        SDL_SetRenderDrawColor(backend->renderer, 139, 69, 19, 255);
        SDL_RenderFillRect(backend->renderer, &trunk);

        SDL_SetRenderDrawColor(backend->renderer, 34, 139, 34, 255);
        SDL_RenderFillRect(backend->renderer, &leaves);
    }

    // Desenhar personagem
    SDL_Rect character = {width / 2 - 10, height - 70, 20, 30};
    SDL_SetRenderDrawColor(backend->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(backend->renderer, &character);

    // Restaurar render target
    SDL_SetRenderTarget(backend->renderer, NULL);

    return texture;
}

// Função para criar buffer de profundidade
SDL_Texture *create_depth_buffer(gui_sdl2_backend_t *backend, int width, int height)
{
    SDL_Texture *texture = SDL_CreateTexture(
        backend->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    if (!texture)
        return NULL;

    // Desenhar profundidade
    SDL_SetRenderTarget(backend->renderer, texture);
    SDL_SetRenderDrawColor(backend->renderer, 255, 255, 255, 255);
    SDL_RenderClear(backend->renderer);

    // Profundidade do céu (longe)
    SDL_Rect sky = {0, 0, width, height - 40};
    SDL_SetRenderDrawColor(backend->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(backend->renderer, &sky);

    // Profundidade do chão (perto)
    SDL_Rect ground = {0, height - 40, width, 40};
    SDL_SetRenderDrawColor(backend->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(backend->renderer, &ground);

    // Profundidade das árvores (média)
    for (int i = 0; i < 5; i++)
    {
        int x = 20 + i * 50;
        SDL_Rect trunk = {x, height - 60, 10, 20};
        SDL_Rect leaves = {x - 15, height - 100, 40, 40};

        SDL_SetRenderDrawColor(backend->renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(backend->renderer, &trunk);
        SDL_RenderFillRect(backend->renderer, &leaves);
    }

    // Profundidade do personagem (muito perto)
    SDL_Rect character = {width / 2 - 10, height - 70, 20, 30};
    SDL_SetRenderDrawColor(backend->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(backend->renderer, &character);

    SDL_SetRenderTarget(backend->renderer, NULL);
    return texture;
}

// Função para criar buffer de normais
SDL_Texture *create_normal_buffer(gui_sdl2_backend_t *backend, int width, int height)
{
    SDL_Texture *texture = SDL_CreateTexture(
        backend->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    if (!texture)
        return NULL;

    // Desenhar normais
    SDL_SetRenderTarget(backend->renderer, texture);
    SDL_SetRenderDrawColor(backend->renderer, 128, 128, 255, 255); // Normal para cima
    SDL_RenderClear(backend->renderer);

    // Normal do chão
    SDL_Rect ground = {0, height - 40, width, 40};
    SDL_SetRenderDrawColor(backend->renderer, 128, 128, 0, 255); // Normal para baixo
    SDL_RenderFillRect(backend->renderer, &ground);

    // Normais das árvores
    for (int i = 0; i < 5; i++)
    {
        int x = 20 + i * 50;
        SDL_Rect trunk = {x, height - 60, 10, 20};
        SDL_Rect leaves = {x - 15, height - 100, 40, 40};

        SDL_SetRenderDrawColor(backend->renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(backend->renderer, &trunk);
        SDL_RenderFillRect(backend->renderer, &leaves);
    }

    SDL_SetRenderTarget(backend->renderer, NULL);
    return texture;
}

// Inicializar aplicação
gui_bool_t app_init(app_state_t *app)
{
    srand(time(NULL));

    // Inicializar backend
    gui_size_t window_size = {800, 600};
    if (gui_sdl2_init(&app->backend, "Modern Effects Demo", window_size, GUI_TRUE) != GUI_SUCCESS)
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

    // Criar texturas
    app->game_screen = create_game_texture(&app->backend);
    app->depth_buffer = create_depth_buffer(&app->backend, 256, 224);
    app->normal_buffer = create_normal_buffer(&app->backend, 256, 224);

    if (!app->game_screen || !app->depth_buffer || !app->normal_buffer)
    {
        fprintf(stderr, "Falha ao criar texturas\n");
        if (app->game_screen)
            SDL_DestroyTexture(app->game_screen);
        if (app->depth_buffer)
            SDL_DestroyTexture(app->depth_buffer);
        if (app->normal_buffer)
            SDL_DestroyTexture(app->normal_buffer);
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    // Inicializar sistema de efeitos
    if (modern_effects_init(&app->effects, &app->shaders, 256, 224) != GUI_SUCCESS)
    {
        fprintf(stderr, "Falha ao inicializar efeitos\n");
        SDL_DestroyTexture(app->game_screen);
        SDL_DestroyTexture(app->depth_buffer);
        SDL_DestroyTexture(app->normal_buffer);
        gui_shaders_shutdown(&app->shaders);
        gui_sdl2_shutdown(&app->backend);
        return GUI_FALSE;
    }

    app->current_rom = "test.rom";
    app->current_effect = MODERN_EFFECT_NONE;
    app->running = GUI_TRUE;

    return GUI_TRUE;
}

// Finalizar aplicação
void app_shutdown(app_state_t *app)
{
    if (app->game_screen)
        SDL_DestroyTexture(app->game_screen);
    if (app->depth_buffer)
        SDL_DestroyTexture(app->depth_buffer);
    if (app->normal_buffer)
        SDL_DestroyTexture(app->normal_buffer);

    modern_effects_shutdown(&app->effects);
    gui_shaders_shutdown(&app->shaders);
    gui_sdl2_shutdown(&app->backend);
}

// Atualizar efeito
void app_change_effect(app_state_t *app, modern_effect_type_t effect)
{
    app->current_effect = effect;

    // Configurar parâmetros específicos para cada efeito
    modern_effect_params_t params = app->effects.params;

    switch (effect)
    {
    case MODERN_EFFECT_VOLUMETRIC_LIGHT:
        params.volumetric.density = 0.3f;
        params.volumetric.scatter = 0.5f;
        params.volumetric.num_samples = 20;
        params.volumetric.light_pos = (vec2_t){128.0f, 50.0f};
        params.volumetric.light_color = (gui_color_t){255, 230, 180, 255};
        break;

    case MODERN_EFFECT_WEATHER:
        params.weather.type = WEATHER_RAIN;
        params.weather.intensity = 0.5f;
        params.weather.wind_speed = 1.0f;
        params.weather.wind_direction = 0.5f;
        params.weather.splash_size = 1.0f;
        break;

    case MODERN_EFFECT_REFLECTION:
        params.reflection.reflection_strength = 0.5f;
        params.reflection.roughness = 0.1f;
        params.reflection.fresnel = 1.0f;
        params.reflection.max_steps = 16;
        break;

    case MODERN_EFFECT_PARTICLES:
        params.particles.max_particles = 1000;
        params.particles.spawn_rate = 10.0f;
        params.particles.lifetime = 2.0f;
        params.particles.velocity = 1.0f;
        params.particles.emit_light = GUI_TRUE;
        break;

    case MODERN_EFFECT_DEPTH_OF_FIELD:
        params.depth.focal_distance = 0.5f;
        params.depth.focal_range = 0.2f;
        params.depth.blur_strength = 1.0f;
        params.depth.use_bokeh = GUI_TRUE;
        break;

    default:
        break;
    }

    modern_effects_set_params(&app->effects, &params);
}

// Personalizar efeitos
void app_customize_effects(app_state_t *app)
{
    modern_effect_params_t params = app->effects.params;

    // Exemplo de personalização baseada no efeito atual
    switch (app->current_effect)
    {
    case MODERN_EFFECT_VOLUMETRIC_LIGHT:
        params.volumetric.density += 0.1f;
        if (params.volumetric.density > 1.0f)
            params.volumetric.density = 0.1f;
        break;

    case MODERN_EFFECT_WEATHER:
        params.weather.type = (params.weather.type + 1) % 5;
        break;

    case MODERN_EFFECT_REFLECTION:
        params.reflection.roughness += 0.1f;
        if (params.reflection.roughness > 1.0f)
            params.reflection.roughness = 0.0f;
        break;

    case MODERN_EFFECT_PARTICLES:
        params.particles.emit_light = !params.particles.emit_light;
        break;

    case MODERN_EFFECT_DEPTH_OF_FIELD:
        params.depth.focal_distance += 0.1f;
        if (params.depth.focal_distance > 1.0f)
            params.depth.focal_distance = 0.0f;
        break;

    default:
        break;
    }

    modern_effects_set_params(&app->effects, &params);
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
    modern_effects_begin_frame(&app->effects, result);
    modern_effects_apply(&app->effects, app->game_screen, result);
    modern_effects_end_frame(&app->effects);

    // Renderizar para a tela
    SDL_SetRenderDrawColor(app->backend.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->backend.renderer);

    // Calcular viewport
    int game_width = 256;
    int game_height = 224;
    float scale = 2.0f;
    SDL_Rect viewport = {
        (app->backend.window_size.width - game_width * scale) / 2,
        (app->backend.window_size.height - game_height * scale) / 2,
        game_width * scale,
        game_height * scale};

    SDL_RenderCopy(app->backend.renderer, result, NULL, &viewport);

    // Mostrar informações na tela
    char info[128];
    const char *effect_name = "None";
    switch (app->current_effect)
    {
    case MODERN_EFFECT_VOLUMETRIC_LIGHT:
        effect_name = "Volumetric Light";
        break;
    case MODERN_EFFECT_WEATHER:
        effect_name = "Weather";
        break;
    case MODERN_EFFECT_REFLECTION:
        effect_name = "Reflection";
        break;
    case MODERN_EFFECT_PARTICLES:
        effect_name = "Particles";
        break;
    case MODERN_EFFECT_DEPTH_OF_FIELD:
        effect_name = "Depth of Field";
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
                    app_change_effect(app, MODERN_EFFECT_VOLUMETRIC_LIGHT);
                    break;

                case SDLK_2:
                    app_change_effect(app, MODERN_EFFECT_WEATHER);
                    break;

                case SDLK_3:
                    app_change_effect(app, MODERN_EFFECT_REFLECTION);
                    break;

                case SDLK_4:
                    app_change_effect(app, MODERN_EFFECT_PARTICLES);
                    break;

                case SDLK_5:
                    app_change_effect(app, MODERN_EFFECT_DEPTH_OF_FIELD);
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
