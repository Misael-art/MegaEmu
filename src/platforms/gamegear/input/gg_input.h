/**
 * @file gg_input.h
 * @brief Interface do sistema de entrada do Game Gear
 */

#ifndef GG_INPUT_H
#define GG_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../utils/save_state.h"

// Botões do Game Gear
typedef enum {
    GG_BUTTON_UP     = 0x01,
    GG_BUTTON_DOWN   = 0x02,
    GG_BUTTON_LEFT   = 0x04,
    GG_BUTTON_RIGHT  = 0x08,
    GG_BUTTON_1      = 0x10,
    GG_BUTTON_2      = 0x20,
    GG_BUTTON_START  = 0x40
} gg_button_t;

// Forward declaration
typedef struct gg_input_t gg_input_t;

/**
 * @brief Cria uma nova instância do sistema de entrada
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_input_t *gg_input_create(void);

/**
 * @brief Destrói uma instância do sistema de entrada
 * @param input Ponteiro para a instância
 */
void gg_input_destroy(gg_input_t *input);

/**
 * @brief Reseta o sistema de entrada
 * @param input Ponteiro para a instância
 */
void gg_input_reset(gg_input_t *input);

/**
 * @brief Define o estado de um botão
 * @param input Ponteiro para a instância
 * @param button Botão a ser configurado
 * @param pressed true se o botão está pressionado, false caso contrário
 */
void gg_input_set_button(gg_input_t *input, gg_button_t button, bool pressed);

/**
 * @brief Obtém o estado de um botão
 * @param input Ponteiro para a instância
 * @param button Botão a ser consultado
 * @return true se o botão está pressionado, false caso contrário
 */
bool gg_input_get_button(const gg_input_t *input, gg_button_t button);

/**
 * @brief Lê o estado da porta de I/O 1 (Start, D-Pad)
 * @param input Ponteiro para a instância
 * @return Estado da porta
 */
uint8_t gg_input_read_port1(const gg_input_t *input);

/**
 * @brief Lê o estado da porta de I/O 2 (Botões 1 e 2)
 * @param input Ponteiro para a instância
 * @return Estado da porta
 */
uint8_t gg_input_read_port2(const gg_input_t *input);

/**
 * @brief Registra campos do sistema de entrada no sistema de save state
 * @param input Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_input_register_save_state(gg_input_t *input, save_state_t *state);

#endif // GG_INPUT_H
