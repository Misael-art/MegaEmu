/**
 * @file sms_io.h
 * @brief Definições para o sistema de entrada e controles do Master System
 */

#ifndef SMS_IO_H
#define SMS_IO_H

#include <stdint.h>
#include "../../../core/save_state.h"

/**
 * @brief Definições dos botões do Master System
 */
#define SMS_BUTTON_UP     0x01
#define SMS_BUTTON_DOWN   0x02
#define SMS_BUTTON_LEFT   0x04
#define SMS_BUTTON_RIGHT  0x08
#define SMS_BUTTON_1      0x10
#define SMS_BUTTON_2      0x20
#define SMS_BUTTON_START  0x40  // Botão START (Game Gear) ou PAUSE (Master System)

/**
 * @brief Opaque handle para o sistema de entrada
 */
typedef struct sms_input_t sms_input_t;

/**
 * @brief Cria uma nova instância do sistema de entrada
 * 
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_input_t* sms_input_create(void);

/**
 * @brief Destrói uma instância do sistema de entrada e libera recursos
 * 
 * @param input Ponteiro para a instância
 */
void sms_input_destroy(sms_input_t *input);

/**
 * @brief Reseta o sistema de entrada para o estado inicial
 * 
 * @param input Ponteiro para a instância
 */
void sms_input_reset(sms_input_t *input);

/**
 * @brief Define o estado dos botões do controlador 1
 * 
 * @param input Ponteiro para a instância
 * @param button_state Estado dos botões (bitmap)
 */
void sms_input_set_controller1(sms_input_t *input, uint8_t button_state);

/**
 * @brief Define o estado dos botões do controlador 2
 * 
 * @param input Ponteiro para a instância
 * @param button_state Estado dos botões (bitmap)
 */
void sms_input_set_controller2(sms_input_t *input, uint8_t button_state);

/**
 * @brief Lê o estado atual do controlador 1
 * 
 * @param input Ponteiro para a instância
 * @return Estado dos botões (bitmap)
 */
uint8_t sms_input_read_controller1(sms_input_t *input);

/**
 * @brief Lê o estado atual do controlador 2
 * 
 * @param input Ponteiro para a instância
 * @return Estado dos botões (bitmap)
 */
uint8_t sms_input_read_controller2(sms_input_t *input);

/**
 * @brief Lê o estado atual das portas de I/O
 * 
 * @param input Ponteiro para a instância
 * @param port Número da porta
 * @return Valor lido
 */
uint8_t sms_input_read_port(sms_input_t *input, uint8_t port);

/**
 * @brief Escreve um valor nas portas de I/O
 * 
 * @param input Ponteiro para a instância
 * @param port Número da porta
 * @param value Valor a ser escrito
 */
void sms_input_write_port(sms_input_t *input, uint8_t port, uint8_t value);

/**
 * @brief Registra o sistema de entrada no sistema de save state
 * 
 * @param input Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_input_register_save_state(sms_input_t *input, save_state_t *state);

#endif /* SMS_IO_H */
