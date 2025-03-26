/**
 * @file memory_cache.c
 * @brief Implementação do sistema de cache adaptativo para otimização de acesso à memória
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "memory_cache.h"
#include "../../utils/error_handling.h"

// Contador global de ticks para LRU
static uint32_t g_cache_clock = 0;

/**
 * @brief Incrementa o relógio do cache
 * @return Valor atual do relógio
 */
static inline uint32_t _cache_tick(void)
{
    return ++g_cache_clock;
}

emu_memory_cache_t *emu_memory_cache_create(uint32_t max_entries)
{
    if (max_entries == 0)
    {
        return NULL;
    }

    emu_memory_cache_t *cache = (emu_memory_cache_t *)malloc(sizeof(emu_memory_cache_t));
    if (!cache)
    {
        return NULL;
    }

    cache->entries = (emu_memory_cache_entry_t *)calloc(max_entries, sizeof(emu_memory_cache_entry_t));
    if (!cache->entries)
    {
        free(cache);
        return NULL;
    }

    cache->num_entries = 0;
    cache->max_entries = max_entries;
    cache->hit_count = 0;
    cache->miss_count = 0;

    return cache;
}

void emu_memory_cache_destroy(emu_memory_cache_t *cache)
{
    if (!cache)
    {
        return;
    }

    // Liberar memória de cada entrada
    for (uint32_t i = 0; i < cache->num_entries; i++)
    {
        free(cache->entries[i].data);
        cache->entries[i].data = NULL;
    }

    free(cache->entries);
    free(cache);
}

/**
 * @brief Encontra uma entrada no cache
 * @param cache Ponteiro para o cache
 * @param address Endereço a ser buscado
 * @param size Tamanho dos dados em bytes
 * @return Índice da entrada encontrada ou -1 se não encontrado
 */
static int32_t _find_cache_entry(emu_memory_cache_t *cache, uint32_t address, uint32_t size)
{
    for (uint32_t i = 0; i < cache->num_entries; i++)
    {
        emu_memory_cache_entry_t *entry = &cache->entries[i];

        // Verifica se o intervalo solicitado está contido na entrada
        if (address >= entry->address &&
            address + size <= entry->address + entry->size)
        {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Encontra a entrada menos recentemente usada
 * @param cache Ponteiro para o cache
 * @return Índice da entrada LRU
 */
static uint32_t _find_lru_entry(emu_memory_cache_t *cache)
{
    uint32_t lru_index = 0;
    uint32_t lru_time = cache->entries[0].last_access;

    for (uint32_t i = 1; i < cache->num_entries; i++)
    {
        if (cache->entries[i].last_access < lru_time)
        {
            lru_time = cache->entries[i].last_access;
            lru_index = i;
        }
    }

    return lru_index;
}

bool emu_memory_cache_get(emu_memory_cache_t *cache, uint32_t address, uint32_t size, uint8_t *data)
{
    if (!cache || !data || size == 0)
    {
        return false;
    }

    int32_t index = _find_cache_entry(cache, address, size);
    if (index < 0)
    {
        cache->miss_count++;
        return false;
    }

    // Cache hit - atualizar estatísticas e copiar dados
    emu_memory_cache_entry_t *entry = &cache->entries[index];
    entry->access_count++;
    entry->last_access = _cache_tick();

    // Copiar dados para o buffer de saída
    uint32_t offset = address - entry->address;
    memcpy(data, entry->data + offset, size);

    cache->hit_count++;
    return true;
}

bool emu_memory_cache_put(emu_memory_cache_t *cache, uint32_t address, uint32_t size, const uint8_t *data)
{
    if (!cache || !data || size == 0)
    {
        return false;
    }

    // Verificar se já existe uma entrada que contém este endereço
    int32_t index = _find_cache_entry(cache, address, size);
    if (index >= 0)
    {
        // Atualizar entrada existente
        emu_memory_cache_entry_t *entry = &cache->entries[index];
        uint32_t offset = address - entry->address;
        memcpy(entry->data + offset, data, size);
        entry->access_count++;
        entry->last_access = _cache_tick();
        return true;
    }

    // Precisamos criar uma nova entrada
    uint8_t *new_data = (uint8_t *)malloc(size);
    if (!new_data)
    {
        return false;
    }

    memcpy(new_data, data, size);

    // Se o cache estiver cheio, substituir a entrada LRU
    if (cache->num_entries >= cache->max_entries)
    {
        uint32_t lru_index = _find_lru_entry(cache);
        free(cache->entries[lru_index].data);

        cache->entries[lru_index].address = address;
        cache->entries[lru_index].size = size;
        cache->entries[lru_index].data = new_data;
        cache->entries[lru_index].access_count = 1;
        cache->entries[lru_index].last_access = _cache_tick();
    }
    else
    {
        // Adicionar nova entrada
        cache->entries[cache->num_entries].address = address;
        cache->entries[cache->num_entries].size = size;
        cache->entries[cache->num_entries].data = new_data;
        cache->entries[cache->num_entries].access_count = 1;
        cache->entries[cache->num_entries].last_access = _cache_tick();
        cache->num_entries++;
    }

    return true;
}

void emu_memory_cache_clear(emu_memory_cache_t *cache)
{
    if (!cache)
    {
        return;
    }

    // Liberar memória de cada entrada
    for (uint32_t i = 0; i < cache->num_entries; i++)
    {
        free(cache->entries[i].data);
        cache->entries[i].data = NULL;
    }

    cache->num_entries = 0;
}

void emu_memory_cache_stats(emu_memory_cache_t *cache, uint32_t *hits, uint32_t *misses, float *hit_ratio)
{
    if (!cache)
    {
        if (hits)
            *hits = 0;
        if (misses)
            *misses = 0;
        if (hit_ratio)
            *hit_ratio = 0.0f;
        return;
    }

    if (hits)
        *hits = cache->hit_count;
    if (misses)
        *misses = cache->miss_count;

    if (hit_ratio)
    {
        uint32_t total = cache->hit_count + cache->miss_count;
        *hit_ratio = (total > 0) ? ((float)cache->hit_count / total) : 0.0f;
    }
}

// Mapa de alocações para rastreamento de vazamentos
typedef struct
{
    uint32_t address;
    uint32_t size;
    bool is_allocated;
    char description[64];
} memory_allocation_t;

#define MAX_TRACKED_ALLOCATIONS 1024
static memory_allocation_t g_memory_allocations[MAX_TRACKED_ALLOCATIONS];
static uint32_t g_num_allocations = 0;
static uint32_t g_total_allocated = 0;
static uint32_t g_peak_allocated = 0;

/**
 * @brief Registra uma alocação de memória
 * @param address Endereço da alocação
 * @param size Tamanho em bytes
 * @param description Descrição da alocação
 * @return true se registrado com sucesso, false caso contrário
 */
static bool _register_allocation(uint32_t address, uint32_t size, const char *description)
{
    if (g_num_allocations >= MAX_TRACKED_ALLOCATIONS)
    {
        return false;
    }

    g_memory_allocations[g_num_allocations].address = address;
    g_memory_allocations[g_num_allocations].size = size;
    g_memory_allocations[g_num_allocations].is_allocated = true;

    if (description)
    {
        strncpy(g_memory_allocations[g_num_allocations].description, description, 63);
        g_memory_allocations[g_num_allocations].description[63] = '\0';
    }
    else
    {
        strcpy(g_memory_allocations[g_num_allocations].description, "Unknown");
    }

    g_num_allocations++;
    g_total_allocated += size;

    if (g_total_allocated > g_peak_allocated)
    {
        g_peak_allocated = g_total_allocated;
    }

    return true;
}

/**
 * @brief Registra uma desalocação de memória
 * @param address Endereço da desalocação
 * @return true se encontrado e marcado, false caso contrário
 */
static bool _register_deallocation(uint32_t address)
{
    for (uint32_t i = 0; i < g_num_allocations; i++)
    {
        if (g_memory_allocations[i].address == address && g_memory_allocations[i].is_allocated)
        {
            g_memory_allocations[i].is_allocated = false;
            g_total_allocated -= g_memory_allocations[i].size;
            return true;
        }
    }

    return false;
}

/**
 * @brief Calcula a fragmentação da memória
 * @return Porcentagem de fragmentação (0-100)
 */
static uint32_t _calculate_fragmentation(void)
{
    if (g_num_allocations == 0)
    {
        return 0;
    }

    // Ordenar alocações por endereço
    for (uint32_t i = 0; i < g_num_allocations - 1; i++)
    {
        for (uint32_t j = 0; j < g_num_allocations - i - 1; j++)
        {
            if (g_memory_allocations[j].address > g_memory_allocations[j + 1].address)
            {
                memory_allocation_t temp = g_memory_allocations[j];
                g_memory_allocations[j] = g_memory_allocations[j + 1];
                g_memory_allocations[j + 1] = temp;
            }
        }
    }

    // Calcular espaços vazios entre alocações
    uint32_t total_gaps = 0;
    uint32_t total_space = 0;

    for (uint32_t i = 0; i < g_num_allocations - 1; i++)
    {
        if (g_memory_allocations[i].is_allocated && g_memory_allocations[i + 1].is_allocated)
        {
            uint32_t end_addr = g_memory_allocations[i].address + g_memory_allocations[i].size;
            uint32_t gap = 0;

            if (g_memory_allocations[i + 1].address > end_addr)
            {
                gap = g_memory_allocations[i + 1].address - end_addr;
                total_gaps += gap;
            }

            total_space += g_memory_allocations[i].size + gap;
        }
    }

    // Adicionar última alocação
    if (g_num_allocations > 0 && g_memory_allocations[g_num_allocations - 1].is_allocated)
    {
        total_space += g_memory_allocations[g_num_allocations - 1].size;
    }

    if (total_space == 0)
    {
        return 0;
    }

    return (total_gaps * 100) / total_space;
}

bool emu_memory_analyze(emu_memory_t memory, emu_memory_analysis_t *analysis)
{
    if (!memory || !analysis)
    {
        return false;
    }

    // Contagem de vazamentos
    uint32_t leaks_count = 0;
    for (uint32_t i = 0; i < g_num_allocations; i++)
    {
        if (g_memory_allocations[i].is_allocated)
        {
            leaks_count++;
        }
    }

    analysis->leaks_detected = leaks_count;
    analysis->total_allocated = g_total_allocated;
    analysis->peak_allocated = g_peak_allocated;
    analysis->fragmentation_percent = _calculate_fragmentation();

    return true;
}

bool emu_memory_leak_check(emu_memory_t memory)
{
    if (!memory)
    {
        return false;
    }

    // Verificar vazamentos
    uint32_t leaks_count = 0;
    for (uint32_t i = 0; i < g_num_allocations; i++)
    {
        if (g_memory_allocations[i].is_allocated)
        {
            leaks_count++;
        }
    }

    return (leaks_count == 0);
}

void emu_memory_report(emu_memory_t memory, const char *filename)
{
    if (!memory || !filename)
    {
        return;
    }

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        return;
    }

    fprintf(file, "Mega_Emu Memory Report\n");
    fprintf(file, "=====================\n\n");

    fprintf(file, "Summary:\n");
    fprintf(file, "  Total allocations tracked: %u\n", g_num_allocations);
    fprintf(file, "  Current memory usage: %u bytes\n", g_total_allocated);
    fprintf(file, "  Peak memory usage: %u bytes\n", g_peak_allocated);
    fprintf(file, "  Fragmentation: %u%%\n\n", _calculate_fragmentation());

    fprintf(file, "Memory Regions:\n");
    for (int32_t i = 0; i < memory->num_regions; i++)
    {
        memory_region_t *region = &memory->regions[i];
        fprintf(file, "  Region %d: 0x%08X - 0x%08X (%u bytes)\n",
                i, region->start, region->start + region->size - 1, region->size);
    }

    fprintf(file, "\nAllocation Details:\n");
    for (uint32_t i = 0; i < g_num_allocations; i++)
    {
        fprintf(file, "  [%s] 0x%08X: %u bytes - %s\n",
                g_memory_allocations[i].is_allocated ? "ACTIVE" : "FREED",
                g_memory_allocations[i].address,
                g_memory_allocations[i].size,
                g_memory_allocations[i].description);
    }

    // Detectar vazamentos
    uint32_t leaks_count = 0;
    uint32_t leaked_bytes = 0;
    for (uint32_t i = 0; i < g_num_allocations; i++)
    {
        if (g_memory_allocations[i].is_allocated)
        {
            leaks_count++;
            leaked_bytes += g_memory_allocations[i].size;
        }
    }

    fprintf(file, "\nLeak Analysis:\n");
    if (leaks_count > 0)
    {
        fprintf(file, "  WARNING: %u leaks detected, total %u bytes!\n", leaks_count, leaked_bytes);
        fprintf(file, "  Leaked allocations:\n");
        for (uint32_t i = 0; i < g_num_allocations; i++)
        {
            if (g_memory_allocations[i].is_allocated)
            {
                fprintf(file, "    0x%08X: %u bytes - %s\n",
                        g_memory_allocations[i].address,
                        g_memory_allocations[i].size,
                        g_memory_allocations[i].description);
            }
        }
    }
    else
    {
        fprintf(file, "  No memory leaks detected.\n");
    }

    fclose(file);
}

// Funções de hook para substituir as operações padrão de memória

/**
 * Estas funções podem ser usadas para substituir as funções de
 * alocação/liberação de memória no sistema de memória principal
 * para ativar o rastreamento de alocações.
 */

void *emu_memory_tracked_alloc(uint32_t size, const char *description)
{
    void *ptr = malloc(size);
    if (ptr && description)
    {
        _register_allocation((uint32_t)(uintptr_t)ptr, size, description);
    }
    return ptr;
}

void emu_memory_tracked_free(void *ptr)
{
    if (ptr)
    {
        _register_deallocation((uint32_t)(uintptr_t)ptr);
        free(ptr);
    }
}
