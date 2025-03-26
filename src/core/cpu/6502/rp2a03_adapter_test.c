#include "rp2a03_adapter.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Mock de memória para testes
static uint8_t mock_memory[0x10000];

static uint8_t mock_read(void *ctx, uint32_t addr) {
    (void)ctx;
    return mock_memory[addr & 0xFFFF];
}

static void mock_write(void *ctx, uint32_t addr, uint8_t val) {
    (void)ctx;
    mock_memory[addr & 0xFFFF] = val;
}

// Testes
static void test_interface_creation(void) {
    printf("Testando criação da interface...\n");

    cpu_interface_t *cpu = rp2a03_create_interface();
    assert(cpu != NULL);
    assert(cpu->context != NULL);

    cpu->shutdown(cpu->context);
    free(cpu);

    printf("OK!\n");
}

static void test_initialization(void) {
    printf("Testando inicialização da CPU...\n");

    cpu_interface_t *cpu = rp2a03_create_interface();
    assert(cpu != NULL);

    cpu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0
    };

    int32_t result = cpu->init(cpu->context, &config);
    assert(result == CPU_ERROR_NONE);

    cpu->shutdown(cpu->context);
    free(cpu);

    printf("OK!\n");
}

static void test_reset(void) {
    printf("Testando reset da CPU...\n");

    cpu_interface_t *cpu = rp2a03_create_interface();
    assert(cpu != NULL);

    cpu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0
    };

    cpu->init(cpu->context, &config);

    // Configura vetor de reset
    mock_memory[0xFFFC] = 0x00;
    mock_memory[0xFFFD] = 0x80;

    cpu->reset(cpu->context);

    // Verifica PC após reset
    uint32_t pc = cpu->get_register(cpu->context, "pc");
    assert(pc == 0x8000);

    cpu->shutdown(cpu->context);
    free(cpu);

    printf("OK!\n");
}

static void test_execution(void) {
    printf("Testando execução da CPU...\n");

    cpu_interface_t *cpu = rp2a03_create_interface();
    assert(cpu != NULL);

    cpu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0
    };

    cpu->init(cpu->context, &config);

    // Programa de teste: LDA #$42
    mock_memory[0x0000] = 0xA9; // LDA immediate
    mock_memory[0x0001] = 0x42; // #$42

    // Define PC inicial
    cpu->set_register(cpu->context, "pc", 0x0000);

    // Executa 2 ciclos
    int32_t cycles = cpu->execute(cpu->context, 2);
    assert(cycles == 2);

    // Verifica valor do acumulador
    uint32_t a = cpu->get_register(cpu->context, "a");
    assert(a == 0x42);

    cpu->shutdown(cpu->context);
    free(cpu);

    printf("OK!\n");
}

static void test_interrupts(void) {
    printf("Testando interrupções...\n");

    cpu_interface_t *cpu = rp2a03_create_interface();
    assert(cpu != NULL);

    cpu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0
    };

    cpu->init(cpu->context, &config);

    // Configura vetores de interrupção
    mock_memory[0xFFFA] = 0x00; // NMI vector
    mock_memory[0xFFFB] = 0x90;
    mock_memory[0xFFFE] = 0x00; // IRQ vector
    mock_memory[0xFFFF] = 0x80;

    // Testa NMI
    cpu->trigger_interrupt(cpu->context, CPU_INTERRUPT_NMI);
    cpu->execute(cpu->context, 7); // Ciclos para processar NMI
    uint32_t pc = cpu->get_register(cpu->context, "pc");
    assert(pc == 0x9000);

    // Testa IRQ
    cpu->trigger_interrupt(cpu->context, CPU_INTERRUPT_IRQ);
    cpu->execute(cpu->context, 7); // Ciclos para processar IRQ
    pc = cpu->get_register(cpu->context, "pc");
    assert(pc == 0x8000);

    cpu->shutdown(cpu->context);
    free(cpu);

    printf("OK!\n");
}

static void test_state(void) {
    printf("Testando estado da CPU...\n");

    cpu_interface_t *cpu = rp2a03_create_interface();
    assert(cpu != NULL);

    cpu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0
    };

    cpu->init(cpu->context, &config);

    // Define estado inicial
    cpu_state_t state = {
        .cycles = 1000,
        .remaining_cycles = 2,
        .stall_cycles = 1,
        .interrupt = CPU_INTERRUPT_NMI
    };

    cpu->set_state(cpu->context, &state);

    // Verifica estado
    cpu_state_t current_state;
    cpu->get_state(cpu->context, &current_state);

    assert(current_state.cycles == state.cycles);
    assert(current_state.remaining_cycles == state.remaining_cycles);
    assert(current_state.stall_cycles == state.stall_cycles);
    assert(current_state.interrupt == state.interrupt);

    cpu->shutdown(cpu->context);
    free(cpu);

    printf("OK!\n");
}

int main(void) {
    printf("Iniciando testes do adaptador RP2A03...\n\n");

    test_interface_creation();
    test_initialization();
    test_reset();
    test_execution();
    test_interrupts();
    test_state();

    printf("\nTodos os testes passaram!\n");
    return 0;
}
