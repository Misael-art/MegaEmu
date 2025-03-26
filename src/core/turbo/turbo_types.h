/**
 * @file turbo_types.h
 * @brief Definições de tipos e estruturas para o sistema de turbo/autofire
 */

#ifndef MEGA_EMU_TURBO_TYPES_H
#define MEGA_EMU_TURBO_TYPES_H

#include "../global_defines.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Botões de controle suportados para turbo
 */
typedef enum {
  TURBO_BUTTON_NONE = 0,

  // Botões genéricos (para qualquer plataforma)
  TURBO_BUTTON_A,
  TURBO_BUTTON_B,
  TURBO_BUTTON_C,
  TURBO_BUTTON_X,
  TURBO_BUTTON_Y,
  TURBO_BUTTON_Z,
  TURBO_BUTTON_L,
  TURBO_BUTTON_R,
  TURBO_BUTTON_START,
  TURBO_BUTTON_SELECT,
  TURBO_BUTTON_UP,
  TURBO_BUTTON_DOWN,
  TURBO_BUTTON_LEFT,
  TURBO_BUTTON_RIGHT,

  // Botões NES específicos
  TURBO_BUTTON_NES_A,
  TURBO_BUTTON_NES_B,
  TURBO_BUTTON_NES_START,
  TURBO_BUTTON_NES_SELECT,

  // Botões Mega Drive específicos
  TURBO_BUTTON_MD_A,
  TURBO_BUTTON_MD_B,
  TURBO_BUTTON_MD_C,
  TURBO_BUTTON_MD_X,
  TURBO_BUTTON_MD_Y,
  TURBO_BUTTON_MD_Z,
  TURBO_BUTTON_MD_START,
  TURBO_BUTTON_MD_MODE,

  // Botões Master System específicos
  TURBO_BUTTON_SMS_1,
  TURBO_BUTTON_SMS_2,

  // Botões SNES específicos
  TURBO_BUTTON_SNES_A,
  TURBO_BUTTON_SNES_B,
  TURBO_BUTTON_SNES_X,
  TURBO_BUTTON_SNES_Y,
  TURBO_BUTTON_SNES_L,
  TURBO_BUTTON_SNES_R,
  TURBO_BUTTON_SNES_START,
  TURBO_BUTTON_SNES_SELECT,

  // Limite para enumeração
  TURBO_BUTTON_COUNT
} mega_emu_turbo_button_t;

/**
 * @brief Velocidades predefinidas para turbo
 */
typedef enum {
  TURBO_SPEED_SLOW = 0, ///< Velocidade lenta (5-6 Hz)
  TURBO_SPEED_MEDIUM,   ///< Velocidade média (10-12 Hz)
  TURBO_SPEED_FAST,     ///< Velocidade rápida (15-20 Hz)
  TURBO_SPEED_ULTRA,    ///< Velocidade ultra rápida (30 Hz)
  TURBO_SPEED_CUSTOM    ///< Velocidade personalizada
} mega_emu_turbo_speed_preset_t;

/**
 * @brief Modos de operação para turbo
 */
typedef enum {
  TURBO_MODE_TOGGLE, ///< Alterna entre pressionado e não pressionado
  TURBO_MODE_PULSE,  ///< Pulsa enquanto pressionado
  TURBO_MODE_HOLD    ///< Mantém pressionado até soltar
} mega_emu_turbo_mode_t;

/**
 * @brief Plataformas suportadas para turbo
 */
typedef enum {
  TURBO_PLATFORM_MEGADRIVE,    ///< Mega Drive/Genesis
  TURBO_PLATFORM_MASTERSYSTEM, ///< Master System
  TURBO_PLATFORM_GAMEGEAR,     ///< Game Gear
  TURBO_PLATFORM_NES,          ///< NES
  TURBO_PLATFORM_SNES,         ///< SNES
  TURBO_PLATFORM_GAMEBOY,      ///< Game Boy
  TURBO_PLATFORM_GENERIC       ///< Genérico (para todos os sistemas)
} mega_emu_turbo_platform_t;

/**
 * @brief Estrutura para configuração de turbo de um botão
 */
typedef struct {
  mega_emu_turbo_button_t button;             ///< Botão configurado
  bool enabled;                               ///< Se o turbo está habilitado
  mega_emu_turbo_mode_t mode;                 ///< Modo de operação
  mega_emu_turbo_speed_preset_t speed_preset; ///< Velocidade predefinida
  uint8_t
      custom_speed; ///< Velocidade personalizada (em Hz, se TURBO_SPEED_CUSTOM)
  uint8_t duty_cycle;      ///< Ciclo de trabalho (0-100%)
  uint8_t controller_port; ///< Porta do controle (0-7)

  // Estado interno
  uint32_t counter;    ///< Contador interno
  uint32_t period;     ///< Período entre pulsos
  bool state;          ///< Estado atual (pressionado ou não)
  bool button_pressed; ///< Se o botão físico está pressionado
} mega_emu_turbo_config_t;

/**
 * @brief Mapeamento entre botões turbo e botões de entrada reais
 */
typedef struct {
  mega_emu_turbo_button_t turbo_button;
  uint32_t input_button_mask; ///< Máscara do botão real na interface de entrada
} mega_emu_turbo_button_mapping_t;

/**
 * @brief Callback para eventos de turbo
 */
typedef void (*mega_emu_turbo_callback_t)(mega_emu_turbo_button_t button,
                                          bool state, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_TURBO_TYPES_H */
