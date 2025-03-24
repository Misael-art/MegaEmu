/**
 * @file test_save_state_standalone.c
 * @brief Testes independentes para o sistema de save state
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-23
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Implementações simplificadas
#define MEMORIA_TESTE_TAMANHO 256
static uint8_t memoria_teste[MEMORIA_TESTE_TAMANHO];
static save_state_t *state = NULL;
static const char *test_filename = "test_save.sav";

// Implementações simplificadas para memory.h
int memory_init(void)
{
    return 0;
}

int memory_shutdown(void)
{
    return 0;
}

void *memory_alloc(uint32_t size)
{
    return malloc(size);
}

void memory_free(void *ptr)
{
    free(ptr);
}

void memory_clear(void *ptr, uint32_t size)
{
    memset(ptr, 0, size);
}

// Tipos simplificados para save_state.h
typedef enum
{
    SAVE_STATE_OK = 0,
    SAVE_STATE_ERROR = -1
} save_state_result_t;

#define MAX_REGIOES 10
#define MAX_NAME_LENGTH 64

typedef struct
{
    char nome[MAX_NAME_LENGTH];
    void *memoria;
    uint32_t tamanho;
} memoria_regiao_t;

typedef struct
{
    memoria_regiao_t regioes[MAX_REGIOES];
    int num_regioes;
    char metadados_chave[MAX_REGIOES][MAX_NAME_LENGTH];
    char metadados_valor[MAX_REGIOES][MAX_NAME_LENGTH];
    int num_metadados;
} save_state_t;

// Implementações simplificadas das funções do save_state
save_state_t *save_state_create(void)
{
    save_state_t *state = (save_state_t *)memory_alloc(sizeof(save_state_t));
    if (state)
    {
        state->num_regioes = 0;
        state->num_metadados = 0;
    }
    return state;
}

void save_state_destroy(save_state_t *state)
{
    if (state)
    {
        memory_free(state);
    }
}

save_state_result_t save_state_register_memory(save_state_t *state, const char *name, void *memory, uint32_t size)
{
    if (!state || !name || !memory || state->num_regioes >= MAX_REGIOES)
    {
        return SAVE_STATE_ERROR;
    }

    int idx = state->num_regioes++;
    strncpy(state->regioes[idx].nome, name, MAX_NAME_LENGTH - 1);
    state->regioes[idx].memoria = memory;
    state->regioes[idx].tamanho = size;

    return SAVE_STATE_OK;
}

save_state_result_t save_state_save(save_state_t *state, const char *filename)
{
    if (!state || !filename)
    {
        return SAVE_STATE_ERROR;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        return SAVE_STATE_ERROR;
    }

    // Salvar número de regiões
    fwrite(&state->num_regioes, sizeof(int), 1, fp);

    // Salvar cada região
    for (int i = 0; i < state->num_regioes; i++)
    {
        fwrite(state->regioes[i].nome, MAX_NAME_LENGTH, 1, fp);
        fwrite(&state->regioes[i].tamanho, sizeof(uint32_t), 1, fp);
        fwrite(state->regioes[i].memoria, state->regioes[i].tamanho, 1, fp);
    }

    // Salvar metadados
    fwrite(&state->num_metadados, sizeof(int), 1, fp);
    for (int i = 0; i < state->num_metadados; i++)
    {
        fwrite(state->metadados_chave[i], MAX_NAME_LENGTH, 1, fp);
        fwrite(state->metadados_valor[i], MAX_NAME_LENGTH, 1, fp);
    }

    fclose(fp);
    return SAVE_STATE_OK;
}

save_state_result_t save_state_load(save_state_t *state, const char *filename)
{
    if (!state || !filename)
    {
        return SAVE_STATE_ERROR;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        return SAVE_STATE_ERROR;
    }

    // Carregar número de regiões
    int num_regioes;
    if (fread(&num_regioes, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return SAVE_STATE_ERROR;
    }

    // Verificar número de regiões
    if (num_regioes != state->num_regioes)
    {
        fclose(fp);
        return SAVE_STATE_ERROR;
    }

    // Carregar cada região
    for (int i = 0; i < num_regioes; i++)
    {
        char nome[MAX_NAME_LENGTH];
        uint32_t tamanho;

        if (fread(nome, MAX_NAME_LENGTH, 1, fp) != 1 ||
            fread(&tamanho, sizeof(uint32_t), 1, fp) != 1)
        {
            fclose(fp);
            return SAVE_STATE_ERROR;
        }

        // Verificar se a região existe e tem o mesmo tamanho
        int found = 0;
        for (int j = 0; j < state->num_regioes; j++)
        {
            if (strcmp(nome, state->regioes[j].nome) == 0)
            {
                if (tamanho != state->regioes[j].tamanho)
                {
                    fclose(fp);
                    return SAVE_STATE_ERROR;
                }

                if (fread(state->regioes[j].memoria, tamanho, 1, fp) != 1)
                {
                    fclose(fp);
                    return SAVE_STATE_ERROR;
                }

                found = 1;
                break;
            }
        }

        if (!found)
        {
            fclose(fp);
            return SAVE_STATE_ERROR;
        }
    }

    // Carregar metadados
    if (fread(&state->num_metadados, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        return SAVE_STATE_ERROR;
    }

    for (int i = 0; i < state->num_metadados; i++)
    {
        if (fread(state->metadados_chave[i], MAX_NAME_LENGTH, 1, fp) != 1 ||
            fread(state->metadados_valor[i], MAX_NAME_LENGTH, 1, fp) != 1)
        {
            fclose(fp);
            return SAVE_STATE_ERROR;
        }
    }

    fclose(fp);
    return SAVE_STATE_OK;
}

save_state_result_t save_state_set_metadata(save_state_t *state, const char *key, const char *value)
{
    if (!state || !key || !value || state->num_metadados >= MAX_REGIOES)
    {
        return SAVE_STATE_ERROR;
    }

    // Verificar se a chave já existe
    for (int i = 0; i < state->num_metadados; i++)
    {
        if (strcmp(state->metadados_chave[i], key) == 0)
        {
            strncpy(state->metadados_valor[i], value, MAX_NAME_LENGTH - 1);
            return SAVE_STATE_OK;
        }
    }

    // Adicionar novo metadado
    int idx = state->num_metadados++;
    strncpy(state->metadados_chave[idx], key, MAX_NAME_LENGTH - 1);
    strncpy(state->metadados_valor[idx], value, MAX_NAME_LENGTH - 1);

    return SAVE_STATE_OK;
}

save_state_result_t save_state_get_metadata(save_state_t *state, const char *key, char *value, uint32_t max_len)
{
    if (!state || !key || !value || max_len == 0)
    {
        return SAVE_STATE_ERROR;
    }

    for (int i = 0; i < state->num_metadados; i++)
    {
        if (strcmp(state->metadados_chave[i], key) == 0)
        {
            strncpy(value, state->metadados_valor[i], max_len - 1);
            value[max_len - 1] = '\0';
            return SAVE_STATE_OK;
        }
    }

    return SAVE_STATE_ERROR;
}

void setUp(void)
{
    memory_init();
    state = save_state_create();
    memset(memoria_teste, 0xAA, MEMORIA_TESTE_TAMANHO);
    remove(test_filename);
}

void tearDown(void)
{
    if (state)
    {
        save_state_destroy(state);
        state = NULL;
    }
    memory_shutdown();
    remove(test_filename);
}

void test_criar_destruir_save_state(void)
{
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL_INT(0, state->num_regioes);
    TEST_ASSERT_EQUAL_INT(0, state->num_metadados);
}

void test_registrar_memoria(void)
{
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_register_memory(state, "RAM", memoria_teste, MEMORIA_TESTE_TAMANHO));
    TEST_ASSERT_EQUAL_INT(1, state->num_regioes);
    TEST_ASSERT_EQUAL_STRING("RAM", state->regioes[0].nome);
    TEST_ASSERT_EQUAL_PTR(memoria_teste, state->regioes[0].memoria);
    TEST_ASSERT_EQUAL_UINT32(MEMORIA_TESTE_TAMANHO, state->regioes[0].tamanho);

    // Testar registros inválidos
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, save_state_register_memory(NULL, "RAM", memoria_teste, MEMORIA_TESTE_TAMANHO));
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, save_state_register_memory(state, NULL, memoria_teste, MEMORIA_TESTE_TAMANHO));
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, save_state_register_memory(state, "RAM", NULL, MEMORIA_TESTE_TAMANHO));
}

void test_salvar_carregar(void)
{
    // Registrar memória
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_register_memory(state, "RAM", memoria_teste, MEMORIA_TESTE_TAMANHO));

    // Salvar estado
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_save(state, test_filename));

    // Modificar memória
    memset(memoria_teste, 0x55, MEMORIA_TESTE_TAMANHO);

    // Carregar estado
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_load(state, test_filename));

    // Verificar se a memória foi restaurada
    uint8_t expected[MEMORIA_TESTE_TAMANHO];
    memset(expected, 0xAA, MEMORIA_TESTE_TAMANHO);
    TEST_ASSERT_EQUAL_MEMORY(expected, memoria_teste, MEMORIA_TESTE_TAMANHO);

    // Testar carregamento de arquivo inexistente
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, save_state_load(state, "nao_existe.sav"));
}

void test_metadados(void)
{
    char valor[MAX_NAME_LENGTH];

    // Definir metadados
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_set_metadata(state, "versao", "1.0.0"));
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_set_metadata(state, "plataforma", "NES"));

    // Ler metadados
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_get_metadata(state, "versao", valor, MAX_NAME_LENGTH));
    TEST_ASSERT_EQUAL_STRING("1.0.0", valor);

    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_get_metadata(state, "plataforma", valor, MAX_NAME_LENGTH));
    TEST_ASSERT_EQUAL_STRING("NES", valor);

    // Testar chave inexistente
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, save_state_get_metadata(state, "nao_existe", valor, MAX_NAME_LENGTH));

    // Testar atualização de valor
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_set_metadata(state, "versao", "2.0.0"));
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, save_state_get_metadata(state, "versao", valor, MAX_NAME_LENGTH));
    TEST_ASSERT_EQUAL_STRING("2.0.0", valor);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_criar_destruir_save_state);
    RUN_TEST(test_registrar_memoria);
    RUN_TEST(test_salvar_carregar);
    RUN_TEST(test_metadados);
    return UNITY_END();
}
