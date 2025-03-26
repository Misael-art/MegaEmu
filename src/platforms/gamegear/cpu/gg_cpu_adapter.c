/**
 * @file gg_cpu_adapter.c
 * @brief Implementação do adaptador de CPU do Game Gear
 */

#include "gg_cpu_adapter.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../audio/gg_psg.h"
#include "../memory/gg_memory.h"
#include "../video/gg_vdp.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG_CPU EMU_LOG_CAT_CPU

// Macros de log
#define GG_CPU_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG_CPU, __VA_ARGS__)
#define GG_CPU_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG_CPU, __VA_ARGS__)
#define GG_CPU_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG_CPU, __VA_ARGS__)
#define GG_CPU_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG_CPU, __VA_ARGS__)
#define GG_CPU_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG_CPU, __VA_ARGS__)

// Funções de callback para a CPU Z80
static uint8_t cpu_read(void *ctx, uint16_t addr);
static void cpu_write(void *ctx, uint16_t addr, uint8_t value);
static uint8_t cpu_port_read(void *ctx, uint16_t port);
static void cpu_port_write(void *ctx, uint16_t port, uint8_t value);

/**
 * @brief Estrutura do adaptador de CPU do Game Gear
 */
struct gg_cpu_adapter_t {
  z80_t *cpu;          // Instância da CPU Z80
  gg_memory_t *memory; // Sistema de memória
  gg_vdp_t *vdp;       // VDP
  gg_psg_t *psg;       // PSG
  uint64_t cycles;     // Contador de ciclos
};

/**
 * @brief Cria uma nova instância do adaptador de CPU
 */
gg_cpu_adapter_t *gg_cpu_adapter_create(gg_memory_t *memory, gg_vdp_t *vdp,
                                        gg_psg_t *psg) {
  if (!memory || !vdp || !psg) {
    GG_CPU_LOG_ERROR("Parâmetros inválidos para criar adaptador de CPU");
    return NULL;
  }

  gg_cpu_adapter_t *adapter =
      (gg_cpu_adapter_t *)malloc(sizeof(gg_cpu_adapter_t));
  if (!adapter) {
    GG_CPU_LOG_ERROR("Falha ao alocar memória para adaptador de CPU");
    return NULL;
  }

  // Inicializa estrutura
  adapter->memory = memory;
  adapter->vdp = vdp;
  adapter->psg = psg;
  adapter->cycles = 0;

  // Cria instância da CPU Z80
  z80_config_t config = {.read = cpu_read,
                         .write = cpu_write,
                         .port_read = cpu_port_read,
                         .port_write = cpu_port_write,
                         .ctx = adapter};

  adapter->cpu = z80_create(&config);
  if (!adapter->cpu) {
    GG_CPU_LOG_ERROR("Falha ao criar CPU Z80");
    free(adapter);
    return NULL;
  }

  GG_CPU_LOG_INFO("Adaptador de CPU do Game Gear criado");
  return adapter;
}

/**
 * @brief Destrói uma instância do adaptador de CPU
 */
void gg_cpu_adapter_destroy(gg_cpu_adapter_t *adapter) {
  if (!adapter)
    return;

  if (adapter->cpu) {
    z80_destroy(adapter->cpu);
  }

  free(adapter);
  GG_CPU_LOG_INFO("Adaptador de CPU do Game Gear destruído");
}

/**
 * @brief Reseta o adaptador de CPU
 */
void gg_cpu_adapter_reset(gg_cpu_adapter_t *adapter) {
  if (!adapter)
    return;

  z80_reset(adapter->cpu);
  adapter->cycles = 0;

  GG_CPU_LOG_INFO("Adaptador de CPU do Game Gear resetado");
}

/**
 * @brief Executa um número específico de ciclos da CPU
 */
uint32_t gg_cpu_adapter_run(gg_cpu_adapter_t *adapter, uint32_t cycles) {
  if (!adapter)
    return 0;

  uint32_t executed = z80_run(adapter->cpu, cycles);
  adapter->cycles += executed;

  return executed;
}

/**
 * @brief Obtém o número de ciclos executados desde o último reset
 */
uint64_t gg_cpu_adapter_get_cycles(const gg_cpu_adapter_t *adapter) {
  return adapter ? adapter->cycles : 0;
}

/**
 * @brief Verifica se a CPU está em estado de halt
 */
bool gg_cpu_adapter_is_halted(const gg_cpu_adapter_t *adapter) {
  return adapter ? z80_is_halted(adapter->cpu) : false;
}

/**
 * @brief Dispara uma interrupção na CPU
 */
void gg_cpu_adapter_trigger_interrupt(gg_cpu_adapter_t *adapter) {
  if (!adapter)
    return;
  z80_trigger_interrupt(adapter->cpu, 0x00);
}

/**
 * @brief Registra campos do adaptador de CPU no sistema de save state
 */
int gg_cpu_adapter_register_save_state(gg_cpu_adapter_t *adapter,
                                       save_state_t *state) {
  if (!adapter || !state)
    return -1;

  // Registra estado da CPU Z80
  if (z80_register_save_state(adapter->cpu, state) != 0) {
    return -1;
  }

  // Registra contador de ciclos
  save_state_register_field(state, "gg_cpu_cycles", &adapter->cycles,
                            sizeof(adapter->cycles));

  return 0;
}

/**
 * @brief Lê um byte da memória
 */
static uint8_t cpu_read(void *ctx, uint16_t addr) {
  gg_cpu_adapter_t *adapter = (gg_cpu_adapter_t *)ctx;
  return gg_memory_read(adapter->memory, addr);
}

/**
 * @brief Escreve um byte na memória
 */
static void cpu_write(void *ctx, uint16_t addr, uint8_t value) {
  gg_cpu_adapter_t *adapter = (gg_cpu_adapter_t *)ctx;
  gg_memory_write(adapter->memory, addr, value);
}

/**
 * @brief Lê um byte de uma porta de I/O
 */
static uint8_t cpu_port_read(void *ctx, uint16_t port) {
  gg_cpu_adapter_t *adapter = (gg_cpu_adapter_t *)ctx;
  uint8_t value = 0xFF;

  // Portas do VDP: 0x7E (V Counter), 0x7F (H Counter), 0xBE (Data), 0xBF
  // (Status)
  switch (port & 0xFF) {
  case 0x7E: // V Counter
    // TODO: Implementar leitura do V Counter
    break;

  case 0x7F: // H Counter
    // TODO: Implementar leitura do H Counter
    break;

  case 0xBE: // VDP Data
    // TODO: Implementar leitura de dados do VDP
    break;

  case 0xBF: // VDP Status
    // TODO: Implementar leitura de status do VDP
    break;

  case 0xDC: // Porta de I/O 1 (Start, D-Pad)
  case 0xC0: // Porta de I/O 2 (Botões 1 e 2)
    // TODO: Implementar leitura dos controles
    break;
  }

  return value;
}

/**
 * @brief Escreve um byte em uma porta de I/O
 */
static void cpu_port_write(void *ctx, uint16_t port, uint8_t value) {
  gg_cpu_adapter_t *adapter = (gg_cpu_adapter_t *)ctx;

  // Portas do VDP: 0xBE (Data), 0xBF (Control)
  switch (port & 0xFF) {
  case 0xBE: // VDP Data
    // TODO: Implementar escrita de dados no VDP
    break;

  case 0xBF: // VDP Control
    // TODO: Implementar escrita de controle no VDP
    break;

  case 0x7E:
  case 0x7F: // PSG
    gg_psg_write(adapter->psg, value);
    break;
  }
}
