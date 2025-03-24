#ifndef GUI_BATCH_RENDERER_H
#define GUI_BATCH_RENDERER_H

#include <stdint.h>
#include "gui_types.h"
#include "sdl2_backend.h"
#include "texture_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define GUI_MAX_BATCH_QUADS 1000
#define GUI_VERTICES_PER_QUAD 4
#define GUI_INDICES_PER_QUAD 6

    // Tipos de blend
    typedef enum gui_blend_mode_e
    {
        GUI_BLEND_NONE,
        GUI_BLEND_ALPHA,
        GUI_BLEND_ADDITIVE,
        GUI_BLEND_MULTIPLY
    } gui_blend_mode_t;

    // Estrutura de textura
    typedef struct gui_texture_s
    {
        SDL_Texture *sdl_texture;
        int width;
        int height;
    } gui_texture_t;

    // Estrutura de fonte
    typedef TTF_Font gui_font_t;

    /* Vértice para renderização em lote */
    typedef struct gui_vertex_s
    {
        float x, y;        /* Posição */
        float u, v;        /* Coordenadas de textura */
        gui_color_t color; /* Cor */
    } gui_vertex_t;

    /* Comando de renderização */
    typedef struct gui_render_command_s
    {
        gui_texture_t *texture;
        size_t quad_count;
        size_t first_index;
        gui_blend_mode_t blend_mode;
    } gui_render_command_t;

    /* Lote de renderização */
    typedef struct gui_batch_renderer_s
    {
        gui_vertex_t vertices[GUI_MAX_BATCH_QUADS * GUI_VERTICES_PER_QUAD];
        uint16_t indices[GUI_MAX_BATCH_QUADS * GUI_INDICES_PER_QUAD];
        gui_render_command_t *commands;
        size_t command_count;
        size_t max_commands;
        size_t vertex_count;
        size_t index_count;
        gui_texture_t *current_texture;
        gui_blend_mode_t current_blend_mode;
    } gui_batch_renderer_t;

    /* Funções de gerenciamento do lote */
    gui_result_t gui_batch_renderer_init(gui_batch_renderer_t *batch);
    void gui_batch_renderer_shutdown(gui_batch_renderer_t *batch);
    void gui_batch_renderer_begin(gui_batch_renderer_t *batch);
    void gui_batch_renderer_end(gui_batch_renderer_t *batch, gui_sdl2_backend_t *backend);
    void gui_batch_renderer_flush(gui_batch_renderer_t *batch, gui_sdl2_backend_t *backend);

    /* Funções de renderização */
    void gui_batch_renderer_draw_quad(gui_batch_renderer_t *batch,
                                      gui_rect_t dst,
                                      gui_rect_t src,
                                      gui_texture_t *texture,
                                      gui_color_t color,
                                      gui_blend_mode_t blend_mode);

    void gui_batch_renderer_draw_rect(gui_batch_renderer_t *batch,
                                      gui_rect_t rect,
                                      gui_color_t color,
                                      gui_blend_mode_t blend_mode);

    void gui_batch_renderer_draw_text(gui_batch_renderer_t *batch,
                                      const char *text,
                                      gui_point_t pos,
                                      gui_font_t *font,
                                      gui_color_t color,
                                      gui_blend_mode_t blend_mode);

    /* Funções auxiliares */
    void gui_batch_renderer_set_viewport(gui_batch_renderer_t *batch,
                                         gui_rect_t viewport);

    void gui_batch_renderer_reset_stats(gui_batch_renderer_t *batch);

    void gui_batch_renderer_get_stats(gui_batch_renderer_t *batch,
                                      size_t *draw_calls,
                                      size_t *vertex_count,
                                      size_t *index_count);

#ifdef __cplusplus
}
#endif

#endif /* GUI_BATCH_RENDERER_H */
