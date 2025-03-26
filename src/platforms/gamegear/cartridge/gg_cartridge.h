/**
 * @file gg_cartridge.h
 * @brief Interface do sistema de cartucho do Game Gear
 */

#ifndef GG_CARTRIDGE_H
#define GG_CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../../utils/save_state.h"

// Forward declaration
typedef struct gg_cartridge_t gg_cartridge_t;

// Tamanho máximo do cabeçalho do cartucho
#define GG_CARTRIDGE_HEADER_SIZE 0x10

// Estrutura do cabeçalho do cartucho
typedef struct {
    char magic[8];           // "TMR SEGA" para cartuchos Game Gear
    uint8_t reserved[2];     // Bytes reservados
    uint8_t checksum[2];     // Checksum do cartucho
    uint8_t product_code[3]; // Código do produto
    uint8_t version;         // Versão do cartucho
} gg_cartridge_header_t;

/**
 * @brief Cria uma nova instância do sistema de cartucho
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_cartridge_t *gg_cartridge_create(void);

/**
 * @brief Destrói uma instância do sistema de cartucho
 * @param cart Ponteiro para a instância
 */
void gg_cartridge_destroy(gg_cartridge_t *cart);

/**
 * @brief Carrega uma ROM no cartucho
 * @param cart Ponteiro para a instância
 * @param data Ponteiro para os dados da ROM
 * @param size Tamanho dos dados em bytes
 * @return true se sucesso, false caso contrário
 */
bool gg_cartridge_load_rom(gg_cartridge_t *cart, const uint8_t *data, size_t size);

/**
 * @brief Obtém o cabeçalho do cartucho
 * @param cart Ponteiro para a instância
 * @return Ponteiro para o cabeçalho ou NULL se não houver ROM carregada
 */
const gg_cartridge_header_t *gg_cartridge_get_header(const gg_cartridge_t *cart);

/**
 * @brief Obtém o tamanho da ROM em bytes
 * @param cart Ponteiro para a instância
 * @return Tamanho da ROM ou 0 se não houver ROM carregada
 */
size_t gg_cartridge_get_rom_size(const gg_cartridge_t *cart);

/**
 * @brief Obtém um ponteiro para os dados da ROM
 * @param cart Ponteiro para a instância
 * @return Ponteiro para os dados da ROM ou NULL se não houver ROM carregada
 */
const uint8_t *gg_cartridge_get_rom_data(const gg_cartridge_t *cart);

/**
 * @brief Verifica se o cartucho tem uma ROM carregada
 * @param cart Ponteiro para a instância
 * @return true se há ROM carregada, false caso contrário
 */
bool gg_cartridge_has_rom(const gg_cartridge_t *cart);

/**
 * @brief Verifica se o cartucho tem suporte a SRAM
 * @param cart Ponteiro para a instância
 * @return true se tem suporte a SRAM, false caso contrário
 */
bool gg_cartridge_has_sram(const gg_cartridge_t *cart);

/**
 * @brief Obtém o tamanho da SRAM em bytes
 * @param cart Ponteiro para a instância
 * @return Tamanho da SRAM ou 0 se não houver suporte
 */
size_t gg_cartridge_get_sram_size(const gg_cartridge_t *cart);

/**
 * @brief Carrega dados na SRAM do cartucho
 * @param cart Ponteiro para a instância
 * @param data Ponteiro para os dados
 * @param size Tamanho dos dados em bytes
 * @return true se sucesso, false caso contrário
 */
bool gg_cartridge_load_sram(gg_cartridge_t *cart, const uint8_t *data, size_t size);

/**
 * @brief Obtém um ponteiro para os dados da SRAM
 * @param cart Ponteiro para a instância
 * @return Ponteiro para os dados da SRAM ou NULL se não houver suporte
 */
const uint8_t *gg_cartridge_get_sram_data(const gg_cartridge_t *cart);

/**
 * @brief Registra campos do cartucho no sistema de save state
 * @param cart Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_cartridge_register_save_state(gg_cartridge_t *cart, save_state_t *state);

#endif // GG_CARTRIDGE_H
