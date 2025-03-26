/**
 * @file sms_peripherals.h
 * @brief Interface principal para os periféricos do Master System
 */

#ifndef SMS_PERIPHERALS_H
#define SMS_PERIPHERALS_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/save_state.h"
#include "sms_lightphaser.h"
#include "sms_paddle.h"

/**
 * @brief Tipos de periféricos suportados pelo Master System
 */
typedef enum {
    SMS_PERIPHERAL_CONTROLLER,   // Controle padrão
    SMS_PERIPHERAL_LIGHTPHASER,  // Light Phaser (pistola de luz)
    SMS_PERIPHERAL_PADDLE,       // Paddle (Controle rotativo)
    SMS_PERIPHERAL_SPORTPAD,     // Sports Pad (não implementado ainda)
    SMS_PERIPHERAL_KEYBOARD,     // Teclado (não implementado ainda)
    SMS_PERIPHERAL_NONE          // Nenhum periférico conectado
} sms_peripheral_type_t;

/**
 * @brief Estrutura principal de periféricos do Master System
 */
typedef struct {
    // Tipo de periférico conectado a cada porta
    sms_peripheral_type_t port1_type;
    sms_peripheral_type_t port2_type;

    // Ponteiros para os periféricos específicos
    sms_lightphaser_t* lightphaser;
    sms_paddle_t* paddle;

    // Estado dos controles padrão
    uint8_t controller_state[2];  // 0 = porta 1, 1 = porta 2
} sms_peripherals_t;

/**
 * @brief Inicializa o subsistema de periféricos do Master System
 *
 * @return Ponteiro para a estrutura de periféricos ou NULL em caso de erro
 */
sms_peripherals_t* sms_peripherals_init(void);

/**
 * @brief Libera os recursos do subsistema de periféricos
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 */
void sms_peripherals_free(sms_peripherals_t* peripherals);

/**
 * @brief Redefine o estado de todos os periféricos
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 */
void sms_peripherals_reset(sms_peripherals_t* peripherals);

/**
 * @brief Conecta um periférico a uma porta específica
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param port Número da porta (0 = porta 1, 1 = porta 2)
 * @param type Tipo de periférico a ser conectado
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int sms_peripherals_connect(sms_peripherals_t* peripherals, uint8_t port, sms_peripheral_type_t type);

/**
 * @brief Desconecta o periférico de uma porta específica
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param port Número da porta (0 = porta 1, 1 = porta 2)
 */
void sms_peripherals_disconnect(sms_peripherals_t* peripherals, uint8_t port);

/**
 * @brief Atualiza o estado do controle padrão
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param port Número da porta (0 = porta 1, 1 = porta 2)
 * @param up Estado do botão Up
 * @param down Estado do botão Down
 * @param left Estado do botão Left
 * @param right Estado do botão Right
 * @param button1 Estado do botão 1
 * @param button2 Estado do botão 2
 */
void sms_peripherals_update_controller(sms_peripherals_t* peripherals, uint8_t port,
                                    bool up, bool down, bool left, bool right,
                                    bool button1, bool button2);

/**
 * @brief Atualiza o estado do Light Phaser
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param x Posição X do cursor (0-255)
 * @param y Posição Y do cursor (0-191)
 * @param trigger Estado do gatilho (true = pressionado)
 */
void sms_peripherals_update_lightphaser(sms_peripherals_t* peripherals,
                                     uint16_t x, uint16_t y, bool trigger);

/**
 * @brief Atualiza o estado do Paddle
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param position Posição do Paddle (0-255)
 * @param button1 Estado do botão 1 (true = pressionado)
 * @param button2 Estado do botão 2 (true = pressionado)
 */
void sms_peripherals_update_paddle(sms_peripherals_t* peripherals,
                                uint8_t position, bool button1, bool button2);

/**
 * @brief Verifica a detecção de alvos pelo Light Phaser
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param frame_buffer Buffer de frame atual do VDP
 * @param vdp_line Linha atual do VDP
 * @param h_counter Contador horizontal do VDP
 */
void sms_peripherals_process_lightphaser(sms_peripherals_t* peripherals,
                                      const uint32_t* frame_buffer,
                                      uint8_t vdp_line, uint8_t h_counter);

/**
 * @brief Lê o estado das portas de entrada/saída
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param port Número da porta (0x3F = porta 1, 0xDC = porta 2)
 * @param th_line Estado da linha TH
 * @return Byte representando o estado das linhas da porta
 */
uint8_t sms_peripherals_read_port(sms_peripherals_t* peripherals, uint8_t port, bool th_line);

/**
 * @brief Registra os dados dos periféricos no sistema de save state
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_peripherals_register_save_state(sms_peripherals_t* peripherals, save_state_t* state);

/**
 * @brief Atualiza o estado dos periféricos após o carregamento de um save state
 *
 * @param peripherals Ponteiro para a estrutura de periféricos
 */
void sms_peripherals_update_after_state_load(sms_peripherals_t* peripherals);

#endif // SMS_PERIPHERALS_H
