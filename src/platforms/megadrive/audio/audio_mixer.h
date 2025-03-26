/**
 * @file audio_mixer.h
 * @brief Interface do sistema de mixing de áudio para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "psg_adapter.h"
#include "ym2612_adapter.h"
#include <stdbool.h>
#include <stdint.h>

// Definições do mixer
#define AUDIO_BUFFER_SIZE 2048
#define AUDIO_CHANNELS 2 // Estéreo

// Estrutura de configuração do mixer
typedef struct {
  uint32_t sample_rate; // Taxa de amostragem
  uint32_t buffer_size; // Tamanho do buffer em amostras
  float fm_volume;      // Volume do FM (0.0 - 1.0)
  float psg_volume;     // Volume do PSG (0.0 - 1.0)
  float master_volume;  // Volume master (0.0 - 1.0)
} audio_mixer_config_t;

// Estrutura do mixer de áudio
typedef struct {
  ym2612_context_t *fm; // Contexto do YM2612
  psg_context_t *psg;   // Contexto do PSG
  int16_t *buffer;      // Buffer de amostras
  uint32_t buffer_size; // Tamanho do buffer
  uint32_t write_pos;   // Posição de escrita no buffer
  uint32_t read_pos;    // Posição de leitura no buffer
  float fm_volume;      // Volume do FM
  float psg_volume;     // Volume do PSG
  float master_volume;  // Volume master
  bool buffer_full;     // Indica se o buffer está cheio
} audio_mixer_t;

// Funções de ciclo de vida
audio_mixer_t *audio_mixer_create(audio_mixer_config_t *config);
void audio_mixer_destroy(audio_mixer_t *mixer);
void audio_mixer_reset(audio_mixer_t *mixer);

// Funções de controle
void audio_mixer_set_fm_volume(audio_mixer_t *mixer, float volume);
void audio_mixer_set_psg_volume(audio_mixer_t *mixer, float volume);
void audio_mixer_set_master_volume(audio_mixer_t *mixer, float volume);

// Funções de processamento
void audio_mixer_process(audio_mixer_t *mixer, uint32_t samples);
void audio_mixer_read(audio_mixer_t *mixer, int16_t *buffer, uint32_t samples);
bool audio_mixer_buffer_full(audio_mixer_t *mixer);
uint32_t audio_mixer_available_samples(audio_mixer_t *mixer);

#endif // AUDIO_MIXER_H
