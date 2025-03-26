/**
 * @file rewind_buffer.h
 * @brief Interface para o sistema de rewind otimizado
 */

#ifndef REWIND_BUFFER_H
#define REWIND_BUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "save_state.h"

    /**
     * @brief Inicializa o buffer de rewind
     *
     * @param capacity Número máximo de snapshots
     * @param frames_per_snapshot A cada quantos frames capturar um snapshot
     * @return int32_t 0 em caso de sucesso, código de erro caso contrário
     */
    int32_t rewind_buffer_init(uint32_t capacity, uint32_t frames_per_snapshot);

    /**
     * @brief Finaliza o buffer de rewind
     */
    void rewind_buffer_shutdown(void);

    /**
     * @brief Configura o efeito visual para o rewind
     *
     * @param config Configurações do efeito (NULL para usar padrão)
     * @return int32_t 0 em caso de sucesso, código de erro caso contrário
     */
    int32_t rewind_buffer_configure_effect(const save_state_rewind_effect_config_t *config);

    /**
     * @brief Adiciona um snapshot ao buffer de rewind
     *
     * @param data Ponteiro para os dados
     * @param size Tamanho em bytes
     * @return int32_t 0 em caso de sucesso, código de erro caso contrário
     */
    int32_t rewind_buffer_add_snapshot(const void *data, uint32_t size);

    /**
     * @brief Retrocede um passo no buffer de rewind
     *
     * @param data Ponteiro para receber o ponteiro dos dados
     * @param size Ponteiro para receber o tamanho
     * @return int32_t 0 em caso de sucesso, código de erro caso contrário
     */
    int32_t rewind_buffer_step_back(void **data, uint32_t *size);

    /**
     * @brief Avança um passo no buffer de rewind (após retroceder)
     *
     * @param data Ponteiro para receber o ponteiro dos dados
     * @param size Ponteiro para receber o tamanho
     * @return int32_t 0 em caso de sucesso, código de erro caso contrário
     */
    int32_t rewind_buffer_step_forward(void **data, uint32_t *size);

    /**
     * @brief Inicia o efeito visual de rewind
     *
     * @param seconds_back Segundos para voltar (1-10)
     * @return int32_t 0 em caso de sucesso, código de erro caso contrário
     */
    int32_t rewind_buffer_start_effect(uint32_t seconds_back);

    /**
     * @brief Cancela o efeito visual de rewind
     *
     * @return int32_t 0 em caso de sucesso, código de erro caso contrário
     */
    int32_t rewind_buffer_cancel_effect(void);

    /**
     * @brief Atualiza o efeito visual de rewind
     *
     * Esta função deve ser chamada a cada frame para atualizar o efeito visual.
     * Retorna true quando o efeito termina e é necessário aplicar o snapshot.
     *
     * @param target_data Ponteiro para receber o ponteiro dos dados alvo
     * @param target_size Ponteiro para receber o tamanho dos dados alvo
     * @return bool true se o efeito terminou, false caso contrário
     */
    bool rewind_buffer_update_effect(void **target_data, uint32_t *target_size);

    /**
     * @brief Obtém o progresso atual do efeito de rewind
     *
     * @return float Progresso de 0.0 a 1.0
     */
    float rewind_buffer_get_effect_progress(void);

    /**
     * @brief Verificar se o efeito de rewind está ativo
     *
     * @return bool true se o efeito está ativo, false caso contrário
     */
    bool rewind_buffer_is_effect_active(void);

    /**
     * @brief Obter estatísticas do buffer de rewind
     *
     * @param capacity Ponteiro para receber a capacidade
     * @param count Ponteiro para receber o número de snapshots válidos
     * @param frames_per_snapshot Ponteiro para receber frames por snapshot
     */
    void rewind_buffer_get_stats(uint32_t *capacity, uint32_t *count, uint32_t *frames_per_snapshot);

    /**
     * @brief Limpa todo o buffer de rewind
     */
    void rewind_buffer_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* REWIND_BUFFER_H */
