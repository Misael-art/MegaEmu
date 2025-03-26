/**
 * @file gg_mapper.h
 * @brief Interface do sistema de mapeamento de memória do Game Gear
 */

#ifndef GG_MAPPER_H
#define GG_MAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../utils/save_state.h"

// Forward declarations
typedef struct gg_cartridge_t gg_cartridge_t;
typedef struct gg_mapper_t gg_mapper_t;

// Tipos de mapeadores suportados
typedef enum {
    GG_MAPPER_NONE,     // ROM sem mapeador
    GG_MAPPER_SEGA,     // Mapeador padrão Sega
    GG_MAPPER_CODEMASTERS // Mapeador Codemasters
} gg_mapper_type_t;

/**
 * @brief Cria uma nova instância do sistema de mapeamento
 * @param cart Ponteiro para o cartucho
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_mapper_t *gg_mapper_create(gg_cartridge_t *cart);

/**
 * @brief Destrói uma instância do sistema de mapeamento
 * @param mapper Ponteiro para a instância
 */
void gg_mapper_destroy(gg_mapper_t *mapper);

/**
 * @brief Reseta o sistema de mapeamento
 * @param mapper Ponteiro para a instância
 */
void gg_mapper_reset(gg_mapper_t *mapper);

/**
 * @brief Lê um byte da memória mapeada
 * @param mapper Ponteiro para a instância
 * @param addr Endereço a ser lido
 * @return Byte lido
 */
uint8_t gg_mapper_read(gg_mapper_t *mapper, uint16_t addr);

/**
 * @brief Escreve um byte na memória mapeada
 * @param mapper Ponteiro para a instância
 * @param addr Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void gg_mapper_write(gg_mapper_t *mapper, uint16_t addr, uint8_t value);

/**
 * @brief Obtém o tipo de mapeador em uso
 * @param mapper Ponteiro para a instância
 * @return Tipo do mapeador
 */
gg_mapper_type_t gg_mapper_get_type(const gg_mapper_t *mapper);

/**
 * @brief Registra campos do mapeador no sistema de save state
 * @param mapper Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_mapper_register_save_state(gg_mapper_t *mapper, save_state_t *state);

#endif // GG_MAPPER_H
