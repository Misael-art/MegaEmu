#include "unity/unity.h"
#include "platforms/nes/cartridge/mapper0.h"
#include <stdlib.h>

static uint8_t *rom_data;
static Mapper0 *mapper;

void setUp(void)
{
    // Configurar dados de teste
    rom_data = (uint8_t *)malloc(32768); // 32KB ROM
    for (int i = 0; i < 32768; i++)
    {
        rom_data[i] = i & 0xFF;
    }
    mapper = mapper0_create(rom_data, 32768, 8192); // 32KB ROM, 8KB VROM
}

void tearDown(void)
{
    // Limpar após cada teste
    if (mapper)
    {
        mapper0_destroy(mapper);
        mapper = NULL;
    }
    free(rom_data);
    rom_data = NULL;
}

void test_mapper0_creation(void)
{
    TEST_ASSERT_NOT_NULL(mapper);
    TEST_ASSERT_EQUAL_UINT32(32768, mapper0_get_prg_size(mapper));
    TEST_ASSERT_EQUAL_UINT32(8192, mapper0_get_chr_size(mapper));
}

void test_mapper0_read_prg(void)
{
    uint8_t value;
    // Teste de leitura do primeiro byte
    value = mapper0_read_prg(mapper, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0, value);

    // Teste de leitura do último byte
    value = mapper0_read_prg(mapper, 0xFFFF);
    TEST_ASSERT_EQUAL_UINT8(0xFF, value);

    // Teste de leitura de um byte no meio
    value = mapper0_read_prg(mapper, 0xC000);
    TEST_ASSERT_EQUAL_UINT8(0x00, value);
}

void test_mapper0_write_prg(void)
{
    // Escrever em um endereço de PRG-ROM (deve ser ignorado no Mapper 0)
    mapper0_write_prg(mapper, 0x8000, 0x42);

    // Verificar se o valor não foi alterado
    uint8_t value = mapper0_read_prg(mapper, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0, value);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mapper0_creation);
    RUN_TEST(test_mapper0_read_prg);
    RUN_TEST(test_mapper0_write_prg);
    return UNITY_END();
}
