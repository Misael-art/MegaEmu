#include "cpu_6502.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Mock da memória para testes
static uint8_t mock_memory[0x10000];
static emu_cpu_interface_t mock_memory_interface;

static uint8_t mock_read_byte(void *ctx, uint32_t addr) {
  (void)ctx;
  return mock_memory[addr & 0xFFFF];
}

static void mock_write_byte(void *ctx, uint32_t addr, uint8_t val) {
  (void)ctx;
  mock_memory[addr & 0xFFFF] = val;
}

// Função de inicialização dos testes
static void setup_test(void) {
  memset(mock_memory, 0, sizeof(mock_memory));
  mock_memory_interface.read_byte = mock_read_byte;
  mock_memory_interface.write_byte = mock_write_byte;
}

// Testes unitários
static void test_cpu_create(void) {
  printf("Teste: Criação da CPU 6502...\n");

  emu_cpu_interface_t *cpu = emu_cpu_6502_create();
  assert(cpu != NULL);
  assert(cpu->context != NULL);

  cpu->shutdown(cpu->context);
  free(cpu);
  printf("OK\n");
}

static void test_cpu_init(void) {
  printf("Teste: Inicialização da CPU 6502...\n");

  emu_cpu_interface_t *cpu = emu_cpu_6502_create();
  assert(cpu->init(cpu->context) == 0);

  // Verifica estado inicial
  assert(cpu->get_register(cpu->context, EMU_6502_REG_A) == 0);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_X) == 0);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_Y) == 0);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_SP) == 0xFF);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_P) & EMU_6502_FLAG_I);

  cpu->shutdown(cpu->context);
  free(cpu);
  printf("OK\n");
}

static void test_cpu_reset(void) {
  printf("Teste: Reset da CPU 6502...\n");

  setup_test();
  emu_cpu_interface_t *cpu = emu_cpu_6502_create();

  // Configura o vetor de reset
  mock_memory[0xFFFC] = 0x34;
  mock_memory[0xFFFD] = 0x12;

  // Conecta a memória mock
  emu_6502_context_t *ctx = (emu_6502_context_t *)cpu->context;
  ctx->memory = &mock_memory_interface;

  cpu->reset(cpu->context);

  // Verifica se PC foi carregado do vetor de reset
  assert(cpu->get_register(cpu->context, EMU_6502_REG_PC) == 0x1234);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_SP) == 0xFF);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_P) & EMU_6502_FLAG_I);

  cpu->shutdown(cpu->context);
  free(cpu);
  printf("OK\n");
}

static void test_cpu_registers(void) {
  printf("Teste: Registradores da CPU 6502...\n");

  emu_cpu_interface_t *cpu = emu_cpu_6502_create();

  // Testa escrita/leitura de registradores
  cpu->set_register(cpu->context, EMU_6502_REG_A, 0x42);
  cpu->set_register(cpu->context, EMU_6502_REG_X, 0x69);
  cpu->set_register(cpu->context, EMU_6502_REG_Y, 0xAB);
  cpu->set_register(cpu->context, EMU_6502_REG_PC, 0x1234);

  assert(cpu->get_register(cpu->context, EMU_6502_REG_A) == 0x42);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_X) == 0x69);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_Y) == 0xAB);
  assert(cpu->get_register(cpu->context, EMU_6502_REG_PC) == 0x1234);

  // Testa nomes dos registradores
  assert(strcmp(cpu->get_register_name(cpu->context, EMU_6502_REG_A), "A") ==
         0);
  assert(strcmp(cpu->get_register_name(cpu->context, EMU_6502_REG_X), "X") ==
         0);
  assert(strcmp(cpu->get_register_name(cpu->context, EMU_6502_REG_Y), "Y") ==
         0);
  assert(strcmp(cpu->get_register_name(cpu->context, EMU_6502_REG_PC), "PC") ==
         0);

  cpu->shutdown(cpu->context);
  free(cpu);
  printf("OK\n");
}

// Função principal de testes
int main(void) {
  printf("Iniciando testes da CPU 6502...\n\n");

  test_cpu_create();
  test_cpu_init();
  test_cpu_reset();
  test_cpu_registers();

  printf("\nTodos os testes passaram!\n");
  return 0;
}
