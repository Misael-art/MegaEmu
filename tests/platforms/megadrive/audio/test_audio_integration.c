/**
 * @file test_audio_integration.c
 * @brief Testes de integração para o sistema de áudio
 * @version 1.0
 * @date 2024-03-21
 */

#include "audio_mixer.h"
#include "audio_profiler.h"
#include "psg_adapter.h"
#include "ym2612_adapter.h"
#include <math.h>
#include <string.h>
#include <unity.h>

// Constantes para os testes
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 2048
#define TEST_DURATION_SEC 5
#define SAMPLES_TO_PROCESS (SAMPLE_RATE * TEST_DURATION_SEC)

// Variáveis globais para os testes
static ym2612_context_t *fm;
static psg_context_t *psg;
static audio_mixer_t *mixer;
static audio_profiler_t *profiler;

// Configuração do mixer
static audio_mixer_config_t mixer_config = {.sample_rate = SAMPLE_RATE,
                                            .buffer_size = BUFFER_SIZE,
                                            .fm_volume = 0.75f,
                                            .psg_volume = 0.5f,
                                            .master_volume = 1.0f};

// Buffer para amostras
static int16_t sample_buffer[BUFFER_SIZE * 2];

void setUp(void) {
  // Cria instâncias dos componentes
  fm = ym2612_create();
  TEST_ASSERT_NOT_NULL(fm);

  psg = psg_create(PSG_CLOCK, SAMPLE_RATE);
  TEST_ASSERT_NOT_NULL(psg);

  mixer = audio_mixer_create(&mixer_config);
  TEST_ASSERT_NOT_NULL(mixer);

  profiler = audio_profiler_create();
  TEST_ASSERT_NOT_NULL(profiler);
}

void tearDown(void) {
  if (fm) {
    ym2612_destroy(fm);
    fm = NULL;
  }
  if (psg) {
    psg_destroy(psg);
    psg = NULL;
  }
  if (mixer) {
    audio_mixer_destroy(mixer);
    mixer = NULL;
  }
  if (profiler) {
    audio_profiler_destroy(profiler);
    profiler = NULL;
  }
}

// Função auxiliar para configurar um tom no PSG
static void setup_psg_tone(uint8_t channel, uint16_t freq, uint8_t volume) {
  // Configura frequência
  psg_write(psg, 0x80 | (channel << 5) | 0x00); // Latch/data byte
  psg_write(psg, freq & 0x0F);                  // Frequência LSB
  psg_write(psg, (freq >> 4) & 0x3F);           // Frequência MSB

  // Configura volume
  psg_write(psg, 0x90 | (channel << 5) | (volume & 0x0F));
}

// Função auxiliar para configurar um operador FM
static void setup_fm_operator(uint8_t channel, uint8_t operator_num,
                              uint8_t dt_mul, uint8_t tl, uint8_t rs_ar,
                              uint8_t am_dr, uint8_t sr, uint8_t sl_rr) {
  uint8_t base_addr = operator_num * 4;
  ym2612_write_reg(fm, 0, 0x30 + base_addr + channel, dt_mul); // DT1/MUL
  ym2612_write_reg(fm, 0, 0x40 + base_addr + channel, tl);     // Total Level
  ym2612_write_reg(fm, 0, 0x50 + base_addr + channel, rs_ar);  // RS/AR
  ym2612_write_reg(fm, 0, 0x60 + base_addr + channel, am_dr);  // AM/D1R
  ym2612_write_reg(fm, 0, 0x70 + base_addr + channel, sr);     // D2R
  ym2612_write_reg(fm, 0, 0x80 + base_addr + channel, sl_rr);  // D1L/RR
}

// Função auxiliar para configurar um canal FM
static void setup_fm_channel(uint8_t channel, uint16_t freq, uint8_t algorithm,
                             uint8_t feedback) {
  // Configura frequência
  ym2612_write_reg(fm, 0, 0xA4 + channel, (freq >> 8) & 0xFF); // Frequência MSB
  ym2612_write_reg(fm, 0, 0xA0 + channel, freq & 0xFF);        // Frequência LSB

  // Configura algoritmo e feedback
  ym2612_write_reg(fm, 0, 0xB0 + channel, (feedback << 3) | algorithm);

  // Configura pan L/R
  ym2612_write_reg(fm, 0, 0xB4 + channel, 0xC0); // Centro
}

void test_basic_audio_generation(void) {
  audio_profiler_start(profiler);

  // Configura um tom simples no PSG
  setup_psg_tone(0, 100, 0x0F); // ~440Hz, volume máximo

  // Configura um tom simples no FM
  setup_fm_operator(0, 0, 0x71, 0x23, 0x1F, 0x1B, 0x13, 0x0F); // Operador 1
  setup_fm_channel(0, 0x2247, 0, 0);   // ~440Hz, algoritmo 0, sem feedback
  ym2612_write_reg(fm, 0, 0x28, 0xF0); // Key on

  // Processa amostras
  uint32_t total_samples = 0;
  uint32_t underruns = 0;
  uint32_t overruns = 0;

  while (total_samples < SAMPLES_TO_PROCESS) {
    audio_profiler_start_section(profiler, "mixing");

    // Processa um bloco de amostras
    audio_mixer_process(mixer, BUFFER_SIZE);

    // Lê amostras do mixer
    uint32_t samples_read = audio_mixer_read(mixer, sample_buffer, BUFFER_SIZE);

    audio_profiler_end_section(profiler, "mixing");

    // Atualiza métricas
    total_samples += samples_read;

    // Verifica qualidade do áudio
    for (uint32_t i = 0; i < samples_read * 2; i += 2) {
      // Verifica se as amostras estão dentro dos limites
      TEST_ASSERT_LESS_OR_EQUAL(32767, sample_buffer[i]);
      TEST_ASSERT_GREATER_OR_EQUAL(-32768, sample_buffer[i]);
      TEST_ASSERT_LESS_OR_EQUAL(32767, sample_buffer[i + 1]);
      TEST_ASSERT_GREATER_OR_EQUAL(-32768, sample_buffer[i + 1]);
    }

    // Atualiza métricas do buffer
    uint32_t available = audio_mixer_available_samples(mixer);
    if (available == 0)
      underruns++;
    if (available == BUFFER_SIZE)
      overruns++;

    audio_profiler_update_buffer(profiler, available, BUFFER_SIZE);
    audio_profiler_update_samples(profiler, samples_read, 0);
  }

  audio_profiler_stop(profiler);

  // Verifica métricas de qualidade
  TEST_ASSERT_LESS_THAN(5, underruns); // Máximo de 5 underruns permitidos
  TEST_ASSERT_LESS_THAN(5, overruns);  // Máximo de 5 overruns permitidos

  // Gera relatório
  audio_profiler_generate_report(profiler, "audio_test_report.txt");
}

void test_stress_audio_system(void) {
  audio_profiler_start(profiler);

  // Configura múltiplos canais PSG
  setup_psg_tone(0, 100, 0x0F); // ~440Hz
  setup_psg_tone(1, 150, 0x0A); // ~660Hz
  setup_psg_tone(2, 200, 0x08); // ~880Hz

  // Configura ruído PSG
  psg_write(psg, 0xE0 | 0x04); // White noise
  psg_write(psg, 0xF0 | 0x08); // Volume médio

  // Configura múltiplos canais FM
  for (int i = 0; i < 6; i++) {
    // Configura operadores
    for (int op = 0; op < 4; op++) {
      setup_fm_operator(i, op, 0x71, 0x23, 0x1F, 0x1B, 0x13, 0x0F);
    }

    // Configura canal com frequências diferentes
    setup_fm_channel(i, 0x2247 + (i * 100), i % 8, i % 4);

    // Key on
    ym2612_write_reg(fm, 0, 0x28, 0xF0 | i);
  }

  // Processa amostras com carga máxima
  uint32_t total_samples = 0;
  uint32_t underruns = 0;
  uint32_t overruns = 0;

  while (total_samples < SAMPLES_TO_PROCESS) {
    audio_profiler_start_section(profiler, "mixing");

    audio_mixer_process(mixer, BUFFER_SIZE);
    uint32_t samples_read = audio_mixer_read(mixer, sample_buffer, BUFFER_SIZE);

    audio_profiler_end_section(profiler, "mixing");

    total_samples += samples_read;

    // Verifica qualidade do áudio
    for (uint32_t i = 0; i < samples_read * 2; i += 2) {
      TEST_ASSERT_LESS_OR_EQUAL(32767, sample_buffer[i]);
      TEST_ASSERT_GREATER_OR_EQUAL(-32768, sample_buffer[i]);
      TEST_ASSERT_LESS_OR_EQUAL(32767, sample_buffer[i + 1]);
      TEST_ASSERT_GREATER_OR_EQUAL(-32768, sample_buffer[i + 1]);
    }

    // Atualiza métricas do buffer
    uint32_t available = audio_mixer_available_samples(mixer);
    if (available == 0)
      underruns++;
    if (available == BUFFER_SIZE)
      overruns++;

    audio_profiler_update_buffer(profiler, available, BUFFER_SIZE);
    audio_profiler_update_samples(profiler, samples_read, 0);
  }

  audio_profiler_stop(profiler);

  // Verifica métricas de qualidade sob stress
  TEST_ASSERT_LESS_THAN(10, underruns); // Tolerância maior para teste de stress
  TEST_ASSERT_LESS_THAN(10, overruns);

  // Verifica uso de CPU
  const audio_profiler_metrics_t *metrics =
      audio_profiler_get_metrics(profiler);
  TEST_ASSERT_LESS_THAN(80.0, metrics->total_cpu_usage); // Máximo de 80% de CPU

  // Gera relatório de stress
  audio_profiler_generate_report(profiler, "audio_stress_report.txt");
}

void test_volume_control(void) {
  // Testa controle de volume do mixer
  audio_mixer_set_fm_volume(mixer, 0.0f);
  audio_mixer_set_psg_volume(mixer, 1.0f);

  // Configura tom PSG
  setup_psg_tone(0, 100, 0x0F);

  // Processa algumas amostras
  audio_mixer_process(mixer, BUFFER_SIZE);
  uint32_t samples_read = audio_mixer_read(mixer, sample_buffer, BUFFER_SIZE);

  // Verifica se só há áudio do PSG
  float max_sample = 0.0f;
  for (uint32_t i = 0; i < samples_read * 2; i++) {
    max_sample = fmaxf(max_sample, fabsf((float)sample_buffer[i]));
  }
  TEST_ASSERT_GREATER_THAN(0.0f, max_sample);

  // Inverte volumes
  audio_mixer_set_fm_volume(mixer, 1.0f);
  audio_mixer_set_psg_volume(mixer, 0.0f);

  // Configura tom FM
  setup_fm_operator(0, 0, 0x71, 0x23, 0x1F, 0x1B, 0x13, 0x0F);
  setup_fm_channel(0, 0x2247, 0, 0);
  ym2612_write_reg(fm, 0, 0x28, 0xF0);

  // Processa algumas amostras
  audio_mixer_process(mixer, BUFFER_SIZE);
  samples_read = audio_mixer_read(mixer, sample_buffer, BUFFER_SIZE);

  // Verifica se só há áudio do FM
  max_sample = 0.0f;
  for (uint32_t i = 0; i < samples_read * 2; i++) {
    max_sample = fmaxf(max_sample, fabsf((float)sample_buffer[i]));
  }
  TEST_ASSERT_GREATER_THAN(0.0f, max_sample);

  // Testa volume master
  audio_mixer_set_master_volume(mixer, 0.0f);

  audio_mixer_process(mixer, BUFFER_SIZE);
  samples_read = audio_mixer_read(mixer, sample_buffer, BUFFER_SIZE);

  // Verifica se não há áudio
  for (uint32_t i = 0; i < samples_read * 2; i++) {
    TEST_ASSERT_EQUAL(0, sample_buffer[i]);
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_basic_audio_generation);
  RUN_TEST(test_stress_audio_system);
  RUN_TEST(test_volume_control);

  return UNITY_END();
}
