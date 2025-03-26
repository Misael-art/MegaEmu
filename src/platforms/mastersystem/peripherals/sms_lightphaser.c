/**
 * @file sms_lightphaser.c
 * @brief Implementação do periférico Light Phaser do Master System
 */

#include "sms_lightphaser.h"
#include "../../../core/logging.h"
#include <stdlib.h>
#include <string.h>

#define SMS_LIGHTPHASER_TRIGGER_BIT 0x10 // Bit 4 - estado do gatilho
#define SMS_LIGHTPHASER_LIGHT_BIT 0x20   // Bit 5 - detecção de luz
#define SMS_TH_LINE_BIT 0x40             // Bit 6 - linha TH do controlador
#define SMS_LIGHTPHASER_MASK 0xF0        // Máscara para os bits do Light Phaser

// Configurações de detecção de alvo
#define TARGET_THRESHOLD 0x80 // Limiar de brilho para detecção de luz
#define TARGET_SCAN_RADIUS 2  // Raio de escaneamento em pixels

// Valores padrão quando não conectado
#define SMS_LIGHTPHASER_DEFAULT_STATE 0xFF // Estado padrão de entrada

/**
 * @brief Estrutura interna do Light Phaser
 */
struct sms_lightphaser_t {
  sms_lightphaser_state_t state; // Estado atual
  uint32_t last_frame_time;      // Último tempo de frame para debounce
  uint8_t debounce_count;        // Contador para debounce do gatilho
  bool light_detected;           // Flag de detecção de luz atual
};

// Implementações de funções

sms_lightphaser_t *sms_lightphaser_create(sms_lightphaser_port_t port) {
  sms_lightphaser_t *lightphaser =
      (sms_lightphaser_t *)malloc(sizeof(sms_lightphaser_t));
  if (!lightphaser) {
    LOG_ERROR("Falha ao alocar memória para o Light Phaser");
    return NULL;
  }

  // Inicializa o estado
  memset(lightphaser, 0, sizeof(sms_lightphaser_t));
  lightphaser->state.port = (uint8_t)port;
  lightphaser->state.connected = true;
  lightphaser->state.x = 128; // Posição central padrão
  lightphaser->state.y = 96;

  LOG_INFO("Light Phaser criado e conectado à porta %d", port);
  return lightphaser;
}

void sms_lightphaser_destroy(sms_lightphaser_t *lightphaser) {
  if (lightphaser) {
    LOG_INFO("Light Phaser desconectado da porta %d", lightphaser->state.port);
    free(lightphaser);
  }
}

void sms_lightphaser_reset(sms_lightphaser_t *lightphaser) {
  if (lightphaser) {
    // Mantém a porta e estado de conexão, reseta o restante
    uint8_t port = lightphaser->state.port;
    bool connected = lightphaser->state.connected;

    memset(lightphaser, 0, sizeof(sms_lightphaser_t));

    lightphaser->state.port = port;
    lightphaser->state.connected = connected;
    lightphaser->state.x = 128; // Posição central padrão
    lightphaser->state.y = 96;

    LOG_DEBUG("Light Phaser resetado");
  }
}

void sms_lightphaser_update(sms_lightphaser_t *lightphaser, uint16_t x,
                            uint16_t y, bool trigger) {
  if (!lightphaser || !lightphaser->state.connected) {
    return;
  }

  // Atualiza posição e estado do gatilho
  lightphaser->state.x = x;
  lightphaser->state.y = y;
  lightphaser->state.trigger = trigger;

  LOG_TRACE("Light Phaser atualizado: pos=(%d,%d), trigger=%d", x, y, trigger);
}

bool sms_lightphaser_detect_target(sms_lightphaser_t *lightphaser,
                                   const uint32_t *frame_buffer,
                                   uint8_t vdp_line, uint8_t h_counter) {
  if (!lightphaser || !lightphaser->state.connected || !frame_buffer) {
    return false;
  }

  // Verifica se a posição do cursor está na linha atual do VDP
  if (lightphaser->state.y != vdp_line) {
    return false;
  }

  // Verifica a posição horizontal - implementação simplificada do tempo
  // Na implementação real, seria necessário mapear h_counter para a posição X
  if (h_counter > 0 && abs((int)lightphaser->state.x - (int)h_counter) > 8) {
    return false;
  }

  // Verifica o pixel na posição do cursor e na vizinhança
  bool light_detected = false;
  int width = 256; // Largura padrão do frame buffer
  int x = lightphaser->state.x;
  int y = lightphaser->state.y;

  // Escaneamento em uma pequena área ao redor do ponto alvo
  for (int dy = -TARGET_SCAN_RADIUS;
       dy <= TARGET_SCAN_RADIUS && !light_detected; dy++) {
    int scan_y = y + dy;
    if (scan_y < 0 || scan_y >= 192)
      continue; // Fora da tela

    for (int dx = -TARGET_SCAN_RADIUS;
         dx <= TARGET_SCAN_RADIUS && !light_detected; dx++) {
      int scan_x = x + dx;
      if (scan_x < 0 || scan_x >= width)
        continue; // Fora da tela

      // Obtém a cor do pixel na posição (scan_x, scan_y)
      uint32_t pixel = frame_buffer[scan_y * width + scan_x];

      // Extrai os componentes RGB para calcular o brilho
      uint8_t r = (pixel >> 16) & 0xFF;
      uint8_t g = (pixel >> 8) & 0xFF;
      uint8_t b = pixel & 0xFF;

      // Cálculo de luminância aproximado: 0.299R + 0.587G + 0.114B
      uint16_t brightness = (299 * r + 587 * g + 114 * b) / 1000;

      // Verifica se o brilho está acima do limiar
      if (brightness > TARGET_THRESHOLD) {
        light_detected = true;
        break;
      }
    }
  }

  // Armazena o resultado da detecção
  lightphaser->light_detected = light_detected;

  if (light_detected) {
    LOG_TRACE("Light Phaser detectou alvo na linha %d, posição %d", vdp_line,
              x);
  }

  return light_detected;
}

uint8_t sms_lightphaser_read_port(sms_lightphaser_t *lightphaser,
                                  uint8_t port) {
  if (!lightphaser || !lightphaser->state.connected ||
      port != lightphaser->state.port) {
    return SMS_LIGHTPHASER_DEFAULT_STATE; // Retorna o estado padrão quando não
                                          // conectado
  }

  uint8_t port_value = 0x3F; // Bits 0-5 são para botões, inicialmente todos
                             // ativos (0 = pressionado)

  // Configura o bit de gatilho (0 quando pressionado)
  if (!lightphaser->state.trigger) {
    port_value &= ~SMS_LIGHTPHASER_TRIGGER_BIT;
  }

  // Configura o bit de detecção de luz (0 quando luz detectada)
  if (lightphaser->light_detected) {
    port_value &= ~SMS_LIGHTPHASER_LIGHT_BIT;
  }

  // Adiciona o bit TH conforme necessário para o protocolo
  if (port_value & SMS_TH_LINE_BIT) {
    // TH ativo - retorna o estado do gatilho/luz
    return port_value;
  } else {
    // TH inativo - retorna sempre 1s para os bits da pistola
    return (port_value & 0x3F) | SMS_LIGHTPHASER_MASK;
  }
}

void sms_lightphaser_get_state(sms_lightphaser_t *lightphaser,
                               sms_lightphaser_state_t *state) {
  if (lightphaser && state) {
    *state = lightphaser->state;
  }
}

int sms_lightphaser_register_save_state(sms_lightphaser_t *lightphaser,
                                        save_state_t *state) {
  if (!lightphaser || !state) {
    return -1;
  }

  // Registra os dados necessários para o save state
  SAVE_STATE_REGISTER_FIELD(state, lightphaser, state);
  SAVE_STATE_REGISTER_FIELD(state, lightphaser, last_frame_time);
  SAVE_STATE_REGISTER_FIELD(state, lightphaser, debounce_count);
  SAVE_STATE_REGISTER_FIELD(state, lightphaser, light_detected);

  return 0;
}

void sms_lightphaser_update_after_state_load(sms_lightphaser_t *lightphaser) {
  // Qualquer atualização necessária após o carregamento do estado
  // Neste caso, não há ação adicional necessária
}
