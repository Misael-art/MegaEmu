#include "sdl_game_renderer.h"
#include <stdio.h>
#include <string.h>

// Tamanho máximo do cache de texturas
#define MAX_TEXTURE_CACHE_SIZE 16

// Estrutura para cache de texturas
typedef struct {
    char key[64];                 // Identificador único da textura
    SDL_Texture *texture;         // Ponteiro para a textura
    int width;                    // Largura da textura
    int height;                   // Altura da textura
    Uint32 last_use_time;         // Tempo da última utilização
    Uint32 creation_time;         // Tempo de criação
} texture_cache_entry_t;

// Cache de texturas
static texture_cache_entry_t texture_cache[MAX_TEXTURE_CACHE_SIZE];
static int texture_cache_count = 0;

// Função auxiliar para criar texturas
static SDL_Texture *create_game_texture(SDL_Renderer *renderer, int width, int height, SDL_TextureAccess access, SDL_PixelFormatEnum format)
{
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        format,          // Formato de pixel
        access,          // Tipo de acesso (streaming ou target)
        width, height);

    if (!texture) {
        fprintf(stderr, "Erro ao criar textura: %s\n", SDL_GetError());
    } else {
        printf("Textura criada com sucesso: %dx%d formato=%s acesso=%s\n",
                width, height,
                format == SDL_PIXELFORMAT_ARGB8888 ? "ARGB8888" : "outro",
                access == SDL_TEXTUREACCESS_STREAMING ? "streaming" :
                access == SDL_TEXTUREACCESS_TARGET ? "target" : "static");
    }

    // Definir modo de blending para alpha
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    return texture;
}

// Função para inicializar o cache de texturas
static void init_texture_cache(void)
{
    memset(texture_cache, 0, sizeof(texture_cache));
    texture_cache_count = 0;
}

// Função para limpar o cache de texturas
static void clear_texture_cache(void)
{
    for (int i = 0; i < texture_cache_count; i++) {
        if (texture_cache[i].texture) {
            SDL_DestroyTexture(texture_cache[i].texture);
            texture_cache[i].texture = NULL;
        }
    }
    texture_cache_count = 0;
}

// Função para obter uma textura do cache ou criar uma nova
static SDL_Texture *get_cached_texture(SDL_Renderer *renderer, const char *key, int width, int height,
                                     SDL_TextureAccess access, SDL_PixelFormatEnum format)
{
    Uint32 current_time = SDL_GetTicks();

    // Procurar no cache
    for (int i = 0; i < texture_cache_count; i++) {
        if (strcmp(texture_cache[i].key, key) == 0 &&
            texture_cache[i].width == width &&
            texture_cache[i].height == height) {

            texture_cache[i].last_use_time = current_time;
            return texture_cache[i].texture;
        }
    }

    // Se o cache estiver cheio, remover a entrada mais antiga
    if (texture_cache_count >= MAX_TEXTURE_CACHE_SIZE) {
        int oldest_idx = 0;
        Uint32 oldest_time = texture_cache[0].last_use_time;

        for (int i = 1; i < texture_cache_count; i++) {
            if (texture_cache[i].last_use_time < oldest_time) {
                oldest_time = texture_cache[i].last_use_time;
                oldest_idx = i;
            }
        }

        // Destruir a textura mais antiga
        if (texture_cache[oldest_idx].texture) {
            SDL_DestroyTexture(texture_cache[oldest_idx].texture);
        }

        // Mover a última entrada para a posição da entrada removida
        if (oldest_idx < texture_cache_count - 1) {
            memcpy(&texture_cache[oldest_idx],
                   &texture_cache[texture_cache_count - 1],
                   sizeof(texture_cache_entry_t));
        }

        texture_cache_count--;
    }

    // Criar nova textura
    SDL_Texture *texture = create_game_texture(renderer, width, height, access, format);
    if (texture) {
        // Adicionar ao cache
        strncpy(texture_cache[texture_cache_count].key, key, sizeof(texture_cache[0].key) - 1);
        texture_cache[texture_cache_count].texture = texture;
        texture_cache[texture_cache_count].width = width;
        texture_cache[texture_cache_count].height = height;
        texture_cache[texture_cache_count].last_use_time = current_time;
        texture_cache[texture_cache_count].creation_time = current_time;
        texture_cache_count++;
    }

    return texture;
}

// Determina a paleta de cores para emuladores específicos
static void setup_color_palette_for_system(sdl_game_renderer_t *renderer, const char *system)
{
    if (!renderer || !system)
        return;

    // Configurar paleta com base no sistema sendo emulado
    if (strcmp(system, "NES") == 0) {
        // Paleta padrão para NES (simplificada)
        for (int i = 0; i < 64; i++) {
            int r = ((i >> 4) & 3) * 85;
            int g = ((i >> 2) & 3) * 85;
            int b = (i & 3) * 85;

            renderer->color_palette[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
        renderer->using_color_palette = true;
    }
    else if (strcmp(system, "MEGA_DRIVE") == 0 || strcmp(system, "GENESIS") == 0) {
        // Para Mega Drive, usamos direto RGB
        renderer->using_color_palette = false;
    }
    else {
        // Sistemas não especificados usam RGB direto
        renderer->using_color_palette = false;
    }
}

bool sdl_game_renderer_init(sdl_game_renderer_t *renderer, const sdl_renderer_config_t *config)
{
    if (!renderer || !config)
        return false;

    // Copiar configuração
    renderer->config = *config;
    renderer->initialized = false;
    renderer->using_color_palette = false;

    // Inicializar SDL se necessário
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            fprintf(stderr, "Erro ao inicializar SDL: %s\n", SDL_GetError());
            return false;
        }
    }

    // Criar janela
    Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (config->fullscreen)
    {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    renderer->window = SDL_CreateWindow(
        "Mega_Emu",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config->window_width, config->window_height,
        window_flags);

    if (!renderer->window)
    {
        fprintf(stderr, "Erro ao criar janela SDL: %s\n", SDL_GetError());
        return false;
    }

    // Criar renderer
    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED;
    if (config->vsync_enabled)
    {
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    }
    renderer_flags |= SDL_RENDERER_TARGETTEXTURE; // Suportar render-to-texture

    renderer->renderer = SDL_CreateRenderer(renderer->window, -1, renderer_flags);
    if (!renderer->renderer)
    {
        fprintf(stderr, "Erro ao criar renderer SDL: %s\n", SDL_GetError());
        SDL_DestroyWindow(renderer->window);
        renderer->window = NULL;
        return false;
    }

    // Configurar qualidade de escala
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
                config->smooth_scaling ? "linear" : "nearest");

    // Configurar escala inteira
    SDL_RenderSetIntegerScale(renderer->renderer,
                              config->integer_scaling ? SDL_TRUE : SDL_FALSE);

    // Inicializar cache de texturas
    init_texture_cache();

    // Criar textura principal do jogo
    renderer->texture = get_cached_texture(
        renderer->renderer,
        "main_game",
        config->game_width,
        config->game_height,
        SDL_TEXTUREACCESS_STREAMING,
        SDL_PIXELFORMAT_ARGB8888);

    if (!renderer->texture)
    {
        fprintf(stderr, "Erro ao criar textura do jogo: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer->renderer);
        SDL_DestroyWindow(renderer->window);
        renderer->renderer = NULL;
        renderer->window = NULL;
        return false;
    }

    // Alocar frame buffer
    renderer->frame_buffer = (uint32_t *)malloc(config->game_width * config->game_height * sizeof(uint32_t));
    if (!renderer->frame_buffer)
    {
        fprintf(stderr, "Erro ao alocar frame buffer\n");
        clear_texture_cache();
        SDL_DestroyRenderer(renderer->renderer);
        SDL_DestroyWindow(renderer->window);
        renderer->texture = NULL;
        renderer->renderer = NULL;
        renderer->window = NULL;
        return false;
    }

    // Configurar dimensões do jogo
    renderer->game_width = config->game_width;
    renderer->game_height = config->game_height;

    // Criar textura de overlay
    renderer->overlay_texture = get_cached_texture(
        renderer->renderer,
        "overlay",
        config->window_width,
        config->window_height,
        SDL_TEXTUREACCESS_TARGET,
        SDL_PIXELFORMAT_ARGB8888);

    if (!renderer->overlay_texture)
    {
        fprintf(stderr, "Erro ao criar textura de overlay: %s\n", SDL_GetError());
        free(renderer->frame_buffer);
        clear_texture_cache();
        SDL_DestroyRenderer(renderer->renderer);
        SDL_DestroyWindow(renderer->window);
        renderer->frame_buffer = NULL;
        renderer->texture = NULL;
        renderer->renderer = NULL;
        renderer->window = NULL;
        return false;
    }

    // Configurar textura de scanlines (opcional para efeito retrô)
    if (config->scanlines_enabled) {
        renderer->scanlines_texture = get_cached_texture(
            renderer->renderer,
            "scanlines",
            config->game_width,
            config->game_height,
            SDL_TEXTUREACCESS_STATIC,
            SDL_PIXELFORMAT_ARGB8888);

        if (renderer->scanlines_texture) {
            // Criar padrão de scanlines
            uint32_t *scanline_pixels = (uint32_t *)malloc(config->game_width * config->game_height * sizeof(uint32_t));
            if (scanline_pixels) {
                for (int y = 0; y < config->game_height; y++) {
                    for (int x = 0; x < config->game_width; x++) {
                        // Linhas pares são semi-transparentes, ímpares são transparentes
                        scanline_pixels[y * config->game_width + x] = (y % 2) ? 0x80000000 : 0x00000000;
                    }
                }
                SDL_UpdateTexture(renderer->scanlines_texture, NULL, scanline_pixels, config->game_width * sizeof(uint32_t));
                free(scanline_pixels);
            }
        }
    } else {
        renderer->scanlines_texture = NULL;
    }

    // Configurar sistema sendo emulado (NES, Mega Drive, etc.)
    setup_color_palette_for_system(renderer, config->system_name);

    renderer->initialized = true;
    return true;
}

void sdl_game_renderer_shutdown(sdl_game_renderer_t *renderer)
{
    if (!renderer)
        return;

    // Limpar o cache de texturas
    clear_texture_cache();

    // Liberar recursos específicos
    if (renderer->frame_buffer)
    {
        free(renderer->frame_buffer);
        renderer->frame_buffer = NULL;
    }

    if (renderer->renderer)
    {
        SDL_DestroyRenderer(renderer->renderer);
        renderer->renderer = NULL;
    }

    if (renderer->window)
    {
        SDL_DestroyWindow(renderer->window);
        renderer->window = NULL;
    }

    renderer->initialized = false;
}

bool sdl_game_renderer_begin_frame(sdl_game_renderer_t *renderer)
{
    if (!renderer || !renderer->initialized)
        return false;

    SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer->renderer);
    return true;
}

bool sdl_game_renderer_end_frame(sdl_game_renderer_t *renderer)
{
    if (!renderer || !renderer->initialized)
        return false;

    SDL_RenderPresent(renderer->renderer);
    return true;
}

bool sdl_game_renderer_update_game_texture(sdl_game_renderer_t *renderer, const uint32_t *pixels)
{
    if (!renderer || !renderer->initialized || !pixels)
        return false;

    void *texture_pixels;
    int pitch;
    if (SDL_LockTexture(renderer->texture, NULL, &texture_pixels, &pitch) < 0)
    {
        fprintf(stderr, "Erro ao bloquear textura: %s\n", SDL_GetError());
        return false;
    }

    // Copia os pixels para a textura
    const int bytes_per_pixel = 4;
    const int row_size = renderer->game_width * bytes_per_pixel;
    uint8_t *dst = (uint8_t *)texture_pixels;

    if (renderer->using_color_palette) {
        // Usar paleta para sistemas como NES
        for (int y = 0; y < renderer->game_height; y++) {
            uint32_t *row_dst = (uint32_t*)(dst + y * pitch);

            for (int x = 0; x < renderer->game_width; x++) {
                uint8_t palette_idx = (uint8_t)pixels[y * renderer->game_width + x];
                if (palette_idx < 64) { // Limite seguro de índice
                    row_dst[x] = renderer->color_palette[palette_idx];
                } else {
                    row_dst[x] = 0xFF000000; // Preto como fallback
                }
            }
        }
    } else {
        // Cópia direta para sistemas como Mega Drive
        const uint8_t *src = (const uint8_t *)pixels;
        for (int y = 0; y < renderer->game_height; y++) {
            memcpy(dst, src, row_size);
            dst += pitch;
            src += row_size;
        }
    }

    SDL_UnlockTexture(renderer->texture);
    return true;
}

bool sdl_game_renderer_draw_frame(sdl_game_renderer_t *renderer)
{
    if (!renderer || !renderer->initialized)
        return false;

    // Limpa o renderer
    SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer->renderer);

    // Calcula o retângulo de destino mantendo a proporção
    SDL_Rect dst_rect;
    int window_width, window_height;
    SDL_GetRendererOutputSize(renderer->renderer, &window_width, &window_height);

    float scale_x = (float)window_width / renderer->game_width;
    float scale_y = (float)window_height / renderer->game_height;
    float scale = renderer->config.integer_scaling ? floorf(fminf(scale_x, scale_y)) : fminf(scale_x, scale_y);

    dst_rect.w = (int)(renderer->game_width * scale);
    dst_rect.h = (int)(renderer->game_height * scale);
    dst_rect.x = (window_width - dst_rect.w) / 2;
    dst_rect.y = (window_height - dst_rect.h) / 2;

    // Configura a qualidade de escala
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
                renderer->config.smooth_scaling ? "linear" : "nearest");

    // Renderiza a textura do jogo
    SDL_RenderCopy(renderer->renderer, renderer->texture, NULL, &dst_rect);

    // Renderiza scanlines se habilitado
    if (renderer->config.scanlines_enabled && renderer->scanlines_texture) {
        SDL_RenderCopy(renderer->renderer, renderer->scanlines_texture, NULL, &dst_rect);
    }

    // Desenha a borda se necessário (para aspect ratio correta)
    if (dst_rect.x > 0 || dst_rect.y > 0) {
        SDL_SetRenderDrawColor(renderer->renderer, 20, 20, 20, 255);

        // Barras horizontais
        if (dst_rect.y > 0) {
            SDL_Rect top_bar = {0, 0, window_width, dst_rect.y};
            SDL_Rect bottom_bar = {0, dst_rect.y + dst_rect.h, window_width, window_height - (dst_rect.y + dst_rect.h)};
            SDL_RenderFillRect(renderer->renderer, &top_bar);
            SDL_RenderFillRect(renderer->renderer, &bottom_bar);
        }

        // Barras verticais
        if (dst_rect.x > 0) {
            SDL_Rect left_bar = {0, dst_rect.y, dst_rect.x, dst_rect.h};
            SDL_Rect right_bar = {dst_rect.x + dst_rect.w, dst_rect.y, window_width - (dst_rect.x + dst_rect.w), dst_rect.h};
            SDL_RenderFillRect(renderer->renderer, &left_bar);
            SDL_RenderFillRect(renderer->renderer, &right_bar);
        }
    }

    // Renderiza overlay se existir
    if (renderer->overlay_texture)
    {
        SDL_RenderCopy(renderer->renderer, renderer->overlay_texture, NULL, NULL);
    }

    return true;
}

bool sdl_game_renderer_present(sdl_game_renderer_t *renderer)
{
    if (!renderer || !renderer->initialized)
        return false;

    SDL_RenderPresent(renderer->renderer);
    return true;
}

bool sdl_game_renderer_draw_overlay(sdl_game_renderer_t *renderer, const uint32_t *pixels)
{
    if (!renderer || !renderer->initialized || !pixels)
        return false;

    void *texture_pixels;
    int pitch;
    if (SDL_LockTexture(renderer->overlay_texture, NULL, &texture_pixels, &pitch) < 0)
    {
        fprintf(stderr, "Erro ao bloquear textura de overlay: %s\n", SDL_GetError());
        return false;
    }

    const int bytes_per_pixel = 4;
    const int row_size = renderer->config.window_width * bytes_per_pixel;
    uint8_t *dst = (uint8_t *)texture_pixels;
    const uint8_t *src = (const uint8_t *)pixels;

    for (int y = 0; y < renderer->config.window_height; y++)
    {
        memcpy(dst, src, row_size);
        dst += pitch;
        src += row_size;
    }

    SDL_UnlockTexture(renderer->overlay_texture);

    // Renderizar textura de overlay
    SDL_RenderCopy(renderer->renderer, renderer->overlay_texture, NULL, NULL);

    return true;
}

// Aplica filtros visuais baseados na plataforma sendo emulada
bool sdl_game_renderer_apply_filter(sdl_game_renderer_t *renderer, const char *filter_name)
{
    if (!renderer || !renderer->initialized || !filter_name)
        return false;

    // Desativar filtro atual
    renderer->config.scanlines_enabled = false;
    renderer->config.smooth_scaling = false;
    renderer->config.crt_effect = false;

    // Aplicar novo filtro
    if (strcmp(filter_name, "scanlines") == 0) {
        renderer->config.scanlines_enabled = true;
    }
    else if (strcmp(filter_name, "smooth") == 0) {
        renderer->config.smooth_scaling = true;
    }
    else if (strcmp(filter_name, "crt") == 0) {
        renderer->config.scanlines_enabled = true;
        renderer->config.crt_effect = true;
    }
    else if (strcmp(filter_name, "pixel_perfect") == 0) {
        renderer->config.integer_scaling = true;
    }

    // Recriar texturas necessárias para o filtro
    if (renderer->config.scanlines_enabled && !renderer->scanlines_texture) {
        renderer->scanlines_texture = get_cached_texture(
            renderer->renderer,
            "scanlines",
            renderer->game_width,
            renderer->game_height,
            SDL_TEXTUREACCESS_STATIC,
            SDL_PIXELFORMAT_ARGB8888);

        if (renderer->scanlines_texture) {
            // Criar padrão de scanlines
            uint32_t *scanline_pixels = (uint32_t *)malloc(renderer->game_width * renderer->game_height * sizeof(uint32_t));
            if (scanline_pixels) {
                for (int y = 0; y < renderer->game_height; y++) {
                    for (int x = 0; x < renderer->game_width; x++) {
                        scanline_pixels[y * renderer->game_width + x] = (y % 2) ? 0x80000000 : 0x00000000;
                    }
                }
                SDL_UpdateTexture(renderer->scanlines_texture, NULL, scanline_pixels, renderer->game_width * sizeof(uint32_t));
                free(scanline_pixels);
            }
        }
    }

    // Atualizar configurações de renderização
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
                renderer->config.smooth_scaling ? "linear" : "nearest");

    SDL_RenderSetIntegerScale(renderer->renderer,
                              renderer->config.integer_scaling ? SDL_TRUE : SDL_FALSE);

    return true;
}

bool sdl_game_renderer_set_config(sdl_game_renderer_t *renderer, const sdl_renderer_config_t *config)
{
    if (!renderer || !config)
        return false;

    // Salvar configuração antiga
    sdl_renderer_config_t old_config = renderer->config;
    renderer->config = *config;

    // Configurar sistema emulado (se alterado)
    if (strcmp(old_config.system_name, config->system_name) != 0) {
        setup_color_palette_for_system(renderer, config->system_name);
    }

    // Recriar texturas se necessário
    if (old_config.game_width != config->game_width ||
        old_config.game_height != config->game_height)
    {
        // Precisamos de uma nova textura principal
        renderer->texture = get_cached_texture(
            renderer->renderer,
            "main_game",
            config->game_width,
            config->game_height,
            SDL_TEXTUREACCESS_STREAMING,
            SDL_PIXELFORMAT_ARGB8888);

        if (!renderer->texture) {
            renderer->config = old_config;
            return false;
        }

        // Realocar frame buffer
        free(renderer->frame_buffer);
        renderer->frame_buffer = (uint32_t *)malloc(config->game_width * config->game_height * sizeof(uint32_t));
        if (!renderer->frame_buffer) {
            renderer->config = old_config;
            return false;
        }

        renderer->game_width = config->game_width;
        renderer->game_height = config->game_height;

        // Recriar texture de scanlines se necessário
        if (config->scanlines_enabled) {
            renderer->scanlines_texture = get_cached_texture(
                renderer->renderer,
                "scanlines",
                config->game_width,
                config->game_height,
                SDL_TEXTUREACCESS_STATIC,
                SDL_PIXELFORMAT_ARGB8888);

            if (renderer->scanlines_texture) {
                // Criar padrão de scanlines
                uint32_t *scanline_pixels = (uint32_t *)malloc(config->game_width * config->game_height * sizeof(uint32_t));
                if (scanline_pixels) {
                    for (int y = 0; y < config->game_height; y++) {
                        for (int x = 0; x < config->game_width; x++) {
                            scanline_pixels[y * config->game_width + x] = (y % 2) ? 0x80000000 : 0x00000000;
                        }
                    }
                    SDL_UpdateTexture(renderer->scanlines_texture, NULL, scanline_pixels, config->game_width * sizeof(uint32_t));
                    free(scanline_pixels);
                }
            }
        } else {
            renderer->scanlines_texture = NULL;
        }
    }

    if (old_config.window_width != config->window_width ||
        old_config.window_height != config->window_height)
    {
        // Precisamos atualizar o overlay
        renderer->overlay_texture = get_cached_texture(
            renderer->renderer,
            "overlay",
            config->window_width,
            config->window_height,
            SDL_TEXTUREACCESS_TARGET,
            SDL_PIXELFORMAT_ARGB8888);

        if (!renderer->overlay_texture) {
            renderer->config = old_config;
            return false;
        }

        SDL_SetWindowSize(renderer->window,
                          config->window_width,
                          config->window_height);
    }

    // Atualizar flags de renderização
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
                config->smooth_scaling ? "linear" : "nearest");

    SDL_RenderSetIntegerScale(renderer->renderer,
                              config->integer_scaling ? SDL_TRUE : SDL_FALSE);

    return true;
}

void sdl_game_renderer_get_config(const sdl_game_renderer_t *renderer, sdl_renderer_config_t *config)
{
    if (!renderer || !config)
        return;

    *config = renderer->config;
}

bool sdl_game_renderer_toggle_fullscreen(sdl_game_renderer_t *renderer)
{
    if (!renderer || !renderer->initialized)
        return false;

    bool is_fullscreen = SDL_GetWindowFlags(renderer->window) & SDL_WINDOW_FULLSCREEN_DESKTOP;

    if (SDL_SetWindowFullscreen(renderer->window,
                                is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
    {
        return false;
    }

    renderer->config.fullscreen = !is_fullscreen;
    return true;
}

bool sdl_game_renderer_set_scale(sdl_game_renderer_t *renderer, float scale)
{
    if (!renderer || !renderer->initialized || scale <= 0)
        return false;

    renderer->config.scale_factor = scale;

    if (!renderer->config.fullscreen)
    {
        SDL_SetWindowSize(renderer->window,
                          (int)(renderer->config.game_width * scale),
                          (int)(renderer->config.game_height * scale));
    }

    return true;
}

void sdl_game_renderer_get_output_size(const sdl_game_renderer_t *renderer, int *width, int *height)
{
    if (!renderer || !renderer->initialized)
        return;

    SDL_GetRendererOutputSize(renderer->renderer, width, height);
}

void sdl_game_renderer_get_game_rect(const sdl_game_renderer_t *renderer, SDL_Rect *rect)
{
    if (!renderer || !renderer->initialized || !rect)
        return;

    int window_width, window_height;
    SDL_GetRendererOutputSize(renderer->renderer, &window_width, &window_height);

    float game_aspect = (float)renderer->config.game_width / renderer->config.game_height;
    float window_aspect = (float)window_width / window_height;

    if (game_aspect > window_aspect)
    {
        // Ajustar pela largura
        rect->w = window_width;
        rect->h = (int)(window_width / game_aspect);
        rect->x = 0;
        rect->y = (window_height - rect->h) / 2;
    }
    else
    {
        // Ajustar pela altura
        rect->h = window_height;
        rect->w = (int)(window_height * game_aspect);
        rect->x = (window_width - rect->w) / 2;
        rect->y = 0;
    }
}

bool sdl_game_renderer_handle_resize(sdl_game_renderer_t *renderer, int width, int height)
{
    if (!renderer || !renderer->initialized)
        return false;

    renderer->config.window_width = width;
    renderer->config.window_height = height;

    // Recriar textura de overlay com novo tamanho
    renderer->overlay_texture = get_cached_texture(
        renderer->renderer,
        "overlay",
        width,
        height,
        SDL_TEXTUREACCESS_TARGET,
        SDL_PIXELFORMAT_ARGB8888);

    return renderer->overlay_texture != NULL;
}

/**
 * @brief Aplica efeito de scanlines ao frame renderizado
 *
 * @param renderer Renderizador do jogo
 * @param intensity Intensidade do efeito (0.0 a 1.0)
 * @return true Se aplicado com sucesso
 * @return false Se falhou
 */
bool sdl_game_renderer_apply_scanlines(sdl_game_renderer_t *renderer, float intensity) {
    if (!renderer || !renderer->renderer || !renderer->texture) {
        return false;
    }

    // Criar texture de scanlines se ainda não existe
    if (!renderer->scanlines_texture) {
        int texture_width, texture_height;
        SDL_QueryTexture(renderer->texture, NULL, NULL, &texture_width, &texture_height);

        renderer->scanlines_texture = SDL_CreateTexture(
            renderer->renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            texture_width,
            texture_height
        );

        if (!renderer->scanlines_texture) {
            return false;
        }

        // Configurar blending para a textura de scanlines
        SDL_SetTextureBlendMode(renderer->scanlines_texture, SDL_BLENDMODE_BLEND);

        // Renderizar padrão de scanlines na textura
        SDL_SetRenderTarget(renderer->renderer, renderer->scanlines_texture);
        SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer->renderer);

        // Desenhar linhas horizontais a cada 2 pixels
        SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 128); // Valor alpha determina intensidade base

        for (int y = 0; y < texture_height; y += 2) {
            SDL_RenderDrawLine(renderer->renderer, 0, y, texture_width, y);
        }

        // Voltar para o render target padrão
        SDL_SetRenderTarget(renderer->renderer, NULL);
    }

    // Aplicar a intensidade ajustando o alpha da textura
    uint8_t alpha = (uint8_t)(intensity * 128);
    SDL_SetTextureAlphaMod(renderer->scanlines_texture, alpha);

    // Obter o retângulo do jogo
    SDL_Rect game_rect;
    sdl_game_renderer_get_game_rect(renderer, &game_rect);

    // Renderizar a textura de scanlines sobre o frame do jogo
    SDL_RenderCopy(renderer->renderer, renderer->scanlines_texture, NULL, &game_rect);

    return true;
}

/**
 * @brief Aplica efeito CRT (tubo de raios catódicos) ao frame renderizado
 *
 * @param renderer Renderizador do jogo
 * @param curvature Intensidade da curvatura (0.0 a 1.0)
 * @param corner_size Tamanho dos cantos arredondados (0.0 a 1.0)
 * @param vignette Intensidade do efeito vignette (0.0 a 1.0)
 * @return true Se aplicado com sucesso
 * @return false Se falhou
 */
bool sdl_game_renderer_apply_crt(sdl_game_renderer_t *renderer, float curvature, float corner_size, float vignette) {
    if (!renderer || !renderer->renderer || !renderer->texture) {
        return false;
    }

    // Aplicar shader CRT usando SDL2_gfx (versão simplificada)
    // Em uma implementação completa, isto usaria um fragment shader

    // Obter retângulo do jogo e dimensões da textura
    SDL_Rect game_rect;
    sdl_game_renderer_get_game_rect(renderer, &game_rect);

    int texture_width, texture_height;
    SDL_QueryTexture(renderer->texture, NULL, NULL, &texture_width, &texture_height);

    // 1. Aplicar curvatura - criamos uma textura temporária distorcida
    SDL_Texture *distorted_texture = SDL_CreateTexture(
        renderer->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        game_rect.w,
        game_rect.h
    );

    if (!distorted_texture) {
        return false;
    }

    // Configurar blending para a textura distorcida
    SDL_SetTextureBlendMode(distorted_texture, SDL_BLENDMODE_BLEND);

    // Renderizar para a textura distorcida
    SDL_SetRenderTarget(renderer->renderer, distorted_texture);
    SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer->renderer);

    // Aplicar distorção - dividimos a textura em uma grade e aplicamos
    // transformação para simular curvatura CRT
    int grid_size = 16; // Número de divisões na grade

    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            // Coordenadas normalizadas (0-1) para esta célula da grade
            float u1 = (float)i / grid_size;
            float v1 = (float)j / grid_size;
            float u2 = (float)(i + 1) / grid_size;
            float v2 = (float)(j + 1) / grid_size;

            // Converter para coordenadas de pixel na textura
            int x1 = (int)(u1 * texture_width);
            int y1 = (int)(v1 * texture_height);
            int x2 = (int)(u2 * texture_width);
            int y2 = (int)(v2 * texture_height);

            // Origem da célula na grade para a textura fonte
            SDL_Rect src_rect = {x1, y1, x2 - x1, y2 - y1};

            // Aplicar distorção para simular curvatura
            // Mapear para coordenadas -1 a 1 no espaço de tela
            float nx1 = u1 * 2.0f - 1.0f;
            float ny1 = v1 * 2.0f - 1.0f;
            float nx2 = u2 * 2.0f - 1.0f;
            float ny2 = v2 * 2.0f - 1.0f;

            // Aplicar função de distorção quadrática
            // r = raio da coordenada normalizada
            float r1 = sqrtf(nx1 * nx1 + ny1 * ny1);
            float r2 = sqrtf(nx2 * nx2 + ny2 * ny2);

            // Fator de distorção
            float d1 = 1.0f + (curvature * r1 * r1);
            float d2 = 1.0f + (curvature * r2 * r2);

            // Aplicar distorção
            float dx1 = nx1 * d1;
            float dy1 = ny1 * d1;
            float dx2 = nx2 * d2;
            float dy2 = ny2 * d2;

            // Voltar para o espaço de tela (0-1)
            float tu1 = (dx1 + 1.0f) * 0.5f;
            float tv1 = (dy1 + 1.0f) * 0.5f;
            float tu2 = (dx2 + 1.0f) * 0.5f;
            float tv2 = (dy2 + 1.0f) * 0.5f;

            // Converter para coordenadas de pixel no destino
            int ox1 = (int)(tu1 * game_rect.w);
            int oy1 = (int)(tv1 * game_rect.h);
            int ox2 = (int)(tu2 * game_rect.w);
            int oy2 = (int)(tv2 * game_rect.h);

            // Destino da célula na textura distorcida
            SDL_Rect dst_rect = {ox1, oy1, ox2 - ox1, oy2 - oy1};

            // Renderizar esta célula da grade
            SDL_RenderCopy(renderer->renderer, renderer->texture, &src_rect, &dst_rect);
        }
    }

    // 2. Aplicar efeito vignette (escurecimento nas bordas)
    if (vignette > 0.0f) {
        // Criar gradiente radial para vignette
        for (int y = 0; y < game_rect.h; y++) {
            for (int x = 0; x < game_rect.w; x++) {
                // Converter para coordenadas normalizadas centradas (-1 a 1)
                float nx = (float)x / game_rect.w * 2.0f - 1.0f;
                float ny = (float)y / game_rect.h * 2.0f - 1.0f;

                // Calcular distância ao centro (0 no centro, 1 nas bordas)
                float dist = sqrtf(nx * nx + ny * ny);

                // Aplicar função de falloff para o vignette
                float factor = dist * 1.414f; // Fator 1.414 = sqrt(2) para normalizar
                float alpha = vignette * std::min(1.0f, factor * factor);

                // Desenhar pixel com alpha proporcional à distância
                uint8_t a = (uint8_t)(alpha * 255);
                if (a > 0) {
                    SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, a);
                    SDL_RenderDrawPoint(renderer->renderer, x, y);
                }
            }
        }
    }

    // 3. Aplicar cantos arredondados
    if (corner_size > 0.0f) {
        int corner_radius = (int)(corner_size * std::min(game_rect.w, game_rect.h) * 0.25f);

        // Dibujar máscaras en las esquinas
        SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 255);

        // Esquina superior izquierda
        for (int y = 0; y < corner_radius; y++) {
            for (int x = 0; x < corner_radius; x++) {
                float dist = sqrtf((float)((corner_radius - x) * (corner_radius - x) +
                                         (corner_radius - y) * (corner_radius - y)));
                if (dist > corner_radius) {
                    SDL_RenderDrawPoint(renderer->renderer, x, y);
                }
            }
        }

        // Esquina superior derecha
        for (int y = 0; y < corner_radius; y++) {
            for (int x = game_rect.w - corner_radius; x < game_rect.w; x++) {
                float dist = sqrtf((float)((x - (game_rect.w - corner_radius)) * (x - (game_rect.w - corner_radius)) +
                                         (corner_radius - y) * (corner_radius - y)));
                if (dist > corner_radius) {
                    SDL_RenderDrawPoint(renderer->renderer, x, y);
                }
            }
        }

        // Esquina inferior izquierda
        for (int y = game_rect.h - corner_radius; y < game_rect.h; y++) {
            for (int x = 0; x < corner_radius; x++) {
                float dist = sqrtf((float)((corner_radius - x) * (corner_radius - x) +
                                         (y - (game_rect.h - corner_radius)) * (y - (game_rect.h - corner_radius))));
                if (dist > corner_radius) {
                    SDL_RenderDrawPoint(renderer->renderer, x, y);
                }
            }
        }

        // Esquina inferior derecha
        for (int y = game_rect.h - corner_radius; y < game_rect.h; y++) {
            for (int x = game_rect.w - corner_radius; x < game_rect.w; x++) {
                float dist = sqrtf((float)((x - (game_rect.w - corner_radius)) * (x - (game_rect.w - corner_radius)) +
                                         (y - (game_rect.h - corner_radius)) * (y - (game_rect.h - corner_radius))));
                if (dist > corner_radius) {
                    SDL_RenderDrawPoint(renderer->renderer, x, y);
                }
            }
        }
    }

    // Voltar ao render target padrão
    SDL_SetRenderTarget(renderer->renderer, NULL);

    // Renderizar a textura distorcida para a tela
    SDL_RenderCopy(renderer->renderer, distorted_texture, NULL, &game_rect);

    // Liberar textura temporária
    SDL_DestroyTexture(distorted_texture);

    return true;
}

/**
 * @brief Aplica efeito de filtro retro ao jogo
 *
 * @param renderer Renderizador do jogo
 * @param filter_type Tipo de filtro (1=Blur, 2=Pixelate, 3=Sepia, 4=Noise)
 * @param intensity Intensidade do filtro (0.0 a 1.0)
 * @return true Se aplicado com sucesso
 * @return false Se falhou
 */
bool sdl_game_renderer_apply_retro_filter(sdl_game_renderer_t *renderer, int filter_type, float intensity) {
    if (!renderer || !renderer->renderer || !renderer->texture) {
        return false;
    }

    // Obter retângulo do jogo
    SDL_Rect game_rect;
    sdl_game_renderer_get_game_rect(renderer, &game_rect);

    // Criar textura temporária para aplicar o filtro
    SDL_Texture *filter_texture = SDL_CreateTexture(
        renderer->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        game_rect.w,
        game_rect.h
    );

    if (!filter_texture) {
        return false;
    }

    // Configurar blending para a textura de filtro
    SDL_SetTextureBlendMode(filter_texture, SDL_BLENDMODE_BLEND);

    // Renderizar para a textura de filtro
    SDL_SetRenderTarget(renderer->renderer, filter_texture);

    // Copiar o frame atual para a textura de filtro
    SDL_RenderCopy(renderer->renderer, renderer->texture, NULL, NULL);

    // Aplicar o filtro especificado
    switch (filter_type) {
        case 1: // Blur
            // Aplicar blur com múltiplas passadas
            {
                int blur_passes = (int)(intensity * 5.0f) + 1;
                int blur_size = (int)(intensity * 2.0f) + 1;

                // Textura temporária para o efeito de blur
                SDL_Texture *blur_temp = SDL_CreateTexture(
                    renderer->renderer,
                    SDL_PIXELFORMAT_RGBA8888,
                    SDL_TEXTUREACCESS_TARGET,
                    game_rect.w,
                    game_rect.h
                );

                if (blur_temp) {
                    SDL_SetTextureBlendMode(blur_temp, SDL_BLENDMODE_BLEND);

                    // Alternar entre as texturas para múltiplas passadas
                    SDL_Texture *src = filter_texture;
                    SDL_Texture *dst = blur_temp;

                    for (int pass = 0; pass < blur_passes; pass++) {
                        SDL_SetRenderTarget(renderer->renderer, dst);
                        SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 0);
                        SDL_RenderClear(renderer->renderer);

                        // Renderizar múltiplas cópias com pequenos deslocamentos
                        for (int y = -blur_size; y <= blur_size; y++) {
                            for (int x = -blur_size; x <= blur_size; x++) {
                                // Posição com deslocamento
                                SDL_Rect offset_rect = {
                                    x, y,
                                    game_rect.w,
                                    game_rect.h
                                };

                                // Desenhar com alpha reduzido
                                SDL_SetTextureAlphaMod(src, 255 / ((blur_size * 2 + 1) * (blur_size * 2 + 1)));
                                SDL_RenderCopy(renderer->renderer, src, NULL, &offset_rect);
                            }
                        }

                        // Trocar texturas para próxima passada
                        SDL_Texture *temp = src;
                        src = dst;
                        dst = temp;
                    }

                    // Copiar resultado final para a textura de filtro
                    SDL_SetRenderTarget(renderer->renderer, filter_texture);
                    SDL_SetTextureAlphaMod(src, 255);
                    SDL_RenderCopy(renderer->renderer, src, NULL, NULL);

                    SDL_DestroyTexture(blur_temp);
                }
            }
            break;

        case 2: // Pixelate
            {
                // Tamanho dos pixels (maior = mais pixelado)
                int pixel_size = (int)(intensity * 8.0f) + 1;

                if (pixel_size > 1) {
                    // Criar textura de baixa resolução
                    SDL_Texture *low_res = SDL_CreateTexture(
                        renderer->renderer,
                        SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET,
                        game_rect.w / pixel_size,
                        game_rect.h / pixel_size
                    );

                    if (low_res) {
                        // Renderizar para textura de baixa resolução
                        SDL_SetRenderTarget(renderer->renderer, low_res);
                        SDL_RenderCopy(renderer->renderer, filter_texture, NULL, NULL);

                        // Renderizar textura de baixa resolução para a textura de filtro (ampliada)
                        SDL_SetRenderTarget(renderer->renderer, filter_texture);
                        SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 0);
                        SDL_RenderClear(renderer->renderer);
                        SDL_RenderCopy(renderer->renderer, low_res, NULL, NULL);

                        SDL_DestroyTexture(low_res);
                    }
                }
            }
            break;

        case 3: // Sepia
            {
                // Criar um overlay para efeito sepia
                SDL_SetRenderDrawColor(renderer->renderer, 112, 66, 20, (Uint8)(intensity * 128));
                SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
                SDL_RenderFillRect(renderer->renderer, NULL);
            }
            break;

        case 4: // Noise
            {
                // Adicionar ruído estático aleatório
                int noise_amount = (int)(intensity * 128);

                SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);

                for (int i = 0; i < game_rect.w * game_rect.h * intensity / 10; i++) {
                    int x = rand() % game_rect.w;
                    int y = rand() % game_rect.h;
                    int gray = rand() % 256;

                    SDL_SetRenderDrawColor(renderer->renderer, gray, gray, gray, noise_amount);
                    SDL_RenderDrawPoint(renderer->renderer, x, y);
                }
            }
            break;
    }

    // Voltar ao render target padrão
    SDL_SetRenderTarget(renderer->renderer, NULL);

    // Renderizar a textura de filtro para a tela
    SDL_RenderCopy(renderer->renderer, filter_texture, NULL, &game_rect);

    // Liberar textura temporária
    SDL_DestroyTexture(filter_texture);

    return true;
}

/**
 * @brief Aplica um filtro visual ao frame do jogo
 *
 * @param renderer Ponteiro para o renderizador
 * @param filter_name Nome do filtro a ser aplicado
 * @return true Se o filtro foi aplicado com sucesso
 * @return false Se houve erro ou o filtro não existe
 */
bool sdl_game_renderer_apply_filter(sdl_game_renderer_t *renderer, const char *filter_name) {
    if (!renderer || !filter_name) {
        return false;
    }

    // Configurações para filtros visuais
    sdl_renderer_config_t *config = &renderer->config;

    // Aplicar filtros solicitados
    if (strcmp(filter_name, "scanlines") == 0) {
        return sdl_game_renderer_apply_scanlines(renderer, 0.5f);
    }
    else if (strcmp(filter_name, "crt") == 0) {
        return sdl_game_renderer_apply_crt(renderer, 0.1f, 0.1f, 0.3f);
    }
    else if (strcmp(filter_name, "pixelate") == 0) {
        return sdl_game_renderer_apply_retro_filter(renderer, 2, 0.5f);
    }
    else if (strcmp(filter_name, "blur") == 0) {
        return sdl_game_renderer_apply_retro_filter(renderer, 1, 0.3f);
    }
    else if (strcmp(filter_name, "sepia") == 0) {
        return sdl_game_renderer_apply_retro_filter(renderer, 3, 0.7f);
    }
    else if (strcmp(filter_name, "noise") == 0) {
        return sdl_game_renderer_apply_retro_filter(renderer, 4, 0.2f);
    }

    // Filtro não encontrado
    return false;
}
