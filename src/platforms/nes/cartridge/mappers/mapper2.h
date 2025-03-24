/**
 * @file mapper2.h
 * @brief Definições para o Mapper 2 (UxROM) do NES
 *
 * O Mapper 2 (UxROM) é usado por jogos como Mega Man, Castlevania, Contra, Metal Gear
 * e possui as seguintes características:
 * - PRG-ROM: Até 512KB (32 bancos de 16KB)
 * - Último banco fixo em $C000-$FFFF
 * - Banco selecionável em $8000-$BFFF
 * - Não possui CHR-ROM, usa CHR-RAM (8KB)
 * - Não possui PRG-RAM com bateria
 *
 * Este é um mapper relativamente simples, com apenas uma chave de registro para
 * selecionar o banco de PRG-ROM na região $8000-$BFFF.
 */

#ifndef NES_MAPPER2_H
#define NES_MAPPER2_H

#include "../nes_cartridge.h"

/**
 * @brief Inicializa o Mapper 2 (UxROM)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para a estrutura do mapper ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_2_init(nes_cartridge_t *cartridge);

#endif /* NES_MAPPER2_H */
