/**
 * @file mapper24.h
 * @brief Definições para o Mapper 24 (VRC6) do NES
 *
 * O Mapper 24 (VRC6) é usado em jogos da Konami como Akumajou Densetsu.
 * Características:
 * - PRG-ROM: Até 512KB com bancos de 8KB/16KB
 * - CHR-ROM: Até 256KB com bancos de 1KB
 * - PRG-RAM: 8KB com bateria opcional
 * - IRQ baseado em scanline
 * - Som de expansão (2 canais de pulso + 1 canal de dente de serra)
 */

#ifndef NES_MAPPER24_H
#define NES_MAPPER24_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 24 (VRC6)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_24_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER24_H */
