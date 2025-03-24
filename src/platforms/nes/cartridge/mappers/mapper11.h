/**
 * @file mapper11.h
 * @brief Definições para o Mapper 11 (Color Dreams) do NES
 *
 * O Mapper 11 é um mapper simples usado pela Color Dreams e AGCI.
 * Características:
 * - PRG-ROM: Banco único de 32KB
 * - CHR-ROM: Banco único de 8KB selecionável
 * - Sem PRG-RAM
 * - Sem bateria
 */

#ifndef NES_MAPPER11_H
#define NES_MAPPER11_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 11 (Color Dreams)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_11_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER11_H */
