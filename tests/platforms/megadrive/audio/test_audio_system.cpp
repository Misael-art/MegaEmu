/**
 * @file test_audio_system.cpp
 * @brief Testes unitários para o sistema de áudio do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cmath>
#include "../../../../src/platforms/megadrive/audio/audio_system.h"

// Fixture para testes do sistema de áudio do Mega Drive
class MegaDriveAudioTest : public ::testing::Test {
protected:
    md_audio_system_t audio;
    
    void SetUp() override {
        // Inicializar o sistema de áudio com valores padrão
        md_audio_init(&audio, 7670454, MD_AUDIO_SAMPLE_RATE); // Clock do Mega Drive NTSC
    }
    
    void TearDown() override {
        // Liberar recursos
        md_audio_shutdown(&audio);
    }
};

// Teste de inicialização
TEST_F(MegaDriveAudioTest, Initialization) {
    // Verificar se a inicialização foi bem-sucedida
    EXPECT_EQ(audio.system_clock, 7670454);
    EXPECT_EQ(audio.sample_rate, MD_AUDIO_SAMPLE_RATE);
    EXPECT_FLOAT_EQ(audio.cycles_per_sample, (float)7670454 / (float)MD_AUDIO_SAMPLE_RATE);
    
    // Verificar se os volumes foram inicializados corretamente
    EXPECT_FLOAT_EQ(audio.ym2612_volume, 0.8f);
    EXPECT_FLOAT_EQ(audio.sn76489_volume, 0.6f);
    EXPECT_FLOAT_EQ(audio.master_volume, 1.0f);
    
    // Verificar se os buffers foram alocados
    EXPECT_NE(audio.buffer_left, nullptr);
    EXPECT_NE(audio.buffer_right, nullptr);
    EXPECT_NE(audio.ym2612_buffer_left, nullptr);
    EXPECT_NE(audio.ym2612_buffer_right, nullptr);
    EXPECT_NE(audio.sn76489_buffer_left, nullptr);
    EXPECT_NE(audio.sn76489_buffer_right, nullptr);
    
    // Verificar se o sistema está habilitado
    EXPECT_TRUE(audio.enabled);
}

// Teste de reset
TEST_F(MegaDriveAudioTest, Reset) {
    // Modificar alguns valores
    audio.samples_generated = 1000;
    audio.cycles = 5000;
    
    // Resetar o sistema
    md_audio_reset(&audio);
    
    // Verificar se os valores foram resetados
    EXPECT_EQ(audio.samples_generated, 0);
    EXPECT_EQ(audio.cycles, 0);
}

// Teste de escrita no YM2612
TEST_F(MegaDriveAudioTest, YM2612Write) {
    // Escrever um valor no YM2612
    md_audio_write_ym2612(&audio, 0, 0x28, 0xF0); // Key-off para todos os canais
    
    // Verificar se o valor foi escrito corretamente (indiretamente, verificando o status)
    uint8_t status = md_audio_read_ym2612(&audio, 0, 0);
    
    // O status deve ter o bit 7 zerado (não ocupado)
    EXPECT_EQ(status & 0x80, 0);
}

// Teste de escrita no SN76489
TEST_F(MegaDriveAudioTest, SN76489Write) {
    // Escrever um valor no SN76489
    md_audio_write_sn76489(&audio, 0x9F); // Volume máximo no canal 0
    
    // Não há como ler diretamente do SN76489, então verificamos indiretamente
    // gerando amostras e verificando se há saída de áudio
    
    const int num_samples = 100;
    int16_t buffer_left[num_samples];
    int16_t buffer_right[num_samples];
    
    // Configurar apenas o SN76489 para gerar som
    audio.ym2612_volume = 0.0f;
    audio.sn76489_volume = 1.0f;
    
    // Gerar amostras
    md_audio_update(&audio, buffer_left, buffer_right, num_samples);
    
    // Verificar se há saída de áudio (pelo menos um valor não-zero)
    bool has_output = false;
    for (int i = 0; i < num_samples; i++) {
        if (buffer_left[i] != 0 || buffer_right[i] != 0) {
            has_output = true;
            break;
        }
    }
    
    EXPECT_TRUE(has_output);
}

// Teste de configuração estéreo do SN76489
TEST_F(MegaDriveAudioTest, SN76489StereoConfiguration) {
    // Configurar estéreo (canal 0 à esquerda, canal 1 à direita)
    uint8_t stereo_config = 0x01 | (0x01 << 5); // Bit 0 e bit 5
    md_audio_set_sn76489_stereo(&audio, stereo_config);
    
    // Verificar se a configuração foi aplicada (indiretamente)
    EXPECT_EQ(audio.sn76489.stereo, stereo_config);
}

// Teste de configuração de taxa de amostragem
TEST_F(MegaDriveAudioTest, SampleRateConfiguration) {
    // Definir nova taxa de amostragem
    uint32_t new_rate = 48000;
    md_audio_set_sample_rate(&audio, new_rate);
    
    // Verificar se a taxa foi aplicada
    EXPECT_EQ(audio.sample_rate, new_rate);
    EXPECT_FLOAT_EQ(audio.cycles_per_sample, (float)audio.system_clock / (float)new_rate);
    
    // Verificar se a taxa foi propagada para os chips
    EXPECT_EQ(audio.ym2612.sample_rate, new_rate);
    EXPECT_EQ(audio.sn76489.rate, new_rate);
}

// Teste de configuração de volume
TEST_F(MegaDriveAudioTest, VolumeConfiguration) {
    // Definir novos volumes
    md_audio_set_ym2612_volume(&audio, 0.5f);
    md_audio_set_sn76489_volume(&audio, 0.3f);
    md_audio_set_master_volume(&audio, 0.8f);
    
    // Verificar se os volumes foram aplicados
    EXPECT_FLOAT_EQ(audio.ym2612_volume, 0.5f);
    EXPECT_FLOAT_EQ(audio.sn76489_volume, 0.3f);
    EXPECT_FLOAT_EQ(audio.master_volume, 0.8f);
    
    // Testar limites
    md_audio_set_ym2612_volume(&audio, -0.1f);
    EXPECT_FLOAT_EQ(audio.ym2612_volume, 0.0f);
    
    md_audio_set_sn76489_volume(&audio, 1.5f);
    EXPECT_FLOAT_EQ(audio.sn76489_volume, 1.0f);
}

// Teste de habilitação/desabilitação
TEST_F(MegaDriveAudioTest, EnableDisable) {
    // Desabilitar o sistema
    md_audio_set_enabled(&audio, false);
    EXPECT_FALSE(audio.enabled);
    
    // Habilitar o sistema
    md_audio_set_enabled(&audio, true);
    EXPECT_TRUE(audio.enabled);
}

// Teste de avanço de ciclos
TEST_F(MegaDriveAudioTest, CycleAdvancement) {
    // Avançar alguns ciclos
    uint32_t cycles = 1000;
    md_audio_advance(&audio, cycles);
    
    // Verificar se os ciclos foram contabilizados
    EXPECT_EQ(audio.cycles, cycles);
    
    // Verificar se as amostras esperadas foram calculadas
    uint32_t expected_samples = (uint32_t)((float)cycles / audio.cycles_per_sample);
    EXPECT_EQ(audio.samples_generated, expected_samples);
}

// Teste de geração de amostras
TEST_F(MegaDriveAudioTest, SampleGeneration) {
    // Configurar YM2612 para gerar um tom simples
    md_audio_write_ym2612(&audio, 0, 0x22, 0x00); // LFO desativado
    md_audio_write_ym2612(&audio, 0, 0x27, 0x00); // Timer desativado
    md_audio_write_ym2612(&audio, 0, 0x28, 0x00); // Key-off para todos os canais
    md_audio_write_ym2612(&audio, 0, 0x30, 0x71); // DT1/MUL para operador 1, canal 1
    md_audio_write_ym2612(&audio, 0, 0x34, 0x0D); // DT1/MUL para operador 2, canal 1
    md_audio_write_ym2612(&audio, 0, 0x38, 0x33); // DT1/MUL para operador 3, canal 1
    md_audio_write_ym2612(&audio, 0, 0x3C, 0x01); // DT1/MUL para operador 4, canal 1
    md_audio_write_ym2612(&audio, 0, 0x40, 0x23); // TL para operador 1, canal 1
    md_audio_write_ym2612(&audio, 0, 0x44, 0x2D); // TL para operador 2, canal 1
    md_audio_write_ym2612(&audio, 0, 0x48, 0x26); // TL para operador 3, canal 1
    md_audio_write_ym2612(&audio, 0, 0x4C, 0x00); // TL para operador 4, canal 1
    md_audio_write_ym2612(&audio, 0, 0x50, 0x5F); // RS/AR para operador 1, canal 1
    md_audio_write_ym2612(&audio, 0, 0x54, 0x99); // RS/AR para operador 2, canal 1
    md_audio_write_ym2612(&audio, 0, 0x58, 0x5F); // RS/AR para operador 3, canal 1
    md_audio_write_ym2612(&audio, 0, 0x5C, 0x94); // RS/AR para operador 4, canal 1
    md_audio_write_ym2612(&audio, 0, 0x60, 0x05); // AM/D1R para operador 1, canal 1
    md_audio_write_ym2612(&audio, 0, 0x64, 0x05); // AM/D1R para operador 2, canal 1
    md_audio_write_ym2612(&audio, 0, 0x68, 0x05); // AM/D1R para operador 3, canal 1
    md_audio_write_ym2612(&audio, 0, 0x6C, 0x07); // AM/D1R para operador 4, canal 1
    md_audio_write_ym2612(&audio, 0, 0x70, 0x02); // D2R para operador 1, canal 1
    md_audio_write_ym2612(&audio, 0, 0x74, 0x02); // D2R para operador 2, canal 1
    md_audio_write_ym2612(&audio, 0, 0x78, 0x02); // D2R para operador 3, canal 1
    md_audio_write_ym2612(&audio, 0, 0x7C, 0x02); // D2R para operador 4, canal 1
    md_audio_write_ym2612(&audio, 0, 0x80, 0x11); // D1L/RR para operador 1, canal 1
    md_audio_write_ym2612(&audio, 0, 0x84, 0x11); // D1L/RR para operador 2, canal 1
    md_audio_write_ym2612(&audio, 0, 0x88, 0x11); // D1L/RR para operador 3, canal 1
    md_audio_write_ym2612(&audio, 0, 0x8C, 0xA6); // D1L/RR para operador 4, canal 1
    md_audio_write_ym2612(&audio, 0, 0x90, 0x00); // SSG-EG para operador 1, canal 1
    md_audio_write_ym2612(&audio, 0, 0x94, 0x00); // SSG-EG para operador 2, canal 1
    md_audio_write_ym2612(&audio, 0, 0x98, 0x00); // SSG-EG para operador 3, canal 1
    md_audio_write_ym2612(&audio, 0, 0x9C, 0x00); // SSG-EG para operador 4, canal 1
    md_audio_write_ym2612(&audio, 0, 0xA0, 0x22); // Frequência (parte baixa) para canal 1
    md_audio_write_ym2612(&audio, 0, 0xA4, 0x01); // Frequência (parte alta) para canal 1
    md_audio_write_ym2612(&audio, 0, 0xB0, 0x32); // Feedback/Algoritmo para canal 1
    md_audio_write_ym2612(&audio, 0, 0xB4, 0xC0); // Estéreo e LFO para canal 1
    md_audio_write_ym2612(&audio, 0, 0x28, 0xF1); // Key-on para canal 1
    
    // Configurar SN76489 para gerar um tom simples
    md_audio_write_sn76489(&audio, 0x80 | 0x00 | 0x0A); // Latch + Canal 0 + Valor 10 (parte baixa do tom)
    md_audio_write_sn76489(&audio, 0x00); // Valor 0 (parte alta do tom)
    md_audio_write_sn76489(&audio, 0x90 | 0x00 | 0x0F); // Latch + Canal 0 + Volume + Valor 0 (volume máximo)
    
    // Gerar algumas amostras
    const int num_samples = 1000;
    int16_t buffer_left[num_samples];
    int16_t buffer_right[num_samples];
    
    int32_t generated = md_audio_update(&audio, buffer_left, buffer_right, num_samples);
    
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
}

// Teste de redimensionamento de buffer
TEST_F(MegaDriveAudioTest, BufferResize) {
    // Tamanho original do buffer
    uint32_t original_size = audio.buffer_size;
    
    // Novo tamanho do buffer
    uint32_t new_size = original_size * 2;
    
    // Redimensionar buffer
    emu_error_t error = md_audio_resize_buffer(&audio, new_size);
    
    // Verificar se o redimensionamento foi bem-sucedido
    EXPECT_EQ(error, EMU_ERROR_NONE);
    EXPECT_EQ(audio.buffer_size, new_size);
    
    // Verificar se os buffers foram alocados com o novo tamanho
    EXPECT_NE(audio.buffer_left, nullptr);
    EXPECT_NE(audio.buffer_right, nullptr);
    EXPECT_NE(audio.ym2612_buffer_left, nullptr);
    EXPECT_NE(audio.ym2612_buffer_right, nullptr);
    EXPECT_NE(audio.sn76489_buffer_left, nullptr);
    EXPECT_NE(audio.sn76489_buffer_right, nullptr);
}

// Teste de mixagem de áudio
TEST_F(MegaDriveAudioTest, AudioMixing) {
    // Configurar volumes para teste
    md_audio_set_ym2612_volume(&audio, 1.0f);
    md_audio_set_sn76489_volume(&audio, 1.0f);
    md_audio_set_master_volume(&audio, 0.5f);
    
    // Configurar YM2612 para gerar um tom simples
    md_audio_write_ym2612(&audio, 0, 0x28, 0x00); // Key-off para todos os canais
    md_audio_write_ym2612(&audio, 0, 0xA0, 0x44); // Frequência (parte baixa) para canal 1
    md_audio_write_ym2612(&audio, 0, 0xA4, 0x01); // Frequência (parte alta) para canal 1
    md_audio_write_ym2612(&audio, 0, 0x28, 0xF1); // Key-on para canal 1
    
    // Configurar SN76489 para gerar um tom simples
    md_audio_write_sn76489(&audio, 0x80 | 0x00 | 0x0A); // Latch + Canal 0 + Valor 10 (parte baixa do tom)
    md_audio_write_sn76489(&audio, 0x00); // Valor 0 (parte alta do tom)
    md_audio_write_sn76489(&audio, 0x90 | 0x00 | 0x00); // Latch + Canal 0 + Volume + Valor 0 (volume máximo)
    
    // Gerar amostras com ambos os chips
    const int num_samples = 100;
    int16_t buffer_both_left[num_samples];
    int16_t buffer_both_right[num_samples];
    
    md_audio_update(&audio, buffer_both_left, buffer_both_right, num_samples);
    
    // Gerar amostras apenas com YM2612
    md_audio_set_sn76489_volume(&audio, 0.0f);
    int16_t buffer_ym2612_left[num_samples];
    int16_t buffer_ym2612_right[num_samples];
    
    md_audio_update(&audio, buffer_ym2612_left, buffer_ym2612_right, num_samples);
    
    // Gerar amostras apenas com SN76489
    md_audio_set_ym2612_volume(&audio, 0.0f);
    md_audio_set_sn76489_volume(&audio, 1.0f);
    int16_t buffer_sn76489_left[num_samples];
    int16_t buffer_sn76489_right[num_samples];
    
    md_audio_update(&audio, buffer_sn76489_left, buffer_sn76489_right, num_samples);
    
    // Verificar se a mixagem está funcionando corretamente
    // A soma dos sinais individuais deve ser diferente do sinal mixado
    bool is_different = false;
    for (int i = 0; i < num_samples; i++) {
        if (buffer_both_left[i] != buffer_ym2612_left[i] && 
            buffer_both_left[i] != buffer_sn76489_left[i]) {
            is_different = true;
            break;
        }
    }
    
    EXPECT_TRUE(is_different);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
