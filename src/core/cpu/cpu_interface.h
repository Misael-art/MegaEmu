/**
 * @file cpu_interface.h
 * @brief Interface genérica para CPUs
 */
#ifndef CPU_INTERFACE_H
#define CPU_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Códigos de erro genéricos para CPUs
 */
#define CPU_ERROR_NONE 0              /**< Sem erro */
#define CPU_ERROR_INVALID_OPCODE -10  /**< Opcode inválido */
#define CPU_ERROR_INVALID_ADDRESS -11 /**< Endereço inválido */
#define CPU_ERROR_STACK_OVERFLOW -12  /**< Estouro de pilha */
#define CPU_ERROR_STACK_UNDERFLOW -13 /**< Subfluxo de pilha */

/**
 * @brief Tipos de interrupção genéricos
 */
typedef enum {
  CPU_INTERRUPT_NONE, /**< Sem interrupção */
  CPU_INTERRUPT_NMI,  /**< Interrupção Não-Mascarável */
  CPU_INTERRUPT_IRQ,  /**< Interrupção normal */
  CPU_INTERRUPT_RESET /**< Reset */
} cpu_interrupt_t;

/**
 * @brief Funções de callback para acesso à memória
 */
typedef uint8_t (*cpu_read_func_t)(void *context, uint32_t address);
typedef void (*cpu_write_func_t)(void *context, uint32_t address,
                                 uint8_t value);

/**
 * @brief Configuração genérica de CPU
 */
typedef struct {
  cpu_read_func_t read_mem;   /**< Função para leitura de memória */
  cpu_write_func_t write_mem; /**< Função para escrita de memória */
  void *context;              /**< Contexto para callbacks de memória */
  int32_t log_level;          /**< Nível de log para a CPU */
} cpu_config_t;

/**
 * @brief Estado genérico da CPU
 */
typedef struct {
  uint64_t cycles;           /**< Contador de ciclos */
  int32_t remaining_cycles;  /**< Ciclos restantes para a instrução atual */
  int32_t stall_cycles;      /**< Ciclos de stall (DMA, etc) */
  cpu_interrupt_t interrupt; /**< Interrupção pendente */
} cpu_state_t;

/**
 * @brief Interface genérica de CPU
 */
typedef struct {
  void *context; /**< Contexto específico da CPU */

  // Funções de controle
  int32_t (*init)(void *ctx, const cpu_config_t *config);
  void (*shutdown)(void *ctx);
  void (*reset)(void *ctx);
  int32_t (*execute)(void *ctx, int32_t cycles);

  // Funções de estado
  void (*get_state)(void *ctx, cpu_state_t *state);
  void (*set_state)(void *ctx, const cpu_state_t *state);

  // Funções de interrupção
  void (*trigger_interrupt)(void *ctx, cpu_interrupt_t interrupt);
  void (*add_stall_cycles)(void *ctx, int32_t cycles);

  // Funções de registro
  uint32_t (*get_register)(void *ctx, const char *reg);
  void (*set_register)(void *ctx, const char *reg, uint32_t value);

  // Funções de debug
  int32_t (*dump_state)(void *ctx, char *buffer, int32_t buffer_size);
} cpu_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* CPU_INTERFACE_H */
