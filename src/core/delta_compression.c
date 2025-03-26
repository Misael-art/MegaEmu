/**
 * @file delta_compression.c
 * @brief Implementação de compressão delta para save states
 */

#include "save_state.h"
#include "../utils/enhanced_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Definição da categoria de log
#define LOG_CAT_DELTA EMU_LOG_CAT_CORE

// Tamanho do buffer de trabalho
#define DELTA_WORK_BUFFER_SIZE (1024 * 1024) // 1 MB

// Estrutura para armazenar o estado anterior para compressão delta
typedef struct
{
    void *data;     // Dados do estado anterior
    uint32_t size;  // Tamanho dos dados
    uint32_t crc32; // Checksum dos dados
} previous_state_t;

// Estrutura para registro delta
typedef struct
{
    uint32_t offset; // Posição onde a alteração começa
    uint32_t length; // Tamanho da alteração em bytes
    uint8_t data[];  // Dados alterados (tamanho variável)
} delta_record_t;

// Buffer de trabalho global para compressão delta
static uint8_t *g_delta_work_buffer = NULL;

// Estado anterior para cada campo
static previous_state_t *g_previous_states = NULL;
static int g_previous_states_count = 0;
static int g_previous_states_capacity = 0;

/**
 * @brief Inicializa o sistema de compressão delta
 */
int32_t delta_compression_init(void)
{
    // Alocar buffer de trabalho
    g_delta_work_buffer = (uint8_t *)malloc(DELTA_WORK_BUFFER_SIZE);
    if (!g_delta_work_buffer)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao alocar buffer de trabalho para compressão delta");
        return SAVE_STATE_ERROR_MEMORY;
    }

    // Inicializar array para estados anteriores
    g_previous_states_capacity = 256; // Capacidade inicial
    g_previous_states = (previous_state_t *)malloc(g_previous_states_capacity * sizeof(previous_state_t));
    if (!g_previous_states)
    {
        free(g_delta_work_buffer);
        g_delta_work_buffer = NULL;
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao alocar memória para estados anteriores");
        return SAVE_STATE_ERROR_MEMORY;
    }

    g_previous_states_count = 0;

    EMU_LOG_INFO(LOG_CAT_DELTA, "Sistema de compressão delta inicializado");
    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Finaliza o sistema de compressão delta
 */
void delta_compression_shutdown(void)
{
    // Liberar buffer de trabalho
    if (g_delta_work_buffer)
    {
        free(g_delta_work_buffer);
        g_delta_work_buffer = NULL;
    }

    // Liberar estados anteriores
    if (g_previous_states)
    {
        for (int i = 0; i < g_previous_states_count; i++)
        {
            if (g_previous_states[i].data)
            {
                free(g_previous_states[i].data);
            }
        }
        free(g_previous_states);
        g_previous_states = NULL;
    }

    g_previous_states_count = 0;
    g_previous_states_capacity = 0;

    EMU_LOG_INFO(LOG_CAT_DELTA, "Sistema de compressão delta finalizado");
}

/**
 * @brief Calcula o CRC32 de um buffer de dados
 */
static uint32_t calculate_crc32(const void *data, size_t size)
{
    // Implementação simples de CRC32
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t *bytes = (const uint8_t *)data;

    for (size_t i = 0; i < size; i++)
    {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }

    return ~crc;
}

/**
 * @brief Encontra o índice do estado anterior para um campo
 */
static int find_previous_state(const char *field_name)
{
    for (int i = 0; i < g_previous_states_count; i++)
    {
        if (strcmp(field_name, (const char *)g_previous_states[i].data) == 0)
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Adiciona ou atualiza um estado anterior
 */
static int update_previous_state(const char *field_name, const void *data, uint32_t size)
{
    // Verificar se o campo já existe
    int index = find_previous_state(field_name);

    if (index < 0)
    {
        // Campo não encontrado, verificar se tem espaço
        if (g_previous_states_count >= g_previous_states_capacity)
        {
            // Expandir o array
            int new_capacity = g_previous_states_capacity * 2;
            previous_state_t *new_states = (previous_state_t *)realloc(
                g_previous_states,
                new_capacity * sizeof(previous_state_t));

            if (!new_states)
            {
                EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao expandir array de estados anteriores");
                return -1;
            }

            g_previous_states = new_states;
            g_previous_states_capacity = new_capacity;
        }

        // Adicionar novo estado
        index = g_previous_states_count++;

        // Alocar espaço para o nome do campo e dados
        size_t name_len = strlen(field_name) + 1;
        g_previous_states[index].data = malloc(size + name_len);
        if (!g_previous_states[index].data)
        {
            EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao alocar memória para estado anterior");
            g_previous_states_count--;
            return -1;
        }

        // Guardar o nome do campo no início do bloco
        memcpy(g_previous_states[index].data, field_name, name_len);

        // Guardar os dados após o nome
        uint8_t *data_ptr = (uint8_t *)g_previous_states[index].data + name_len;
        memcpy(data_ptr, data, size);

        g_previous_states[index].size = size;
        g_previous_states[index].crc32 = calculate_crc32(data, size);

        EMU_LOG_DEBUG(LOG_CAT_DELTA, "Novo estado anterior adicionado: %s (%u bytes)", field_name, size);
    }
    else
    {
        // Campo encontrado, atualizar dados
        size_t name_len = strlen(field_name) + 1;
        uint8_t *data_ptr = (uint8_t *)g_previous_states[index].data + name_len;

        // Verificar se o tamanho mudou
        if (g_previous_states[index].size != size)
        {
            // Realocar memória
            void *new_data = realloc(g_previous_states[index].data, size + name_len);
            if (!new_data)
            {
                EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao realocar memória para estado anterior");
                return -1;
            }

            g_previous_states[index].data = new_data;
            data_ptr = (uint8_t *)g_previous_states[index].data + name_len;
        }

        // Atualizar dados
        memcpy(data_ptr, data, size);
        g_previous_states[index].size = size;
        g_previous_states[index].crc32 = calculate_crc32(data, size);

        EMU_LOG_DEBUG(LOG_CAT_DELTA, "Estado anterior atualizado: %s (%u bytes)", field_name, size);
    }

    return index;
}

/**
 * @brief Calcula as diferenças entre dois blocos de dados
 */
static uint32_t calculate_delta(
    const uint8_t *current_data,
    uint32_t current_size,
    const uint8_t *prev_data,
    uint32_t prev_size,
    uint8_t *delta_buffer,
    uint32_t delta_buffer_size)
{
    // Verifica se os tamanhos são diferentes
    if (current_size != prev_size)
    {
        // Se os tamanhos são diferentes, não podemos usar delta compression
        // Retornamos 0 para indicar que não conseguimos comprimir
        return 0;
    }

    uint32_t delta_size = 0;
    uint32_t header_size = sizeof(uint32_t); // Para armazenar o número de registros
    uint32_t record_count = 0;

    // Ponteiro para a posição atual no buffer de delta
    uint8_t *delta_ptr = delta_buffer + header_size;

    // Espaço restante no buffer
    uint32_t remaining_space = delta_buffer_size - header_size;

    // Procurar por diferenças
    uint32_t diff_start = 0;
    bool in_diff = false;

    for (uint32_t i = 0; i < current_size; i++)
    {
        if (current_data[i] != prev_data[i])
        {
            // Encontrou uma diferença
            if (!in_diff)
            {
                // Inicia um novo registro de diferença
                diff_start = i;
                in_diff = true;
            }
        }
        else
        {
            // Dados iguais
            if (in_diff)
            {
                // Finaliza o registro de diferença anterior
                uint32_t diff_length = i - diff_start;

                // Verificar se temos espaço suficiente no buffer
                uint32_t record_size = sizeof(delta_record_t) + diff_length;
                if (remaining_space < record_size)
                {
                    // Sem espaço suficiente, delta não é vantajoso
                    return 0;
                }

                // Criar o registro
                delta_record_t *record = (delta_record_t *)delta_ptr;
                record->offset = diff_start;
                record->length = diff_length;
                memcpy(record->data, &current_data[diff_start], diff_length);

                // Atualizar ponteiros e contadores
                delta_ptr += record_size;
                remaining_space -= record_size;
                delta_size += record_size;
                record_count++;

                in_diff = false;
            }
        }
    }

    // Verificar se ainda existe uma diferença no final do buffer
    if (in_diff)
    {
        uint32_t diff_length = current_size - diff_start;

        // Verificar se temos espaço suficiente no buffer
        uint32_t record_size = sizeof(delta_record_t) + diff_length;
        if (remaining_space < record_size)
        {
            // Sem espaço suficiente, delta não é vantajoso
            return 0;
        }

        // Criar o registro
        delta_record_t *record = (delta_record_t *)delta_ptr;
        record->offset = diff_start;
        record->length = diff_length;
        memcpy(record->data, &current_data[diff_start], diff_length);

        // Atualizar contadores
        delta_size += record_size;
        record_count++;
    }

    // Escrever o número de registros no cabeçalho
    *(uint32_t *)delta_buffer = record_count;

    // Adicionar o tamanho do cabeçalho ao tamanho total
    delta_size += header_size;

    // Verificar se vale a pena usar delta compression
    if (delta_size >= current_size)
    {
        // Delta não é vantajoso, é maior ou igual ao tamanho original
        return 0;
    }

    EMU_LOG_DEBUG(LOG_CAT_DELTA, "Delta calculado: %u bytes (%u registros)", delta_size, record_count);
    return delta_size;
}

/**
 * @brief Aplica delta em um bloco de dados
 */
static bool apply_delta(
    uint8_t *data,
    uint32_t data_size,
    const uint8_t *delta,
    uint32_t delta_size)
{
    // Verificar tamanho mínimo
    if (delta_size < sizeof(uint32_t))
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Delta muito pequeno para conter cabeçalho");
        return false;
    }

    // Ler número de registros
    uint32_t record_count = *(uint32_t *)delta;

    // Ponteiro para o primeiro registro
    const uint8_t *record_ptr = delta + sizeof(uint32_t);

    // Aplicar cada registro
    for (uint32_t i = 0; i < record_count; i++)
    {
        // Converter ponteiro para registro
        const delta_record_t *record = (const delta_record_t *)record_ptr;

        // Verificar se o registro está dentro dos limites
        if (record->offset + record->length > data_size)
        {
            EMU_LOG_ERROR(LOG_CAT_DELTA, "Registro delta fora dos limites: offset=%u, length=%u, data_size=%u",
                          record->offset, record->length, data_size);
            return false;
        }

        // Aplicar as alterações
        memcpy(&data[record->offset], record->data, record->length);

        // Avançar para o próximo registro
        record_ptr += sizeof(delta_record_t) + record->length;
    }

    return true;
}

/**
 * @brief Comprime um campo usando delta compression
 */
int32_t delta_compress_field(
    save_state_t *state,
    const char *field_name,
    const void *data,
    uint32_t size,
    void **compressed_data,
    uint32_t *compressed_size)
{
    if (!state || !field_name || !data || !compressed_data || !compressed_size)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Parâmetros inválidos para compressão delta");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se o buffer de trabalho está inicializado
    if (!g_delta_work_buffer || !g_previous_states)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Sistema de compressão delta não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se o campo tem um estado anterior
    int prev_index = find_previous_state(field_name);
    if (prev_index < 0)
    {
        // Sem estado anterior, armazenar o estado atual e usar dados não comprimidos
        update_previous_state(field_name, data, size);

        // Copiar dados não comprimidos
        *compressed_data = malloc(size);
        if (!*compressed_data)
        {
            EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao alocar memória para dados comprimidos");
            return SAVE_STATE_ERROR_MEMORY;
        }

        memcpy(*compressed_data, data, size);
        *compressed_size = size;

        EMU_LOG_INFO(LOG_CAT_DELTA, "Sem estado anterior para %s, usando dados não comprimidos", field_name);
        return SAVE_STATE_ERROR_NONE;
    }

    // Obter dados do estado anterior
    size_t name_len = strlen(field_name) + 1;
    const uint8_t *prev_data = (const uint8_t *)g_previous_states[prev_index].data + name_len;
    uint32_t prev_size = g_previous_states[prev_index].size;

    // Calcular o delta
    uint32_t delta_size = calculate_delta(
        data, size,
        prev_data, prev_size,
        g_delta_work_buffer, DELTA_WORK_BUFFER_SIZE);

    if (delta_size == 0)
    {
        // Delta não é vantajoso, usar dados não comprimidos
        *compressed_data = malloc(size);
        if (!*compressed_data)
        {
            EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao alocar memória para dados comprimidos");
            return SAVE_STATE_ERROR_MEMORY;
        }

        memcpy(*compressed_data, data, size);
        *compressed_size = size;

        // Atualizar estado anterior
        update_previous_state(field_name, data, size);

        EMU_LOG_INFO(LOG_CAT_DELTA, "Delta não vantajoso para %s, usando dados não comprimidos", field_name);
        return SAVE_STATE_ERROR_NONE;
    }

    // Delta é vantajoso, copiar para buffer de saída
    *compressed_data = malloc(delta_size);
    if (!*compressed_data)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao alocar memória para dados delta");
        return SAVE_STATE_ERROR_MEMORY;
    }

    memcpy(*compressed_data, g_delta_work_buffer, delta_size);
    *compressed_size = delta_size;

    // Atualizar estado anterior
    update_previous_state(field_name, data, size);

    EMU_LOG_INFO(LOG_CAT_DELTA, "Campo %s comprimido com delta: %u -> %u bytes (%.1f%%)",
                 field_name, size, delta_size, (float)delta_size * 100 / size);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Descomprime um campo usando delta compression
 */
int32_t delta_decompress_field(
    save_state_t *state,
    const char *field_name,
    const void *compressed_data,
    uint32_t compressed_size,
    void *output_data,
    uint32_t output_size)
{
    if (!state || !field_name || !compressed_data || !output_data)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Parâmetros inválidos para descompressão delta");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se o campo tem um estado anterior
    int prev_index = find_previous_state(field_name);
    if (prev_index < 0)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Sem estado anterior para %s, impossível aplicar delta", field_name);
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se os dados são delta ou não
    if (compressed_size >= output_size)
    {
        // Provavelmente não é delta comprimido, copiar diretamente
        memcpy(output_data, compressed_data, output_size);

        // Atualizar estado anterior
        update_previous_state(field_name, output_data, output_size);

        EMU_LOG_INFO(LOG_CAT_DELTA, "Dados não delta para %s, copiados diretamente", field_name);
        return SAVE_STATE_ERROR_NONE;
    }

    // Obter dados do estado anterior
    size_t name_len = strlen(field_name) + 1;
    const uint8_t *prev_data = (const uint8_t *)g_previous_states[prev_index].data + name_len;
    uint32_t prev_size = g_previous_states[prev_index].size;

    // Verificar tamanho
    if (prev_size != output_size)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Tamanho do estado anterior (%u) difere do tamanho de saída (%u)",
                      prev_size, output_size);
        return SAVE_STATE_ERROR_INVALID;
    }

    // Copiar estado anterior para o buffer de saída
    memcpy(output_data, prev_data, output_size);

    // Aplicar delta
    if (!apply_delta(output_data, output_size, compressed_data, compressed_size))
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Falha ao aplicar delta para campo %s", field_name);
        return SAVE_STATE_ERROR_DECOMPRESSION;
    }

    // Atualizar estado anterior
    update_previous_state(field_name, output_data, output_size);

    EMU_LOG_INFO(LOG_CAT_DELTA, "Campo %s descomprimido com delta: %u -> %u bytes",
                 field_name, compressed_size, output_size);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Limpa o estado anterior de um campo
 */
int32_t delta_clear_field_state(const char *field_name)
{
    if (!field_name)
    {
        EMU_LOG_ERROR(LOG_CAT_DELTA, "Nome do campo inválido");
        return SAVE_STATE_ERROR_INVALID;
    }

    int prev_index = find_previous_state(field_name);
    if (prev_index < 0)
    {
        // Campo não encontrado, nada a fazer
        return SAVE_STATE_ERROR_NONE;
    }

    // Liberar memória
    free(g_previous_states[prev_index].data);

    // Mover os elementos restantes para preencher o espaço
    if (prev_index < g_previous_states_count - 1)
    {
        memmove(
            &g_previous_states[prev_index],
            &g_previous_states[prev_index + 1],
            (g_previous_states_count - prev_index - 1) * sizeof(previous_state_t));
    }

    g_previous_states_count--;

    EMU_LOG_INFO(LOG_CAT_DELTA, "Estado anterior do campo %s removido", field_name);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Limpa todos os estados anteriores
 */
void delta_clear_all_states(void)
{
    if (!g_previous_states)
    {
        return;
    }

    for (int i = 0; i < g_previous_states_count; i++)
    {
        if (g_previous_states[i].data)
        {
            free(g_previous_states[i].data);
            g_previous_states[i].data = NULL;
        }
    }

    g_previous_states_count = 0;

    EMU_LOG_INFO(LOG_CAT_DELTA, "Todos os estados anteriores foram removidos");
}
