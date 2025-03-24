/**
 * @file test_nes_cpu.c
 * @brief Testes unitários para o CPU do NES (6502/2A03)
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../../../src/platforms/nes/cpu/nes_cpu.hpp"

// Mocks e stubs necessários
typedef struct
{
    uint8_t data[0x10000];
    uint8_t (*read_byte)(void *, uint16_t);
    void (*write_byte)(void *, uint16_t, uint8_t);

    uint8_t read(uint16_t address)
    {
        return data[address];
    }

    void write(uint16_t address, uint8_t value)
    {
        data[address] = value;
    }
} MockMemory;

// Funções de callback para leitura/escrita de memória
uint8_t memory_read_callback(void *memory, uint16_t address)
{
    return ((MockMemory *)memory)->read(address);
}

void memory_write_callback(void *memory, uint16_t address, uint8_t value)
{
    ((MockMemory *)memory)->write(address, value);
}

// Variáveis globais para os testes
static MockMemory *memory;
static MegaEmu::Platforms::NES::NESCPU *cpu;

void setUp(void)
{
    // Alocar memória para os mocks
    memory = (MockMemory *)malloc(sizeof(MockMemory));

    // Inicializar memória com valores padrão
    memset(memory->data, 0, sizeof(memory->data));

    // Configurar callbacks de memória
    memory->read_byte = memory_read_callback;
    memory->write_byte = memory_write_callback;

    // Criar instância do CPU
    cpu = new MegaEmu::Platforms::NES::NESCPU(
        static_cast<void *>(memory));
}

void tearDown(void)
{
    delete cpu;
    free(memory);
}

static void load_test_program(void)
{
    // Endereço de reset do 6502 (0xFFFC-0xFFFD)
    memory->data[0xFFFC] = 0x00;
    memory->data[0xFFFD] = 0x80;

    // Programa simples começando em 0x8000
    memory->data[0x8000] = 0xA9; // LDA #imm
    memory->data[0x8001] = 0x42; // valor 0x42
    memory->data[0x8002] = 0x8D; // STA abs
    memory->data[0x8003] = 0x00; // endereço low byte
    memory->data[0x8004] = 0x02; // endereço high byte
    memory->data[0x8005] = 0x4C; // JMP abs
    memory->data[0x8006] = 0x00; // endereço low byte
    memory->data[0x8007] = 0x80; // endereço high byte
}

void test_initialization(void)
{
    // Verificar se o CPU foi inicializado corretamente
    TEST_ASSERT_NOT_NULL(cpu);

    // Verificar estado inicial após reset
    cpu->reset();
    TEST_ASSERT_EQUAL_UINT16(0x8000, cpu->getPC());
    TEST_ASSERT_EQUAL_UINT8(0x34, cpu->getP()); // Flags iniciais
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu->getSP()); // Stack Pointer inicial
}

void test_cycle(void)
{
    load_test_program();
    cpu->reset();

    // Executar um ciclo
    int cycles = cpu->cycle();

    // Verificar se o número de ciclos está correto
    TEST_ASSERT_GREATER_THAN(0, cycles);

    // Verificar se o PC foi incrementado
    TEST_ASSERT_NOT_EQUAL(0x8000, cpu->getPC());
}

void test_step(void)
{
    load_test_program();
    cpu->reset();

    // Executar uma instrução
    cpu->step();

    // Verificar se a instrução LDA #$42 foi executada corretamente
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu->getA());
    TEST_ASSERT_EQUAL_UINT16(0x8002, cpu->getPC());
}

void test_nmi(void)
{
    // Configurar vetor NMI
    memory->data[0xFFFA] = 0x00;
    memory->data[0xFFFB] = 0x90;

    // Configurar rotina de NMI
    memory->data[0x9000] = 0xA9; // LDA #imm
    memory->data[0x9001] = 0x55; // valor 0x55
    memory->data[0x9002] = 0x40; // RTI

    // Executar NMI
    cpu->reset();
    cpu->triggerNMI();

    // Executar até RTI
    while (memory->data[cpu->getPC()] != 0x40) {
        cpu->step();
    }
    cpu->step(); // Executar RTI

    // Verificar se NMI foi processado corretamente
    TEST_ASSERT_EQUAL_UINT8(0x55, cpu->getA());
}

void test_irq(void)
{
    // Configurar vetor IRQ
    memory->data[0xFFFE] = 0x00;
    memory->data[0xFFFF] = 0xA0;

    // Configurar rotina de IRQ
    memory->data[0xA000] = 0xA9; // LDA #imm
    memory->data[0xA001] = 0x33; // valor 0x33
    memory->data[0xA002] = 0x40; // RTI

    // Executar IRQ
    cpu->reset();
    cpu->setP(cpu->getP() & ~0x04); // Limpar flag I
    cpu->triggerIRQ();

    // Executar até RTI
    while (memory->data[cpu->getPC()] != 0x40) {
        cpu->step();
    }
    cpu->step(); // Executar RTI

    // Verificar se IRQ foi processado corretamente
    TEST_ASSERT_EQUAL_UINT8(0x33, cpu->getA());
}

void test_register_access(void)
{
    // Testar acesso aos registradores
    cpu->reset();

    // Acumulador
    cpu->setA(0x42);
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu->getA());

    // Registrador X
    cpu->setX(0x55);
    TEST_ASSERT_EQUAL_UINT8(0x55, cpu->getX());

    // Registrador Y
    cpu->setY(0xAA);
    TEST_ASSERT_EQUAL_UINT8(0xAA, cpu->getY());

    // Program Counter
    cpu->setPC(0x1234);
    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu->getPC());

    // Stack Pointer
    cpu->setSP(0xFD);
    TEST_ASSERT_EQUAL_UINT8(0xFD, cpu->getSP());
}

void test_processor_flags(void)
{
    cpu->reset();

    // Testar flags individuais
    cpu->setP(0x00);

    // Carry flag
    cpu->setCarry(true);
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu->getP() & 0x01);

    // Zero flag
    cpu->setZero(true);
    TEST_ASSERT_EQUAL_UINT8(0x02, cpu->getP() & 0x02);

    // Interrupt disable
    cpu->setInterruptDisable(true);
    TEST_ASSERT_EQUAL_UINT8(0x04, cpu->getP() & 0x04);

    // Decimal mode
    cpu->setDecimal(true);
    TEST_ASSERT_EQUAL_UINT8(0x08, cpu->getP() & 0x08);

    // Break command
    cpu->setBreak(true);
    TEST_ASSERT_EQUAL_UINT8(0x10, cpu->getP() & 0x10);

    // Overflow flag
    cpu->setOverflow(true);
    TEST_ASSERT_EQUAL_UINT8(0x40, cpu->getP() & 0x40);

    // Negative flag
    cpu->setNegative(true);
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu->getP() & 0x80);
}

void test_execute_multiple_instructions(void)
{
    load_test_program();
    cpu->reset();

    // Executar programa completo
    for (int i = 0; i < 3; i++) {
        cpu->step();
    }

    // Verificar resultado final
    TEST_ASSERT_EQUAL_UINT8(0x42, memory->data[0x0200]);
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu->getA());
    TEST_ASSERT_EQUAL_UINT16(0x8000, cpu->getPC());
}

void test_illegal_opcodes(void)
{
    cpu->reset();

    // Testar alguns opcodes ilegais conhecidos
    uint8_t illegal_opcodes[] = {0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, 0x92, 0xB2, 0xD2, 0xF2};

    for (size_t i = 0; i < sizeof(illegal_opcodes); i++) {
        memory->data[0x8000] = illegal_opcodes[i];
        cpu->setPC(0x8000);

        // Verificar se o opcode ilegal é tratado sem crash
        TEST_ASSERT_TRUE(cpu->step() > 0);
    }
}

void test_page_crossing_timing(void)
{
    cpu->reset();

    // Testar timing de instruções que cruzam páginas

    // LDA abs,X com page crossing
    memory->data[0x8000] = 0xBD; // LDA abs,X
    memory->data[0x8001] = 0xFF; // Endereço low byte
    memory->data[0x8002] = 0x20; // Endereço high byte
    cpu->setPC(0x8000);
    cpu->setX(0x01); // Isso fará o endereço cruzar a página (0x20FF + 0x01 = 0x2100)

    int cycles = cpu->step();
    TEST_ASSERT_EQUAL(5, cycles); // LDA abs,X leva 4 ciclos + 1 para page crossing

    // Testar outros casos de page crossing
    // ADC abs,X
    memory->data[0x8003] = 0x7D; // ADC abs,X
    memory->data[0x8004] = 0xFF; // Endereço low byte
    memory->data[0x8005] = 0x20; // Endereço high byte
    cpu->setPC(0x8003);

    cycles = cpu->step();
    TEST_ASSERT_EQUAL(5, cycles); // ADC abs,X leva 4 ciclos + 1 para page crossing

    // LDX abs,Y
    memory->data[0x8006] = 0xBE; // LDX abs,Y
    memory->data[0x8007] = 0xFF; // Endereço low byte
    memory->data[0x8008] = 0x20; // Endereço high byte
    cpu->setPC(0x8006);
    cpu->setY(0x01);

    cycles = cpu->step();
    TEST_ASSERT_EQUAL(5, cycles); // LDX abs,Y leva 4 ciclos + 1 para page crossing
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_initialization);
    RUN_TEST(test_cycle);
    RUN_TEST(test_step);
    RUN_TEST(test_nmi);
    RUN_TEST(test_irq);
    RUN_TEST(test_register_access);
    RUN_TEST(test_processor_flags);
    RUN_TEST(test_execute_multiple_instructions);
    RUN_TEST(test_illegal_opcodes);
    RUN_TEST(test_page_crossing_timing);

    return UNITY_END();
}
