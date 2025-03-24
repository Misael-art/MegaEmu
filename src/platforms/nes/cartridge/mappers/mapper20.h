/**
 * @file mapper20.h
 * @brief Definições para o Mapper 20 (FDS - Famicom Disk System) do NES
 *
 * O Mapper 20 é usado para emular o Famicom Disk System.
 * Características:
 * - Memória de programa expansível (32KB + discos)
 * - CHR-RAM de 8KB
 * - RAM de expansão de 32KB
 * - Registradores de controle do disco
 * - IRQ baseado em timer
 * - Som de expansão (Wavetable)
 */

#ifndef NES_MAPPER20_H
#define NES_MAPPER20_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 20 (FDS)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_20_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER20_H */
