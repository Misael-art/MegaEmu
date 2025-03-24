/**
 * @file mapper10.h
 * @brief Declarações para o Mapper 10 (MMC4/FxROM) do NES
 */

#ifndef _NES_MAPPER10_H
#define _NES_MAPPER10_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 10 (MMC4/FxROM)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_10_init(nes_cartridge_t *cartridge);

#endif /* _NES_MAPPER10_H */
