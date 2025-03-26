/**
 * @file sms_lightphaser.h
 * @brief Interface para o periférico Light Phaser do Master System
 */

#ifndef SMS_LIGHTPHASER_H
#define SMS_LIGHTPHASER_H

#include "../../../core/save_state.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Opaque handle para o Light Phaser
 */
typedef struct sms_lightphaser_t sms_lightphaser_t;

/**
 * @brief Modos de conexão do Light Phaser
 */
typedef enum {
  SMS_LIGHTPHASER_PORT1 = 0, // Light Phaser conectado à porta 1
  SMS_LIGHTPHASER_PORT2 = 1  // Light Phaser conectado à porta 2
} sms_lightphaser_port_t;

/**
 * @brief Estado do Light Phaser
 */
typedef struct {
  uint16_t x;     // Posição X do cursor (0-255)
  uint16_t y;     // Posição Y do cursor (0-191)
  bool trigger;   // Estado do gatilho (true = pressionado)
  bool connected; // Indica se o Light Phaser está conectado
  uint8_t port;   // Porta à qual o Light Phaser está conectado (0-1)
} sms_lightphaser_state_t;

/**
 * @brief Cria uma nova instância do Light Phaser
 *
 * @param port Porta à qual o Light Phaser será conectado
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_lightphaser_t *sms_lightphaser_create(sms_lightphaser_port_t port);

/**
 * @brief Destrói uma instância do Light Phaser e libera recursos
 *
 * @param lightphaser Ponteiro para a instância
 */
void sms_lightphaser_destroy(sms_lightphaser_t *lightphaser);

/**
 * @brief Reseta o Light Phaser para o estado inicial
 *
 * @param lightphaser Ponteiro para a instância
 */
void sms_lightphaser_reset(sms_lightphaser_t *lightphaser);

/**
 * @brief Atualiza o estado do Light Phaser
 *
 * @param lightphaser Ponteiro para a instância
 * @param x Posição X do cursor (0-255)
 * @param y Posição Y do cursor (0-191)
 * @param trigger Estado do gatilho (true = pressionado)
 */
void sms_lightphaser_update(sms_lightphaser_t *lightphaser, uint16_t x,
                            uint16_t y, bool trigger);

/**
 * @brief Verifica se o Light Phaser está detectando um alvo na posição atual
 *
 * @param lightphaser Ponteiro para a instância
 * @param frame_buffer Buffer de frame atual do VDP
 * @param vdp_line Linha atual do VDP
 * @param h_counter Contador horizontal do VDP
 * @return true se o Light Phaser detecta um alvo, false caso contrário
 */
bool sms_lightphaser_detect_target(sms_lightphaser_t *lightphaser,
                                   const uint32_t *frame_buffer,
                                   uint8_t vdp_line, uint8_t h_counter);

/**
 * @brief Obtém o estado atual do Light Phaser na porta especificada
 *
 * @param lightphaser Ponteiro para a instância
 * @param port Porta a ser verificada (0-1)
 * @return Byte representando o estado das linhas da porta
 */
uint8_t sms_lightphaser_read_port(sms_lightphaser_t *lightphaser, uint8_t port);

/**
 * @brief Obtém o estado atual do Light Phaser
 *
 * @param lightphaser Ponteiro para a instância
 * @param state Ponteiro para a estrutura que receberá o estado
 */
void sms_lightphaser_get_state(sms_lightphaser_t *lightphaser,
                               sms_lightphaser_state_t *state);

/**
 * @brief Registra o estado do Light Phaser no sistema de save state
 *
 * @param lightphaser Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_lightphaser_register_save_state(sms_lightphaser_t *lightphaser,
                                        save_state_t *state);

/**
 * @brief Atualiza o estado do Light Phaser após um carregamento de estado
 *
 * @param lightphaser Ponteiro para a instância
 */
void sms_lightphaser_update_after_state_load(sms_lightphaser_t *lightphaser);

#endif // SMS_LIGHTPHASER_H
