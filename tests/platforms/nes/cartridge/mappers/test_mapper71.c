/**
 * @file test_mapper71.c
 * @brief Testes para o Mapper 71 (Camerica)
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper71.h"
#include <stdlib.h>
#include <string.h>

// Mock para funções de log
void EMU_LOG_ERROR(int category, const char* format, ...) {}
void EMU_LOG_WARN(int category, const char* format, ...) {}
void EMU_LOG_INFO(int category, const char* format, ...) {}
void EMU_LOG_DEBUG(int category, const char* format, ...) {}
void EMU_LOG_TRACE(int category, const char* format, ...) {}

// Estrutura para o contexto de teste
typedef struct {
    nes_cartridge_t* cart;
    nes_mapper_t* mapper;
    uint8_t prg_rom[64 * 1024];  // 64KB de PRG-ROM
    uint8_t chr_ram[8 * 1024];   // 8KB de CHR-RAM
} test_context_t;

static test_context_t ctx;

void setUp(void) {
    // Inicializa o cartucho
    ctx.cart = nes_cartridge_init();
    TEST_ASSERT_NOT_NULL(ctx.cart);

    // Configura PRG-ROM com padrão de teste
    for (int i = 0; i < sizeof(ctx.prg_rom); i++) {
        ctx.prg_rom[i] = i & 0xFF;
    }
    ctx.cart->prg_rom = ctx.prg_rom;
    ctx.cart->prg_rom_size = sizeof(ctx.prg_rom);

    // Configura CHR-RAM
    memset(ctx.chr_ram, 0, sizeof(ctx.chr_ram));
    ctx.cart->chr_ram = ctx.chr_ram;
    ctx.cart->chr_ram_size = sizeof(ctx.chr_ram);

    // Inicializa o mapper
    ctx.mapper = nes_mapper_71_init(ctx.cart);
    TEST_ASSERT_NOT_NULL(ctx.mapper);
    ctx.cart->mapper = ctx.mapper;
}

void tearDown(void) {
    if (ctx.mapper) {
        if (ctx.mapper->shutdown) {
            ctx.mapper->shutdown(ctx.mapper->context);
        }
        free(ctx.mapper);
    }
    if (ctx.cart) {
        // Limpa ponteiros antes de liberar para evitar double free
        ctx.cart->prg_rom = NULL;
        ctx.cart->chr_ram = NULL;
        nes_cartridge_shutdown(ctx.cart);
    }
}

void test_mapper71_init(void) {
    TEST_ASSERT_NOT_NULL(ctx.mapper);
    TEST_ASSERT_NOT_NULL(ctx.mapper->context);
    TEST_ASSERT_EQUAL(71, ctx.mapper->number);
    TEST_ASSERT_EQUAL_STRING("Camerica", ctx.mapper->name);
}

void test_mapper71_cpu_read_fixed_bank(void) {
    // Testa leitura do banco fixo em $C000-$FFFF (último banco)
    uint8_t expected = ctx.prg_rom[sizeof(ctx.prg_rom) - 0x4000 + 0x1234];
    uint8_t value = ctx.mapper->cpu_read(ctx.mapper->context, 0xD234);
    TEST_ASSERT_EQUAL_HEX8(expected, value);
}

void test_mapper71_cpu_read_switchable_bank(void) {
    // Testa leitura do banco comutável em $8000-$BFFF
    uint8_t expected = ctx.prg_rom[0x1234];
    uint8_t value = ctx.mapper->cpu_read(ctx.mapper->context, 0x9234);
    TEST_ASSERT_EQUAL_HEX8(expected, value);

    // Troca para o banco 1 e testa novamente
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC000, 1);
    expected = ctx.prg_rom[0x4000 + 0x1234];
    value = ctx.mapper->cpu_read(ctx.mapper->context, 0x9234);
    TEST_ASSERT_EQUAL_HEX8(expected, value);
}

void test_mapper71_cpu_write(void) {
    // Testa escrita no registrador de seleção de banco
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC000, 0x05);
    uint8_t expected = ctx.prg_rom[0x14000 + 0x1234]; // Banco 5
    uint8_t value = ctx.mapper->cpu_read(ctx.mapper->context, 0x9234);
    TEST_ASSERT_EQUAL_HEX8(expected, value);
}

void test_mapper71_ppu_read(void) {
    // Configura padrão na CHR-RAM
    for (int i = 0; i < sizeof(ctx.chr_ram); i++) {
        ctx.chr_ram[i] = i & 0xFF;
    }

    // Testa leitura da CHR-RAM
    uint8_t expected = ctx.chr_ram[0x1234];
    uint8_t value = ctx.mapper->ppu_read(ctx.mapper->context, 0x1234);
    TEST_ASSERT_EQUAL_HEX8(expected, value);
}

void test_mapper71_ppu_write(void) {
    // Testa escrita na CHR-RAM
    ctx.mapper->ppu_write(ctx.mapper->context, 0x1234, 0xAB);
    TEST_ASSERT_EQUAL_HEX8(0xAB, ctx.chr_ram[0x1234]);
}

void test_mapper71_reset(void) {
    // Configura um estado não-inicial
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC000, 0x05);

    // Reseta o mapper
    ctx.mapper->reset(ctx.mapper->context);

    // Verifica se voltou ao banco 0
    uint8_t expected = ctx.prg_rom[0x1234];
    uint8_t value = ctx.mapper->cpu_read(ctx.mapper->context, 0x9234);
    TEST_ASSERT_EQUAL_HEX8(expected, value);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mapper71_init);
    RUN_TEST(test_mapper71_cpu_read_fixed_bank);
    RUN_TEST(test_mapper71_cpu_read_switchable_bank);
    RUN_TEST(test_mapper71_cpu_write);
    RUN_TEST(test_mapper71_ppu_read);
    RUN_TEST(test_mapper71_ppu_write);
    RUN_TEST(test_mapper71_reset);

    return UNITY_END();
}
