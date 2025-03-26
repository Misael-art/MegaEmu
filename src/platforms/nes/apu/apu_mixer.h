/**
 * @file apu_mixer.h
 * @brief Mixer de áudio otimizado para o APU do NES
 */

#ifndef NES_APU_MIXER_H
#define NES_APU_MIXER_H

#include <stdbool.h>
#include <stdint.h>

// Tamanho das tabelas de lookup
#define APU_PULSE_TABLE_SIZE 31 // 15 níveis por canal pulse (2 canais)
#define APU_TND_TABLE_SIZE                                                     \
  203 // 203 combinações possíveis de Triangle/Noise/DMC

// Estrutura do mixer
typedef struct {
  // Volumes dos canais
  float pulse1_volume;
  float pulse2_volume;
  float triangle_volume;
  float noise_volume;
  float dmc_volume;
  float master_volume;

  // Tabelas de lookup para mistura não-linear
  float pulse_table[APU_PULSE_TABLE_SIZE];
  float tnd_table[APU_TND_TABLE_SIZE];

  // Estado do filtro passa-baixa
  float lpf_acc;  // Acumulador do filtro
  float lpf_prev; // Valor anterior
  float lpf_beta; // Coeficiente do filtro (0.0 - 1.0)

  // Configurações
  bool high_quality_mode; // Modo de alta qualidade (mais CPU)
  bool filter_enabled;    // Filtro passa-baixa habilitado
  uint32_t sample_rate;   // Taxa de amostragem
} nes_apu_mixer_t;

// Funções de interface
void apu_mixer_init(nes_apu_mixer_t *mixer, uint32_t sample_rate);
void apu_mixer_reset(nes_apu_mixer_t *mixer);
void apu_mixer_set_volumes(nes_apu_mixer_t *mixer, float pulse1, float pulse2,
                           float triangle, float noise, float dmc,
                           float master);
void apu_mixer_enable_filter(nes_apu_mixer_t *mixer, bool enabled);
void apu_mixer_set_quality(nes_apu_mixer_t *mixer, bool high_quality);
float apu_mixer_mix(nes_apu_mixer_t *mixer, uint8_t pulse1, uint8_t pulse2,
                    uint8_t triangle, uint8_t noise, uint8_t dmc);

#endif // NES_APU_MIXER_H
