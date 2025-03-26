/**
 * @file test_profiler.c
 * @brief Testes unitários para o profiler do sistema de áudio
 * @version 1.0
 * @date 2024-03-21
 */

#include "audio_profiler.h"
#include <string.h>
#include <unistd.h>
#include <unity.h>

// Variáveis globais para os testes
static audio_profiler_t *profiler;

void setUp(void) {
  profiler = audio_profiler_create();
  TEST_ASSERT_NOT_NULL(profiler);
}

void tearDown(void) {
  if (profiler) {
    audio_profiler_destroy(profiler);
    profiler = NULL;
  }
}

// Função auxiliar para simular processamento
static void simulate_processing(void) {
  usleep(10000); // 10ms
}

void test_profiler_create_destroy(void) {
  TEST_ASSERT_NOT_NULL(profiler);
  TEST_ASSERT_EQUAL(0, profiler->is_profiling);
  TEST_ASSERT_EQUAL(0, profiler->metrics.total_time);
}

void test_profiler_start_stop(void) {
  TEST_ASSERT_EQUAL(0, profiler->is_profiling);

  audio_profiler_start(profiler);
  TEST_ASSERT_EQUAL(1, profiler->is_profiling);

  simulate_processing();

  audio_profiler_stop(profiler);
  TEST_ASSERT_EQUAL(0, profiler->is_profiling);
  TEST_ASSERT_GREATER_THAN(0.0, profiler->metrics.total_time);
}

void test_profiler_section_timing(void) {
  audio_profiler_start(profiler);

  // Testa seção FM
  audio_profiler_start_section(profiler, "fm");
  simulate_processing();
  audio_profiler_end_section(profiler, "fm");
  TEST_ASSERT_GREATER_THAN(0.0, profiler->metrics.fm_processing_time);

  // Testa seção PSG
  audio_profiler_start_section(profiler, "psg");
  simulate_processing();
  audio_profiler_end_section(profiler, "psg");
  TEST_ASSERT_GREATER_THAN(0.0, profiler->metrics.psg_processing_time);

  // Testa seção Mixing
  audio_profiler_start_section(profiler, "mixing");
  simulate_processing();
  audio_profiler_end_section(profiler, "mixing");
  TEST_ASSERT_GREATER_THAN(0.0, profiler->metrics.mixing_time);

  audio_profiler_stop(profiler);

  // Verifica uso de CPU
  TEST_ASSERT_GREATER_THAN(0.0, profiler->metrics.fm_cpu_usage);
  TEST_ASSERT_GREATER_THAN(0.0, profiler->metrics.psg_cpu_usage);
  TEST_ASSERT_GREATER_THAN(0.0, profiler->metrics.mixing_cpu_usage);
  TEST_ASSERT_LESS_OR_EQUAL(100.0, profiler->metrics.total_cpu_usage);
}

void test_profiler_memory_tracking(void) {
  // Testa atualização de memória FM
  audio_profiler_update_memory(profiler, "fm", 1024);
  TEST_ASSERT_EQUAL(1024, profiler->metrics.fm_memory_usage);

  // Testa atualização de memória PSG
  audio_profiler_update_memory(profiler, "psg", 512);
  TEST_ASSERT_EQUAL(512, profiler->metrics.psg_memory_usage);

  // Testa atualização de memória Mixing
  audio_profiler_update_memory(profiler, "mixing", 2048);
  TEST_ASSERT_EQUAL(2048, profiler->metrics.mixing_memory_usage);

  // Verifica memória total
  TEST_ASSERT_EQUAL(3584, profiler->metrics.total_memory_usage);
}

void test_profiler_buffer_metrics(void) {
  // Testa buffer vazio (underrun)
  audio_profiler_update_buffer(profiler, 0, 1000);
  TEST_ASSERT_EQUAL(1, profiler->metrics.buffer_underruns);
  TEST_ASSERT_EQUAL(0.0f, profiler->metrics.buffer_usage);

  // Testa buffer cheio (overrun)
  audio_profiler_update_buffer(profiler, 1000, 1000);
  TEST_ASSERT_EQUAL(1, profiler->metrics.buffer_overruns);
  TEST_ASSERT_EQUAL(100.0f, profiler->metrics.buffer_usage);

  // Testa buffer parcial
  audio_profiler_update_buffer(profiler, 500, 1000);
  TEST_ASSERT_EQUAL(50.0f, profiler->metrics.buffer_usage);
}

void test_profiler_sample_metrics(void) {
  // Inicia profiling
  audio_profiler_start(profiler);
  simulate_processing();

  // Testa processamento de amostras
  audio_profiler_update_samples(profiler, 1000, 10);
  TEST_ASSERT_EQUAL(1000, profiler->metrics.samples_processed);
  TEST_ASSERT_EQUAL(10, profiler->metrics.samples_dropped);

  // Adiciona mais amostras
  audio_profiler_update_samples(profiler, 500, 5);
  TEST_ASSERT_EQUAL(1500, profiler->metrics.samples_processed);
  TEST_ASSERT_EQUAL(15, profiler->metrics.samples_dropped);

  audio_profiler_stop(profiler);

  // Verifica taxa de amostragem
  TEST_ASSERT_GREATER_THAN(0.0f, profiler->metrics.sample_rate);
}

void test_profiler_reset(void) {
  // Configura algumas métricas
  audio_profiler_start(profiler);
  audio_profiler_start_section(profiler, "fm");
  simulate_processing();
  audio_profiler_end_section(profiler, "fm");
  audio_profiler_update_memory(profiler, "fm", 1024);
  audio_profiler_update_buffer(profiler, 500, 1000);
  audio_profiler_update_samples(profiler, 1000, 10);
  audio_profiler_stop(profiler);

  // Reseta o profiler
  audio_profiler_reset(profiler);

  // Verifica se todas as métricas foram zeradas
  TEST_ASSERT_EQUAL(0.0, profiler->metrics.fm_processing_time);
  TEST_ASSERT_EQUAL(0.0, profiler->metrics.psg_processing_time);
  TEST_ASSERT_EQUAL(0.0, profiler->metrics.mixing_time);
  TEST_ASSERT_EQUAL(0.0, profiler->metrics.total_time);
  TEST_ASSERT_EQUAL(0.0, profiler->metrics.fm_cpu_usage);
  TEST_ASSERT_EQUAL(0.0, profiler->metrics.total_cpu_usage);
  TEST_ASSERT_EQUAL(0, profiler->metrics.fm_memory_usage);
  TEST_ASSERT_EQUAL(0, profiler->metrics.total_memory_usage);
  TEST_ASSERT_EQUAL(0, profiler->metrics.buffer_underruns);
  TEST_ASSERT_EQUAL(0, profiler->metrics.buffer_overruns);
  TEST_ASSERT_EQUAL(0.0f, profiler->metrics.buffer_usage);
  TEST_ASSERT_EQUAL(0, profiler->metrics.samples_processed);
  TEST_ASSERT_EQUAL(0, profiler->metrics.samples_dropped);
  TEST_ASSERT_EQUAL(0.0f, profiler->metrics.sample_rate);
}

void test_profiler_report_generation(void) {
  // Configura algumas métricas
  audio_profiler_start(profiler);
  audio_profiler_start_section(profiler, "fm");
  simulate_processing();
  audio_profiler_end_section(profiler, "fm");
  audio_profiler_update_memory(profiler, "fm", 1024);
  audio_profiler_update_buffer(profiler, 500, 1000);
  audio_profiler_update_samples(profiler, 1000, 10);
  audio_profiler_stop(profiler);

  // Gera relatório
  const char *filename = "test_report.txt";
  audio_profiler_generate_report(profiler, filename);

  // Verifica se o arquivo foi criado
  FILE *file = fopen(filename, "r");
  TEST_ASSERT_NOT_NULL(file);
  fclose(file);

  // Remove o arquivo de teste
  remove(filename);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_profiler_create_destroy);
  RUN_TEST(test_profiler_start_stop);
  RUN_TEST(test_profiler_section_timing);
  RUN_TEST(test_profiler_memory_tracking);
  RUN_TEST(test_profiler_buffer_metrics);
  RUN_TEST(test_profiler_sample_metrics);
  RUN_TEST(test_profiler_reset);
  RUN_TEST(test_profiler_report_generation);

  return UNITY_END();
}
