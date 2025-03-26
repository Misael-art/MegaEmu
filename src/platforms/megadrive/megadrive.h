/**
 * @file megadrive.h
 * @brief Interface da plataforma Mega Drive/Genesis
 */

#ifndef MEGADRIVE_H
#define MEGADRIVE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../../core/cpu/cpu.h"
#include "../../core/video/ppu.h"

// Estrutura da plataforma Mega Drive
typedef struct {
    // Componentes principais
    CPU* cpu;
    PPU* ppu;

    // Memórias
    uint8_t* rom;      // Cartucho ROM
    uint8_t* ram;      // RAM principal
    uint8_t* vram;     // Video RAM
    uint8_t* z80_ram;  // Z80 RAM

    // Estado
    bool running;
    uint32_t cycles;
} MegaDrive;

// Funções da plataforma
bool megadrive_init(MegaDrive* md);
bool megadrive_load_rom(MegaDrive* md, const char* filename);
void megadrive_run_frame(MegaDrive* md);
void megadrive_reset(MegaDrive* md);
void megadrive_destroy(MegaDrive* md);

#endif  // MEGADRIVE_H
