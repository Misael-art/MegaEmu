#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

// Estrutura básica da CPU
typedef struct {
    uint16_t pc;     // Program Counter
    uint16_t sp;     // Stack Pointer
    uint8_t a;       // Acumulador
    uint8_t x;       // Registrador X
    uint8_t y;       // Registrador Y
    uint8_t status;  // Status Register
} CPU;

// Funções da CPU
void cpu_init(CPU* cpu);
void cpu_step(CPU* cpu);
void cpu_reset(CPU* cpu);

#endif  // CPU_H
