/**
 * @file mapper5.h
 * @brief Definições para o Mapper 5 (MMC5) do Nintendo Entertainment System
 *
 * O Mapper 5 (MMC5) é um dos mappers mais complexos do NES, usado em jogos como
 * Castlevania III. Características principais:
 * - PRG-ROM: Até 1MB com bancos de 8KB/16KB/32KB
 * - CHR-ROM: Até 1MB com bancos de 1KB/2KB/4KB/8KB
 * - PRG-RAM: Até 64KB com bateria
 * - ExRAM: 1KB de RAM adicional
 * - Multiplicador de hardware 8x8
 * - Geração de IRQ por scanline
 * - Modos de espelhamento avançados
 * - Suporte a split screen
 */

#ifndef NES_MAPPER5_H
#define NES_MAPPER5_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 5 (MMC5)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_5_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER5_H */
