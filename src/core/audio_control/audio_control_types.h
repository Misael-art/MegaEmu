/**
 * @file audio_control_types.h
 * @brief Definições de tipos para controle de canais de áudio
 */

#ifndef MEGA_EMU_AUDIO_CONTROL_TYPES_H
#define MEGA_EMU_AUDIO_CONTROL_TYPES_H

#include "../global_defines.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tipos de canais de áudio suportados
 */
typedef enum {
  // Canais comuns para múltiplas plataformas
  AUDIO_CHANNEL_MASTER = 0, ///< Canal master (todos os canais)

  // Canais específicos do Mega Drive
  AUDIO_CHANNEL_YM2612_FM1, ///< Canal FM 1 (YM2612)
  AUDIO_CHANNEL_YM2612_FM2, ///< Canal FM 2 (YM2612)
  AUDIO_CHANNEL_YM2612_FM3, ///< Canal FM 3 (YM2612)
  AUDIO_CHANNEL_YM2612_FM4, ///< Canal FM 4 (YM2612)
  AUDIO_CHANNEL_YM2612_FM5, ///< Canal FM 5 (YM2612)
  AUDIO_CHANNEL_YM2612_FM6, ///< Canal FM 6 (YM2612)
  AUDIO_CHANNEL_PSG1,       ///< Canal PSG 1 (SN76489)
  AUDIO_CHANNEL_PSG2,       ///< Canal PSG 2 (SN76489)
  AUDIO_CHANNEL_PSG3,       ///< Canal PSG 3 (SN76489)
  AUDIO_CHANNEL_PSG_NOISE,  ///< Canal de ruído do PSG (SN76489)

  // Canais específicos do NES
  AUDIO_CHANNEL_NES_PULSE1,   ///< Canal Pulse 1 (APU)
  AUDIO_CHANNEL_NES_PULSE2,   ///< Canal Pulse 2 (APU)
  AUDIO_CHANNEL_NES_TRIANGLE, ///< Canal Triangle (APU)
  AUDIO_CHANNEL_NES_NOISE,    ///< Canal Noise (APU)
  AUDIO_CHANNEL_NES_DMC,      ///< Canal DMC (APU)

  // Canais específicos do SNES
  AUDIO_CHANNEL_SNES_VOICE1, ///< Canal Voice 1 (SPC700)
  AUDIO_CHANNEL_SNES_VOICE2, ///< Canal Voice 2 (SPC700)
  AUDIO_CHANNEL_SNES_VOICE3, ///< Canal Voice 3 (SPC700)
  AUDIO_CHANNEL_SNES_VOICE4, ///< Canal Voice 4 (SPC700)
  AUDIO_CHANNEL_SNES_VOICE5, ///< Canal Voice 5 (SPC700)
  AUDIO_CHANNEL_SNES_VOICE6, ///< Canal Voice 6 (SPC700)
  AUDIO_CHANNEL_SNES_VOICE7, ///< Canal Voice 7 (SPC700)
  AUDIO_CHANNEL_SNES_VOICE8, ///< Canal Voice 8 (SPC700)

  // Canais específicos do Game Boy
  AUDIO_CHANNEL_GB_PULSE1, ///< Canal Pulse 1 (Game Boy)
  AUDIO_CHANNEL_GB_PULSE2, ///< Canal Pulse 2 (Game Boy)
  AUDIO_CHANNEL_GB_WAVE,   ///< Canal Wave (Game Boy)
  AUDIO_CHANNEL_GB_NOISE,  ///< Canal Noise (Game Boy)

  // Canais específicos do Master System e Game Gear
  AUDIO_CHANNEL_SMS_PSG1,      ///< Canal PSG 1 (Master System/Game Gear)
  AUDIO_CHANNEL_SMS_PSG2,      ///< Canal PSG 2 (Master System/Game Gear)
  AUDIO_CHANNEL_SMS_PSG3,      ///< Canal PSG 3 (Master System/Game Gear)
  AUDIO_CHANNEL_SMS_PSG_NOISE, ///< Canal de ruído do PSG (Master System/Game
                               ///< Gear)

  // Número total de canais possíveis
  AUDIO_CHANNEL_COUNT
} mega_emu_audio_channel_t;

/**
 * @brief Estado de um canal de áudio
 */
typedef struct {
  mega_emu_audio_channel_t id; ///< ID do canal
  char name[32];               ///< Nome legível
  bool enabled;                ///< Se o canal está habilitado
  uint8_t volume;              ///< Volume do canal (0-255)
  bool muted;                  ///< Se o canal está silenciado
  bool solo;                   ///< Se o canal está em modo solo

  // Dados específicos do canal
  bool is_active;     ///< Se o canal está sendo usado atualmente
  uint32_t frequency; ///< Frequência atual (se aplicável)
  uint32_t period;    ///< Período atual (se aplicável)
  uint32_t duty;      ///< Ciclo de trabalho (para canais pulse)

  // Buffer para visualização de forma de onda
  int16_t *wave_buffer; ///< Buffer para visualização
  uint32_t buffer_size; ///< Tamanho do buffer
  uint32_t buffer_pos;  ///< Posição atual no buffer
} mega_emu_audio_channel_state_t;

/**
 * @brief Plataformas suportadas para controle de áudio
 */
typedef enum {
  AUDIO_PLATFORM_MEGADRIVE,     ///< Mega Drive/Genesis
  AUDIO_PLATFORM_MASTERSYSTEM,  ///< Master System
  AUDIO_PLATFORM_GAMEGEAR,      ///< Game Gear
  AUDIO_PLATFORM_NES,           ///< NES
  AUDIO_PLATFORM_SNES,          ///< SNES
  AUDIO_PLATFORM_GAMEBOY,       ///< Game Boy
  AUDIO_PLATFORM_GAMEBOY_COLOR, ///< Game Boy Color
  AUDIO_PLATFORM_GENERIC        ///< Genérico
} mega_emu_audio_platform_t;

/**
 * @brief Função de callback para notificação de mudanças em canais de áudio
 */
typedef void (*mega_emu_audio_channel_callback_t)(
    mega_emu_audio_channel_t channel, bool enabled, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_AUDIO_CONTROL_TYPES_H */
