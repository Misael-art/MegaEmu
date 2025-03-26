/**
 * @file enhanced_log.c
 * @brief Implementação do sistema de log aprimorado
 */

#include "enhanced_log.h"
#include "log_categories.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Strings para os níveis de log
static const char *level_strings[] = {"ERROR", "WARN", "INFO", "DEBUG",
                                      "TRACE"};

// Cores ANSI para console
static const char *level_colors[] = {
    "\x1b[31m", // ERROR - Vermelho
    "\x1b[33m", // WARN - Amarelo
    "\x1b[32m", // INFO - Verde
    "\x1b[36m", // DEBUG - Ciano
    "\x1b[35m"  // TRACE - Magenta
};

// Estado do sistema de log
static struct {
  FILE *file;                               // Arquivo de log
  emu_log_level_t level;                    // Nível mínimo de log
  bool category_enabled[EMU_LOG_CAT_COUNT]; // Estado de cada categoria
  bool use_colors;                          // Usar cores no console
  char timestamp[32];                       // Buffer para timestamp
} log_state;

/**
 * @brief Obtém o timestamp atual formatado
 * @return String com o timestamp
 */
static const char *get_timestamp(void) {
  time_t now;
  struct tm *timeinfo;

  time(&now);
  timeinfo = localtime(&now);

  strftime(log_state.timestamp, sizeof(log_state.timestamp),
           "%Y-%m-%d %H:%M:%S", timeinfo);

  return log_state.timestamp;
}

/**
 * @brief Inicializa o sistema de log
 */
bool emu_log_init(const char *log_file) {
  // Inicializa o estado
  memset(&log_state, 0, sizeof(log_state));
  log_state.level = EMU_LOG_LEVEL_INFO;
  log_state.use_colors = true;

  // Habilita todas as categorias por padrão
  for (int i = 0; i < EMU_LOG_CAT_COUNT; i++) {
    log_state.category_enabled[i] = true;
  }

  // Abre o arquivo de log se especificado
  if (log_file) {
    log_state.file = fopen(log_file, "w");
    if (!log_state.file) {
      fprintf(stderr, "Erro ao abrir arquivo de log: %s\n", log_file);
      return false;
    }
  }

  // Log inicial
  emu_log_message(EMU_LOG_LEVEL_INFO, EMU_LOG_CAT_CORE, __FILE__, __LINE__,
                  __func__, "Sistema de log inicializado");

  return true;
}

/**
 * @brief Finaliza o sistema de log
 */
void emu_log_shutdown(void) {
  if (log_state.file) {
    emu_log_message(EMU_LOG_LEVEL_INFO, EMU_LOG_CAT_CORE, __FILE__, __LINE__,
                    __func__, "Sistema de log finalizado");
    fclose(log_state.file);
    log_state.file = NULL;
  }
}

/**
 * @brief Define o nível mínimo de log
 */
void emu_log_set_level(emu_log_level_t level) {
  if (level >= EMU_LOG_LEVEL_ERROR && level <= EMU_LOG_LEVEL_TRACE) {
    log_state.level = level;
    emu_log_message(EMU_LOG_LEVEL_INFO, EMU_LOG_CAT_CORE, __FILE__, __LINE__,
                    __func__, "Nível de log alterado para %s",
                    level_strings[level]);
  }
}

/**
 * @brief Habilita ou desabilita uma categoria de log
 */
void emu_log_set_category_enabled(int category, bool enabled) {
  if (category >= 0 && category < EMU_LOG_CAT_COUNT) {
    log_state.category_enabled[category] = enabled;
    emu_log_message(EMU_LOG_LEVEL_INFO, EMU_LOG_CAT_CORE, __FILE__, __LINE__,
                    __func__, "Categoria %s %s",
                    emu_log_category_names[category],
                    enabled ? "habilitada" : "desabilitada");
  }
}

/**
 * @brief Registra uma mensagem de log com argumentos variáveis
 */
void emu_log_message_v(emu_log_level_t level, int category, const char *file,
                       int line, const char *func, const char *fmt,
                       va_list args) {
  // Verifica o nível e categoria
  if (level > log_state.level || !log_state.category_enabled[category]) {
    return;
  }

  // Buffer para a mensagem formatada
  char message[1024];
  vsnprintf(message, sizeof(message), fmt, args);

  // Obtém apenas o nome do arquivo sem o path
  const char *filename = strrchr(file, '/');
  if (filename) {
    filename++;
  } else {
    filename = file;
  }

  // Formata a mensagem de log
  char full_message[2048];
  snprintf(full_message, sizeof(full_message), "%s [%s] [%s] %s:%d %s(): %s\n",
           get_timestamp(), level_strings[level],
           emu_log_category_names[category], filename, line, func, message);

  // Escreve no arquivo se estiver aberto
  if (log_state.file) {
    fputs(full_message, log_state.file);
    fflush(log_state.file);
  }

  // Escreve no console com cores se habilitado
  if (log_state.use_colors) {
    fprintf(stderr, "%s%s\x1b[0m", level_colors[level], full_message);
  } else {
    fputs(full_message, stderr);
  }
}

/**
 * @brief Registra uma mensagem de log
 */
void emu_log_message(emu_log_level_t level, int category, const char *file,
                     int line, const char *func, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  emu_log_message_v(level, category, file, line, func, fmt, args);
  va_end(args);
}
