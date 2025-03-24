/**
 * @file mapper8.h
 * @brief Declarações para o Mapper 8 (FFE F3xxx) do NES
 */

#ifndef _NES_MAPPER8_H
#define _NES_MAPPER8_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 8 (FFE F3xxx)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_8_init(nes_cartridge_t *cartridge);

#endif /* _NES_MAPPER8_H */
