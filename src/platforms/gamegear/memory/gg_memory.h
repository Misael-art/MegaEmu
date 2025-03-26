/**
 * @file gg_memory.h
 * @brief Sistema de memória do Game Gear
 */

#ifndef GG_MEMORY_H
#define GG_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/save_state.h"

// Tamanhos de memória
#define GG_ROM_BANK_SIZE 0x4000    // 16KB por banco de ROM
#define GG_RAM_SIZE 0x2000         // 8KB de RAM
#define GG_TOTAL_BANKS 32          // Máximo de 32 bancos (512KB)

// Mapeamento de memória
#define GG_ROM_BANK0_START 0x0000  // Banco 0 fixo
#define GG_ROM_BANK0_END 0x3FFF
#define GG_ROM_BANK1_START 0x4000  // Banco 1 fixo
#define GG_ROM_BANK1_END 0x7FFF
#define GG_ROM_BANK2_START 0x8000  // Banco 2 comutável
#define GG_ROM_BANK2_END 0xBFFF
#define GG_RAM_START 0xC000        // RAM
#define GG_RAM_END 0xDFFF
#define GG_RAM_MIRROR_START 0xE000 // Espelho da RAM
#define GG_RAM_MIRROR_END 0xFFFF

// Forward declaration
typedef struct gg_memory_t gg_memory_t;

/**
 * @brief Cria uma nova instância do sistema de memória
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_memory_t *gg_memory_create(void);

/**
 * @brief Destrói uma instância do sistema de memória
 * @param mem Ponteiro para a instância
 */
void gg_memory_destroy(gg_memory_t *mem);

/**
 * @brief Reseta o sistema de memória
 * @param mem Ponteiro para a instância
 */
void gg_memory_reset(gg_memory_t *mem);

/**
 * @brief Carrega uma ROM no sistema de memória
 * @param mem Ponteiro para a instância
 * @param data Ponteiro para os dados da ROM
 * @param size Tamanho da ROM em bytes
 * @return true se sucesso, false caso contrário
 */
bool gg_memory_load_rom(gg_memory_t *mem, const uint8_t *data, size_t size);

/**
 * @brief Lê um byte da memória
 * @param mem Ponteiro para a instância
 * @param addr Endereço a ser lido
 * @return Byte lido
 */
uint8_t gg_memory_read(gg_memory_t *mem, uint16_t addr);

/**
 * @brief Escreve um byte na memória
 * @param mem Ponteiro para a instância
 * @param addr Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void gg_memory_write(gg_memory_t *mem, uint16_t addr, uint8_t value);

/**
 * @brief Obtém ponteiro para a RAM
 * @param mem Ponteiro para a instância
 * @return Ponteiro para a RAM ou NULL em caso de erro
 */
uint8_t *gg_memory_get_ram(gg_memory_t *mem);

/**
 * @brief Obtém ponteiro para um banco de ROM
 * @param mem Ponteiro para a instância
 * @param bank Número do banco
 * @return Ponteiro para o banco ou NULL em caso de erro
 */
const uint8_t *gg_memory_get_rom_bank(gg_memory_t *mem, uint8_t bank);

/**
 * @brief Registra campos do sistema de memória no sistema de save state
 * @param mem Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_memory_register_save_state(gg_memory_t *mem, save_state_t *state);

#endif // GG_MEMORY_H
