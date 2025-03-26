/**
 * @file input_adapter.h
 * @brief Adaptador de input para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef MEGADRIVE_INPUT_ADAPTER_H
#define MEGADRIVE_INPUT_ADAPTER_H

#include "core/interfaces/input_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Constantes do controle
 */
#define MD_MAX_CONTROLLERS 2
#define MD_BUTTON_COUNT_3BTN 8
#define MD_BUTTON_COUNT_6BTN 12

/**
 * @brief Botões do controle de 3 botões
 */
typedef enum {
  MD_BTN_UP = 0,
  MD_BTN_DOWN = 1,
  MD_BTN_LEFT = 2,
  MD_BTN_RIGHT = 3,
  MD_BTN_A = 4,
  MD_BTN_B = 5,
  MD_BTN_C = 6,
  MD_BTN_START = 7
} md_3button_t;

/**
 * @brief Botões adicionais do controle de 6 botões
 */
typedef enum {
  MD_BTN_X = 8,
  MD_BTN_Y = 9,
  MD_BTN_Z = 10,
  MD_BTN_MODE = 11
} md_6button_t;

/**
 * @brief Tipo de controle
 */
typedef enum {
  MD_PAD_TYPE_NONE = 0,
  MD_PAD_TYPE_3BTN,
  MD_PAD_TYPE_6BTN
} md_pad_type_t;

/**
 * @brief Estado do controle
 */
typedef struct {
  md_pad_type_t type;    // Tipo do controle
  uint16_t buttons;      // Estado dos botões
  uint16_t buttons_prev; // Estado anterior dos botões
  uint8_t counter;       // Contador para detecção de 6 botões
  bool connected;        // Controle conectado
} md_pad_state_t;

/**
 * @brief Contexto específico do adaptador de input
 */
typedef struct {
  md_pad_state_t pads[MD_MAX_CONTROLLERS]; // Estado dos controles
  uint32_t poll_counter;                   // Contador de polling
  bool polling_enabled;                    // Polling habilitado
  void *user_data;                         // Dados específicos da implementação
} megadrive_input_context_t;

/**
 * @brief Cria uma nova instância do adaptador de input
 * @return Ponteiro para a interface de input ou NULL em caso de erro
 */
emu_input_interface_t *megadrive_input_adapter_create(void);

/**
 * @brief Destrói uma instância do adaptador de input
 * @param input Ponteiro para a interface de input
 */
void megadrive_input_adapter_destroy(emu_input_interface_t *input);

/**
 * @brief Obtém o contexto específico do adaptador de input
 * @param input Ponteiro para a interface de input
 * @return Ponteiro para o contexto ou NULL em caso de erro
 */
megadrive_input_context_t *
megadrive_input_get_context(emu_input_interface_t *input);

/**
 * @brief Define o contexto específico do adaptador de input
 * @param input Ponteiro para a interface de input
 * @param context Ponteiro para o novo contexto
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int megadrive_input_set_context(emu_input_interface_t *input,
                                const megadrive_input_context_t *context);

/**
 * @brief Define o tipo de controle para uma porta
 * @param input Ponteiro para a interface de input
 * @param port Número da porta (0 ou 1)
 * @param type Tipo do controle
 */
void megadrive_input_set_pad_type(emu_input_interface_t *input, uint8_t port,
                                  md_pad_type_t type);

/**
 * @brief Define o estado de um botão
 * @param input Ponteiro para a interface de input
 * @param port Número da porta (0 ou 1)
 * @param button Número do botão
 * @param pressed Estado do botão (true = pressionado)
 */
void megadrive_input_set_button(emu_input_interface_t *input, uint8_t port,
                                uint8_t button, bool pressed);

/**
 * @brief Lê o estado de um botão
 * @param input Ponteiro para a interface de input
 * @param port Número da porta (0 ou 1)
 * @param button Número do botão
 * @return true se o botão está pressionado
 */
bool megadrive_input_get_button(emu_input_interface_t *input, uint8_t port,
                                uint8_t button);

/**
 * @brief Lê o estado de todos os botões de um controle
 * @param input Ponteiro para a interface de input
 * @param port Número da porta (0 ou 1)
 * @return Estado dos botões em formato bitmap
 */
uint16_t megadrive_input_get_pad_state(emu_input_interface_t *input,
                                       uint8_t port);

#ifdef __cplusplus
}
#endif

#endif // MEGADRIVE_INPUT_ADAPTER_H
