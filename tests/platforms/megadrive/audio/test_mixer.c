/**
 * @file test_mixer.c
 * @brief Testes unitários para o mixer de áudio
 * @version 1.0
 * @date 2024-03-21
 */

#include "audio_mixer.h"
#include "unity.h"

// Contexto global para testes
static audio_mixer_t *mixer;

void setUp(void) {
  audio_mixer_config_t config = {.sample_rate = 44100,
                                 .buffer_size = AUDIO_BUFFER_SIZE,
                                 .fm_volume = 1.0f,
                                 .psg_volume = 1.0f,
                                 .master_volume = 1.0f};
  mixer = audio_mixer_create(&config);
  TEST_ASSERT_NOT_NULL(mixer);
}

void tearDown(void) {
  if (mixer) {
    audio_mixer_destroy(mixer);
    mixer = NULL;
  }
}

/**
 * @brief Testa a criação e destruição do mixer
 */
void test_mixer_create_destroy(void) {
  audio_mixer_config_t config = {.sample_rate = 44100,
                                 .buffer_size = AUDIO_BUFFER_SIZE,
                                 .fm_volume = 1.0f,
                                 .psg_volume = 1.0f,
                                 .master_volume = 1.0f};
  audio_mixer_t *test_mixer = audio_mixer_create(&config);
  TEST_ASSERT_NOT_NULL(test_mixer);
  audio_mixer_destroy(test_mixer);
}

/**
 * @brief Testa o reset do mixer
 */
void test_mixer_reset(void) {
  // Processa algumas amostras
  audio_mixer_process(mixer, 100);

  // Reseta o mixer
  audio_mixer_reset(mixer);

  // Verifica se o buffer foi limpo
  TEST_ASSERT_EQUAL_UINT32(0, mixer->write_pos);
  TEST_ASSERT_EQUAL_UINT32(0, mixer->read_pos);
  TEST_ASSERT_FALSE(audio_mixer_buffer_full(mixer));
}

/**
 * @brief Testa o controle de volume
 */
void test_mixer_volume_control(void) {
  // Testa volume FM
  audio_mixer_set_fm_volume(mixer, 0.5f);
  TEST_ASSERT_EQUAL_FLOAT(0.5f, mixer->fm_volume);

  // Testa volume PSG
  audio_mixer_set_psg_volume(mixer, 0.75f);
  TEST_ASSERT_EQUAL_FLOAT(0.75f, mixer->psg_volume);

  // Testa volume master
  audio_mixer_set_master_volume(mixer, 0.8f);
  TEST_ASSERT_EQUAL_FLOAT(0.8f, mixer->master_volume);

  // Testa limites de volume
  audio_mixer_set_fm_volume(mixer, 1.5f);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, mixer->fm_volume);

  audio_mixer_set_psg_volume(mixer, -0.5f);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, mixer->psg_volume);
}

/**
 * @brief Testa o processamento de amostras
 */
void test_mixer_process(void) {
  // Processa algumas amostras
  audio_mixer_process(mixer, 100);

  // Verifica se o buffer foi preenchido
  TEST_ASSERT_EQUAL_UINT32(100 * AUDIO_CHANNELS, mixer->write_pos);
  TEST_ASSERT_EQUAL_UINT32(0, mixer->read_pos);
}

/**
 * @brief Testa a leitura de amostras
 */
void test_mixer_read(void) {
  // Processa algumas amostras
  audio_mixer_process(mixer, 100);

  // Buffer para leitura
  int16_t buffer[200];

  // Lê as amostras
  uint32_t samples_read = audio_mixer_read(mixer, buffer, 100);

  // Verifica se leu o número correto de amostras
  TEST_ASSERT_EQUAL_UINT32(100, samples_read);

  // Verifica se as posições foram atualizadas
  TEST_ASSERT_EQUAL_UINT32(100 * AUDIO_CHANNELS, mixer->write_pos);
  TEST_ASSERT_EQUAL_UINT32(100 * AUDIO_CHANNELS, mixer->read_pos);
}

/**
 * @brief Testa o estado do buffer
 */
void test_mixer_buffer_state(void) {
  // Buffer inicialmente vazio
  TEST_ASSERT_FALSE(audio_mixer_buffer_full(mixer));
  TEST_ASSERT_EQUAL_UINT32(0, audio_mixer_available_samples(mixer));

  // Preenche metade do buffer
  audio_mixer_process(mixer, AUDIO_BUFFER_SIZE / 2);
  TEST_ASSERT_FALSE(audio_mixer_buffer_full(mixer));
  TEST_ASSERT_EQUAL_UINT32(AUDIO_BUFFER_SIZE / 2,
                           audio_mixer_available_samples(mixer));

  // Preenche o buffer completamente
  audio_mixer_process(mixer, AUDIO_BUFFER_SIZE / 2);
  TEST_ASSERT_TRUE(audio_mixer_buffer_full(mixer));
  TEST_ASSERT_EQUAL_UINT32(AUDIO_BUFFER_SIZE,
                           audio_mixer_available_samples(mixer));
}

/**
 * @brief Testa o comportamento do buffer circular
 */
void test_mixer_circular_buffer(void) {
  // Preenche o buffer
  audio_mixer_process(mixer, AUDIO_BUFFER_SIZE);
  TEST_ASSERT_TRUE(audio_mixer_buffer_full(mixer));

  // Lê metade do buffer
  int16_t buffer[AUDIO_BUFFER_SIZE];
  audio_mixer_read(mixer, buffer, AUDIO_BUFFER_SIZE / 2);

  // Verifica estado do buffer
  TEST_ASSERT_FALSE(audio_mixer_buffer_full(mixer));
  TEST_ASSERT_EQUAL_UINT32(AUDIO_BUFFER_SIZE / 2,
                           audio_mixer_available_samples(mixer));

  // Adiciona mais amostras
  audio_mixer_process(mixer, AUDIO_BUFFER_SIZE / 2);
  TEST_ASSERT_TRUE(audio_mixer_buffer_full(mixer));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_mixer_create_destroy);
  RUN_TEST(test_mixer_reset);
  RUN_TEST(test_mixer_volume_control);
  RUN_TEST(test_mixer_process);
  RUN_TEST(test_mixer_read);
  RUN_TEST(test_mixer_buffer_state);
  RUN_TEST(test_mixer_circular_buffer);
  return UNITY_END();
}
