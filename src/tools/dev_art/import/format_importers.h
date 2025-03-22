#include <stdint.h>typedef struct {    const char* format_name;    const char* file_extension;    ImportOptions options;} ImporterDescriptor;typedef struct {    bool (*validate)(const char* file_path);    SpriteData* (*import)(const char* file_path);    void (*cleanup)(void);} ImporterOps;typedef struct {    ImporterDescriptor descriptor;    ImporterOps ops;} FormatImporter;// Registradores de importadoresvoid register_spine_importer(void);void register_godot_importer(void);void register_unreal_importer(void);void register_construct_importer(void);