/**
 * @file z80_test.c
 * @brief Testes unitários para a implementação do Z80
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "../../src/core/cpu/z80/z80.h"
#include "../../src/core/cpu/z80/z80_instructions.h"

// Mock de contexto de memória
typedef struct {
    uint8_t memory[0x10000];  // 64KB
    uint8_t io[0x100];        // 256 portas
} z80_test_context_t;

// Mock de callbacks para operações de memória e I/O
static uint8_t test_read_mem(void* context, uint16_t addr) {
    z80_test_context_t* ctx = (z80_test_context_t*)context;
    return ctx->memory[addr];
}

static void test_write_mem(void* context, uint16_t addr, uint8_t value) {
    z80_test_context_t* ctx = (z80_test_context_t*)context;
    ctx->memory[addr] = value;
}

static uint8_t test_read_io(void* context, uint16_t port) {
    z80_test_context_t* ctx = (z80_test_context_t*)context;
    return ctx->io[port & 0xFF];
}

static void test_write_io(void* context, uint16_t port, uint8_t value) {
    z80_test_context_t* ctx = (z80_test_context_t*)context;
    ctx->io[port & 0xFF] = value;
}

// Teste básico de criação e destruição
static void test_z80_create_destroy(void) {
    printf("Teste: Criação e destruição do Z80\n");

    z80_cpu_t* cpu = z80_create();
    assert(cpu != NULL);

    z80_destroy(cpu);
    printf("Passou\n\n");
}

// Teste de reset
static void test_z80_reset(void) {
    printf("Teste: Reset do Z80\n");

    z80_cpu_t* cpu = z80_create();
    assert(cpu != NULL);

    // Definir alguns valores arbitrários nos registradores
    z80_set_register(cpu, Z80_REG_A, 0x12);
    z80_set_register(cpu, Z80_REG_BC, 0x3456);
    z80_set_register(cpu, Z80_REG_DE, 0x7890);
    z80_set_register(cpu, Z80_REG_HL, 0xABCD);
    z80_set_register(cpu, Z80_REG_IX, 0xEF01);
    z80_set_register(cpu, Z80_REG_IY, 0x2345);

    // Resetar CPU
    z80_reset(cpu);

    // Verificar valores após reset
    assert(z80_get_register(cpu, Z80_REG_A) == 0);
    assert(z80_get_register(cpu, Z80_REG_BC) == 0);
    assert(z80_get_register(cpu, Z80_REG_DE) == 0);
    assert(z80_get_register(cpu, Z80_REG_HL) == 0);
    assert(z80_get_register(cpu, Z80_REG_IX) == 0);
    assert(z80_get_register(cpu, Z80_REG_IY) == 0);
    assert(z80_get_register(cpu, Z80_REG_SP) == 0xFFFF);
    assert(z80_get_register(cpu, Z80_REG_PC) == 0);

    z80_destroy(cpu);
    printf("Passou\n\n");
}

// Teste de configuração de callbacks
static void test_z80_callbacks(void) {
    printf("Teste: Configuração de callbacks do Z80\n");

    z80_cpu_t* cpu = z80_create();
    assert(cpu != NULL);

    z80_test_context_t context;
    memset(&context, 0, sizeof(context));

    z80_callbacks_t callbacks = {
        .read_mem = test_read_mem,
        .write_mem = test_write_mem,
        .read_io = test_read_io,
        .write_io = test_write_io,
        .context = &context
    };

    z80_set_callbacks(cpu, &callbacks);

    z80_destroy(cpu);
    printf("Passou\n\n");
}

// Teste de instruções básicas
static void test_z80_basic_instructions(void) {
    printf("Teste: Instruções básicas do Z80\n");

    z80_cpu_t* cpu = z80_create();
    assert(cpu != NULL);

    z80_test_context_t context;
    memset(&context, 0, sizeof(context));

    // Configurar programa de teste na memória
    // 0x0000: NOP
    // 0x0001: DI
    // 0x0002: EI
    // 0x0003: HALT
    context.memory[0x0000] = 0x00;  // NOP
    context.memory[0x0001] = 0xF3;  // DI
    context.memory[0x0002] = 0xFB;  // EI
    context.memory[0x0003] = 0x76;  // HALT

    z80_callbacks_t callbacks = {
        .read_mem = test_read_mem,
        .write_mem = test_write_mem,
        .read_io = test_read_io,
        .write_io = test_write_io,
        .context = &context
    };

    z80_set_callbacks(cpu, &callbacks);

    // Executar NOP
    int cycles = z80_step(cpu);
    assert(cycles == 4);
    assert(z80_get_register(cpu, Z80_REG_PC) == 1);

    // Executar DI
    cycles = z80_step(cpu);
    assert(cycles == 4);
    assert(z80_get_register(cpu, Z80_REG_PC) == 2);
    assert(z80_interrupts_enabled(cpu) == false);

    // Executar EI
    cycles = z80_step(cpu);
    assert(cycles == 4);
    assert(z80_get_register(cpu, Z80_REG_PC) == 3);
    assert(z80_interrupts_enabled(cpu) == true);

    // Executar HALT
    cycles = z80_step(cpu);
    assert(cycles == 4);
    assert(z80_get_register(cpu, Z80_REG_PC) == 4);

    // Verificar que o próximo step não avança PC (está em HALT)
    cycles = z80_step(cpu);
    assert(cycles == 4);
    assert(z80_get_register(cpu, Z80_REG_PC) == 4);

    z80_destroy(cpu);
    printf("Passou\n\n");
}

// Teste de interrupções
static void test_z80_interrupts(void) {
    printf("Teste: Interrupções do Z80\n");

    z80_cpu_t* cpu = z80_create();
    assert(cpu != NULL);

    z80_test_context_t context;
    memset(&context, 0, sizeof(context));

    // Configurar rotina de interrupção
    context.memory[0x0038] = 0x00;  // NOP em 0x0038 (RST 38h)
    context.memory[0x0039] = 0xC9;  // RET (ainda não implementado, só para representar)

    z80_callbacks_t callbacks = {
        .read_mem = test_read_mem,
        .write_mem = test_write_mem,
        .read_io = test_read_io,
        .write_io = test_write_io,
        .context = &context
    };

    z80_set_callbacks(cpu, &callbacks);

    // Habilitar interrupções
    z80_set_register(cpu, Z80_REG_PC, 0x1000);
    z80_set_register(cpu, Z80_REG_SP, 0x2000);

    // Configurar EI
    context.memory[0x1000] = 0xFB;  // EI

    // Executar EI
    int cycles = z80_step(cpu);
    assert(cycles == 4);
    assert(z80_interrupts_enabled(cpu) == true);

    // Gerar interrupção
    z80_interrupt(cpu, Z80_INT_IRQ, 0xFF);

    // Executar próximo ciclo (deve tratar a interrupção)
    cycles = z80_step(cpu);

    // Modo 1: Deve saltar para 0x0038
    assert(z80_get_register(cpu, Z80_REG_PC) == 0x0038);

    // Verificar que a interrupção foi desabilitada
    assert(z80_interrupts_enabled(cpu) == false);

    z80_destroy(cpu);
    printf("Passou\n\n");
}

// Função principal
int main(int argc, char** argv) {
    printf("Testes unitários do Z80\n");
    printf("=======================\n\n");

    // Inicializar tabelas de instruções
    if (!z80_instructions_init()) {
        printf("Falha ao inicializar tabelas de instruções\n");
        return 1;
    }

    // Executar testes
    test_z80_create_destroy();
    test_z80_reset();
    test_z80_callbacks();
    test_z80_basic_instructions();
    test_z80_interrupts();

    // Liberar recursos
    z80_instructions_shutdown();

    printf("Todos os testes passaram com sucesso!\n");
    return 0;
}
