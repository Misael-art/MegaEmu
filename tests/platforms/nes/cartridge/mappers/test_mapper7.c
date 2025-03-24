/**
 * @file test_mapper7.c
 * @brief Testes para o Mapper 7 (AxROM) do Nintendo Entertainment System
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper7.h"
#include <stdlib.h>
#include <string.h>

// Definições para os testes
#define PRG_ROM_SIZE (8 * 32 * 1024) // 8 bancos de 32KB
#define CHR_RAM_SIZE (8 * 1024)      // 8KB de CHR-RAM

// Mock de funções de log para os testes
void emu_log_error(int category, const char *format, ...) {}
void emu_log_warn(int category, const char *format, ...) {}
void emu_log_info(int category, const char *format, ...) {}
void emu_log_debug(int category, const char *format, ...) {}
void emu_log_trace(int category, const char *format, ...) {}

// Estrutura para manter o contexto do teste
typedef struct
{
    nes_cartridge_t *cartridge;
    nes_mapper_t *mapper;
    uint8_t *prg_rom;
    uint8_t *chr_ram;
} test_context_t;

// Contexto global de teste
static test_context_t ctx;

// Função de configuração executada antes de cada teste
void setUp(void)
{
    // Aloca e inicializa o cartucho
    ctx.cartridge = (nes_cartridge_t *)malloc(sizeof(nes_cartridge_t));
    memset(ctx.cartridge, 0, sizeof(nes_cartridge_t));

    // Aloca e inicializa a PRG-ROM com padrões reconhecíveis
    ctx.prg_rom = (uint8_t *)malloc(PRG_ROM_SIZE);
    for (int i = 0; i < PRG_ROM_SIZE; i++)
    {
        // Cada banco de 32KB tem um valor diferente para facilitar a identificação
        ctx.prg_rom[i] = (i / 0x8000) + 0x10; // Banco 0 = 0x10, Banco 1 = 0x11, etc.
    }

    // Aloca e inicializa a CHR-RAM
    ctx.chr_ram = (uint8_t *)malloc(CHR_RAM_SIZE);
    memset(ctx.chr_ram, 0, CHR_RAM_SIZE);

    // Configura o cartucho
    ctx.cartridge->prg_rom = ctx.prg_rom;
    ctx.cartridge->prg_rom_size = PRG_ROM_SIZE;
    ctx.cartridge->chr_ram = ctx.chr_ram;
    ctx.cartridge->chr_ram_size = CHR_RAM_SIZE;
    ctx.cartridge->mapper_number = 7;
    ctx.cartridge->mirror_mode = NES_MIRROR_HORIZONTAL; // O mapper mudará isso

    // Inicializa o mapper
    ctx.mapper = nes_mapper_7_init(ctx.cartridge);
}

// Função de limpeza executada após cada teste
void tearDown(void)
{
    // Libera recursos
    if (ctx.mapper)
    {
        ctx.mapper->shutdown(ctx.mapper->context);
        free(ctx.mapper);
    }

    free(ctx.prg_rom);
    free(ctx.chr_ram);
    free(ctx.cartridge);
}

/**
 * @brief Testa a inicialização do Mapper 7
 */
void test_mapper7_init(void)
{
    // Verifica se a inicialização foi bem-sucedida
    TEST_ASSERT_NOT_NULL(ctx.mapper);
    TEST_ASSERT_EQUAL_INT(7, ctx.mapper->mapper_number);

    // Verifica se as funções do mapper foram configuradas corretamente
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->chr_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->chr_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->reset);
    TEST_ASSERT_NOT_NULL(ctx.mapper->shutdown);

    // Verifica se o espelhamento inicial é SINGLE_SCREEN_NT0
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_SINGLE_SCREEN_NT0, ctx.cartridge->mirror_mode);
}

/**
 * @brief Testa a leitura de CPU do Mapper 7
 */
void test_mapper7_cpu_read(void)
{
    // Leitura de PRG-ROM no banco 0 (inicial)
    uint8_t val1 = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    uint8_t val2 = ctx.mapper->cpu_read(ctx.mapper->context, 0xFFFF);

    // Deve retornar valores do primeiro banco (0x10)
    TEST_ASSERT_EQUAL_UINT8(0x10, val1);
    TEST_ASSERT_EQUAL_UINT8(0x10, val2);

    // Agora mudar para outro banco (2) através de escrita
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x02); // Banco 2

    // Ler novamente
    val1 = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    val2 = ctx.mapper->cpu_read(ctx.mapper->context, 0xFFFF);

    // Deve retornar valores do banco 2 (0x12)
    TEST_ASSERT_EQUAL_UINT8(0x12, val1);
    TEST_ASSERT_EQUAL_UINT8(0x12, val2);
}

/**
 * @brief Testa a escrita de CPU do Mapper 7
 */
void test_mapper7_cpu_write(void)
{
    // Testa escrita em PRG-RAM (0x6000-0x7FFF)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0xAB);
    uint8_t val = ctx.mapper->cpu_read(ctx.mapper->context, 0x6000);
    TEST_ASSERT_EQUAL_UINT8(0xAB, val);

    // Testa a mudança de banco e de espelhamento

    // Teste 1: Banco 0, Nametable 0
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x00); // Banco 0, Nametable 0
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_SINGLE_SCREEN_NT0, ctx.cartridge->mirror_mode);
    uint8_t val1 = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x10, val1); // Conteúdo do banco 0

    // Teste 2: Banco 1, Nametable 0
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x01); // Banco 1, Nametable 0
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_SINGLE_SCREEN_NT0, ctx.cartridge->mirror_mode);
    val1 = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x11, val1); // Conteúdo do banco 1

    // Teste 3: Banco 2, Nametable 1
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x12); // Banco 2, Nametable 1 (bit 4 = 1)
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_SINGLE_SCREEN_NT1, ctx.cartridge->mirror_mode);
    val1 = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x12, val1); // Conteúdo do banco 2

    // Teste 4: Banco 3, Nametable 1
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x13); // Banco 3, Nametable 1
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_SINGLE_SCREEN_NT1, ctx.cartridge->mirror_mode);
    val1 = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x13, val1); // Conteúdo do banco 3
}

/**
 * @brief Testa a leitura de CHR do Mapper 7
 */
void test_mapper7_chr_read(void)
{
    // Preencher a CHR-RAM com valores de teste
    for (int i = 0; i < CHR_RAM_SIZE; i++)
    {
        ctx.chr_ram[i] = i & 0xFF;
    }

    // Ler alguns valores da CHR-RAM
    uint8_t val1 = ctx.mapper->chr_read(ctx.mapper->context, 0x0000);
    uint8_t val2 = ctx.mapper->chr_read(ctx.mapper->context, 0x0FFF);
    uint8_t val3 = ctx.mapper->chr_read(ctx.mapper->context, 0x1000);
    uint8_t val4 = ctx.mapper->chr_read(ctx.mapper->context, 0x1FFF);

    // Verificar se os valores lidos correspondem aos esperados
    TEST_ASSERT_EQUAL_UINT8(0x00, val1);
    TEST_ASSERT_EQUAL_UINT8(0xFF, val2);
    TEST_ASSERT_EQUAL_UINT8(0x00, val3);
    TEST_ASSERT_EQUAL_UINT8(0xFF, val4);
}

/**
 * @brief Testa a escrita de CHR do Mapper 7
 */
void test_mapper7_chr_write(void)
{
    // Escrever valores na CHR-RAM
    ctx.mapper->chr_write(ctx.mapper->context, 0x0000, 0xAA);
    ctx.mapper->chr_write(ctx.mapper->context, 0x0FFF, 0xBB);
    ctx.mapper->chr_write(ctx.mapper->context, 0x1000, 0xCC);
    ctx.mapper->chr_write(ctx.mapper->context, 0x1FFF, 0xDD);

    // Ler os valores escritos
    uint8_t val1 = ctx.mapper->chr_read(ctx.mapper->context, 0x0000);
    uint8_t val2 = ctx.mapper->chr_read(ctx.mapper->context, 0x0FFF);
    uint8_t val3 = ctx.mapper->chr_read(ctx.mapper->context, 0x1000);
    uint8_t val4 = ctx.mapper->chr_read(ctx.mapper->context, 0x1FFF);

    // Verificar se os valores lidos correspondem aos esperados
    TEST_ASSERT_EQUAL_UINT8(0xAA, val1);
    TEST_ASSERT_EQUAL_UINT8(0xBB, val2);
    TEST_ASSERT_EQUAL_UINT8(0xCC, val3);
    TEST_ASSERT_EQUAL_UINT8(0xDD, val4);
}

/**
 * @brief Testa o reset do Mapper 7
 */
void test_mapper7_reset(void)
{
    // Mudar para um banco diferente e espelhamento diferente
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x13); // Banco 3, Nametable 1

    // Verificar se as mudanças foram aplicadas
    uint8_t val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x13, val);
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_SINGLE_SCREEN_NT1, ctx.cartridge->mirror_mode);

    // Resetar o mapper
    ctx.mapper->reset(ctx.mapper->context);

    // Verificar se voltou para o estado inicial
    val = ctx.mapper->cpu_read(ctx.mapper->context, 0x8000);
    TEST_ASSERT_EQUAL_UINT8(0x10, val); // Banco 0
    TEST_ASSERT_EQUAL_INT(NES_MIRROR_SINGLE_SCREEN_NT0, ctx.cartridge->mirror_mode);
}

// Função principal para executar todos os testes
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper7_init);
    RUN_TEST(test_mapper7_cpu_read);
    RUN_TEST(test_mapper7_cpu_write);
    RUN_TEST(test_mapper7_chr_read);
    RUN_TEST(test_mapper7_chr_write);
    RUN_TEST(test_mapper7_reset);

    return UNITY_END();
}
