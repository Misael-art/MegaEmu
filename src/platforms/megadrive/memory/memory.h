/**
 * @file memory.h
 * @brief Interface do sistema de memória do Mega Drive
 */

#ifndef __MEGADRIVE_MEMORY_H__
#define __MEGADRIVE_MEMORY_H__

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/common/error_codes.h"

/**
 * @brief Inicializa o sistema de memória do Mega Drive
 * @return Código de erro (EMU_ERROR_NONE se sucesso)
 */
int md_memory_init(void);

/**
 * @brief Desliga o sistema de memória do Mega Drive
 */
void md_memory_shutdown(void);

/**
 * @brief Lê um byte da memória
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint8_t md_memory_read_8(uint32_t address);

/**
 * @brief Lê uma word da memória
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint16_t md_memory_read_16(uint32_t address);

/**
 * @brief Lê uma long word da memória
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint32_t md_memory_read_32(uint32_t address);

/**
 * @brief Escreve um byte na memória
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void md_memory_write_8(uint32_t address, uint8_t value);

/**
 * @brief Escreve uma word na memória
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void md_memory_write_16(uint32_t address, uint16_t value);

/**
 * @brief Escreve uma long word na memória
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void md_memory_write_32(uint32_t address, uint32_t value);

/**
 * @brief Carrega uma ROM no sistema de memória
 * @param rom_data Ponteiro para os dados da ROM
 * @param rom_size Tamanho dos dados da ROM
 * @return true se sucesso, false caso contrário
 */
bool md_memory_load_rom(const uint8_t *rom_data, uint32_t rom_size);

/**
 * @brief Salva os dados de SRAM do jogo em um arquivo
 * @param filename Nome do arquivo para salvar
 * @return true se sucesso, false caso contrário
 */
bool md_memory_save_sram(const char *filename);

/**
 * @brief Carrega os dados de SRAM do jogo de um arquivo
 * @param filename Nome do arquivo para carregar
 * @return true se sucesso, false caso contrário
 */
bool md_memory_load_sram(const char *filename);

#endif /* __MEGADRIVE_MEMORY_H__ */
