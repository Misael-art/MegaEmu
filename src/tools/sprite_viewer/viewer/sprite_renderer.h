#include <stdint.h>#ifndef SPRITE_RENDERER_H#define SPRITE_RENDERER_H#include "core/types.h"#include "core/graphics.h"typedef struct {    float zoom;    bool grid_enabled;    bool show_collision;    ColorMode color_mode;    RenderFlags flags;} ViewerSettings;void sv_init_renderer(void);void sv_render_sprite(Sprite* sprite, ViewerSettings* settings);void sv_render_animation(SpriteSheet* sheet, uint32_t frame);void sv_toggle_grid(bool enabled);void sv_set_zoom(float zoom_level);#endif