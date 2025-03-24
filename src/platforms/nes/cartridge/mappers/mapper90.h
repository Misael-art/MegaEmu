/**
 * @file mapper90.h
 * @brief Definições para o Mapper 90 (JY Company)
 *
 * O Mapper 90 é um mapper complexo usado pela JY Company.
 * Características:
 * - PRG-ROM: Até 512KB com bancos de 8KB
 * - CHR-ROM: Até 256KB com bancos de 1KB
 * - PRG-RAM: 8KB com bateria opcional
 * - Mirroring controlado por registrador
 * - IRQ baseado em scanline ou CPU clock
 * - Multiplicador de hardware 8x8
 * - Registradores de proteção
 */

#ifndef NES_MAPPER90_H
#define NES_MAPPER90_H

#include "../nes_cartridge.h"

// Registradores do Mapper 90
#define M90_REG_PRG_MODE 0x6000
#define M90_REG_CHR_MODE 0x6001
#define M90_REG_MIRROR 0x6002
#define M90_REG_IRQ_LATCH 0x6003
#define M90_REG_IRQ_ENABLE 0x6004
#define M90_REG_IRQ_MODE 0x6005
#define M90_REG_MULT_A 0x6006
#define M90_REG_MULT_B 0x6007
#define M90_REG_PROTECT 0x6008

// Bits de controle
#define M90_IRQ_ENABLE 0x01
#define M90_IRQ_MODE_CPU 0x00
#define M90_IRQ_MODE_PPU 0x01
#define M90_PROTECT_ENABLE 0x01

/**
 * @brief Inicializa o Mapper 90 (JY)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_90_init(nes_cartridge_t *cartridge);

#endif // NES_MAPPER90_H
