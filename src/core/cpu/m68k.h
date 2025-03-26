#ifndef M68K_H
#define M68K_H

#include <stdbool.h>
#include <stdint.h>

// Estrutura de registradores do 68000
typedef struct {
  uint32_t d[8]; // Registradores de dados D0-D7
  uint32_t a[8]; // Registradores de endereço A0-A7
  uint32_t pc;   // Program Counter
  uint16_t sr;   // Status Register
  uint32_t usp;  // User Stack Pointer
  uint32_t ssp;  // Supervisor Stack Pointer
} m68k_registers_t;

// Estrutura do CPU 68000
typedef struct {
  m68k_registers_t regs;
  uint32_t cycles;
  bool stopped;
  bool halted;
  void *memory_ctx;
  uint8_t (*read_byte)(void *ctx, uint32_t addr);
  uint16_t (*read_word)(void *ctx, uint32_t addr);
  uint32_t (*read_long)(void *ctx, uint32_t addr);
  void (*write_byte)(void *ctx, uint32_t addr, uint8_t value);
  void (*write_word)(void *ctx, uint32_t addr, uint16_t value);
  void (*write_long)(void *ctx, uint32_t addr, uint32_t value);
} m68k_t;

// Funções de inicialização e controle
void m68k_init(m68k_t *cpu);
void m68k_reset(m68k_t *cpu);
int m68k_execute(m68k_t *cpu, int cycles);
void m68k_set_irq(m68k_t *cpu, int level);

// Funções de acesso aos registradores
uint32_t m68k_get_reg(m68k_t *cpu, int reg);
void m68k_set_reg(m68k_t *cpu, int reg, uint32_t value);
uint32_t m68k_get_pc(m68k_t *cpu);
void m68k_set_pc(m68k_t *cpu, uint32_t pc);

// Funções de depuração
void m68k_set_breakpoint(m68k_t *cpu, uint32_t addr);
void m68k_clear_breakpoint(m68k_t *cpu, uint32_t addr);
char *m68k_disassemble(m68k_t *cpu, uint32_t addr, int *size);

#endif // M68K_H
