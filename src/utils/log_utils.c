/**
 * @file log_utils.c
 * @brief Implementação simplificada do sistema de log para uso legado
 */

#include "log_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Inicializa o sistema de log para escrita em arquivo
 *
 * @param filename Nome do arquivo para escrita de log
 */
void emu_log_init_file(const char *filename) {
  emu_log_config_t config;
  memset(&config, 0, sizeof(config));

  config.level = EMU_LOG_LEVEL_INFO;
  config.flags = EMU_LOG_FLAG_USE_TIMESTAMP | EMU_LOG_FLAG_USE_LEVEL |
                 EMU_LOG_FLAG_USE_CATEGORY;
  config.output_file = filename;

  emu_log_init(&config);
  emu_log_set_file(filename);
}

/**
 * @brief Fecha o arquivo de log se estiver aberto
 */
void emu_log_close_file(void) { emu_log_shutdown(); }

/**
 * @brief Define o arquivo de saída para o log
 *
 * @param output Ponteiro para o arquivo de saída
 */
void emu_log_set_output(FILE *output) {
  emu_log_config_t config;
  memset(&config, 0, sizeof(config));

  config.level = EMU_LOG_LEVEL_INFO;
  config.flags = EMU_LOG_FLAG_USE_TIMESTAMP | EMU_LOG_FLAG_USE_LEVEL |
                 EMU_LOG_FLAG_USE_CATEGORY;
  config.output = output;

  emu_log_init(&config);
}

/**
 * @brief Escreve uma mensagem de log
 *
 * @param level Nível da mensagem
 * @param format String de formato
 * @param ... Argumentos variáveis para o formato
 */
void emu_log_write(emu_log_level_t level, const char *format, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // Converte a chamada para a nova API de log
  switch (level) {
  case EMU_LOG_LEVEL_ERROR:
    EMU_LOG_ERROR(EMU_LOG_CAT_CORE, "%s", buffer);
    break;
  case EMU_LOG_LEVEL_WARNING:
    EMU_LOG_WARNING(EMU_LOG_CAT_CORE, "%s", buffer);
    break;
  case EMU_LOG_LEVEL_INFO:
    EMU_LOG_INFO(EMU_LOG_CAT_CORE, "%s", buffer);
    break;
  case EMU_LOG_LEVEL_DEBUG:
    EMU_LOG_DEBUG(EMU_LOG_CAT_CORE, "%s", buffer);
    break;
  default:
    EMU_LOG_INFO(EMU_LOG_CAT_CORE, "%s", buffer);
    break;
  }
}
