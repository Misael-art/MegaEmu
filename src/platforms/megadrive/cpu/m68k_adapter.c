/**
 * @file m68k_adapter.c
 * @brief Implementação do adaptador para o processador Motorola 68000 do Mega
 * Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "m68k_adapter.h"
#include <stdlib.h>
#include <string.h>

// Constantes do M68000
#define SR_TRACE 0x8000      // Modo trace
#define SR_SUPERVISOR 0x2000 // Modo supervisor
#define SR_INT_MASK 0x0700   // Máscara de interrupção
#define SR_EXTEND 0x0010     // Flag extend
#define SR_NEGATIVE 0x0008   // Flag negative
#define SR_ZERO 0x0004       // Flag zero
#define SR_OVERFLOW 0x0002   // Flag overflow
#define SR_CARRY 0x0001      // Flag carry

// Registradores
#define REG_D0 0 // Data registers
#define REG_D1 1
#define REG_D2 2
#define REG_D3 3
#define REG_D4 4
#define REG_D5 5
#define REG_D6 6
#define REG_D7 7
#define REG_A0 8 // Address registers
#define REG_A1 9
#define REG_A2 10
#define REG_A3 11
#define REG_A4 12
#define REG_A5 13
#define REG_A6 14
#define REG_A7 15 // Stack pointer

// Declaração da função de execução definida em m68k_execute.c
extern int m68k_execute_cycles(megadrive_m68k_context_t *ctx,
                               int target_cycles);

// Funções estáticas do adaptador
static int adapter_init(void *ctx, const emu_cpu_config_t *config) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context || !config)
    return -1;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));

  // Aloca memória RAM
  context->ram = calloc(1, MD_M68K_RAM_SIZE);
  if (!context->ram)
    return -1;

  // Configura estado inicial
  context->pc = 0;
  context->sr = SR_SUPERVISOR; // Inicia em modo supervisor
  context->stopped = false;
  context->interrupt_level = 0;
  context->interrupt_pending = false;
  context->cycles = 0;
  context->target_cycles = 0;

  return 0;
}

static void adapter_reset(void *ctx) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context)
    return;

  // Lê vetor de reset da ROM
  if (context->rom) {
    context->pc = (context->rom[4] << 24) | (context->rom[5] << 16) |
                  (context->rom[6] << 8) | context->rom[7];
  } else {
    context->pc = 0;
  }

  // Reseta estado
  context->sr = SR_SUPERVISOR;
  context->stopped = false;
  context->interrupt_level = 0;
  context->interrupt_pending = false;
  context->cycles = 0;
  context->target_cycles = 0;

  // Limpa registradores
  memset(context->registers, 0, sizeof(context->registers));

  // Configura stack pointer inicial
  if (context->rom) {
    context->registers[REG_A7] = (context->rom[0] << 24) |
                                 (context->rom[1] << 16) |
                                 (context->rom[2] << 8) | context->rom[3];
  } else {
    context->registers[REG_A7] = 0;
  }
}

static void adapter_shutdown(void *ctx) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context)
    return;

  // Libera memória RAM
  free(context->ram);
  context->ram = NULL;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));
}

static int adapter_execute(void *ctx, int cycles) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context || cycles <= 0)
    return 0;

  // Usa a nova função de execução
  return m68k_execute_cycles(context, cycles);
}

static uint8_t adapter_read_memory(void *ctx, uint32_t address) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context)
    return 0xFF;

  // Mapeia endereço para região de memória
  if (address < MD_M68K_RAM_SIZE) {
    // RAM
    return context->ram[address];
  } else if (context->rom && address < context->rom_size) {
    // ROM
    return context->rom[address];
  } else if (context->read_callback) {
    // Callback de leitura para outros dispositivos
    return context->read_callback(address, context->callback_data) & 0xFF;
  }

  return 0xFF;
}

static void adapter_write_memory(void *ctx, uint32_t address, uint8_t value) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context)
    return;

  // Mapeia endereço para região de memória
  if (address < MD_M68K_RAM_SIZE) {
    // RAM
    context->ram[address] = value;
  } else if (context->write_callback) {
    // Callback de escrita para outros dispositivos
    context->write_callback(address, value, context->callback_data);
  }
}

static void adapter_get_state(void *ctx, emu_cpu_state_t *state) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context || !state)
    return;

  state->pc = context->pc;
  state->cycles = context->cycles;
  state->flags = context->sr;
  state->context = context;
}

static void adapter_set_state(void *ctx, const emu_cpu_state_t *state) {
  megadrive_m68k_context_t *context = (megadrive_m68k_context_t *)ctx;
  if (!context || !state)
    return;

  context->pc = state->pc;
  context->cycles = state->cycles;
  context->sr = state->flags;
}

// Funções públicas
emu_cpu_interface_t *megadrive_m68k_adapter_create(void) {
  emu_cpu_interface_t *interface = calloc(1, sizeof(emu_cpu_interface_t));
  megadrive_m68k_context_t *context =
      calloc(1, sizeof(megadrive_m68k_context_t));

  if (!interface || !context) {
    free(interface);
    free(context);
            return NULL;
        }

  // Configura a interface
  interface->context = context;
  interface->init = adapter_init;
  interface->reset = adapter_reset;
  interface->shutdown = adapter_shutdown;
  interface->execute = adapter_execute;
  interface->read_memory = adapter_read_memory;
  interface->write_memory = adapter_write_memory;
  interface->get_state = adapter_get_state;
  interface->set_state = adapter_set_state;

  return interface;
}

void megadrive_m68k_adapter_destroy(emu_cpu_interface_t *cpu) {
  if (!cpu)
    return;

  if (cpu->context) {
    adapter_shutdown(cpu->context);
    free(cpu->context);
  }

  free(cpu);
}

megadrive_m68k_context_t *megadrive_m68k_get_context(emu_cpu_interface_t *cpu) {
  if (!cpu || !cpu->context)
    return NULL;
  return (megadrive_m68k_context_t *)cpu->context;
}

void megadrive_m68k_set_context(emu_cpu_interface_t *cpu,
                                megadrive_m68k_context_t *context) {
  if (!cpu || !cpu->context || !context)
    return;
  memcpy(cpu->context, context, sizeof(megadrive_m68k_context_t));
}

void m68k_set_memory_callbacks(megadrive_m68k_context_t *ctx,
                               md_m68k_read_callback_t read_cb,
                               md_m68k_write_callback_t write_cb,
                               void *user_data) {
  if (!ctx)
    return;
  ctx->read_callback = read_cb;
  ctx->write_callback = write_cb;
  ctx->callback_data = user_data;
}

void m68k_load_rom(megadrive_m68k_context_t *ctx, const uint8_t *data,
                   uint32_t size) {
  if (!ctx || !data || size == 0)
    return;

  // Verifica tamanho máximo
  if (size > MD_M68K_ROM_BANK_SIZE * MD_M68K_MAX_ROM_BANKS) {
    size = MD_M68K_ROM_BANK_SIZE * MD_M68K_MAX_ROM_BANKS;
  }

  // Copia ROM para os bancos
  uint32_t banks = (size + MD_M68K_ROM_BANK_SIZE - 1) / MD_M68K_ROM_BANK_SIZE;
  for (uint32_t i = 0; i < banks; i++) {
    uint32_t offset = i * MD_M68K_ROM_BANK_SIZE;
    uint32_t remaining = size - offset;
    uint32_t copy_size =
        remaining < MD_M68K_ROM_BANK_SIZE ? remaining : MD_M68K_ROM_BANK_SIZE;
    memcpy(ctx->rom_banks[i], data + offset, copy_size);
  }

  // Configura banco inicial
  ctx->rom = ctx->rom_banks[0];
  ctx->rom_size = size;
  ctx->current_bank = 0;
}

void m68k_set_rom_bank(megadrive_m68k_context_t *ctx, uint8_t bank) {
  if (!ctx || bank >= MD_M68K_MAX_ROM_BANKS)
    return;
  ctx->current_bank = bank;
  ctx->rom = ctx->rom_banks[bank];
}

void m68k_trigger_interrupt(megadrive_m68k_context_t *ctx,
                            md_m68k_interrupt_t level) {
  if (!ctx)
    return;
  ctx->interrupt_level = level;
  ctx->interrupt_pending = true;
}

void m68k_clear_interrupt(megadrive_m68k_context_t *ctx,
                          md_m68k_interrupt_t level) {
  if (!ctx)
    return;
  if (ctx->interrupt_level == level) {
    ctx->interrupt_level = 0;
    ctx->interrupt_pending = false;
  }
}

uint32_t m68k_get_pc(const megadrive_m68k_context_t *ctx) {
  return ctx ? ctx->pc : 0;
}

uint32_t m68k_get_register(const megadrive_m68k_context_t *ctx, uint8_t reg) {
  if (!ctx || reg >= 16)
        return 0;
  return ctx->registers[reg];
}

uint16_t m68k_get_sr(const megadrive_m68k_context_t *ctx) {
  return ctx ? ctx->sr : 0;
}

bool m68k_is_stopped(const megadrive_m68k_context_t *ctx) {
  return ctx ? ctx->stopped : false;
}
