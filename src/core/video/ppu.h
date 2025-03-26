#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// Estrutura básica da PPU
typedef struct {
    uint8_t control;     // Registrador de controle
    uint8_t mask;        // Registrador de máscara
    uint8_t status;      // Registrador de status
    uint16_t vram_addr;  // Endereço atual da VRAM
    uint8_t* vram;       // Memória de vídeo
    uint8_t* oam;        // Object Attribute Memory
    uint32_t* framebuffer; // Buffer de frame
} PPU;

// Funções da PPU
void ppu_init(PPU* ppu);
void ppu_destroy(PPU* ppu);
void ppu_step(PPU* ppu);
void ppu_reset(PPU* ppu);

#endif // PPU_H
