/**
 * @file mapper75.h
 * @brief Definições para o Mapper 75 (VRC1)
 *
 * O Mapper 75 (VRC1) é um mapper da Konami usado em alguns jogos.
 * Características:
 * - PRG-ROM: Até 512KB com bancos de 8KB/16KB
 * - CHR-ROM: Até 128KB com bancos de 2KB/4KB
 * - PRG-RAM: 8KB opcional
 * - Mirroring controlado por registrador
 */

#ifndef NES_MAPPER75_H
#define NES_MAPPER75_H

#include "../nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 75 (VRC1)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_75_init(nes_cartridge_t *cartridge);

#endif // NES_MAPPER75_H
