/**
 * @file audio_profiler.h
 * @brief Profiler para análise de desempenho do sistema de áudio
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef AUDIO_PROFILER_H
#define AUDIO_PROFILER_H

#include <stdint.h>
#include <time.h>

/**
 * @brief Estrutura para armazenar métricas de desempenho
 */
typedef struct {
  // Métricas de tempo
  double fm_processing_time;  ///< Tempo gasto no processamento FM
  double psg_processing_time; ///< Tempo gasto no processamento PSG
  double mixing_time;         ///< Tempo gasto no mixing
  double total_time;          ///< Tempo total de processamento

  // Métricas de CPU
  double fm_cpu_usage;     ///< Uso de CPU do FM (%)
  double psg_cpu_usage;    ///< Uso de CPU do PSG (%)
  double mixing_cpu_usage; ///< Uso de CPU do mixing (%)
  double total_cpu_usage;  ///< Uso total de CPU (%)

  // Métricas de memória
  size_t fm_memory_usage;     ///< Uso de memória do FM (bytes)
  size_t psg_memory_usage;    ///< Uso de memória do PSG (bytes)
  size_t mixing_memory_usage; ///< Uso de memória do mixing (bytes)
  size_t total_memory_usage;  ///< Uso total de memória (bytes)

  // Métricas de buffer
  uint32_t buffer_underruns; ///< Número de underruns do buffer
  uint32_t buffer_overruns;  ///< Número de overruns do buffer
  float buffer_usage;        ///< Uso médio do buffer (%)

  // Métricas de amostras
  uint32_t samples_processed; ///< Total de amostras processadas
  uint32_t samples_dropped;   ///< Amostras descartadas
  float sample_rate;          ///< Taxa de amostragem efetiva
} audio_profiler_metrics_t;

/**
 * @brief Estrutura do profiler
 */
typedef struct {
  audio_profiler_metrics_t metrics; ///< Métricas coletadas
  clock_t start_time;               ///< Tempo de início do profiling
  int is_profiling;                 ///< Estado do profiling
} audio_profiler_t;

/**
 * @brief Cria uma nova instância do profiler
 * @return Ponteiro para o profiler ou NULL em caso de erro
 */
audio_profiler_t *audio_profiler_create(void);

/**
 * @brief Destrói uma instância do profiler
 * @param profiler Ponteiro para o profiler
 */
void audio_profiler_destroy(audio_profiler_t *profiler);

/**
 * @brief Reseta todas as métricas do profiler
 * @param profiler Ponteiro para o profiler
 */
void audio_profiler_reset(audio_profiler_t *profiler);

/**
 * @brief Inicia a coleta de métricas
 * @param profiler Ponteiro para o profiler
 */
void audio_profiler_start(audio_profiler_t *profiler);

/**
 * @brief Para a coleta de métricas
 * @param profiler Ponteiro para o profiler
 */
void audio_profiler_stop(audio_profiler_t *profiler);

/**
 * @brief Verifica se o profiling está ativo
 * @param profiler Ponteiro para o profiler
 * @return 1 se ativo, 0 caso contrário
 */
int audio_profiler_is_active(audio_profiler_t *profiler);

/**
 * @brief Inicia a medição de uma seção
 * @param profiler Ponteiro para o profiler
 * @param section Nome da seção ("fm", "psg" ou "mixing")
 */
void audio_profiler_start_section(audio_profiler_t *profiler,
                                  const char *section);

/**
 * @brief Finaliza a medição de uma seção
 * @param profiler Ponteiro para o profiler
 * @param section Nome da seção ("fm", "psg" ou "mixing")
 */
void audio_profiler_end_section(audio_profiler_t *profiler,
                                const char *section);

/**
 * @brief Atualiza métricas de memória
 * @param profiler Ponteiro para o profiler
 * @param section Nome da seção ("fm", "psg" ou "mixing")
 * @param bytes Quantidade de bytes em uso
 */
void audio_profiler_update_memory(audio_profiler_t *profiler,
                                  const char *section, size_t bytes);

/**
 * @brief Atualiza métricas de buffer
 * @param profiler Ponteiro para o profiler
 * @param available Número de amostras disponíveis
 * @param total Capacidade total do buffer
 */
void audio_profiler_update_buffer(audio_profiler_t *profiler,
                                  uint32_t available, uint32_t total);

/**
 * @brief Atualiza métricas de amostras
 * @param profiler Ponteiro para o profiler
 * @param processed Número de amostras processadas
 * @param dropped Número de amostras descartadas
 */
void audio_profiler_update_samples(audio_profiler_t *profiler,
                                   uint32_t processed, uint32_t dropped);

/**
 * @brief Gera um relatório em arquivo
 * @param profiler Ponteiro para o profiler
 * @param filename Nome do arquivo de saída
 */
void audio_profiler_generate_report(audio_profiler_t *profiler,
                                    const char *filename);

/**
 * @brief Imprime as métricas no console
 * @param profiler Ponteiro para o profiler
 */
void audio_profiler_print_metrics(audio_profiler_t *profiler);

/**
 * @brief Obtém as métricas atuais
 * @param profiler Ponteiro para o profiler
 * @return Ponteiro para a estrutura de métricas
 */
const audio_profiler_metrics_t *
audio_profiler_get_metrics(audio_profiler_t *profiler);

#endif // AUDIO_PROFILER_H
