/**
 * @file test_mapper3.c
 * @brief Testes para o Mapper 3 (CNROM) do NES
 */

#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper3.h"

// Mock functions para simulação de log
#define EMU_LOG_ERROR(cat, ...) ((void)0)
#define EMU_LOG_WARN(cat, ...) ((void)0)
#define EMU_LOG_INFO(cat, ...) ((void)0)
#define EMU_LOG_DEBUG(cat, ...) ((void)0)
#define EMU_LOG_TRACE(cat, ...) ((void)0)

// Estrutura de contexto para os testes
typedef struct
{
    nes_cartridge_t cartridge;
    nes_mapper_t *mapper;
    uint8_t prg_rom[32 * 1024]; // 32KB PRG-ROM
    uint8_t chr_rom[32 * 1024]; // 32KB CHR-ROM (4 bancos de 8KB)
    uint8_t prg_ram[8 * 1024];  // 8KB PRG-RAM
} test_context_t;

static test_context_t g_ctx;

// Configuração para cada teste
void setUp(void)
{
    // Inicializa a estrutura de contexto
    memset(&g_ctx, 0, sizeof(test_context_t));

    // Preenche a PRG-ROM com padrão reconhecível
    for (int i = 0; i < sizeof(g_ctx.prg_rom); i++)
    {
        g_ctx.prg_rom[i] = (i & 0xFF);
    }

    // Preenche a CHR-ROM com padrão reconhecível por banco
    for (int bank = 0; bank < 4; bank++)
    {
        for (int i = 0; i < 8 * 1024; i++)
        {
            g_ctx.chr_rom[bank * 8 * 1024 + i] = ((bank << 4) | (i & 0xF));
        }
    }

    // Configura o cartucho
    g_ctx.cartridge.prg_rom = g_ctx.prg_rom;
    g_ctx.cartridge.prg_rom_size = sizeof(g_ctx.prg_rom);
    g_ctx.cartridge.chr_rom = g_ctx.chr_rom;
    g_ctx.cartridge.chr_rom_size = sizeof(g_ctx.chr_rom);
    g_ctx.cartridge.prg_ram = g_ctx.prg_ram;
    g_ctx.cartridge.prg_ram_size = sizeof(g_ctx.prg_ram);
    g_ctx.cartridge.chr_ram = NULL;
    g_ctx.cartridge.chr_ram_size = 0;
    g_ctx.cartridge.mapper_number = 3;

    // Inicializa o mapper
    g_ctx.mapper = nes_mapper_3_init(&g_ctx.cartridge);
}

// Limpeza após cada teste
void tearDown(void)
{
    if (g_ctx.mapper)
    {
        if (g_ctx.mapper->shutdown)
        {
            g_ctx.mapper->shutdown(g_ctx.mapper->context);
        }
        free(g_ctx.mapper);
        g_ctx.mapper = NULL;
    }
}

/**
 * @brief Testa a inicialização do Mapper 3
 */
void test_mapper3_init(void)
{
    TEST_ASSERT_NOT_NULL(g_ctx.mapper);
    TEST_ASSERT_EQUAL_INT(3, g_ctx.mapper->mapper_number);
    TEST_ASSERT_NOT_NULL(g_ctx.mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(g_ctx.mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(g_ctx.mapper->chr_read);
    TEST_ASSERT_NOT_NULL(g_ctx.mapper->chr_write);
    TEST_ASSERT_NOT_NULL(g_ctx.mapper->reset);
    TEST_ASSERT_NOT_NULL(g_ctx.mapper->shutdown);
    TEST_ASSERT_NOT_NULL(g_ctx.mapper->context);
}

/**
 * @brief Testa a leitura da CPU para o Mapper 3
 */
void test_mapper3_cpu_read(void)
{
    // Testa leitura de PRG-ROM em $8000-$FFFF
    for (uint16_t addr = 0x8000; addr < 0x10000; addr += 0x1000)
    {
        uint8_t expected = (addr - 0x8000) & 0xFF;
        uint8_t actual = g_ctx.mapper->cpu_read(g_ctx.mapper->context, addr);
        TEST_ASSERT_EQUAL_HEX8(expected, actual);
    }

    // Testa leitura de PRG-RAM em $6000-$7FFF
    for (uint16_t addr = 0x6000; addr < 0x8000; addr += 0x400)
    {
        // Escreve um valor na PRG-RAM
        uint8_t test_val = (addr & 0xFF);
        g_ctx.prg_ram[addr - 0x6000] = test_val;

        // Lê o valor escrito
        uint8_t actual = g_ctx.mapper->cpu_read(g_ctx.mapper->context, addr);
        TEST_ASSERT_EQUAL_HEX8(test_val, actual);
    }
}

/**
 * @brief Testa a escrita da CPU para o Mapper 3
 */
void test_mapper3_cpu_write(void)
{
    // Testa seleção de banco CHR
    for (uint8_t bank = 0; bank < 4; bank++)
    {
        // Seleciona o banco escrevendo em $8000
        g_ctx.mapper->cpu_write(g_ctx.mapper->context, 0x8000, bank);

        // Verifica se o banco foi selecionado lendo de $0000 (CHR)
        uint8_t expected = (bank << 4) | 0;
        uint8_t actual = g_ctx.mapper->chr_read(g_ctx.mapper->context, 0x0000);
        TEST_ASSERT_EQUAL_HEX8(expected, actual);
    }

    // Testa escrita na PRG-RAM
    for (uint16_t addr = 0x6000; addr < 0x7000; addr += 0x400)
    {
        uint8_t test_val = (addr & 0xFF);

        // Escreve na PRG-RAM
        g_ctx.mapper->cpu_write(g_ctx.mapper->context, addr, test_val);

        // Verifica se o valor foi escrito
        TEST_ASSERT_EQUAL_HEX8(test_val, g_ctx.prg_ram[addr - 0x6000]);
    }
}

/**
 * @brief Testa a leitura da PPU para o Mapper 3
 */
void test_mapper3_ppu_read(void)
{
    // Seleciona cada banco e testa a leitura
    for (uint8_t bank = 0; bank < 4; bank++)
    {
        // Seleciona o banco
        g_ctx.mapper->cpu_write(g_ctx.mapper->context, 0x8000, bank);

        // Testa leitura em diferentes pontos do banco
        for (uint16_t addr = 0; addr < 0x2000; addr += 0x800)
        {
            uint8_t expected = (bank << 4) | (addr & 0xF);
            uint8_t actual = g_ctx.mapper->chr_read(g_ctx.mapper->context, addr);
            TEST_ASSERT_EQUAL_HEX8(expected, actual);
        }
    }
}

/**
 * @brief Testa a escrita da PPU para o Mapper 3
 */
void test_mapper3_ppu_write(void)
{
    // CNROM usa CHR-ROM que é somente leitura, então escrita não deveria ter efeito
    // Vamos testar que a escrita não causa crash e não altera o conteúdo da CHR-ROM

    // Guarda valor original
    uint8_t original = g_ctx.mapper->chr_read(g_ctx.mapper->context, 0x1000);

    // Tenta escrever
    g_ctx.mapper->chr_write(g_ctx.mapper->context, 0x1000, 0xAA);

    // Verifica que o valor não mudou
    uint8_t actual = g_ctx.mapper->chr_read(g_ctx.mapper->context, 0x1000);
    TEST_ASSERT_EQUAL_HEX8(original, actual);
}

/**
 * @brief Testa o reset do Mapper 3
 */
void test_mapper3_reset(void)
{
    // Seleciona um banco diferente de 0
    g_ctx.mapper->cpu_write(g_ctx.mapper->context, 0x8000, 2);

    // Verifica que o banco foi selecionado
    uint8_t pre_reset = g_ctx.mapper->chr_read(g_ctx.mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_HEX8(0x20, pre_reset & 0xF0);

    // Reseta o mapper
    g_ctx.mapper->reset(g_ctx.mapper->context);

    // Verifica que o banco voltou para 0
    uint8_t post_reset = g_ctx.mapper->chr_read(g_ctx.mapper->context, 0x0000);
    TEST_ASSERT_EQUAL_HEX8(0x00, post_reset & 0xF0);
}

// Função principal para executar os testes
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper3_init);
    RUN_TEST(test_mapper3_cpu_read);
    RUN_TEST(test_mapper3_cpu_write);
    RUN_TEST(test_mapper3_ppu_read);
    RUN_TEST(test_mapper3_ppu_write);
    RUN_TEST(test_mapper3_reset);

    return UNITY_END();
}
