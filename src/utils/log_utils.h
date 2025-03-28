/**
 * @file log_utils.h
 * @brief Interface do sistema de logging legado
 */
#ifndef EMU_LOG_UTILS_H
#define EMU_LOG_UTILS_H

#include "enhanced_log.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Funções de interface */
void emu_log_init_file(const char *filename);
void emu_log_close_file(void);
void emu_log_set_output(FILE *output);
void emu_log_write(emu_log_level_t level, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* EMU_LOG_UTILS_H */
