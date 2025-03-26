/**
 * @file test_md_mapper.c
 * @brief Testes para o sistema de mappers do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-29
 */

#include "platforms/megadrive/memory/md_mapper.h"
#include <string.h>
#include <unity.h>

// Dados de teste
static uint8_t *test_rom_data;
static uint32_t test_rom_size;
static md_mapper_t test_mapper;

void setUp(void) {
  // Criar ROM de teste (4MB)
  test_rom_size = 4 * 1024 * 1024;
  test_rom_data = (uint8_t *)malloc(test_rom_size);

  // Preencher com padrão de teste
  for (uint32_t i = 0; i < test_rom_size; i++) {
    test_rom_data[i] = i & 0xFF;
  }

  // Adicionar assinatura "SEGA"
  memcpy(test_rom_data + 0x100, "SEGA", 4);
}

void tearDown(void) {
  if (test_rom_data) {
    free(test_rom_data);
    test_rom_data = NULL;
  }
  md_mapper_shutdown(&test_mapper);
}

// Testes para mapper padrão (sem banco)
void test_mapper_none(void) {
  TEST_ASSERT_TRUE(md_mapper_init(&test_mapper, MD_MAPPER_NONE, test_rom_data,
                                  test_rom_size));
  TEST_ASSERT_EQUAL(1, test_mapper.num_banks);
  TEST_ASSERT_EQUAL(test_rom_size, test_mapper.bank_size);

  // Testar leitura direta
  uint8_t value = md_mapper_read_rom(&test_mapper, 0x1234);
  TEST_ASSERT_EQUAL(0x34, value);

  // Testar escrita (deve ser ignorada)
  md_mapper_write_rom(&test_mapper, 0x1234, 0xFF);
  value = md_mapper_read_rom(&test_mapper, 0x1234);
  TEST_ASSERT_EQUAL(0x34, value);
}

// Testes para mapper SSF2
void test_mapper_ssf2(void) {
  TEST_ASSERT_TRUE(md_mapper_init(&test_mapper, MD_MAPPER_SSF2, test_rom_data,
                                  test_rom_size));
  TEST_ASSERT_EQUAL(8, test_mapper.num_banks);
  TEST_ASSERT_EQUAL(512 * 1024, test_mapper.bank_size);

  // Testar leitura inicial (banco 0)
  uint8_t value = md_mapper_read_rom(&test_mapper, 0x000000);
  TEST_ASSERT_EQUAL(0x00, value);

  // Trocar banco e testar leitura
  md_mapper_write_rom(&test_mapper, 0xA13000, 0x07); // Selecionar último banco
  value = md_mapper_read_rom(&test_mapper, 0x000000);
  TEST_ASSERT_EQUAL(0x00, value + (7 * 512 * 1024));
}

// Testes para mapper SSRPG
void test_mapper_ssrpg(void) {
  TEST_ASSERT_TRUE(md_mapper_init(&test_mapper, MD_MAPPER_SSRPG, test_rom_data,
                                  test_rom_size));

  // Testar SRAM desabilitada inicialmente
  TEST_ASSERT_FALSE(test_mapper.sram_enabled);

  // Habilitar SRAM
  md_mapper_write_rom(&test_mapper, 0xA130F1, 0x03); // Habilitar SRAM e escrita
  TEST_ASSERT_TRUE(test_mapper.sram_enabled);

  // Testar escrita/leitura SRAM
  md_mapper_write_sram(&test_mapper, 0x200000, 0x42);
  uint8_t value = md_mapper_read_sram(&test_mapper, 0x200000);
  TEST_ASSERT_EQUAL(0x42, value);
}

// Testes para mapper EEPROM
void test_mapper_eeprom(void) {
  TEST_ASSERT_TRUE(md_mapper_init(&test_mapper, MD_MAPPER_EEPROM, test_rom_data,
                                  test_rom_size));

  // Testar sequência de comandos EEPROM
  md_mapper_write_rom(&test_mapper, 0x200000, 0x06); // WREN
  md_mapper_write_rom(&test_mapper, 0x200000, 0x02); // WRITE
  md_mapper_write_rom(&test_mapper, 0x200000, 0x00); // Endereço alto
  md_mapper_write_rom(&test_mapper, 0x200000, 0x00); // Endereço baixo
  md_mapper_write_rom(&test_mapper, 0x200000, 0x42); // Dado

  // Ler dado escrito
  md_mapper_write_rom(&test_mapper, 0x200000, 0x03); // READ
  md_mapper_write_rom(&test_mapper, 0x200000, 0x00); // Endereço alto
  md_mapper_write_rom(&test_mapper, 0x200000, 0x00); // Endereço baixo
  uint8_t value = md_mapper_read_rom(&test_mapper, 0x200000);
  TEST_ASSERT_EQUAL(0x42, value);
}

// Testes para mapper Codemasters
void test_mapper_codemasters(void) {
  TEST_ASSERT_TRUE(md_mapper_init(&test_mapper, MD_MAPPER_CODEMASTERS,
                                  test_rom_data, test_rom_size));
  TEST_ASSERT_EQUAL(256, test_mapper.num_banks); // 4MB / 16KB
  TEST_ASSERT_EQUAL(16 * 1024, test_mapper.bank_size);

  // Testar troca de bancos
  md_mapper_write_rom(&test_mapper, 0x8000, 0x0F); // Banco 15 em 0x8000
  uint8_t value = md_mapper_read_rom(&test_mapper, 0x8000);
  TEST_ASSERT_EQUAL(0x00 + (15 * 16 * 1024), value);
}

// Testes para mapper EA
void test_mapper_ea(void) {
  TEST_ASSERT_TRUE(
      md_mapper_init(&test_mapper, MD_MAPPER_EA, test_rom_data, test_rom_size));
  TEST_ASSERT_EQUAL(256, test_mapper.num_banks);
  TEST_ASSERT_EQUAL(16 * 1024, test_mapper.bank_size);

  // Testar troca de bancos
  md_mapper_write_rom(&test_mapper, 0xA13000, 0x0F); // Banco 15
  uint8_t value = md_mapper_read_rom(&test_mapper, 0x000000);
  TEST_ASSERT_EQUAL(0x00 + (15 * 16 * 1024), value);
}

// Testes para mapper Pier Solar
void test_mapper_pier_solar(void) {
  TEST_ASSERT_TRUE(md_mapper_init(&test_mapper, MD_MAPPER_PIER_SOLAR,
                                  test_rom_data, test_rom_size));
  TEST_ASSERT_EQUAL(8, test_mapper.num_banks);
  TEST_ASSERT_EQUAL(512 * 1024, test_mapper.bank_size);

  // Testar controle geral
  md_mapper_write_rom(&test_mapper, 0xA130F0, 0x03); // Habilitar SRAM e RTC

  // Testar troca de bancos
  md_mapper_write_rom(&test_mapper, 0xA13000, 0x07); // Último banco
  uint8_t value = md_mapper_read_rom(&test_mapper, 0x000000);
  TEST_ASSERT_EQUAL(0x00 + (7 * 512 * 1024), value);

  // Testar SRAM expandida
  md_mapper_write_sram(&test_mapper, 0x200000, 0x42);
  value = md_mapper_read_sram(&test_mapper, 0x200000);
  TEST_ASSERT_EQUAL(0x42, value);
}

// Testes para detecção de mapper
void test_mapper_detection(void) {
  // Testar detecção por cabeçalho
  test_rom_data[0x1F1] = 0x02; // Flag de SRAM
  md_mapper_type_t type = md_mapper_detect_type(test_rom_data, test_rom_size);
  TEST_ASSERT_EQUAL(MD_MAPPER_SEGA, type);

  // Testar detecção por checksum
  test_rom_data[0x18E] = 0x12;
  test_rom_data[0x18F] = 0x34;
  type = md_mapper_detect_type(test_rom_data, test_rom_size);
  TEST_ASSERT_EQUAL(MD_MAPPER_SSF2, type);

  // Testar detecção por string
  memcpy(test_rom_data + 0x150, "PHANTASY STAR IV", 15);
  type = md_mapper_detect_type(test_rom_data, test_rom_size);
  TEST_ASSERT_EQUAL(MD_MAPPER_SSRPG, type);
}

// Testes para save state
void test_mapper_save_state(void) {
  TEST_ASSERT_TRUE(md_mapper_init(&test_mapper, MD_MAPPER_SSF2, test_rom_data,
                                  test_rom_size));

  // Configurar estado inicial
  md_mapper_write_rom(&test_mapper, 0xA13000, 0x07);
  md_mapper_set_sram_enabled(&test_mapper, true);

  // Criar save state
  save_state_t state;
  TEST_ASSERT_EQUAL(0, md_mapper_register_save_state(&state));

  // Modificar estado
  md_mapper_write_rom(&test_mapper, 0xA13000, 0x00);
  md_mapper_set_sram_enabled(&test_mapper, false);

  // Restaurar estado
  TEST_ASSERT_EQUAL(0, md_mapper_restore_save_state(&state));

  // Verificar estado restaurado
  uint8_t value = md_mapper_read_rom(&test_mapper, 0x000000);
  TEST_ASSERT_EQUAL(0x00 + (7 * 512 * 1024), value);
  TEST_ASSERT_TRUE(test_mapper.sram_enabled);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_mapper_none);
  RUN_TEST(test_mapper_ssf2);
  RUN_TEST(test_mapper_ssrpg);
  RUN_TEST(test_mapper_eeprom);
  RUN_TEST(test_mapper_codemasters);
  RUN_TEST(test_mapper_ea);
  RUN_TEST(test_mapper_pier_solar);
  RUN_TEST(test_mapper_detection);
  RUN_TEST(test_mapper_save_state);

  return UNITY_END();
}
