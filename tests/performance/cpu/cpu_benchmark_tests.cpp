#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include "core/cpu/cpu.h"

class CPUBenchmarkTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        cpu = cpu_create();
    }

    void TearDown() override
    {
        cpu_destroy(cpu);
    }

    // Utilitário para medir tempo de execução
    template <typename F>
    double measure_execution_time(F func, int iterations = 1)
    {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; i++) {
            func();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count() / static_cast<double>(iterations);
    }

    CPU* cpu;
};

// Benchmark de instruções básicas
TEST_F(CPUBenchmarkTest, BasicInstructionsBenchmark)
{
    struct InstructionTest {
        const char* name;
        uint8_t opcode;
        uint8_t operand;
        int expected_cycles;
    };

    std::vector<InstructionTest> tests = {{"LDA #", 0xA9, 0x42, 2},   {"LDX #", 0xA2, 0x42, 2},
                                          {"LDY #", 0xA0, 0x42, 2},   {"STA abs", 0x8D, 0x00, 4},
                                          {"STX abs", 0x8E, 0x00, 4}, {"STY abs", 0x8C, 0x00, 4},
                                          {"TAX", 0xAA, 0x00, 2},     {"TAY", 0xA8, 0x00, 2},
                                          {"TXA", 0x8A, 0x00, 2},     {"TYA", 0x98, 0x00, 2}};

    for (const auto& test : tests) {
        // Preparar memória
        cpu_write_memory(cpu, 0x0000, test.opcode);
        cpu_write_memory(cpu, 0x0001, test.operand);

        // Medir tempo de execução
        double avg_time = measure_execution_time(
            [&]() {
                cpu_execute(cpu);
                cpu_set_pc(cpu, 0x0000);
            },
            10000);

        printf("%s: %.2f µs (%.2f MHz)\n", test.name, avg_time, 1.0 / (avg_time * 1e-6));

        // Verificar se o tempo está dentro do limite aceitável
        EXPECT_LT(avg_time, 1.0);  // Menos de 1µs por instrução
    }
}

// Benchmark de acesso à memória
TEST_F(CPUBenchmarkTest, MemoryAccessBenchmark)
{
    struct MemoryTest {
        const char* name;
        uint16_t address;
        size_t size;
    };

    std::vector<MemoryTest> tests = {{"Zero Page", 0x0000, 256},
                                     {"Stack", 0x0100, 256},
                                     {"RAM", 0x0200, 2048},
                                     {"ROM", 0x8000, 32768}};

    for (const auto& test : tests) {
        // Teste de escrita
        double write_time = measure_execution_time([&]() {
            for (size_t i = 0; i < test.size; i++) {
                cpu_write_memory(cpu, test.address + i, i & 0xFF);
            }
        });

        // Teste de leitura
        double read_time = measure_execution_time([&]() {
            for (size_t i = 0; i < test.size; i++) {
                cpu_read_memory(cpu, test.address + i);
            }
        });

        printf("%s Write: %.2f µs (%.2f MB/s)\n", test.name, write_time, (test.size / write_time));

        printf("%s Read: %.2f µs (%.2f MB/s)\n", test.name, read_time, (test.size / read_time));

        // Verificar performance mínima
        EXPECT_LT(write_time / test.size, 0.1);  // Menos de 0.1µs por byte
        EXPECT_LT(read_time / test.size, 0.1);   // Menos de 0.1µs por byte
    }
}

// Benchmark de interrupções
TEST_F(CPUBenchmarkTest, InterruptBenchmark)
{
    // Configurar vetor de interrupção
    cpu_write_memory(cpu, 0xFFFE, 0x00);
    cpu_write_memory(cpu, 0xFFFF, 0x10);

    // Medir tempo de processamento de interrupção
    double irq_time = measure_execution_time(
        [&]() {
            cpu_interrupt(cpu, INT_IRQ);
            cpu_set_pc(cpu, 0x0000);
        },
        1000);

    printf("IRQ Processing: %.2f µs\n", irq_time);

    // Verificar performance mínima
    EXPECT_LT(irq_time, 5.0);  // Menos de 5µs por interrupção
}

// Benchmark de ciclos de CPU
TEST_F(CPUBenchmarkTest, CPUCyclesBenchmark)
{
    // Programa de teste (loop simples)
    const uint8_t test_program[] = {
        0xA9, 0x00,  // LDA #$00
        0xA2, 0xFF,  // LDX #$FF
        0xE8,        // INX
        0xD0, 0xFD,  // BNE -3
        0x69, 0x01,  // ADC #$01
        0xC9, 0x10,  // CMP #$10
        0xD0, 0xF5   // BNE -11
    };

    // Carregar programa
    for (size_t i = 0; i < sizeof(test_program); i++) {
        cpu_write_memory(cpu, 0x0200 + i, test_program[i]);
    }

    cpu_set_pc(cpu, 0x0200);

    // Medir tempo de execução do programa
    double program_time = measure_execution_time([&]() {
        while (cpu_get_pc(cpu) != 0x0200) {
            cpu_execute(cpu);
        }
    });

    printf("Program Execution: %.2f µs\n", program_time);

    // Verificar performance mínima
    EXPECT_LT(program_time, 1000.0);  // Menos de 1ms para o programa completo
}

// Benchmark de operações complexas
TEST_F(CPUBenchmarkTest, ComplexOperationsBenchmark)
{
    struct ComplexTest {
        const char* name;
        std::function<void()> operation;
    };

    std::vector<ComplexTest> tests = {{"16-bit Addition",
                                       [&]() {
                                           uint16_t a = 0x1234;
                                           uint16_t b = 0x5678;
                                           cpu_write_memory(cpu, 0x00, a & 0xFF);
                                           cpu_write_memory(cpu, 0x01, a >> 8);
                                           cpu_write_memory(cpu, 0x02, b & 0xFF);
                                           cpu_write_memory(cpu, 0x03, b >> 8);
                                           // Executar adição de 16 bits
                                           cpu_execute(cpu);
                                           cpu_execute(cpu);
                                       }},
                                      {"Block Transfer",
                                       [&]() {
                                           // Transferir 256 bytes
                                           for (int i = 0; i < 256; i++) {
                                               uint8_t value = cpu_read_memory(cpu, 0x1000 + i);
                                               cpu_write_memory(cpu, 0x2000 + i, value);
                                           }
                                       }},
                                      {"Stack Operations", [&]() {
                                           // Push e pop de 16 valores
                                           for (int i = 0; i < 16; i++) {
                                               cpu_push(cpu, i);
                                           }
                                           for (int i = 0; i < 16; i++) {
                                               cpu_pop(cpu);
                                           }
                                       }}};

    for (const auto& test : tests) {
        double avg_time = measure_execution_time(test.operation, 1000);

        printf("%s: %.2f µs\n", test.name, avg_time);

        // Verificar performance mínima
        EXPECT_LT(avg_time, 10.0);  // Menos de 10µs por operação complexa
    }
}
