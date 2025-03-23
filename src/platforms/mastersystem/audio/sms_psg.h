/**
 * @file sms_psg.h
 * @brief Definições para o Programmable Sound Generator (PSG) do Master System
 */

#ifndef SMS_PSG_H
#define SMS_PSG_H

#include <stdint.h>
#include "../../../core/save_state.h"

/**
 * @brief Opaque handle para o PSG (Programmable Sound Generator)
 */
typedef struct sms_psg_t sms_psg_t;

/**
 * @brief Cria uma nova instância do PSG
 *
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_psg_t *sms_psg_create(void);

/**
 * @brief Destrói uma instância do PSG e libera recursos
 *
 * @param psg Ponteiro para a instância
 */
void sms_psg_destroy(sms_psg_t *psg);

/**
 * @brief Reseta o PSG para o estado inicial
 *
 * @param psg Ponteiro para a instância
 */
void sms_psg_reset(sms_psg_t *psg);

/**
 * @brief Conecta o PSG à CPU
 *
 * @param psg Ponteiro para a instância
 * @param cpu Ponteiro para a CPU
 */
void sms_psg_connect_cpu(sms_psg_t *psg, void *cpu);

/**
 * @brief Inicia um novo frame de áudio no PSG
 *
 * @param psg Ponteiro para a instância
 * @param audio_buffer Buffer para receber os dados de áudio
 * @param buffer_size Tamanho do buffer de áudio em amostras
 */
void sms_psg_start_frame(sms_psg_t *psg, int16_t *audio_buffer, int32_t buffer_size);

/**
 * @brief Atualiza o estado do PSG com base nos ciclos executados
 *
 * @param psg Ponteiro para a instância
 * @param cycles Número de ciclos executados
 */
void sms_psg_update(sms_psg_t *psg, uint8_t cycles);

/**
 * @brief Finaliza o frame atual e gera amostras de áudio
 *
 * @param psg Ponteiro para a instância
 */
void sms_psg_end_frame(sms_psg_t *psg);

/**
 * @brief Escreve no registrador do PSG
 *
 * @param psg Ponteiro para a instância
 * @param value Valor a ser escrito (comando ou dados)
 */
void sms_psg_write_port(sms_psg_t *psg, uint8_t value);

/**
 * @brief Lê o estado atual do PSG
 *
 * @param psg Ponteiro para a instância
 * @return Valor lido
 */
uint8_t sms_psg_read(sms_psg_t *psg);

/**
 * @brief Atualiza o estado interno do PSG após um carregamento de estado
 *
 * @param psg Ponteiro para a instância
 */
void sms_psg_update_after_state_load(sms_psg_t *psg);

/**
 * @brief Obtém amostras de áudio geradas pelo PSG
 *
 * @param psg Ponteiro para a instância
 * @param buffer Buffer para receber as amostras (16 bits signed PCM)
 * @param num_samples Número de amostras a serem geradas
 * @return Número de amostras efetivamente geradas
 */
uint32_t sms_psg_get_samples(sms_psg_t *psg, int16_t *buffer, uint32_t num_samples);

/**
 * @brief Registra o estado do PSG no sistema de save state
 *
 * @param psg Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_psg_register_save_state(sms_psg_t *psg, save_state_t *state);

#endif /* SMS_PSG_H */
