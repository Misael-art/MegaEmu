#include <stdint.h>typedef struct {    uint32_t major;    uint32_t minor;    uint32_t patch;    const char* hash;} ShaderVersion;typedef struct {    ShaderVersion version;    time_t timestamp;    const char* author;    const char* changelog;    const char* compatibility;} ShaderMetadata;typedef struct {    CustomShader shader;    ShaderMetadata metadata;    ShaderVersion* version_history;    size_t history_count;} VersionedShader;void shader_version_init(void);ShaderVersion shader_create_version(const char* source);bool shader_validate_version(const ShaderVersion* version);void shader_store_history(VersionedShader* shader);