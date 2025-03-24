/**
 * @file mapper71.h
 * @brief Definições para o Mapper 71 (Camerica)
 */

#ifndef NES_MAPPER71_H
#define NES_MAPPER71_H

#include "../nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 71
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_71_init(nes_cartridge_t *cartridge);

#endif // NES_MAPPER71_H
