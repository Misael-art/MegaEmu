/**
 * @file config.c
 * @brief Sistema de configuração do emulador
 */

#include "config.h"
#include "../utils/enhanced_log.h"
#include "../utils/log_categories.h"
#include <cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_CONFIG EMU_LOG_CAT_CORE

// Macros de log
#define CONFIG_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_CONFIG, __VA_ARGS__)
#define CONFIG_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_CONFIG, __VA_ARGS__)
#define CONFIG_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_CONFIG, __VA_ARGS__)
#define CONFIG_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_CONFIG, __VA_ARGS__)
#define CONFIG_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_CONFIG, __VA_ARGS__)

// Valores padrão
#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 480
#define DEFAULT_FULLSCREEN false
#define DEFAULT_VSYNC true
#define DEFAULT_AUDIO_ENABLED true
#define DEFAULT_AUDIO_VOLUME 100
#define DEFAULT_AUDIO_SAMPLE_RATE 44100
#define DEFAULT_AUDIO_BUFFER_SIZE 2048
#define DEFAULT_INPUT_DEADZONE 8000
#define DEFAULT_SAVE_STATE_SLOTS 10
#define DEFAULT_AUTO_SAVE_INTERVAL 300 // 5 minutos
#define DEFAULT_REWIND_ENABLED true
#define DEFAULT_REWIND_BUFFER_SIZE 60 // 1 minuto
#define DEFAULT_SHADER_ENABLED false
#define DEFAULT_BILINEAR_FILTER false

/**
 * @brief Estrutura de configuração do emulador
 */
struct config_t {
    // Vídeo
    struct {
        int window_width;
        int window_height;
        bool fullscreen;
        bool vsync;
        bool shader_enabled;
        bool bilinear_filter;
        char shader_path[256];
    } video;

    // Áudio
    struct {
        bool enabled;
        int volume;
        int sample_rate;
        int buffer_size;
    } audio;

    // Input
    struct {
        int deadzone;
        struct {
            int device;
            int button;
        } mappings[16];
    } input;

    // Sistema
    struct {
        int save_state_slots;
        int auto_save_interval;
        bool rewind_enabled;
        int rewind_buffer_size;
        char save_path[256];
        char screenshot_path[256];
        char bios_path[256];
    } system;

    // Paths
    char config_path[256];
    bool dirty;
};

/**
 * @brief Cria uma nova instância de configuração
 */
config_t *config_create(void) {
    config_t *config = (config_t *)malloc(sizeof(config_t));
    if (!config) {
        CONFIG_LOG_ERROR("Falha ao alocar memória para configuração");
        return NULL;
    }

    // Inicializa com valores padrão
    memset(config, 0, sizeof(config_t));

    // Vídeo
    config->video.window_width = DEFAULT_WINDOW_WIDTH;
    config->video.window_height = DEFAULT_WINDOW_HEIGHT;
    config->video.fullscreen = DEFAULT_FULLSCREEN;
    config->video.vsync = DEFAULT_VSYNC;
    config->video.shader_enabled = DEFAULT_SHADER_ENABLED;
    config->video.bilinear_filter = DEFAULT_BILINEAR_FILTER;

    // Áudio
    config->audio.enabled = DEFAULT_AUDIO_ENABLED;
    config->audio.volume = DEFAULT_AUDIO_VOLUME;
    config->audio.sample_rate = DEFAULT_AUDIO_SAMPLE_RATE;
    config->audio.buffer_size = DEFAULT_AUDIO_BUFFER_SIZE;

    // Input
    config->input.deadzone = DEFAULT_INPUT_DEADZONE;

    // Sistema
    config->system.save_state_slots = DEFAULT_SAVE_STATE_SLOTS;
    config->system.auto_save_interval = DEFAULT_AUTO_SAVE_INTERVAL;
    config->system.rewind_enabled = DEFAULT_REWIND_ENABLED;
    config->system.rewind_buffer_size = DEFAULT_REWIND_BUFFER_SIZE;

    CONFIG_LOG_INFO("Configuração criada com valores padrão");
    return config;
}

/**
 * @brief Destrói uma instância de configuração
 */
void config_destroy(config_t *config) {
    if (!config) return;

    // Salva configurações se houver alterações
    if (config->dirty) {
        config_save(config);
    }

    free(config);
    CONFIG_LOG_INFO("Configuração destruída");
}

/**
 * @brief Carrega configurações de um arquivo JSON
 */
bool config_load(config_t *config, const char *path) {
    if (!config || !path) return false;

    // Abre o arquivo
    FILE *file = fopen(path, "rb");
    if (!file) {
        CONFIG_LOG_WARN("Arquivo de configuração não encontrado: %s", path);
        return false;
    }

    // Lê o arquivo
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *json = (char *)malloc(size + 1);
    if (!json) {
        CONFIG_LOG_ERROR("Falha ao alocar memória para JSON");
        fclose(file);
        return false;
    }

    fread(json, 1, size, file);
    json[size] = '\0';
    fclose(file);

    // Parse JSON
    cJSON *root = cJSON_Parse(json);
    free(json);

    if (!root) {
        CONFIG_LOG_ERROR("Falha ao fazer parse do JSON");
        return false;
    }

    // Vídeo
    cJSON *video = cJSON_GetObjectItem(root, "video");
    if (video) {
        cJSON *width = cJSON_GetObjectItem(video, "window_width");
        if (width) config->video.window_width = width->valueint;

        cJSON *height = cJSON_GetObjectItem(video, "window_height");
        if (height) config->video.window_height = height->valueint;

        cJSON *fullscreen = cJSON_GetObjectItem(video, "fullscreen");
        if (fullscreen) config->video.fullscreen = fullscreen->valueint;

        cJSON *vsync = cJSON_GetObjectItem(video, "vsync");
        if (vsync) config->video.vsync = vsync->valueint;

        cJSON *shader = cJSON_GetObjectItem(video, "shader_enabled");
        if (shader) config->video.shader_enabled = shader->valueint;

        cJSON *filter = cJSON_GetObjectItem(video, "bilinear_filter");
        if (filter) config->video.bilinear_filter = filter->valueint;

        cJSON *shader_path = cJSON_GetObjectItem(video, "shader_path");
        if (shader_path) strncpy(config->video.shader_path, shader_path->valuestring, 255);
    }

    // Áudio
    cJSON *audio = cJSON_GetObjectItem(root, "audio");
    if (audio) {
        cJSON *enabled = cJSON_GetObjectItem(audio, "enabled");
        if (enabled) config->audio.enabled = enabled->valueint;

        cJSON *volume = cJSON_GetObjectItem(audio, "volume");
        if (volume) config->audio.volume = volume->valueint;

        cJSON *sample_rate = cJSON_GetObjectItem(audio, "sample_rate");
        if (sample_rate) config->audio.sample_rate = sample_rate->valueint;

        cJSON *buffer_size = cJSON_GetObjectItem(audio, "buffer_size");
        if (buffer_size) config->audio.buffer_size = buffer_size->valueint;
    }

    // Input
    cJSON *input = cJSON_GetObjectItem(root, "input");
    if (input) {
        cJSON *deadzone = cJSON_GetObjectItem(input, "deadzone");
        if (deadzone) config->input.deadzone = deadzone->valueint;

        cJSON *mappings = cJSON_GetObjectItem(input, "mappings");
        if (mappings) {
            for (int i = 0; i < cJSON_GetArraySize(mappings) && i < 16; i++) {
                cJSON *mapping = cJSON_GetArrayItem(mappings, i);
                if (mapping) {
                    cJSON *device = cJSON_GetObjectItem(mapping, "device");
                    if (device) config->input.mappings[i].device = device->valueint;

                    cJSON *button = cJSON_GetObjectItem(mapping, "button");
                    if (button) config->input.mappings[i].button = button->valueint;
                }
            }
        }
    }

    // Sistema
    cJSON *system = cJSON_GetObjectItem(root, "system");
    if (system) {
        cJSON *slots = cJSON_GetObjectItem(system, "save_state_slots");
        if (slots) config->system.save_state_slots = slots->valueint;

        cJSON *auto_save = cJSON_GetObjectItem(system, "auto_save_interval");
        if (auto_save) config->system.auto_save_interval = auto_save->valueint;

        cJSON *rewind = cJSON_GetObjectItem(system, "rewind_enabled");
        if (rewind) config->system.rewind_enabled = rewind->valueint;

        cJSON *rewind_size = cJSON_GetObjectItem(system, "rewind_buffer_size");
        if (rewind_size) config->system.rewind_buffer_size = rewind_size->valueint;

        cJSON *save_path = cJSON_GetObjectItem(system, "save_path");
        if (save_path) strncpy(config->system.save_path, save_path->valuestring, 255);

        cJSON *screenshot_path = cJSON_GetObjectItem(system, "screenshot_path");
        if (screenshot_path) strncpy(config->system.screenshot_path, screenshot_path->valuestring, 255);

        cJSON *bios_path = cJSON_GetObjectItem(system, "bios_path");
        if (bios_path) strncpy(config->system.bios_path, bios_path->valuestring, 255);
    }

    cJSON_Delete(root);
    strncpy(config->config_path, path, 255);
    config->dirty = false;

    CONFIG_LOG_INFO("Configuração carregada: %s", path);
    return true;
}

/**
 * @brief Salva configurações em um arquivo JSON
 */
bool config_save(config_t *config) {
    if (!config || !config->config_path[0]) return false;

    // Cria objeto JSON
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        CONFIG_LOG_ERROR("Falha ao criar objeto JSON");
        return false;
    }

    // Vídeo
    cJSON *video = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "video", video);
    cJSON_AddNumberToObject(video, "window_width", config->video.window_width);
    cJSON_AddNumberToObject(video, "window_height", config->video.window_height);
    cJSON_AddBoolToObject(video, "fullscreen", config->video.fullscreen);
    cJSON_AddBoolToObject(video, "vsync", config->video.vsync);
    cJSON_AddBoolToObject(video, "shader_enabled", config->video.shader_enabled);
    cJSON_AddBoolToObject(video, "bilinear_filter", config->video.bilinear_filter);
    cJSON_AddStringToObject(video, "shader_path", config->video.shader_path);

    // Áudio
    cJSON *audio = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "audio", audio);
    cJSON_AddBoolToObject(audio, "enabled", config->audio.enabled);
    cJSON_AddNumberToObject(audio, "volume", config->audio.volume);
    cJSON_AddNumberToObject(audio, "sample_rate", config->audio.sample_rate);
    cJSON_AddNumberToObject(audio, "buffer_size", config->audio.buffer_size);

    // Input
    cJSON *input = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "input", input);
    cJSON_AddNumberToObject(input, "deadzone", config->input.deadzone);

    cJSON *mappings = cJSON_CreateArray();
    cJSON_AddItemToObject(input, "mappings", mappings);
    for (int i = 0; i < 16; i++) {
        cJSON *mapping = cJSON_CreateObject();
        cJSON_AddItemToArray(mappings, mapping);
        cJSON_AddNumberToObject(mapping, "device", config->input.mappings[i].device);
        cJSON_AddNumberToObject(mapping, "button", config->input.mappings[i].button);
    }

    // Sistema
    cJSON *system = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "system", system);
    cJSON_AddNumberToObject(system, "save_state_slots", config->system.save_state_slots);
    cJSON_AddNumberToObject(system, "auto_save_interval", config->system.auto_save_interval);
    cJSON_AddBoolToObject(system, "rewind_enabled", config->system.rewind_enabled);
    cJSON_AddNumberToObject(system, "rewind_buffer_size", config->system.rewind_buffer_size);
    cJSON_AddStringToObject(system, "save_path", config->system.save_path);
    cJSON_AddStringToObject(system, "screenshot_path", config->system.screenshot_path);
    cJSON_AddStringToObject(system, "bios_path", config->system.bios_path);

    // Converte para string
    char *json = cJSON_Print(root);
    cJSON_Delete(root);

    if (!json) {
        CONFIG_LOG_ERROR("Falha ao converter JSON para string");
        return false;
    }

    // Salva no arquivo
    FILE *file = fopen(config->config_path, "wb");
    if (!file) {
        CONFIG_LOG_ERROR("Falha ao abrir arquivo para escrita: %s", config->config_path);
        free(json);
        return false;
    }

    fprintf(file, "%s", json);
    fclose(file);
    free(json);

    config->dirty = false;
    CONFIG_LOG_INFO("Configuração salva: %s", config->config_path);
    return true;
}

/**
 * @brief Obtém uma configuração de vídeo
 */
bool config_get_video(config_t *config, const char *key, void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "window_width") == 0) {
        *(int *)value = config->video.window_width;
    } else if (strcmp(key, "window_height") == 0) {
        *(int *)value = config->video.window_height;
    } else if (strcmp(key, "fullscreen") == 0) {
        *(bool *)value = config->video.fullscreen;
    } else if (strcmp(key, "vsync") == 0) {
        *(bool *)value = config->video.vsync;
    } else if (strcmp(key, "shader_enabled") == 0) {
        *(bool *)value = config->video.shader_enabled;
    } else if (strcmp(key, "bilinear_filter") == 0) {
        *(bool *)value = config->video.bilinear_filter;
    } else if (strcmp(key, "shader_path") == 0) {
        strcpy((char *)value, config->video.shader_path);
    } else {
        return false;
    }

    return true;
}

/**
 * @brief Define uma configuração de vídeo
 */
bool config_set_video(config_t *config, const char *key, const void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "window_width") == 0) {
        config->video.window_width = *(int *)value;
    } else if (strcmp(key, "window_height") == 0) {
        config->video.window_height = *(int *)value;
    } else if (strcmp(key, "fullscreen") == 0) {
        config->video.fullscreen = *(bool *)value;
    } else if (strcmp(key, "vsync") == 0) {
        config->video.vsync = *(bool *)value;
    } else if (strcmp(key, "shader_enabled") == 0) {
        config->video.shader_enabled = *(bool *)value;
    } else if (strcmp(key, "bilinear_filter") == 0) {
        config->video.bilinear_filter = *(bool *)value;
    } else if (strcmp(key, "shader_path") == 0) {
        strncpy(config->video.shader_path, (char *)value, 255);
    } else {
        return false;
    }

    config->dirty = true;
    return true;
}

/**
 * @brief Obtém uma configuração de áudio
 */
bool config_get_audio(config_t *config, const char *key, void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "enabled") == 0) {
        *(bool *)value = config->audio.enabled;
    } else if (strcmp(key, "volume") == 0) {
        *(int *)value = config->audio.volume;
    } else if (strcmp(key, "sample_rate") == 0) {
        *(int *)value = config->audio.sample_rate;
    } else if (strcmp(key, "buffer_size") == 0) {
        *(int *)value = config->audio.buffer_size;
    } else {
        return false;
    }

    return true;
}

/**
 * @brief Define uma configuração de áudio
 */
bool config_set_audio(config_t *config, const char *key, const void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "enabled") == 0) {
        config->audio.enabled = *(bool *)value;
    } else if (strcmp(key, "volume") == 0) {
        config->audio.volume = *(int *)value;
    } else if (strcmp(key, "sample_rate") == 0) {
        config->audio.sample_rate = *(int *)value;
    } else if (strcmp(key, "buffer_size") == 0) {
        config->audio.buffer_size = *(int *)value;
    } else {
        return false;
    }

    config->dirty = true;
    return true;
}

/**
 * @brief Obtém uma configuração de input
 */
bool config_get_input(config_t *config, const char *key, void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "deadzone") == 0) {
        *(int *)value = config->input.deadzone;
    } else if (strncmp(key, "mapping_", 8) == 0) {
        int index = atoi(key + 8);
        if (index >= 0 && index < 16) {
            *(struct input_mapping *)value = config->input.mappings[index];
        } else {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

/**
 * @brief Define uma configuração de input
 */
bool config_set_input(config_t *config, const char *key, const void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "deadzone") == 0) {
        config->input.deadzone = *(int *)value;
    } else if (strncmp(key, "mapping_", 8) == 0) {
        int index = atoi(key + 8);
        if (index >= 0 && index < 16) {
            config->input.mappings[index] = *(struct input_mapping *)value;
        } else {
            return false;
        }
    } else {
        return false;
    }

    config->dirty = true;
    return true;
}

/**
 * @brief Obtém uma configuração do sistema
 */
bool config_get_system(config_t *config, const char *key, void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "save_state_slots") == 0) {
        *(int *)value = config->system.save_state_slots;
    } else if (strcmp(key, "auto_save_interval") == 0) {
        *(int *)value = config->system.auto_save_interval;
    } else if (strcmp(key, "rewind_enabled") == 0) {
        *(bool *)value = config->system.rewind_enabled;
    } else if (strcmp(key, "rewind_buffer_size") == 0) {
        *(int *)value = config->system.rewind_buffer_size;
    } else if (strcmp(key, "save_path") == 0) {
        strcpy((char *)value, config->system.save_path);
    } else if (strcmp(key, "screenshot_path") == 0) {
        strcpy((char *)value, config->system.screenshot_path);
    } else if (strcmp(key, "bios_path") == 0) {
        strcpy((char *)value, config->system.bios_path);
    } else {
        return false;
    }

    return true;
}

/**
 * @brief Define uma configuração do sistema
 */
bool config_set_system(config_t *config, const char *key, const void *value) {
    if (!config || !key || !value) return false;

    if (strcmp(key, "save_state_slots") == 0) {
        config->system.save_state_slots = *(int *)value;
    } else if (strcmp(key, "auto_save_interval") == 0) {
        config->system.auto_save_interval = *(int *)value;
    } else if (strcmp(key, "rewind_enabled") == 0) {
        config->system.rewind_enabled = *(bool *)value;
    } else if (strcmp(key, "rewind_buffer_size") == 0) {
        config->system.rewind_buffer_size = *(int *)value;
    } else if (strcmp(key, "save_path") == 0) {
        strncpy(config->system.save_path, (char *)value, 255);
    } else if (strcmp(key, "screenshot_path") == 0) {
        strncpy(config->system.screenshot_path, (char *)value, 255);
    } else if (strcmp(key, "bios_path") == 0) {
        strncpy(config->system.bios_path, (char *)value, 255);
    } else {
        return false;
    }

    config->dirty = true;
    return true;
}
