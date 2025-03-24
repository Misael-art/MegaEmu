/**
 * @file mapper85.h
 * @brief Definições para o Mapper 85 (VRC7)
 *
 * O Mapper 85 (VRC7) é um mapper avançado da Konami com suporte a som.
 * Características:
 * - PRG-ROM: Até 512KB com bancos de 8KB/16KB
 * - CHR-ROM: Até 256KB com bancos de 1KB/2KB/4KB/8KB
 * - PRG-RAM: 8KB com bateria opcional
 * - Mirroring controlado por registrador
 * - IRQ baseado em scanline
 * - Chip de som YM2413 (6 canais FM)
 */

#ifndef NES_MAPPER_85_H
#define NES_MAPPER_85_H

#include "platforms/nes/cartridge/nes_cartridge.h"

// Registradores do VRC7
#define VRC7_PRG_SEL_8K_0 0x8000   // Seleção do primeiro banco PRG de 8KB
#define VRC7_PRG_SEL_8K_1 0x8010   // Seleção do segundo banco PRG de 8KB
#define VRC7_PRG_SEL_8K_2 0x9000   // Seleção do terceiro banco PRG de 8KB
#define VRC7_CHR_SEL_1K_0 0xA000   // Seleção do primeiro banco CHR de 1KB
#define VRC7_CHR_SEL_1K_1 0xA010   // Seleção do segundo banco CHR de 1KB
#define VRC7_CHR_SEL_1K_2 0xB000   // Seleção do terceiro banco CHR de 1KB
#define VRC7_CHR_SEL_1K_3 0xB010   // Seleção do quarto banco CHR de 1KB
#define VRC7_CHR_SEL_1K_4 0xC000   // Seleção do quinto banco CHR de 1KB
#define VRC7_CHR_SEL_1K_5 0xC010   // Seleção do sexto banco CHR de 1KB
#define VRC7_CHR_SEL_1K_6 0xD000   // Seleção do sétimo banco CHR de 1KB
#define VRC7_CHR_SEL_1K_7 0xD010   // Seleção do oitavo banco CHR de 1KB
#define VRC7_IRQ_LATCH 0xE000      // Registrador de latch do IRQ
#define VRC7_IRQ_CONTROL 0xE010    // Registrador de controle do IRQ
#define VRC7_IRQ_ACK 0xF000        // Registrador de reconhecimento do IRQ
#define VRC7_SOUND_REG_ADDR 0x9010 // Registrador de endereço do som
#define VRC7_SOUND_REG_DATA 0x9030 // Registrador de dados do som

// Bits de controle do IRQ
#define VRC7_IRQ_ENABLE 0x02 // Habilita o IRQ
#define VRC7_IRQ_MODE 0x04   // Modo de contagem do IRQ

/**
 * @brief Inicializa o Mapper 85 (VRC7)
 *
 * O VRC7 é um mapper complexo que adiciona um chip de som FM ao NES.
 * Características:
 * - Suporta até 512KB de PRG-ROM
 * - Suporta até 256KB de CHR-ROM
 * - 8KB de PRG-RAM com bateria opcional
 * - IRQ baseado em scanline
 * - Chip de som FM YM2413 (OPLL)
 *
 * @param cartridge Ponteiro para a estrutura do cartucho
 * @return Ponteiro para a estrutura do mapper ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_85_init(nes_cartridge_t *cartridge);

#endif // NES_MAPPER_85_H
