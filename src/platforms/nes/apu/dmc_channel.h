/**
 * @file dmc_channel.h
 * @brief Implementação do canal DMC (Delta Modulation Channel) do APU do NES
 */

#ifndef NES_DMC_CHANNEL_H
#define NES_DMC_CHANNEL_H

#include <stdbool.h>
#include <stdint.h>

// Constantes do DMC
#define DMC_BUFFER_SIZE 4096
#define DMC_RATE_TABLE_SIZE 16

// Estrutura do canal DMC
typedef struct {
  // Registradores
  bool irq_enable;        // IRQ habilitado
  bool loop_flag;         // Flag de loop
  uint8_t rate_index;     // Índice na tabela de taxas
  uint8_t direct_load;    // Valor de load direto
  uint16_t sample_addr;   // Endereço da amostra
  uint16_t sample_length; // Comprimento da amostra em bytes

  // Estado do timer
  uint16_t timer_period;  // Período do timer
  uint16_t timer_counter; // Contador do timer

  // Estado da amostra
  uint16_t current_addr;    // Endereço atual
  uint16_t bytes_remaining; // Bytes restantes
  uint8_t sample_buffer;    // Buffer da amostra
  bool sample_buffer_empty; // Estado do buffer
  uint8_t shift_register;   // Registrador de deslocamento
  uint8_t bits_remaining;   // Bits restantes no shift register
  uint8_t output_level;     // Nível de saída atual (0-127)

  // Estado de DMA
  bool dma_pending;   // DMA pendente
  uint16_t dma_addr;  // Endereço para DMA
  uint8_t dma_buffer; // Buffer de DMA

  // Flags de estado
  bool enabled;      // Canal habilitado
  bool irq_flag;     // Flag de IRQ
  bool silence_flag; // Flag de silêncio
} nes_dmc_channel_t;

// Funções de interface
void dmc_init(nes_dmc_channel_t *dmc);
void dmc_reset(nes_dmc_channel_t *dmc);
void dmc_write_register(nes_dmc_channel_t *dmc, uint16_t addr, uint8_t value);
uint8_t dmc_read_status(nes_dmc_channel_t *dmc);
void dmc_clock(nes_dmc_channel_t *dmc);
int16_t dmc_output(nes_dmc_channel_t *dmc);
bool dmc_irq_pending(nes_dmc_channel_t *dmc);
void dmc_acknowledge_irq(nes_dmc_channel_t *dmc);
bool dmc_dma_needed(nes_dmc_channel_t *dmc);
void dmc_dma_complete(nes_dmc_channel_t *dmc, uint8_t data);

#endif // NES_DMC_CHANNEL_H
