/**
 * @file mapper230.h
 * @brief Definições para o Mapper 230 (22-in-1)
 *
 * O Mapper 230 é um mapper simples usado em cartuchos multicart.
 * Características:
 * - PRG-ROM: Bancos de 32KB
 * - CHR-ROM: Bancos de 8KB
 * - Sem PRG-RAM
 * - Mirroring fixo
 */

#ifndef NES_MAPPER230_H
#define NES_MAPPER230_H

#include "../nes_cartridge.h"

// Registradores do Mapper 230
#define M230_REG_BANK 0x8000

/**
 * @brief Inicializa o Mapper 230
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_230_init(nes_cartridge_t *cartridge);

#endif // NES_MAPPER230_H
