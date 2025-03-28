#ifndef GUI_TEXTURE_MANAGER_H
#define GUI_TEXTURE_MANAGER_H

#include <stdint.h>
#include "gui_types.h"
#include "sdl2_backend.h"
#include <SDL2/SDL_ttf.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Tipos de flip para texturas
    typedef enum gui_flip_e
    {
        GUI_FLIP_NONE = SDL_FLIP_NONE,
        GUI_FLIP_HORIZONTAL = SDL_FLIP_HORIZONTAL,
        GUI_FLIP_VERTICAL = SDL_FLIP_VERTICAL
    } gui_flip_t;

    /* Estrutura de textura */
    typedef struct gui_texture_s
    {
        SDL_Texture *handle;
        int32_t width;
        int32_t height;
        bool is_render_target;
    } gui_texture_t;

    /* Estrutura de fonte */
    typedef struct gui_font_s
    {
        TTF_Font *handle;
        int32_t size;
        const char *path;
    } gui_font_t;

    /* Funções de gerenciamento de texturas */
    gui_result_t gui_texture_init(void);
    void gui_texture_shutdown(void);

    gui_texture_t *gui_texture_create(gui_sdl2_backend_t *backend,
                                      int32_t width,
                                      int32_t height);

    gui_texture_t *gui_texture_load(gui_sdl2_backend_t *backend,
                                    const char *path);

    void gui_texture_destroy(gui_texture_t *texture);

    /* Funções de renderização de texturas */
    gui_result_t gui_texture_render(gui_sdl2_backend_t *backend,
                                    gui_texture_t *texture,
                                    gui_rect_t src,
                                    gui_rect_t dst,
                                    float rotation,
                                    gui_point_t center,
                                    gui_flip_t flip);

    gui_result_t gui_texture_set_color_mod(gui_texture_t *texture,
                                           gui_color_t color);

    gui_result_t gui_texture_set_blend_mode(gui_texture_t *texture,
                                            gui_blend_mode_t blend_mode);

    gui_result_t gui_texture_set_alpha_mod(gui_texture_t *texture,
                                           uint8_t alpha);

    /* Funções de gerenciamento de fontes */
    gui_result_t gui_font_init(void);
    void gui_font_shutdown(void);

    gui_font_t *gui_font_load(const char *path, int32_t size);
    void gui_font_destroy(gui_font_t *font);

    /* Funções de renderização de texto */
    gui_texture_t *gui_font_render_text(gui_sdl2_backend_t *backend,
                                        gui_font_t *font,
                                        const char *text,
                                        gui_color_t color);

    gui_texture_t *gui_font_render_text_shaded(gui_sdl2_backend_t *backend,
                                               gui_font_t *font,
                                               const char *text,
                                               gui_color_t fg_color,
                                               gui_color_t bg_color);

    gui_texture_t *gui_font_render_text_blended(gui_sdl2_backend_t *backend,
                                                gui_font_t *font,
                                                const char *text,
                                                gui_color_t color);

    /* Funções de medição de texto */
    void gui_font_measure_text(gui_font_t *font,
                               const char *text,
                               int32_t *width,
                               int32_t *height);

#ifdef __cplusplus
}
#endif

#endif /* GUI_TEXTURE_MANAGER_H */
