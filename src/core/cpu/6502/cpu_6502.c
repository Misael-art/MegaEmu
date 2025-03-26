#include "cpu_6502.h"
#include <stdlib.h>
#include <string.h>

// Funções estáticas de suporte
static void update_zero_negative_flags(emu_6502_context_t *ctx, uint8_t value) {
  ctx->p &= ~(EMU_6502_FLAG_Z | EMU_6502_FLAG_N);
  if (value == 0)
    ctx->p |= EMU_6502_FLAG_Z;
  if (value & 0x80)
    ctx->p |= EMU_6502_FLAG_N;
}

// Implementações das funções da interface

static int cpu_6502_init(void *ctx) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu)
    return -1;

  // Inicialização dos registradores
  cpu->a = 0;
  cpu->x = 0;
  cpu->y = 0;
  cpu->sp = 0xFF; // Stack pointer começa no topo
  cpu->pc = 0;
  cpu->p = EMU_6502_FLAG_I; // Interrupções desabilitadas inicialmente
  cpu->cycles = 0;

  return 0;
}

static void cpu_6502_reset(void *ctx) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu)
    return;

  // Reset vector está em 0xFFFC-0xFFFD
  uint16_t reset_vector =
      (cpu_6502_read_byte(ctx, 0xFFFD) << 8) | cpu_6502_read_byte(ctx, 0xFFFC);
  cpu->pc = reset_vector;
  cpu->sp = 0xFF;
  cpu->p |= EMU_6502_FLAG_I;
  cpu->cycles = 0;
}

static void cpu_6502_shutdown(void *ctx) {
  if (ctx) {
    free(ctx);
  }
}

static uint8_t cpu_6502_read_byte(void *ctx, uint32_t addr) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu || !cpu->memory)
    return 0;

  // Delega a leitura para a interface de memória
  return ((emu_cpu_interface_t *)cpu->memory)->read_byte(cpu->memory, addr);
}

static void cpu_6502_write_byte(void *ctx, uint32_t addr, uint8_t val) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu || !cpu->memory)
    return;

  // Delega a escrita para a interface de memória
  ((emu_cpu_interface_t *)cpu->memory)->write_byte(cpu->memory, addr, val);
}

static int cpu_6502_execute(void *ctx, int cycles) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu)
    return 0;

  int executed_cycles = 0;
  while (executed_cycles < cycles) {
    // Implementação do fetch-decode-execute cycle
    uint8_t opcode = cpu_6502_read_byte(ctx, cpu->pc++);

    // TODO: Implementar o decode e execute de cada opcode
    // Por enquanto, apenas incrementa os ciclos
    executed_cycles += 2;
    cpu->cycles += 2;
  }

  return executed_cycles;
}

static void cpu_6502_get_state(void *ctx, emu_cpu_state_t *state) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu || !state)
    return;

  state->cycles = cpu->cycles;
  state->flags = EMU_CPU_FLAG_RUNNING; // Por padrão
  state->context = cpu;
}

static void cpu_6502_set_state(void *ctx, const emu_cpu_state_t *state) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu || !state)
    return;

  cpu->cycles = state->cycles;
  // Outros estados específicos podem ser restaurados aqui
}

static uint32_t cpu_6502_get_register(void *ctx, int reg) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu)
    return 0;

  switch (reg) {
  case EMU_6502_REG_A:
    return cpu->a;
  case EMU_6502_REG_X:
    return cpu->x;
  case EMU_6502_REG_Y:
    return cpu->y;
  case EMU_6502_REG_SP:
    return cpu->sp;
  case EMU_6502_REG_PC:
    return cpu->pc;
  case EMU_6502_REG_P:
    return cpu->p;
  default:
    return 0;
  }
}

static void cpu_6502_set_register(void *ctx, int reg, uint32_t value) {
  emu_6502_context_t *cpu = (emu_6502_context_t *)ctx;
  if (!cpu)
    return;

  switch (reg) {
  case EMU_6502_REG_A:
    cpu->a = value & 0xFF;
    break;
  case EMU_6502_REG_X:
    cpu->x = value & 0xFF;
    break;
  case EMU_6502_REG_Y:
    cpu->y = value & 0xFF;
    break;
  case EMU_6502_REG_SP:
    cpu->sp = value & 0xFF;
    break;
  case EMU_6502_REG_PC:
    cpu->pc = value & 0xFFFF;
    break;
  case EMU_6502_REG_P:
    cpu->p = value & 0xFF;
    break;
  }
}

static const char *cpu_6502_get_register_name(void *ctx, int reg) {
  switch (reg) {
  case EMU_6502_REG_A:
    return "A";
  case EMU_6502_REG_X:
    return "X";
  case EMU_6502_REG_Y:
    return "Y";
  case EMU_6502_REG_SP:
    return "SP";
  case EMU_6502_REG_PC:
    return "PC";
  case EMU_6502_REG_P:
    return "P";
  default:
    return "Unknown";
  }
}

// Criação da interface
emu_cpu_interface_t *emu_cpu_6502_create(void) {
  emu_cpu_interface_t *interface = calloc(1, sizeof(emu_cpu_interface_t));
  if (!interface)
    return NULL;

  emu_6502_context_t *context = calloc(1, sizeof(emu_6502_context_t));
  if (!context) {
    free(interface);
    return NULL;
  }

  interface->context = context;
  interface->init = cpu_6502_init;
  interface->reset = cpu_6502_reset;
  interface->shutdown = cpu_6502_shutdown;
  interface->execute = cpu_6502_execute;
  interface->read_byte = cpu_6502_read_byte;
  interface->write_byte = cpu_6502_write_byte;
  interface->get_state = cpu_6502_get_state;
  interface->set_state = cpu_6502_set_state;
  interface->get_register = cpu_6502_get_register;
  interface->set_register = cpu_6502_set_register;
  interface->get_register_name = cpu_6502_get_register_name;

  return interface;
}
