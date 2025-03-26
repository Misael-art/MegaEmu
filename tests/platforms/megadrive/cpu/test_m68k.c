/**
 * @file test_m68k.c
 * @brief Testes unitários para o adaptador M68000 do Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "platforms/megadrive/cpu/m68k_adapter.h"
#include "test/test_framework.h"
#include <stdlib.h>
#include <string.h>

// Mocks e stubs
static uint8_t mock_memory[0x10000];
static uint16_t mock_read_memory(uint32_t addr, void *user_data) {
  (void)user_data; // Não utilizado
  return (mock_memory[addr & 0xFFFF] << 8) | mock_memory[(addr + 1) & 0xFFFF];
}

static void mock_write_memory(uint32_t addr, uint16_t value, void *user_data) {
  (void)user_data; // Não utilizado
  mock_memory[addr & 0xFFFF] = value >> 8;
  mock_memory[(addr + 1) & 0xFFFF] = value & 0xFF;
}

// Funções auxiliares
static void setup_m68k(emu_cpu_interface_t **cpu,
                       megadrive_m68k_context_t **context) {
  *cpu = megadrive_m68k_adapter_create();
  TEST_ASSERT_NOT_NULL(*cpu);

  *context = megadrive_m68k_get_context(*cpu);
  TEST_ASSERT_NOT_NULL(*context);

  // Configura callbacks de memória
  m68k_set_memory_callbacks(*context, mock_read_memory, mock_write_memory,
                            NULL);

  // Inicializa com configuração padrão
  emu_cpu_config_t config = {0};
  TEST_ASSERT_EQUAL(0, (*cpu)->init((*cpu)->context, &config));
}

static void teardown_m68k(emu_cpu_interface_t *cpu) {
  if (cpu) {
    megadrive_m68k_adapter_destroy(cpu);
  }
}

// Testes de inicialização
TEST_CASE(test_m68k_create) {
  emu_cpu_interface_t *cpu = NULL;
  megadrive_m68k_context_t *context = NULL;

  // Testa criação
  setup_m68k(&cpu, &context);
  TEST_ASSERT_NOT_NULL(cpu);
  TEST_ASSERT_NOT_NULL(context);

  // Verifica valores iniciais
  TEST_ASSERT_EQUAL(0, context->pc);
  TEST_ASSERT_EQUAL(SR_SUPERVISOR, context->sr);
  TEST_ASSERT_FALSE(context->stopped);
  TEST_ASSERT_EQUAL(0, context->interrupt_level);
  TEST_ASSERT_FALSE(context->interrupt_pending);

  teardown_m68k(cpu);
}

// Testes de memória
TEST_CASE(test_m68k_memory) {
  emu_cpu_interface_t *cpu = NULL;
  megadrive_m68k_context_t *context = NULL;
  setup_m68k(&cpu, &context);

  // Testa RAM
  uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
  memcpy(context->ram, test_data, sizeof(test_data));

  TEST_ASSERT_EQUAL(0x12, cpu->read_memory(cpu->context, 0));
  TEST_ASSERT_EQUAL(0x34, cpu->read_memory(cpu->context, 1));
  TEST_ASSERT_EQUAL(0x56, cpu->read_memory(cpu->context, 2));
  TEST_ASSERT_EQUAL(0x78, cpu->read_memory(cpu->context, 3));

  // Testa ROM
  uint8_t rom_data[MD_M68K_ROM_BANK_SIZE] = {0};
  for (int i = 0; i < 256; i++) {
    rom_data[i] = i;
  }

  m68k_load_rom(context, rom_data, sizeof(rom_data));
  TEST_ASSERT_EQUAL(0, context->current_bank);
  TEST_ASSERT_EQUAL(rom_data[0], context->rom[0]);
  TEST_ASSERT_EQUAL(rom_data[1], context->rom[1]);

  // Testa troca de banco
  m68k_set_rom_bank(context, 1);
  TEST_ASSERT_EQUAL(1, context->current_bank);
  TEST_ASSERT_EQUAL(context->rom_banks[1], context->rom);

  teardown_m68k(cpu);
}

// Testes de registradores
TEST_CASE(test_m68k_registers) {
  emu_cpu_interface_t *cpu = NULL;
  megadrive_m68k_context_t *context = NULL;
  setup_m68k(&cpu, &context);

  // Testa registradores de dados
  for (int i = REG_D0; i <= REG_D7; i++) {
    context->registers[i] = 0x12345678;
    TEST_ASSERT_EQUAL(0x12345678, m68k_get_register(context, i));
  }

  // Testa registradores de endereço
  for (int i = REG_A0; i <= REG_A7; i++) {
    context->registers[i] = 0x9ABCDEF0;
    TEST_ASSERT_EQUAL(0x9ABCDEF0, m68k_get_register(context, i));
  }

  // Testa PC e SR
  context->pc = 0x00FF0000;
  context->sr = 0x2700;
  TEST_ASSERT_EQUAL(0x00FF0000, m68k_get_pc(context));
  TEST_ASSERT_EQUAL(0x2700, m68k_get_sr(context));

  teardown_m68k(cpu);
}

// Testes de interrupções
TEST_CASE(test_m68k_interrupts) {
  emu_cpu_interface_t *cpu = NULL;
  megadrive_m68k_context_t *context = NULL;
  setup_m68k(&cpu, &context);

  // Configura SR para permitir interrupções
  context->sr = 0x2000; // Nível 0 (todas permitidas)

  // Testa VBLANK (nível 6)
  m68k_trigger_interrupt(context, MD_M68K_INT_VBLANK);
  TEST_ASSERT_TRUE(context->interrupt_pending);
  TEST_ASSERT_EQUAL(MD_M68K_INT_VBLANK, context->interrupt_level);

  // Limpa interrupção
  m68k_clear_interrupt(context, MD_M68K_INT_VBLANK);
  TEST_ASSERT_FALSE(context->interrupt_pending);
  TEST_ASSERT_EQUAL(0, context->interrupt_level);

  // Testa HBLANK (nível 4)
  m68k_trigger_interrupt(context, MD_M68K_INT_HBLANK);
  TEST_ASSERT_TRUE(context->interrupt_pending);
  TEST_ASSERT_EQUAL(MD_M68K_INT_HBLANK, context->interrupt_level);

  // Testa prioridade (VBLANK deve ter precedência)
  m68k_trigger_interrupt(context, MD_M68K_INT_VBLANK);
  TEST_ASSERT_TRUE(context->interrupt_pending);
  TEST_ASSERT_EQUAL(MD_M68K_INT_VBLANK, context->interrupt_level);

  teardown_m68k(cpu);
}

// Testes de execução
TEST_CASE(test_m68k_execute) {
  emu_cpu_interface_t *cpu = NULL;
  megadrive_m68k_context_t *context = NULL;
  setup_m68k(&cpu, &context);

  // Configura ciclos
  int cycles = cpu->execute(cpu->context, 100);
  TEST_ASSERT_TRUE(cycles > 0);
  TEST_ASSERT_TRUE(cycles <= 100);
  TEST_ASSERT_EQUAL(cycles, context->cycles);

  // Testa parada
  context->stopped = true;
  cycles = cpu->execute(cpu->context, 100);
  TEST_ASSERT_EQUAL(0, cycles);

  teardown_m68k(cpu);
}

// Testes de estado
TEST_CASE(test_m68k_state) {
  emu_cpu_interface_t *cpu = NULL;
  megadrive_m68k_context_t *context = NULL;
  setup_m68k(&cpu, &context);

  // Configura estado inicial
  context->pc = 0x1234;
  context->sr = 0x2700;
  context->cycles = 1000;

  // Obtém estado
  emu_cpu_state_t state = {0};
  cpu->get_state(cpu->context, &state);
  TEST_ASSERT_EQUAL(0x1234, state.pc);
  TEST_ASSERT_EQUAL(0x2700, state.flags);
  TEST_ASSERT_EQUAL(1000, state.cycles);

  // Modifica estado
  state.pc = 0x5678;
  state.flags = 0x2000;
  state.cycles = 2000;
  cpu->set_state(cpu->context, &state);

  TEST_ASSERT_EQUAL(0x5678, context->pc);
  TEST_ASSERT_EQUAL(0x2000, context->sr);
  TEST_ASSERT_EQUAL(2000, context->cycles);

  teardown_m68k(cpu);
}

// Executa todos os testes
int main(void) {
  TEST_INIT();

  RUN_TEST(test_m68k_create);
  RUN_TEST(test_m68k_memory);
  RUN_TEST(test_m68k_registers);
  RUN_TEST(test_m68k_interrupts);
  RUN_TEST(test_m68k_execute);
  RUN_TEST(test_m68k_state);

  TEST_REPORT();
  return 0;
}
