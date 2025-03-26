#ifndef EMU_CPU_6502_H
#define EMU_CPU_6502_H

#include "core/interfaces/cpu_interface.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registradores da CPU 6502
 */
typedef enum {
  EMU_6502_REG_A = 0, // Acumulador
  EMU_6502_REG_X,     // Índice X
  EMU_6502_REG_Y,     // Índice Y
  EMU_6502_REG_SP,    // Stack Pointer
  EMU_6502_REG_PC,    // Program Counter
  EMU_6502_REG_P,     // Status Register (Flags)
  EMU_6502_REG_COUNT
} emu_6502_registers_t;

/**
 * @brief Flags do registrador de status da CPU 6502
 */
typedef enum {
  EMU_6502_FLAG_C = 0x01, // Carry
  EMU_6502_FLAG_Z = 0x02, // Zero
  EMU_6502_FLAG_I = 0x04, // Interrupt Disable
  EMU_6502_FLAG_D = 0x08, // Decimal Mode
  EMU_6502_FLAG_B = 0x10, // Break Command
  EMU_6502_FLAG_V = 0x40, // Overflow
  EMU_6502_FLAG_N = 0x80  // Negative
} emu_6502_flags_t;

/**
 * @brief Contexto específico da CPU 6502
 */
typedef struct {
  uint8_t a;       // Acumulador
  uint8_t x;       // Índice X
  uint8_t y;       // Índice Y
  uint8_t sp;      // Stack Pointer
  uint16_t pc;     // Program Counter
  uint8_t p;       // Status Register
  uint32_t cycles; // Ciclos executados
  void *memory;    // Interface de memória
} emu_6502_context_t;

/**
 * @brief Cria uma nova instância da CPU 6502
 * @return Ponteiro para a interface da CPU ou NULL em caso de erro
 */
emu_cpu_interface_t *emu_cpu_6502_create(void);

#ifdef __cplusplus
}
#endif

#endif // EMU_CPU_6502_H
