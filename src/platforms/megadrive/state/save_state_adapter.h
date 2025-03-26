/**
 * @file save_state_adapter.h
 * @brief Definições para o adaptador do sistema de save states para o Mega
 * Drive
 * @version 1.0
 * @date 2024-03-25
 */

#ifndef SAVE_STATE_ADAPTER_H
#define SAVE_STATE_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

// Tipos opacos
typedef struct megadrive_s megadrive_t;
typedef struct emu_save_state emu_save_state_t;
typedef struct emu_save_options emu_save_options_t;
typedef struct emu_encryption_config emu_encryption_config_t;
typedef struct emu_cloud_config emu_cloud_config_t;

/**
 * @brief Inicializa o sistema de save state para o Mega Drive
 * @param md Contexto do Mega Drive
 * @return Contexto de save state ou NULL em caso de erro
 */
emu_save_state_t *md_save_state_init(megadrive_t *md);

/**
 * @brief Salva o estado do Mega Drive
 * @param md Contexto do Mega Drive
 * @param filename Nome do arquivo para salvar
 * @param options Opções para salvar (NULL para usar opções padrão)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_save(megadrive_t *md, const char *filename,
                           const emu_save_options_t *options);

/**
 * @brief Carrega um estado salvo
 * @param md Contexto do Mega Drive
 * @param filename Nome do arquivo para carregar
 * @param options Opções para carregamento (NULL para usar opções padrão)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_load(megadrive_t *md, const char *filename,
                           const emu_save_options_t *options);

/**
 * @brief Configura o sistema de rewind
 * @param md Contexto do Mega Drive
 * @param frames Número máximo de frames a armazenar
 * @param interval Intervalo entre capturas (em frames)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_config_rewind(megadrive_t *md, uint32_t frames,
                                    uint32_t interval);

/**
 * @brief Captura o estado atual para o buffer de rewind
 * @param md Contexto do Mega Drive
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_rewind_capture(megadrive_t *md);

/**
 * @brief Retrocede ou avança no buffer de rewind
 * @param md Contexto do Mega Drive
 * @param steps Número de passos (negativo para retroceder, positivo para
 * avançar)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_rewind_step(megadrive_t *md, int32_t steps);

/**
 * @brief Obtém informações sobre o estado atual do buffer de rewind
 * @param md Contexto do Mega Drive
 * @param total_frames Ponteiro para receber o número total de frames
 * armazenados
 * @param current_position Ponteiro para receber a posição atual no buffer
 * @param memory_usage Ponteiro para receber o uso de memória em bytes
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_rewind_get_info(megadrive_t *md, uint32_t *total_frames,
                                      uint32_t *current_position,
                                      uint32_t *memory_usage);

/**
 * @brief Limpa todos os recursos do sistema de save state
 * @param md Contexto do Mega Drive
 */
void md_save_state_shutdown(megadrive_t *md);

/**
 * @brief Configura a criptografia para salvar estados
 * @param md Contexto do Mega Drive
 * @param config Configuração de criptografia
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_set_encryption(megadrive_t *md,
                                     const emu_encryption_config_t *config);

/**
 * @brief Configura a integração com serviços de nuvem
 * @param md Contexto do Mega Drive
 * @param config Configuração para a nuvem
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_cloud_configure(megadrive_t *md,
                                      const emu_cloud_config_t *config);

/**
 * @brief Sincroniza um save state com a nuvem
 * @param md Contexto do Mega Drive
 * @param filename Caminho do arquivo local
 * @param upload true para upload, false para download
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_cloud_sync(megadrive_t *md, const char *filename,
                                 bool upload);

#endif // SAVE_STATE_ADAPTER_H
