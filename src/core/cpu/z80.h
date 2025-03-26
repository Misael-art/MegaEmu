#ifndef Z80_H
#define Z80_H

#include <stdbool.h>
#include <stdint.h>

// Estrutura de registradores do Z80
typedef struct {
  union {
    struct {
      uint8_t f;
      uint8_t a;
    };
    uint16_t af;
  };
  union {
    struct {
      uint8_t c;
      uint8_t b;
    };
    uint16_t bc;
  };
  union {
    struct {
      uint8_t e;
      uint8_t d;
    };
    uint16_t de;
  };
  union {
    struct {
      uint8_t l;
      uint8_t h;
    };
    uint16_t hl;
  };
  uint16_t ix;
  uint16_t iy;
  uint16_t sp;
  uint16_t pc;
  // Registradores alternativos
  uint16_t af_;
  uint16_t bc_;
  uint16_t de_;
  uint16_t hl_;
  // Registradores especiais
  uint8_t i;
  uint8_t r;
  bool iff1;
  bool iff2;
  uint8_t im;
} z80_registers_t;

// Estrutura do CPU Z80
typedef struct {
  z80_registers_t regs;
  uint32_t cycles;
  bool halted;
  void *memory_ctx;
  uint8_t (*read_byte)(void *ctx, uint16_t addr);
  void (*write_byte)(void *ctx, uint16_t addr, uint8_t value);
  uint8_t (*read_io)(void *ctx, uint16_t port);
  void (*write_io)(void *ctx, uint16_t port, uint8_t value);
} z80_t;

// Funções de inicialização e controle
void z80_init(z80_t *cpu);
void z80_reset(z80_t *cpu);
int z80_execute(z80_t *cpu, int cycles);
void z80_interrupt(z80_t *cpu, bool nmi);

// Funções de acesso aos registradores
uint16_t z80_get_reg(z80_t *cpu, int reg);
void z80_set_reg(z80_t *cpu, int reg, uint16_t value);
uint16_t z80_get_pc(z80_t *cpu);
void z80_set_pc(z80_t *cpu, uint16_t pc);

// Funções de depuração
void z80_set_breakpoint(z80_t *cpu, uint16_t addr);
void z80_clear_breakpoint(z80_t *cpu, uint16_t addr);
char *z80_disassemble(z80_t *cpu, uint16_t addr, int *size);

#endif // Z80_H
