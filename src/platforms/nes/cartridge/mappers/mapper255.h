/**
 * @file mapper255.h
 * @brief Definições para o Mapper 255 (110-in-1)
 *
 * O Mapper 255 é um mapper usado em cartuchos multicart.
 * Características:
 * - PRG-ROM: Bancos de 32KB
 * - CHR-ROM: Bancos de 8KB
 * - PRG-RAM: 8KB opcional
 * - Mirroring controlado por registrador
 * - Suporte a proteção de escrita
 */

#ifndef NES_MAPPER255_H
#define NES_MAPPER255_H

#include "../nes_cartridge.h"

// Registradores do Mapper 255
#define M255_REG_BANK 0x8000
#define M255_REG_PROTECT 0x8001

// Bits de controle
#define M255_PROTECT_ON 0x80
#define M255_MIRROR_VERT 0x01

/**
 * @brief Inicializa o Mapper 255
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_255_init(nes_cartridge_t *cartridge);

#endif // NES_MAPPER255_H
