#include "rp2a03_adapter.h"
#include <stdlib.h>
#include <string.h>

// Funções de adaptação
static int32_t adapter_init(void *ctx, const cpu_config_t *config) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  rp2a03_config_t rp_config = {.read_mem = (rp2a03_read_func_t)config->read_mem,
                               .write_mem =
                                   (rp2a03_write_func_t)config->write_mem,
                               .context = config->context,
                               .log_level = config->log_level};

  if (!rp2a03_init(&rp_config)) {
    return CPU_ERROR_NONE;
  }
  return -1;
}

static void adapter_shutdown(void *ctx) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  rp2a03_shutdown(cpu);
}

static void adapter_reset(void *ctx) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  rp2a03_reset(cpu);
}

static int32_t adapter_execute(void *ctx, int32_t cycles) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  return rp2a03_execute(cpu, cycles);
}

static void adapter_get_state(void *ctx, cpu_state_t *state) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  state->cycles = cpu->cycles;
  state->remaining_cycles = cpu->remaining_cycles;
  state->stall_cycles = cpu->stall_cycles;
  state->interrupt = (cpu_interrupt_t)cpu->pending_interrupt;
}

static void adapter_set_state(void *ctx, const cpu_state_t *state) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  cpu->cycles = state->cycles;
  cpu->remaining_cycles = state->remaining_cycles;
  cpu->stall_cycles = state->stall_cycles;
  cpu->pending_interrupt = (rp2a03_interrupt_t)state->interrupt;
}

static void adapter_trigger_interrupt(void *ctx, cpu_interrupt_t interrupt) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  rp2a03_trigger_interrupt(cpu, (rp2a03_interrupt_t)interrupt);
}

static void adapter_add_stall_cycles(void *ctx, int32_t cycles) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  rp2a03_add_stall_cycles(cpu, cycles);
}

static uint32_t adapter_get_register(void *ctx, const char *reg) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  return rp2a03_get_register(cpu, reg);
}

static void adapter_set_register(void *ctx, const char *reg, uint32_t value) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  rp2a03_set_register(cpu, reg, (uint16_t)value);
}

static int32_t adapter_dump_state(void *ctx, char *buffer,
                                  int32_t buffer_size) {
  rp2a03_t *cpu = (rp2a03_t *)ctx;
  return rp2a03_dump_state(cpu, buffer, buffer_size);
}

cpu_interface_t *rp2a03_create_interface(void) {
  cpu_interface_t *interface =
      (cpu_interface_t *)malloc(sizeof(cpu_interface_t));
  if (!interface) {
    return NULL;
  }

  // Aloca contexto para a CPU
  interface->context = malloc(sizeof(rp2a03_t));
  if (!interface->context) {
    free(interface);
    return NULL;
  }

  // Inicializa funções da interface
  interface->init = adapter_init;
  interface->shutdown = adapter_shutdown;
  interface->reset = adapter_reset;
  interface->execute = adapter_execute;
  interface->get_state = adapter_get_state;
  interface->set_state = adapter_set_state;
  interface->trigger_interrupt = adapter_trigger_interrupt;
  interface->add_stall_cycles = adapter_add_stall_cycles;
  interface->get_register = adapter_get_register;
  interface->set_register = adapter_set_register;
  interface->dump_state = adapter_dump_state;

  return interface;
}
