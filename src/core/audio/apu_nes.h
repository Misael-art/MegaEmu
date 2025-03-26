#ifndef APU_NES_H
#define APU_NES_H

#include <stdbool.h>
#include <stdint.h>

// Constantes do APU
#define APU_SAMPLE_RATE 44100
#define APU_FRAME_COUNTER_RATE 240 // Hz

// Estrutura do canal de pulso (2)
typedef struct {
  uint8_t duty;           // Duty cycle (2 bits)
  uint8_t volume;         // Volume (4 bits)
  bool constant_volume;   // Volume constante vs envelope
  bool halt;              // Halt length counter/envelope
  uint16_t timer;         // Timer (11 bits)
  uint8_t length_counter; // Length counter (7 bits)
  struct {
    uint8_t period;  // Período (4 bits)
    uint8_t counter; // Contador
    uint8_t volume;  // Volume atual
    bool start;      // Flag de início
  } envelope;
  struct {
    uint8_t shift;   // Quantidade de shift (3 bits)
    bool negate;     // Flag de negação
    uint8_t period;  // Período (3 bits)
    uint8_t counter; // Contador
    bool enabled;    // Sweep habilitado
    bool reload;     // Flag de reload
  } sweep;
  bool enabled;   // Canal habilitado
  uint8_t output; // Saída atual
} apu_pulse_t;

// Estrutura do canal de triângulo
typedef struct {
  uint8_t linear_counter; // Linear counter
  uint8_t length_counter; // Length counter
  uint16_t timer;         // Timer (11 bits)
  bool control;           // Control flag
  bool halt;              // Halt flag
  bool enabled;           // Canal habilitado
  uint8_t output;         // Saída atual
  uint8_t step;           // Step atual da sequência
} apu_triangle_t;

// Estrutura do canal de ruído
typedef struct {
  uint8_t volume;         // Volume (4 bits)
  bool constant_volume;   // Volume constante vs envelope
  uint16_t timer;         // Timer
  uint8_t length_counter; // Length counter
  bool mode;              // Mode flag
  struct {
    uint8_t period;  // Período (4 bits)
    uint8_t counter; // Contador
    uint8_t volume;  // Volume atual
    bool start;      // Flag de início
  } envelope;
  bool enabled;   // Canal habilitado
  uint16_t shift; // Shift register
  uint8_t output; // Saída atual
} apu_noise_t;

// Estrutura do canal DMC
typedef struct {
  uint8_t freq;           // Frequência
  bool loop;              // Loop flag
  bool irq_enable;        // IRQ enable
  uint16_t sample_addr;   // Sample address
  uint16_t sample_length; // Sample length
  uint8_t current_addr;   // Current address
  uint8_t current_length; // Current length
  uint8_t output;         // Saída atual
  bool enabled;           // Canal habilitado
  bool irq_pending;       // IRQ pending
} apu_dmc_t;

// Estrutura do APU
typedef struct {
  apu_pulse_t pulse[2];    // Canais de pulso
  apu_triangle_t triangle; // Canal de triângulo
  apu_noise_t noise;       // Canal de ruído
  apu_dmc_t dmc;           // Canal DMC
  uint8_t frame_counter;   // Frame counter
  bool frame_irq_enable;   // Frame IRQ enable
  bool frame_irq_pending;  // Frame IRQ pending
  bool step_mode;          // 4-step vs 5-step mode
  uint32_t clock;          // Clock frequency
  int16_t *buffer;         // Buffer de saída
  int buffer_pos;          // Posição no buffer
  int buffer_size;         // Tamanho do buffer
} apu_nes_t;

// Funções de inicialização e controle
void apu_nes_init(apu_nes_t *apu, uint32_t clock);
void apu_nes_reset(apu_nes_t *apu);
void apu_nes_write_reg(apu_nes_t *apu, uint16_t addr, uint8_t value);
uint8_t apu_nes_read_status(apu_nes_t *apu);

// Funções de geração de som
void apu_nes_run(apu_nes_t *apu, int cycles);
void apu_nes_end_frame(apu_nes_t *apu);
void apu_nes_mix_output(apu_nes_t *apu, int16_t *buffer, int length);

// Funções de controle de frame
void apu_nes_clock_frame_counter(apu_nes_t *apu);
void apu_nes_clock_length_counters(apu_nes_t *apu);
void apu_nes_clock_envelopes(apu_nes_t *apu);
void apu_nes_clock_sweep_units(apu_nes_t *apu);

// Funções de debug
void apu_nes_dump_registers(apu_nes_t *apu);
void apu_nes_get_channel_status(apu_nes_t *apu, int channel, char *buffer);

#endif // APU_NES_H
