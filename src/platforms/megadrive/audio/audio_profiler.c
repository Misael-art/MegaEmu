/**
 * @file audio_profiler.c
 * @brief Implementação do profiler para análise de desempenho do sistema de
 * áudio
 * @version 1.0
 * @date 2024-03-21
 */

#include "audio_profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura para armazenar tempos de seção
typedef struct {
  clock_t start;
  double total;
} section_timing_t;

// Estrutura interna do profiler com dados adicionais
typedef struct {
  audio_profiler_t base;
  section_timing_t fm_timing;
  section_timing_t psg_timing;
  section_timing_t mixing_timing;
  uint32_t sample_count;
  uint32_t report_interval;
  FILE *log_file;
} audio_profiler_internal_t;

// Funções auxiliares
static void reset_section_timing(section_timing_t *timing) {
  timing->start = 0;
  timing->total = 0.0;
}

static double calculate_cpu_usage(double section_time, double total_time) {
  return total_time > 0.0 ? (section_time / total_time) * 100.0 : 0.0;
}

// Implementação das funções públicas
audio_profiler_t *audio_profiler_create(void) {
  audio_profiler_internal_t *profiler =
      calloc(1, sizeof(audio_profiler_internal_t));
  if (!profiler) {
    return NULL;
  }

  profiler->base.is_profiling = 0;
  profiler->report_interval = 1000; // Intervalo padrão de relatório (amostras)
  audio_profiler_reset((audio_profiler_t *)profiler);

  return (audio_profiler_t *)profiler;
}

void audio_profiler_destroy(audio_profiler_t *profiler) {
  if (profiler) {
    audio_profiler_internal_t *internal = (audio_profiler_internal_t *)profiler;
    if (internal->log_file) {
      fclose(internal->log_file);
    }
    free(profiler);
  }
}

void audio_profiler_reset(audio_profiler_t *profiler) {
  if (!profiler)
    return;

  audio_profiler_internal_t *internal = (audio_profiler_internal_t *)profiler;
  memset(&internal->base.metrics, 0, sizeof(audio_profiler_metrics_t));
  reset_section_timing(&internal->fm_timing);
  reset_section_timing(&internal->psg_timing);
  reset_section_timing(&internal->mixing_timing);
  internal->sample_count = 0;
}

void audio_profiler_start(audio_profiler_t *profiler) {
  if (!profiler)
    return;

  profiler->is_profiling = 1;
  profiler->start_time = clock();
}

void audio_profiler_stop(audio_profiler_t *profiler) {
  if (!profiler || !profiler->is_profiling)
    return;

  audio_profiler_internal_t *internal = (audio_profiler_internal_t *)profiler;
  clock_t end_time = clock();
  double total_time =
      (double)(end_time - profiler->start_time) / CLOCKS_PER_SEC;

  // Atualiza métricas de CPU
  internal->base.metrics.total_time = total_time;
  internal->base.metrics.fm_cpu_usage =
      calculate_cpu_usage(internal->fm_timing.total, total_time);
  internal->base.metrics.psg_cpu_usage =
      calculate_cpu_usage(internal->psg_timing.total, total_time);
  internal->base.metrics.mixing_cpu_usage =
      calculate_cpu_usage(internal->mixing_timing.total, total_time);
  internal->base.metrics.total_cpu_usage =
      internal->base.metrics.fm_cpu_usage +
      internal->base.metrics.psg_cpu_usage +
      internal->base.metrics.mixing_cpu_usage;

  profiler->is_profiling = 0;
}

int audio_profiler_is_active(audio_profiler_t *profiler) {
  return profiler ? profiler->is_profiling : 0;
}

void audio_profiler_start_section(audio_profiler_t *profiler,
                                  const char *section) {
  if (!profiler || !profiler->is_profiling || !section)
    return;

  audio_profiler_internal_t *internal = (audio_profiler_internal_t *)profiler;
  section_timing_t *timing = NULL;

  if (strcmp(section, "fm") == 0) {
    timing = &internal->fm_timing;
  } else if (strcmp(section, "psg") == 0) {
    timing = &internal->psg_timing;
  } else if (strcmp(section, "mixing") == 0) {
    timing = &internal->mixing_timing;
  }

  if (timing) {
    timing->start = clock();
  }
}

void audio_profiler_end_section(audio_profiler_t *profiler,
                                const char *section) {
  if (!profiler || !profiler->is_profiling || !section)
    return;

  audio_profiler_internal_t *internal = (audio_profiler_internal_t *)profiler;
  section_timing_t *timing = NULL;
  double *time_metric = NULL;

  if (strcmp(section, "fm") == 0) {
    timing = &internal->fm_timing;
    time_metric = &internal->base.metrics.fm_processing_time;
  } else if (strcmp(section, "psg") == 0) {
    timing = &internal->psg_timing;
    time_metric = &internal->base.metrics.psg_processing_time;
  } else if (strcmp(section, "mixing") == 0) {
    timing = &internal->mixing_timing;
    time_metric = &internal->base.metrics.mixing_time;
  }

  if (timing && timing->start && time_metric) {
    clock_t end = clock();
    double duration = (double)(end - timing->start) / CLOCKS_PER_SEC;
    timing->total += duration;
    *time_metric = timing->total;
  }
}

void audio_profiler_update_memory(audio_profiler_t *profiler,
                                  const char *section, size_t bytes) {
  if (!profiler || !section)
    return;

  if (strcmp(section, "fm") == 0) {
    profiler->metrics.fm_memory_usage = bytes;
  } else if (strcmp(section, "psg") == 0) {
    profiler->metrics.psg_memory_usage = bytes;
  } else if (strcmp(section, "mixing") == 0) {
    profiler->metrics.mixing_memory_usage = bytes;
  }

  profiler->metrics.total_memory_usage = profiler->metrics.fm_memory_usage +
                                         profiler->metrics.psg_memory_usage +
                                         profiler->metrics.mixing_memory_usage;
}

void audio_profiler_update_buffer(audio_profiler_t *profiler,
                                  uint32_t available, uint32_t total) {
  if (!profiler)
    return;

  float usage = total > 0 ? ((float)available / total) * 100.0f : 0.0f;
  profiler->metrics.buffer_usage = usage;

  if (available == 0) {
    profiler->metrics.buffer_underruns++;
  } else if (available == total) {
    profiler->metrics.buffer_overruns++;
  }
}

void audio_profiler_update_samples(audio_profiler_t *profiler,
                                   uint32_t processed, uint32_t dropped) {
  if (!profiler)
    return;

  profiler->metrics.samples_processed += processed;
  profiler->metrics.samples_dropped += dropped;

  // Calcula taxa de amostragem efetiva
  if (profiler->metrics.total_time > 0.0) {
    profiler->metrics.sample_rate = (float)profiler->metrics.samples_processed /
                                    (float)profiler->metrics.total_time;
  }
}

void audio_profiler_generate_report(audio_profiler_t *profiler,
                                    const char *filename) {
  if (!profiler || !filename)
    return;

  FILE *file = fopen(filename, "w");
  if (!file)
    return;

  fprintf(file, "Relatório de Desempenho do Sistema de Áudio\n");
  fprintf(file, "========================================\n\n");

  // Métricas de tempo
  fprintf(file, "Métricas de Tempo:\n");
  fprintf(file, "  FM Processing: %.3f ms\n",
          profiler->metrics.fm_processing_time * 1000.0);
  fprintf(file, "  PSG Processing: %.3f ms\n",
          profiler->metrics.psg_processing_time * 1000.0);
  fprintf(file, "  Mixing: %.3f ms\n", profiler->metrics.mixing_time * 1000.0);
  fprintf(file, "  Total: %.3f ms\n\n", profiler->metrics.total_time * 1000.0);

  // Métricas de CPU
  fprintf(file, "Uso de CPU:\n");
  fprintf(file, "  FM: %.2f%%\n", profiler->metrics.fm_cpu_usage);
  fprintf(file, "  PSG: %.2f%%\n", profiler->metrics.psg_cpu_usage);
  fprintf(file, "  Mixing: %.2f%%\n", profiler->metrics.mixing_cpu_usage);
  fprintf(file, "  Total: %.2f%%\n\n", profiler->metrics.total_cpu_usage);

  // Métricas de memória
  fprintf(file, "Uso de Memória:\n");
  fprintf(file, "  FM: %zu bytes\n", profiler->metrics.fm_memory_usage);
  fprintf(file, "  PSG: %zu bytes\n", profiler->metrics.psg_memory_usage);
  fprintf(file, "  Mixing: %zu bytes\n", profiler->metrics.mixing_memory_usage);
  fprintf(file, "  Total: %zu bytes\n\n", profiler->metrics.total_memory_usage);

  // Métricas de buffer
  fprintf(file, "Estado do Buffer:\n");
  fprintf(file, "  Uso Médio: %.2f%%\n", profiler->metrics.buffer_usage);
  fprintf(file, "  Underruns: %u\n", profiler->metrics.buffer_underruns);
  fprintf(file, "  Overruns: %u\n\n", profiler->metrics.buffer_overruns);

  // Métricas de amostras
  fprintf(file, "Processamento de Amostras:\n");
  fprintf(file, "  Processadas: %u\n", profiler->metrics.samples_processed);
  fprintf(file, "  Descartadas: %u\n", profiler->metrics.samples_dropped);
  fprintf(file, "  Taxa Efetiva: %.2f Hz\n", profiler->metrics.sample_rate);

  fclose(file);
}

void audio_profiler_print_metrics(audio_profiler_t *profiler) {
  if (!profiler)
    return;

  printf("\nMétricas do Sistema de Áudio:\n");
  printf("==========================\n");

  // Métricas de CPU
  printf("CPU: FM=%.1f%% PSG=%.1f%% Mix=%.1f%% Total=%.1f%%\n",
         profiler->metrics.fm_cpu_usage, profiler->metrics.psg_cpu_usage,
         profiler->metrics.mixing_cpu_usage, profiler->metrics.total_cpu_usage);

  // Métricas de buffer
  printf("Buffer: Uso=%.1f%% Under=%u Over=%u\n",
         profiler->metrics.buffer_usage, profiler->metrics.buffer_underruns,
         profiler->metrics.buffer_overruns);

  // Métricas de amostras
  printf("Amostras: Proc=%u Drop=%u Taxa=%.1f Hz\n",
         profiler->metrics.samples_processed, profiler->metrics.samples_dropped,
         profiler->metrics.sample_rate);
}

const audio_profiler_metrics_t *
audio_profiler_get_metrics(audio_profiler_t *profiler) {
  return profiler ? &profiler->metrics : NULL;
}
