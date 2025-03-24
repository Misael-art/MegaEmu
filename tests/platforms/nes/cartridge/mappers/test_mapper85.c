/**
 * @file test_mapper85.c
 * @brief Testes para o Mapper 85 (VRC7)
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper85.h"
#include <stdlib.h>
#include <string.h>

// Mock para funções de log
#define EMU_LOG_CAT_NES_MAPPERS 0
#define EMU_LOG_ERROR(cat, fmt, ...) ((void)0)
#define EMU_LOG_WARN(cat, fmt, ...) ((void)0)
#define EMU_LOG_INFO(cat, fmt, ...) ((void)0)
#define EMU_LOG_DEBUG(cat, fmt, ...) ((void)0)
#define EMU_LOG_TRACE(cat, fmt, ...) ((void)0)

// Estrutura para o contexto de teste
typedef struct
{
    nes_cartridge_t *cart;
    nes_mapper_t *mapper;
    uint8_t prg_rom[128 * 1024]; // 128KB PRG-ROM
    uint8_t chr_rom[128 * 1024]; // 128KB CHR-ROM
    uint8_t prg_ram[8 * 1024];   // 8KB PRG-RAM
} test_context_t;

static test_context_t ctx;

void setUp(void)
{
    // Inicializa o contexto de teste
    memset(&ctx, 0, sizeof(test_context_t));

    // Cria um cartucho de teste
    ctx.cart = (nes_cartridge_t *)malloc(sizeof(nes_cartridge_t));
    TEST_ASSERT_NOT_NULL(ctx.cart);
    memset(ctx.cart, 0, sizeof(nes_cartridge_t));

    // Configura o cartucho
    ctx.cart->mapper_number = 85;
    ctx.cart->prg_rom = ctx.prg_rom;
    ctx.cart->chr_rom = ctx.chr_rom;
    ctx.cart->prg_ram = ctx.prg_ram;
    ctx.cart->prg_rom_size = sizeof(ctx.prg_rom);
    ctx.cart->chr_rom_size = sizeof(ctx.chr_rom);
    ctx.cart->prg_ram_size = sizeof(ctx.prg_ram);

    // Preenche as ROMs com padrões de teste
    for (size_t i = 0; i < sizeof(ctx.prg_rom); i++)
    {
        ctx.prg_rom[i] = i & 0xFF;
    }
    for (size_t i = 0; i < sizeof(ctx.chr_rom); i++)
    {
        ctx.chr_rom[i] = (i * 2) & 0xFF;
    }

    // Inicializa o mapper
    ctx.mapper = nes_mapper_85_init(ctx.cart);
    TEST_ASSERT_NOT_NULL(ctx.mapper);
}

void tearDown(void)
{
    if (ctx.mapper)
    {
        if (ctx.mapper->shutdown)
        {
            ctx.mapper->shutdown(ctx.mapper->context);
        }
        free(ctx.mapper);
    }
    if (ctx.cart)
    {
        free(ctx.cart);
    }
}

void test_mapper85_init(void)
{
    TEST_ASSERT_EQUAL(85, ctx.mapper->number);
    TEST_ASSERT_EQUAL_STRING("VRC7", ctx.mapper->name);
    TEST_ASSERT_NOT_NULL(ctx.mapper->context);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->ppu_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->ppu_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->scanline);
    TEST_ASSERT_NOT_NULL(ctx.mapper->reset);
    TEST_ASSERT_NOT_NULL(ctx.mapper->shutdown);
}

void test_mapper85_cpu_read_prg_rom(void)
{
    // Testa leitura do primeiro banco de PRG-ROM (8000-9FFF)
    uint8_t val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL(0x00, val);

    val = ctx.mapper->cpu_read(ctx.mapper->context, 0x9FFF);
    TEST_ASSERT_EQUAL(0xFF, val & 0xFF);

    // Testa leitura do segundo banco de PRG-ROM (A000-BFFF)
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0xA000);
    TEST_ASSERT_EQUAL(0x00, val);

    // Testa leitura do terceiro banco de PRG-ROM (C000-DFFF)
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0xC000);
    TEST_ASSERT_EQUAL(0x00, val);

    // Testa leitura do último banco fixo (E000-FFFF)
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0xE000);
    TEST_ASSERT_EQUAL(ctx.prg_rom[ctx.cart->prg_rom_size - 0x2000], val);
}

void test_mapper85_cpu_read_prg_ram(void)
{
    // Testa leitura da PRG-RAM (6000-7FFF)
    ctx.prg_ram[0] = 0x42;
    uint8_t val = ctx.mapper->cpu_read(ctx.mapper->context, 0x6000);
    TEST_ASSERT_EQUAL(0x42, val);

    ctx.prg_ram[0x1FFF] = 0x24;
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0x7FFF);
    TEST_ASSERT_EQUAL(0x24, val);
}

void test_mapper85_cpu_write_prg_ram(void)
{
    // Testa escrita na PRG-RAM (6000-7FFF)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0x42);
    TEST_ASSERT_EQUAL(0x42, ctx.prg_ram[0]);

    ctx.mapper->cpu_write(ctx.mapper->context, 0x7FFF, 0x24);
    TEST_ASSERT_EQUAL(0x24, ctx.prg_ram[0x1FFF]);
}

void test_mapper85_cpu_write_registers(void)
{
    // Testa escrita nos registradores de banco PRG
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x01); // PRG banco 0
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8010, 0x02); // PRG banco 1
    ctx.mapper->cpu_write(ctx.mapper->context, 0x9000, 0x03); // PRG banco 2

    // Testa escrita nos registradores de banco CHR
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0x00); // CHR banco 0
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA010, 0x01); // CHR banco 1
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB000, 0x02); // CHR banco 2
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB010, 0x03); // CHR banco 3
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC000, 0x04); // CHR banco 4
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC010, 0x05); // CHR banco 5
    ctx.mapper->cpu_write(ctx.mapper->context, 0xD000, 0x06); // CHR banco 6
    ctx.mapper->cpu_write(ctx.mapper->context, 0xD010, 0x07); // CHR banco 7

    // Testa escrita nos registradores de IRQ
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE000, 0x42); // IRQ latch
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE010, 0x03); // IRQ control
    ctx.mapper->cpu_write(ctx.mapper->context, 0xF000, 0x00); // IRQ acknowledge

    // Testa escrita nos registradores de som
    ctx.mapper->cpu_write(ctx.mapper->context, 0x9010, 0x01); // Sound register address
    ctx.mapper->cpu_write(ctx.mapper->context, 0x9030, 0x42); // Sound register data
}

void test_mapper85_ppu_read(void)
{
    // Configura os bancos CHR
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0x00); // CHR banco 0
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA010, 0x01); // CHR banco 1
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB000, 0x02); // CHR banco 2
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB010, 0x03); // CHR banco 3
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC000, 0x04); // CHR banco 4
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC010, 0x05); // CHR banco 5
    ctx.mapper->cpu_write(ctx.mapper->context, 0xD000, 0x06); // CHR banco 6
    ctx.mapper->cpu_write(ctx.mapper->context, 0xD010, 0x07); // CHR banco 7

    // Testa leitura de cada banco CHR
    uint8_t val;

    // Banco 0 (0000-03FF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x0000);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0], val);

    // Banco 1 (0400-07FF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x0400);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x400], val);

    // Banco 2 (0800-0BFF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x0800);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x800], val);

    // Banco 3 (0C00-0FFF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x0C00);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0xC00], val);

    // Banco 4 (1000-13FF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x1000);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x1000], val);

    // Banco 5 (1400-17FF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x1400);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x1400], val);

    // Banco 6 (1800-1BFF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x1800);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x1800], val);

    // Banco 7 (1C00-1FFF)
    val = ctx.mapper->ppu_read(ctx.mapper->context, 0x1C00);
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x1C00], val);
}

void test_mapper85_ppu_write(void)
{
    // VRC7 não tem CHR-RAM, então escritas devem ser ignoradas
    uint8_t original = ctx.chr_rom[0];
    ctx.mapper->ppu_write(ctx.mapper->context, 0x0000, 0x42);
    TEST_ASSERT_EQUAL(original, ctx.chr_rom[0]);
}

void test_mapper85_scanline(void)
{
    // Configura o IRQ
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE000, 0x42); // IRQ latch
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE010, 0x03); // IRQ control (enabled)

    // Simula vários scanlines
    for (int i = 0; i < 0xFF; i++)
    {
        ctx.mapper->scanline(ctx.mapper->context);
    }

    // O IRQ deve ter sido acionado
    ctx.mapper->scanline(ctx.mapper->context);
}

void test_mapper85_reset(void)
{
    // Configura alguns registradores
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x01); // PRG banco 0
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE000, 0x42); // IRQ latch
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE010, 0x03); // IRQ control
    ctx.mapper->cpu_write(ctx.mapper->context, 0x9010, 0x01); // Sound register address
    ctx.mapper->cpu_write(ctx.mapper->context, 0x9030, 0x42); // Sound register data

    // Reseta o mapper
    ctx.mapper->reset(ctx.mapper->context);

    // Verifica se os registradores foram resetados
    uint8_t val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL(ctx.prg_rom[0], val);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper85_init);
    RUN_TEST(test_mapper85_cpu_read_prg_rom);
    RUN_TEST(test_mapper85_cpu_read_prg_ram);
    RUN_TEST(test_mapper85_cpu_write_prg_ram);
    RUN_TEST(test_mapper85_cpu_write_registers);
    RUN_TEST(test_mapper85_ppu_read);
    RUN_TEST(test_mapper85_ppu_write);
    RUN_TEST(test_mapper85_scanline);
    RUN_TEST(test_mapper85_reset);

    return UNITY_END();
}
