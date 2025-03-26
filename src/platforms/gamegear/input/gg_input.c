/**
 * @file gg_input.c
 * @brief Implementação do sistema de entrada do Game Gear
 */

#include "gg_input.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG_INPUT EMU_LOG_CAT_INPUT

// Macros de log
#define GG_INPUT_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG_INPUT, __VA_ARGS__)
#define GG_INPUT_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG_INPUT, __VA_ARGS__)
#define GG_INPUT_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG_INPUT, __VA_ARGS__)
#define GG_INPUT_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG_INPUT, __VA_ARGS__)
#define GG_INPUT_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG_INPUT, __VA_ARGS__)

/**
 * @brief Estrutura do sistema de entrada do Game Gear
 */
struct gg_input_t {
  uint8_t button_state; // Estado dos botões
};

/**
 * @brief Cria uma nova instância do sistema de entrada
 */
gg_input_t *gg_input_create(void) {
  gg_input_t *input = (gg_input_t *)malloc(sizeof(gg_input_t));
  if (!input) {
    GG_INPUT_LOG_ERROR("Falha ao alocar memória para sistema de entrada");
    return NULL;
  }

  // Inicializa estrutura
  input->button_state = 0;

  GG_INPUT_LOG_INFO("Sistema de entrada do Game Gear criado");
  return input;
}

/**
 * @brief Destrói uma instância do sistema de entrada
 */
void gg_input_destroy(gg_input_t *input) {
  if (!input)
    return;
  free(input);
  GG_INPUT_LOG_INFO("Sistema de entrada do Game Gear destruído");
}

/**
 * @brief Reseta o sistema de entrada
 */
void gg_input_reset(gg_input_t *input) {
  if (!input)
    return;
  input->button_state = 0;
  GG_INPUT_LOG_INFO("Sistema de entrada do Game Gear resetado");
}

/**
 * @brief Define o estado de um botão
 */
void gg_input_set_button(gg_input_t *input, gg_button_t button, bool pressed) {
  if (!input)
    return;

  if (pressed) {
    input->button_state |= button;
  } else {
    input->button_state &= ~button;
  }

  GG_INPUT_LOG_TRACE("Estado do botão %02X atualizado: %s", button,
                     pressed ? "pressionado" : "solto");
}

/**
 * @brief Obtém o estado de um botão
 */
bool gg_input_get_button(const gg_input_t *input, gg_button_t button) {
  if (!input)
    return false;
  return (input->button_state & button) != 0;
}

/**
 * @brief Lê o estado da porta de I/O 1 (Start, D-Pad)
 */
uint8_t gg_input_read_port1(const gg_input_t *input) {
  if (!input)
    return 0xFF;

  uint8_t value = 0xFF;

  // O Game Gear usa lógica invertida (0 = pressionado, 1 = solto)
  if (input->button_state & GG_BUTTON_UP)
    value &= ~0x01;
  if (input->button_state & GG_BUTTON_DOWN)
    value &= ~0x02;
  if (input->button_state & GG_BUTTON_LEFT)
    value &= ~0x04;
  if (input->button_state & GG_BUTTON_RIGHT)
    value &= ~0x08;
  if (input->button_state & GG_BUTTON_START)
    value &= ~0x40;

  return value;
}

/**
 * @brief Lê o estado da porta de I/O 2 (Botões 1 e 2)
 */
uint8_t gg_input_read_port2(const gg_input_t *input) {
  if (!input)
    return 0xFF;

  uint8_t value = 0xFF;

  // O Game Gear usa lógica invertida (0 = pressionado, 1 = solto)
  if (input->button_state & GG_BUTTON_1)
    value &= ~0x10;
  if (input->button_state & GG_BUTTON_2)
    value &= ~0x20;

  return value;
}

/**
 * @brief Registra campos do sistema de entrada no sistema de save state
 */
int gg_input_register_save_state(gg_input_t *input, save_state_t *state) {
  if (!input || !state)
    return -1;

  // Registra estado dos botões
  save_state_register_field(state, "gg_input_button_state",
                            &input->button_state, sizeof(input->button_state));

  return 0;
}
