#ifndef EMU_LOG_MODULES_H
#define EMU_LOG_MODULES_H

#include <stdint.h>

/**
 * @file log_modules.h
 * @brief Definição dos módulos de log
 */

#ifdef __cplusplus
extern "C"
{
#endif

    /* IDs dos módulos do sistema - usando nomes diferentes para evitar conflito */
    typedef enum
    {
        EMU_LOG_MODULE_CORE = 100,
        EMU_LOG_MODULE_CPU = 101,
        EMU_LOG_MODULE_MEMORY = 102,
        EMU_LOG_MODULE_VIDEO = 103,
        EMU_LOG_MODULE_AUDIO = 104,
        EMU_LOG_MODULE_INPUT = 105, /* Renomeado para evitar conflito com Windows */
        EMU_LOG_MODULE_SAVE = 106,
        EMU_LOG_MODULE_PERF = 107,
        EMU_LOG_MODULE_MAX = 108
    } emu_log_module_t;

/* Macros de compatibilidade */
#define EMU_CORE EMU_LOG_MODULE_CORE
#define EMU_CPU EMU_LOG_MODULE_CPU
#define EMU_MEMORY EMU_LOG_MODULE_MEMORY
#define EMU_VIDEO EMU_LOG_MODULE_VIDEO
#define EMU_AUDIO EMU_LOG_MODULE_AUDIO
#define EMU_INPUT EMU_LOG_MODULE_INPUT
#define EMU_SAVE EMU_LOG_MODULE_SAVE
#define EMU_PERF EMU_LOG_MODULE_PERF

#ifdef __cplusplus
}
#endif

#endif /* EMU_LOG_MODULES_H */
