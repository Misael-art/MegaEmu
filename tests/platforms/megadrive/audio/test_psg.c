/**
 * @file test_psg.c
 * @brief Testes unitários para o adaptador PSG
 * @version 1.0
 * @date 2024-03-21
 */

#include "psg_adapter.h"
#include "unity.h"

// Contexto global para testes
static psg_context_t *ctx;

void setUp(void) {
  ctx = psg_create(PSG_CLOCK, 44100);
  TEST_ASSERT_NOT_NULL(ctx);
}

void tearDown(void) {
  if (ctx) {
    psg_destroy(ctx);
    ctx = NULL;
  }
}

/**
 * @brief Testa a criação e destruição do contexto
 */
void test_psg_create_destroy(void) {
  psg_context_t *test_ctx = psg_create(PSG_CLOCK, 44100);
  TEST_ASSERT_NOT_NULL(test_ctx);
  psg_destroy(test_ctx);
}

/**
 * @brief Testa o reset do PSG
 */
void test_psg_reset(void) {
  // Configura alguns valores
  psg_write(ctx, 0x80); // Latch canal 0, volume
  psg_write(ctx, 0x0F); // Volume máximo
  psg_write(ctx, 0x90); // Latch canal 0, frequência
  psg_write(ctx, 0x1F); // Frequência = 31

  // Reseta
  psg_reset(ctx);

  // Verifica se os valores foram resetados
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL_UINT16(0, ctx->channels[i].frequency);
    TEST_ASSERT_EQUAL_UINT8(0x0F, ctx->channels[i].volume);
    TEST_ASSERT_EQUAL_UINT16(0, ctx->channels[i].counter);
    TEST_ASSERT_FALSE(ctx->channels[i].output);
  }

  TEST_ASSERT_EQUAL_UINT8(0, ctx->noise.mode);
  TEST_ASSERT_EQUAL_UINT8(0, ctx->noise.shift_rate);
  TEST_ASSERT_EQUAL_UINT8(0x0F, ctx->noise.volume);
  TEST_ASSERT_EQUAL_UINT16(0, ctx->noise.counter);
  TEST_ASSERT_EQUAL_UINT16(0x8000, ctx->noise.shift_reg);
}

/**
 * @brief Testa a escrita de registradores
 */
void test_psg_write_registers(void) {
  // Testa escrita de volume do canal 0
  psg_write(ctx, 0x90); // Latch canal 0, volume
  psg_write(ctx, 0x0F); // Volume mínimo
  TEST_ASSERT_EQUAL_UINT8(0x0F, ctx->channels[0].volume);

  // Testa escrita de frequência do canal 1
  psg_write(ctx, 0xA0); // Latch canal 1, frequência
  psg_write(ctx, 0x1F); // Frequência = 31
  TEST_ASSERT_EQUAL_UINT16(0x1F, ctx->channels[1].frequency);

  // Testa escrita de configuração de ruído
  psg_write(ctx, 0xE0); // Latch canal de ruído
  psg_write(ctx, 0x03); // Modo periódico, taxa máxima
  TEST_ASSERT_EQUAL_UINT8(0, ctx->noise.mode);
  TEST_ASSERT_EQUAL_UINT8(3, ctx->noise.shift_rate);
}

/**
 * @brief Testa a geração de tons
 */
void test_psg_tone_generation(void) {
  // Configura canal 0 para gerar tom
  psg_write(ctx, 0x80); // Latch canal 0, volume
  psg_write(ctx, 0x0F); // Volume máximo
  psg_write(ctx, 0x90); // Latch canal 0, frequência
  psg_write(ctx, 0x1F); // Frequência = 31

  // Buffer para amostras
  int16_t buffer[100];

  // Gera algumas amostras
  psg_update(ctx, buffer, 100);

  // Verifica se há variação nas amostras (onda quadrada)
  bool has_variation = false;
  int16_t last_sample = buffer[0];

  for (int i = 1; i < 100; i++) {
    if (buffer[i] != last_sample) {
      has_variation = true;
      break;
    }
    last_sample = buffer[i];
  }

  TEST_ASSERT_TRUE(has_variation);
}

/**
 * @brief Testa a geração de ruído
 */
void test_psg_noise_generation(void) {
  // Configura canal de ruído
  psg_write(ctx, 0xE0); // Latch canal de ruído
  psg_write(ctx, 0x03); // Modo periódico, taxa máxima
  psg_write(ctx, 0xF0); // Volume do ruído
  psg_write(ctx, 0x0F); // Volume máximo

  // Buffer para amostras
  int16_t buffer[100];

  // Gera algumas amostras
  psg_update(ctx, buffer, 100);

  // Verifica se há variação nas amostras (ruído)
  bool has_variation = false;
  int16_t last_sample = buffer[0];

  for (int i = 1; i < 100; i++) {
    if (buffer[i] != last_sample) {
      has_variation = true;
      break;
    }
    last_sample = buffer[i];
  }

  TEST_ASSERT_TRUE(has_variation);
}

/**
 * @brief Testa o controle de volume
 */
void test_psg_volume_control(void) {
  // Configura canal 0 com volume máximo
  psg_write(ctx, 0x80); // Latch canal 0, volume
  psg_write(ctx, 0x0F); // Volume máximo
  psg_write(ctx, 0x90); // Latch canal 0, frequência
  psg_write(ctx, 0x1F); // Frequência = 31

  // Buffer para amostras
  int16_t buffer1[100];
  int16_t buffer2[100];

  // Gera amostras com volume máximo
  psg_update(ctx, buffer1, 100);

  // Muda para volume mínimo
  psg_write(ctx, 0x80); // Latch canal 0, volume
  psg_write(ctx, 0x00); // Volume mínimo

  // Gera amostras com volume mínimo
  psg_update(ctx, buffer2, 100);

  // Verifica se o volume mudou
  bool volume_changed = false;
  for (int i = 0; i < 100; i++) {
    if (abs(buffer1[i]) > abs(buffer2[i])) {
      volume_changed = true;
      break;
    }
  }

  TEST_ASSERT_TRUE(volume_changed);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_psg_create_destroy);
  RUN_TEST(test_psg_reset);
  RUN_TEST(test_psg_write_registers);
  RUN_TEST(test_psg_tone_generation);
  RUN_TEST(test_psg_noise_generation);
  RUN_TEST(test_psg_volume_control);
  return UNITY_END();
}
