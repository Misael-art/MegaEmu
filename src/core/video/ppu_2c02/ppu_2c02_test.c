#include "ppu_2c02.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Mock da memória para testes
static uint8_t mock_memory[0x4000];
static emu_ppu_interface_t mock_memory_interface;

static uint8_t mock_read_byte(void *ctx, uint32_t addr) {
  (void)ctx;
  return mock_memory[addr & 0x3FFF];
}

static void mock_write_byte(void *ctx, uint32_t addr, uint8_t val) {
  (void)ctx;
  mock_memory[addr & 0x3FFF] = val;
}

// Função de inicialização dos testes
static void setup_test(void) {
  memset(mock_memory, 0, sizeof(mock_memory));
  mock_memory_interface.read_register = mock_read_byte;
  mock_memory_interface.write_register = mock_write_byte;
}

// Testes unitários
static void test_ppu_create(void) {
  printf("Teste: Criação da PPU 2C02...\n");

  emu_ppu_interface_t *ppu = emu_ppu_2c02_create();
  assert(ppu != NULL);
  assert(ppu->context != NULL);

  ppu->shutdown(ppu->context);
  free(ppu);
  printf("OK\n");
}

static void test_ppu_init(void) {
  printf("Teste: Inicialização da PPU 2C02...\n");

  emu_ppu_interface_t *ppu = emu_ppu_2c02_create();
  assert(ppu->init(ppu->context) == 0);

  // Verifica estado inicial
  emu_ppu_state_t state;
  ppu->get_state(ppu->context, &state);
  assert(state.cycles == 0);
  assert(state.scanline == 0);
  assert(state.frame == 0);
  assert(state.flags == EMU_PPU_FLAG_NONE);

  ppu->shutdown(ppu->context);
  free(ppu);
  printf("OK\n");
}

static void test_ppu_registers(void) {
  printf("Teste: Registradores da PPU 2C02...\n");

  emu_ppu_interface_t *ppu = emu_ppu_2c02_create();
  ppu->init(ppu->context);

  // Testa escrita/leitura de registradores
  ppu->write_register(ppu->context, EMU_2C02_REG_PPUCTRL, 0x80); // Enable NMI
  ppu->write_register(ppu->context, EMU_2C02_REG_PPUMASK,
                      0x18); // Show background and sprites

  // Status deve ser 0 inicialmente
  assert((ppu->read_register(ppu->context, EMU_2C02_REG_PPUSTATUS) & 0x80) ==
         0);

  // Executa alguns ciclos para entrar em vblank
  for (int i = 0; i < 89342; i++) { // ~262 scanlines
    ppu->execute(ppu->context, 1);
  }

  // Agora deve estar em vblank
  assert((ppu->read_register(ppu->context, EMU_2C02_REG_PPUSTATUS) & 0x80) !=
         0);

  ppu->shutdown(ppu->context);
  free(ppu);
  printf("OK\n");
}

static void test_ppu_vram_access(void) {
  printf("Teste: Acesso à VRAM da PPU 2C02...\n");

  emu_ppu_interface_t *ppu = emu_ppu_2c02_create();
  ppu->init(ppu->context);

  // Testa escrita/leitura de VRAM
  ppu->write_register(ppu->context, EMU_2C02_REG_PPUADDR, 0x20); // High byte
  ppu->write_register(ppu->context, EMU_2C02_REG_PPUADDR, 0x00); // Low byte

  // Escreve alguns valores
  for (int i = 0; i < 16; i++) {
    ppu->write_register(ppu->context, EMU_2C02_REG_PPUDATA, i);
  }

  // Lê os valores de volta
  ppu->write_register(ppu->context, EMU_2C02_REG_PPUADDR, 0x20);
  ppu->write_register(ppu->context, EMU_2C02_REG_PPUADDR, 0x00);

  // O primeiro read é descartado (buffer interno da PPU)
  ppu->read_register(ppu->context, EMU_2C02_REG_PPUDATA);

  for (int i = 0; i < 16; i++) {
    uint8_t val = ppu->read_register(ppu->context, EMU_2C02_REG_PPUDATA);
    assert(val == i);
  }

  ppu->shutdown(ppu->context);
  free(ppu);
  printf("OK\n");
}

static void test_ppu_rendering(void) {
  printf("Teste: Renderização da PPU 2C02...\n");

  emu_ppu_interface_t *ppu = emu_ppu_2c02_create();
  ppu->init(ppu->context);

  // Habilita renderização
  ppu->write_register(ppu->context, EMU_2C02_REG_PPUMASK, 0x18);

  // Executa um frame completo
  for (int i = 0; i < 89342; i++) {
    ppu->execute(ppu->context, 1);
  }

  // Verifica se o frame foi incrementado
  emu_ppu_state_t state;
  ppu->get_state(ppu->context, &state);
  assert(state.frame == 1);

  ppu->shutdown(ppu->context);
  free(ppu);
  printf("OK\n");
}

// Função principal de testes
int main(void) {
  printf("Iniciando testes da PPU 2C02...\n\n");

  test_ppu_create();
  test_ppu_init();
  test_ppu_registers();
  test_ppu_vram_access();
  test_ppu_rendering();

  printf("\nTodos os testes passaram!\n");
  return 0;
}
