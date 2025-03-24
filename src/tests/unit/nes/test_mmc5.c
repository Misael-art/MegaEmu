/**
 * @file test_mmc5.c
 * @brief Testes unitários para o mapper MMC5
 */

#include <unity.h>
#include "../../../platforms/nes/cartridge/nes_cartridge.h"
#include "../../../platforms/nes/nes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock das funções de log
#define EMU_LOG_ERROR(cat, fmt, ...) ((void)0)
#define EMU_LOG_WARN(cat, fmt, ...) ((void)0)
#define EMU_LOG_INFO(cat, fmt, ...) ((void)0)
#define EMU_LOG_DEBUG(cat, fmt, ...) ((void)0)
#define EMU_LOG_TRACE(cat, fmt, ...) ((void)0)

// Estrutura para o contexto dos testes
typedef struct
{
    nes_cartridge_t *cartridge;
    uint8_t *prg_rom;
    uint8_t *chr_rom;
} mmc5_test_context_t;

static mmc5_test_context_t test_ctx = {0}; // Inicializar com zero

void setUp(void)
{
    // Limpar o contexto
    memset(&test_ctx, 0, sizeof(mmc5_test_context_t));

    // Alocar memória para ROMs de teste
    test_ctx.prg_rom = (uint8_t *)malloc(32 * 1024); // 32KB PRG ROM
    test_ctx.chr_rom = (uint8_t *)malloc(8 * 1024);  // 8KB CHR ROM

    TEST_ASSERT_NOT_NULL(test_ctx.prg_rom);
    TEST_ASSERT_NOT_NULL(test_ctx.chr_rom);

    // Inicializar dados de teste
    for (int i = 0; i < 32 * 1024; i++)
    {
        test_ctx.prg_rom[i] = i & 0xFF;
    }

    for (int i = 0; i < 8 * 1024; i++)
    {
        test_ctx.chr_rom[i] = i & 0xFF;
    }

    // Criar cartridge de teste
    test_ctx.cartridge = nes_cartridge_init();
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge);

    // Configurar o mapper antes de atribuir as ROMs
    test_ctx.cartridge->mapper_number = 5; // MMC5

    // Configurar tamanhos antes de atribuir os ponteiros
    test_ctx.cartridge->prg_rom_size = 32 * 1024;
    test_ctx.cartridge->chr_rom_size = 8 * 1024;

    // Atribuir os ponteiros depois de configurar o mapper
    test_ctx.cartridge->prg_rom = test_ctx.prg_rom;
    test_ctx.cartridge->chr_rom = test_ctx.chr_rom;

    // Inicializar o mapper explicitamente
    nes_cartridge_create_mapper(test_ctx.cartridge);
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge->mapper);
}

void tearDown(void)
{
    if (test_ctx.cartridge)
    {
        // Desvincula os ponteiros do cartridge antes de fechá-lo
        test_ctx.cartridge->prg_rom = NULL;
        test_ctx.cartridge->chr_rom = NULL;

        // Liberar mapper data se existir
        if (test_ctx.cartridge->mapper_data)
        {
            free(test_ctx.cartridge->mapper_data);
            test_ctx.cartridge->mapper_data = NULL;
        }

        // Primeiro, limpa o mapper se existir
        if (test_ctx.cartridge->mapper)
        {
            free(test_ctx.cartridge->mapper);
            test_ctx.cartridge->mapper = NULL;
        }

        // Depois fecha o cartridge
        nes_cartridge_shutdown(test_ctx.cartridge);
        test_ctx.cartridge = NULL;
    }

    // Agora podemos liberar os buffers alocados por nós
    if (test_ctx.prg_rom)
    {
        free(test_ctx.prg_rom);
        test_ctx.prg_rom = NULL;
    }

    if (test_ctx.chr_rom)
    {
        free(test_ctx.chr_rom);
        test_ctx.chr_rom = NULL;
    }
}

void test_mmc5_init(void)
{
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge);
    TEST_ASSERT_EQUAL_INT(5, test_ctx.cartridge->mapper_number);
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge->mapper);
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge->mapper_data);
}

void test_mmc5_prg_read(void)
{
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge);

    // Testar leitura de PRG ROM
    uint8_t value = nes_cartridge_cpu_read(test_ctx.cartridge, 0x8000);
    TEST_ASSERT_EQUAL_HEX8(0x00, value);

    value = nes_cartridge_cpu_read(test_ctx.cartridge, 0xC000);
    TEST_ASSERT_EQUAL_HEX8(0x40, value);
}

void test_mmc5_chr_read(void)
{
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge);

    // Testar leitura de CHR ROM
    uint8_t value = nes_cartridge_chr_read(test_ctx.cartridge, 0x0000);
    TEST_ASSERT_EQUAL_HEX8(0x00, value);

    value = nes_cartridge_chr_read(test_ctx.cartridge, 0x1000);
    TEST_ASSERT_EQUAL_HEX8(0x00, value);
}

void test_mmc5_bank_switching(void)
{
    TEST_ASSERT_NOT_NULL(test_ctx.cartridge);

    // Configurar banco de PRG ROM
    nes_cartridge_cpu_write(test_ctx.cartridge, 0x5113, 0x01); // Selecionar banco 1 para PRG

    // Verificar se o banco foi trocado
    uint8_t value = nes_cartridge_cpu_read(test_ctx.cartridge, 0x8000);
    TEST_ASSERT_EQUAL_HEX8(0x40, value);

    // Configurar banco de CHR ROM
    nes_cartridge_cpu_write(test_ctx.cartridge, 0x5120, 0x01); // Selecionar banco 1 para CHR

    // Verificar se o banco foi trocado
    value = nes_cartridge_chr_read(test_ctx.cartridge, 0x0000);
    TEST_ASSERT_EQUAL_HEX8(0x00, value);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mmc5_init);
    RUN_TEST(test_mmc5_prg_read);
    RUN_TEST(test_mmc5_chr_read);
    RUN_TEST(test_mmc5_bank_switching);

    return UNITY_END();
}
