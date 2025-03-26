/**
 * @file cpu_interface.h
 * @brief Interface padrão para CPUs no Mega_Emu
 * @version 2.0
 *
 * Esta interface DEVE ser implementada por todos os adaptadores de CPU.
 * Parte da Fase 1 do plano de migração.
 */

#ifndef EMU_CPU_INTERFACE_H
#define EMU_CPU_INTERFACE_H

#include "core_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flags de status da CPU
 */
typedef enum {
  EMU_CPU_FLAG_NONE = 0x00,
  EMU_CPU_FLAG_RUNNING = 0x01,
  EMU_CPU_FLAG_HALTED = 0x02,
  EMU_CPU_FLAG_IRQ = 0x04,
  EMU_CPU_FLAG_NMI = 0x08,
  EMU_CPU_FLAG_RESET = 0x10
} emu_cpu_flags_t;

/**
 * @brief Estado da CPU
 */
typedef struct {
  uint32_t cycles;        // Ciclos executados
  uint32_t target_cycles; // Ciclos alvo para execução
  emu_cpu_flags_t flags;  // Flags de status
  void *context;          // Contexto específico da CPU
} emu_cpu_state_t;

/**
 * @brief Interface padrão para CPUs
 */
typedef struct {
  void *context; // Contexto da implementação

  // Funções de ciclo de vida
  int (*init)(void *ctx);
  void (*reset)(void *ctx);
  void (*shutdown)(void *ctx);

  // Funções de execução
  int (*execute)(void *ctx, int cycles);
  void (*halt)(void *ctx);
  void (*resume)(void *ctx);

  // Funções de memória
  uint8_t (*read_byte)(void *ctx, uint32_t addr);
  void (*write_byte)(void *ctx, uint32_t addr, uint8_t val);
  uint16_t (*read_word)(void *ctx, uint32_t addr);
  void (*write_word)(void *ctx, uint32_t addr, uint16_t val);

  // Funções de interrupção
  void (*set_irq)(void *ctx, int level);
  void (*clear_irq)(void *ctx, int level);
  void (*trigger_nmi)(void *ctx);

  // Funções de estado
  void (*get_state)(void *ctx, emu_cpu_state_t *state);
  void (*set_state)(void *ctx, const emu_cpu_state_t *state);

  // Funções de debug
  uint32_t (*get_register)(void *ctx, int reg);
  void (*set_register)(void *ctx, int reg, uint32_t value);
  const char *(*get_register_name)(void *ctx, int reg);
  int (*disassemble)(void *ctx, uint32_t addr, char *buffer, int size);
} emu_cpu_interface_t;

/**
 * @brief Cria uma nova instância da interface CPU
 * @param type Tipo de CPU a ser criada
 * @return Ponteiro para a interface ou NULL em caso de erro
 */
emu_cpu_interface_t *emu_cpu_create(int type);

/**
 * @brief Destrói uma instância da interface CPU
 * @param cpu Ponteiro para a interface
 */
void emu_cpu_destroy(emu_cpu_interface_t *cpu);

#ifdef __cplusplus
}
#endif

#endif // EMU_CPU_INTERFACE_H
