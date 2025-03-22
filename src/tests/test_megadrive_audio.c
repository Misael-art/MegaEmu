/** * @file test_megadrive_audio.c * @brief Testes unitários para o sistema de áudio do Mega Drive * * Este arquivo implementa testes para o chip de som YM2612 do Mega Drive * e a interface de áudio relacionada. */#include <stdio.h>#include <stdlib.h>#include <string.h>#include <assert.h>#include "../platforms/megadrive/audio/ym2612.h"#include "../utils/common_types.h"// Tamanho de buffer para teste#define TEST_BUFFER_SIZE 735 // ~1/60s a 44.1kHz// Buffers para saída de áudiostatic int16_t left_buffer[TEST_BUFFER_SIZE];static int16_t right_buffer[TEST_BUFFER_SIZE];/** * @brief Testa a inicialização e reset do chip YM2612 */static void test_ym2612_init_reset(void){    printf("Teste: Inicialização e Reset do YM2612\n");    ym2612_t chip;    // Testar inicialização    emu_error_t result = ym2612_init(&chip, YM2612_CLOCK_FREQ, 44100);    assert(result == EMU_ERROR_NONE);    // Verificar valores iniciais    assert(chip.clock == YM2612_CLOCK_FREQ);    assert(chip.rate == 44100);    assert(chip.lfo_enable == 0);    assert(chip.timer_a_val == 0);    assert(chip.timer_b_val == 0);    // Testar reset explícito    chip.lfo_enable = 1;    chip.timer_a_val = 123;    result = ym2612_reset(&chip);    assert(result == EMU_ERROR_NONE);    // Verificar se os valores foram resetados    assert(chip.lfo_enable == 0);    assert(chip.timer_a_val == 0);    // Testar parâmetros inválidos    result = ym2612_init(NULL, YM2612_CLOCK_FREQ, 44100);    assert(result == EMU_ERROR_INVALID_PARAM);    result = ym2612_reset(NULL);    assert(result == EMU_ERROR_INVALID_PARAM);    ym2612_shutdown(&chip);    printf("OK\n");}/** * @brief Testa a escrita e leitura de registradores */static void test_ym2612_registers(void){    printf("Teste: Acesso a Registradores do YM2612\n");    ym2612_t chip;    ym2612_init(&chip, YM2612_CLOCK_FREQ, 44100);    // Testar escrita e leitura de registradores    ym2612_write(&chip, 0, 0x22, 0x08); // LFO enable    assert(chip.lfo_enable == 1);    assert(chip.lfo_freq == 0);    ym2612_write(&chip, 0, 0x27, 0x03); // Timer enable    assert(chip.timer_a_enable == true);    assert(chip.timer_b_enable == true);    // Testar leitura    uint8_t status = ym2612_read(&chip, 0, 0x00);    assert((status & 0x03) == 0x03); // Ambos os timers ativos    // Testar escrita em registradores de operadores    ym2612_write(&chip, 0, 0x30, 0x71); // Detune/Multiple para operador 0, canal 0    assert(chip.channels[0].operators[0].dt == 7);    assert(chip.channels[0].operators[0].mul == 1);    ym2612_write(&chip, 0, 0xB0, 0x11); // Frequência e key-off para canal 0    assert(chip.channels[0].freq_num == 0x100);    assert(chip.channels[0].block == 2);    assert(chip.channels[0].key_on == false);    ym2612_write(&chip, 0, 0xB0, 0x91); // Frequência e key-on para canal 0    assert(chip.channels[0].key_on == true);    // Testar parâmetros inválidos    ym2612_write(NULL, 0, 0x22, 0x08); // Não deve falhar, apenas retornar    ym2612_shutdown(&chip);    printf("OK\n");}/** * @brief Testa a geração de amostras */static void test_ym2612_sample_generation(void){    printf("Teste: Geração de Amostras do YM2612\n");    ym2612_t chip;    ym2612_init(&chip, YM2612_CLOCK_FREQ, 44100);    // Configurar para gerar som    // Na implementação atual, gera silêncio, mas estamos testando a API    // Limpar buffers    memset(left_buffer, 0, sizeof(left_buffer));    memset(right_buffer, 0, sizeof(right_buffer));    // Gerar amostras    int samples_generated = ym2612_update(&chip, left_buffer, right_buffer, TEST_BUFFER_SIZE);    // Verificar se gerou a quantidade correta    assert(samples_generated == TEST_BUFFER_SIZE);    assert(chip.samples_generated == TEST_BUFFER_SIZE);    // Testar parâmetros inválidos    samples_generated = ym2612_update(NULL, left_buffer, right_buffer, TEST_BUFFER_SIZE);    assert(samples_generated == 0);    samples_generated = ym2612_update(&chip, NULL, right_buffer, TEST_BUFFER_SIZE);    assert(samples_generated == 0);    samples_generated = ym2612_update(&chip, left_buffer, NULL, TEST_BUFFER_SIZE);    assert(samples_generated == 0);    samples_generated = ym2612_update(&chip, left_buffer, right_buffer, 0);    assert(samples_generated == 0);    ym2612_shutdown(&chip);    printf("OK\n");}/** * @brief Testa a configuração de parâmetros */static void test_ym2612_configuration(void){    printf("Teste: Configuração de Parâmetros do YM2612\n");    ym2612_t chip;    ym2612_init(&chip, YM2612_CLOCK_FREQ, 44100);    // Testar mudança de taxa de amostragem    ym2612_set_sample_rate(&chip, 48000);    assert(chip.rate == 48000);    // Verificar atualização da razão de clock    float expected_ratio = (float)YM2612_CLOCK_FREQ / 48000.0f;    assert(fabs(chip.clock_ratio - expected_ratio) < 0.0001f);    // Testar mudança de clock    ym2612_set_clock(&chip, 8000000);    assert(chip.clock == 8000000);    // Verificar atualização da razão de clock    expected_ratio = 8000000.0f / 48000.0f;    assert(fabs(chip.clock_ratio - expected_ratio) < 0.0001f);    // Testar parâmetros inválidos    ym2612_set_sample_rate(NULL, 48000); // Não deve falhar    ym2612_set_sample_rate(&chip, 0);    // Não deve fazer nada    assert(chip.rate == 48000);          // Não deve ter mudado    ym2612_set_clock(NULL, 8000000); // Não deve falhar    ym2612_set_clock(&chip, 0);      // Não deve fazer nada    assert(chip.clock == 8000000);   // Não deve ter mudado    ym2612_shutdown(&chip);    printf("OK\n");}/** * @brief Função principal que executa todos os testes */int main(void){    printf("==== TESTES DO SISTEMA DE ÁUDIO DO MEGA DRIVE ====\n");    test_ym2612_init_reset();    test_ym2612_registers();    test_ym2612_sample_generation();    test_ym2612_configuration();    printf("Todos os testes de áudio do Mega Drive passaram!\n");    return 0;}