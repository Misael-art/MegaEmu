#ifndef YM2612_H
#define YM2612_H

#include <stdbool.h>
#include <stdint.h>

// Constantes do YM2612
#define YM2612_CHANNELS 6
#define YM2612_OPERATORS 4
#define YM2612_SAMPLE_RATE 44100

// Estrutura do operador FM
typedef struct {
  uint8_t dt;     // Detune
  uint8_t mul;    // Frequency multiplier
  uint8_t tl;     // Total level
  uint8_t ks;     // Key scale
  uint8_t ar;     // Attack rate
  uint8_t am;     // Amplitude modulation
  uint8_t dr;     // Decay rate
  uint8_t sr;     // Sustain rate
  uint8_t sl;     // Sustain level
  uint8_t rr;     // Release rate
  uint8_t ssg_eg; // SSG-EG
} ym2612_operator_t;

// Estrutura do canal FM
typedef struct {
  ym2612_operator_t op[4]; // Operadores
  uint16_t freq;           // Frequência
  uint8_t algorithm;       // Algoritmo
  uint8_t feedback;        // Feedback
  uint8_t ams;             // Amplitude modulation sensitivity
  uint8_t fms;             // Frequency modulation sensitivity
  uint8_t pms;             // Phase modulation sensitivity
  bool key_on;             // Key on/off
} ym2612_channel_t;

// Estrutura do YM2612
typedef struct {
  ym2612_channel_t channels[YM2612_CHANNELS];
  uint8_t lfo_enable;    // LFO enable
  uint8_t lfo_freq;      // LFO frequency
  uint8_t timer_a;       // Timer A
  uint8_t timer_b;       // Timer B
  bool timer_a_enable;   // Timer A enable
  bool timer_b_enable;   // Timer B enable
  bool timer_a_overflow; // Timer A overflow
  bool timer_b_overflow; // Timer B overflow
  uint32_t clock;        // Clock frequency
  int16_t *buffer;       // Buffer de saída
  int buffer_pos;        // Posição no buffer
  int buffer_size;       // Tamanho do buffer
} ym2612_t;

// Funções de inicialização e controle
void ym2612_init(ym2612_t *ym, uint32_t clock);
void ym2612_reset(ym2612_t *ym);
void ym2612_write_reg(ym2612_t *ym, uint8_t port, uint8_t reg, uint8_t value);
uint8_t ym2612_read_status(ym2612_t *ym);

// Funções de geração de som
void ym2612_update(ym2612_t *ym, int16_t *buffer, int length);
void ym2612_mix_output(ym2612_t *ym, int16_t *buffer, int length);

// Funções de controle de timer
void ym2612_timer_a_load(ym2612_t *ym, uint16_t value);
void ym2612_timer_b_load(ym2612_t *ym, uint8_t value);
void ym2612_timer_update(ym2612_t *ym);

// Funções de debug
void ym2612_dump_registers(ym2612_t *ym);
void ym2612_get_channel_status(ym2612_t *ym, int channel, char *buffer);

#endif // YM2612_H
