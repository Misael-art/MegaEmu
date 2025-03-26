/**
 * @file gg_cpu_adapter.h
 * @brief Interface do adaptador de CPU do Game Gear
 */

#ifndef GG_CPU_ADAPTER_H
#define GG_CPU_ADAPTER_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/z80/z80.h"
#include "../../../utils/save_state.h"

// Forward declarations
typedef struct gg_memory_t gg_memory_t;
typedef struct gg_psg_t gg_psg_t;
typedef struct gg_vdp_t gg_vdp_t;
typedef struct gg_cpu_adapter_t gg_cpu_adapter_t;

/**
 * @brief Cria uma nova instância do adaptador de CPU
 * @param memory Ponteiro para o sistema de memória
 * @param vdp Ponteiro para o VDP
 * @param psg Ponteiro para o PSG
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_cpu_adapter_t *gg_cpu_adapter_create(gg_memory_t *memory, gg_vdp_t *vdp, gg_psg_t *psg);

/**
 * @brief Destrói uma instância do adaptador de CPU
 * @param adapter Ponteiro para a instância
 */
void gg_cpu_adapter_destroy(gg_cpu_adapter_t *adapter);

/**
 * @brief Reseta o adaptador de CPU
 * @param adapter Ponteiro para a instância
 */
void gg_cpu_adapter_reset(gg_cpu_adapter_t *adapter);

/**
 * @brief Executa um número específico de ciclos da CPU
 * @param adapter Ponteiro para a instância
 * @param cycles Número de ciclos a executar
 * @return Número real de ciclos executados
 */
uint32_t gg_cpu_adapter_run(gg_cpu_adapter_t *adapter, uint32_t cycles);

/**
 * @brief Obtém o número de ciclos executados desde o último reset
 * @param adapter Ponteiro para a instância
 * @return Número de ciclos executados
 */
uint64_t gg_cpu_adapter_get_cycles(const gg_cpu_adapter_t *adapter);

/**
 * @brief Verifica se a CPU está em estado de halt
 * @param adapter Ponteiro para a instância
 * @return true se a CPU está em halt, false caso contrário
 */
bool gg_cpu_adapter_is_halted(const gg_cpu_adapter_t *adapter);

/**
 * @brief Dispara uma interrupção na CPU
 * @param adapter Ponteiro para a instância
 */
void gg_cpu_adapter_trigger_interrupt(gg_cpu_adapter_t *adapter);

/**
 * @brief Registra campos do adaptador de CPU no sistema de save state
 * @param adapter Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_cpu_adapter_register_save_state(gg_cpu_adapter_t *adapter, save_state_t *state);

#endif // GG_CPU_ADAPTER_H
