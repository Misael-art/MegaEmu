/**
 * @file sms_memory.h
 * @brief Definições para o sistema de memória do Master System
 */

#ifndef SMS_MEMORY_H
#define SMS_MEMORY_H

#include <stdint.h>
#include "../../../core/save_state.h"

/**
 * @brief Opaque handle para o sistema de memória
 */
typedef struct sms_memory_t sms_memory_t;

/**
 * @brief Cria uma nova instância do sistema de memória
 * 
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_memory_t* sms_memory_create(void);

/**
 * @brief Destrói uma instância do sistema de memória e libera recursos
 * 
 * @param memory Ponteiro para a instância
 */
void sms_memory_destroy(sms_memory_t *memory);

/**
 * @brief Reseta o sistema de memória para o estado inicial
 * 
 * @param memory Ponteiro para a instância
 */
void sms_memory_reset(sms_memory_t *memory);

/**
 * @brief Conecta o sistema de memória ao VDP
 * 
 * @param memory Ponteiro para a instância
 * @param vdp Ponteiro para o VDP
 */
void sms_memory_connect_vdp(sms_memory_t *memory, void *vdp);

/**
 * @brief Conecta o sistema de memória ao PSG
 * 
 * @param memory Ponteiro para a instância
 * @param psg Ponteiro para o PSG
 */
void sms_memory_connect_psg(sms_memory_t *memory, void *psg);

/**
 * @brief Conecta o sistema de memória ao sistema de entrada
 * 
 * @param memory Ponteiro para a instância
 * @param input Ponteiro para o sistema de entrada
 */
void sms_memory_connect_input(sms_memory_t *memory, void *input);

/**
 * @brief Conecta o sistema de memória ao cartucho
 * 
 * @param memory Ponteiro para a instância
 * @param cartridge Ponteiro para o cartucho
 */
void sms_memory_connect_cartridge(sms_memory_t *memory, void *cartridge);

/**
 * @brief Lê um byte da memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint8_t sms_memory_read(sms_memory_t *memory, uint16_t address);

/**
 * @brief Escreve um byte na memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_memory_write(sms_memory_t *memory, uint16_t address, uint8_t value);

/**
 * @brief Lê uma palavra (16 bits) da memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint16_t sms_memory_read_word(sms_memory_t *memory, uint16_t address);

/**
 * @brief Escreve uma palavra (16 bits) na memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_memory_write_word(sms_memory_t *memory, uint16_t address, uint16_t value);

/**
 * @brief Registra o sistema de memória no sistema de save state
 * 
 * @param memory Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_memory_register_save_state(sms_memory_t *memory, save_state_t *state);

#endif /* SMS_MEMORY_H */
