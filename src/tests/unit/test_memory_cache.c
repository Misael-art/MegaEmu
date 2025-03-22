/**
 * @file test_memory_cache.c
 * @brief Testes unitários para o sistema de cache de memória
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../deps/unity/src/unity.h"
#include "../../core/memory/memory_cache.h"
#include "../../core/memory/memory_optimization.h"
#include "../test_common.h"

// Variáveis para teste
static emu_memory_cache_t *g_cache = NULL;
static emu_memory_t g_memory = NULL;
static uint8_t g_test_data[1024];

void setUp(void)
{
    // Inicializar cache para testes
    g_cache = emu_memory_cache_create(16);
    TEST_ASSERT_NOT_NULL(g_cache);

    // Inicializar dados de teste
    for (int i = 0; i < sizeof(g_test_data); i++)
    {
        g_test_data[i] = (uint8_t)i;
    }

    // Inicializar memória para testes
    g_memory = emu_memory_create();
    TEST_ASSERT_NOT_NULL(g_memory);
    TEST_ASSERT_TRUE(emu_memory_init(g_memory));

    // Adicionar uma região de teste
    uint8_t *region_data = (uint8_t *)malloc(1024);
    TEST_ASSERT_NOT_NULL(region_data);
    memcpy(region_data, g_test_data, 1024);

    memory_callbacks_t callbacks = {0};
    TEST_ASSERT_TRUE(emu_memory_add_region(g_memory, 0x1000, 1024, region_data, EMU_MEMORY_RAM, &callbacks));
}

void tearDown(void)
{
    if (g_cache)
    {
        emu_memory_cache_destroy(g_cache);
        g_cache = NULL;
    }

    if (g_memory)
    {
        emu_memory_shutdown(g_memory);
        emu_memory_destroy(g_memory);
        g_memory = NULL;
    }
}

// Testes de Criação/Destruição
void test_cache_create_destroy(void)
{
    emu_memory_cache_t *cache = emu_memory_cache_create(32);
    TEST_ASSERT_NOT_NULL(cache);

    emu_memory_cache_destroy(cache);

    // Teste com valor inválido
    cache = emu_memory_cache_create(0);
    TEST_ASSERT_NULL(cache);
}

// Testes de Operações Básicas
void test_cache_put_get(void)
{
    uint8_t data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint8_t output[16] = {0};

    // Adicionar dados ao cache
    TEST_ASSERT_TRUE(emu_memory_cache_put(g_cache, 0x1000, sizeof(data), data));

    // Verificar se conseguimos recuperar os dados
    TEST_ASSERT_TRUE(emu_memory_cache_get(g_cache, 0x1000, sizeof(data), output));
    TEST_ASSERT_EQUAL_MEMORY(data, output, sizeof(data));

    // Verificar para endereço não cacheado
    memset(output, 0, sizeof(output));
    TEST_ASSERT_FALSE(emu_memory_cache_get(g_cache, 0x2000, sizeof(data), output));

    // Verificar para leitura parcial
    memset(output, 0, sizeof(output));
    TEST_ASSERT_TRUE(emu_memory_cache_get(g_cache, 0x1004, 4, output));
    TEST_ASSERT_EQUAL_MEMORY(&data[4], output, 4);
}

// Teste de Substituição LRU
void test_cache_lru_replacement(void)
{
    // Preencher o cache completo
    for (uint32_t i = 0; i < g_cache->max_entries; i++)
    {
        uint8_t data[8] = {(uint8_t)i, (uint8_t)(i + 1), (uint8_t)(i + 2), (uint8_t)(i + 3),
                           (uint8_t)(i + 4), (uint8_t)(i + 5), (uint8_t)(i + 6), (uint8_t)(i + 7)};

        TEST_ASSERT_TRUE(emu_memory_cache_put(g_cache, 0x1000 + i * 16, sizeof(data), data));
    }

    // Acessar todas as entradas, exceto a primeira, para atualizá-las no LRU
    uint8_t output[8];
    for (uint32_t i = 1; i < g_cache->max_entries; i++)
    {
        TEST_ASSERT_TRUE(emu_memory_cache_get(g_cache, 0x1000 + i * 16, sizeof(output), output));
    }

    // Adicionar uma nova entrada, que deve substituir a primeira (LRU)
    uint8_t new_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    TEST_ASSERT_TRUE(emu_memory_cache_put(g_cache, 0x2000, sizeof(new_data), new_data));

    // Verificar se a primeira entrada foi substituída
    TEST_ASSERT_FALSE(emu_memory_cache_get(g_cache, 0x1000, sizeof(output), output));

    // Verificar se a nova entrada está disponível
    TEST_ASSERT_TRUE(emu_memory_cache_get(g_cache, 0x2000, sizeof(output), output));
    TEST_ASSERT_EQUAL_MEMORY(new_data, output, sizeof(output));
}

// Teste de Estatísticas
void test_cache_statistics(void)
{
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t output[8];

    uint32_t hits, misses;
    float hit_ratio;

    // Verificar estatísticas iniciais
    emu_memory_cache_stats(g_cache, &hits, &misses, &hit_ratio);
    TEST_ASSERT_EQUAL_UINT32(0, hits);
    TEST_ASSERT_EQUAL_UINT32(0, misses);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, hit_ratio);

    // Adicionar uma entrada e acessá-la
    TEST_ASSERT_TRUE(emu_memory_cache_put(g_cache, 0x1000, sizeof(data), data));
    TEST_ASSERT_TRUE(emu_memory_cache_get(g_cache, 0x1000, sizeof(output), output));

    // Verificar estatísticas após hit
    emu_memory_cache_stats(g_cache, &hits, &misses, &hit_ratio);
    TEST_ASSERT_EQUAL_UINT32(1, hits);
    TEST_ASSERT_EQUAL_UINT32(0, misses);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, hit_ratio);

    // Tentar acessar um endereço não cacheado
    TEST_ASSERT_FALSE(emu_memory_cache_get(g_cache, 0x2000, sizeof(output), output));

    // Verificar estatísticas após miss
    emu_memory_cache_stats(g_cache, &hits, &misses, &hit_ratio);
    TEST_ASSERT_EQUAL_UINT32(1, hits);
    TEST_ASSERT_EQUAL_UINT32(1, misses);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, hit_ratio);
}

// Teste de Limpeza
void test_cache_clear(void)
{
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t output[8];

    // Adicionar dados ao cache
    TEST_ASSERT_TRUE(emu_memory_cache_put(g_cache, 0x1000, sizeof(data), data));
    TEST_ASSERT_TRUE(emu_memory_cache_get(g_cache, 0x1000, sizeof(output), output));

    // Limpar o cache
    emu_memory_cache_clear(g_cache);

    // Verificar que os dados não estão mais no cache
    TEST_ASSERT_FALSE(emu_memory_cache_get(g_cache, 0x1000, sizeof(output), output));

    // Verificar que o contador de entradas foi resetado
    TEST_ASSERT_EQUAL_UINT32(0, g_cache->num_entries);
}

// Testes de Análise de Memória
void test_memory_analysis(void)
{
    emu_memory_analysis_t analysis;

    // Análise inicial
    TEST_ASSERT_TRUE(emu_memory_analyze(g_memory, &analysis));

    // Verificar leak check
    TEST_ASSERT_TRUE(emu_memory_leak_check(g_memory));

    // Gerar relatório
    emu_memory_report(g_memory, "memory_test_report.txt");

    // Este é apenas um stub de teste, como não temos a implementação completa
    // com o rastreamento de vazamentos real, apenas verificamos se as funções
    // são chamadas sem erros
}

// Testes de Perfis de Otimização
void test_memory_profiles(void)
{
    // Obter perfil predefinido
    emu_memory_profile_t *nes_profile = emu_memory_get_profile(EMU_MEMORY_PROFILE_NES);
    TEST_ASSERT_NOT_NULL(nes_profile);
    TEST_ASSERT_EQUAL(EMU_MEMORY_PROFILE_NES, nes_profile->type);

    // Aplicar perfil
    TEST_ASSERT_TRUE(emu_memory_apply_profile(g_memory, nes_profile));

    // Criar perfil personalizado
    emu_memory_profile_t *custom_profile = emu_memory_create_profile("TestProfile", 64);
    TEST_ASSERT_NOT_NULL(custom_profile);
    TEST_ASSERT_EQUAL(EMU_MEMORY_PROFILE_CUSTOM, custom_profile->type);
    TEST_ASSERT_EQUAL_STRING("TestProfile", custom_profile->name);
    TEST_ASSERT_EQUAL_UINT32(64, custom_profile->cache_size);

    // Adicionar região
    TEST_ASSERT_TRUE(emu_memory_profile_add_region(custom_profile, 0x1000, 0x1000, true, false));
    TEST_ASSERT_EQUAL_UINT32(1, custom_profile->num_regions);

    // Configurar prefetch
    TEST_ASSERT_TRUE(emu_memory_profile_configure_prefetch(custom_profile, true, 256, 2));
    TEST_ASSERT_TRUE(custom_profile->prefetch_config.enabled);
    TEST_ASSERT_EQUAL_UINT32(256, custom_profile->prefetch_config.window_size);
    TEST_ASSERT_EQUAL_UINT32(2, custom_profile->prefetch_config.trigger_count);

    // Aplicar perfil personalizado
    TEST_ASSERT_TRUE(emu_memory_apply_profile(g_memory, custom_profile));

    // Limpar
    emu_memory_destroy_profile(nes_profile);
    emu_memory_destroy_profile(custom_profile);
}

// Testes de Performance
void test_cache_performance(void)
{
    uint8_t data[64];
    uint8_t output[64];

    // Preencher dados de teste
    for (int i = 0; i < sizeof(data); i++)
    {
        data[i] = (uint8_t)i;
    }

    // 1. Teste sem cache
    uint32_t start_time = test_get_time_ms();

    for (int i = 0; i < 10000; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            uint32_t addr = 0x1000 + (j * 64);
            emu_memory_read_8(g_memory, addr, output, sizeof(output));
        }
    }

    uint32_t no_cache_time = test_get_time_ms() - start_time;

    // 2. Teste com cache
    emu_memory_cache_t *test_cache = emu_memory_cache_create(20);
    TEST_ASSERT_NOT_NULL(test_cache);

    // Pré-carregar o cache
    for (int j = 0; j < 10; j++)
    {
        uint32_t addr = 0x1000 + (j * 64);
        emu_memory_read_8(g_memory, addr, data, sizeof(data));
        emu_memory_cache_put(test_cache, addr, sizeof(data), data);
    }

    start_time = test_get_time_ms();

    for (int i = 0; i < 10000; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            uint32_t addr = 0x1000 + (j * 64);
            if (!emu_memory_cache_get(test_cache, addr, sizeof(output), output))
            {
                emu_memory_read_8(g_memory, addr, output, sizeof(output));
                emu_memory_cache_put(test_cache, addr, sizeof(output), output);
            }
        }
    }

    uint32_t with_cache_time = test_get_time_ms() - start_time;

    // Verificar se o cache melhorou o desempenho (pode não ser sempre o caso em testes simples)
    printf("Performance sem cache: %u ms\n", no_cache_time);
    printf("Performance com cache: %u ms\n", with_cache_time);
    printf("Melhoria: %d%%\n", (no_cache_time > with_cache_time) ? (int)(100.0f * (no_cache_time - with_cache_time) / no_cache_time) : 0);

    emu_memory_cache_destroy(test_cache);
}

// Função auxiliar para obter tempo em ms
uint32_t test_get_time_ms(void)
{
    // Esta é uma implementação de stub para o teste
    // Em um ambiente real, usaríamos uma função de tempo do sistema
    static uint32_t counter = 0;
    return counter++;
}

// Runner de testes
int main(void)
{
    UNITY_BEGIN();

    // Testes de estrutura e operação básica
    RUN_TEST(test_cache_create_destroy);
    RUN_TEST(test_cache_put_get);
    RUN_TEST(test_cache_lru_replacement);
    RUN_TEST(test_cache_statistics);
    RUN_TEST(test_cache_clear);

    // Testes de análise e performance
    RUN_TEST(test_memory_analysis);
    RUN_TEST(test_memory_profiles);
    RUN_TEST(test_cache_performance);

    return UNITY_END();
}
