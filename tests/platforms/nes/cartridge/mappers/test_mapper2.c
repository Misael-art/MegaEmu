/**
 * @file test_mapper2.c
 * @brief Testes para o Mapper 2 (UxROM)
 */

#include <unity.h>
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper2.h"

// Mock das funções de log
#define EMU_LOG_ERROR(cat, fmt, ...) ((void)0)
#define EMU_LOG_WARN(cat, fmt, ...) ((void)0)
#define EMU_LOG_INFO(cat, fmt, ...) ((void)0)
#define EMU_LOG_DEBUG(cat, fmt, ...) ((void)0)
#define EMU_LOG_TRACE(cat, fmt, ...) ((void)0)

// Estrutura para o contexto de teste
typedef struct
{
    nes_cartridge_t cart;
    uint8_t prg_rom[64 * 1024]; // 64KB de PRG ROM
    uint8_t chr_ram[8 * 1024];  // 8KB de CHR RAM
    nes_mapper_t *mapper;
} test_context_t;

static test_context_t ctx;

void setUp(void)
{
    // Inicializa o contexto de teste
    memset(&ctx, 0, sizeof(test_context_t));

    // Configura o cartucho
    ctx.cart.prg_rom = ctx.prg_rom;
    ctx.cart.prg_rom_size = sizeof(ctx.prg_rom);
    ctx.cart.chr_ram = ctx.chr_ram;
    ctx.cart.chr_ram_size = sizeof(ctx.chr_ram);
    ctx.cart.mapper_number = 2;

    // Preenche PRG ROM com valores de teste
    for (int i = 0; i < sizeof(ctx.prg_rom); i++)
    {
        ctx.prg_rom[i] = i & 0xFF;
    }

    // Inicializa o mapper
    ctx.mapper = nes_mapper_2_init(&ctx.cart);
    TEST_ASSERT_NOT_NULL(ctx.mapper);
}

void tearDown(void)
{
    if (ctx.mapper)
    {
        ctx.mapper->shutdown(ctx.mapper->context);
        free(ctx.mapper);
        ctx.mapper = NULL;
    }
}

void test_mapper2_init(void)
{
    TEST_ASSERT_NOT_NULL(ctx.mapper);
    TEST_ASSERT_NOT_NULL(ctx.mapper->context);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->ppu_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->ppu_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->reset);
    TEST_ASSERT_NOT_NULL(ctx.mapper->shutdown);
}

void test_mapper2_cpu_read_fixed_bank(void)
{
    // Testa leitura do banco fixo ($C000-$FFFF)
    uint8_t val;

    // Deve ler do último banco (bank 3 em 64KB)
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0xC000);
    TEST_ASSERT_EQUAL_HEX8(0x00, val); // Offset 0x0000 do último banco

    val = ctx.mapper->cpu_read(ctx.mapper->context, 0xFFFF);
    TEST_ASSERT_EQUAL_HEX8(0xFF, val); // Último byte do último banco
}

void test_mapper2_cpu_read_switchable_bank(void)
{
    // Testa leitura do banco comutável ($8000-$BFFF)
    uint8_t val;

    // Inicialmente no banco 0
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_HEX8(0x00, val);

    // Troca para o banco 1
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 1);
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_HEX8(0x00, val); // Primeiro byte do banco 1

    val = ctx.mapper->cpu_read(ctx.mapper->context, 0xBFFF);
    TEST_ASSERT_EQUAL_HEX8(0xFF, val); // Último byte do banco 1
}

void test_mapper2_cpu_write_bank_switching(void)
{
    // Testa a troca de bancos
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 1); // Seleciona banco 1
    uint8_t val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_HEX8(0x00, val);

    ctx.mapper->cpu_write(ctx.mapper->context, 0xFFFF, 2); // Seleciona banco 2
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_HEX8(0x00, val);
}

void test_mapper2_ppu_read_write(void)
{
    // Testa leitura/escrita de CHR RAM
    uint8_t val;

    // Escreve na CHR RAM
    ctx.mapper->ppu_write(ctx.mapper->context, 0x0000, 0x42);
    ctx.mapper->ppu_write(ctx.mapper->context, 0x1FFF, 0x24);

    // Lê da CHR RAM
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_HEX8(0x42, val);

    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x1FFF);
    TEST_ASSERT_EQUAL_HEX8(0x24, val);
}

void test_mapper2_reset(void)
{
    // Testa o reset do mapper
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 2); // Seleciona banco 2
    ctx.mapper->reset(ctx.mapper->context);

    // Após reset, deve voltar ao banco 0
    uint8_t val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_HEX8(0x00, val);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper2_init);
    RUN_TEST(test_mapper2_cpu_read_fixed_bank);
    RUN_TEST(test_mapper2_cpu_read_switchable_bank);
    RUN_TEST(test_mapper2_cpu_write_bank_switching);
    RUN_TEST(test_mapper2_ppu_read_write);
    RUN_TEST(test_mapper2_reset);

    return UNITY_END();
}
