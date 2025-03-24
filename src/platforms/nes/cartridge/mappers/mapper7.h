/**
 * @file mapper7.h
 * @brief Definições para o Mapper 7 (AxROM) do Nintendo Entertainment System
 *
 * O Mapper 7 (AxROM) possui as seguintes características:
 * - Suporta PRG-ROM com bancos de 32KB
 * - Usa apenas CHR-RAM (normalmente 8KB)
 * - Suporta espelhamento de nametable único (single-screen) selecionável
 */

#ifndef NES_MAPPER7_H
#define NES_MAPPER7_H

#include "cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 7
 *
 * @param cartridge Ponteiro para o cartucho NES
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_7_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER7_H */
