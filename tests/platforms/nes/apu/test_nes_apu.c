/**
 * @file test_nes_apu.c
 * @brief Testes unitários para o APU do NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../../../src/platforms/nes/apu/nes_apu.hpp"

// Mocks e stubs necessários
typedef struct
{
    uint8_t registers[8];
    int irq_triggered;
} MockCPU;

typedef struct
{
    uint8_t data[0x10000];
    uint8_t read(uint16_t address)
    {
        return data[address];
    }
    void write(uint16_t address, uint8_t value)
    {
        data[address] = value;
    }
} MockMemory;

// Variáveis globais para os testes
static MockCPU *cpu;
static MockMemory *memory;
static MegaEmu::Platforms::NES::NESAPU *apu;
static int16_t test_buffer[1024];

void setUp(void)
{
    // Alocar memória para os mocks
    cpu = (MockCPU *)malloc(sizeof(MockCPU));
    memory = (MockMemory *)malloc(sizeof(MockMemory));

    // Inicializar memória com valores padrão
    memset(memory->data, 0, sizeof(memory->data));

    // Inicializar CPU
    memset(cpu->registers, 0, sizeof(cpu->registers));
    cpu->irq_triggered = 0;

    // Criar instância do APU
    apu = new MegaEmu::Platforms::NES::NESAPU(
        static_cast<void *>(cpu),
        static_cast<void *>(memory),
        44100 // Taxa de amostragem padrão
    );

    // Limpar buffer de teste
    memset(test_buffer, 0, sizeof(test_buffer));
}

void tearDown(void)
{
    delete apu;
    free(memory);
    free(cpu);
}

void test_initialization(void)
{
    // Verificar se o APU foi inicializado corretamente
    TEST_ASSERT_NOT_NULL(apu);

    // Verificar estado inicial
    apu->initialize();

    // Verificar se os registradores foram inicializados corretamente
    TEST_ASSERT_EQUAL_UINT8(0, apu->readRegister(0x4000)); // Pulse 1 control
    TEST_ASSERT_EQUAL_UINT8(0, apu->readRegister(0x4004)); // Pulse 2 control
    TEST_ASSERT_EQUAL_UINT8(0, apu->readRegister(0x4008)); // Triangle control
    TEST_ASSERT_EQUAL_UINT8(0, apu->readRegister(0x400C)); // Noise control
    TEST_ASSERT_EQUAL_UINT8(0, apu->readRegister(0x4010)); // DMC control
    TEST_ASSERT_EQUAL_UINT8(0, apu->readRegister(0x4015)); // Status register
}

void test_cycle(void)
{
    // Configurar alguns registradores para teste
    apu->writeRegister(0x4000, 0x3F); // Pulse 1: volume máximo, duty 0
    apu->writeRegister(0x4002, 0x70); // Pulse 1: período baixo
    apu->writeRegister(0x4003, 0x08); // Pulse 1: período alto

    // Habilitar canal
    apu->writeRegister(0x4015, 0x01); // Habilitar Pulse 1

    // Executar alguns ciclos
    for (int i = 0; i < 100; i++)
    {
        apu->cycle();
    }

    // Verificar se o canal está ativo
    TEST_ASSERT_NOT_EQUAL(0, apu->readRegister(0x4015) & 0x01);
}

void test_generate_samples(void)
{
    // Configurar canal pulse para gerar som
    apu->writeRegister(0x4000, 0x3F); // Pulse 1: volume máximo, duty 0
    apu->writeRegister(0x4002, 0x70); // Pulse 1: período baixo
    apu->writeRegister(0x4003, 0x08); // Pulse 1: período alto
    apu->writeRegister(0x4015, 0x01); // Habilitar Pulse 1

    // Gerar algumas amostras
    int samples_generated = apu->generateSamples(test_buffer, 100);

    // Verificar se amostras foram geradas
    TEST_ASSERT_EQUAL(100, samples_generated);

    // Verificar se as amostras não são todas zero
    bool has_non_zero = false;
    for (int i = 0; i < 100; i++)
    {
        if (test_buffer[i] != 0)
        {
            has_non_zero = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_non_zero);
}

void test_register_read_write(void)
{
    // Testar escrita e leitura de registradores

    // Pulse 1
    apu->writeRegister(0x4000, 0x3F);
    TEST_ASSERT_EQUAL_UINT8(0x3F, apu->readRegister(0x4000));

    // Pulse 2
    apu->writeRegister(0x4004, 0x7F);
    TEST_ASSERT_EQUAL_UINT8(0x7F, apu->readRegister(0x4004));

    // Triangle
    apu->writeRegister(0x4008, 0x81);
    TEST_ASSERT_EQUAL_UINT8(0x81, apu->readRegister(0x4008));

    // Noise
    apu->writeRegister(0x400C, 0x30);
    TEST_ASSERT_EQUAL_UINT8(0x30, apu->readRegister(0x400C));

    // DMC
    apu->writeRegister(0x4010, 0x0F);
    TEST_ASSERT_EQUAL_UINT8(0x0F, apu->readRegister(0x4010));
}

void test_irq_handling(void)
{
    // Configurar frame counter para gerar IRQ
    apu->writeRegister(0x4017, 0x00); // Modo 4-step, IRQ habilitado

    // Executar ciclos suficientes para gerar um IRQ
    for (int i = 0; i < 14915; i++)
    { // Um frame completo
        apu->cycle();
    }

    // Verificar se o IRQ foi gerado
    TEST_ASSERT_EQUAL(1, cpu->irq_triggered);

    // Limpar IRQ
    apu->writeRegister(0x4017, 0x40);
    TEST_ASSERT_EQUAL(0, cpu->irq_triggered);
}

void test_sample_rate_configuration(void)
{
    // Testar diferentes taxas de amostragem
    const int test_rates[] = {22050, 44100, 48000, 96000};

    for (size_t i = 0; i < sizeof(test_rates) / sizeof(test_rates[0]); i++)
    {
        // Criar nova instância do APU com taxa de amostragem diferente
        delete apu;
        apu = new MegaEmu::Platforms::NES::NESAPU(
            static_cast<void *>(cpu),
            static_cast<void *>(memory),
            test_rates[i]);

        // Verificar se a taxa foi configurada corretamente
        TEST_ASSERT_EQUAL(test_rates[i], apu->getSampleRate());

        // Gerar algumas amostras para verificar se funciona
        int samples = apu->generateSamples(test_buffer, 100);
        TEST_ASSERT_EQUAL(100, samples);
    }
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_initialization);
    RUN_TEST(test_cycle);
    RUN_TEST(test_generate_samples);
    RUN_TEST(test_register_read_write);
    RUN_TEST(test_irq_handling);
    RUN_TEST(test_sample_rate_configuration);

    return UNITY_END();
}
