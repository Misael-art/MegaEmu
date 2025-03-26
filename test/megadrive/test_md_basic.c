/**
 * @file test_md_basic.c
 * @brief Testes básicos para o emulador Mega Drive
 */

#include "../../src/core/interfaces/memory_interface.h"
#include "../../src/core/interfaces/platform_interface.h"
#include "../../src/core/interfaces/state_interface.h"
#include "../../src/platforms/megadrive/megadrive.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Função para testar a inicialização da plataforma
bool test_md_init() {
  printf("Testando inicialização da plataforma Mega Drive...\n");

  emu_platform_t platform = {0};

  // Teste de inicialização
  bool result = md_platform_init(&platform);
  assert(result == true);
  assert(platform.platform_data != NULL);

  // Verificar campos básicos
  md_platform_data_t *data = (md_platform_data_t *)platform.platform_data;
  assert(data->is_initialized == true);
  assert(data->ram != NULL);
  assert(data->ram_size == 64 * 1024);
  assert(data->z80_ram != NULL);
  assert(data->z80_ram_size == 8 * 1024);
  assert(data->z80 != NULL);

  // Limpar
  md_platform_shutdown(&platform);
  printf("Teste de inicialização: OK\n");
  return true;
}

// Função para testar o reset da plataforma
bool test_md_reset() {
  printf("Testando reset da plataforma Mega Drive...\n");

  emu_platform_t platform = {0};
  md_platform_init(&platform);

  // Obter dados da plataforma
  md_platform_data_t *data = (md_platform_data_t *)platform.platform_data;

  // Modificar alguns valores para verificar reset
  data->pad1_state = 0x12;
  data->pad2_state = 0x34;
  data->vdp_data_buffer = 0x5678;
  data->vdp_control_buffer = 0x9ABC;

  // Realizar reset
  bool result = md_platform_reset(&platform);
  assert(result == true);

  // Verificar se os valores foram resetados
  assert(data->pad1_state == 0xFF);
  assert(data->pad2_state == 0xFF);
  assert(data->vdp_data_buffer == 0);
  assert(data->vdp_control_buffer == 0);
  assert(data->z80_control == 0x01);

  // Limpar
  md_platform_shutdown(&platform);
  printf("Teste de reset: OK\n");
  return true;
}

// Função para testar acesso à memória
bool test_md_memory() {
  printf("Testando acesso à memória da plataforma Mega Drive...\n");

  emu_platform_t platform = {0};
  md_platform_init(&platform);

  // Obter dados da plataforma
  md_platform_data_t *data = (md_platform_data_t *)platform.platform_data;

  // Escrever na RAM
  uint32_t ram_addr = 0xFF0000;
  md_memory_write_u8(&platform, ram_addr, 0xAA);
  md_memory_write_u8(&platform, ram_addr + 1, 0xBB);

  // Ler da RAM e verificar
  uint8_t value1 = md_memory_read_u8(&platform, ram_addr);
  uint8_t value2 = md_memory_read_u8(&platform, ram_addr + 1);
  assert(value1 == 0xAA);
  assert(value2 == 0xBB);

  // Escrever na RAM do Z80
  uint32_t z80_ram_addr = 0xA00000;
  md_memory_write_u8(&platform, z80_ram_addr, 0xCC);
  md_memory_write_u8(&platform, z80_ram_addr + 1, 0xDD);

  // Ler da RAM do Z80 e verificar
  value1 = md_memory_read_u8(&platform, z80_ram_addr);
  value2 = md_memory_read_u8(&platform, z80_ram_addr + 1);
  assert(value1 == 0xCC);
  assert(value2 == 0xDD);

  // Limpar
  md_platform_shutdown(&platform);
  printf("Teste de memória: OK\n");
  return true;
}

// Função para testar execução de ciclos
bool test_md_execute() {
  printf("Testando execução de ciclos da plataforma Mega Drive...\n");

  emu_platform_t platform = {0};
  md_platform_init(&platform);

  // Executar um frame
  bool result = md_platform_run_frame(&platform);
  assert(result == true);

  // Executar ciclos específicos
  uint32_t cycles = md_platform_run_cycles(&platform, 1000);
  assert(cycles > 0);

  // Limpar
  md_platform_shutdown(&platform);
  printf("Teste de execução: OK\n");
  return true;
}

// Função principal
int main() {
  printf("==== Iniciando testes do emulador Mega Drive ====\n");

  // Executar testes
  test_md_init();
  test_md_reset();
  test_md_memory();
  test_md_execute();

  printf("==== Todos os testes concluídos com sucesso! ====\n");
  return 0;
}
