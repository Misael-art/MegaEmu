/**
 * @file mapper9.h
 * @brief Definições para o Mapper 9 (MMC2/PxROM) do Nintendo Entertainment System
 *
 * O Mapper 9 (MMC2/PxROM) possui as seguintes características:
 * - Suporta PRG-ROM com 4 bancos de 8KB (2 bancos fixos, 2 bancos selecionáveis)
 * - Suporta CHR-ROM com mecanismo de latch especial que muda bancos durante renderização
 * - 2 registradores para cada padrão de tiles (0x0000 e 0x1000)
 * - Espelhamento selecionável (vertical/horizontal)
 * - Utilizado principalmente no jogo Punch-Out!!
 */

#ifndef NES_MAPPER9_H
#define NES_MAPPER9_H

#include "cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 9
 *
 * @param cartridge Ponteiro para o cartucho NES
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_9_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER9_H */
