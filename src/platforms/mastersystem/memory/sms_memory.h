/**
 * @file sms_memory.h
 * @brief Definições para o sistema de memória do Master System
 */

#ifndef SMS_MEMORY_H
#define SMS_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/save_state.h"

// Tamanho da página de memória do SMS (16KB)
#define SMS_PAGE_SIZE 0x4000

// Número padrão de páginas de ROM (máximo suportado)
#define SMS_DEFAULT_ROM_PAGES 32

// Tamanho da RAM do SMS (8KB)
#define SMS_RAM_SIZE 0x2000

/**
 * @brief Opaque handle para o sistema de memória
 */
typedef struct sms_memory_s sms_memory_t;

/**
 * @brief Cria uma nova instância do sistema de memória do Master System
 *
 * @return Ponteiro para a instância criada, ou NULL em caso de falha
 */
sms_memory_t *sms_memory_create(void);

/**
 * @brief Destrói uma instância do sistema de memória
 *
 * @param memory Ponteiro para a instância a ser destruída
 */
void sms_memory_destroy(sms_memory_t *memory);

/**
 * @brief Reseta o estado do sistema de memória
 *
 * @param memory Ponteiro para a instância de memória
 */
void sms_memory_reset(sms_memory_t *memory);

/**
 * @brief Lê um byte do espaço de endereçamento
 *
 * @param memory Ponteiro para a instância de memória
 * @param address Endereço a ser lido (0x0000-0xFFFF)
 * @return Valor lido do endereço especificado
 */
uint8_t sms_memory_read(sms_memory_t *memory, uint16_t address);

/**
 * @brief Escreve um byte no espaço de endereçamento
 *
 * @param memory Ponteiro para a instância de memória
 * @param address Endereço a ser escrito (0x0000-0xFFFF)
 * @param value Valor a ser escrito
 */
void sms_memory_write(sms_memory_t *memory, uint16_t address, uint8_t value);

/**
 * @brief Lê uma palavra (16 bits) do espaço de endereçamento
 *
 * @param memory Ponteiro para a instância de memória
 * @param address Endereço a ser lido (0x0000-0xFFFE)
 * @return Valor lido do endereço especificado
 */
uint16_t sms_memory_read_word(sms_memory_t *memory, uint16_t address);

/**
 * @brief Escreve uma palavra (16 bits) no espaço de endereçamento
 *
 * @param memory Ponteiro para a instância de memória
 * @param address Endereço a ser escrito (0x0000-0xFFFE)
 * @param value Valor a ser escrito
 */
void sms_memory_write_word(sms_memory_t *memory, uint16_t address, uint16_t value);

/**
 * @brief Registra o sistema de memória no sistema de save state
 *
 * @param memory Ponteiro para a instância de memória
 * @param state Ponteiro para o sistema de save state
 * @return Código de retorno (0 para sucesso)
 */
int sms_memory_register_save_state(sms_memory_t *memory, save_state_t *state);

/**
 * @brief Carrega uma ROM no sistema de memória
 *
 * @param memory Ponteiro para a instância de memória
 * @param rom_data Ponteiro para os dados da ROM
 * @param rom_size Tamanho dos dados da ROM em bytes
 * @return true se a ROM foi carregada com sucesso, false caso contrário
 */
bool sms_memory_load_rom(sms_memory_t *memory, const uint8_t *rom_data, uint32_t rom_size);

/**
 * @brief Implementa a escrita no registrador de controle de memória
 *
 * @param memory Ponteiro para a instância de memória
 * @param value Valor a ser escrito no registrador
 */
void sms_memory_control_write(sms_memory_t *memory, uint8_t value);

/**
 * @brief Implementa a escrita nos registradores do mapeador de páginas
 *
 * @param memory Ponteiro para a instância de memória
 * @param reg_index Índice do registrador do mapeador (0-3)
 * @param value Valor a ser escrito no registrador
 */
void sms_memory_mapper_write(sms_memory_t *memory, uint8_t reg_index, uint8_t value);

/**
 * @brief Atualiza o estado interno após carregar um save state
 *
 * @param memory Ponteiro para a instância de memória
 */
void sms_memory_update_state(sms_memory_t *memory);

#endif /* SMS_MEMORY_H */
