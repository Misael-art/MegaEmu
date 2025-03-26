/**
 * @file ym2612_adapter.h
 * @brief Interface do adaptador YM2612 (FM) para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef YM2612_ADAPTER_H
#define YM2612_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

// Definições do YM2612
#define YM2612_CHANNELS 6
#define YM2612_OPERATORS 4
#define YM2612_REGISTERS 0x200

// Estrutura do operador FM
typedef struct {
  uint32_t multiple;      // Multiplicador de frequência
  uint32_t detune;        // Detune
  uint32_t total_level;   // Nível total (volume)
  uint32_t rate_scale;    // Rate scaling
  uint32_t attack_rate;   // Taxa de ataque
  uint32_t decay_rate;    // Taxa de decay
  uint32_t sustain_rate;  // Taxa de sustain
  uint32_t release_rate;  // Taxa de release
  uint32_t sustain_level; // Nível de sustain
  bool am_enable;         // Habilita modulação de amplitude
} ym2612_operator_t;

// Estrutura do canal FM
typedef struct {
  ym2612_operator_t operators[YM2612_OPERATORS];
  uint32_t algorithm; // Algoritmo de síntese (0-7)
  uint32_t feedback;  // Valor de feedback
  uint32_t frequency; // Frequência base
  uint32_t block;     // Bloco de oitava
  bool left_enable;   // Habilita saída esquerda
  bool right_enable;  // Habilita saída direita
  bool key_on;        // Estado da tecla (key on/off)
} ym2612_channel_t;

// Estrutura de contexto do YM2612
typedef struct {
  ym2612_channel_t channels[YM2612_CHANNELS];
  uint8_t registers[YM2612_REGISTERS];
  uint32_t clock_rate;
  uint32_t sample_rate;
  uint32_t cycles_per_sample;
  bool lfo_enable;
  uint32_t lfo_frequency;
  uint32_t timer_a_period;
  uint32_t timer_b_period;
  bool timer_a_enable;
  bool timer_b_enable;
  bool timer_a_overflow;
  bool timer_b_overflow;
  void (*timer_a_callback)(void *);
  void (*timer_b_callback)(void *);
  void *timer_callback_data;
} ym2612_context_t;

// Funções de ciclo de vida
ym2612_context_t *ym2612_create(uint32_t clock_rate, uint32_t sample_rate);
void ym2612_destroy(ym2612_context_t *ctx);
void ym2612_reset(ym2612_context_t *ctx);

// Funções de acesso a registradores
void ym2612_write_reg(ym2612_context_t *ctx, uint16_t reg, uint8_t value);
uint8_t ym2612_read_reg(ym2612_context_t *ctx, uint16_t reg);

// Funções de geração de áudio
void ym2612_update(ym2612_context_t *ctx, int16_t *buffer, int length);
void ym2612_run(ym2612_context_t *ctx, uint32_t cycles);

// Funções de timer
void ym2612_set_timer_a_callback(ym2612_context_t *ctx,
                                 void (*callback)(void *), void *data);
void ym2612_set_timer_b_callback(ym2612_context_t *ctx,
                                 void (*callback)(void *), void *data);
void ym2612_timer_tick(ym2612_context_t *ctx);

#endif // YM2612_ADAPTER_H
