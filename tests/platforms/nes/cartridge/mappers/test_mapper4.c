/**
 * @file test_mapper4.c
 * @brief Testes para o Mapper 4 (MMC3) do NES
 */

#include "unity.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include "platforms/nes/cartridge/mappers/mapper4.h"
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
#define CHR_ROM_SIZE (128 * 1024) // 128KB de CHR-ROM
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
    ctx.cartridge.mapper_number = 4;
    ctx.cartridge.mirror_mode = NES_MIRROR_VERTICAL;
    ctx.cartridge.has_battery = 0;

    // Inicializa o mapper
    ctx.mapper = nes_mapper_4_init(&ctx.cartridge);
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

// Testes para o Mapper 4 (MMC3)

void test_mapper4_init(void)
{
    // Verifica inicialização básica
    TEST_ASSERT_EQUAL(4, ctx.mapper->mapper_number);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->cpu_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->chr_read);
    TEST_ASSERT_NOT_NULL(ctx.mapper->chr_write);
    TEST_ASSERT_NOT_NULL(ctx.mapper->reset);
    TEST_ASSERT_NOT_NULL(ctx.mapper->shutdown);
    TEST_ASSERT_NOT_NULL(ctx.mapper->context);

    // Verifica inicialização com valores inválidos
    TEST_ASSERT_NULL(nes_mapper_4_init(NULL));

    // Teste de inicialização com CHR-RAM
    nes_cartridge_t cart_with_chr_ram;
    memcpy(&cart_with_chr_ram, &ctx.cartridge, sizeof(nes_cartridge_t));
    cart_with_chr_ram.chr_rom = NULL;
    cart_with_chr_ram.chr_rom_size = 0;
    cart_with_chr_ram.chr_ram = ctx.chr_ram;
    cart_with_chr_ram.chr_ram_size = CHR_RAM_SIZE;

    nes_mapper_t *mapper_with_chr_ram = nes_mapper_4_init(&cart_with_chr_ram);
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

void test_mapper4_cpu_read(void)
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
    // Por padrão, no MMC3, os bancos estão mapeados:
    // - $8000-$9FFF: Variável (depende do modo e registrador 6)
    // - $A000-$BFFF: Variável (depende do modo e registrador 7)
    // - $C000-$DFFF: Fixo no penúltimo banco (ou variável, dependendo do modo)
    // - $E000-$FFFF: Fixo no último banco

    // Testa o banco no final da ROM (ultimo banco)
    TEST_ASSERT_EQUAL(ctx.prg_rom[PRG_ROM_SIZE - 0x2000], ctx.mapper->cpu_read(ctx.mapper->context, 0xE000));
    TEST_ASSERT_EQUAL(ctx.prg_rom[PRG_ROM_SIZE - 1], ctx.mapper->cpu_read(ctx.mapper->context, 0xFFFF));
}

void test_mapper4_cpu_write(void)
{
    // Escrita fora do intervalo válido
    ctx.mapper->cpu_write(ctx.mapper->context, 0x5000, 0xAA);
    // Não deve causar erro

    // Escrita na PRG-RAM ($6000-$7FFF)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x6000, 0x42);
    ctx.mapper->cpu_write(ctx.mapper->context, 0x7000, 0x69);
    TEST_ASSERT_EQUAL(0x42, ctx.cartridge.prg_ram[0]);
    TEST_ASSERT_EQUAL(0x69, ctx.cartridge.prg_ram[0x1000]);

    // Escrita no registrador de seleção de banco ($8000-$9FFE, pares)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x00); // Modo 0, seleciona R0 (banco CHR 0)

    // Escrita no registrador de dados de banco ($8001-$9FFF, ímpares)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8001, 0x01); // Define o banco CHR 0 como 1

    // Escreve no registrador de controle de espelhamento ($A000-$BFFE, pares)
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0x01); // Espelhamento horizontal
    TEST_ASSERT_EQUAL(NES_MIRROR_HORIZONTAL, ctx.cartridge.mirror_mode);

    // Escreve no registrador de proteção da PRG-RAM ($A001-$BFFF, ímpares)
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA001, 0xC0); // Habilita PRG-RAM, protege escrita
}

void test_mapper4_ppu_read(void)
{
    // Testes para diferentes modos de mapeamento CHR

    // Por padrão, no modo 0, a organização de bancos é:
    // - $0000-$07FF: Banco definido pelo registrador 0 (2KB)
    // - $0800-$0FFF: Banco definido pelo registrador 1 (2KB)
    // - $1000-$13FF: Banco definido pelo registrador 2 (1KB)
    // - $1400-$17FF: Banco definido pelo registrador 3 (1KB)
    // - $1800-$1BFF: Banco definido pelo registrador 4 (1KB)
    // - $1C00-$1FFF: Banco definido pelo registrador 5 (1KB)

    // Selecionar modo 0, R0 (controla banco CHR $0000-$07FF)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x00);
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8001, 0x04); // Selecion banco CHR 2 (0 e 1)

    // Verifica o conteúdo do primeiro banco
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x4000], ctx.mapper->chr_read(ctx.mapper->context, 0x0000));
    TEST_ASSERT_EQUAL(ctx.chr_rom[0x4001], ctx.mapper->chr_read(ctx.mapper->context, 0x0001));

    // Teste com modo 1 (trocar organização dos bancos)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x80); // Bit 7 = 1 -> Modo 1

    // No modo 1, a organização é:
    // - $0000-$03FF: Banco definido pelo registrador 2 (1KB)
    // - $0400-$07FF: Banco definido pelo registrador 3 (1KB)
    // - $0800-$0BFF: Banco definido pelo registrador 4 (1KB)
    // - $0C00-$0FFF: Banco definido pelo registrador 5 (1KB)
    // - $1000-$17FF: Banco definido pelo registrador 0 (2KB)
    // - $1800-$1FFF: Banco definido pelo registrador 1 (2KB)

    // Seleciona R2 (controla banco CHR $0000-$03FF em modo 1)
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x82); // Modo 1, seleciona R2
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8001, 0x0A); // Seleciona o banco 10

    // Verifica o conteúdo do banco selecionado
    TEST_ASSERT_EQUAL(ctx.chr_rom[0xA * 0x400], ctx.mapper->chr_read(ctx.mapper->context, 0x0000));
    TEST_ASSERT_EQUAL(ctx.chr_rom[0xA * 0x400 + 1], ctx.mapper->chr_read(ctx.mapper->context, 0x0001));
}

void test_mapper4_ppu_write(void)
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

    nes_mapper_t *mapper_with_chr_ram = nes_mapper_4_init(&cart_with_chr_ram);
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

void test_mapper4_reset(void)
{
    // Configura registradores para valores não-padrão
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8000, 0x42); // Bank select
    ctx.mapper->cpu_write(ctx.mapper->context, 0x8001, 0x69); // Bank data
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA000, 0x01); // Mirroring
    ctx.mapper->cpu_write(ctx.mapper->context, 0xA001, 0xC0); // PRG RAM protect
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC000, 0x10); // IRQ latch
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC001, 0x00); // IRQ reload
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE001, 0x01); // IRQ enable

    // Executa reset
    ctx.mapper->reset(ctx.mapper->context);

    // Após reset, leituras devem refletir os valores padrão
    // Não podemos acessar registradores diretamente, mas podemos testar efeitos secundários

    // Verifica se o espelhamento voltou ao modo vertical (padrão)
    TEST_ASSERT_EQUAL(NES_MIRROR_VERTICAL, ctx.cartridge.mirror_mode);

    // Verifica se os bancos estão nos valores padrão
    // Podemos testar lendo em endereços específicos e verificando

    // Último banco deve estar mapeado em $E000-$FFFF (fixo)
    TEST_ASSERT_EQUAL(ctx.prg_rom[PRG_ROM_SIZE - 0x2000], ctx.mapper->cpu_read(ctx.mapper->context, 0xE000));
    TEST_ASSERT_EQUAL(ctx.prg_rom[PRG_ROM_SIZE - 1], ctx.mapper->cpu_read(ctx.mapper->context, 0xFFFF));
}

void test_mapper4_irq(void)
{
    // Inicia com IRQ desabilitado
    TEST_ASSERT_EQUAL(0, ctx.mapper->irq_state(ctx.mapper->context));

    // Configura IRQ
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC000, 0x01); // IRQ latch = 1
    ctx.mapper->cpu_write(ctx.mapper->context, 0xC001, 0x00); // Reload counter
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE001, 0x01); // Enable IRQ

    // Simula transições A12 para acionar IRQ
    // No MMC3, IRQ é decrementado nas bordas de subida de A12
    // False -> True -> False (A12 = bit 12 do endereço)
    ctx.mapper->chr_read(ctx.mapper->context, 0x0FFF); // A12 = 0
    ctx.mapper->chr_read(ctx.mapper->context, 0x1000); // A12 = 1 (borda de subida)

    // IRQ não deve disparar ainda (contador = 1)
    TEST_ASSERT_EQUAL(0, ctx.mapper->irq_state(ctx.mapper->context));

    // Mais uma transição
    ctx.mapper->chr_read(ctx.mapper->context, 0x0FFF); // A12 = 0
    ctx.mapper->chr_read(ctx.mapper->context, 0x1000); // A12 = 1 (borda de subida)

    // IRQ deve acionar agora (contador atingiu 0)
    TEST_ASSERT_EQUAL(1, ctx.mapper->irq_state(ctx.mapper->context));

    // Limpa IRQ
    ctx.mapper->irq_clear(ctx.mapper->context);
    TEST_ASSERT_EQUAL(0, ctx.mapper->irq_state(ctx.mapper->context));

    // Desabilita IRQ
    ctx.mapper->cpu_write(ctx.mapper->context, 0xE000, 0x00); // Disable IRQ

    // Mesmo com mais transições, IRQ não deve acionar
    ctx.mapper->chr_read(ctx.mapper->context, 0x0FFF); // A12 = 0
    ctx.mapper->chr_read(ctx.mapper->context, 0x1000); // A12 = 1
    ctx.mapper->chr_read(ctx.mapper->context, 0x0FFF); // A12 = 0
    ctx.mapper->chr_read(ctx.mapper->context, 0x1000); // A12 = 1

    TEST_ASSERT_EQUAL(0, ctx.mapper->irq_state(ctx.mapper->context));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mapper4_init);
    RUN_TEST(test_mapper4_cpu_read);
    RUN_TEST(test_mapper4_cpu_write);
    RUN_TEST(test_mapper4_ppu_read);
    RUN_TEST(test_mapper4_ppu_write);
    RUN_TEST(test_mapper4_reset);
    RUN_TEST(test_mapper4_irq);

    return UNITY_END();
}
