/** * @file core.c * @brief Implementação simplificada do core do emulador */                                           \
#include "core.h" #include<stdlib.h> #include<string.h> #include                                                        \
    "cpu/cpu.h" #include                                                                                                \
    "../utils/error_handling.h" /* Implementação simplificada do core */                                                \
    EMU_CORE_API emu_core_t *                                                                                           \
    emu_core_create(                                                                                                    \
        void){emu_core_t *core = (emu_core_t *)malloc(sizeof(emu_core_t)); if (                                         \
                  core){                                                                                                \
        memset(                                                                                                         \
            core, 0,                                                                                                    \
            sizeof(                                                                                                     \
                emu_core_t));} return core;} EMU_CORE_API void emu_core_destroy(emu_core_t                              \
                                                                                    *core){                             \
        if (core){emu_core_shutdown(core); free(                                                                        \
                      core);} return;} EMU_CORE_API bool emu_core_init(emu_core_t                                       \
                                                                           *core){                                      \
        if (!core){return false;} /* Inicializa os campos */ memset(                                                    \
            core, 0, sizeof(emu_core_t));                                                                               \
        /* Cria o sistema de memória */                                                                                 \
        core -> memory = emu_memory_create();                                                                           \
        if (core->memory == NULL){                                                                                      \
            free(core); return false;} /* Inicializa a CPU */ core->cpu =                                               \
            emu_cpu_create(core->memory);                                                                               \
        if (core->cpu == NULL){                                                                                         \
            emu_memory_destroy(core->memory);                                                                           \
            free(core);                                                                                                 \
            return false;} return true;} EMU_CORE_API void emu_core_shutdown(emu_core_t                                 \
                                                                                 *core){                                \
        if (core){core->running = false;                                                                                \
                  /* Libera a CPU */                                                                                    \
                  if (core->cpu != NULL){emu_cpu_destroy(core->cpu);} /* Libera                                         \
                                                                         a                                              \
                                                                         memória                                       \
                                                                       */                                               \
                  if (core->memory != NULL){emu_memory_destroy(core->memory);} /* Limpa os ponteiros dos componentes */ \
                  core->cpu =                                                                                           \
                      NULL;                                                                                             \
                  core->memory =                                                                                        \
                      NULL;} return;} EMU_CORE_API void emu_core_reset(emu_core_t                                       \
                                                                           *core){                                      \
        if (core){core->running = false;                                                                                \
                  core->cycles =                                                                                        \
                      0;} return;} EMU_CORE_API void emu_core_run(emu_core_t *                                          \
                                                                      core){if (core){                                  \
        core->running =                                                                                                 \
            true;} return;} EMU_CORE_API void emu_core_stop(emu_core_t *                                                \
                                                                core){if (core){                                        \
        core->running =                                                                                                 \
            false;} return;} EMU_CORE_API void emu_core_step(emu_core_t *                                               \
                                                                 core){if (core &&                                      \
                                                                           core->running){                              \
        /* Incrementa o contador de ciclos */                                                                           \
        core->cycles++;} return;} EMU_CORE_API void *emu_core_get_cpu(emu_core_t                                        \
                                                                          *core){                                       \
        return core                                                                                                     \
                   ? core->cpu                                                                                          \
                   : NULL;} EMU_CORE_API void *emu_core_get_ppu(emu_core_t *                                            \
                                                                    core){                                              \
        return core                                                                                                     \
                   ? core->ppu                                                                                          \
                   : NULL;} EMU_CORE_API void *emu_core_get_memory(emu_core_t *                                         \
                                                                       core){                                           \
        return core                                                                                                     \
                   ? core->memory                                                                                       \
                   : NULL;} EMU_CORE_API bool emu_core_is_running(emu_core_t                                            \
                                                                      *core){                                           \
        return core                                                                                                     \
                   ? core->running                                                                                      \
                   : false;} EMU_CORE_API uint64_t emu_core_get_cycles(emu_core_t *                                     \
                                                                           core){                                       \
        return core                                                                                                     \
                   ? core->cycles                                                                                       \
                   : 0;} bool emu_core_load_platform(emu_core_t *core, const char *platform_name){                      \
        if (core == NULL ||                                                                                             \
            platform_name == NULL){emu_error_set(                                                                       \
                                       EMU_ERROR_NULL_POINTER, "Core or platform name is NULL",                         \
                                       __FILE__, __LINE__);                                                             \
                                   return false;} /* Implementação                                                    \
                                                     simplificada do                                                    \
                                                     carregamento de                                                    \
                                                     plataforma */                                                      \
        /* Aqui seria implementado o carregamento dinâmico da plataforma */                                             \
        return true;} void emu_core_unload_platform(emu_core_t *core){                                                  \
        if (core ==                                                                                                     \
            NULL){return;} /* Implementação simplificada do descarregamento de                                        \
                              plataforma */                                                                             \
        /* Aqui seria implementado o descarregamento da plataforma */} bool                                             \
        emu_core_frame(emu_core_t *core){if (core == NULL){                                                             \
            emu_error_set(EMU_ERROR_NULL_POINTER, "Core is NULL", __FILE__,                                             \
                          __LINE__);                                                                                    \
            return false;} /* Executa um frame na CPU */ if (core->cpu !=                                               \
                                                             NULL){                                                     \
            if (!emu_cpu_run_frame(core->cpu)){                                                                         \
                return false;}} return true;} bool emu_core_render(emu_core_t                                           \
                                                                       *core){                                          \
            if (core == NULL){emu_error_set(EMU_ERROR_NULL_POINTER,                                                     \
                                            "Core is NULL",                                                             \
                                            __FILE__, __LINE__);                                                        \
                              return false;} /* Renderiza o frame atual */                                              \
            /* Aqui seria implementada a renderização do frame */                                                       \
            return true;} bool emu_core_input(emu_core_t *core,                                                         \
                                              uint32_t input){                                                          \
            if (core == NULL){emu_error_set(EMU_ERROR_NULL_POINTER,                                                     \
                                            "Core is NULL",                                                             \
                                            __FILE__, __LINE__);                                                        \
                              return false;} EMU_UNUSED(input);                                                         \
            return true;} bool emu_core_set_video_mode(emu_core_t *core,                                                \
                                                       uint32_t width,                                                  \
                                                       uint32_t height,                                                 \
                                                       uint32_t format){                                                \
            if (core == NULL){emu_error_set(EMU_ERROR_NULL_POINTER,                                                     \
                                            "Core is NULL",                                                             \
                                            __FILE__, __LINE__);                                                        \
                              return false;} EMU_UNUSED(width);                                                         \
            EMU_UNUSED(height); EMU_UNUSED(format);                                                                     \
            return true;} bool emu_core_set_audio_mode(emu_core_t *core,                                                \
                                                       uint32_t sample_rate,                                            \
                                                       uint32_t channels){                                              \
            if (core == NULL){emu_error_set(EMU_ERROR_NULL_POINTER,                                                     \
                                            "Core is NULL", __FILE__,                                                   \
                                            __LINE__);                                                                  \
                              return false;} EMU_UNUSED(sample_rate);                                                   \
            EMU_UNUSED(channels);                                                                                       \
            return true;} bool emu_core_set_input_mode(emu_core_t *core,                                                \
                                                       uint32_t type){                                                  \
            if (core == NULL){emu_error_set(EMU_ERROR_NULL_POINTER,                                                     \
                                            "Core is NULL", __FILE__,                                                   \
                                            __LINE__);                                                                  \
                              return false;} EMU_UNUSED(type);                                                          \
            return true;}
