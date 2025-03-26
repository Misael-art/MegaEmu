/**
 * @file audio_adapter.h
 * @brief Adaptador de áudio para o Mega Drive (YM2612 + PSG)
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef MEGADRIVE_AUDIO_ADAPTER_H
#define MEGADRIVE_AUDIO_ADAPTER_H

#include "core/interfaces/audio_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Constantes do YM2612
 */
#define MD_YM2612_CHANNELS 6
#define MD_YM2612_OPERATORS 4
#define MD_YM2612_REGISTERS 0x200

/**
 * @brief Constantes do PSG
 */
#define MD_PSG_CHANNELS 4
#define MD_PSG_REGISTERS 8

/**
 * @brief Estrutura do operador FM
 */
typedef struct {
  uint32_t phase;        // Fase atual
  uint32_t phase_step;   // Incremento de fase
  uint32_t env_phase;    // Fase do envelope
  uint32_t env_step;     // Incremento do envelope
  uint16_t level;        // Nível atual
  uint16_t total_level;  // Nível total
  uint8_t key_state;     // Estado da tecla
  uint8_t algorithm;     // Algoritmo FM
  uint8_t feedback;      // Feedback
  uint8_t multiple;      // Multiplicador de frequência
  uint8_t detune;        // Detune
  uint8_t rate_scaling;  // Rate scaling
  uint8_t attack_rate;   // Taxa de ataque
  uint8_t decay_rate;    // Taxa de decay
  uint8_t sustain_rate;  // Taxa de sustain
  uint8_t release_rate;  // Taxa de release
  uint8_t sustain_level; // Nível de sustain
  bool ams_enabled;      // AMS habilitado
} md_ym2612_operator_t;

/**
 * @brief Estrutura do canal FM
 */
typedef struct {
  md_ym2612_operator_t operators[MD_YM2612_OPERATORS];
  uint32_t frequency; // Frequência base
  uint16_t block;     // Bloco de oitava
  uint8_t algorithm;  // Algoritmo FM
  uint8_t feedback;   // Feedback
  uint8_t ams;        // Amplitude modulation sensitivity
  uint8_t fms;        // Frequency modulation sensitivity
  uint8_t panning;    // Pan (left/right)
  bool enabled;       // Canal habilitado
} md_ym2612_channel_t;

/**
 * @brief Estrutura do canal PSG
 */
typedef struct {
  uint32_t frequency; // Frequência
  uint16_t counter;   // Contador
  uint8_t volume;     // Volume (0-15)
  uint8_t type;       // Tipo (tone/noise)
  bool enabled;       // Canal habilitado
} md_psg_channel_t;

/**
 * @brief Contexto específico do adaptador de áudio
 */
typedef struct {
  // YM2612
  md_ym2612_channel_t fm_channels[MD_YM2612_CHANNELS];
  uint8_t fm_registers[MD_YM2612_REGISTERS];
  uint32_t fm_clock;
  uint32_t fm_rate;
  bool fm_busy;
  bool fm_irq;

  // PSG
  md_psg_channel_t psg_channels[MD_PSG_CHANNELS];
  uint8_t psg_registers[MD_PSG_REGISTERS];
  uint32_t psg_clock;
  uint32_t psg_rate;
  uint16_t psg_noise_shift;
  uint8_t psg_noise_tap;
  uint8_t psg_noise_type;

  // Mixer
  int16_t *mix_buffer;
  uint32_t mix_buffer_size;
  uint32_t mix_position;
  float fm_volume;
  float psg_volume;

  // Estado
  uint32_t sample_rate;
  uint32_t samples_played;
  bool enabled;
  void *user_data;
} megadrive_audio_context_t;

/**
 * @brief Cria uma nova instância do adaptador de áudio
 * @return Ponteiro para a interface de áudio ou NULL em caso de erro
 */
emu_audio_interface_t *megadrive_audio_adapter_create(void);

/**
 * @brief Destrói uma instância do adaptador de áudio
 * @param audio Ponteiro para a interface de áudio
 */
void megadrive_audio_adapter_destroy(emu_audio_interface_t *audio);

/**
 * @brief Obtém o contexto específico do adaptador de áudio
 * @param audio Ponteiro para a interface de áudio
 * @return Ponteiro para o contexto ou NULL em caso de erro
 */
megadrive_audio_context_t *
megadrive_audio_get_context(emu_audio_interface_t *audio);

/**
 * @brief Define o contexto específico do adaptador de áudio
 * @param audio Ponteiro para a interface de áudio
 * @param context Ponteiro para o novo contexto
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int megadrive_audio_set_context(emu_audio_interface_t *audio,
                                const megadrive_audio_context_t *context);

/**
 * @brief Escreve em um registrador do YM2612
 * @param audio Ponteiro para a interface de áudio
 * @param port Porta (0 ou 1)
 * @param reg Registrador
 * @param value Valor
 */
void megadrive_audio_write_ym2612(emu_audio_interface_t *audio, uint8_t port,
                                  uint8_t reg, uint8_t value);

/**
 * @brief Escreve em um registrador do PSG
 * @param audio Ponteiro para a interface de áudio
 * @param value Valor
 */
void megadrive_audio_write_psg(emu_audio_interface_t *audio, uint8_t value);

/**
 * @brief Define o volume do YM2612
 * @param audio Ponteiro para a interface de áudio
 * @param volume Volume (0.0 a 1.0)
 */
void megadrive_audio_set_fm_volume(emu_audio_interface_t *audio, float volume);

/**
 * @brief Define o volume do PSG
 * @param audio Ponteiro para a interface de áudio
 * @param volume Volume (0.0 a 1.0)
 */
void megadrive_audio_set_psg_volume(emu_audio_interface_t *audio, float volume);

#ifdef __cplusplus
}
#endif

#endif // MEGADRIVE_AUDIO_ADAPTER_H
