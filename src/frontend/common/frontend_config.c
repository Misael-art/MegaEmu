/**
 * @file frontend_config.c
 * @brief Implementação das funções de configuração do frontend
 */

#include "frontend_config.h"
#include "utils/enhanced_log.h"
#include "utils/log_categories.h"
#include "utils/log_utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_LINE_MAX 256
#define CONFIG_FILE_HEADER                                                     \
  "# Configuração do emulador\n# Gerado automaticamente\n\n"

// Configuração padrão do frontend
const emu_frontend_config_t EMU_DEFAULT_FRONTEND_CONFIG = {
    // Vídeo
    .window_width = 800,
    .window_height = 600,
    .game_width = 256,
    .game_height = 240,
    .scale_factor = 2.0f,
    .vsync_enabled = true,
    .fullscreen = false,
    .smooth_scaling = false,
    .integer_scaling = true,

    // Áudio
    .audio_sample_rate = 44100,
    .audio_buffer_size = 2048,
    .audio_enabled = true,

    // Entrada
    .keyboard_enabled = true,
    .gamepad_enabled = true,

    // Interface
    .show_fps = true,
    .debug_overlay = false,

    // Theme (valor padrão)
    .theme_id = 0,

    // Sem dados específicos do frontend
    .frontend_specific = NULL};

// Configuração global
static emu_frontend_config_t g_config = {0};

/**
 * @brief Inicializa a configuração do frontend
 */
void emu_frontend_config_init(void) {
  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Inicializando configuração do frontend");

  // Define os valores padrão
  emu_frontend_config_set_defaults(&g_config);
}

/**
 * @brief Define os valores padrão para uma configuração
 */
void emu_frontend_config_set_defaults(emu_frontend_config_t *config) {
  if (!config) {
    EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND,
                  "Ponteiro de configuração inválido ao definir padrões");
    return;
  }

  memcpy(config, &EMU_DEFAULT_FRONTEND_CONFIG, sizeof(emu_frontend_config_t));

  // Preservamos o ponteiro frontend_specific se já existir
  void *specific = config->frontend_specific;
  *config = EMU_DEFAULT_FRONTEND_CONFIG;
  config->frontend_specific = specific;
}

/**
 * @brief Obtém um ponteiro para a configuração global
 */
emu_frontend_config_t *emu_frontend_config_get(void) { return &g_config; }

/**
 * @brief Remove espaços em branco do início e do fim da string
 *
 * @param str String a ser processada
 */
static void trim(char *str) {
  char *start = str;
  char *end = str + strlen(str) - 1;

  // Remover espaços do início
  while (isspace((unsigned char)*start))
    start++;

  // Se a string é só espaços
  if (*start == 0) {
    *str = 0;
    return;
  }

  // Remover espaços do fim
  while (end > start && isspace((unsigned char)*end))
    end--;

  *(end + 1) = 0;

  // Mover a string se necessário
  if (start > str) {
    while (*start)
      *str++ = *start++;
    *str = 0;
  }
}

/**
 * @brief Carrega a configuração do frontend de um arquivo
 */
bool emu_frontend_config_load(const char *config_file,
                              emu_frontend_config_t *config) {
  // Se config for NULL, usa a global
  if (!config) {
    config = &g_config;
  }

  if (!config_file) {
    EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND,
                  "Parâmetro inválido para carregamento de configuração");
    return false;
  }

  FILE *file = fopen(config_file, "r");
  if (!file) {
    EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND,
                  "Não foi possível abrir o arquivo de configuração: %s",
                  config_file);
    return false;
  }

  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Carregando configuração do arquivo: %s",
               config_file);

  char line[CONFIG_LINE_MAX];
  char key[CONFIG_LINE_MAX];
  char value[CONFIG_LINE_MAX];
  bool success = true;

  while (fgets(line, sizeof(line), file)) {
    // Ignorar comentários e linhas vazias
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
      continue;
    }

    // Remover espaços e quebras de linha
    trim(line);
    if (strlen(line) == 0) {
      continue;
    }

    // Separar chave e valor
    if (sscanf(line, "%[^=]=%s", key, value) != 2) {
      EMU_LOG_WARN(EMU_LOG_CAT_FRONTEND, "Linha de configuração inválida: %s",
                   line);
      continue;
    }

    trim(key);
    trim(value);

    // Processar configurações
    if (strcmp(key, "window_width") == 0) {
      config->window_width = atoi(value);
    } else if (strcmp(key, "window_height") == 0) {
      config->window_height = atoi(value);
    } else if (strcmp(key, "game_width") == 0) {
      config->game_width = atoi(value);
    } else if (strcmp(key, "game_height") == 0) {
      config->game_height = atoi(value);
    } else if (strcmp(key, "fullscreen") == 0) {
      config->fullscreen = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "vsync") == 0) {
      config->vsync_enabled = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "scale_factor") == 0) {
      config->scale_factor = atof(value);
    } else if (strcmp(key, "smooth_scaling") == 0) {
      config->smooth_scaling = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "integer_scaling") == 0) {
      config->integer_scaling = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "audio_enabled") == 0) {
      config->audio_enabled = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "audio_sample_rate") == 0) {
      config->audio_sample_rate = atoi(value);
    } else if (strcmp(key, "audio_buffer_size") == 0) {
      config->audio_buffer_size = atoi(value);
    } else if (strcmp(key, "keyboard_enabled") == 0) {
      config->keyboard_enabled = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "gamepad_enabled") == 0) {
      config->gamepad_enabled = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "show_fps") == 0) {
      config->show_fps = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "debug_overlay") == 0) {
      config->debug_overlay = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "theme_id") == 0) {
      config->theme_id = atoi(value);
    } else {
      // Tenta processar opções específicas do frontend
      if (!emu_frontend_config_process_option(key, value, config)) {
        EMU_LOG_WARN(EMU_LOG_CAT_FRONTEND,
                     "Opção de configuração desconhecida: %s=%s", key, value);
      }
    }
  }

  fclose(file);
  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Configuração carregada com sucesso");
  return success;
}

/**
 * @brief Salva a configuração do frontend em um arquivo
 */
bool emu_frontend_config_save(const char *config_file,
                              const emu_frontend_config_t *config) {
  // Se config for NULL, usa a global
  if (!config) {
    config = &g_config;
  }

  if (!config_file) {
    EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND,
                  "Parâmetro inválido para salvamento de configuração");
    return false;
  }

  FILE *file = fopen(config_file, "w");
  if (!file) {
    EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND,
                  "Não foi possível criar o arquivo de configuração: %s",
                  config_file);
    return false;
  }

  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Salvando configuração no arquivo: %s",
               config_file);

  // Escrever cabeçalho
  fprintf(file, CONFIG_FILE_HEADER);

  // Seção Vídeo
  fprintf(file, "# Configurações de Vídeo\n");
  fprintf(file, "window_width=%d\n", config->window_width);
  fprintf(file, "window_height=%d\n", config->window_height);
  fprintf(file, "game_width=%d\n", config->game_width);
  fprintf(file, "game_height=%d\n", config->game_height);
  fprintf(file, "scale_factor=%.2f\n", config->scale_factor);
  fprintf(file, "vsync=%s\n", config->vsync_enabled ? "true" : "false");
  fprintf(file, "fullscreen=%s\n", config->fullscreen ? "true" : "false");
  fprintf(file, "smooth_scaling=%s\n",
          config->smooth_scaling ? "true" : "false");
  fprintf(file, "integer_scaling=%s\n",
          config->integer_scaling ? "true" : "false");
  fprintf(file, "\n");

  // Seção Áudio
  fprintf(file, "# Configurações de Áudio\n");
  fprintf(file, "audio_enabled=%s\n", config->audio_enabled ? "true" : "false");
  fprintf(file, "audio_sample_rate=%d\n", config->audio_sample_rate);
  fprintf(file, "audio_buffer_size=%d\n", config->audio_buffer_size);
  fprintf(file, "\n");

  // Seção Entrada
  fprintf(file, "# Configurações de Entrada\n");
  fprintf(file, "keyboard_enabled=%s\n",
          config->keyboard_enabled ? "true" : "false");
  fprintf(file, "gamepad_enabled=%s\n",
          config->gamepad_enabled ? "true" : "false");
  fprintf(file, "\n");

  // Seção Interface
  fprintf(file, "# Configurações de Interface\n");
  fprintf(file, "show_fps=%s\n", config->show_fps ? "true" : "false");
  fprintf(file, "debug_overlay=%s\n", config->debug_overlay ? "true" : "false");
  fprintf(file, "theme_id=%d\n", config->theme_id);
  fprintf(file, "\n");

  // Escrever opções específicas do frontend
  fprintf(file, "# Configurações Específicas do Frontend\n");
  emu_frontend_config_write_specific_options(file, config);
  fprintf(file, "\n");

  fclose(file);
  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Configuração salva com sucesso");
  return true;
}

/**
 * @brief Implementação padrão para processamento de opções específicas
 *
 * Implementações específicas do frontend devem sobrescrever esta função
 * para processar suas opções proprietárias.
 */
bool emu_frontend_config_process_option(const char *key, const char *value,
                                        emu_frontend_config_t *config) {
  // Implementação padrão não processa nenhuma opção específica
  (void)key;
  (void)value;
  (void)config;
  return false;
}

/**
 * @brief Implementação padrão para escrita de opções específicas
 *
 * Implementações específicas do frontend devem sobrescrever esta função
 * para salvar suas opções proprietárias.
 */
bool emu_frontend_config_write_specific_options(
    FILE *file, const emu_frontend_config_t *config) {
  // Implementação padrão não escreve nenhuma opção específica
  (void)file;
  (void)config;
  return true;
}
