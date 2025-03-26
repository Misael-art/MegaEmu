/**
 * @file test_apu_mixer.c
 * @brief Testes unitários para o mixer de áudio do APU
 */

#include "../../../../src/platforms/nes/apu/apu_mixer.h"
#include <math.h>
#include <unity.h>

static nes_apu_mixer_t mixer;

void setUp(void) { apu_mixer_init(&mixer, 44100); }

void tearDown(void) {
  // Nada a fazer
}

void test_mixer_initialization(void) {
  // Verificar volumes iniciais
  TEST_ASSERT_EQUAL_FLOAT(1.0f, mixer.pulse1_volume);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, mixer.pulse2_volume);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, mixer.triangle_volume);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, mixer.noise_volume);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, mixer.dmc_volume);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, mixer.master_volume);

  // Verificar configurações iniciais
  TEST_ASSERT_TRUE(mixer.filter_enabled);
  TEST_ASSERT_FALSE(mixer.high_quality_mode);
  TEST_ASSERT_EQUAL_UINT32(44100, mixer.sample_rate);
}

void test_mixer_volume_control(void) {
  // Configurar volumes diferentes
  apu_mixer_set_volumes(&mixer, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.4f);

  TEST_ASSERT_EQUAL_FLOAT(0.5f, mixer.pulse1_volume);
  TEST_ASSERT_EQUAL_FLOAT(0.6f, mixer.pulse2_volume);
  TEST_ASSERT_EQUAL_FLOAT(0.7f, mixer.triangle_volume);
  TEST_ASSERT_EQUAL_FLOAT(0.8f, mixer.noise_volume);
  TEST_ASSERT_EQUAL_FLOAT(0.9f, mixer.dmc_volume);
  TEST_ASSERT_EQUAL_FLOAT(0.4f, mixer.master_volume);
}

void test_mixer_quality_modes(void) {
  // Testar modo normal
  TEST_ASSERT_FALSE(mixer.high_quality_mode);
  TEST_ASSERT_EQUAL_FLOAT(0.6f, mixer.lpf_beta);

  // Mudar para modo de alta qualidade
  apu_mixer_set_quality(&mixer, true);
  TEST_ASSERT_TRUE(mixer.high_quality_mode);
  TEST_ASSERT_EQUAL_FLOAT(0.8f, mixer.lpf_beta);

  // Voltar para modo normal
  apu_mixer_set_quality(&mixer, false);
  TEST_ASSERT_FALSE(mixer.high_quality_mode);
  TEST_ASSERT_EQUAL_FLOAT(0.6f, mixer.lpf_beta);
}

void test_mixer_filter(void) {
  // Testar filtro habilitado
  TEST_ASSERT_TRUE(mixer.filter_enabled);

  // Desabilitar filtro
  apu_mixer_enable_filter(&mixer, false);
  TEST_ASSERT_FALSE(mixer.filter_enabled);

  // Habilitar filtro novamente
  apu_mixer_enable_filter(&mixer, true);
  TEST_ASSERT_TRUE(mixer.filter_enabled);
}

void test_mixer_output(void) {
  // Testar saída com todos os canais em silêncio
  float output = apu_mixer_mix(&mixer, 0, 0, 0, 0, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, output);

  // Testar saída com pulsos em volume máximo
  output = apu_mixer_mix(&mixer, 15, 15, 0, 0, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.95f, output);

  // Testar saída com triangle em volume máximo
  output = apu_mixer_mix(&mixer, 0, 0, 15, 0, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, output);

  // Testar saída com noise em volume máximo
  output = apu_mixer_mix(&mixer, 0, 0, 0, 15, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, output);

  // Testar saída com DMC em volume máximo
  output = apu_mixer_mix(&mixer, 0, 0, 0, 0, 127);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, output);
}

void test_mixer_filter_response(void) {
  // Configurar mixer para teste do filtro
  apu_mixer_enable_filter(&mixer, true);
  apu_mixer_reset(&mixer);

  // Simular uma onda quadrada e verificar resposta do filtro
  float prev_output = 0.0f;
  for (int i = 0; i < 100; i++) {
    float input = (i % 2) ? 1.0f : -1.0f;
    float output = apu_mixer_mix(&mixer, input * 15, 0, 0, 0, 0);

    // Verificar se o filtro está suavizando a transição
    if (i > 0) {
      float delta = fabs(output - prev_output);
      TEST_ASSERT_TRUE(delta < 1.0f);
    }

    prev_output = output;
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_mixer_initialization);
  RUN_TEST(test_mixer_volume_control);
  RUN_TEST(test_mixer_quality_modes);
  RUN_TEST(test_mixer_filter);
  RUN_TEST(test_mixer_output);
  RUN_TEST(test_mixer_filter_response);

  return UNITY_END();
}
