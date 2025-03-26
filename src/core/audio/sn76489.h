#ifndef SN76489_H
#define SN76489_H

#include <stdbool.h>
#include <stdint.h>

// Constantes do SN76489
#define SN76489_CHANNELS 4    // 3 tons + 1 ruído
#define SN76489_VOLUME_MAX 15 // Volume máximo (4 bits)
#define SN76489_SAMPLE_RATE 44100

// Estrutura do canal de tom/ruído
typedef struct {
  uint16_t freq;    // Frequência (10 bits)
  uint8_t volume;   // Volume (4 bits)
  uint16_t counter; // Contador de frequência
  bool output;      // Estado atual da onda quadrada
} sn76489_channel_t;

// Estrutura do SN76489
typedef struct {
  sn76489_channel_t tone[3]; // Canais de tom
  struct {
    uint8_t freq;     // Frequência do ruído (2 bits)
    uint8_t volume;   // Volume do ruído (4 bits)
    uint16_t shift;   // Registrador de deslocamento
    bool white_noise; // Ruído branco vs periódico
  } noise;
  uint8_t latch;           // Registrador latch
  uint32_t clock;          // Clock frequency
  int16_t *buffer;         // Buffer de saída
  int buffer_pos;          // Posição no buffer
  int buffer_size;         // Tamanho do buffer
  uint32_t sample_rate;    // Taxa de amostragem
  uint32_t sample_counter; // Contador de amostras
} sn76489_t;

// Funções de inicialização e controle
void sn76489_init(sn76489_t *psg, uint32_t clock);
void sn76489_reset(sn76489_t *psg);
void sn76489_write(sn76489_t *psg, uint8_t value);

// Funções de geração de som
void sn76489_update(sn76489_t *psg, int16_t *buffer, int length);
void sn76489_mix_output(sn76489_t *psg, int16_t *buffer, int length);

// Funções de controle de canais
void sn76489_set_tone(sn76489_t *psg, int channel, uint16_t freq);
void sn76489_set_volume(sn76489_t *psg, int channel, uint8_t volume);
void sn76489_set_noise_control(sn76489_t *psg, uint8_t value);

// Funções de debug
void sn76489_dump_registers(sn76489_t *psg);
void sn76489_get_channel_status(sn76489_t *psg, int channel, char *buffer);

#endif // SN76489_H
