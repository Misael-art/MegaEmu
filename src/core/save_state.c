/**
 * @file save_state.c
 * @brief Implementação do sistema de save state
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-23
 */

#include "save_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Versão simplificada para testes

// Assinatura para identificar arquivos de save state
#define SAVE_STATE_SIGNATURE "MEGAEMU"
#define SAVE_STATE_VERSION 1
#define SAVE_STATE_MAX_REGIONS 16
#define SAVE_STATE_MAX_METADATA 32
#define SAVE_STATE_MAX_NAME_LEN 32
#define SAVE_STATE_MAX_VALUE_LEN 256

// Para substituir os logs
#define EMU_LOG_INFO(cat, fmt, ...) printf("INFO: " fmt "\n", ##__VA_ARGS__)
#define EMU_LOG_ERROR(cat, fmt, ...) printf("ERROR: " fmt "\n", ##__VA_ARGS__)
#define EMU_LOG_DEBUG(cat, fmt, ...) printf("DEBUG: " fmt "\n", ##__VA_ARGS__)
#define EMU_LOG_WARN(cat, fmt, ...) printf("WARN: " fmt "\n", ##__VA_ARGS__)
#define EMU_LOG_CAT_CORE 0
#define EMU_LOG_CAT_SAVE_STATE 0

// Estrutura para uma região de memória
typedef struct
{
    char name[SAVE_STATE_MAX_NAME_LEN];
    void *memory;
    uint32_t size;
} memory_region_t;

// Estrutura para um item de metadados
typedef struct
{
    char key[SAVE_STATE_MAX_NAME_LEN];
    char value[SAVE_STATE_MAX_VALUE_LEN];
} metadata_item_t;

// Estrutura para o cabeçalho do arquivo
typedef struct
{
    char signature[8];
    uint32_t version;
    uint32_t num_regions;
    uint32_t num_metadata;
} save_state_header_t;

// Estrutura para o cabeçalho de uma região
typedef struct
{
    char name[SAVE_STATE_MAX_NAME_LEN];
    uint32_t size;
} region_header_t;

// Estrutura para o contexto de save state
struct save_state_s
{
    memory_region_t regions[SAVE_STATE_MAX_REGIONS];
    uint32_t num_regions;

    metadata_item_t metadata[SAVE_STATE_MAX_METADATA];
    uint32_t num_metadata;
};

// Funções auxiliares
static int find_region_index(save_state_t *state, const char *name)
{
    if (!state || !name)
        return -1;

    for (uint32_t i = 0; i < state->num_regions; i++)
    {
        if (strcmp(state->regions[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

static int find_metadata_index(save_state_t *state, const char *key)
{
    if (!state || !key)
        return -1;

    for (uint32_t i = 0; i < state->num_metadata; i++)
    {
        if (strcmp(state->metadata[i].key, key) == 0)
        {
            return i;
        }
    }

    return -1;
}

save_state_t *save_state_create(void)
{
    save_state_t *state = (save_state_t *)malloc(sizeof(save_state_t));
    if (state)
    {
        memset(state, 0, sizeof(save_state_t));
    }
    return state;
}

void save_state_destroy(save_state_t *state)
{
    if (state)
    {
        free(state);
    }
}

save_state_result_t save_state_register_memory(save_state_t *state, const char *name, void *memory, uint32_t size)
{
    if (!state || !name || !memory || size == 0)
    {
        return SAVE_STATE_ERROR;
    }

    // Verificar se já existe uma região com este nome
    if (find_region_index(state, name) >= 0)
    {
        return SAVE_STATE_ERROR;
    }

    // Verificar se já atingimos o número máximo de regiões
    if (state->num_regions >= SAVE_STATE_MAX_REGIONS)
    {
        return SAVE_STATE_ERROR;
    }

    // Registrar a nova região
    memory_region_t *region = &state->regions[state->num_regions];
    strncpy(region->name, name, SAVE_STATE_MAX_NAME_LEN - 1);
    region->name[SAVE_STATE_MAX_NAME_LEN - 1] = '\0';
    region->memory = memory;
    region->size = size;
    state->num_regions++;

    return SAVE_STATE_OK;
}

save_state_result_t save_state_save(save_state_t *state, const char *filename)
{
    if (!state || !filename)
    {
        return SAVE_STATE_ERROR;
    }

    // Abrir o arquivo para escrita
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        return SAVE_STATE_ERROR;
    }

    // Escrever o cabeçalho
    save_state_header_t header;
    strncpy(header.signature, SAVE_STATE_SIGNATURE, sizeof(header.signature));
    header.version = SAVE_STATE_VERSION;
    header.num_regions = state->num_regions;
    header.num_metadata = state->num_metadata;

    if (fwrite(&header, sizeof(header), 1, file) != 1)
    {
        fclose(file);
        return SAVE_STATE_ERROR;
    }

    // Escrever metadados
    for (uint32_t i = 0; i < state->num_metadata; i++)
    {
        if (fwrite(&state->metadata[i], sizeof(metadata_item_t), 1, file) != 1)
        {
            fclose(file);
            return SAVE_STATE_ERROR;
        }
    }

    // Escrever as regiões de memória
    for (uint32_t i = 0; i < state->num_regions; i++)
    {
        memory_region_t *region = &state->regions[i];

        // Escrever o cabeçalho da região
        region_header_t region_header;
        strncpy(region_header.name, region->name, SAVE_STATE_MAX_NAME_LEN);
        region_header.size = region->size;

        if (fwrite(&region_header, sizeof(region_header), 1, file) != 1)
        {
            fclose(file);
            return SAVE_STATE_ERROR;
        }

        // Escrever os dados da região
        if (fwrite(region->memory, 1, region->size, file) != region->size)
        {
            fclose(file);
            return SAVE_STATE_ERROR;
        }
    }

    fclose(file);
    EMU_LOG_INFO(EMU_LOG_CAT_SAVE_STATE, "Estado salvo em '%s', %d regiões", filename, state->num_regions);
    return SAVE_STATE_OK;
}

save_state_result_t save_state_load(save_state_t *state, const char *filename)
{
    if (!state || !filename)
    {
        return SAVE_STATE_ERROR;
    }

    // Abrir o arquivo para leitura
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        return SAVE_STATE_ERROR;
    }

    // Ler o cabeçalho
    save_state_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        fclose(file);
        return SAVE_STATE_ERROR;
    }

    // Verificar a assinatura
    if (strncmp(header.signature, SAVE_STATE_SIGNATURE, sizeof(header.signature)) != 0)
    {
        fclose(file);
        return SAVE_STATE_ERROR;
    }

    // Verificar a versão
    if (header.version != SAVE_STATE_VERSION)
    {
        fclose(file);
        return SAVE_STATE_ERROR;
    }

    // Ler metadados
    metadata_item_t temp_metadata[SAVE_STATE_MAX_METADATA];
    uint32_t num_metadata = header.num_metadata;

    if (num_metadata > SAVE_STATE_MAX_METADATA)
    {
        fclose(file);
        return SAVE_STATE_ERROR;
    }

    for (uint32_t i = 0; i < num_metadata; i++)
    {
        if (fread(&temp_metadata[i], sizeof(metadata_item_t), 1, file) != 1)
        {
            fclose(file);
            return SAVE_STATE_ERROR;
        }
    }

    // Ler as regiões de memória
    for (uint32_t i = 0; i < header.num_regions; i++)
    {
        // Ler o cabeçalho da região
        region_header_t region_header;
        if (fread(&region_header, sizeof(region_header), 1, file) != 1)
        {
            fclose(file);
            return SAVE_STATE_ERROR;
        }

        // Encontrar a região correspondente
        int region_index = find_region_index(state, region_header.name);
        if (region_index < 0)
        {
            // Região não encontrada, pular
            fseek(file, region_header.size, SEEK_CUR);
            continue;
        }

        memory_region_t *region = &state->regions[region_index];

        // Verificar tamanho
        if (region->size != region_header.size)
        {
            fclose(file);
            return SAVE_STATE_ERROR;
        }

        // Ler os dados da região
        if (fread(region->memory, 1, region->size, file) != region->size)
        {
            fclose(file);
            return SAVE_STATE_ERROR;
        }
    }

    // Atualizar metadados
    for (uint32_t i = 0; i < num_metadata; i++)
    {
        state->metadata[i] = temp_metadata[i];
    }
    state->num_metadata = num_metadata;

    fclose(file);
    EMU_LOG_INFO(EMU_LOG_CAT_SAVE_STATE, "Estado carregado de '%s', %d regiões", filename, state->num_regions);
    return SAVE_STATE_OK;
}

save_state_result_t save_state_set_metadata(save_state_t *state, const char *key, const char *value)
{
    if (!state || !key || !value)
    {
        return SAVE_STATE_ERROR;
    }

    // Verificar se já existe um metadado com esta chave
    int index = find_metadata_index(state, key);
    if (index >= 0)
    {
        // Atualizar valor
        strncpy(state->metadata[index].value, value, SAVE_STATE_MAX_VALUE_LEN - 1);
        state->metadata[index].value[SAVE_STATE_MAX_VALUE_LEN - 1] = '\0';
        return SAVE_STATE_OK;
    }

    // Verificar se já atingimos o número máximo de metadados
    if (state->num_metadata >= SAVE_STATE_MAX_METADATA)
    {
        return SAVE_STATE_ERROR;
    }

    // Adicionar novo metadado
    metadata_item_t *item = &state->metadata[state->num_metadata];
    strncpy(item->key, key, SAVE_STATE_MAX_NAME_LEN - 1);
    item->key[SAVE_STATE_MAX_NAME_LEN - 1] = '\0';
    strncpy(item->value, value, SAVE_STATE_MAX_VALUE_LEN - 1);
    item->value[SAVE_STATE_MAX_VALUE_LEN - 1] = '\0';
    state->num_metadata++;

    return SAVE_STATE_OK;
}

save_state_result_t save_state_get_metadata(save_state_t *state, const char *key, char *value, uint32_t max_len)
{
    if (!state || !key)
    {
        return SAVE_STATE_ERROR;
    }

    // Encontrar o metadado
    int index = find_metadata_index(state, key);
    if (index < 0)
    {
        return SAVE_STATE_ERROR;
    }

    // Se value for NULL, apenas verificamos a existência
    if (value)
    {
        strncpy(value, state->metadata[index].value, max_len - 1);
        value[max_len - 1] = '\0';
    }

    return SAVE_STATE_OK;
}
