#include <stdint.h>#ifndef PLATFORM_TEMPLATES_H#define PLATFORM_TEMPLATES_Htypedef struct {    char* name;    PlatformType platform;    uint32_t width;    uint32_t height;    uint32_t color_depth;    uint32_t max_frames;    bool supports_transparency;} SpriteTemplate;void template_init_system(void);SpriteTemplate* template_create_megadrive_sprite(void);SpriteTemplate* template_create_nes_sprite(void);void template_apply(Sprite* sprite, SpriteTemplate* template);bool template_validate(Sprite* sprite, SpriteTemplate* template);#endif