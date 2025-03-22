#include <stdint.h>#ifndef FORMAT_HANDLER_H#define FORMAT_HANDLER_Htypedef enum {    FORMAT_PNG,    FORMAT_BMP,    FORMAT_GIF,    FORMAT_ASEPRITE,    FORMAT_NATIVE} ImportFormat;typedef struct {    ImportFormat format;    const char* extension;    bool (*validate)(const char* filename);    SpriteSheet* (*import)(const char* filename);} FormatHandler;// Registro de handlers de formatovoid sv_register_format_handler(FormatHandler* handler);SpriteSheet* sv_import_file(const char* filename);bool sv_is_format_supported(const char* filename);#endif