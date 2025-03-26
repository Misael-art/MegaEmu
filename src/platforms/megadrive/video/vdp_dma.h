/**
 * @file vdp_dma.h
 * @brief Definições e protótipos para o sistema de DMA do VDP do Mega Drive
 */

#ifndef EMU_VDP_DMA_H
#define EMU_VDP_DMA_H

#include <stdint.h>

// Tipos de operação DMA
typedef enum {
  VDP_DMA_VRAM_FILL,      // Preencher VRAM com valor constante
  VDP_DMA_VRAM_COPY,      // Copiar de VRAM para VRAM
  VDP_DMA_MEMORY_TO_VRAM, // Transferir da memória para VRAM
  VDP_DMA_MEMORY_TO_CRAM, // Transferir da memória para CRAM
  VDP_DMA_MEMORY_TO_VSRAM // Transferir da memória para VSRAM
} md_vdp_dma_type_t;

// Estrutura para armazenar o estado do DMA
typedef struct {
  uint8_t enabled;           // DMA habilitado
  uint32_t source;           // Endereço fonte
  uint16_t length;           // Comprimento da transferência
  uint16_t dest;             // Endereço destino
  md_vdp_dma_type_t type;    // Tipo de operação
  uint8_t fill_data;         // Dado para preenchimento
  uint8_t in_progress;       // Operação em andamento
  uint32_t cycles_remaining; // Ciclos restantes
} emu_vdp_dma_state_t;

// Funções de inicialização e controle
void emu_vdp_dma_init(void);
int emu_vdp_dma_start(md_vdp_dma_type_t type, uint32_t source, uint16_t dest,
                      uint16_t length, uint8_t fill_data);
int emu_vdp_dma_update(int cycles);
int emu_vdp_dma_is_active(void);
void emu_vdp_dma_get_state(emu_vdp_dma_state_t *state);
void emu_vdp_dma_cancel(void);

// Funções de conveniência para operações específicas
int emu_vdp_dma_fill(uint16_t dest, uint8_t data, uint16_t length);
int emu_vdp_dma_copy(uint16_t source, uint16_t dest, uint16_t length);
int emu_vdp_dma_transfer(uint32_t source, uint16_t dest, uint16_t length);
int emu_vdp_dma_transfer_cram(uint32_t source, uint16_t dest, uint16_t length);
int emu_vdp_dma_transfer_vsram(uint32_t source, uint16_t dest, uint16_t length);

#endif // EMU_VDP_DMA_H
