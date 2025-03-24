/**
 * @file mapper6.h
 * @brief Definição do Mapper 6 (FFE F4xxx) para o Nintendo Entertainment System
 *
 * Características do Mapper 6:
 * - PRG-ROM: Até 128KB (bancos de 16KB configuráveis)
 * - CHR-ROM/RAM: Até 32KB (bancos de 8KB)
 * - Usado por jogos da Front Fareast (FFE)
 * - Suporte para PRG-RAM com opção para proteger escrita
 * - Controle de espelhamento vertical/horizontal
 */

#ifndef NES_MAPPER6_H
#define NES_MAPPER6_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 6 (FFE F4xxx)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_6_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER6_H */
