/**
 * @file test_dmc_channel.c
 * @brief Testes unitários para o canal DMC do APU
 */

#include <unity.h>
#include "../../../../src/platforms/nes/apu/dmc_channel.h"

static nes_dmc_channel_t dmc;

void setUp(void) {
    dmc_init(&dmc);
}

void tearDown(void) {
    // Nada a fazer
}

void test_dmc_initialization(void) {
    // Verificar estado inicial após inicialização
    TEST_ASSERT_FALSE(dmc.irq_enable);
    TEST_ASSERT_FALSE(dmc.loop_flag);
    TEST_ASSERT_EQUAL_UINT8(0, dmc.rate_index);
    TEST_ASSERT_EQUAL_UINT8(0, dmc.direct_load);
    TEST_ASSERT_EQUAL_UINT16(0xC000, dmc.sample_addr);
    TEST_ASSERT_EQUAL_UINT16(0, dmc.sample_length);
    TEST_ASSERT_EQUAL_UINT8(0, dmc.output_level);
    TEST_ASSERT_FALSE(dmc.enabled);
    TEST_ASSERT_FALSE(dmc.irq_flag);
    TEST_ASSERT_TRUE(dmc.silence_flag);
}

void test_dmc_register_writes(void) {
    // Testar escrita no registrador de flags/taxa ($4010)
    dmc_write_register(&dmc, 0x00, 0xC5); // IRQ enable, loop disable, rate 5
    TEST_ASSERT_TRUE(dmc.irq_enable);
    TEST_ASSERT_FALSE(dmc.loop_flag);
    TEST_ASSERT_EQUAL_UINT8(5, dmc.rate_index);
    TEST_ASSERT_EQUAL_UINT16(254, dmc.timer_period); // Rate 5 = 254 ciclos

    // Testar escrita no registrador de load counter ($4011)
    dmc_write_register(&dmc, 0x01, 0x40);
    TEST_ASSERT_EQUAL_UINT8(0x40, dmc.output_level);

    // Testar escrita no registrador de endereço ($4012)
    dmc_write_register(&dmc, 0x02, 0x40);
    TEST_ASSERT_EQUAL_UINT16(0xC000 | (0x40 << 6), dmc.sample_addr);

    // Testar escrita no registrador de comprimento ($4013)
    dmc_write_register(&dmc, 0x03, 0x10);
    TEST_ASSERT_EQUAL_UINT16((0x10 << 4) | 0x0001, dmc.sample_length);
}

void test_dmc_sample_playback(void) {
    // Configurar canal para reprodução
    dmc_write_register(&dmc, 0x00, 0x0F); // Taxa máxima
    dmc_write_register(&dmc, 0x02, 0x40); // Endereço inicial
    dmc_write_register(&dmc, 0x03, 0x01); // 1 página de comprimento

    dmc.enabled = true;
    dmc.bytes_remaining = dmc.sample_length;
    dmc.current_addr = dmc.sample_addr;

    // Simular DMA completado com sample de teste
    dmc.sample_buffer_empty = true;
    dmc_dma_complete(&dmc, 0xAA); // 10101010 em binário

    TEST_ASSERT_FALSE(dmc.sample_buffer_empty);
    TEST_ASSERT_EQUAL_UINT8(0xAA, dmc.sample_buffer);

    // Simular 8 clocks para processar todos os bits
    for (int i = 0; i < 8; i++) {
        int16_t prev_output = dmc.output_level;
        dmc_clock(&dmc);

        // Verificar se o nível de saída foi alterado corretamente
        // 10101010 -> alternar entre incrementar e decrementar
        if (i % 2 == 0) {
            TEST_ASSERT_GREATER_THAN(prev_output, dmc.output_level);
        } else {
            TEST_ASSERT_LESS_THAN(prev_output, dmc.output_level);
        }
    }
}

void test_dmc_irq_generation(void) {
    // Configurar canal com IRQ habilitado
    dmc_write_register(&dmc, 0x00, 0x80); // IRQ enable
    dmc_write_register(&dmc, 0x02, 0x40); // Endereço inicial
    dmc_write_register(&dmc, 0x03, 0x01); // 1 página de comprimento

    dmc.enabled = true;
    dmc.bytes_remaining = 1; // Último byte

    // Simular DMA e processamento do último byte
    dmc_dma_complete(&dmc, 0x00);

    // Verificar se IRQ foi gerado
    TEST_ASSERT_TRUE(dmc.irq_flag);

    // Verificar se IRQ é limpo ao ler status
    uint8_t status = dmc_read_status(&dmc);
    TEST_ASSERT_EQUAL_UINT8(0x80, status & 0x80);

    dmc_acknowledge_irq(&dmc);
    TEST_ASSERT_FALSE(dmc.irq_flag);
}

void test_dmc_looping(void) {
    // Configurar canal com loop habilitado
    dmc_write_register(&dmc, 0x00, 0x40); // Loop enable
    dmc_write_register(&dmc, 0x02, 0x40); // Endereço inicial
    dmc_write_register(&dmc, 0x03, 0x01); // 1 página de comprimento

    dmc.enabled = true;
    dmc.bytes_remaining = 1; // Último byte
    uint16_t initial_addr = dmc.current_addr;
    uint16_t initial_length = dmc.bytes_remaining;

    // Simular DMA e processamento do último byte
    dmc_dma_complete(&dmc, 0x00);

    // Verificar se amostra foi reiniciada
    TEST_ASSERT_EQUAL_UINT16(initial_addr, dmc.current_addr);
    TEST_ASSERT_EQUAL_UINT16(initial_length, dmc.bytes_remaining);
    TEST_ASSERT_FALSE(dmc.irq_flag); // Não deve gerar IRQ em modo loop
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_dmc_initialization);
    RUN_TEST(test_dmc_register_writes);
    RUN_TEST(test_dmc_sample_playback);
    RUN_TEST(test_dmc_irq_generation);
    RUN_TEST(test_dmc_looping);

    return UNITY_END();
}
