#ifndef SDL_GAME_RENDERER_H#define SDL_GAME_RENDERER_H#ifdef __cplusplusextern "C"{#endif#include <SDL2/SDL.h>#include <stdbool.h>#include <stdint.h>    // Configuração do renderizador    typedef struct    {        int32_t window_width;        int32_t window_height;        int32_t game_width;        int32_t game_height;        float scale_factor;        bool vsync_enabled;        bool fullscreen;        bool smooth_scaling;        bool integer_scaling;    } sdl_renderer_config_t;    // Estado do renderizador    typedef struct    {        SDL_Window *window;        SDL_Renderer *renderer;        SDL_Texture *texture;        SDL_Texture *overlay_texture;        uint32_t *frame_buffer;        int32_t game_width;        int32_t game_height;        sdl_renderer_config_t config;        bool initialized;    } sdl_game_renderer_t;    // Funções de inicialização e finalização    bool sdl_game_renderer_init(sdl_game_renderer_t *renderer, const sdl_renderer_config_t *config);    void sdl_game_renderer_shutdown(sdl_game_renderer_t *renderer);    // Funções de renderização    bool sdl_game_renderer_begin_frame(sdl_game_renderer_t *renderer);    bool sdl_game_renderer_end_frame(sdl_game_renderer_t *renderer);    bool sdl_game_renderer_update_game_texture(sdl_game_renderer_t *renderer, const uint32_t *pixels);    bool sdl_game_renderer_draw_frame(sdl_game_renderer_t *renderer);    bool sdl_game_renderer_draw_overlay(sdl_game_renderer_t *renderer, const uint32_t *pixels);    // Funções de configuração    bool sdl_game_renderer_set_config(sdl_game_renderer_t *renderer, const sdl_renderer_config_t *config);    void sdl_game_renderer_get_config(const sdl_game_renderer_t *renderer, sdl_renderer_config_t *config);    bool sdl_game_renderer_toggle_fullscreen(sdl_game_renderer_t *renderer);    bool sdl_game_renderer_set_scale(sdl_game_renderer_t *renderer, float scale);    // Funções de utilidade    void sdl_game_renderer_get_output_size(const sdl_game_renderer_t *renderer, int32_t *width, int32_t *height);    void sdl_game_renderer_get_game_rect(const sdl_game_renderer_t *renderer, SDL_Rect *rect);    bool sdl_game_renderer_handle_resize(sdl_game_renderer_t *renderer, int32_t width, int32_t height);#ifdef __cplusplus}#endif#endif // SDL_GAME_RENDERER_H