/**
 * @file save_state.h
 * @brief Interface para o sistema de save state
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-23
 */

#ifndef MEGA_EMU_SAVE_STATE_H
#define MEGA_EMU_SAVE_STATE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Códigos de retorno para operações de save state
 */
typedef enum
{
    SAVE_STATE_OK = 0,    /**< Operação concluída com sucesso */
    SAVE_STATE_ERROR = -1 /**< Erro durante a operação */
} save_state_result_t;

/**
 * @brief Tipo opaco para o contexto de save state
 */
typedef struct save_state_s save_state_t;

/**
 * @brief Cria um novo contexto de save state
 *
 * @return Ponteiro para o contexto ou NULL em caso de erro
 */
save_state_t *save_state_create(void);

/**
 * @brief Destrói um contexto de save state
 *
 * @param state Ponteiro para o contexto a ser destruído
 */
void save_state_destroy(save_state_t *state);

/**
 * @brief Registra uma região de memória para ser salva/carregada
 *
 * @param state Contexto de save state
 * @param name Nome da região (usado para identificação)
 * @param memory Ponteiro para a região de memória
 * @param size Tamanho da região em bytes
 * @return SAVE_STATE_OK em caso de sucesso ou SAVE_STATE_ERROR em caso de erro
 */
save_state_result_t save_state_register_memory(save_state_t *state, const char *name, void *memory, uint32_t size);

/**
 * @brief Salva o estado atual em um arquivo
 *
 * @param state Contexto de save state
 * @param filename Nome do arquivo para salvar
 * @return SAVE_STATE_OK em caso de sucesso ou SAVE_STATE_ERROR em caso de erro
 */
save_state_result_t save_state_save(save_state_t *state, const char *filename);

/**
 * @brief Carrega um estado de um arquivo
 *
 * @param state Contexto de save state
 * @param filename Nome do arquivo para carregar
 * @return SAVE_STATE_OK em caso de sucesso ou SAVE_STATE_ERROR em caso de erro
 */
save_state_result_t save_state_load(save_state_t *state, const char *filename);

/**
 * @brief Adiciona metadados ao save state
 *
 * @param state Contexto de save state
 * @param key Chave do metadado
 * @param value Valor do metadado
 * @return SAVE_STATE_OK em caso de sucesso ou SAVE_STATE_ERROR em caso de erro
 */
save_state_result_t save_state_set_metadata(save_state_t *state, const char *key, const char *value);

/**
 * @brief Obtém metadados do save state
 *
 * @param state Contexto de save state
 * @param key Chave do metadado
 * @param value Buffer para armazenar o valor (pode ser NULL para verificar existência)
 * @param max_len Tamanho máximo do buffer
 * @return SAVE_STATE_OK em caso de sucesso ou SAVE_STATE_ERROR em caso de erro
 */
save_state_result_t save_state_get_metadata(save_state_t *state, const char *key, char *value, uint32_t max_len);

#endif /* MEGA_EMU_SAVE_STATE_H */
