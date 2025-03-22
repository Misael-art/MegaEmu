#include <stdint.h>#ifndef EXPORT_SYSTEM_H#define EXPORT_SYSTEM_Htypedef enum {    EXPORT_C_ARRAY,    EXPORT_ASM,    EXPORT_BINARY,    EXPORT_JSON,    EXPORT_PNG_SHEET} ExportFormat;typedef struct {    ExportFormat format;    bool include_metadata;    bool optimize_output;    bool split_animations;    char* output_path;} ExportOptions;void export_init_system(void);void export_sprite(Sprite* sprite, ExportOptions* options);void export_animation(Animation* anim, ExportOptions* options);void export_palette(CustomPalette* pal, ExportOptions* options);bool export_validate_output(const char* output_path, ExportFormat format);#endif