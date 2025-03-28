#ifndef MD_M68K_EXCEPTIONS_H
#define MD_M68K_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Tipos de exceção
typedef enum {
  M68K_EXCEPTION_RESET = 0,
  M68K_EXCEPTION_BUS_ERROR = 2,
  M68K_EXCEPTION_ADDRESS_ERROR = 3,
  M68K_EXCEPTION_ILLEGAL_INSTRUCTION = 4,
  M68K_EXCEPTION_ZERO_DIVIDE = 5,
  M68K_EXCEPTION_CHK = 6,
  M68K_EXCEPTION_TRAPV = 7,
  M68K_EXCEPTION_PRIVILEGE_VIOLATION = 8,
  M68K_EXCEPTION_TRACE = 9,
  M68K_EXCEPTION_LINE_1010 = 10,
  M68K_EXCEPTION_LINE_1111 = 11,
  M68K_EXCEPTION_FORMAT_ERROR = 14,
  M68K_EXCEPTION_UNINITIALIZED_INTERRUPT = 15,
  M68K_EXCEPTION_SPURIOUS_INTERRUPT = 24,
  M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_1 = 25,
  M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_2 = 26,
  M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_3 = 27,
  M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_4 = 28,
  M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_5 = 29,
  M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_6 = 30,
  M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_7 = 31,
  M68K_EXCEPTION_TRAP_0 = 32,
  M68K_EXCEPTION_TRAP_15 = 47
} md_m68k_exception_t;

// Prioridades de exceção com timing preciso
typedef enum {
  M68K_PRIORITY_RESET = 7,
  M68K_PRIORITY_BUS_ERROR = 6,
  M68K_PRIORITY_ADDRESS_ERROR = 6,
  M68K_PRIORITY_ILLEGAL_INSTRUCTION = 6,
  M68K_PRIORITY_ZERO_DIVIDE = 6,
  M68K_PRIORITY_CHK = 6,
  M68K_PRIORITY_TRAPV = 6,
  M68K_PRIORITY_PRIVILEGE_VIOLATION = 6,
  M68K_PRIORITY_TRACE = 6,
  M68K_PRIORITY_LINE_1010 = 6,
  M68K_PRIORITY_LINE_1111 = 6,
  M68K_PRIORITY_FORMAT_ERROR = 6,
  M68K_PRIORITY_UNINITIALIZED_INTERRUPT = 6,
  M68K_PRIORITY_SPURIOUS_INTERRUPT = 5,
  M68K_PRIORITY_AUTOVECTOR_1 = 1,
  M68K_PRIORITY_AUTOVECTOR_2 = 2,
  M68K_PRIORITY_AUTOVECTOR_3 = 3,
  M68K_PRIORITY_AUTOVECTOR_4 = 4,
  M68K_PRIORITY_AUTOVECTOR_5 = 5,
  M68K_PRIORITY_AUTOVECTOR_6 = 6,
  M68K_PRIORITY_AUTOVECTOR_7 = 7,
  M68K_PRIORITY_TRAP = 6
} md_m68k_priority_t;

// Estrutura para timing preciso de exceções
typedef struct {
  uint32_t cycles_to_acknowledge; // Ciclos até reconhecimento
  uint32_t cycles_to_process;     // Ciclos para processar
  uint32_t cycles_stack_push;     // Ciclos para push na stack
  uint32_t cycles_vector_fetch;   // Ciclos para buscar vetor
} md_m68k_exception_timing_t;

// Estrutura para informações da exceção
typedef struct {
  md_m68k_exception_t type;
  md_m68k_priority_t priority;
  uint32_t address;
  uint16_t status_register;
  uint32_t data;
  md_m68k_exception_timing_t timing;
  uint32_t instruction_address; // Endereço da instrução que causou a exceção
  uint16_t instruction_opcode;  // Opcode da instrução
  uint8_t group_priority;       // Prioridade dentro do grupo
  uint8_t is_reentrant;         // Flag para exceções reentrantes
} md_m68k_exception_info_t;

// Tipo de callback para handler de exceção
typedef void (*md_m68k_exception_handler_t)(
    const md_m68k_exception_info_t *info);

// Funções de gerenciamento de exceções
void md_m68k_init_exceptions(void);
void md_m68k_set_exception_handler(md_m68k_exception_t type,
                                   md_m68k_exception_handler_t handler);
void md_m68k_raise_exception(md_m68k_exception_t type, uint32_t address,
                             uint32_t data);
void md_m68k_set_interrupt_mask(uint8_t mask);
uint8_t md_m68k_get_interrupt_mask(void);
int32_t md_m68k_check_interrupts(void);

// Novas funções para timing preciso
void md_m68k_set_exception_timing(md_m68k_exception_t type,
                                  const md_m68k_exception_timing_t *timing);
uint32_t md_m68k_get_exception_cycles(const md_m68k_exception_info_t *info);
void md_m68k_acknowledge_interrupt(uint8_t level);
void md_m68k_set_vector_base(uint32_t address);
uint32_t md_m68k_get_vector_base(void);

// Funções para debug e profiling
void md_m68k_get_exception_stats(uint32_t *total_exceptions,
                                 uint32_t *cycles_spent);
void md_m68k_reset_exception_stats(void);

#ifdef __cplusplus
}
#endif

#endif // MD_M68K_EXCEPTIONS_H
