/**
 * @file test_mapper10.c
 * @brief Testes para o Mapper 10 (MMC4/FxROM) do NES
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper10.h"
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

    // Alocar PRG-ROM (128KB = 8 bancos de 16KB)
    ctx.prg_rom = (uint8_t *)malloc(128 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.prg_rom);

    // Preencher com padrão de teste: endereço & 0xFF
    for (int i = 0; i < 128 * 1024; i++)
    {
        ctx.prg_rom[i] = i & 0xFF;
    }

    // Alocar CHR-ROM (128KB = 32 bancos de 4KB)
    ctx.chr_rom = (uint8_t *)malloc(128 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.chr_rom);

    // Preencher com padrão de teste: (endereço + 128) & 0xFF
    for (int i = 0; i < 128 * 1024; i++)
    {
        ctx.chr_rom[i] = (i + 128) & 0xFF;
    }

    // Configurar cartridge
    ctx.cartridge->prg_rom = ctx.prg_rom;
    ctx.cartridge->chr_rom = ctx.chr_rom;
    ctx.cartridge->prg_rom_size = 128 * 1024;
    ctx.cartridge->chr_rom_size = 128 * 1024;
    ctx.cartridge->mapper_number = 10;
    ctx.cartridge->mirror_mode = NES_MIRROR_HORIZONTAL; // Padrão inicial
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

void test_mapper10_init(void)
{
    // Testar inicialização do mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    TEST_ASSERT_EQUAL_INT(10, mapper->number);
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

void test_mapper10_cpu_read(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar leitura da SRAM (0x6000-0x7FFF)
    ctx.cartridge->prg_ram = (uint8_t *)malloc(8192);
    memset(ctx.cartridge->prg_ram, 0xAA, 8192);
    ctx.cartridge->prg_ram_size = 8192;

    uint8_t value = mapper->cpu_read(mapper->context, 0x6000);
    TEST_ASSERT_EQUAL_UINT8(0xAA, value);

    // Testar leitura da PRG-ROM (0x8000-0xFFFF)
    // O primeiro banco (0x8000-0xBFFF) deve ser selecionável
    value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x00, value); // Primeiro byte do banco 0

    // Mudar para banco 1 (registrador 0xA000)
    mapper->cpu_write(mapper->context, 0xA000, 0x01);

    // Verificar leitura do banco 1 (0x8000-0xBFFF)
    value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x8000], value); // Primeiro byte do banco 1

    // Último banco fixo (0xC000-0xFFFF)
    value = mapper->cpu_read(mapper->context, 0xFFFF);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x1FFFF], value); // Último byte

    // Liberar PRG-RAM
    free(ctx.cartridge->prg_ram);
    ctx.cartridge->prg_ram = NULL;
}

void test_mapper10_cpu_write(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar escrita na SRAM (0x6000-0x7FFF)
    ctx.cartridge->prg_ram = (uint8_t *)malloc(8192);
    memset(ctx.cartridge->prg_ram, 0, 8192);
    ctx.cartridge->prg_ram_size = 8192;

    mapper->cpu_write(mapper->context, 0x6000, 0xBB);
    TEST_ASSERT_EQUAL_UINT8(0xBB, ctx.cartridge->prg_ram[0]);

    // Testar escrita nos registros de controle
    // Selecionar banco PRG (0xA000)
    mapper->cpu_write(mapper->context, 0xA000, 0x03);

    // Verificar se o banco foi selecionado lendo o endereço 0x8000
    uint8_t value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x18000], value); // Banco 3 começa em 0x18000

    // Selecionar banco CHR 0 FD (0xB000)
    mapper->cpu_write(mapper->context, 0xB000, 0x05);

    // Selecionar banco CHR 0 FE (0xC000)
    mapper->cpu_write(mapper->context, 0xC000, 0x06);

    // Selecionar banco CHR 1 FD (0xD000)
    mapper->cpu_write(mapper->context, 0xD000, 0x07);

    // Selecionar banco CHR 1 FE (0xE000)
    mapper->cpu_write(mapper->context, 0xE000, 0x08);

    // Configurar espelhamento (0xF000)
    mapper->cpu_write(mapper->context, 0xF000, 0x01); // Vertical

    // Liberar PRG-RAM
    free(ctx.cartridge->prg_ram);
    ctx.cartridge->prg_ram = NULL;
}

void test_mapper10_ppu_read(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Configurar os bancos de pattern table
    mapper->cpu_write(mapper->context, 0xB000, 0x01); // CHR 0 FD = banco 1 (0x1000-0x1FFF)
    mapper->cpu_write(mapper->context, 0xC000, 0x02); // CHR 0 FE = banco 2 (0x2000-0x2FFF)
    mapper->cpu_write(mapper->context, 0xD000, 0x03); // CHR 1 FD = banco 3 (0x3000-0x3FFF)
    mapper->cpu_write(mapper->context, 0xE000, 0x04); // CHR 1 FE = banco 4 (0x4000-0x4FFF)

    // Leitura da Pattern Table 0 com latch FD ($FD no tile)
    uint8_t value = mapper->ppu_read(mapper->context, 0x0FD0);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x1000 + 0x0FD0], value); // Pattern 0, banco 1 (FD)

    // Leitura da Pattern Table 0 com latch FE ($FE no tile)
    value = mapper->ppu_read(mapper->context, 0x0FE0);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x2000 + 0x0FE0], value); // Pattern 0, banco 2 (FE)

    // Leitura da Pattern Table 1 com latch FD ($1FD no tile)
    value = mapper->ppu_read(mapper->context, 0x1FD0);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x3000 + 0x0FD0], value); // Pattern 1, banco 3 (FD)

    // Leitura da Pattern Table 1 com latch FE ($1FE no tile)
    value = mapper->ppu_read(mapper->context, 0x1FE0);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x4000 + 0x0FE0], value); // Pattern 1, banco 4 (FE)
}

void test_mapper10_ppu_write(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
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

void test_mapper10_latch_mechanism(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Configurar os bancos de pattern table
    mapper->cpu_write(mapper->context, 0xB000, 0x01); // CHR 0 FD = banco 1
    mapper->cpu_write(mapper->context, 0xC000, 0x02); // CHR 0 FE = banco 2
    mapper->cpu_write(mapper->context, 0xD000, 0x03); // CHR 1 FD = banco 3
    mapper->cpu_write(mapper->context, 0xE000, 0x04); // CHR 1 FE = banco 4

    // Validar que o mecanismo de latch funciona específicamente para o MMC4
    // (diferente do MMC2 do Mapper 9)

    // Leitura da Pattern Table 0 (0x0000-0x0FFF)
    // Inicialmente deve usar o banco 0
    uint8_t value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0], value);

    // Ativar latch FD
    mapper->ppu_read(mapper->context, 0x0FD8);
    value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x1000], value); // Banco 1 (FD)

    // Ativar latch FE
    mapper->ppu_read(mapper->context, 0x0FE8);
    value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x2000], value); // Banco 2 (FE)

    // Leitura da Pattern Table 1 (0x1000-0x1FFF)
    // Inicialmente deve usar um banco padrão
    value = mapper->ppu_read(mapper->context, 0x1000);

    // Ativar latch FD para a segunda pattern table
    mapper->ppu_read(mapper->context, 0x1FD8);
    value = mapper->ppu_read(mapper->context, 0x1000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x3000], value); // Banco 3 (FD)

    // Ativar latch FE para a segunda pattern table
    mapper->ppu_read(mapper->context, 0x1FE8);
    value = mapper->ppu_read(mapper->context, 0x1000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x4000], value); // Banco 4 (FE)
}

void test_mapper10_mirror_control(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Por padrão, inicializa com espelhamento horizontal
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_HORIZONTAL, ctx.cartridge->mirror_mode);

    // Mudar para espelhamento vertical (0xF000 = 1)
    mapper->cpu_write(mapper->context, 0xF000, 0x01);
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_VERTICAL, ctx.cartridge->mirror_mode);

    // Mudar para espelhamento horizontal (0xF000 = 0)
    mapper->cpu_write(mapper->context, 0xF000, 0x00);
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_HORIZONTAL, ctx.cartridge->mirror_mode);
}

void test_mapper10_reset(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_10_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Mudar alguns valores
    mapper->cpu_write(mapper->context, 0xA000, 0x03); // Banco PRG
    mapper->cpu_write(mapper->context, 0xB000, 0x05); // CHR 0 FD
    mapper->cpu_write(mapper->context, 0xC000, 0x06); // CHR 0 FE
    mapper->cpu_write(mapper->context, 0xD000, 0x07); // CHR 1 FD
    mapper->cpu_write(mapper->context, 0xE000, 0x08); // CHR 1 FE
    mapper->cpu_write(mapper->context, 0xF000, 0x01); // Espelhamento vertical

    // Resetar o mapper
    mapper->reset(mapper->context);

    // Verificar se os valores voltaram ao padrão
    uint8_t value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0], value); // Banco 0 após reset

    // Espelhamento deve voltar ao valor inicial (horizontal)
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_HORIZONTAL, ctx.cartridge->mirror_mode);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper10_init);
    RUN_TEST(test_mapper10_cpu_read);
    RUN_TEST(test_mapper10_cpu_write);
    RUN_TEST(test_mapper10_ppu_read);
    RUN_TEST(test_mapper10_ppu_write);
    RUN_TEST(test_mapper10_latch_mechanism);
    RUN_TEST(test_mapper10_mirror_control);
    RUN_TEST(test_mapper10_reset);

    return UNITY_END();
}
