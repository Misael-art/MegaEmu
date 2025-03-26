#ifndef SDL_GAME_RENDERER_H
#define SDL_GAME_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

// Tamanho máximo da paleta de cores (64 cores para NES)
#define COLOR_PALETTE_SIZE 64

// Configuração do renderizador
typedef struct {
  int32_t window_width;
  int32_t window_height;
  int32_t game_width;
  int32_t game_height;
  float scale_factor;
  bool vsync_enabled;
  bool fullscreen;
  bool smooth_scaling;
  bool integer_scaling;
  bool scanlines_enabled;
  bool crt_effect;
  char system_name[32]; // Nome do sistema: "NES", "MEGA_DRIVE", etc.
} sdl_renderer_config_t;

// Estado do renderizador
typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Texture *overlay_texture;
  SDL_Texture *scanlines_texture; // Textura para efeito de scanlines
  uint32_t *frame_buffer;
  int32_t game_width;
  int32_t game_height;
  sdl_renderer_config_t config;
  bool initialized;

  // Suporte a paleta de cores para sistemas específicos (NES, etc.)
  uint32_t color_palette[COLOR_PALETTE_SIZE];
  bool using_color_palette;
} sdl_game_renderer_t;

// Funções de inicialização e finalização
bool sdl_game_renderer_init(sdl_game_renderer_t *renderer,
                            const sdl_renderer_config_t *config);
void sdl_game_renderer_shutdown(sdl_game_renderer_t *renderer);

// Funções de renderização
bool sdl_game_renderer_begin_frame(sdl_game_renderer_t *renderer);
bool sdl_game_renderer_end_frame(sdl_game_renderer_t *renderer);
bool sdl_game_renderer_update_game_texture(sdl_game_renderer_t *renderer,
                                           const uint32_t *pixels);
bool sdl_game_renderer_draw_frame(sdl_game_renderer_t *renderer);
bool sdl_game_renderer_present(sdl_game_renderer_t *renderer);
bool sdl_game_renderer_draw_overlay(sdl_game_renderer_t *renderer,
                                    const uint32_t *pixels);

// Funções de efeitos visuais
bool sdl_game_renderer_apply_filter(sdl_game_renderer_t *renderer,
                                    const char *filter_name);

// Funções de configuração
bool sdl_game_renderer_set_config(sdl_game_renderer_t *renderer,
                                  const sdl_renderer_config_t *config);
void sdl_game_renderer_get_config(const sdl_game_renderer_t *renderer,
                                  sdl_renderer_config_t *config);
bool sdl_game_renderer_toggle_fullscreen(sdl_game_renderer_t *renderer);
bool sdl_game_renderer_set_scale(sdl_game_renderer_t *renderer, float scale);
bool sdl_game_renderer_set_smooth_scaling(sdl_game_renderer_t *renderer,
                                         bool smooth_scaling);
bool sdl_game_renderer_set_integer_scaling(sdl_game_renderer_t *renderer,
                                          bool integer_scaling);
bool sdl_game_renderer_set_scanlines(sdl_game_renderer_t *renderer,
                                    bool enabled);
bool sdl_game_renderer_set_crt_effect(sdl_game_renderer_t *renderer,
                                     bool enabled);
bool sdl_game_renderer_set_color_palette(sdl_game_renderer_t *renderer,
                                        const uint32_t *palette,
                                        int palette_size);

// Funções de utilidade
void sdl_game_renderer_get_output_size(const sdl_game_renderer_t *renderer,
                                       int *width, int *height);
void sdl_game_renderer_get_game_rect(const sdl_game_renderer_t *renderer,
                                     SDL_Rect *rect);
bool sdl_game_renderer_handle_resize(sdl_game_renderer_t *renderer, int width,
                                     int height);

#ifdef __cplusplus
}
#endif

#endif // SDL_GAME_RENDERER_H
