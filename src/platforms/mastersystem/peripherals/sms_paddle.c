/**
 * @file sms_paddle.c
 * @brief Implementação do periférico Paddle do Master System
 */

#include "sms_paddle.h"
#include "../../../core/logging.h"
#include <stdlib.h>
#include <string.h>

// Definições de constantes
#define SMS_PADDLE_BUTTON1_BIT 0x10 // Bit 4 - botão 1
#define SMS_PADDLE_BUTTON2_BIT 0x20 // Bit 5 - botão 2
#define SMS_TH_LINE_BIT 0x40        // Bit 6 - linha TH do controlador
#define SMS_PADDLE_MASK 0xF0        // Máscara para os bits do Paddle

// Valores padrão quando não conectado
#define SMS_PADDLE_DEFAULT_STATE 0xFF // Estado padrão de entrada

/**
 * @brief Estrutura interna do Paddle
 */
struct sms_paddle_t {
  sms_paddle_state_t state; // Estado atual
  uint8_t position_latch;   // Valor retido para o contador de posição
  uint8_t counter;          // Contador de leituras para o protocolo
  bool previous_th; // Estado anterior da linha TH para detecção de borda
};

// Implementações de funções

sms_paddle_t *sms_paddle_create(sms_paddle_port_t port) {
  sms_paddle_t *paddle = (sms_paddle_t *)malloc(sizeof(sms_paddle_t));
  if (!paddle) {
    LOG_ERROR("Falha ao alocar memória para o Paddle");
    return NULL;
  }

  // Inicializa o estado
  memset(paddle, 0, sizeof(sms_paddle_t));
  paddle->state.port = (uint8_t)port;
  paddle->state.connected = true;
  paddle->state.position = 128; // Posição central padrão

  LOG_INFO("Paddle criado e conectado à porta %d", port);
  return paddle;
}

void sms_paddle_destroy(sms_paddle_t *paddle) {
  if (paddle) {
    LOG_INFO("Paddle desconectado da porta %d", paddle->state.port);
    free(paddle);
  }
}

void sms_paddle_reset(sms_paddle_t *paddle) {
  if (paddle) {
    // Mantém a porta e estado de conexão, reseta o restante
    uint8_t port = paddle->state.port;
    bool connected = paddle->state.connected;

    memset(paddle, 0, sizeof(sms_paddle_t));

    paddle->state.port = port;
    paddle->state.connected = connected;
    paddle->state.position = 128; // Posição central padrão

    LOG_DEBUG("Paddle resetado");
  }
}

void sms_paddle_update(sms_paddle_t *paddle, uint8_t position, bool button1,
                       bool button2) {
  if (!paddle || !paddle->state.connected) {
    return;
  }

  // Atualiza posição e estado dos botões
  paddle->state.position = position;
  paddle->state.button1 = button1;
  paddle->state.button2 = button2;

  LOG_TRACE("Paddle atualizado: pos=%d, btn1=%d, btn2=%d", position, button1,
            button2);
}

uint8_t sms_paddle_read_port(sms_paddle_t *paddle, uint8_t port, bool th_line) {
  if (!paddle || !paddle->state.connected || port != paddle->state.port) {
    return SMS_PADDLE_DEFAULT_STATE; // Retorna o estado padrão quando não
                                     // conectado
  }

  uint8_t port_value = 0x3F; // Bits 0-5 são para botões, inicialmente todos
                             // ativos (0 = pressionado)

  // Detecta transição na linha TH (de alto para baixo)
  if (paddle->previous_th && !th_line) {
    // Quando TH cai, atualiza o valor retido
    paddle->position_latch = paddle->state.position;
    paddle->counter = 0; // Reinicia o contador
  }
  paddle->previous_th = th_line;

  // Comportamento diferente baseado no estado da linha TH
  if (th_line) {
    // TH alto - retorna o estado dos botões
    if (paddle->state.button1) {
      port_value &= ~SMS_PADDLE_BUTTON1_BIT;
    }
    if (paddle->state.button2) {
      port_value &= ~SMS_PADDLE_BUTTON2_BIT;
    }
  } else {
    // TH baixo - retorna o valor da posição bit a bit
    // A cada leitura, retorna um bit, começando pelo menos significativo
    if (paddle->counter < 8) {
      // Verifica se o bit correspondente na posição está definido
      if (!(paddle->position_latch & (1 << paddle->counter))) {
        port_value &= ~SMS_PADDLE_BUTTON1_BIT; // Bit 0 = botão 1
      }

      // Incrementa o contador para a próxima leitura
      paddle->counter++;
    }
  }

  // Mantém o estado da linha TH no valor retornado
  if (th_line) {
    port_value |= SMS_TH_LINE_BIT;
  } else {
    port_value &= ~SMS_TH_LINE_BIT;
  }

  return port_value;
}

void sms_paddle_get_state(sms_paddle_t *paddle, sms_paddle_state_t *state) {
  if (paddle && state) {
    *state = paddle->state;
  }
}

int sms_paddle_register_save_state(sms_paddle_t *paddle, save_state_t *state) {
  if (!paddle || !state) {
    return -1;
  }

  // Registra os dados necessários para o save state
  SAVE_STATE_REGISTER_FIELD(state, paddle, state);
  SAVE_STATE_REGISTER_FIELD(state, paddle, position_latch);
  SAVE_STATE_REGISTER_FIELD(state, paddle, counter);
  SAVE_STATE_REGISTER_FIELD(state, paddle, previous_th);

  return 0;
}

void sms_paddle_update_after_state_load(sms_paddle_t *paddle) {
  // Qualquer atualização necessária após o carregamento do estado
  // Neste caso, não há ação adicional necessária
}
