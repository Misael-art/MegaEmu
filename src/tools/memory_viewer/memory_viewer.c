/**
 * @file memory_viewer.c
 * @brief Implementação do visualizador e editor de memória
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "memory_viewer.h"
#include "core/core.h"
#include "utils/logger.h"

// Define a categoria de log
#define LOG_CATEGORY "memory_viewer"

// Macros auxiliares para log
#define MV_LOG_ERROR(...) log_error(LOG_CATEGORY, __VA_ARGS__)
#define MV_LOG_WARN(...) log_warn(LOG_CATEGORY, __VA_ARGS__)
#define MV_LOG_INFO(...) log_info(LOG_CATEGORY, __VA_ARGS__)
#define MV_LOG_DEBUG(...) log_debug(LOG_CATEGORY, __VA_ARGS__)

// Número máximo de regiões de memória
#define MAX_MEMORY_REGIONS 32

// Número máximo de anotações
#define MAX_ANNOTATIONS 512

// Número máximo de watches
#define MAX_WATCHES 64

// Número máximo de callbacks de alteração
#define MAX_CHANGE_CALLBACKS 16

// Tamanho máximo do buffer de histórico para um único endereço
#define MAX_ADDRESS_HISTORY 64

// Acesso às bandeiras de região
#define REGION_FLAG_READABLE (1 << 0)
#define REGION_FLAG_WRITABLE (1 << 1)
#define REGION_FLAG_EXECUTABLE (1 << 2)

// Definição da estrutura de histórico de alterações para um endereço
typedef struct
{
    memory_change_info_t entries[MAX_ADDRESS_HISTORY];
    uint32_t current_index;
    uint32_t count;
} address_history_t;

// Definição da estrutura de watch
typedef struct
{
    int32_t id;
    uint32_t address;
    char name[64];
    uint8_t last_value;
    bool active;
} memory_watch_t;

// Definição da estrutura de callback de alteração
typedef struct
{
    memory_change_callback_t callback;
    void *context;
    bool active;
} memory_change_callback_entry_t;

// Implementação interna do visualizador de memória
struct memory_viewer_t
{
    uint32_t platform_id;

    // Acesso à memória
    void *memory_context;
    memory_read_callback_t read_callback;
    memory_write_callback_t write_callback;

    // Configuração
    memory_viewer_config_t config;

    // Regiões de memória
    memory_region_desc_t regions[MAX_MEMORY_REGIONS];
    int32_t active_region_id;
    uint32_t region_count;

    // Estado de visualização
    uint32_t current_address;
    memory_view_mode_t view_mode;

    // Anotações e marcadores
    memory_annotation_t annotations[MAX_ANNOTATIONS];
    uint32_t annotation_count;

    // Histórico de alterações
    address_history_t *address_histories;
    uint32_t history_table_size;

    // Watches
    memory_watch_t watches[MAX_WATCHES];
    uint32_t watch_count;

    // Callbacks de alteração
    memory_change_callback_entry_t change_callbacks[MAX_CHANGE_CALLBACKS];
    uint32_t change_callback_count;

    // Callback para visualização personalizada
    memory_custom_view_callback_t custom_view_callback;
    void *custom_view_context;

    // Callback para pré-processamento
    memory_preprocess_callback_t preprocess_callback;
    void *preprocess_context;

    // Estado interno
    uint64_t last_refresh_time;
    bool needs_refresh;
    uint32_t next_annotation_id;
    uint32_t next_watch_id;
};

// Função auxiliar para obter timestamp atual em ms
static uint64_t get_current_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// Função auxiliar para inicializar uma nova instância com valores padrão
static void init_memory_viewer_defaults(memory_viewer_t *viewer)
{
    if (!viewer)
        return;

    // Inicializar configuração com valores sensatos
    viewer->config.default_view_mode = MEMORY_VIEW_HEX;
    viewer->config.bytes_per_row = 16;
    viewer->config.visible_rows = 16;
    viewer->config.refresh_interval_ms = 250;
    viewer->config.flags = MEMORY_VIEWER_FLAG_HIGHLIGHT_CHANGES |
                           MEMORY_VIEWER_FLAG_AUTO_REFRESH;
    viewer->config.highlight_duration_ms = 1000;
    viewer->config.history_size = 100;
    viewer->config.follow_execution = false;

    // Inicializar estado
    viewer->view_mode = viewer->config.default_view_mode;
    viewer->current_address = 0;
    viewer->active_region_id = -1;
    viewer->region_count = 0;
    viewer->annotation_count = 0;
    viewer->watch_count = 0;
    viewer->change_callback_count = 0;
    viewer->needs_refresh = true;
    viewer->next_annotation_id = 1;
    viewer->next_watch_id = 1;

    // Inicializar tabela de histórico (implementação simples)
    // Uma tabela de hash mais eficiente seria melhor para produção
    viewer->history_table_size = 256; // Tamanho arbitrário, ajustar conforme necessário
    viewer->address_histories = (address_history_t *)calloc(
        viewer->history_table_size, sizeof(address_history_t));
}

// Função auxiliar para calcular índice de hash para histórico
static uint32_t history_hash_index(memory_viewer_t *viewer, uint32_t address)
{
    if (!viewer || !viewer->address_histories)
        return 0;
    return address % viewer->history_table_size;
}

// Função auxiliar para encontrar região por endereço
static int32_t find_region_by_address(memory_viewer_t *viewer, uint32_t address)
{
    if (!viewer)
        return -1;

    for (uint32_t i = 0; i < viewer->region_count; i++)
    {
        if (address >= viewer->regions[i].start_address &&
            address <= viewer->regions[i].end_address)
        {
            return i;
        }
    }

    return -1;
}

// Função auxiliar para verificar acesso de leitura em uma região
static bool is_region_readable(const memory_region_desc_t *region)
{
    if (!region)
        return false;
    return (region->access_flags & REGION_FLAG_READABLE) != 0;
}

// Função auxiliar para verificar acesso de escrita em uma região
static bool is_region_writable(const memory_region_desc_t *region)
{
    if (!region)
        return false;
    return (region->access_flags & REGION_FLAG_WRITABLE) != 0;
}

// Funções de implementação da API

memory_viewer_t *memory_viewer_create(
    uint32_t platform_id,
    void *context,
    memory_read_callback_t read_callback,
    memory_write_callback_t write_callback)
{
    // Verificar parâmetros essenciais
    if (!read_callback)
    {
        MV_LOG_ERROR("Não foi possível criar o visualizador de memória: callback de leitura necessário");
        return NULL;
    }

    // Alocar estrutura
    memory_viewer_t *viewer = (memory_viewer_t *)calloc(1, sizeof(memory_viewer_t));
    if (!viewer)
    {
        MV_LOG_ERROR("Não foi possível alocar memória para o visualizador de memória");
        return NULL;
    }

    // Configurar acesso à memória
    viewer->platform_id = platform_id;
    viewer->memory_context = context;
    viewer->read_callback = read_callback;
    viewer->write_callback = write_callback;

    // Inicializar valores padrão
    init_memory_viewer_defaults(viewer);

    MV_LOG_INFO("Visualizador de memória criado para plataforma ID %u", platform_id);
    return viewer;
}

void memory_viewer_destroy(memory_viewer_t *viewer)
{
    if (!viewer)
        return;

    // Limpar histórico
    if (viewer->address_histories)
    {
        free(viewer->address_histories);
        viewer->address_histories = NULL;
    }

    // Liberar a estrutura principal
    free(viewer);

    MV_LOG_INFO("Visualizador de memória destruído");
}

bool memory_viewer_configure(memory_viewer_t *viewer, const memory_viewer_config_t *config)
{
    if (!viewer || !config)
    {
        MV_LOG_ERROR("Parâmetros inválidos para configuração do visualizador");
        return false;
    }

    // Copiar a configuração
    memcpy(&viewer->config, config, sizeof(memory_viewer_config_t));

    // Atualizar estado dependente da configuração
    viewer->view_mode = config->default_view_mode;
    viewer->needs_refresh = true;

    MV_LOG_INFO("Visualizador de memória configurado");
    return true;
}

bool memory_viewer_get_config(memory_viewer_t *viewer, memory_viewer_config_t *config)
{
    if (!viewer || !config)
    {
        MV_LOG_ERROR("Parâmetros inválidos para obter configuração");
        return false;
    }

    // Copiar a configuração atual
    memcpy(config, &viewer->config, sizeof(memory_viewer_config_t));

    return true;
}

int32_t memory_viewer_add_region(memory_viewer_t *viewer, const memory_region_desc_t *region)
{
    if (!viewer || !region)
    {
        MV_LOG_ERROR("Parâmetros inválidos para adicionar região");
        return -1;
    }

    // Verificar se já atingimos o limite de regiões
    if (viewer->region_count >= MAX_MEMORY_REGIONS)
    {
        MV_LOG_ERROR("Número máximo de regiões atingido");
        return -1;
    }

    // Verificar parâmetros da região
    if (region->start_address > region->end_address)
    {
        MV_LOG_ERROR("Endereço inicial maior que endereço final para região");
        return -1;
    }

    // Adicionar a região
    int32_t region_id = (int32_t)viewer->region_count;
    memcpy(&viewer->regions[region_id], region, sizeof(memory_region_desc_t));

    // Incrementar contador
    viewer->region_count++;

    // Se for a primeira região, torná-la ativa automaticamente
    if (viewer->region_count == 1)
    {
        viewer->active_region_id = 0;
        viewer->current_address = region->visible_start;
    }

    MV_LOG_INFO("Região adicionada: %s (0x%08X-0x%08X)",
                region->name, region->start_address, region->end_address);

    return region_id;
}

bool memory_viewer_remove_region(memory_viewer_t *viewer, int32_t region_id)
{
    if (!viewer || region_id < 0 || region_id >= (int32_t)viewer->region_count)
    {
        MV_LOG_ERROR("ID de região inválido para remoção: %d", region_id);
        return false;
    }

    // Se for a última região, basta decrementar o contador
    if (region_id == (int32_t)viewer->region_count - 1)
    {
        viewer->region_count--;
    }
    else
    {
        // Caso contrário, mover as regiões subsequentes para preencher o espaço
        for (uint32_t i = region_id; i < viewer->region_count - 1; i++)
        {
            memcpy(&viewer->regions[i], &viewer->regions[i + 1], sizeof(memory_region_desc_t));
        }
        viewer->region_count--;
    }

    // Atualizar região ativa se necessário
    if (viewer->active_region_id == region_id)
    {
        viewer->active_region_id = (viewer->region_count > 0) ? 0 : -1;
    }
    else if (viewer->active_region_id > region_id)
    {
        // Ajustar índice da região ativa se ela estava após a removida
        viewer->active_region_id--;
    }

    MV_LOG_INFO("Região %d removida", region_id);
    return true;
}

bool memory_viewer_set_active_region(memory_viewer_t *viewer, int32_t region_id)
{
    if (!viewer || region_id < 0 || region_id >= (int32_t)viewer->region_count)
    {
        MV_LOG_ERROR("ID de região inválido para ativação: %d", region_id);
        return false;
    }

    viewer->active_region_id = region_id;

    // Atualizar endereço atual para o início da região visível
    viewer->current_address = viewer->regions[region_id].visible_start;
    viewer->needs_refresh = true;

    MV_LOG_INFO("Região ativa definida: %d (%s)",
                region_id, viewer->regions[region_id].name);
    return true;
}

bool memory_viewer_set_address(memory_viewer_t *viewer, uint32_t address)
{
    if (!viewer)
    {
        MV_LOG_ERROR("Visualizador inválido para definir endereço");
        return false;
    }

    // Se não tiver região ativa, tentar encontrar região para o endereço
    if (viewer->active_region_id < 0)
    {
        int32_t region_id = find_region_by_address(viewer, address);
        if (region_id >= 0)
        {
            viewer->active_region_id = region_id;
        }
        else
        {
            MV_LOG_WARN("Endereço 0x%08X não corresponde a nenhuma região", address);
            // Ainda assim definir o endereço, mas avisar
        }
    }
    else
    {
        // Verificar se o endereço está dentro da região ativa
        memory_region_desc_t *region = &viewer->regions[viewer->active_region_id];
        if (address < region->start_address || address > region->end_address)
        {
            // Tentar encontrar outra região para este endereço
            int32_t region_id = find_region_by_address(viewer, address);
            if (region_id >= 0)
            {
                viewer->active_region_id = region_id;
                MV_LOG_INFO("Mudou para região %d (%s) para endereço 0x%08X",
                            region_id, viewer->regions[region_id].name, address);
            }
            else
            {
                MV_LOG_WARN("Endereço 0x%08X está fora da região ativa e não corresponde a nenhuma região",
                            address);
                // Ainda assim definir o endereço, mas avisar
            }
        }
    }

    viewer->current_address = address;
    viewer->needs_refresh = true;

    MV_LOG_DEBUG("Endereço atual definido: 0x%08X", address);
    return true;
}

uint32_t memory_viewer_get_address(memory_viewer_t *viewer)
{
    if (!viewer)
    {
        MV_LOG_ERROR("Visualizador inválido para obter endereço");
        return 0xFFFFFFFF;
    }

    return viewer->current_address;
}

bool memory_viewer_set_view_mode(memory_viewer_t *viewer, memory_view_mode_t mode)
{
    if (!viewer)
    {
        MV_LOG_ERROR("Visualizador inválido para definir modo de visualização");
        return false;
    }

    // Verificar modo válido
    if (mode < MEMORY_VIEW_HEX || mode > MEMORY_VIEW_CUSTOM)
    {
        MV_LOG_ERROR("Modo de visualização inválido: %d", mode);
        return false;
    }

    viewer->view_mode = mode;
    viewer->needs_refresh = true;

    MV_LOG_INFO("Modo de visualização definido: %d", mode);
    return true;
}

// Função auxiliar para registrar alteração no histórico
static void register_memory_change(
    memory_viewer_t *viewer,
    uint32_t address,
    uint8_t old_value,
    uint8_t new_value)
{
    if (!viewer || !viewer->address_histories)
        return;

    // Calcular índice na tabela de hash
    uint32_t index = history_hash_index(viewer, address);
    address_history_t *history = &viewer->address_histories[index];

    // Registrar a alteração
    uint32_t entry_index = history->current_index;
    history->entries[entry_index].address = address;
    history->entries[entry_index].old_value = old_value;
    history->entries[entry_index].new_value = new_value;
    history->entries[entry_index].timestamp = get_current_time_ms();

    // Atualizar índice e contador
    history->current_index = (history->current_index + 1) % MAX_ADDRESS_HISTORY;
    if (history->count < MAX_ADDRESS_HISTORY)
    {
        history->count++;
    }

    // Notificar callbacks de alteração
    for (uint32_t i = 0; i < viewer->change_callback_count; i++)
    {
        if (viewer->change_callbacks[i].active && viewer->change_callbacks[i].callback)
        {
            viewer->change_callbacks[i].callback(
                viewer->change_callbacks[i].context,
                &history->entries[entry_index]);
        }
    }

    // Verificar e atualizar watches
    for (uint32_t i = 0; i < viewer->watch_count; i++)
    {
        if (viewer->watches[i].active && viewer->watches[i].address == address)
        {
            viewer->watches[i].last_value = new_value;
        }
    }
}

// Implementação de funções de escrita e leitura
bool memory_viewer_write(
    memory_viewer_t *viewer,
    uint32_t address,
    const uint8_t *value,
    uint32_t size)
{
    if (!viewer || !value || size == 0)
    {
        MV_LOG_ERROR("Parâmetros inválidos para escrita na memória");
        return false;
    }

    // Verificar callback de escrita
    if (!viewer->write_callback)
    {
        MV_LOG_ERROR("Callback de escrita não definido");
        return false;
    }

    // Verificar se a região permite escrita
    int32_t region_id = find_region_by_address(viewer, address);
    if (region_id >= 0)
    {
        if (!is_region_writable(&viewer->regions[region_id]))
        {
            MV_LOG_ERROR("Endereço 0x%08X não é gravável", address);
            return false;
        }
    }

    // Escrever byte a byte para registrar alterações
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t curr_addr = address + i;

        // Ler valor antigo para registro de alteração
        uint8_t old_value = 0;
        if (viewer->read_callback)
        {
            old_value = viewer->read_callback(viewer->memory_context, curr_addr);
        }

        // Executar a escrita
        viewer->write_callback(viewer->memory_context, curr_addr, value[i]);

        // Registrar a alteração
        register_memory_change(viewer, curr_addr, old_value, value[i]);
    }

    viewer->needs_refresh = true;
    return true;
}

bool memory_viewer_read(
    memory_viewer_t *viewer,
    uint32_t address,
    uint8_t *buffer,
    uint32_t size)
{
    if (!viewer || !buffer || size == 0)
    {
        MV_LOG_ERROR("Parâmetros inválidos para leitura da memória");
        return false;
    }

    // Verificar callback de leitura
    if (!viewer->read_callback)
    {
        MV_LOG_ERROR("Callback de leitura não definido");
        return false;
    }

    // Verificar se a região permite leitura
    int32_t region_id = find_region_by_address(viewer, address);
    if (region_id >= 0)
    {
        if (!is_region_readable(&viewer->regions[region_id]))
        {
            MV_LOG_ERROR("Endereço 0x%08X não é legível", address);
            return false;
        }
    }

    // Ler byte a byte
    for (uint32_t i = 0; i < size; i++)
    {
        buffer[i] = viewer->read_callback(viewer->memory_context, address + i);
    }

    // Aplicar pré-processamento se definido
    if (viewer->preprocess_callback)
    {
        viewer->preprocess_callback(viewer->preprocess_context, address, buffer, size);
    }

    return true;
}

// Funções de gerenciamento de anotações
int32_t memory_viewer_add_annotation(memory_viewer_t *viewer, const memory_annotation_t *annotation)
{
    if (!viewer || !annotation)
    {
        MV_LOG_ERROR("Parâmetros inválidos para adicionar anotação");
        return -1;
    }

    // Verificar se há espaço para mais anotações
    if (viewer->annotation_count >= MAX_ANNOTATIONS)
    {
        MV_LOG_ERROR("Número máximo de anotações atingido");
        return -1;
    }

    // Adicionar a anotação
    int32_t annotation_id = viewer->next_annotation_id++;
    uint32_t index = viewer->annotation_count++;

    memcpy(&viewer->annotations[index], annotation, sizeof(memory_annotation_t));

    MV_LOG_INFO("Anotação adicionada para endereço 0x%08X: %s",
                annotation->address, annotation->label);
    return annotation_id;
}

// Função auxiliar para encontrar índice de uma anotação pelo ID
static int32_t find_annotation_index(memory_viewer_t *viewer, int32_t annotation_id)
{
    if (!viewer)
        return -1;

    for (uint32_t i = 0; i < viewer->annotation_count; i++)
    {
        if (viewer->annotations[i].address == annotation_id)
        {
            return i;
        }
    }

    return -1;
}
