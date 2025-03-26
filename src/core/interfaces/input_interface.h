/**
 * @file input_interface.h
 * @brief Interface padrão para sistemas de input no Mega_Emu
 * @version 2.0
 *
 * Esta interface DEVE ser implementada por todos os adaptadores de input.
 * Parte da Fase 1 do plano de migração.
 */

#ifndef EMU_INPUT_INTERFACE_H
#define EMU_INPUT_INTERFACE_H

#include "core_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tipos de dispositivos de input suportados
 */
typedef enum {
  EMU_INPUT_DEVICE_NONE = 0,
  EMU_INPUT_DEVICE_JOYPAD,
  EMU_INPUT_DEVICE_MOUSE,
  EMU_INPUT_DEVICE_KEYBOARD,
  EMU_INPUT_DEVICE_LIGHTGUN,
  EMU_INPUT_DEVICE_PADDLE,
  EMU_INPUT_DEVICE_TRACKBALL
} emu_input_device_type_t;

/**
 * @brief Estados dos botões
 */
typedef enum {
  EMU_BUTTON_STATE_RELEASED = 0,
  EMU_BUTTON_STATE_PRESSED,
  EMU_BUTTON_STATE_HELD,
  EMU_BUTTON_STATE_DOUBLE_PRESSED
} emu_button_state_t;

/**
 * @brief Flags de status do input
 */
typedef enum {
  EMU_INPUT_FLAG_NONE = 0x00,
  EMU_INPUT_FLAG_CONNECTED = 0x01,
  EMU_INPUT_FLAG_CONFIGURED = 0x02,
  EMU_INPUT_FLAG_CALIBRATED = 0x04,
  EMU_INPUT_FLAG_ACTIVE = 0x08,
  EMU_INPUT_FLAG_ERROR = 0x10
} emu_input_flags_t;

/**
 * @brief Configuração do dispositivo de input
 */
typedef struct {
  emu_input_device_type_t type; // Tipo do dispositivo
  uint8_t port;                 // Porta do dispositivo
  uint8_t num_buttons;          // Número de botões
  uint8_t num_axes;             // Número de eixos
  bool analog;                  // Suporte a controles analógicos
  bool rumble;                  // Suporte a vibração
  void *user_data;              // Dados específicos da implementação
} emu_input_config_t;

/**
 * @brief Estado do dispositivo de input
 */
typedef struct {
  uint32_t buttons;        // Estado dos botões (bitmap)
  int16_t *axes;           // Estado dos eixos (-32768 a 32767)
  uint8_t num_axes;        // Número de eixos disponíveis
  emu_input_flags_t flags; // Flags de status
  void *context;           // Contexto específico
} emu_input_state_t;

/**
 * @brief Interface padrão para sistemas de input
 */
typedef struct {
  void *context; // Contexto da implementação

  // Funções de ciclo de vida
  int (*init)(void *ctx, const emu_input_config_t *config);
  void (*reset)(void *ctx);
  void (*shutdown)(void *ctx);

  // Funções de polling e eventos
  void (*poll)(void *ctx);
  void (*process_events)(void *ctx);
  bool (*is_button_pressed)(void *ctx, uint8_t button);
  bool (*is_button_released)(void *ctx, uint8_t button);
  bool (*is_button_held)(void *ctx, uint8_t button);
  int16_t (*get_axis)(void *ctx, uint8_t axis);

  // Funções de configuração
  void (*set_deadzone)(void *ctx, uint8_t axis, float deadzone);
  void (*set_sensitivity)(void *ctx, uint8_t axis, float sensitivity);
  void (*calibrate)(void *ctx);
  void (*map_button)(void *ctx, uint8_t physical, uint8_t logical);
  void (*map_axis)(void *ctx, uint8_t physical, uint8_t logical);

  // Funções de feedback
  void (*set_rumble)(void *ctx, float strong, float weak);
  void (*set_led)(void *ctx, uint8_t led, uint8_t r, uint8_t g, uint8_t b);

  // Funções de estado
  void (*get_state)(void *ctx, emu_input_state_t *state);
  void (*set_state)(void *ctx, const emu_input_state_t *state);
  emu_input_flags_t (*get_flags)(void *ctx);

  // Funções de debug
  void (*dump_state)(void *ctx, void *buffer, uint32_t size);
  void (*get_info)(void *ctx, void *info, uint32_t size);
  const char *(*get_device_name)(void *ctx);
} emu_input_interface_t;

/**
 * @brief Cria uma nova instância da interface de input
 * @param type Tipo de dispositivo de input
 * @return Ponteiro para a interface ou NULL em caso de erro
 */
emu_input_interface_t *emu_input_create(emu_input_device_type_t type);

/**
 * @brief Destrói uma instância da interface de input
 * @param input Ponteiro para a interface
 */
void emu_input_destroy(emu_input_interface_t *input);

#ifdef __cplusplus
}
#endif

#endif // EMU_INPUT_INTERFACE_H
