/**
 * @file config.h
 * @brief Interface do sistema de configuração do emulador
 */

#ifndef EMU_CONFIG_H
#define EMU_CONFIG_H

#include <stdbool.h>

/**
 * @brief Estrutura opaca de configuração
 */
typedef struct config_t config_t;

/**
 * @brief Cria uma nova instância de configuração
 * @return Ponteiro para a configuração ou NULL em caso de erro
 */
config_t *config_create(void);

/**
 * @brief Destrói uma instância de configuração
 * @param config Ponteiro para a configuração
 */
void config_destroy(config_t *config);

/**
 * @brief Carrega configurações de um arquivo JSON
 * @param config Ponteiro para a configuração
 * @param path Caminho do arquivo
 * @return true se sucesso, false caso contrário
 */
bool config_load(config_t *config, const char *path);

/**
 * @brief Salva configurações em um arquivo JSON
 * @param config Ponteiro para a configuração
 * @return true se sucesso, false caso contrário
 */
bool config_save(config_t *config);

/**
 * @brief Obtém uma configuração de vídeo
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para armazenar o valor
 * @return true se sucesso, false caso contrário
 */
bool config_get_video(config_t *config, const char *key, void *value);

/**
 * @brief Define uma configuração de vídeo
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para o valor
 * @return true se sucesso, false caso contrário
 */
bool config_set_video(config_t *config, const char *key, const void *value);

/**
 * @brief Obtém uma configuração de áudio
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para armazenar o valor
 * @return true se sucesso, false caso contrário
 */
bool config_get_audio(config_t *config, const char *key, void *value);

/**
 * @brief Define uma configuração de áudio
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para o valor
 * @return true se sucesso, false caso contrário
 */
bool config_set_audio(config_t *config, const char *key, const void *value);

/**
 * @brief Obtém uma configuração de input
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para armazenar o valor
 * @return true se sucesso, false caso contrário
 */
bool config_get_input(config_t *config, const char *key, void *value);

/**
 * @brief Define uma configuração de input
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para o valor
 * @return true se sucesso, false caso contrário
 */
bool config_set_input(config_t *config, const char *key, const void *value);

/**
 * @brief Obtém uma configuração do sistema
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para armazenar o valor
 * @return true se sucesso, false caso contrário
 */
bool config_get_system(config_t *config, const char *key, void *value);

/**
 * @brief Define uma configuração do sistema
 * @param config Ponteiro para a configuração
 * @param key Nome da configuração
 * @param value Ponteiro para o valor
 * @return true se sucesso, false caso contrário
 */
bool config_set_system(config_t *config, const char *key, const void *value);

#endif // EMU_CONFIG_H
