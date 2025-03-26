/**
 * @file sms_paddle.h
 * @brief Interface para o periférico Paddle do Master System
 */

#ifndef SMS_PADDLE_H
#define SMS_PADDLE_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/save_state.h"

/**
 * @brief Opaque handle para o Paddle
 */
typedef struct sms_paddle_t sms_paddle_t;

/**
 * @brief Modos de conexão do Paddle
 */
typedef enum {
    SMS_PADDLE_PORT1 = 0,    // Paddle conectado à porta 1
    SMS_PADDLE_PORT2 = 1     // Paddle conectado à porta 2
} sms_paddle_port_t;

/**
 * @brief Estado do Paddle
 */
typedef struct {
    uint8_t position;      // Posição do Paddle (0-255)
    bool button1;          // Estado do botão 1 (true = pressionado)
    bool button2;          // Estado do botão 2 (true = pressionado)
    bool connected;        // Indica se o Paddle está conectado
    uint8_t port;          // Porta à qual o Paddle está conectado (0-1)
} sms_paddle_state_t;

/**
 * @brief Cria uma nova instância do Paddle
 *
 * @param port Porta à qual o Paddle será conectado
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_paddle_t* sms_paddle_create(sms_paddle_port_t port);

/**
 * @brief Destrói uma instância do Paddle e libera recursos
 *
 * @param paddle Ponteiro para a instância
 */
void sms_paddle_destroy(sms_paddle_t* paddle);

/**
 * @brief Reseta o Paddle para o estado inicial
 *
 * @param paddle Ponteiro para a instância
 */
void sms_paddle_reset(sms_paddle_t* paddle);

/**
 * @brief Atualiza o estado do Paddle
 *
 * @param paddle Ponteiro para a instância
 * @param position Posição do Paddle (0-255)
 * @param button1 Estado do botão 1 (true = pressionado)
 * @param button2 Estado do botão 2 (true = pressionado)
 */
void sms_paddle_update(sms_paddle_t* paddle, uint8_t position, bool button1, bool button2);

/**
 * @brief Obtém o estado atual do Paddle em uma porta específica
 *
 * @param paddle Ponteiro para a instância
 * @param port Porta a ser verificada (0-1)
 * @param th_line Estado da linha TH
 * @return Byte representando o estado das linhas da porta
 */
uint8_t sms_paddle_read_port(sms_paddle_t* paddle, uint8_t port, bool th_line);

/**
 * @brief Obtém o estado atual do Paddle
 *
 * @param paddle Ponteiro para a instância
 * @param state Ponteiro para a estrutura que receberá o estado
 */
void sms_paddle_get_state(sms_paddle_t* paddle, sms_paddle_state_t* state);

/**
 * @brief Registra o estado do Paddle no sistema de save state
 *
 * @param paddle Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_paddle_register_save_state(sms_paddle_t* paddle, save_state_t* state);

/**
 * @brief Atualiza o estado do Paddle após um carregamento de estado
 *
 * @param paddle Ponteiro para a instância
 */
void sms_paddle_update_after_state_load(sms_paddle_t* paddle);

#endif // SMS_PADDLE_H
