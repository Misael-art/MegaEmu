/**
 * @file sms_io.c
 * @brief Implementação do sistema de I/O do Master System
 */

#include "sms_io.h"
#include "../../../core/logging.h"
#include <stdlib.h>
#include <string.h>

// Implementações de funções

sms_io_t *sms_io_init(bool is_japan) {
  sms_io_t *io = (sms_io_t *)malloc(sizeof(sms_io_t));
  if (!io) {
    LOG_ERROR("Falha ao alocar memória para o sistema de I/O");
    return NULL;
  }

  // Inicializa a estrutura
  memset(io, 0, sizeof(sms_io_t));
  io->region_is_japan = is_japan;

  // Inicializa os periféricos
  io->peripherals = sms_peripherals_init();
  if (!io->peripherals) {
    LOG_ERROR("Falha ao inicializar periféricos");
    free(io);
    return NULL;
  }

  // Inicializa os registros de controle com valores padrão
  io->io_control = 0xE0; // Valores padrão com bits não usados definidos como 1
  io->memory_control =
      0xE0; // Valores padrão com bits não usados definidos como 1

  // Inicializa o estado da linha TH para ambas as portas
  io->th_line_status[0] = 0x40; // TH ativo (bit 6 = 1)
  io->th_line_status[1] = 0x40; // TH ativo (bit 6 = 1)

  LOG_INFO("Sistema de I/O do Master System inicializado, região: %s",
           is_japan ? "Japão" : "Exportação");

  return io;
}

void sms_io_free(sms_io_t *io) {
  if (!io) {
    return;
  }

  // Libera os periféricos
  if (io->peripherals) {
    sms_peripherals_free(io->peripherals);
    io->peripherals = NULL;
  }

  // Libera a estrutura principal
  free(io);

  LOG_INFO("Sistema de I/O do Master System liberado");
}

void sms_io_reset(sms_io_t *io) {
  if (!io) {
    return;
  }

  // Reseta os registros de controle
  io->io_control = 0xE0;
  io->memory_control = 0xE0;

  // Reseta o estado da linha TH
  io->th_line_status[0] = 0x40;
  io->th_line_status[1] = 0x40;

  // Reseta os periféricos
  if (io->peripherals) {
    sms_peripherals_reset(io->peripherals);
  }

  LOG_DEBUG("Sistema de I/O do Master System resetado");
}

void sms_io_write_port(sms_io_t *io, uint8_t port, uint8_t value) {
  if (!io) {
    return;
  }

  switch (port) {
  case SMS_IO_PORT_MEMCTRL:
    // Porta de controle de memória
    io->memory_control =
        value |
        0xE0; // Os 3 bits superiores não são usados e sempre lidos como 1
    LOG_TRACE("Escrita em porta de controle de memória: 0x%02X", value);
    break;

  case SMS_IO_PORT_IOCTRL:
    // Porta de controle de I/O
    io->io_control =
        value |
        0xE0; // Os 3 bits superiores não são usados e sempre lidos como 1

    // Atualiza o estado da linha TH para ambas as portas
    bool th_line1 = (value & 0x01) != 0; // Bit 0 controla TH para porta 1
    bool th_line2 = (value & 0x02) != 0; // Bit 1 controla TH para porta 2

    io->th_line_status[0] = th_line1 ? 0x40 : 0x00;
    io->th_line_status[1] = th_line2 ? 0x40 : 0x00;

    LOG_TRACE("Escrita em porta de controle de I/O: 0x%02X, TH1=%d, TH2=%d",
              value, th_line1, th_line2);
    break;

  // Outras portas não são processadas diretamente pelo sistema de I/O
  default:
    LOG_TRACE("Escrita em porta não processada pelo I/O: 0x%02X = 0x%02X", port,
              value);
    break;
  }
}

uint8_t sms_io_read_port(sms_io_t *io, uint8_t port) {
  if (!io) {
    return 0xFF; // Valor padrão quando inválido
  }

  uint8_t value = 0xFF; // Valor padrão para portas não implementadas

  switch (port) {
  case SMS_IO_PORT_JOYSTICK1:
  case SMS_IO_PORT_REGION:
    // Porta do controle 1 (ou porta de região que inclui o controle 1)
    if (io->peripherals) {
      // Determina o estado da linha TH
      bool th_line = (io->th_line_status[0] & 0x40) != 0;

      // Lê o estado do periférico
      value = sms_peripherals_read_port(io->peripherals, SMS_IO_PORT_JOYSTICK1,
                                        th_line);

      // Se for a porta de região, adiciona informação de região
      if (port == SMS_IO_PORT_REGION) {
        // Bit 7 indica região (0 = Japão, 1 = Exportação)
        if (!io->region_is_japan) {
          value |= 0x80; // Exportação
        }
        // O resto dos bits vem do periférico
      }
    }
    break;

  case SMS_IO_PORT_JOYSTICK2:
    // Porta do controle 2
    if (io->peripherals) {
      // Determina o estado da linha TH
      bool th_line = (io->th_line_status[1] & 0x40) != 0;

      // Lê o estado do periférico
      value = sms_peripherals_read_port(io->peripherals, SMS_IO_PORT_JOYSTICK2,
                                        th_line);
    }
    break;

  case SMS_IO_PORT_MEMCTRL:
    // Porta de controle de memória
    value = io->memory_control;
    break;

  case SMS_IO_PORT_IOCTRL:
    // Porta de controle de I/O
    value = io->io_control;
    break;

  // Outras portas não são processadas diretamente pelo sistema de I/O
  default:
    LOG_TRACE("Leitura de porta não processada pelo I/O: 0x%02X", port);
    break;
  }

  return value;
}

void sms_io_process_lightphaser(sms_io_t *io, const uint32_t *frame_buffer,
                                uint8_t vdp_line, uint8_t h_counter) {
  if (!io || !io->peripherals || !frame_buffer) {
    return;
  }

  // Delega o processamento para o sistema de periféricos
  sms_peripherals_process_lightphaser(io->peripherals, frame_buffer, vdp_line,
                                      h_counter);
}

void sms_io_update_controller(sms_io_t *io, uint8_t port, bool up, bool down,
                              bool left, bool right, bool button1,
                              bool button2) {
  if (!io || !io->peripherals || port > 1) {
    return;
  }

  // Delega a atualização para o sistema de periféricos
  sms_peripherals_update_controller(io->peripherals, port, up, down, left,
                                    right, button1, button2);
}

void sms_io_update_lightphaser(sms_io_t *io, uint16_t x, uint16_t y,
                               bool trigger) {
  if (!io || !io->peripherals) {
    return;
  }

  // Delega a atualização para o sistema de periféricos
  sms_peripherals_update_lightphaser(io->peripherals, x, y, trigger);
}

void sms_io_update_paddle(sms_io_t *io, uint8_t position, bool button1,
                          bool button2) {
  if (!io || !io->peripherals) {
    return;
  }

  // Delega a atualização para o sistema de periféricos
  sms_peripherals_update_paddle(io->peripherals, position, button1, button2);
}

int sms_io_register_save_state(sms_io_t *io, save_state_t *state) {
  if (!io || !state) {
    return -1;
  }

  // Registra os campos da estrutura principal
  SAVE_STATE_REGISTER_FIELD(state, io, io_control);
  SAVE_STATE_REGISTER_FIELD(state, io, memory_control);
  SAVE_STATE_REGISTER_FIELD(state, io, region_is_japan);
  SAVE_STATE_REGISTER_FIELD(state, io, th_line_status);

  // Registra o estado dos periféricos
  if (io->peripherals) {
    sms_peripherals_register_save_state(io->peripherals, state);
  }

  return 0;
}

void sms_io_update_after_state_load(sms_io_t *io) {
  if (!io) {
    return;
  }

  // Atualiza os periféricos
  if (io->peripherals) {
    sms_peripherals_update_after_state_load(io->peripherals);
  }
}

int sms_io_connect_peripheral(sms_io_t *io, uint8_t port,
                              sms_peripheral_type_t type) {
  if (!io || !io->peripherals || port > 1) {
    return -1;
  }

  // Delega a conexão para o sistema de periféricos
  return sms_peripherals_connect(io->peripherals, port, type);
}

void sms_io_disconnect_peripheral(sms_io_t *io, uint8_t port) {
  if (!io || !io->peripherals || port > 1) {
    return;
  }

  // Delega a desconexão para o sistema de periféricos
  sms_peripherals_disconnect(io->peripherals, port);
}
