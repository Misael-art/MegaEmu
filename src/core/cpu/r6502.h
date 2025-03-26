#ifndef R6502_H
#define R6502_H

#include <stdbool.h>
#include <stdint.h>

// Estrutura de registradores do 6502
typedef struct {
  uint8_t a;   // Acumulador
  uint8_t x;   // Índice X
  uint8_t y;   // Índice Y
  uint8_t s;   // Stack Pointer
  uint16_t pc; // Program Counter
  struct {
    uint8_t c : 1; // Carry
    uint8_t z : 1; // Zero
    uint8_t i : 1; // Interrupt Disable
    uint8_t d : 1; // Decimal Mode
    uint8_t b : 1; // Break Command
    uint8_t u : 1; // Unused
    uint8_t v : 1; // Overflow
    uint8_t n : 1; // Negative
  } p;             // Status Register
} r6502_registers_t;

// Estrutura do CPU 6502
typedef struct {
  r6502_registers_t regs;
  uint32_t cycles;
  bool irq_line;
  bool nmi_line;
  void *memory_ctx;
  uint8_t (*read_byte)(void *ctx, uint16_t addr);
  void (*write_byte)(void *ctx, uint16_t addr, uint8_t value);
} r6502_t;

// Funções de inicialização e controle
void r6502_init(r6502_t *cpu);
void r6502_reset(r6502_t *cpu);
int r6502_execute(r6502_t *cpu, int cycles);
void r6502_irq(r6502_t *cpu);
void r6502_nmi(r6502_t *cpu);

// Funções de acesso aos registradores
uint8_t r6502_get_reg(r6502_t *cpu, int reg);
void r6502_set_reg(r6502_t *cpu, int reg, uint8_t value);
uint16_t r6502_get_pc(r6502_t *cpu);
void r6502_set_pc(r6502_t *cpu, uint16_t pc);

// Funções de depuração
void r6502_set_breakpoint(r6502_t *cpu, uint16_t addr);
void r6502_clear_breakpoint(r6502_t *cpu, uint16_t addr);
char *r6502_disassemble(r6502_t *cpu, uint16_t addr, int *size);

#endif // R6502_H
