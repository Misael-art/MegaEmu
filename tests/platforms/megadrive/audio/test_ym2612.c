/**
 * @file test_ym2612.c
 * @brief Testes unitários para o adaptador YM2612
 * @version 1.0
 * @date 2024-03-21
 */

#include "unity.h"
#include "ym2612_adapter.h"

// Contexto global para testes
static ym2612_context_t *ctx;

void setUp(void) {
  ctx = ym2612_create();
  TEST_ASSERT_NOT_NULL(ctx);
}

void tearDown(void) {
  if (ctx) {
    ym2612_destroy(ctx);
    ctx = NULL;
  }
}

/**
 * @brief Testa a criação e destruição do contexto
 */
void test_ym2612_create_destroy(void) {
  ym2612_context_t *test_ctx = ym2612_create();
  TEST_ASSERT_NOT_NULL(test_ctx);
  ym2612_destroy(test_ctx);
}

/**
 * @brief Testa o reset do YM2612
 */
void test_ym2612_reset(void) {
  // Configura alguns registradores
  ym2612_write_reg(ctx, 0, 0x28, 0xF0); // Key on para canal 0
  ym2612_write_reg(ctx, 0, 0x30, 0x71); // DT1/MUL para operador 1
  ym2612_write_reg(ctx, 0, 0x40, 0x23); // TL para operador 1

  // Reseta
  ym2612_reset(ctx);

  // Verifica se os registradores foram resetados
  TEST_ASSERT_EQUAL_UINT8(0, ym2612_read_reg(ctx, 0, 0x28));
  TEST_ASSERT_EQUAL_UINT8(0, ym2612_read_reg(ctx, 0, 0x30));
  TEST_ASSERT_EQUAL_UINT8(0, ym2612_read_reg(ctx, 0, 0x40));

  // Verifica se os canais foram resetados
  for (int i = 0; i < YM2612_NUM_CHANNELS; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].algorithm);
    TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].feedback);
    TEST_ASSERT_FALSE(ctx->channels[i].key_on);

    for (int j = 0; j < 4; j++) {
      TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].operators[j].multiple);
      TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].operators[j].detune);
      TEST_ASSERT_EQUAL_UINT8(127, ctx->channels[i].operators[j].total_level);
      TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].operators[j].rate_scaling);
      TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].operators[j].attack_rate);
      TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].operators[j].decay_rate);
      TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].operators[j].sustain_level);
      TEST_ASSERT_EQUAL_UINT8(0, ctx->channels[i].operators[j].release_rate);
    }
  }
}

/**
 * @brief Testa a escrita e leitura de registradores
 */
void test_ym2612_registers(void) {
  // Testa escrita/leitura de registradores do banco 0
  ym2612_write_reg(ctx, 0, 0x30, 0x71); // DT1/MUL para operador 1
  TEST_ASSERT_EQUAL_UINT8(0x71, ym2612_read_reg(ctx, 0, 0x30));

  // Testa escrita/leitura de registradores do banco 1
  ym2612_write_reg(ctx, 1, 0xA4, 0x22); // Frequência canal 4
  TEST_ASSERT_EQUAL_UINT8(0x22, ym2612_read_reg(ctx, 1, 0xA4));

  // Testa registrador de key on/off
  ym2612_write_reg(ctx, 0, 0x28, 0xF0); // Key on para canal 0
  TEST_ASSERT_TRUE(ctx->channels[0].key_on);
}

/**
 * @brief Testa os timers
 */
void test_ym2612_timers(void) {
  bool timer_a_fired = false;
  bool timer_b_fired = false;

  // Configura callbacks dos timers
  ym2612_set_timer_a_callback(
      ctx,
      [](void *data) {
        bool *fired = (bool *)data;
        *fired = true;
      },
      &timer_a_fired);

  ym2612_set_timer_b_callback(
      ctx,
      [](void *data) {
        bool *fired = (bool *)data;
        *fired = true;
      },
      &timer_b_fired);

  // Configura e inicia timer A
  ym2612_write_reg(ctx, 0, 0x24, 0x80); // Timer A MSB
  ym2612_write_reg(ctx, 0, 0x25, 0x00); // Timer A LSB
  ym2612_write_reg(ctx, 0, 0x27, 0x15); // Timer A enable e reset

  // Configura e inicia timer B
  ym2612_write_reg(ctx, 0, 0x26, 0x80); // Timer B
  ym2612_write_reg(ctx, 0, 0x27, 0x2A); // Timer B enable e reset

  // Executa alguns ciclos dos timers
  for (int i = 0; i < 1000; i++) {
    ym2612_timer_tick(ctx);
  }

  // Verifica se os timers dispararam
  TEST_ASSERT_TRUE(timer_a_fired);
  TEST_ASSERT_TRUE(timer_b_fired);
}

/**
 * @brief Testa a configuração de operadores FM
 */
void test_ym2612_operators(void) {
  // Configura operador 1 do canal 0
  ym2612_write_reg(ctx, 0, 0x30, 0x71); // DT1/MUL
  ym2612_write_reg(ctx, 0, 0x40, 0x23); // TL
  ym2612_write_reg(ctx, 0, 0x50, 0x1F); // RS/AR
  ym2612_write_reg(ctx, 0, 0x60, 0x1B); // AM/D1R
  ym2612_write_reg(ctx, 0, 0x70, 0x13); // D2R
  ym2612_write_reg(ctx, 0, 0x80, 0x0F); // D1L/RR

  // Verifica configuração do operador
  TEST_ASSERT_EQUAL_UINT8(0x1, ctx->channels[0].operators[0].detune);
  TEST_ASSERT_EQUAL_UINT8(0x7, ctx->channels[0].operators[0].multiple);
  TEST_ASSERT_EQUAL_UINT8(0x23, ctx->channels[0].operators[0].total_level);
  TEST_ASSERT_EQUAL_UINT8(0x1, ctx->channels[0].operators[0].rate_scaling);
  TEST_ASSERT_EQUAL_UINT8(0xF, ctx->channels[0].operators[0].attack_rate);
  TEST_ASSERT_EQUAL_UINT8(0x1B, ctx->channels[0].operators[0].decay_rate);
  TEST_ASSERT_EQUAL_UINT8(0x13, ctx->channels[0].operators[0].sustain_rate);
  TEST_ASSERT_EQUAL_UINT8(0x0F, ctx->channels[0].operators[0].release_rate);
}

/**
 * @brief Testa a configuração de canais FM
 */
void test_ym2612_channels(void) {
  // Configura canal 0
  ym2612_write_reg(ctx, 0, 0xB0, 0x32); // Feedback/Algorithm
  ym2612_write_reg(ctx, 0, 0xA4, 0x22); // Frequência (block/fnum MSB)
  ym2612_write_reg(ctx, 0, 0xA0, 0x47); // Frequência (fnum LSB)
  ym2612_write_reg(ctx, 0, 0xB4, 0xC0); // LR/AMS/FMS

  // Verifica configuração do canal
  TEST_ASSERT_EQUAL_UINT8(0x2, ctx->channels[0].algorithm);
  TEST_ASSERT_EQUAL_UINT8(0x3, ctx->channels[0].feedback);
  TEST_ASSERT_EQUAL_UINT16(0x247, ctx->channels[0].frequency);
  TEST_ASSERT_EQUAL_UINT8(0x2, ctx->channels[0].block);
  TEST_ASSERT_TRUE(ctx->channels[0].left_enable);
  TEST_ASSERT_TRUE(ctx->channels[0].right_enable);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ym2612_create_destroy);
  RUN_TEST(test_ym2612_reset);
  RUN_TEST(test_ym2612_registers);
  RUN_TEST(test_ym2612_timers);
  RUN_TEST(test_ym2612_operators);
  RUN_TEST(test_ym2612_channels);
  return UNITY_END();
}
