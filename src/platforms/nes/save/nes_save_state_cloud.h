/**
 * @file nes_save_state_cloud.h
 * @brief Interface para integração com serviços de nuvem para save states do NES
 * @version 1.0
 * @date 2025-04-30
 *
 * Este arquivo define as funções para habilitar a integração com serviços
 * de nuvem para save states do NES, permitindo sincronização automática,
 * backup e compartilhamento.
 */

#ifndef NES_SAVE_STATE_CLOUD_H
#define NES_SAVE_STATE_CLOUD_H

#include <stdbool.h>
#include "../../../core/save_state.h"
#include "../../../core/save_state_cloud.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configura a integração com serviços de nuvem para os save states do NES
 *
 * Esta função configura a integração com o serviço de nuvem especificado
 * para o sistema de save states do NES.
 *
 * @param state Contexto do save state
 * @param provider Provedor de nuvem (EMU_CLOUD_*)
 * @param auth_token Token de autenticação para o serviço
 * @param auto_sync Habilita sincronização automática
 * @return true se a configuração foi bem-sucedida, false caso contrário
 */
bool nes_save_state_enable_cloud(emu_save_state_t *state,
                                emu_cloud_provider_t provider,
                                const char *auth_token,
                                bool auto_sync);

/**
 * @brief Desativa a integração com nuvem para os save states do NES
 *
 * @param state Contexto do save state
 * @return true se a desativação foi bem-sucedida, false caso contrário
 */
bool nes_save_state_disable_cloud(emu_save_state_t *state);

/**
 * @brief Sincroniza manualmente o save state com a nuvem
 *
 * @param state Contexto do save state
 * @return true se a sincronização foi bem-sucedida, false caso contrário
 */
bool nes_save_state_sync_with_cloud(emu_save_state_t *state);

/**
 * @brief Obtém a lista de save states disponíveis na nuvem
 *
 * @param state Contexto do save state
 * @param count Ponteiro para retornar o número de save states encontrados
 * @return Array de estruturas de metadados dos save states, NULL em caso de erro
 */
emu_cloud_file_info_t* nes_save_state_list_cloud_saves(emu_save_state_t *state, uint32_t *count);

/**
 * @brief Baixa um save state específico da nuvem
 *
 * @param state Contexto do save state
 * @param cloud_id ID do save state na nuvem
 * @param local_path Caminho local para salvar o arquivo
 * @return true se o download foi bem-sucedido, false caso contrário
 */
bool nes_save_state_download_from_cloud(emu_save_state_t *state,
                                      const char *cloud_id,
                                      const char *local_path);

/**
 * @brief Faz upload de um save state para a nuvem
 *
 * @param state Contexto do save state
 * @param local_path Caminho local do arquivo
 * @param description Descrição do save state
 * @return true se o upload foi bem-sucedido, false caso contrário
 */
bool nes_save_state_upload_to_cloud(emu_save_state_t *state,
                                 const char *local_path,
                                 const char *description);

/**
 * @brief Configura a estratégia de resolução de conflitos
 *
 * @param state Contexto do save state
 * @param strategy Estratégia a ser utilizada (EMU_CLOUD_CONFLICT_*)
 * @return true se a configuração foi bem-sucedida, false caso contrário
 */
bool nes_save_state_set_conflict_strategy(emu_save_state_t *state, emu_cloud_conflict_strategy_t strategy);

/**
 * @brief Habilita ou desabilita backup automático na nuvem
 *
 * @param state Contexto do save state
 * @param enable Flag para habilitar (true) ou desabilitar (false)
 * @param interval_minutes Intervalo em minutos entre backups (0 para usar padrão)
 * @return true se a configuração foi bem-sucedida, false caso contrário
 */
bool nes_save_state_configure_auto_backup(emu_save_state_t *state, bool enable, uint32_t interval_minutes);

#ifdef __cplusplus
}
#endif

#endif /* NES_SAVE_STATE_CLOUD_H */
