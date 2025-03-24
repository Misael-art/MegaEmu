/**
 * @file mapper3.h
 * @brief Definições para o Mapper 3 (CNROM) do Nintendo Entertainment System
 *
 * O Mapper 3 (CNROM) é usado em jogos como Adventure Island, Arkanoid, Bump'n'Jump
 * e possui as seguintes características:
 * - PRG-ROM: 16KB ou 32KB (fixos, sem troca de bancos)
 * - CHR-ROM: Até 32KB (4 bancos de 8KB)
 * - Chaveamento simples de bancos de CHR-ROM
 * - Não possui PRG-RAM com bateria
 * - Suporta espelhamento vertical e horizontal (determinado pelo cabeçalho do arquivo)
 *
 * Este é um mapper relativamente simples, sem chaveamento de PRG-ROM, apenas
 * permite a seleção de bancos de CHR-ROM através de escritas em $8000-$FFFF.
 */

#ifndef NES_MAPPER3_H
#define NES_MAPPER3_H

#include "platforms/nes/cartridge/nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 3 (CNROM)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_3_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER3_H */
