/**
 * @file test_mapper8.c
 * @brief Testes para o Mapper 8 (FFE F3xxx) do NES
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper8.h"
#include <stdlib.h>
#include <string.h>

// Mock para funções de log
#define EMU_LOG_ERROR(...)
#define EMU_LOG_WARN(...)
#define EMU_LOG_INFO(...)
#define EMU_LOG_DEBUG(...)
#define EMU_LOG_TRACE(...)

typedef struct
{
    nes_cartridge_t *cartridge;
    uint8_t *prg_rom;
    uint8_t *chr_rom;
} test_context_t;

static test_context_t ctx;

void setUp(void)
{
    // Configuração comum para cada teste
    ctx.cartridge = (nes_cartridge_t *)malloc(sizeof(nes_cartridge_t));
    TEST_ASSERT_NOT_NULL(ctx.cartridge);

    memset(ctx.cartridge, 0, sizeof(nes_cartridge_t));

    // Alocar PRG-ROM (64KB = 4 bancos de 16KB)
    ctx.prg_rom = (uint8_t *)malloc(64 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.prg_rom);

    // Preencher com padrão de teste: endereço & 0xFF
    for (int i = 0; i < 64 * 1024; i++)
    {
        ctx.prg_rom[i] = i & 0xFF;
    }

    // Alocar CHR-ROM (32KB = 4 bancos de 8KB)
    ctx.chr_rom = (uint8_t *)malloc(32 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.chr_rom);

    // Preencher com padrão de teste: endereço & 0xFF
    for (int i = 0; i < 32 * 1024; i++)
    {
        ctx.chr_rom[i] = (i + 128) & 0xFF;
    }

    // Configurar cartridge
    ctx.cartridge->prg_rom = ctx.prg_rom;
    ctx.cartridge->chr_rom = ctx.chr_rom;
    ctx.cartridge->prg_rom_size = 64 * 1024;
    ctx.cartridge->chr_rom_size = 32 * 1024;
    ctx.cartridge->mapper_number = 8;
    ctx.cartridge->mirror_mode = NES_MIRROR_HORIZONTAL;
}

void tearDown(void)
{
    // Liberar recursos após cada teste
    if (ctx.cartridge && ctx.cartridge->mapper)
    {
        if (ctx.cartridge->mapper->shutdown)
        {
            ctx.cartridge->mapper->shutdown(ctx.cartridge->mapper->context);
        }
        free(ctx.cartridge->mapper);
        ctx.cartridge->mapper = NULL;
    }

    if (ctx.prg_rom)
    {
        free(ctx.prg_rom);
        ctx.prg_rom = NULL;
    }

    if (ctx.chr_rom)
    {
        free(ctx.chr_rom);
        ctx.chr_rom = NULL;
    }

    if (ctx.cartridge)
    {
        free(ctx.cartridge);
        ctx.cartridge = NULL;
    }
}

void test_mapper8_init(void)
{
    // Testar inicialização do mapper
    nes_mapper_t *mapper = nes_mapper_8_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    TEST_ASSERT_EQUAL_INT(8, mapper->number);
    TEST_ASSERT_NOT_NULL(mapper->context);
    TEST_ASSERT_NOT_NULL(mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(mapper->ppu_read);
    TEST_ASSERT_NOT_NULL(mapper->ppu_write);
    TEST_ASSERT_NOT_NULL(mapper->reset);
    TEST_ASSERT_NOT_NULL(mapper->shutdown);

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper8_cpu_read(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_8_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar leitura da SRAM (0x6000-0x7FFF)
    ctx.cartridge->prg_ram = (uint8_t *)malloc(8192);
    memset(ctx.cartridge->prg_ram, 0xAA, 8192);
    ctx.cartridge->prg_ram_size = 8192;

    uint8_t value = mapper->cpu_read(mapper->context, 0x6000);
    TEST_ASSERT_EQUAL_UINT8(0xAA, value);

    // Testar leitura da PRG-ROM (0x8000-0xFFFF)
    // Banco 0 (padrão após inicialização)
    value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x00, value); // Primeiro byte do banco 0

    value = mapper->cpu_read(mapper->context, 0x8001);
    TEST_ASSERT_EQUAL_UINT8(0x01, value); // Segundo byte do banco 0

    // Mudar para banco 1
    mapper->cpu_write(mapper->context, 0x8000, 0x01);

    // Verificar leitura do banco 1
    value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x00, value); // Primeiro byte do banco 1

    value = mapper->cpu_read(mapper->context, 0xFFFF);
    TEST_ASSERT_EQUAL_UINT8(0xFF, value); // Último byte do banco 1

    // Liberar PRG-RAM
    free(ctx.cartridge->prg_ram);
    ctx.cartridge->prg_ram = NULL;
}

void test_mapper8_cpu_write(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_8_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar escrita na SRAM (0x6000-0x7FFF)
    ctx.cartridge->prg_ram = (uint8_t *)malloc(8192);
    memset(ctx.cartridge->prg_ram, 0, 8192);
    ctx.cartridge->prg_ram_size = 8192;

    mapper->cpu_write(mapper->context, 0x6000, 0xBB);
    TEST_ASSERT_EQUAL_UINT8(0xBB, ctx.cartridge->prg_ram[0]);

    // Testar escrita no registro de seleção de banco PRG
    mapper->cpu_write(mapper->context, 0x8000, 0x02); // Selecionar banco PRG 2

    // Verificar se o banco foi selecionado lendo o endereço 0x8000
    uint8_t value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x10000], value); // Banco 2 começa em 0x10000

    // Testar escrita no registro de seleção de banco CHR
    mapper->cpu_write(mapper->context, 0xB000, 0x02); // Selecionar banco CHR 2

    // Verificar se o banco foi selecionado lendo o endereço 0x0000
    value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x4000], value); // Banco 2 começa em 0x4000

    // Liberar PRG-RAM
    free(ctx.cartridge->prg_ram);
    ctx.cartridge->prg_ram = NULL;
}

void test_mapper8_ppu_read(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_8_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar leitura da CHR-ROM (0x0000-0x1FFF)
    // Banco 0 (padrão após inicialização)
    uint8_t value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0], value); // Primeiro byte do banco 0

    // Mudar para banco 1
    mapper->cpu_write(mapper->context, 0xB000, 0x01);

    // Verificar leitura do banco 1
    value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x2000], value); // Primeiro byte do banco 1

    value = mapper->ppu_read(mapper->context, 0x1FFF);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x3FFF], value); // Último byte do banco 1
}

void test_mapper8_ppu_write(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_8_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar escrita na CHR-RAM
    // Primeiro, liberar CHR-ROM e alocar CHR-RAM
    free(ctx.chr_rom);
    ctx.chr_rom = NULL;
    ctx.cartridge->chr_rom = NULL;
    ctx.cartridge->chr_rom_size = 0;

    ctx.cartridge->chr_ram = (uint8_t *)malloc(8192);
    memset(ctx.cartridge->chr_ram, 0, 8192);
    ctx.cartridge->chr_ram_size = 8192;

    // Testar escrita em CHR-RAM
    mapper->ppu_write(mapper->context, 0x0000, 0xCC);
    TEST_ASSERT_EQUAL_UINT8(0xCC, ctx.cartridge->chr_ram[0]);

    mapper->ppu_write(mapper->context, 0x1FFF, 0xDD);
    TEST_ASSERT_EQUAL_UINT8(0xDD, ctx.cartridge->chr_ram[0x1FFF]);

    // Liberar CHR-RAM
    free(ctx.cartridge->chr_ram);
    ctx.cartridge->chr_ram = NULL;
}

void test_mapper8_reset(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_8_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Mudar alguns valores
    mapper->cpu_write(mapper->context, 0x8000, 0x03); // Banco PRG 3
    mapper->cpu_write(mapper->context, 0xB000, 0x02); // Banco CHR 2

    // Verificar se os valores foram alterados
    uint8_t prg_value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x18000], prg_value); // Banco 3 começa em 0x18000

    uint8_t chr_value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x4000], chr_value); // Banco 2 começa em 0x4000

    // Resetar o mapper
    mapper->reset(mapper->context);

    // Verificar se os valores voltaram ao padrão
    prg_value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0], prg_value); // Banco 0 começa em 0x0

    chr_value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0], chr_value); // Banco 0 começa em 0x0
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper8_init);
    RUN_TEST(test_mapper8_cpu_read);
    RUN_TEST(test_mapper8_cpu_write);
    RUN_TEST(test_mapper8_ppu_read);
    RUN_TEST(test_mapper8_ppu_write);
    RUN_TEST(test_mapper8_reset);

    return UNITY_END();
}
