/**
 * @file mapper26.h
 * @brief Definições para o Mapper 26 (VRC6a) do NES
 *
 * O Mapper 26 (VRC6a) é uma variante do VRC6 usado em jogos da Konami.
 * Características:
 * - PRG-ROM: Até 512KB com bancos de 8KB/16KB
 * - CHR-ROM: Até 256KB com bancos de 1KB
 * - PRG-RAM: 8KB com bateria opcional
 * - IRQ baseado em scanline
 * - Som de expansão (2 canais de pulso + 1 canal de dente de serra)
 * - Variante do VRC6 com pinagem A0/A1 trocada
 */

#ifndef NES_MAPPER26_H
#define NES_MAPPER26_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 26 (VRC6a)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_26_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER26_H */
