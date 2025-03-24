/**
 * @file test_mapper5.c
 * @brief Testes para o Mapper 5 (MMC5) do NES
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper5.h"
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
    uint8_t *prg_ram;
    uint8_t *chr_ram;
} test_context_t;

static test_context_t ctx;

void setUp(void)
{
    // Configuração comum para cada teste
    ctx.cartridge = (nes_cartridge_t *)malloc(sizeof(nes_cartridge_t));
    TEST_ASSERT_NOT_NULL(ctx.cartridge);

    memset(ctx.cartridge, 0, sizeof(nes_cartridge_t));

    // Alocar PRG-ROM (1MB = 64 bancos de 16KB)
    ctx.prg_rom = (uint8_t *)malloc(1024 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.prg_rom);

    // Preencher com padrão de teste: endereço & 0xFF
    for (int i = 0; i < 1024 * 1024; i++)
    {
        ctx.prg_rom[i] = i & 0xFF;
    }

    // Alocar CHR-ROM (1MB = 256 bancos de 4KB)
    ctx.chr_rom = (uint8_t *)malloc(1024 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.chr_rom);

    // Preencher com padrão de teste: (endereço + 128) & 0xFF
    for (int i = 0; i < 1024 * 1024; i++)
    {
        ctx.chr_rom[i] = (i + 128) & 0xFF;
    }

    // Alocar PRG-RAM (64KB)
    ctx.prg_ram = (uint8_t *)malloc(64 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.prg_ram);
    memset(ctx.prg_ram, 0, 64 * 1024);

    // Alocar CHR-RAM (32KB)
    ctx.chr_ram = (uint8_t *)malloc(32 * 1024);
    TEST_ASSERT_NOT_NULL(ctx.chr_ram);
    memset(ctx.chr_ram, 0, 32 * 1024);

    // Configurar cartridge
    ctx.cartridge->prg_rom = ctx.prg_rom;
    ctx.cartridge->chr_rom = ctx.chr_rom;
    ctx.cartridge->prg_ram = ctx.prg_ram;
    ctx.cartridge->chr_ram = ctx.chr_ram;
    ctx.cartridge->prg_rom_size = 1024 * 1024;
    ctx.cartridge->chr_rom_size = 1024 * 1024;
    ctx.cartridge->prg_ram_size = 64 * 1024;
    ctx.cartridge->chr_ram_size = 32 * 1024;
    ctx.cartridge->mapper_number = 5;
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

    if (ctx.prg_ram)
    {
        free(ctx.prg_ram);
        ctx.prg_ram = NULL;
    }

    if (ctx.chr_ram)
    {
        free(ctx.chr_ram);
        ctx.chr_ram = NULL;
    }

    if (ctx.cartridge)
    {
        free(ctx.cartridge);
        ctx.cartridge = NULL;
    }
}

void test_mapper5_init(void)
{
    // Testar inicialização do mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    TEST_ASSERT_EQUAL_INT(5, mapper->number);
    TEST_ASSERT_EQUAL_STRING("MMC5", mapper->name);
    TEST_ASSERT_NOT_NULL(mapper->context);
    TEST_ASSERT_NOT_NULL(mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(mapper->ppu_read);
    TEST_ASSERT_NOT_NULL(mapper->ppu_write);
    TEST_ASSERT_NOT_NULL(mapper->scanline);
    TEST_ASSERT_NOT_NULL(mapper->reset);
    TEST_ASSERT_NOT_NULL(mapper->shutdown);

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper5_prg_modes(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar modo 0 (32KB)
    mapper->cpu_write(mapper->context, 0x5100, 0); // PRG mode 0
    mapper->cpu_write(mapper->context, 0x5117, 4); // Banco 4 em $8000-$FFFF

    uint8_t value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x20000], value); // Banco 4 * 32KB

    // Testar modo 1 (16KB + 16KB)
    mapper->cpu_write(mapper->context, 0x5100, 1); // PRG mode 1
    mapper->cpu_write(mapper->context, 0x5115, 2); // Banco 2 em $8000-$BFFF
    mapper->cpu_write(mapper->context, 0x5117, 3); // Banco 3 em $C000-$FFFF

    value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x10000], value); // Banco 2 * 16KB

    value = mapper->cpu_read(mapper->context, 0xC000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x18000], value); // Banco 3 * 16KB

    // Testar modo 2 (16KB + 8KB + 8KB)
    mapper->cpu_write(mapper->context, 0x5100, 2); // PRG mode 2
    mapper->cpu_write(mapper->context, 0x5115, 2); // Banco 2 em $8000-$BFFF
    mapper->cpu_write(mapper->context, 0x5116, 4); // Banco 4 em $C000-$DFFF
    mapper->cpu_write(mapper->context, 0x5117, 5); // Banco 5 em $E000-$FFFF

    value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x10000], value); // Banco 2 * 16KB

    value = mapper->cpu_read(mapper->context, 0xC000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x8000], value); // Banco 4 * 8KB

    value = mapper->cpu_read(mapper->context, 0xE000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0xA000], value); // Banco 5 * 8KB

    // Testar modo 3 (8KB + 8KB + 8KB + 8KB)
    mapper->cpu_write(mapper->context, 0x5100, 3); // PRG mode 3
    mapper->cpu_write(mapper->context, 0x5113, 1); // Banco 1 em $8000-$9FFF
    mapper->cpu_write(mapper->context, 0x5114, 2); // Banco 2 em $A000-$BFFF
    mapper->cpu_write(mapper->context, 0x5115, 3); // Banco 3 em $C000-$DFFF
    mapper->cpu_write(mapper->context, 0x5116, 4); // Banco 4 em $E000-$FFFF

    value = mapper->cpu_read(mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x2000], value); // Banco 1 * 8KB

    value = mapper->cpu_read(mapper->context, 0xA000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x4000], value); // Banco 2 * 8KB

    value = mapper->cpu_read(mapper->context, 0xC000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x6000], value); // Banco 3 * 8KB

    value = mapper->cpu_read(mapper->context, 0xE000);
    TEST_ASSERT_EQUAL_UINT8(ctx.prg_rom[0x8000], value); // Banco 4 * 8KB

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper5_chr_modes(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar modo 0 (8KB)
    mapper->cpu_write(mapper->context, 0x5101, 0); // CHR mode 0
    mapper->cpu_write(mapper->context, 0x5127, 4); // Banco 4 em $0000-$1FFF

    uint8_t value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x8000], value); // Banco 4 * 8KB

    // Testar modo 1 (4KB + 4KB)
    mapper->cpu_write(mapper->context, 0x5101, 1); // CHR mode 1
    mapper->cpu_write(mapper->context, 0x5123, 2); // Banco 2 em $0000-$0FFF
    mapper->cpu_write(mapper->context, 0x5127, 3); // Banco 3 em $1000-$1FFF

    value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x2000], value); // Banco 2 * 4KB

    value = mapper->ppu_read(mapper->context, 0x1000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x3000], value); // Banco 3 * 4KB

    // Testar modo 2 (2KB + 2KB + 2KB + 2KB)
    mapper->cpu_write(mapper->context, 0x5101, 2); // CHR mode 2
    mapper->cpu_write(mapper->context, 0x5120, 1); // Banco 1 em $0000-$07FF
    mapper->cpu_write(mapper->context, 0x5121, 2); // Banco 2 em $0800-$0FFF
    mapper->cpu_write(mapper->context, 0x5122, 3); // Banco 3 em $1000-$17FF
    mapper->cpu_write(mapper->context, 0x5123, 4); // Banco 4 em $1800-$1FFF

    value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x0800], value); // Banco 1 * 2KB

    value = mapper->ppu_read(mapper->context, 0x0800);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x1000], value); // Banco 2 * 2KB

    value = mapper->ppu_read(mapper->context, 0x1000);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x1800], value); // Banco 3 * 2KB

    value = mapper->ppu_read(mapper->context, 0x1800);
    TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[0x2000], value); // Banco 4 * 2KB

    // Testar modo 3 (1KB x 8)
    mapper->cpu_write(mapper->context, 0x5101, 3); // CHR mode 3
    for (int i = 0; i < 8; i++)
    {
        mapper->cpu_write(mapper->context, 0x5120 + i, i + 1);
    }

    for (int i = 0; i < 8; i++)
    {
        value = mapper->ppu_read(mapper->context, i * 0x400);
        TEST_ASSERT_EQUAL_UINT8(ctx.chr_rom[(i + 1) * 0x400], value);
    }

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper5_prg_ram(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar escrita/leitura na PRG-RAM
    mapper->cpu_write(mapper->context, 0x6000, 0xAA);
    uint8_t value = mapper->cpu_read(mapper->context, 0x6000);
    TEST_ASSERT_EQUAL_UINT8(0xAA, value);

    mapper->cpu_write(mapper->context, 0x7FFF, 0xBB);
    value = mapper->cpu_read(mapper->context, 0x7FFF);
    TEST_ASSERT_EQUAL_UINT8(0xBB, value);

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper5_chr_ram(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Desabilitar CHR-ROM temporariamente
    ctx.cartridge->chr_rom = NULL;
    ctx.cartridge->chr_rom_size = 0;

    // Testar escrita/leitura na CHR-RAM
    mapper->ppu_write(mapper->context, 0x0000, 0xCC);
    uint8_t value = mapper->ppu_read(mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(0xCC, value);

    mapper->ppu_write(mapper->context, 0x1FFF, 0xDD);
    value = mapper->ppu_read(mapper->context, 0x1FFF);
    TEST_ASSERT_EQUAL_UINT8(0xDD, value);

    // Restaurar CHR-ROM
    ctx.cartridge->chr_rom = ctx.chr_rom;
    ctx.cartridge->chr_rom_size = 1024 * 1024;

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper5_irq(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Configurar IRQ
    mapper->cpu_write(mapper->context, 0x5203, 100);  // IRQ scanline = 100
    mapper->cpu_write(mapper->context, 0x5204, 0x80); // Habilitar IRQ

    // Simular scanlines
    for (int i = 0; i < 99; i++)
    {
        mapper->scanline(mapper->context);
        uint8_t status = mapper->cpu_read(mapper->context, 0x5204);
        TEST_ASSERT_EQUAL_UINT8(0x00, status & 0x80);
    }

    // Scanline 100 deve gerar IRQ
    mapper->scanline(mapper->context);
    uint8_t status = mapper->cpu_read(mapper->context, 0x5204);
    TEST_ASSERT_EQUAL_UINT8(0x80, status & 0x80);

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper5_multiplier(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Testar multiplicador
    mapper->cpu_write(mapper->context, 0x5205, 10); // Multiplicando
    mapper->cpu_write(mapper->context, 0x5206, 20); // Multiplicador

    uint8_t result_low = mapper->cpu_read(mapper->context, 0x5205);
    uint8_t result_high = mapper->cpu_read(mapper->context, 0x5206);
    uint16_t result = (result_high << 8) | result_low;

    TEST_ASSERT_EQUAL_UINT16(200, result); // 10 * 20 = 200

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

void test_mapper5_reset(void)
{
    // Inicializar mapper
    nes_mapper_t *mapper = nes_mapper_5_init(ctx.cartridge);
    TEST_ASSERT_NOT_NULL(mapper);
    ctx.cartridge->mapper = mapper;

    // Configurar alguns valores
    mapper->cpu_write(mapper->context, 0x5100, 1);    // PRG mode 1
    mapper->cpu_write(mapper->context, 0x5101, 2);    // CHR mode 2
    mapper->cpu_write(mapper->context, 0x5203, 100);  // IRQ scanline
    mapper->cpu_write(mapper->context, 0x5204, 0x80); // IRQ enable
    mapper->cpu_write(mapper->context, 0x5205, 10);   // Multiplicando
    mapper->cpu_write(mapper->context, 0x5206, 20);   // Multiplicador

    // Resetar
    mapper->reset(mapper->context);

    // Verificar valores após reset
    uint8_t prg_mode = mapper->cpu_read(mapper->context, 0x5100);
    uint8_t chr_mode = mapper->cpu_read(mapper->context, 0x5101);
    uint8_t irq_scanline = mapper->cpu_read(mapper->context, 0x5203);
    uint8_t irq_status = mapper->cpu_read(mapper->context, 0x5204);
    uint8_t mult_a = mapper->cpu_read(mapper->context, 0x5205);
    uint8_t mult_b = mapper->cpu_read(mapper->context, 0x5206);

    TEST_ASSERT_EQUAL_UINT8(3, prg_mode); // Modo padrão é 3
    TEST_ASSERT_EQUAL_UINT8(3, chr_mode); // Modo padrão é 3
    TEST_ASSERT_EQUAL_UINT8(0, irq_scanline);
    TEST_ASSERT_EQUAL_UINT8(0, irq_status);
    TEST_ASSERT_EQUAL_UINT8(0, mult_a);
    TEST_ASSERT_EQUAL_UINT8(0, mult_b);

    // Limpar
    mapper->shutdown(mapper->context);
    free(mapper);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper5_init);
    RUN_TEST(test_mapper5_prg_modes);
    RUN_TEST(test_mapper5_chr_modes);
    RUN_TEST(test_mapper5_prg_ram);
    RUN_TEST(test_mapper5_chr_ram);
    RUN_TEST(test_mapper5_irq);
    RUN_TEST(test_mapper5_multiplier);
    RUN_TEST(test_mapper5_reset);

    return UNITY_END();
}
