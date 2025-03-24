/**
 * @file test_mapper6.c
 * @brief Testes para o Mapper 6 (FFE F4xxx) do NES
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper6.h"
#include <stdlib.h>
#include <string.h>

// Definições para mocking das funções de log
#define EMU_LOG_ERROR(cat, ...) ((void)0)
#define EMU_LOG_WARN(cat, ...) ((void)0)
#define EMU_LOG_INFO(cat, ...) ((void)0)
#define EMU_LOG_DEBUG(cat, ...) ((void)0)
#define EMU_LOG_TRACE(cat, ...) ((void)0)

// Definições para o teste
#define PRG_ROM_SIZE (128 * 1024) // 128KB de PRG-ROM
#define CHR_ROM_SIZE (32 * 1024)  // 32KB de CHR-ROM
#define PRG_RAM_SIZE (8 * 1024)   // 8KB de PRG-RAM
#define CHR_RAM_SIZE (8 * 1024)   // 8KB de CHR-RAM (usado se não houver CHR-ROM)

// Estrutura para o contexto de teste
typedef struct
{
    nes_cartridge_t cartridge;
    nes_mapper_t *mapper;
    uint8_t prg_rom[PRG_ROM_SIZE];
    uint8_t chr_rom[CHR_ROM_SIZE];
    uint8_t prg_ram[PRG_RAM_SIZE];
    uint8_t chr_ram[CHR_RAM_SIZE];
} test_context_t;

test_context_t ctx;

// Configuração inicial para cada teste
void setUp(void)
{
    // Inicializa a estrutura do cartucho
    memset(&ctx.cartridge, 0, sizeof(nes_cartridge_t));

    // Configura PRG-ROM com padrão para testes
    for (int i = 0; i < PRG_ROM_SIZE; i++)
    {
        ctx.prg_rom[i] = (i & 0xFF);
    }
    ctx.cartridge.prg_rom = ctx.prg_rom;
    ctx.cartridge.prg_rom_size = PRG_ROM_SIZE;

    // Configura CHR-ROM com padrão para testes
    for (int i = 0; i < CHR_ROM_SIZE; i++)
    {
        ctx.chr_rom[i] = ((i ^ 0xAA) & 0xFF);
    }
    ctx.cartridge.chr_rom = ctx.chr_rom;
    ctx.cartridge.chr_rom_size = CHR_ROM_SIZE;

    // Configura PRG-RAM e CHR-RAM
    memset(ctx.prg_ram, 0, PRG_RAM_SIZE);
    memset(ctx.chr_ram, 0, CHR_RAM_SIZE);
    ctx.cartridge.prg_ram = ctx.prg_ram;
    ctx.cartridge.prg_ram_size = PRG_RAM_SIZE;

    // CHR-RAM não é usada se houver CHR-ROM
    ctx.cartridge.chr_ram = NULL;
    ctx.cartridge.chr_ram_size = 0;

    // Outras configurações do cartucho
    ctx.cartridge.mapper_number = 6;
    ctx.cartridge.mirror_mode = NES_MIRROR_VERTICAL;
    ctx.cartridge.has_battery = 0;

    // Inicializa o mapper
    ctx.mapper = nes_mapper_6_init(&ctx.cartridge);
    TEST_ASSERT_NOT_NULL(ctx.mapper);
}

// Limpeza após cada teste
void tearDown(void)
{
    if (ctx.mapper)
    {
        if (ctx.mapper->shutdown)
        {
            ctx.mapper->shutdown(ctx.mapper->context);
        }
        free(ctx.mapper);
        ctx.mapper = NULL;
    }
}

// Testes para o Mapper 6 (FFE F4xxx)

void test_mapper6_init(void)
{
    // Verifica inicialização básica
    TEST_ASSERT_EQUAL(6, ctx.mapper->mapper_number);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->chr_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->chr_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->reset);
    TEST_ASSERT_NOT_NULL(ctx.mapper->shutdown);
    TEST_ASSERT_NOT_NULL(ctx.mapper->context);

    // Verifica inicialização com valores inválidos
    TEST_ASSERT_NULL(nes_mapper_6_init(NULL));

    // Teste de inicialização com CHR-RAM
    nes_cartridge_t cart_with_chr_ram;
    memcpy(&cart_with_chr_ram, &ctx.cartridge, sizeof(nes_cartridge_t));
    cart_with_chr_ram.chr_rom = NULL;
    cart_with_chr_ram.chr_rom_size = 0;
    cart_with_chr_ram.chr_ram = ctx.chr_ram;
    cart_with_chr_ram.chr_ram_size = CHR_RAM_SIZE;

    nes_mapper_t *mapper_with_chr_ram = nes_mapper_6_init(&cart_with_chr_ram);
    TEST_ASSERT_NOT_NULL(mapper_with_chr_ram);

    // Limpa o mapper adicional
    if (mapper_with_chr_ram)
    {
        if (mapper_with_chr_ram->shutdown)
        {
            mapper_with_chr_ram->shutdown(mapper_with_chr_ram->context);
        }
        free(mapper_with_chr_ram);
    }
}

void test_mapper6_cpu_read(void)
{
    // Leitura fora do intervalo válido
    TEST_ASSERT_EQUAL(0, ctx.mapper->cpu_read(ctx.mapper->context, 0x5000));

    // Leitura da PRG-RAM ($6000-$7FFF)
    // O conteúdo da PRG-RAM foi inicializado com zeros
    TEST_ASSERT_EQUAL(0, ctx.mapper->cpu_read(ctx.mapper->context, 0x6000));
    TEST_ASSERT_EQUAL(0, ctx.mapper->cpu_read(ctx.mapper->context, 0x7000));
    TEST_ASSERT_EQUAL(0, ctx.mapper->cpu_read(ctx.mapper->context, 0x7FFF));

    // Escreve na PRG-RAM para teste
    ctx.cartridge.prg_ram[0] = 0x42;
    ctx.cartridge.prg_ram[0x100] = 0x69;
    TEST_ASSERT_EQUAL(0x42, ctx.mapper->cpu_read(ctx.mapper->context, 0x6000));
    TEST_ASSERT_EQUAL(0x69, ctx.mapper->cpu_read(ctx.mapper->context, 0x6100));

    // Teste de leitura dos bancos PRG-ROM
    // Por padrão, no Mapper 6, os bancos estão mapeados:
    // - $8000-$BFFF: Banco selecionável (default 0)
    // - $C000-$FFFF: Fixo no último banco

    // Testa o banco no início da ROM (banco 0, default)
    TEST_ASSERT_EQUAL(ctx.prg_rom[0], ctx.mapper->cpu_read(ctx.mapper->context, 0x8000));
    TEST_ASSERT_EQUAL(ctx.prg_rom[0x1000], ctx.mapper->cpu_read(ctx.mapper->context, 0x9000));

    // Testa o banco no final da ROM (ultimo banco)
    TEST_ASSERT_EQUAL(ctx.prg_rom[PRG_ROM_SIZE - 0x4000], ctx.mapper->cpu_read(ctx.mapper->context, 0xC000));
    TEST_ASSERT_EQUAL(ctx.prg_rom[PRG_ROM_SIZE - 1], ctx.mapper->cpu_read(ctx.mapper->context, 0xFFFF));
}

void test_mapper6_cpu_write(void)
{
    // Escrita fora do intervalo válido
    ctx.mapper->cpu_write(ctx.mapper->context, 0x5000, 0xAA);
    // Não deve causar erro

    // Escrita na PRG-RAM ($6000-$7FFF)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0x42);
    ctx.mapper->cpu_write(ctx.mapper->context, 0x7000, 0x69);
    TEST_ASSERT_EQUAL(0x42, ctx.cartridge.prg_ram[0]);
    TEST_ASSERT_EQUAL(0x69, ctx.cartridge.prg_ram[0x1000]);

    // Escrita nos registradores do mapper
    // $8000-$8FFF: Seleciona banco PRG
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x01);                                  // Seleciona banco 1 de PRG
    TEST_ASSERT_EQUAL(ctx.prg_rom[0x4000], ctx.mapper->cpu_read(ctx.mapper->context, 0x8000)); // Banco 1 = offset 0x4000

    // $9000-$9FFF: Controle de espelhamento
    ctx.mapper->cpu_write(ctx.mapper->context, 0x9000, 0x01); // Espelhamento horizontal
    TEST_ASSERT_EQUAL(NES_MIRROR_HORIZONTAL, ctx.cartridge.mirror_mode);

    // $A000-$AFFF: Proteção de PRG-RAM
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0xC0); // Habilita RAM e protege escrita

    // Testa proteção de escrita
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6200, 0x99); // Não deve escrever
    TEST_ASSERT_NOT_EQUAL(0x99, ctx.cartridge.prg_ram[0x200]);

    // $B000-$BFFF: Seleciona banco CHR
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB000, 0x02);                                  // Seleciona banco 2 de CHR
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x4000], ctx.mapper->chr_read(ctx.mapper->context, 0x0000)); // Banco 2 = offset 0x4000
}

void test_mapper6_ppu_read(void)
{
    // Verifica leitura básica de CHR-ROM
    TEST_ASSERT_EQUAL(ctx.chr_rom[0], ctx.mapper->chr_read(ctx.mapper->context, 0x0000));
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x1000], ctx.mapper->chr_read(ctx.mapper->context, 0x1000));

    // Seleciona banco de CHR
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB000, 0x01); // Seleciona banco 1

    // Verifica que o banco correto está selecionado
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x2000], ctx.mapper->chr_read(ctx.mapper->context, 0x0000)); // Banco 1 = offset 0x2000
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x2001], ctx.mapper->chr_read(ctx.mapper->context, 0x0001));

    // Seleciona outro banco
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB000, 0x03); // Seleciona banco 3

    // Verifica que o banco correto está selecionado
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x6000], ctx.mapper->chr_read(ctx.mapper->context, 0x0000)); // Banco 3 = offset 0x6000
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x6001], ctx.mapper->chr_read(ctx.mapper->context, 0x0001));
}

void test_mapper6_ppu_write(void)
{
    // Escrita direta na CHR-ROM não deve ter efeito
    ctx.mapper->chr_write(ctx.mapper->context, 0x0000, 0x42);
    // Não deve alterar CHR-ROM
    TEST_ASSERT_NOT_EQUAL(0x42, ctx.chr_rom[0]);

    // Teste com cartucho usando CHR-RAM
    nes_cartridge_t cart_with_chr_ram;
    memcpy(&cart_with_chr_ram, &ctx.cartridge, sizeof(nes_cartridge_t));
    cart_with_chr_ram.chr_rom = NULL;
    cart_with_chr_ram.chr_rom_size = 0;
    cart_with_chr_ram.chr_ram = ctx.chr_ram;
    cart_with_chr_ram.chr_ram_size = CHR_RAM_SIZE;

    nes_mapper_t *mapper_with_chr_ram = nes_mapper_6_init(&cart_with_chr_ram);
    TEST_ASSERT_NOT_NULL(mapper_with_chr_ram);

    // Escreve na CHR-RAM
    mapper_with_chr_ram->chr_write(mapper_with_chr_ram->context, 0x0000, 0x42);
    mapper_with_chr_ram->chr_write(mapper_with_chr_ram->context, 0x0001, 0x69);

    // Lê de volta para garantir que a escrita funcionou
    TEST_ASSERT_EQUAL(0x42, mapper_with_chr_ram->chr_read(mapper_with_chr_ram->context, 0x0000));
    TEST_ASSERT_EQUAL(0x69, mapper_with_chr_ram->chr_read(mapper_with_chr_ram->context, 0x0001));

    // Limpa o mapper adicional
    if (mapper_with_chr_ram)
    {
        if (mapper_with_chr_ram->shutdown)
        {
            mapper_with_chr_ram->shutdown(mapper_with_chr_ram->context);
        }
        free(mapper_with_chr_ram);
    }
}

void test_mapper6_reset(void)
{
    // Configura registradores para valores não-padrão
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x03); // PRG banco 3
    ctx.mapper->cpu_write(ctx.mapper->context, 0x9000, 0x01); // Espelhamento horizontal
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0xC0); // Proteção de PRG-RAM
    ctx.mapper->cpu_write(ctx.mapper->context, 0xB000, 0x02); // CHR banco 2

    // Verifica que os bancos estão mapeados corretamente
    TEST_ASSERT_EQUAL(ctx.prg_rom[0xC000], ctx.mapper->cpu_read(ctx.mapper->context, 0x8000)); // Banco 3 = offset 0xC000
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x4000], ctx.mapper->chr_read(ctx.mapper->context, 0x0000)); // Banco 2 = offset 0x4000
    TEST_ASSERT_EQUAL(NES_MIRROR_HORIZONTAL, ctx.cartridge.mirror_mode);

    // Executa reset
    ctx.mapper->reset(ctx.mapper->context);

    // Após reset, leituras devem refletir os valores padrão
    // Verifica se o espelhamento voltou ao modo vertical (padrão para este teste)
    TEST_ASSERT_EQUAL(NES_MIRROR_VERTICAL, ctx.cartridge.mirror_mode);

    // Verifica se os bancos estão nos valores padrão
    TEST_ASSERT_EQUAL(ctx.prg_rom[0], ctx.mapper->cpu_read(ctx.mapper->context, 0x8000)); // Banco 0
    TEST_ASSERT_EQUAL(ctx.chr_rom[0], ctx.mapper->chr_read(ctx.mapper->context, 0x0000)); // Banco 0
}

void test_mapper6_prg_ram_protect(void)
{
    // Inicialmente, a PRG-RAM está habilitada sem proteção
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0x42);
    TEST_ASSERT_EQUAL(0x42, ctx.cartridge.prg_ram[0]);

    // Desativa a PRG-RAM
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0x00);

    // Escrita na PRG-RAM não deve ter efeito quando desativada
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0x69);
    TEST_ASSERT_EQUAL(0x42, ctx.cartridge.prg_ram[0]); // Mantém o valor anterior

    // Leitura da PRG-RAM desativada deve retornar 0xFF (pull-up)
    TEST_ASSERT_EQUAL(0xFF, ctx.mapper->cpu_read(ctx.mapper->context, 0x6000));

    // Ativa PRG-RAM com proteção de escrita
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0xC0);

    // Escrita na PRG-RAM não deve ter efeito quando protegida
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0x99);
    TEST_ASSERT_EQUAL(0x42, ctx.cartridge.prg_ram[0]); // Mantém o valor anterior

    // Leitura deve funcionar normalmente
    TEST_ASSERT_EQUAL(0x42, ctx.mapper->cpu_read(ctx.mapper->context, 0x6000));

    // Ativa PRG-RAM com escrita permitida
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0x80);

    // Agora escrita deve funcionar
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0x55);
    TEST_ASSERT_EQUAL(0x55, ctx.cartridge.prg_ram[0]);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper6_init);
    RUN_TEST(test_mapper6_cpu_read);
    RUN_TEST(test_mapper6_cpu_write);
    RUN_TEST(test_mapper6_ppu_read);
    RUN_TEST(test_mapper6_ppu_write);
    RUN_TEST(test_mapper6_reset);
    RUN_TEST(test_mapper6_prg_ram_protect);

    return UNITY_END();
}
