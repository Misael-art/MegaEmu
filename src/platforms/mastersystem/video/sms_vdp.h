/**
 * @file sms_vdp.h
 * @brief Definições para o Video Display Processor (VDP) do Master System
 */

#ifndef SMS_VDP_H
#define SMS_VDP_H

#include <stdint.h>
#include "../../../core/save_state.h"

/**
 * @brief Dimensões da tela do Master System
 */
#define SMS_SCREEN_WIDTH 256
#define SMS_SCREEN_HEIGHT 192
#define SMS_GG_SCREEN_WIDTH 160
#define SMS_GG_SCREEN_HEIGHT 144

/**
 * @brief Opaque handle para o VDP
 */
typedef struct sms_vdp_t sms_vdp_t;

/**
 * @brief Cria uma nova instância do VDP
 * 
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_vdp_t* sms_vdp_create(void);

/**
 * @brief Destrói uma instância do VDP e libera recursos
 * 
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_destroy(sms_vdp_t *vdp);

/**
 * @brief Reseta o VDP para o estado inicial
 * 
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_reset(sms_vdp_t *vdp);

/**
 * @brief Conecta o VDP à CPU
 * 
 * @param vdp Ponteiro para a instância
 * @param cpu Ponteiro para a CPU
 */
void sms_vdp_connect_cpu(sms_vdp_t *vdp, void *cpu);

/**
 * @brief Inicia um novo frame no VDP
 * 
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_start_frame(sms_vdp_t *vdp);

/**
 * @brief Atualiza o estado do VDP com base nos ciclos executados
 * 
 * @param vdp Ponteiro para a instância
 * @param cycles Número de ciclos executados
 */
void sms_vdp_update(sms_vdp_t *vdp, uint8_t cycles);

/**
 * @brief Finaliza o frame atual e renderiza para o buffer
 * 
 * @param vdp Ponteiro para a instância
 * @param frame_buffer Buffer para receber os dados do frame (32 bits RGBA)
 */
void sms_vdp_end_frame(sms_vdp_t *vdp, uint32_t *frame_buffer);

/**
 * @brief Verifica se o VDP está gerando uma interrupção
 * 
 * @param vdp Ponteiro para a instância
 * @return 1 se há interrupção, 0 caso contrário
 */
uint8_t sms_vdp_check_interrupt(sms_vdp_t *vdp);

/**
 * @brief Lê um byte do VDP no endereço especificado
 * 
 * @param vdp Ponteiro para a instância
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint8_t sms_vdp_read(sms_vdp_t *vdp, uint16_t address);

/**
 * @brief Escreve um byte no VDP no endereço especificado
 * 
 * @param vdp Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_vdp_write(sms_vdp_t *vdp, uint16_t address, uint8_t value);

/**
 * @brief Atualiza o estado interno do VDP após um carregamento de estado
 * 
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_update_after_state_load(sms_vdp_t *vdp);

/**
 * @brief Registra o VDP no sistema de save state
 * 
 * @param vdp Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_vdp_register_save_state(sms_vdp_t *vdp, save_state_t *state);

#endif /* SMS_VDP_H */
