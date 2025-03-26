/**
 * @file enhanced_log.h
 * @brief Sistema de log aprimorado com suporte a categorias e níveis
 */

#ifndef EMU_ENHANCED_LOG_H
#define EMU_ENHANCED_LOG_H

#include "common_types.h"
#include "log_categories.h"
#include "log_modules.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Níveis de log disponíveis
 */
typedef enum {
  EMU_LOG_LEVEL_ERROR = 0, // Erros críticos que impedem a execução
  EMU_LOG_LEVEL_WARN,      // Avisos importantes mas não críticos
  EMU_LOG_LEVEL_INFO,      // Informações gerais sobre o estado do sistema
  EMU_LOG_LEVEL_DEBUG,     // Informações detalhadas para debug
  EMU_LOG_LEVEL_TRACE      // Informações muito detalhadas para trace
} emu_log_level_t;

/* Flags de configuração do log */
#define EMU_LOG_FLAG_NONE 0
#define EMU_LOG_FLAG_USE_TIMESTAMP (1 << 0)
#define EMU_LOG_FLAG_USE_LEVEL (1 << 1)
#define EMU_LOG_FLAG_USE_CATEGORY (1 << 2)
#define EMU_LOG_FLAG_USE_FILE_LINE (1 << 3)
#define EMU_LOG_FLAG_USE_COLOR (1 << 4)

typedef int emu_log_flags_t;

/* Estrutura de configuração do log */
struct emu_log_config {
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

/**
 * @brief Inicializa o sistema de log
 * @param log_file Caminho do arquivo de log ou NULL para log no console
 * @return true se sucesso, false caso contrário
 */
bool emu_log_init(const char *log_file);

/**
 * @brief Finaliza o sistema de log
 */
void emu_log_shutdown(void);

/**
 * @brief Define o nível mínimo de log
 * @param level Nível mínimo de log
 */
void emu_log_set_level(emu_log_level_t level);

/**
 * @brief Habilita ou desabilita uma categoria de log
 * @param category Categoria de log
 * @param enabled true para habilitar, false para desabilitar
 */
void emu_log_set_category_enabled(int category, bool enabled);

/**
 * @brief Registra uma mensagem de log
 * @param level Nível da mensagem
 * @param category Categoria da mensagem
 * @param file Arquivo fonte
 * @param line Linha no arquivo
 * @param func Função
 * @param fmt Formato da mensagem
 * @param ... Argumentos do formato
 */
void emu_log_message(emu_log_level_t level, int category, const char *file,
                     int line, const char *func, const char *fmt, ...);

/**
 * @brief Registra uma mensagem de log com argumentos variáveis
 * @param level Nível da mensagem
 * @param category Categoria da mensagem
 * @param file Arquivo fonte
 * @param line Linha no arquivo
 * @param func Função
 * @param fmt Formato da mensagem
 * @param args Lista de argumentos
 */
void emu_log_message_v(emu_log_level_t level, int category, const char *file,
                       int line, const char *func, const char *fmt,
                       va_list args);

// Macros para facilitar o uso do log
#define EMU_LOG_ERROR(cat, ...)                                                \
  emu_log_message(EMU_LOG_LEVEL_ERROR, cat, __FILE__, __LINE__, __func__,      \
                  __VA_ARGS__)

#define EMU_LOG_WARN(cat, ...)                                                 \
  emu_log_message(EMU_LOG_LEVEL_WARN, cat, __FILE__, __LINE__, __func__,       \
                  __VA_ARGS__)

#define EMU_LOG_INFO(cat, ...)                                                 \
  emu_log_message(EMU_LOG_LEVEL_INFO, cat, __FILE__, __LINE__, __func__,       \
                  __VA_ARGS__)

#define EMU_LOG_DEBUG(cat, ...)                                                \
  emu_log_message(EMU_LOG_LEVEL_DEBUG, cat, __FILE__, __LINE__, __func__,      \
                  __VA_ARGS__)

#define EMU_LOG_TRACE(cat, ...)                                                \
  emu_log_message(EMU_LOG_LEVEL_TRACE, cat, __FILE__, __LINE__, __func__,      \
                  __VA_ARGS__)

/* Macros de log simples */
#define LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_WARNING(...) EMU_LOG_WARN(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_CORE, __VA_ARGS__)
#define LOG_FATAL(...) EMU_LOG_ERROR(EMU_LOG_CAT_CORE, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* EMU_ENHANCED_LOG_H */
