#include "unity/unity.h"
#include "platforms/nes/cartridge/mapper1.h"
#include <stdlib.h>

static uint8_t *rom_data;
static Mapper1 *mapper;

void setUp(void)
{
    // Configurar dados de teste - 128KB PRG-ROM, 128KB CHR-ROM
    rom_data = (uint8_t *)malloc(256 * 1024);
    for (int i = 0; i < 256 * 1024; i++)
    {
        rom_data[i] = i & 0xFF;
    }
    mapper = mapper1_create(rom_data, 128 * 1024, 128 * 1024);
}

void tearDown(void)
{
    if (mapper)
    {
        mapper1_destroy(mapper);
        mapper = NULL;
    }
    free(rom_data);
    rom_data = NULL;
}

void test_mapper1_creation(void)
{
    TEST_ASSERT_NOT_NULL(mapper);
    TEST_ASSERT_EQUAL_UINT32(128 * 1024, mapper1_get_prg_size(mapper));
    TEST_ASSERT_EQUAL_UINT32(128 * 1024, mapper1_get_chr_size(mapper));
}

void test_mapper1_register_write(void)
{
    // Teste de escrita sequencial no registrador de controle
    mapper1_write_prg(mapper, 0x8000, 0x80); // Reset
    mapper1_write_prg(mapper, 0x8000, 0x01); // Bit 0
    mapper1_write_prg(mapper, 0x8000, 0x01); // Bit 1
    mapper1_write_prg(mapper, 0x8000, 0x01); // Bit 2
    mapper1_write_prg(mapper, 0x8000, 0x01); // Bit 3
    mapper1_write_prg(mapper, 0x8000, 0x01); // Bit 4

    // Verificar se o registrador foi configurado corretamente
    uint8_t control = mapper1_get_control(mapper);
    TEST_ASSERT_EQUAL_UINT8(0x1F, control);
}

void test_mapper1_prg_bank_switching(void)
{
    // Configurar modo de 16KB
    mapper1_write_prg(mapper, 0x8000, 0x80); // Reset
    mapper1_write_prg(mapper, 0x8000, 0x00); // Modo 16KB

    // Selecionar banco PRG
    mapper1_write_prg(mapper, 0xE000, 0x0F); // Último banco

    // Verificar se o banco foi selecionado corretamente
    uint8_t value = mapper1_read_prg(mapper, 0xC000);
    TEST_ASSERT_NOT_EQUAL(0, value);
}

void test_mapper1_chr_bank_switching(void)
{
    // Configurar modo de 4KB
    mapper1_write_prg(mapper, 0x8000, 0x80); // Reset
    mapper1_write_prg(mapper, 0x8000, 0x01); // Modo 4KB

    // Selecionar bancos CHR
    mapper1_write_prg(mapper, 0xA000, 0x0F); // Banco 0
    mapper1_write_prg(mapper, 0xC000, 0x1F); // Banco 1

    // Verificar se os bancos foram selecionados corretamente
    uint8_t value0 = mapper1_read_chr(mapper, 0x0000);
    uint8_t value1 = mapper1_read_chr(mapper, 0x1000);
    TEST_ASSERT_NOT_EQUAL(value0, value1);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mapper1_creation);
    RUN_TEST(test_mapper1_register_write);
    RUN_TEST(test_mapper1_prg_bank_switching);
    RUN_TEST(test_mapper1_chr_bank_switching);
    return UNITY_END();
}
