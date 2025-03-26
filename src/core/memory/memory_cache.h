/**
 * @file memory_cache.h
 * @brief Sistema de cache adaptativo para otimização de acesso à memória
 */

#ifndef MEMORY_CACHE_H
#define MEMORY_CACHE_H

#include "memory_interface.h"

/**
 * @brief Entrada do cache de memória
 */
typedef struct
{
    uint32_t address;      /**< Endereço base da entrada */
    uint32_t size;         /**< Tamanho dos dados em bytes */
    uint8_t *data;         /**< Dados em cache */
    uint32_t access_count; /**< Contador de acessos */
    uint32_t last_access;  /**< Timestamp do último acesso */
} emu_memory_cache_entry_t;

/**
 * @brief Cache de memória adaptativo
 */
typedef struct
{
    emu_memory_cache_entry_t *entries; /**< Entradas do cache */
    uint32_t num_entries;              /**< Número atual de entradas */
    uint32_t max_entries;              /**< Número máximo de entradas */
    uint32_t hit_count;                /**< Contador de acertos */
    uint32_t miss_count;               /**< Contador de falhas */
} emu_memory_cache_t;

/**
 * @brief Cria um novo cache de memória
 * @param max_entries Número máximo de entradas
 * @return Ponteiro para o cache ou NULL em caso de erro
 */
emu_memory_cache_t *emu_memory_cache_create(uint32_t max_entries);

/**
 * @brief Destrói um cache de memória
 * @param cache Ponteiro para o cache
 */
void emu_memory_cache_destroy(emu_memory_cache_t *cache);

/**
 * @brief Obtém dados do cache
 * @param cache Ponteiro para o cache
 * @param address Endereço a ser buscado
 * @param size Tamanho dos dados em bytes
 * @param data Buffer para receber os dados
 * @return true se os dados foram encontrados, false caso contrário
 */
bool emu_memory_cache_get(emu_memory_cache_t *cache, uint32_t address, uint32_t size, uint8_t *data);

/**
 * @brief Adiciona dados ao cache
 * @param cache Ponteiro para o cache
 * @param address Endereço dos dados
 * @param size Tamanho dos dados em bytes
 * @param data Dados a serem armazenados
 * @return true se os dados foram adicionados com sucesso, false caso contrário
 */
bool emu_memory_cache_put(emu_memory_cache_t *cache, uint32_t address, uint32_t size, const uint8_t *data);

/**
 * @brief Limpa o cache
 * @param cache Ponteiro para o cache
 */
void emu_memory_cache_clear(emu_memory_cache_t *cache);

/**
 * @brief Obtém estatísticas do cache
 * @param cache Ponteiro para o cache
 * @param hits Ponteiro para variável que receberá o número de acertos
 * @param misses Ponteiro para variável que receberá o número de falhas
 * @param hit_ratio Ponteiro para variável que receberá a taxa de acertos
 */
void emu_memory_cache_stats(emu_memory_cache_t *cache, uint32_t *hits, uint32_t *misses, float *hit_ratio);

/**
 * @brief Estrutura para análise de uso de memória
 */
typedef struct
{
    uint32_t leaks_detected;        /**< Número de vazamentos detectados */
    uint32_t total_allocated;       /**< Total de memória alocada */
    uint32_t peak_allocated;        /**< Pico de memória alocada */
    uint32_t fragmentation_percent; /**< Porcentagem de fragmentação */
} emu_memory_analysis_t;

/**
 * @brief Analisa o estado da memória
 * @param memory Instância de memória
 * @param analysis Estrutura para armazenar os resultados da análise
 * @return true se a análise foi concluída com sucesso, false caso contrário
 */
bool emu_memory_analyze(emu_memory_t memory, emu_memory_analysis_t *analysis);

/**
 * @brief Verifica vazamentos de memória
 * @param memory Instância de memória
 * @return true se não foram encontrados vazamentos, false caso contrário
 */
bool emu_memory_leak_check(emu_memory_t memory);

/**
 * @brief Gera relatório de uso de memória
 * @param memory Instância de memória
 * @param filename Nome do arquivo para o relatório
 */
void emu_memory_report(emu_memory_t memory, const char *filename);

#endif // MEMORY_CACHE_H
