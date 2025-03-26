/**
 * @file sms_io.h
 * @brief Interface para o sistema de I/O do Master System
 */

#ifndef SMS_IO_H
#define SMS_IO_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/save_state.h"
#include "../peripherals/sms_peripherals.h"

// Endereços de portas do Master System
#define SMS_IO_PORT_JOYSTICK1       0xDC    // Leitura do controle 1
#define SMS_IO_PORT_JOYSTICK2       0xDD    // Leitura do controle 2
#define SMS_IO_PORT_EXP             0xDE    // Porta de expansão
#define SMS_IO_PORT_REGION          0x3F    // Leitura de região e controle 1
#define SMS_IO_PORT_VDPCTRL         0xBF    // Porta de controle do VDP
#define SMS_IO_PORT_VDPDATA         0xBE    // Porta de dados do VDP
#define SMS_IO_PORT_HCOUNTER        0x7F    // Contador horizontal do VDP
#define SMS_IO_PORT_VCOUNTER        0x7E    // Contador vertical do VDP
#define SMS_IO_PORT_PSGDATA         0x7F    // Porta de dados do PSG
#define SMS_IO_PORT_MEMCTRL         0x3E    // Controle de memória
#define SMS_IO_PORT_IOCTRL          0x3F    // Controle de I/O

/**
 * @brief Estrutura do sistema de I/O do Master System
 */
typedef struct {
    sms_peripherals_t* peripherals;     // Periféricos conectados
    uint8_t io_control;                 // Registro de controle de I/O
    uint8_t memory_control;             // Registro de controle de memória
    bool region_is_japan;               // Flag de região: true = Japão, false = Exportação
    uint8_t th_line_status[2];          // Estado da linha TH para cada porta (0 = porta 1, 1 = porta 2)
} sms_io_t;

/**
 * @brief Inicializa o sistema de I/O do Master System
 *
 * @param is_japan Flag indicando se o sistema é versão japonesa
 * @return Ponteiro para a estrutura de I/O ou NULL em caso de erro
 */
sms_io_t* sms_io_init(bool is_japan);

/**
 * @brief Libera os recursos do sistema de I/O
 *
 * @param io Ponteiro para a estrutura de I/O
 */
void sms_io_free(sms_io_t* io);

/**
 * @brief Reseta o sistema de I/O para o estado inicial
 *
 * @param io Ponteiro para a estrutura de I/O
 */
void sms_io_reset(sms_io_t* io);

/**
 * @brief Escreve um byte em uma porta de I/O
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param port Endereço da porta
 * @param value Valor a ser escrito
 */
void sms_io_write_port(sms_io_t* io, uint8_t port, uint8_t value);

/**
 * @brief Lê um byte de uma porta de I/O
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param port Endereço da porta
 * @return Valor lido da porta
 */
uint8_t sms_io_read_port(sms_io_t* io, uint8_t port);

/**
 * @brief Processa a detecção de alvos do Light Phaser para a linha atual do VDP
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param frame_buffer Buffer de frame atual do VDP
 * @param vdp_line Linha atual do VDP
 * @param h_counter Contador horizontal do VDP
 */
void sms_io_process_lightphaser(sms_io_t* io, const uint32_t* frame_buffer,
                              uint8_t vdp_line, uint8_t h_counter);

/**
 * @brief Atualiza o estado do controle padrão
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param port Número da porta (0 = porta 1, 1 = porta 2)
 * @param up Estado do botão Up
 * @param down Estado do botão Down
 * @param left Estado do botão Left
 * @param right Estado do botão Right
 * @param button1 Estado do botão 1
 * @param button2 Estado do botão 2
 */
void sms_io_update_controller(sms_io_t* io, uint8_t port,
                           bool up, bool down, bool left, bool right,
                           bool button1, bool button2);

/**
 * @brief Atualiza o estado do Light Phaser
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param x Posição X do cursor (0-255)
 * @param y Posição Y do cursor (0-191)
 * @param trigger Estado do gatilho (true = pressionado)
 */
void sms_io_update_lightphaser(sms_io_t* io, uint16_t x, uint16_t y, bool trigger);

/**
 * @brief Atualiza o estado do Paddle
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param position Posição do Paddle (0-255)
 * @param button1 Estado do botão 1 (true = pressionado)
 * @param button2 Estado do botão 2 (true = pressionado)
 */
void sms_io_update_paddle(sms_io_t* io, uint8_t position, bool button1, bool button2);

/**
 * @brief Registra o estado do sistema de I/O no save state
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_io_register_save_state(sms_io_t* io, save_state_t* state);

/**
 * @brief Atualiza o sistema de I/O após carregar um save state
 *
 * @param io Ponteiro para a estrutura de I/O
 */
void sms_io_update_after_state_load(sms_io_t* io);

/**
 * @brief Conecta um periférico específico à porta indicada
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param port Número da porta (0 = porta 1, 1 = porta 2)
 * @param type Tipo de periférico a ser conectado
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int sms_io_connect_peripheral(sms_io_t* io, uint8_t port, sms_peripheral_type_t type);

/**
 * @brief Desconecta o periférico da porta indicada
 *
 * @param io Ponteiro para a estrutura de I/O
 * @param port Número da porta (0 = porta 1, 1 = porta 2)
 */
void sms_io_disconnect_peripheral(sms_io_t* io, uint8_t port);

#endif // SMS_IO_H
