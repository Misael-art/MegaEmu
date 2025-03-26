/**
 * @file frontend_config.c
 * @brief Implementação específica para SDL das funções de configuração do
 * frontend
 *
 * NOTA: Esta implementação estende a implementação genérica
 * em src/frontend/common/frontend_config.c com funcionalidades
 * específicas do SDL.
 */

#include "frontend_config.h"
#include "../common/frontend_config.h"
#include "utils/enhanced_log.h"
#include "utils/log_categories.h"
#include "utils/log_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura para configurações específicas do SDL
typedef struct {
  // Adicionamos aqui configurações específicas do SDL
  int sdl_render_driver;   // Identificador do driver de renderização (0 = auto)
  bool sdl_hardware_accel; // Usar aceleração de hardware
  int sdl_audio_device;    // Índice do dispositivo de áudio (-1 = padrão)
  int sdl_joystick_index;  // Índice do joystick a usar (-1 = automatico)
  char sdl_shader_path[256]; // Caminho para arquivo de shader personalizado
} sdl_specific_config_t;

// Configuração global específica do SDL
static sdl_specific_config_t g_sdl_specific = {.sdl_render_driver = 0,
                                               .sdl_hardware_accel = true,
                                               .sdl_audio_device = -1,
                                               .sdl_joystick_index = -1,
                                               .sdl_shader_path = ""};

/**
 * @brief Inicializa as configurações específicas do SDL
 */
void sdl_frontend_config_init(void) {
  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND,
               "Inicializando configuração do frontend SDL");

  // Inicializa a configuração comum
  emu_frontend_config_init();

  // Obtém a configuração global
  emu_frontend_config_t *config = emu_frontend_config_get();

  // Configura dados específicos do SDL
  config->frontend_specific = &g_sdl_specific;

  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND,
               "Configuração do frontend SDL inicializada: width=%d, "
               "height=%d, theme=%d",
               config->window_width, config->window_height, config->theme_id);
}

/**
 * @brief Obtém a configuração atual do SDL
 *
 * @return emu_frontend_config_t* Ponteiro para a configuração
 */
emu_frontend_config_t *sdl_frontend_get_config(void) {
  return emu_frontend_config_get();
}

/**
 * @brief Processa opções específicas do SDL durante o carregamento
 */
bool sdl_process_specific_option(const char *key, const char *value,
                                 emu_frontend_config_t *config) {
  // Primeiro verifica se temos um ponteiro válido para a configuração
  // específica do SDL
  sdl_specific_config_t *sdl_config =
      (sdl_specific_config_t *)config->frontend_specific;
  if (!sdl_config) {
    return false;
  }

  // Processa opções específicas do SDL
  if (strcmp(key, "sdl_render_driver") == 0) {
    sdl_config->sdl_render_driver = atoi(value);
    return true;
  } else if (strcmp(key, "sdl_hardware_accel") == 0) {
    sdl_config->sdl_hardware_accel = strcmp(value, "true") == 0;
    return true;
  } else if (strcmp(key, "sdl_audio_device") == 0) {
    sdl_config->sdl_audio_device = atoi(value);
    return true;
  } else if (strcmp(key, "sdl_joystick_index") == 0) {
    sdl_config->sdl_joystick_index = atoi(value);
    return true;
  } else if (strcmp(key, "sdl_shader_path") == 0) {
    strncpy(sdl_config->sdl_shader_path, value,
            sizeof(sdl_config->sdl_shader_path) - 1);
    sdl_config->sdl_shader_path[sizeof(sdl_config->sdl_shader_path) - 1] = '\0';
    return true;
  }

  // Opção não reconhecida
  return false;
}

/**
 * @brief Escreve opções específicas do SDL ao salvar a configuração
 */
bool sdl_write_specific_options(FILE *file,
                                const emu_frontend_config_t *config) {
  // Primeiro verifica se temos um ponteiro válido para a configuração
  // específica do SDL
  const sdl_specific_config_t *sdl_config =
      (const sdl_specific_config_t *)config->frontend_specific;
  if (!sdl_config || !file) {
    return false;
  }

  // Escreve configurações específicas do SDL
  fprintf(file, "# Configurações específicas do SDL\n");
  fprintf(file, "sdl_render_driver=%d\n", sdl_config->sdl_render_driver);
  fprintf(file, "sdl_hardware_accel=%s\n",
          sdl_config->sdl_hardware_accel ? "true" : "false");
  fprintf(file, "sdl_audio_device=%d\n", sdl_config->sdl_audio_device);
  fprintf(file, "sdl_joystick_index=%d\n", sdl_config->sdl_joystick_index);

  if (strlen(sdl_config->sdl_shader_path) > 0) {
    fprintf(file, "sdl_shader_path=%s\n", sdl_config->sdl_shader_path);
  }

  return true;
}

/**
 * @brief Sobrescreve as funções de processamento de opções específicas
 */
bool emu_frontend_config_process_option(const char *key, const char *value,
                                        emu_frontend_config_t *config) {
  return sdl_process_specific_option(key, value, config);
}

/**
 * @brief Sobrescreve as funções de escrita de opções específicas
 */
bool emu_frontend_config_write_specific_options(
    FILE *file, const emu_frontend_config_t *config) {
  return sdl_write_specific_options(file, config);
}

/**
 * @brief Salva a configuração específica do SDL
 *
 * @return bool true se a configuração foi salva com sucesso, false caso
 * contrário
 */
bool sdl_frontend_save_config(void) {
  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Salvando configuração específica do SDL");

  // Usa a função comum para salvar a configuração
  return emu_frontend_config_save("./config/sdl_config.ini",
                                  emu_frontend_get_config());
}

/**
 * @brief Carrega a configuração específica do SDL
 *
 * @return bool true se a configuração foi carregada com sucesso, false caso
 * contrário
 */
bool sdl_frontend_load_config(void) {
  EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND,
               "Carregando configuração específica do SDL");

  // Garante que temos as configurações específicas do SDL inicializadas
  emu_frontend_config_t *config = emu_frontend_get_config();
  if (!config->frontend_specific) {
    config->frontend_specific = &g_sdl_specific;
  }

  // Usa a função comum para carregar a configuração
  bool success = emu_frontend_config_load("./config/sdl_config.ini", config);

  if (success) {
    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND,
                 "Configuração do SDL carregada: width=%d, height=%d, theme=%d",
                 config->window_width, config->window_height, config->theme_id);
  } else {
    EMU_LOG_WARN(EMU_LOG_CAT_FRONTEND,
                 "Falha ao carregar configuração. Usando valores padrão.");
  }

  return success;
}

// Funções específicas do SDL para configuração
// ... Adicionar conforme necessário
