#include <stdint.h>
#include <stdbool.h>

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

// Inicializa a PPU
void ppu_init(PPU* ppu) {
    ppu->control = 0;
    ppu->mask = 0;
    ppu->status = 0;
    ppu->vram_addr = 0;

    // Aloca memória para VRAM e OAM
    ppu->vram = (uint8_t*)malloc(0x4000); // 16KB VRAM
    ppu->oam = (uint8_t*)malloc(256);      // 256 bytes OAM
    ppu->framebuffer = (uint32_t*)malloc(256 * 240 * sizeof(uint32_t));

    // Limpa as memórias
    memset(ppu->vram, 0, 0x4000);
    memset(ppu->oam, 0, 256);
    memset(ppu->framebuffer, 0, 256 * 240 * sizeof(uint32_t));
}

// Libera recursos da PPU
void ppu_destroy(PPU* ppu) {
    free(ppu->vram);
    free(ppu->oam);
    free(ppu->framebuffer);
}

// Executa um ciclo da PPU
void ppu_step(PPU* ppu) {
    // TODO: Implementar lógica de renderização
}

// Reset da PPU
void ppu_reset(PPU* ppu) {
    ppu->control = 0;
    ppu->mask = 0;
    ppu->status = 0;
    ppu->vram_addr = 0;

    // Limpa as memórias
    memset(ppu->vram, 0, 0x4000);
    memset(ppu->oam, 0, 256);
    memset(ppu->framebuffer, 0, 256 * 240 * sizeof(uint32_t));
}
