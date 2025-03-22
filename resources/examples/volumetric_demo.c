#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "../src/gui/volumetric_effects.h"
#include "../src/gui/vol_effects_integration.h"
#include "../src/gui/sdl2_backend.h"

// Estrutura para janela de exemplo
typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *game_texture;      // Textura com frame do jogo
    SDL_Texture *processed_texture; // Textura com efeitos aplicados
    int width;
    int height;
    bool running;
} demo_window_t;

// Estrutura para estado do demo
typedef struct
{
    demo_window_t window;
    gui_sdl2_backend_t sdl_backend;
    vol_effects_state_t effects_state;

    // Controles de efeitos
    vol_light_config_t light_config;
    bool effects_enabled;
    int current_preset;
    float light_pos_x;
    float light_pos_y;

    // Controle de FPS
    Uint32 last_time;
    Uint32 frame_count;
    float fps;
} demo_state_t;

// Inicializa a janela de demonstração
bool init_demo_window(demo_window_t *window, int width, int height)
{
    window->width = width;
    window->height = height;
    window->running = true;

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "Erro ao inicializar SDL: %s\n", SDL_GetError());
        return false;
    }

    // Criar janela
    window->window = SDL_CreateWindow(
        "Mega_Emu Volumetric Effects Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window->window)
    {
        fprintf(stderr, "Erro ao criar janela: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Criar renderer
    window->renderer = SDL_CreateRenderer(
        window->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    if (!window->renderer)
    {
        fprintf(stderr, "Erro ao criar renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window->window);
        SDL_Quit();
        return false;
    }

    // Criar texturas para o jogo e processamento
    window->game_texture = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    window->processed_texture = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    if (!window->game_texture || !window->processed_texture)
    {
        fprintf(stderr, "Erro ao criar texturas: %s\n", SDL_GetError());
        SDL_DestroyRenderer(window->renderer);
        SDL_DestroyWindow(window->window);
        SDL_Quit();
        return false;
    }

    return true;
}

// Finaliza a janela de demonstração
void shutdown_demo_window(demo_window_t *window)
{
    if (window->processed_texture)
    {
        SDL_DestroyTexture(window->processed_texture);
    }

    if (window->game_texture)
    {
        SDL_DestroyTexture(window->game_texture);
    }

    if (window->renderer)
    {
        SDL_DestroyRenderer(window->renderer);
    }

    if (window->window)
    {
        SDL_DestroyWindow(window->window);
    }

    SDL_Quit();
}

// Inicializa o estado do demo
bool init_demo_state(demo_state_t *state, int width, int height)
{
    // Inicializar janela SDL
    if (!init_demo_window(&state->window, width, height))
    {
        return false;
    }

    // Inicializar backend SDL2
    gui_sdl2_backend_t *sdl_backend = &state->sdl_backend;
    sdl_backend->window = state->window.window;
    sdl_backend->renderer = state->window.renderer;
    sdl_backend->window_size.width = width;
    sdl_backend->window_size.height = height;
    sdl_backend->vsync_enabled = true;
    sdl_backend->scale_factor = 1.0f;

    // Inicializar sistema de efeitos volumétricos
    if (!vol_effects_init_with_sdl2(
            &state->effects_state,
            &state->sdl_backend,
            "shaders/modern/", // Caminho para shaders
            3                  // Qualidade média
            ))
    {
        fprintf(stderr, "Falha ao inicializar efeitos volumétricos\n");
        shutdown_demo_window(&state->window);
        return false;
    }

    // Configurar efeitos iniciais
    state->current_preset = 0; // 0 = NES, 1 = SNES, 2 = Mega Drive
    vol_effects_load_preset(&state->effects_state, "nes");

    // Salvar configuração de luz para manipulação posterior
    memcpy(&state->light_config, &state->effects_state.light, sizeof(vol_light_config_t));

    // Inicializar controles
    state->effects_enabled = true;
    state->light_pos_x = 0.5f;
    state->light_pos_y = 0.3f;

    // Inicializar temporizador FPS
    state->last_time = SDL_GetTicks();
    state->frame_count = 0;
    state->fps = 0.0f;

    return true;
}

// Finaliza o estado do demo
void shutdown_demo_state(demo_state_t *state)
{
    vol_effects_shutdown(&state->effects_state);
    shutdown_demo_window(&state->window);
}

// Renderiza um frame de jogo (simulado)
void render_game_frame(demo_state_t *state)
{
    SDL_Renderer *renderer = state->window.renderer;
    SDL_Texture *game_texture = state->window.game_texture;

    // Definir game_texture como target de renderização
    SDL_SetRenderTarget(renderer, game_texture);

    // Limpar a textura com cor de fundo
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Simular um jogo 2D:

    // 1. Desenhar "céu" (fundo degradê)
    SDL_Rect sky_rect = {0, 0, state->window.width, state->window.height / 2};
    SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
    SDL_RenderFillRect(renderer, &sky_rect);

    // 2. Desenhar "chão"
    SDL_Rect ground_rect = {0, state->window.height / 2, state->window.width, state->window.height / 2};
    SDL_SetRenderDrawColor(renderer, 76, 153, 0, 255);
    SDL_RenderFillRect(renderer, &ground_rect);

    // 3. Desenhar algumas "árvores" (retângulos que bloqueiam a luz)
    SDL_SetRenderDrawColor(renderer, 70, 100, 0, 255);

    const int num_trees = 10;
    for (int i = 0; i < num_trees; i++)
    {
        int tree_x = (state->window.width / num_trees) * i + 20;
        int tree_height = 50 + (i % 3) * 30;

        SDL_Rect tree_rect = {
            tree_x,
            state->window.height / 2 - tree_height,
            20,
            tree_height};

        SDL_RenderFillRect(renderer, &tree_rect);

        // Adicionar copa da árvore
        SDL_SetRenderDrawColor(renderer, 30, 100, 0, 255);
        SDL_Rect canopy_rect = {
            tree_x - 15,
            state->window.height / 2 - tree_height - 30,
            50,
            40};

        SDL_RenderFillRect(renderer, &canopy_rect);
        SDL_SetRenderDrawColor(renderer, 70, 100, 0, 255);
    }

    // 4. Desenhar uma "casa"
    SDL_SetRenderDrawColor(renderer, 200, 100, 50, 255);
    SDL_Rect house_rect = {
        state->window.width / 2 - 50,
        state->window.height / 2 - 80,
        100,
        80};
    SDL_RenderFillRect(renderer, &house_rect);

    // Telhado
    SDL_SetRenderDrawColor(renderer, 150, 50, 50, 255);
    SDL_Point roof_points[3] = {
        {state->window.width / 2 - 60, state->window.height / 2 - 80},
        {state->window.width / 2 + 60, state->window.height / 2 - 80},
        {state->window.width / 2, state->window.height / 2 - 120}};
    SDL_RenderDrawLines(renderer, roof_points, 3);

    // Janela da casa (por onde a luz pode passar)
    SDL_SetRenderDrawColor(renderer, 200, 200, 100, 255);
    SDL_Rect window_rect = {
        state->window.width / 2 - 20,
        state->window.height / 2 - 60,
        15,
        15};
    SDL_RenderFillRect(renderer, &window_rect);

    // Restaurar renderização para a tela
    SDL_SetRenderTarget(renderer, NULL);
}

// Cria uma máscara de luz (objetos que bloqueiam a luz são pretos)
void create_light_mask(demo_state_t *state, SDL_Texture *mask_texture)
{
    SDL_Renderer *renderer = state->window.renderer;

    // Definir máscara como target de renderização
    SDL_SetRenderTarget(renderer, mask_texture);

    // Limpar a textura com cor transparente (luz passa por tudo)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Desenhar áreas opacas (que bloqueiam a luz)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Árvores como obstáculos
    const int num_trees = 10;
    for (int i = 0; i < num_trees; i++)
    {
        int tree_x = (state->window.width / num_trees) * i + 20;
        int tree_height = 50 + (i % 3) * 30;

        // Tronco
        SDL_Rect tree_rect = {
            tree_x,
            state->window.height / 2 - tree_height,
            20,
            tree_height};
        SDL_RenderFillRect(renderer, &tree_rect);

        // Copa da árvore
        SDL_Rect canopy_rect = {
            tree_x - 15,
            state->window.height / 2 - tree_height - 30,
            50,
            40};
        SDL_RenderFillRect(renderer, &canopy_rect);
    }

    // Casa como obstáculo (mas não a janela)
    SDL_Rect house_rect = {
        state->window.width / 2 - 50,
        state->window.height / 2 - 80,
        100,
        80};
    SDL_RenderFillRect(renderer, &house_rect);

    // A janela deixa passar a luz
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect window_rect = {
        state->window.width / 2 - 20,
        state->window.height / 2 - 60,
        15,
        15};
    SDL_RenderFillRect(renderer, &window_rect);

    // Restaurar renderização para a tela
    SDL_SetRenderTarget(renderer, NULL);
}

// Processa eventos de entrada
void process_input(demo_state_t *state)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            state->window.running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                state->window.running = false;
                break;

            case SDLK_e:
                // Ativar/desativar efeitos
                state->effects_enabled = !state->effects_enabled;
                vol_effects_enable(&state->effects_state, state->effects_enabled);
                break;

            case SDLK_1:
            case SDLK_2:
            case SDLK_3:
                // Mudar preset
                if (event.key.keysym.sym == SDLK_1)
                {
                    vol_effects_load_preset(&state->effects_state, "nes");
                    state->current_preset = 0;
                }
                else if (event.key.keysym.sym == SDLK_2)
                {
                    vol_effects_load_preset(&state->effects_state, "snes");
                    state->current_preset = 1;
                }
                else
                {
                    vol_effects_load_preset(&state->effects_state, "megadrive");
                    state->current_preset = 2;
                }

                // Atualizar configuração de luz salva
                memcpy(&state->light_config, &state->effects_state.light, sizeof(vol_light_config_t));
                break;

            case SDLK_UP:
                // Mover luz para cima
                state->light_pos_y -= 0.05f;
                if (state->light_pos_y < 0.0f)
                    state->light_pos_y = 0.0f;
                break;

            case SDLK_DOWN:
                // Mover luz para baixo
                state->light_pos_y += 0.05f;
                if (state->light_pos_y > 1.0f)
                    state->light_pos_y = 1.0f;
                break;

            case SDLK_LEFT:
                // Mover luz para esquerda
                state->light_pos_x -= 0.05f;
                if (state->light_pos_x < 0.0f)
                    state->light_pos_x = 0.0f;
                break;

            case SDLK_RIGHT:
                // Mover luz para direita
                state->light_pos_x += 0.05f;
                if (state->light_pos_x > 1.0f)
                    state->light_pos_x = 1.0f;
                break;

            case SDLK_PLUS:
            case SDLK_KP_PLUS:
                // Aumentar densidade da luz
                state->light_config.density += 0.05f;
                if (state->light_config.density > 1.0f)
                    state->light_config.density = 1.0f;
                break;

            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                // Diminuir densidade da luz
                state->light_config.density -= 0.05f;
                if (state->light_config.density < 0.0f)
                    state->light_config.density = 0.0f;
                break;
            }
            break;
        }
    }

    // Atualizar posição da luz
    state->light_config.position.x = state->light_pos_x;
    state->light_config.position.y = state->light_pos_y;

    // Aplicar configuração de luz
    vol_effects_set_light(&state->effects_state, &state->light_config);
}

// Atualiza estatísticas de FPS
void update_fps(demo_state_t *state)
{
    state->frame_count++;

    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed = current_time - state->last_time;

    // Atualizar FPS a cada segundo
    if (elapsed >= 1000)
    {
        state->fps = (float)state->frame_count * 1000.0f / (float)elapsed;
        state->frame_count = 0;
        state->last_time = current_time;

        printf("FPS: %.2f\n", state->fps);
    }
}

// Renderiza a interface do usuário
void render_ui(demo_state_t *state)
{
    SDL_Renderer *renderer = state->window.renderer;

    // Definir cor branca para texto
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Desenhar um retângulo semi-transparente para o painel de ajuda
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect help_rect = {10, 10, 300, 120};
    SDL_RenderFillRect(renderer, &help_rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Aqui normalmente usaríamos SDL_ttf para renderizar texto,
    // mas para manter o exemplo simples, omitiremos o texto real.

    // Demonstrar visualmente a posição da luz
    int light_x = (int)(state->light_pos_x * state->window.width);
    int light_y = (int)(state->light_pos_y * state->window.height);

    // Desenhar um círculo representando a fonte de luz
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (int w = 0; w < 10; w++)
    {
        for (int h = 0; h < 10; h++)
        {
            int dx = w - 5;
            int dy = h - 5;
            if (dx * dx + dy * dy <= 25)
            {
                SDL_RenderDrawPoint(renderer, light_x + dx, light_y + dy);
            }
        }
    }
}

// Função principal do demo
bool run_demo(demo_state_t *state)
{
    while (state->window.running)
    {
        // Processar entrada do usuário
        process_input(state);

        // Renderizar frame do jogo simulado
        render_game_frame(state);

        // Criar máscara de luz
        SDL_Texture *mask_texture = vol_effects_create_light_mask(
            &state->effects_state,
            &state->sdl_backend,
            state->window.width,
            state->window.height);

        if (mask_texture)
        {
            create_light_mask(state, mask_texture);
        }

        // Aplicar efeitos volumétricos
        vol_effects_apply_to_frame(
            &state->effects_state,
            &state->sdl_backend,
            state->window.game_texture,
            state->window.processed_texture);

        // Renderizar para a tela
        SDL_SetRenderTarget(state->window.renderer, NULL);
        SDL_RenderClear(state->window.renderer);

        // Desenhar textura processada
        SDL_RenderCopy(
            state->window.renderer,
            state->window.processed_texture,
            NULL, NULL);

        // Renderizar UI
        render_ui(state);

        // Apresentar na tela
        SDL_RenderPresent(state->window.renderer);

        // Atualizar FPS
        update_fps(state);

        // Pequeno delay para limitar FPS
        SDL_Delay(16); // ~60 FPS
    }

    return true;
}

// Função main
int main(int argc, char *argv[])
{
    demo_state_t state;

    printf("Inicializando demo de efeitos volumétricos...\n");

    if (!init_demo_state(&state, 800, 600))
    {
        fprintf(stderr, "Falha ao inicializar demo\n");
        return 1;
    }

    printf("Controles:\n");
    printf("E: Ativar/desativar efeitos\n");
    printf("1-3: Mudar preset (NES, SNES, Mega Drive)\n");
    printf("Setas: Mover fonte de luz\n");
    printf("+/-: Ajustar densidade da luz\n");
    printf("ESC: Sair\n\n");

    if (!run_demo(&state))
    {
        fprintf(stderr, "Erro ao executar demo\n");
    }

    printf("Finalizando demo...\n");
    shutdown_demo_state(&state);

    return 0;
}
