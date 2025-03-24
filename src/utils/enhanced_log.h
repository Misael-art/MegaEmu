#ifndef EMU_ENHANCED_LOG_H
#define EMU_ENHANCED_LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "common_types.h"
#include "log_modules.h"
#include "log_categories.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Níveis de log */
#define EMU_LOG_LEVEL_ERROR 0
#define EMU_LOG_LEVEL_WARNING 1
#define EMU_LOG_LEVEL_INFO 2
#define EMU_LOG_LEVEL_DEBUG 3
#define EMU_LOG_LEVEL_TRACE 4
#define EMU_LOG_LEVEL_MAX 5

    typedef int emu_log_level_t;

/* Flags de configuração do log */
#define EMU_LOG_FLAG_NONE 0
#define EMU_LOG_FLAG_USE_TIMESTAMP (1 << 0)
#define EMU_LOG_FLAG_USE_LEVEL (1 << 1)
#define EMU_LOG_FLAG_USE_CATEGORY (1 << 2)
#define EMU_LOG_FLAG_USE_FILE_LINE (1 << 3)
#define EMU_LOG_FLAG_USE_COLOR (1 << 4)

    typedef int emu_log_flags_t;

    /* Estrutura de configuração do log */
    struct emu_log_config
    {
        emu_log_level_t level;
        FILE *output;
        emu_log_flags_t flags;
        /* Campos adicionais para compatibilidade com testes */
        const char *output_file; /* Nome do arquivo de saída, se usado */
        bool use_timestamp;      /* Flag para uso de timestamp */
        bool use_level;          /* Flag para mostrar o nível de log */
        bool use_category;       /* Flag para mostrar a categoria */
    };

    typedef struct emu_log_config emu_log_config_t;

    /* Funções de interface */
    bool emu_log_init(const emu_log_config_t *config);
    void emu_log_shutdown(void);
    void emu_log_set_level(emu_log_level_t level);
    void emu_log_set_module_level(emu_log_module_t module, emu_log_level_t level);
    void emu_log_set_category_level(emu_log_category_t category, emu_log_level_t level);
    void emu_log_set_file(const char *filename);
    void emu_log_message(emu_log_level_t level, emu_log_category_t category, const char *file, int32_t line, const char *fmt, ...);
    bool emu_enhanced_log_is_enabled(emu_log_level_t level);

/* Macros de log por categoria */
#define EMU_LOG_ERROR(category, ...) \
    emu_log_message(EMU_LOG_LEVEL_ERROR, category, __FILE__, __LINE__, __VA_ARGS__)

#define EMU_LOG_WARN(category, ...) \
    emu_log_message(EMU_LOG_LEVEL_WARNING, category, __FILE__, __LINE__, __VA_ARGS__)

#define EMU_LOG_WARNING(category, ...) \
    emu_log_message(EMU_LOG_LEVEL_WARNING, category, __FILE__, __LINE__, __VA_ARGS__)

#define EMU_LOG_INFO(category, ...) \
    emu_log_message(EMU_LOG_LEVEL_INFO, category, __FILE__, __LINE__, __VA_ARGS__)

#define EMU_LOG_DEBUG(category, ...) \
    emu_log_message(EMU_LOG_LEVEL_DEBUG, category, __FILE__, __LINE__, __VA_ARGS__)

#define EMU_LOG_TRACE(category, ...) \
    emu_log_message(EMU_LOG_LEVEL_TRACE, category, __FILE__, __LINE__, __VA_ARGS__)

/* Macros de log simples */
#define LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_WARNING(...) EMU_LOG_WARNING(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_FATAL(...) EMU_LOG_ERROR(EMU_LOG_CAT_CORE, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* EMU_ENHANCED_LOG_H */
