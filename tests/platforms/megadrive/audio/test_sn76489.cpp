/**
 * @file test_sn76489.cpp
 * @brief Testes unitários para o chip de som SN76489 (PSG) do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cmath>
#include "../../../../src/platforms/megadrive/audio/sn76489.h"

// Fixture para testes do SN76489
class SN76489Test : public ::testing::Test {
protected:
    sn76489_t chip;
    
    void SetUp() override {
        // Inicializar o chip com valores padrão
        sn76489_init(&chip, SN76489_CLOCK_FREQ, SN76489_SAMPLE_RATE);
    }
    
    void TearDown() override {
        // Liberar recursos
        sn76489_shutdown(&chip);
    }
};

// Teste de inicialização
TEST_F(SN76489Test, Initialization) {
    // Verificar se a inicialização foi bem-sucedida
    EXPECT_EQ(chip.clock, SN76489_CLOCK_FREQ);
    EXPECT_EQ(chip.rate, SN76489_SAMPLE_RATE);
    EXPECT_FLOAT_EQ(chip.clock_ratio, (float)SN76489_CLOCK_FREQ / (float)SN76489_SAMPLE_RATE);
    
    // Verificar se os canais foram inicializados corretamente
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(chip.tone_channels[i].attenuation, 0x0F); // Silêncio
    }
    
    EXPECT_EQ(chip.noise_channel.attenuation, 0x0F); // Silêncio
    EXPECT_EQ(chip.noise_channel.shift_reg, 0x8000); // Valor inicial
}

// Teste de reset
TEST_F(SN76489Test, Reset) {
    // Modificar alguns valores
    chip.tone_channels[0].tone_reg = 123;
    chip.tone_channels[0].attenuation = 7;
    chip.noise_channel.attenuation = 5;
    
    // Resetar o chip
    sn76489_reset(&chip);
    
    // Verificar se os valores foram resetados
    EXPECT_EQ(chip.tone_channels[0].tone_reg, 0x400); // Valor padrão
    EXPECT_EQ(chip.tone_channels[0].attenuation, 0x0F); // Silêncio
    EXPECT_EQ(chip.noise_channel.attenuation, 0x0F); // Silêncio
}

// Teste de escrita de registradores
TEST_F(SN76489Test, RegisterWrite) {
    // Escrever valor de tom para o canal 0 (parte baixa)
    sn76489_write(&chip, 0x80 | 0x00 | 0x0A); // Latch + Canal 0 + Valor 10
    
    // Verificar se o valor foi escrito corretamente
    EXPECT_EQ(chip.tone_channels[0].tone_reg & 0x0F, 0x0A);
    
    // Escrever valor de tom para o canal 0 (parte alta)
    sn76489_write(&chip, 0x05); // Valor 5 (sem latch)
    
    // Verificar se o valor completo foi escrito corretamente
    EXPECT_EQ(chip.tone_channels[0].tone_reg, 0x05A); // 5 << 4 | 10
    
    // Escrever valor de atenuação para o canal 1
    sn76489_write(&chip, 0x90 | 0x10 | 0x07); // Latch + Canal 1 + Volume + Valor 7
    
    // Verificar se o valor foi escrito corretamente
    EXPECT_EQ(chip.tone_channels[1].attenuation, 0x07);
    
    // Escrever valor de controle de ruído
    sn76489_write(&chip, 0x80 | 0x60 | 0x05); // Latch + Canal 3 + Valor 5
    
    // Verificar se o valor foi escrito corretamente
    EXPECT_EQ(chip.noise_channel.shift_rate, 0x01); // Bits 0-1: 01
    EXPECT_EQ(chip.noise_channel.fb_type, true); // Bit 2: 1 (ruído branco)
}

// Teste de configuração estéreo
TEST_F(SN76489Test, StereoConfiguration) {
    // Configurar estéreo (canal 0 à esquerda, canal 1 à direita)
    uint8_t stereo_config = 0x01 | (0x01 << 5); // Bit 0 e bit 5
    sn76489_set_stereo(&chip, stereo_config);
    
    // Verificar se a configuração foi aplicada
    EXPECT_EQ(chip.stereo, stereo_config);
}

// Teste de geração de som (tom)
TEST_F(SN76489Test, ToneGeneration) {
    // Configurar canal 0 para gerar um tom simples
    chip.tone_channels[0].tone_reg = 100; // Frequência
    chip.tone_channels[0].attenuation = 0; // Volume máximo
    
    // Desativar outros canais
    chip.tone_channels[1].attenuation = 0x0F; // Silêncio
    chip.tone_channels[2].attenuation = 0x0F; // Silêncio
    chip.noise_channel.attenuation = 0x0F; // Silêncio
    
    // Configurar estéreo (canal 0 em ambos os lados)
    sn76489_set_stereo(&chip, 0x11); // Bits 0 e 4
    
    // Gerar algumas amostras
    const int num_samples = 1000;
    int16_t buffer_left[num_samples];
    int16_t buffer_right[num_samples];
    
    int32_t generated = sn76489_update(&chip, buffer_left, buffer_right, num_samples);
    
    // Verificar se o número correto de amostras foi gerado
    EXPECT_EQ(generated, num_samples);
    
    // Verificar se o som foi gerado (deve haver alternância de valores positivos e negativos)
    bool has_positive = false;
    bool has_negative = false;
    
    for (int i = 0; i < num_samples; i++) {
        if (buffer_left[i] > 0) has_positive = true;
        if (buffer_left[i] < 0) has_negative = true;
    }
    
    EXPECT_TRUE(has_positive);
    EXPECT_TRUE(has_negative);
    
    // Verificar se os canais esquerdo e direito têm o mesmo conteúdo (devido à configuração estéreo)
    EXPECT_EQ(memcmp(buffer_left, buffer_right, num_samples * sizeof(int16_t)), 0);
}

// Teste de geração de som (ruído)
TEST_F(SN76489Test, NoiseGeneration) {
    // Configurar canal de ruído
    chip.noise_channel.shift_rate = 0; // N/512
    chip.noise_channel.fb_type = true; // Ruído branco
    chip.noise_channel.attenuation = 0; // Volume máximo
    
    // Desativar canais de tom
    chip.tone_channels[0].attenuation = 0x0F; // Silêncio
    chip.tone_channels[1].attenuation = 0x0F; // Silêncio
    chip.tone_channels[2].attenuation = 0x0F; // Silêncio
    
    // Configurar estéreo (canal de ruído em ambos os lados)
    sn76489_set_stereo(&chip, 0x88); // Bits 3 e 7
    
    // Gerar algumas amostras
    const int num_samples = 1000;
    int16_t buffer_left[num_samples];
    int16_t buffer_right[num_samples];
    
    int32_t generated = sn76489_update(&chip, buffer_left, buffer_right, num_samples);
    
    // Verificar se o número correto de amostras foi gerado
    EXPECT_EQ(generated, num_samples);
    
    // Verificar se o som foi gerado (deve haver alternância de valores positivos e negativos)
    bool has_positive = false;
    bool has_negative = false;
    
    for (int i = 0; i < num_samples; i++) {
        if (buffer_left[i] > 0) has_positive = true;
        if (buffer_left[i] < 0) has_negative = true;
    }
    
    EXPECT_TRUE(has_positive);
    EXPECT_TRUE(has_negative);
    
    // Verificar se os canais esquerdo e direito têm o mesmo conteúdo (devido à configuração estéreo)
    EXPECT_EQ(memcmp(buffer_left, buffer_right, num_samples * sizeof(int16_t)), 0);
}

// Teste de configuração de clock e taxa de amostragem
TEST_F(SN76489Test, ClockAndSampleRateConfiguration) {
    // Definir novos valores
    uint32_t new_clock = 4000000;
    uint32_t new_rate = 48000;
    
    // Aplicar novos valores
    sn76489_set_clock(&chip, new_clock);
    sn76489_set_sample_rate(&chip, new_rate);
    
    // Verificar se os valores foram aplicados
    EXPECT_EQ(chip.clock, new_clock);
    EXPECT_EQ(chip.rate, new_rate);
    EXPECT_FLOAT_EQ(chip.clock_ratio, (float)new_clock / (float)new_rate);
}

// Teste de avanço de ciclos
TEST_F(SN76489Test, CycleAdvancement) {
    // Avançar alguns ciclos
    uint32_t cycles = 1000;
    sn76489_advance(&chip, cycles);
    
    // Verificar se os ciclos foram contabilizados
    EXPECT_EQ(chip.cycles, cycles);
    
    // Verificar se as amostras esperadas foram calculadas
    uint32_t expected_samples = (uint32_t)((float)cycles / chip.clock_ratio);
    EXPECT_EQ(chip.samples_generated, expected_samples);
}

// Teste de volume
TEST_F(SN76489Test, VolumeControl) {
    // Configurar canal 0 com diferentes níveis de volume
    chip.tone_channels[0].tone_reg = 100; // Frequência
    
    // Desativar outros canais
    chip.tone_channels[1].attenuation = 0x0F; // Silêncio
    chip.tone_channels[2].attenuation = 0x0F; // Silêncio
    chip.noise_channel.attenuation = 0x0F; // Silêncio
    
    // Configurar estéreo (canal 0 à esquerda)
    sn76489_set_stereo(&chip, 0x01);
    
    // Gerar amostras com volume máximo
    const int num_samples = 100;
    int16_t buffer_left_max[num_samples];
    int16_t buffer_right_max[num_samples];
    
    chip.tone_channels[0].attenuation = 0; // Volume máximo
    sn76489_update(&chip, buffer_left_max, buffer_right_max, num_samples);
    
    // Gerar amostras com volume médio
    int16_t buffer_left_mid[num_samples];
    int16_t buffer_right_mid[num_samples];
    
    chip.tone_channels[0].attenuation = 7; // Volume médio
    sn76489_update(&chip, buffer_left_mid, buffer_right_mid, num_samples);
    
    // Gerar amostras com volume mínimo (quase silêncio)
    int16_t buffer_left_min[num_samples];
    int16_t buffer_right_min[num_samples];
    
    chip.tone_channels[0].attenuation = 14; // Volume mínimo
    sn76489_update(&chip, buffer_left_min, buffer_right_min, num_samples);
    
    // Verificar se o volume diminui conforme a atenuação aumenta
    int max_amplitude = 0;
    int mid_amplitude = 0;
    int min_amplitude = 0;
    
    for (int i = 0; i < num_samples; i++) {
        max_amplitude = std::max(max_amplitude, std::abs(buffer_left_max[i]));
        mid_amplitude = std::max(mid_amplitude, std::abs(buffer_left_mid[i]));
        min_amplitude = std::max(min_amplitude, std::abs(buffer_left_min[i]));
    }
    
    EXPECT_GT(max_amplitude, mid_amplitude);
    EXPECT_GT(mid_amplitude, min_amplitude);
    EXPECT_GT(min_amplitude, 0); // Ainda deve haver algum som
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
