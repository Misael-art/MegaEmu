/**
 * @file z80_adapter.c
 * @brief Adaptador de memória para o Z80 no Master System
 */

#include "z80_adapter.h"
#include "../../../core/cpu/z80/z80.h"
#include "../../../core/cpu/z80/z80_instructions.h"
#include "../../../core/save_state.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_Z80 EMU_LOG_CAT_MASTERSYSTEM

// Macros de log
#define SMS_Z80_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_Z80, __VA_ARGS__)

// Portas de I/O padrão
#define SMS_IO_MEMORY_CONTROL 0x3E
#define SMS_IO_MAPPER_CONTROL 0x3F
#define SMS_IO_VDP_DATA 0xBE
#define SMS_IO_VDP_CONTROL 0xBF
#define SMS_IO_VDP_VCOUNT 0x7E
#define SMS_IO_VDP_HCOUNT 0x7F
#define SMS_IO_PSG 0x7F
#define SMS_IO_INPUT_PORT1 0xDC
#define SMS_IO_INPUT_PORT2 0xDD

/**
 * @brief Estrutura do adaptador Z80
 */
struct sms_z80_adapter_t {
  // Componentes conectados
  sms_memory_t *memory; // Sistema de memória
  sms_vdp_t *vdp;       // VDP
  sms_psg_t *psg;       // PSG
  sms_input_t *input;   // Sistema de entrada

  // Estado do Z80
  uint8_t interrupt_enable; // Estado de interrupções
  uint8_t interrupt_vector; // Vetor de interrupção atual
  bool nmi_pending;         // NMI pendente
  bool irq_pending;         // IRQ pendente

  // Cache de estado
  uint8_t last_vdp_status;  // Último status do VDP lido
  uint8_t last_input_state; // Último estado de entrada lido

  // Contadores
  uint32_t cycle_count;     // Contador de ciclos
  uint32_t cycles_per_line; // Ciclos por linha
  uint32_t lines_per_frame; // Linhas por frame
};

/**
 * @brief Cria uma nova instância do adaptador Z80
 */
sms_z80_adapter_t *sms_z80_adapter_create(void) {
  sms_z80_adapter_t *adapter =
      (sms_z80_adapter_t *)malloc(sizeof(sms_z80_adapter_t));
  if (!adapter) {
    SMS_Z80_LOG_ERROR("Falha ao alocar memória para o adaptador Z80");
    return NULL;
  }

  // Inicializa a estrutura
  memset(adapter, 0, sizeof(sms_z80_adapter_t));

  // Configura parâmetros padrão
  adapter->cycles_per_line = 228; // Padrão NTSC
  adapter->lines_per_frame = 262; // Padrão NTSC

  SMS_Z80_LOG_INFO("Adaptador Z80 criado com sucesso");
  return adapter;
}

/**
 * @brief Destrói uma instância do adaptador Z80
 */
void sms_z80_adapter_destroy(sms_z80_adapter_t *adapter) {
  if (!adapter)
    return;

  free(adapter);
  SMS_Z80_LOG_INFO("Adaptador Z80 destruído");
}

/**
 * @brief Reseta o adaptador Z80
 */
void sms_z80_adapter_reset(sms_z80_adapter_t *adapter) {
  if (!adapter)
    return;

  adapter->interrupt_enable = 0;
  adapter->interrupt_vector = 0;
  adapter->nmi_pending = false;
  adapter->irq_pending = false;
  adapter->cycle_count = 0;

  SMS_Z80_LOG_INFO("Adaptador Z80 resetado");
}

/**
 * @brief Conecta os componentes ao adaptador
 */
void sms_z80_adapter_connect(sms_z80_adapter_t *adapter, sms_memory_t *memory,
                             sms_vdp_t *vdp, sms_psg_t *psg,
                             sms_input_t *input) {
  if (!adapter)
    return;

  adapter->memory = memory;
  adapter->vdp = vdp;
  adapter->psg = psg;
  adapter->input = input;

  SMS_Z80_LOG_DEBUG("Componentes conectados ao adaptador Z80");
}

/**
 * @brief Lê um byte da memória
 */
uint8_t sms_z80_adapter_read(sms_z80_adapter_t *adapter, uint16_t address) {
  if (!adapter || !adapter->memory)
    return 0xFF;

  return sms_memory_read(adapter->memory, address);
}

/**
 * @brief Escreve um byte na memória
 */
void sms_z80_adapter_write(sms_z80_adapter_t *adapter, uint16_t address,
                           uint8_t value) {
  if (!adapter || !adapter->memory)
    return;

  sms_memory_write(adapter->memory, address, value);
}

/**
 * @brief Lê uma porta de I/O
 */
uint8_t sms_z80_adapter_port_read(sms_z80_adapter_t *adapter, uint16_t port) {
  if (!adapter)
    return 0xFF;

  // Máscara de porta (SMS usa apenas 8 bits)
  port &= 0xFF;

  switch (port) {
  case SMS_IO_VDP_DATA:
    return adapter->vdp ? sms_vdp_read_data(adapter->vdp) : 0xFF;

  case SMS_IO_VDP_CONTROL:
    return adapter->vdp ? sms_vdp_read_status(adapter->vdp) : 0xFF;

  case SMS_IO_VDP_VCOUNT:
    return adapter->vdp ? sms_vdp_get_vcount(adapter->vdp) : 0xFF;

  case SMS_IO_VDP_HCOUNT:
    return adapter->vdp ? sms_vdp_get_hcount(adapter->vdp) : 0xFF;

  case SMS_IO_INPUT_PORT1:
  case SMS_IO_INPUT_PORT2:
    return adapter->input ? sms_input_read_port(adapter->input, port) : 0xFF;

  default:
    SMS_Z80_LOG_TRACE("Leitura de porta não mapeada: 0x%04X", port);
    return 0xFF;
  }
}

/**
 * @brief Escreve em uma porta de I/O
 */
void sms_z80_adapter_port_write(sms_z80_adapter_t *adapter, uint16_t port,
                                uint8_t value) {
  if (!adapter)
    return;

  // Máscara de porta (SMS usa apenas 8 bits)
  port &= 0xFF;

  switch (port) {
  case SMS_IO_MEMORY_CONTROL:
    if (adapter->memory) {
      sms_memory_control_write(adapter->memory, value);
    }
    break;

  case SMS_IO_MAPPER_CONTROL:
    if (adapter->memory) {
      sms_memory_mapper_write(adapter->memory, value);
    }
    break;

  case SMS_IO_VDP_DATA:
    if (adapter->vdp) {
      sms_vdp_write_data(adapter->vdp, value);
    }
    break;

  case SMS_IO_VDP_CONTROL:
    if (adapter->vdp) {
      sms_vdp_write_control(adapter->vdp, value);
    }
    break;

  case SMS_IO_PSG:
    if (adapter->psg) {
      sms_psg_write(adapter->psg, value);
    }
    break;

  default:
    SMS_Z80_LOG_TRACE("Escrita em porta não mapeada: 0x%04X = 0x%02X", port,
                      value);
    break;
  }
}

/**
 * @brief Verifica e processa interrupções
 */
bool sms_z80_adapter_check_interrupt(sms_z80_adapter_t *adapter) {
  if (!adapter || !adapter->vdp)
    return false;

  // Verifica NMI primeiro (maior prioridade)
  if (adapter->nmi_pending) {
    adapter->nmi_pending = false;
    return true;
  }

  // Verifica IRQ do VDP
  if (adapter->interrupt_enable && adapter->irq_pending) {
    if (sms_vdp_check_interrupt(adapter->vdp)) {
      adapter->irq_pending = false;
      return true;
    }
  }

  return false;
}

/**
 * @brief Atualiza o estado do adaptador
 */
void sms_z80_adapter_update(sms_z80_adapter_t *adapter, uint8_t cycles) {
  if (!adapter)
    return;

  adapter->cycle_count += cycles;

  // Verifica se completou uma linha
  if (adapter->cycle_count >= adapter->cycles_per_line) {
    adapter->cycle_count -= adapter->cycles_per_line;

    // Atualiza VDP
    if (adapter->vdp) {
      sms_vdp_update_line(adapter->vdp);
    }
  }
}

/**
 * @brief Define o modo de timing (NTSC/PAL)
 */
void sms_z80_adapter_set_timing(sms_z80_adapter_t *adapter, bool is_pal) {
  if (!adapter)
    return;

  if (is_pal) {
    adapter->cycles_per_line = 228;
    adapter->lines_per_frame = 313;
    SMS_Z80_LOG_INFO("Timing definido para PAL");
  } else {
    adapter->cycles_per_line = 228;
    adapter->lines_per_frame = 262;
    SMS_Z80_LOG_INFO("Timing definido para NTSC");
  }
}

/**
 * @brief Registra o estado no sistema de save state
 */
int sms_z80_adapter_register_save_state(sms_z80_adapter_t *adapter,
                                        save_state_t *state) {
  if (!adapter || !state)
    return -1;

  save_state_register_field(state, "z80_interrupt_enable",
                            &adapter->interrupt_enable,
                            sizeof(adapter->interrupt_enable));
  save_state_register_field(state, "z80_interrupt_vector",
                            &adapter->interrupt_vector,
                            sizeof(adapter->interrupt_vector));
  save_state_register_field(state, "z80_nmi_pending", &adapter->nmi_pending,
                            sizeof(adapter->nmi_pending));
  save_state_register_field(state, "z80_irq_pending", &adapter->irq_pending,
                            sizeof(adapter->irq_pending));
  save_state_register_field(state, "z80_cycle_count", &adapter->cycle_count,
                            sizeof(adapter->cycle_count));

  return 0;
}

/**
 * @brief Atualiza o estado após carregar um save state
 */
void sms_z80_adapter_update_state(sms_z80_adapter_t *adapter) {
  if (!adapter)
    return;

  SMS_Z80_LOG_INFO(
      "Estado do adaptador Z80 atualizado após carregar save state");
}

// Implementações simplificadas das funções do VDP e PSG
// Estas funções seriam substituídas por chamadas reais para os componentes
// correspondentes

/**
 * @brief Lê dados do VDP
 *
 * Esta função lê dados da porta do VDP e reseta o latch de controle.
 */
static uint8_t sms_vdp_read_data(void *vdp) {
  if (!vdp) {
    return 0xFF;
  }

  // Função externa do VDP para leitura de dados
  extern uint8_t sms_vdp_read_data_port(sms_vdp_t * vdp);
  uint8_t value = sms_vdp_read_data_port((sms_vdp_t *)vdp);

  SMS_Z80_LOG_TRACE("Leitura da porta de dados do VDP: 0x%02X", value);
  return value;
}

/**
 * @brief Lê status do VDP
 *
 * Esta função lê o registro de status do VDP e reseta o latch de controle.
 */
static uint8_t sms_vdp_read_status(void *vdp) {
  if (!vdp) {
    return 0xFF;
  }

  // Função externa do VDP para leitura de status
  extern uint8_t sms_vdp_read_status_port(sms_vdp_t * vdp);
  uint8_t value = sms_vdp_read_status_port((sms_vdp_t *)vdp);

  SMS_Z80_LOG_TRACE("Leitura da porta de status do VDP: 0x%02X", value);
  return value;
}

/**
 * @brief Escreve dados no VDP
 *
 * Esta função escreve na porta de dados do VDP.
 */
static void sms_vdp_write_data(void *vdp, uint8_t value) {
  if (!vdp) {
    return;
  }

  // Função externa do VDP para escrita de dados
  extern void sms_vdp_write_data_port(sms_vdp_t * vdp, uint8_t value);
  sms_vdp_write_data_port((sms_vdp_t *)vdp, value);

  SMS_Z80_LOG_TRACE("Escrita na porta de dados do VDP: 0x%02X", value);
}

/**
 * @brief Escreve no controle do VDP
 *
 * Esta função escreve na porta de controle do VDP, que pode ser um
 * endereço para operações subsequentes ou um valor para registrador.
 */
static void sms_vdp_write_control(void *vdp, uint8_t value) {
  if (!vdp) {
    return;
  }

  // Função externa do VDP para escrita no controle
  extern void sms_vdp_write_control_port(sms_vdp_t * vdp, uint8_t value);
  sms_vdp_write_control_port((sms_vdp_t *)vdp, value);

  SMS_Z80_LOG_TRACE("Escrita na porta de controle do VDP: 0x%02X", value);
}

/**
 * @brief Escreve no PSG (Programmable Sound Generator)
 *
 * Esta função envia um comando para o PSG.
 */
static void sms_psg_write(void *psg, uint8_t value) {
  if (!psg) {
    return;
  }

  // Função externa do PSG para escrita
  extern void sms_psg_write_port(sms_psg_t * psg, uint8_t value);
  sms_psg_write_port((sms_psg_t *)psg, value);

  SMS_Z80_LOG_TRACE("Escrita na porta do PSG: 0x%02X", value);
}
