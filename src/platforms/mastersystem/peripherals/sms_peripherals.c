/**
 * @file sms_peripherals.c
 * @brief Implementação do gerenciador de periféricos do Master System
 */

#include "sms_peripherals.h"
#include "../../../core/logging.h"
#include <stdlib.h>
#include <string.h>

// Definições para portas de I/O
#define SMS_PORT1_ADDRESS 0x3F
#define SMS_PORT2_ADDRESS 0xDC

// Bits para os controles padrão
#define SMS_BUTTON_UP 0x01
#define SMS_BUTTON_DOWN 0x02
#define SMS_BUTTON_LEFT 0x04
#define SMS_BUTTON_RIGHT 0x08
#define SMS_BUTTON1 0x10
#define SMS_BUTTON2 0x20
#define SMS_TH_LINE 0x40
#define SMS_TR_LINE 0x80

// Implementações de funções

sms_peripherals_t *sms_peripherals_init(void) {
  sms_peripherals_t *peripherals =
      (sms_peripherals_t *)malloc(sizeof(sms_peripherals_t));
  if (!peripherals) {
    LOG_ERROR("Falha ao alocar memória para os periféricos");
    return NULL;
  }

  // Inicializa a estrutura
  memset(peripherals, 0, sizeof(sms_peripherals_t));

  // Por padrão, configura controles padrão em ambas as portas
  peripherals->port1_type = SMS_PERIPHERAL_CONTROLLER;
  peripherals->port2_type = SMS_PERIPHERAL_CONTROLLER;

  // Inicializa o estado dos controles com todos os botões liberados (1 = não
  // pressionado)
  peripherals->controller_state[0] = 0xFF;
  peripherals->controller_state[1] = 0xFF;

  LOG_INFO("Subsistema de periféricos do Master System inicializado");
  return peripherals;
}

void sms_peripherals_free(sms_peripherals_t *peripherals) {
  if (!peripherals) {
    return;
  }

  // Libera os periféricos especiais
  if (peripherals->lightphaser) {
    sms_lightphaser_destroy(peripherals->lightphaser);
    peripherals->lightphaser = NULL;
  }

  if (peripherals->paddle) {
    sms_paddle_destroy(peripherals->paddle);
    peripherals->paddle = NULL;
  }

  // Libera a estrutura principal
  free(peripherals);

  LOG_INFO("Subsistema de periféricos do Master System liberado");
}

void sms_peripherals_reset(sms_peripherals_t *peripherals) {
  if (!peripherals) {
    return;
  }

  // Reseta o estado dos controles
  peripherals->controller_state[0] = 0xFF;
  peripherals->controller_state[1] = 0xFF;

  // Reseta os periféricos especiais
  if (peripherals->lightphaser) {
    sms_lightphaser_reset(peripherals->lightphaser);
  }

  if (peripherals->paddle) {
    sms_paddle_reset(peripherals->paddle);
  }

  LOG_DEBUG("Periféricos resetados");
}

int sms_peripherals_connect(sms_peripherals_t *peripherals, uint8_t port,
                            sms_peripheral_type_t type) {
  if (!peripherals || port > 1) {
    return -1; // Erro: parâmetros inválidos
  }

  // Primeiro desconecta qualquer periférico existente
  sms_peripherals_disconnect(peripherals, port);

  // Define o tipo de periférico na porta especificada
  if (port == 0) {
    peripherals->port1_type = type;
  } else {
    peripherals->port2_type = type;
  }

  // Cria o periférico específico se necessário
  switch (type) {
  case SMS_PERIPHERAL_LIGHTPHASER:
    if (!peripherals->lightphaser) {
      peripherals->lightphaser = sms_lightphaser_create(
          port == 0 ? SMS_LIGHTPHASER_PORT1 : SMS_LIGHTPHASER_PORT2);
      if (!peripherals->lightphaser) {
        LOG_ERROR("Falha ao criar Light Phaser para porta %d", port);
        if (port == 0) {
          peripherals->port1_type = SMS_PERIPHERAL_NONE;
        } else {
          peripherals->port2_type = SMS_PERIPHERAL_NONE;
        }
        return -2; // Erro: falha ao criar periférico
      }
    }
    break;

  case SMS_PERIPHERAL_PADDLE:
    if (!peripherals->paddle) {
      peripherals->paddle =
          sms_paddle_create(port == 0 ? SMS_PADDLE_PORT1 : SMS_PADDLE_PORT2);
      if (!peripherals->paddle) {
        LOG_ERROR("Falha ao criar Paddle para porta %d", port);
        if (port == 0) {
          peripherals->port1_type = SMS_PERIPHERAL_NONE;
        } else {
          peripherals->port2_type = SMS_PERIPHERAL_NONE;
        }
        return -2; // Erro: falha ao criar periférico
      }
    }
    break;

  // Casos não implementados (apenas para futuras expansões)
  case SMS_PERIPHERAL_SPORTPAD:
  case SMS_PERIPHERAL_KEYBOARD:
    LOG_WARN("Tipo de periférico não implementado: %d", type);
    if (port == 0) {
      peripherals->port1_type = SMS_PERIPHERAL_NONE;
    } else {
      peripherals->port2_type = SMS_PERIPHERAL_NONE;
    }
    return -3; // Erro: tipo não suportado

  case SMS_PERIPHERAL_CONTROLLER:
  case SMS_PERIPHERAL_NONE:
    // Estes não requerem alocação especial
    break;
  }

  LOG_INFO("Periférico do tipo %d conectado à porta %d", type, port);
  return 0; // Sucesso
}

void sms_peripherals_disconnect(sms_peripherals_t *peripherals, uint8_t port) {
  if (!peripherals || port > 1) {
    return;
  }

  sms_peripheral_type_t current_type =
      (port == 0) ? peripherals->port1_type : peripherals->port2_type;

  // Libera o periférico específico se necessário
  switch (current_type) {
  case SMS_PERIPHERAL_LIGHTPHASER:
    if (peripherals->lightphaser) {
      sms_lightphaser_destroy(peripherals->lightphaser);
      peripherals->lightphaser = NULL;
    }
    break;

  case SMS_PERIPHERAL_PADDLE:
    if (peripherals->paddle) {
      sms_paddle_destroy(peripherals->paddle);
      peripherals->paddle = NULL;
    }
    break;

  // Outros tipos não necessitam de limpeza especial
  default:
    break;
  }

  // Marca a porta como desconectada
  if (port == 0) {
    peripherals->port1_type = SMS_PERIPHERAL_NONE;
    peripherals->controller_state[0] = 0xFF; // Estado padrão
  } else {
    peripherals->port2_type = SMS_PERIPHERAL_NONE;
    peripherals->controller_state[1] = 0xFF; // Estado padrão
  }

  LOG_INFO("Periférico desconectado da porta %d", port);
}

void sms_peripherals_update_controller(sms_peripherals_t *peripherals,
                                       uint8_t port, bool up, bool down,
                                       bool left, bool right, bool button1,
                                       bool button2) {
  if (!peripherals || port > 1) {
    return;
  }

  // Verifica se há um controle padrão na porta
  sms_peripheral_type_t type =
      (port == 0) ? peripherals->port1_type : peripherals->port2_type;

  if (type != SMS_PERIPHERAL_CONTROLLER) {
    return; // Não é um controle padrão
  }

  // Inicia com todos os bits em 1 (não pressionados)
  uint8_t state = 0xFF;

  // Configura os bits para cada botão (0 = pressionado)
  if (up)
    state &= ~SMS_BUTTON_UP;
  if (down)
    state &= ~SMS_BUTTON_DOWN;
  if (left)
    state &= ~SMS_BUTTON_LEFT;
  if (right)
    state &= ~SMS_BUTTON_RIGHT;
  if (button1)
    state &= ~SMS_BUTTON1;
  if (button2)
    state &= ~SMS_BUTTON2;

  // Atualiza o estado do controle
  peripherals->controller_state[port] = state;

  LOG_TRACE("Controle atualizado, porta=%d, estado=0x%02X", port, state);
}

void sms_peripherals_update_lightphaser(sms_peripherals_t *peripherals,
                                        uint16_t x, uint16_t y, bool trigger) {
  if (!peripherals || !peripherals->lightphaser) {
    return;
  }

  sms_lightphaser_update(peripherals->lightphaser, x, y, trigger);
}

void sms_peripherals_update_paddle(sms_peripherals_t *peripherals,
                                   uint8_t position, bool button1,
                                   bool button2) {
  if (!peripherals || !peripherals->paddle) {
    return;
  }

  sms_paddle_update(peripherals->paddle, position, button1, button2);
}

void sms_peripherals_process_lightphaser(sms_peripherals_t *peripherals,
                                         const uint32_t *frame_buffer,
                                         uint8_t vdp_line, uint8_t h_counter) {
  if (!peripherals || !peripherals->lightphaser || !frame_buffer) {
    return;
  }

  sms_lightphaser_detect_target(peripherals->lightphaser, frame_buffer,
                                vdp_line, h_counter);
}

uint8_t sms_peripherals_read_port(sms_peripherals_t *peripherals, uint8_t port,
                                  bool th_line) {
  if (!peripherals) {
    return 0xFF; // Valor padrão quando desconectado
  }

  uint8_t port_idx;
  sms_peripheral_type_t type;

  // Identifica a porta e o tipo de periférico
  if (port == SMS_PORT1_ADDRESS) {
    port_idx = 0;
    type = peripherals->port1_type;
  } else if (port == SMS_PORT2_ADDRESS) {
    port_idx = 1;
    type = peripherals->port2_type;
  } else {
    // Porta desconhecida
    return 0xFF;
  }

  // Lê o valor conforme o tipo de periférico
  switch (type) {
  case SMS_PERIPHERAL_CONTROLLER:
    // Para controles padrão, retorna o estado já armazenado
    // Mas leva em conta o estado da linha TH
    if (th_line) {
      return peripherals->controller_state[port_idx] | SMS_TH_LINE;
    } else {
      return peripherals->controller_state[port_idx] & ~SMS_TH_LINE;
    }

  case SMS_PERIPHERAL_LIGHTPHASER:
    if (peripherals->lightphaser) {
      return sms_lightphaser_read_port(peripherals->lightphaser, port_idx);
    }
    break;

  case SMS_PERIPHERAL_PADDLE:
    if (peripherals->paddle) {
      return sms_paddle_read_port(peripherals->paddle, port_idx, th_line);
    }
    break;

  // Tipos não implementados
  case SMS_PERIPHERAL_SPORTPAD:
  case SMS_PERIPHERAL_KEYBOARD:
  case SMS_PERIPHERAL_NONE:
  default:
    break;
  }

  // Valor padrão para periféricos desconectados ou não suportados
  return 0xFF;
}

int sms_peripherals_register_save_state(sms_peripherals_t *peripherals,
                                        save_state_t *state) {
  if (!peripherals || !state) {
    return -1;
  }

  // Registra o estado da estrutura principal
  SAVE_STATE_REGISTER_FIELD(state, peripherals, port1_type);
  SAVE_STATE_REGISTER_FIELD(state, peripherals, port2_type);
  SAVE_STATE_REGISTER_FIELD(state, peripherals, controller_state);

  // Registra o estado dos periféricos específicos
  if (peripherals->lightphaser) {
    sms_lightphaser_register_save_state(peripherals->lightphaser, state);
  }

  if (peripherals->paddle) {
    sms_paddle_register_save_state(peripherals->paddle, state);
  }

  return 0;
}

void sms_peripherals_update_after_state_load(sms_peripherals_t *peripherals) {
  if (!peripherals) {
    return;
  }

  // Atualiza os periféricos específicos se existirem
  if (peripherals->lightphaser) {
    sms_lightphaser_update_after_state_load(peripherals->lightphaser);
  }

  if (peripherals->paddle) {
    sms_paddle_update_after_state_load(peripherals->paddle);
  }
}
