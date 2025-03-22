/**
 * @file z80_test_conformance.c
 * @brief Testes de conformidade para a implementação do Z80
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "../../src/core/cpu/z80/z80.h"
#include "../../src/core/cpu/z80/z80_instructions.h"

// Contexto simulado para testes
typedef struct
{
    uint8_t memory[0x10000]; // 64KB
    uint8_t io[0x100];       // 256 portas
    z80_t *cpu;              // Instância do CPU
} test_context_t;

// Instância global para testes
static test_context_t ctx;

// Callbacks de memória para testes
static uint8_t test_read_byte(void *context, uint16_t address)
{
    test_context_t *ctx = (test_context_t *)context;
    return ctx->memory[address];
}

static void test_write_byte(void *context, uint16_t address, uint8_t value)
{
    test_context_t *ctx = (test_context_t *)context;
    ctx->memory[address] = value;
}

static uint8_t test_read_io(void *context, uint16_t port)
{
    test_context_t *ctx = (test_context_t *)context;
    return ctx->io[port & 0xFF];
}

static void test_write_io(void *context, uint16_t port, uint8_t value)
{
    test_context_t *ctx = (test_context_t *)context;
    ctx->io[port & 0xFF] = value;
}

// Inicialização e finalização de cada teste
static void setup(void)
{
    // Inicializar contexto de teste
    memset(ctx.memory, 0, sizeof(ctx.memory));
    memset(ctx.io, 0, sizeof(ctx.io));

    // Criar e configurar o CPU
    ctx.cpu = z80_create();
    assert(ctx.cpu != NULL);

    // Configurar callbacks
    ctx.cpu->read_byte = test_read_byte;
    ctx.cpu->write_byte = test_write_byte;
    ctx.cpu->read_io = test_read_io;
    ctx.cpu->write_io = test_write_io;
    ctx.cpu->context = &ctx;

    // Resetar CPU
    z80_reset(ctx.cpu);
}

static void teardown(void)
{
    if (ctx.cpu)
    {
        z80_destroy(ctx.cpu);
        ctx.cpu = NULL;
    }
}

// Testes específicos de timing
static void test_timing_ex_de_hl(void)
{
    printf("Teste: Timing da instrução EX DE, HL\n");

    // Configurar instrução
    ctx.memory[0] = 0xEB; // EX DE, HL

    // Configurar registradores iniciais
    ctx.cpu->regs.hl = 0x1234;
    ctx.cpu->regs.de = 0x5678;

    // Executar instrução
    int cycles = z80_execute(ctx.cpu, 5); // Pedir mais ciclos do que necessário

    // Verificar resultados
    assert(ctx.cpu->regs.hl == 0x5678);
    assert(ctx.cpu->regs.de == 0x1234);
    assert(cycles == 4); // Deve consumir exatamente 4 ciclos

    printf("  Resultado: OK - Instrução consumiu %d ciclos\n", cycles);
}

// Teste de interrupções
static void test_interrupts_im1(void)
{
    printf("Teste: Interrupções no modo IM 1\n");

    // Configurar modo de interrupção 1
    ctx.memory[0] = 0xED; // Prefixo ED
    ctx.memory[1] = 0x56; // IM 1

    // Executar instrução para configurar IM 1
    z80_execute(ctx.cpu, 8);

    // Verificar se IM 1 está configurado
    assert(ctx.cpu->im == 1);

    // Testar interrupção
    ctx.cpu->pc = 0x100;     // Algum ponto no código
    ctx.memory[0x38] = 0xC9; // Instrução RET no handler de interrupção

    // Habilitar interrupções
    ctx.cpu->iff1 = 1;
    ctx.cpu->iff2 = 1;

    // Gerar interrupção
    z80_trigger_int(ctx.cpu, 0xFF);

    // Executar ciclos suficientes para processar a interrupção
    z80_execute(ctx.cpu, 20);

    // PC deve retornar após a instrução RET do handler
    assert(ctx.cpu->pc == 0x101);

    printf("  Resultado: OK - Interrupção processada corretamente\n");
}

// Teste específico para detecção de flag overflow em aritmética
static void test_flag_overflow_detect(void)
{
    printf("Teste: Detecção de overflow em operações aritméticas\n");

    // Testar overflow em operação aritmética
    ctx.memory[0] = 0x3E; // LD A, nn
    ctx.memory[1] = 0x7F; // A = 127 (maior número positivo em 8 bits com sinal)
    ctx.memory[2] = 0xC6; // ADD A, nn
    ctx.memory[3] = 0x01; // Adicionar 1 deve causar overflow

    // Resetar PC
    ctx.cpu->pc = 0;

    // Executar instruções
    z80_execute(ctx.cpu, 18);

    // Verificar overflow flag (P/V)
    assert(ctx.cpu->regs.f & Z80_FLAG_PV);
    // O resultado deve ser 128 (0x80)
    assert((ctx.cpu->regs.a & 0xFF) == 0x80);

    printf("  Resultado: OK - Flag de overflow detectada corretamente\n");
}

// Teste para instruções não documentadas
static void test_undocumented_ix_registers(void)
{
    printf("Teste: Acesso a registradores IX não documentados\n");

    // Testar acesso a IXH (não documentado)
    ctx.memory[0] = 0xDD; // Prefixo IX
    ctx.memory[1] = 0x26; // LD IXH, nn
    ctx.memory[2] = 0x42; // Valor 0x42

    // Resetar PC
    ctx.cpu->pc = 0;

    // Executar
    z80_execute(ctx.cpu, 11);

    // Verificar se o valor foi carregado corretamente
    // IXH é a parte alta do registrador IX
    assert((ctx.cpu->regs.ix >> 8) == 0x42);

    // Agora testar IXL
    ctx.memory[3] = 0xDD; // Prefixo IX
    ctx.memory[4] = 0x2E; // LD IXL, nn
    ctx.memory[5] = 0x24; // Valor 0x24

    // Executar
    z80_execute(ctx.cpu, 11);

    // Verificar se o valor foi carregado corretamente
    // IXL é a parte baixa do registrador IX
    assert((ctx.cpu->regs.ix & 0xFF) == 0x24);

    // O IX completo deve ser 0x4224
    assert(ctx.cpu->regs.ix == 0x4224);

    printf("  Resultado: OK - Registradores IX não documentados funcionam corretamente\n");
}

// Teste para flags em operações lógicas
static void test_logical_operation_flags(void)
{
    printf("Teste: Flags em operações lógicas\n");

    // Testar flags após operação AND
    ctx.memory[0] = 0x3E; // LD A, nn
    ctx.memory[1] = 0xAA; // A = 10101010b
    ctx.memory[2] = 0xE6; // AND nn
    ctx.memory[3] = 0x55; // 01010101b

    // Resetar PC
    ctx.cpu->pc = 0;

    // Executar
    z80_execute(ctx.cpu, 14);

    // Resultado deve ser 0
    assert(ctx.cpu->regs.a == 0x00);

    // Verificar flags
    assert(ctx.cpu->regs.f & Z80_FLAG_Z);        // Zero flag deve estar ativo
    assert(ctx.cpu->regs.f & Z80_FLAG_H);        // Half-carry deve estar ativo
    assert((ctx.cpu->regs.f & Z80_FLAG_N) == 0); // Subtract flag deve estar inativo

    printf("  Resultado: OK - Flags após operação AND estão corretas\n");
}

// Teste para flags após operações de rotação
static void test_rotation_operation_flags(void)
{
    printf("Teste: Flags após operações de rotação\n");

    // Testar flags após operação RLC A
    ctx.memory[0] = 0x3E; // LD A, nn
    ctx.memory[1] = 0x81; // A = 10000001b
    ctx.memory[2] = 0xCB; // Prefixo CB
    ctx.memory[3] = 0x07; // RLC A

    // Resetar PC
    ctx.cpu->pc = 0;

    // Executar
    z80_execute(ctx.cpu, 15);

    // Resultado deve ser 0x03 (00000011b) - bit 7 vai para bit 0 e carry
    assert(ctx.cpu->regs.a == 0x03);

    // Verificar flags
    assert(ctx.cpu->regs.f & Z80_FLAG_C);        // Carry flag deve estar ativo pelo bit 7
    assert((ctx.cpu->regs.f & Z80_FLAG_Z) == 0); // Zero flag deve estar inativo
    assert((ctx.cpu->regs.f & Z80_FLAG_H) == 0); // Half-carry deve estar inativo
    assert((ctx.cpu->regs.f & Z80_FLAG_N) == 0); // Subtract flag deve estar inativo

    printf("  Resultado: OK - Flags após operação de rotação estão corretas\n");
}

// Teste para o registrador de refresh R
static void test_refresh_register(void)
{
    printf("Teste: Incremento do registrador de refresh R\n");

    // Salvar valor inicial de R
    uint8_t initial_r = ctx.cpu->r;

    // Executar algumas instruções e verificar se R incrementa
    ctx.memory[0] = 0x00; // NOP
    ctx.memory[1] = 0x00; // NOP
    ctx.memory[2] = 0x00; // NOP

    // Resetar PC
    ctx.cpu->pc = 0;

    // Executar
    z80_execute(ctx.cpu, 12); // 3 NOPs, 4 ciclos cada

    // R deve ter incrementado 3 vezes (7 bits inferiores)
    assert(((ctx.cpu->r & 0x7F) - (initial_r & 0x7F)) % 0x80 == 3);

    printf("  Resultado: OK - Registrador R incrementou corretamente\n");
}

// Função principal
int main(int argc, char **argv)
{
    printf("Testes de conformidade para Z80 iniciados\n");

    // Executar todos os testes
    setup();
    test_timing_ex_de_hl();
    teardown();

    setup();
    test_interrupts_im1();
    teardown();

    setup();
    test_flag_overflow_detect();
    teardown();

    setup();
    test_undocumented_ix_registers();
    teardown();

    setup();
    test_logical_operation_flags();
    teardown();

    setup();
    test_rotation_operation_flags();
    teardown();

    setup();
    test_refresh_register();
    teardown();

    printf("Todos os testes concluídos com sucesso!\n");
    return 0;
}
