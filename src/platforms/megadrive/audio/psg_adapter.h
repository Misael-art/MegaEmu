/**
 * @file psg_adapter.h
 * @brief Interface do adaptador PSG (SN76489) para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef PSG_ADAPTER_H
#define PSG_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

// Definições do PSG
#define PSG_CHANNELS 4    // 3 canais de tom + 1 canal de ruído
#define PSG_CLOCK 3579545 // Clock padrão do PSG (3.58 MHz)

// Estrutura do canal PSG
typedef struct {
  uint16_t frequency; // Frequência do tom (10 bits)
  uint8_t volume;     // Volume (4 bits, 0=máximo, 15=mudo)
  uint16_t counter;   // Contador para geração de tom
  bool output;        // Estado atual da onda quadrada
} psg_channel_t;

// Estrutura do canal de ruído
typedef struct {
  uint8_t mode;       // Modo de ruído (periódico/branco)
  uint8_t shift_rate; // Taxa de deslocamento
  uint8_t volume;     // Volume (4 bits, 0=máximo, 15=mudo)
  uint16_t counter;   // Contador para geração de ruído
  uint16_t shift_reg; // Registrador de deslocamento
} psg_noise_t;

// Estrutura de contexto do PSG
typedef struct {
  psg_channel_t channels[3];  // Canais de tom
  psg_noise_t noise;          // Canal de ruído
  uint32_t clock_rate;        // Taxa de clock do PSG
  uint32_t sample_rate;       // Taxa de amostragem
  uint32_t cycles_per_sample; // Ciclos por amostra
  uint8_t latch;              // Registrador latch
  bool is_latched;            // Estado do latch
} psg_context_t;

// Funções de ciclo de vida
psg_context_t *psg_create(uint32_t clock_rate, uint32_t sample_rate);
void psg_destroy(psg_context_t *ctx);
void psg_reset(psg_context_t *ctx);

// Funções de acesso a registradores
void psg_write(psg_context_t *ctx, uint8_t value);

// Funções de geração de áudio
void psg_update(psg_context_t *ctx, int16_t *buffer, int length);
void psg_run(psg_context_t *ctx, uint32_t cycles);

#endif // PSG_ADAPTER_H
