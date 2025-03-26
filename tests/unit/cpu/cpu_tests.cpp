#include <gtest/gtest.h>
#include "core/cpu/cpu.h"

class CPUTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Configuração inicial para cada teste
        cpu = cpu_create();
    }

    void TearDown() override
    {
        // Limpeza após cada teste
        cpu_destroy(cpu);
    }

    CPU* cpu;
};

// Testes básicos de inicialização
TEST_F(CPUTest, Initialization)
{
    EXPECT_NE(cpu, nullptr);
    EXPECT_EQ(cpu_get_pc(cpu), 0);
    EXPECT_EQ(cpu_get_sp(cpu), 0xFF);
}

// Testes de registradores
TEST_F(CPUTest, Registers)
{
    cpu_set_register(cpu, REG_A, 0x42);
    EXPECT_EQ(cpu_get_register(cpu, REG_A), 0x42);

    cpu_set_register(cpu, REG_X, 0x24);
    EXPECT_EQ(cpu_get_register(cpu, REG_X), 0x24);
}

// Testes de flags
TEST_F(CPUTest, Flags)
{
    cpu_set_flag(cpu, FLAG_Z, true);
    EXPECT_TRUE(cpu_get_flag(cpu, FLAG_Z));

    cpu_set_flag(cpu, FLAG_C, false);
    EXPECT_FALSE(cpu_get_flag(cpu, FLAG_C));
}

// Testes de memória
TEST_F(CPUTest, Memory)
{
    cpu_write_memory(cpu, 0x1000, 0x42);
    EXPECT_EQ(cpu_read_memory(cpu, 0x1000), 0x42);

    cpu_write_memory(cpu, 0x2000, 0x24);
    EXPECT_EQ(cpu_read_memory(cpu, 0x2000), 0x24);
}

// Testes de instruções
TEST_F(CPUTest, Instructions)
{
    // LDA #$42
    cpu_write_memory(cpu, 0x0000, 0xA9);
    cpu_write_memory(cpu, 0x0001, 0x42);

    int cycles = cpu_execute(cpu);

    EXPECT_EQ(cycles, 2);
    EXPECT_EQ(cpu_get_register(cpu, REG_A), 0x42);
    EXPECT_EQ(cpu_get_pc(cpu), 0x0002);
}

// Testes de modos de endereçamento
TEST_F(CPUTest, AddressingModes)
{
    // Zero Page
    cpu_write_memory(cpu, 0x0000, 0xA5);  // LDA $42
    cpu_write_memory(cpu, 0x0001, 0x42);
    cpu_write_memory(cpu, 0x0042, 0x24);

    int cycles = cpu_execute(cpu);

    EXPECT_EQ(cycles, 3);
    EXPECT_EQ(cpu_get_register(cpu, REG_A), 0x24);
    EXPECT_EQ(cpu_get_pc(cpu), 0x0002);
}

// Testes de interrupções
TEST_F(CPUTest, Interrupts)
{
    // Configurar vetor de interrupção
    cpu_write_memory(cpu, 0xFFFE, 0x00);
    cpu_write_memory(cpu, 0xFFFF, 0x10);

    // Gerar interrupção
    cpu_interrupt(cpu, INT_IRQ);

    EXPECT_EQ(cpu_get_pc(cpu), 0x1000);
    EXPECT_FALSE(cpu_get_flag(cpu, FLAG_I));
}

// Testes de stack
TEST_F(CPUTest, Stack)
{
    cpu_push(cpu, 0x42);
    EXPECT_EQ(cpu_pop(cpu), 0x42);

    cpu_push_word(cpu, 0x1234);
    EXPECT_EQ(cpu_pop_word(cpu), 0x1234);
}

// Testes de ciclos
TEST_F(CPUTest, Cycles)
{
    // NOP (2 ciclos)
    cpu_write_memory(cpu, 0x0000, 0xEA);
    EXPECT_EQ(cpu_execute(cpu), 2);

    // LDA #$42 (2 ciclos)
    cpu_write_memory(cpu, 0x0001, 0xA9);
    cpu_write_memory(cpu, 0x0002, 0x42);
    EXPECT_EQ(cpu_execute(cpu), 2);
}
