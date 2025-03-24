/**
 * @file mapper4.h
 * @brief Definição do Mapper 4 (MMC3) para o Nintendo Entertainment System
 *
 * Características do Mapper 4 (MMC3):
 * - PRG-ROM: Até 512KB (bancos de 8KB configuráveis)
 * - CHR-ROM/RAM: Até 256KB (bancos de 1KB e 2KB configuráveis)
 * - IRQ baseado em scanline da PPU
 * - Controle de espelhamento vertical/horizontal
 * - Suporte para PRG-RAM com bateria
 */

#ifndef NES_MAPPER4_H
#define NES_MAPPER4_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 4 (MMC3)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_4_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER4_H */
