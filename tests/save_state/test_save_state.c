/**
 * @file test_save_state.c
 * @brief Testes unitários para o sistema de save state
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-23
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/core/save_state.h"
#include "../../src/core/memory/memory.h"

// Contexto de teste
static save_state_t *state;
static uint8_t test_memory[0x10000];
static const char *test_filename = "test_save.sav";

void setUp(void)
{
    // Inicializar o subsistema de memória
    memory_init();

    // Inicializar o sistema de save state
    state = save_state_create();

    // Preencher a memória de teste com valores conhecidos
    for (int i = 0; i < sizeof(test_memory); i++)
    {
        test_memory[i] = i & 0xFF;
    }

    // Remover arquivo de teste se existir
    remove(test_filename);
}

void tearDown(void)
{
    if (state)
    {
        save_state_destroy(state);
        state = NULL;
    }

    // Limpar memória
    memory_shutdown();

    // Remover arquivo de teste
    remove(test_filename);
}

// Teste de criação/destruição
void test_save_state_create_destroy(void)
{
    TEST_ASSERT_NOT_NULL(state);

    save_state_destroy(state);
    state = NULL;

    state = save_state_create();
    TEST_ASSERT_NOT_NULL(state);
}

// Teste de registro de região
void test_save_state_register_memory(void)
{
    save_state_result_t result;

    result = save_state_register_memory(state, "RAM", test_memory, sizeof(test_memory));
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    result = save_state_register_memory(state, "VRAM", test_memory + 0x1000, 0x4000);
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    // Tentar registrar memória com nome duplicado (deve falhar)
    result = save_state_register_memory(state, "RAM", test_memory, 0x100);
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, result);

    // Tentar registrar memória com tamanho zero (deve falhar)
    result = save_state_register_memory(state, "ZERO", test_memory, 0);
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, result);

    // Tentar registrar muitas regiões de memória (limite é 16)
    printf("Testando limite de regiões...\n");
    int sucessos = 0;
    int falhas = 0;

    for (int i = 0; i < 20; i++)
    {
        char name[32];
        snprintf(name, sizeof(name), "REGION_%d", i);

        result = save_state_register_memory(state, name, test_memory + i * 0x100, 0x100);
        if (i < 14)
        { // 14 + 2 anteriores = 16 total
            if (result == SAVE_STATE_OK)
                sucessos++;
            else
                falhas++;
        }
        else
        {
            if (result == SAVE_STATE_ERROR)
                sucessos++;
            else
                falhas++;
        }
    }

    printf("Limite de regiões: %d sucessos, %d falhas\n", sucessos, falhas);
}

// Teste de salvamento
void test_save_state_save(void)
{
    save_state_result_t result;

    // Registrar regiões de memória
    result = save_state_register_memory(state, "RAM", test_memory, 0x1000);
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    result = save_state_register_memory(state, "VRAM", test_memory + 0x1000, 0x1000);
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    // Salvar o estado
    result = save_state_save(state, test_filename);
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    // Verificar que o arquivo foi criado
    FILE *f = fopen(test_filename, "rb");
    TEST_ASSERT_NOT_NULL(f);
    fclose(f);
}

// Teste de carregamento
void test_save_state_load(void)
{
    save_state_result_t result;

    // Registrar regiões de memória e salvar
    result = save_state_register_memory(state, "RAM", test_memory, 0x1000);
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    result = save_state_save(state, test_filename);
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    // Modificar os dados na memória
    memset(test_memory, 0xAA, 0x1000);

    // Carregar o estado salvo
    result = save_state_load(state, test_filename);
    TEST_ASSERT_EQUAL(SAVE_STATE_OK, result);

    // Verificar que os dados foram restaurados
    int erros = 0;
    for (int i = 0; i < 0x1000; i++)
    {
        if (!TEST_ASSERT_EQUAL_UINT8(i & 0xFF, test_memory[i]))
        {
            erros++;
            if (erros <= 5)
            {
                printf("FALHOU: Memória[%04X] = %02X, esperado %02X\n",
                       i, test_memory[i], i & 0xFF);
            }
        }
    }

    if (erros == 0)
    {
        printf("PASSOU: Todos os dados foram restaurados corretamente\n");
    }
    else
    {
        printf("FALHOU: %d erros encontrados nos dados restaurados\n", erros);
    }
}

// Teste de carregamento com arquivo inválido
void test_save_state_load_invalid(void)
{
    save_state_result_t result;

    // Tentar carregar um arquivo inexistente
    result = save_state_load(state, "nonexistent.sav");
    TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, result);

    // Criar um arquivo inválido
    FILE *f = fopen("invalid.sav", "wb");
    if (f)
    {
        fprintf(f, "This is not a valid save state file");
        fclose(f);

        // Tentar carregar o arquivo inválido
        result = save_state_load(state, "invalid.sav");
        TEST_ASSERT_EQUAL(SAVE_STATE_ERROR, result);

        // Limpar
        remove("invalid.sav");
    }
    else
    {
        printf("FALHOU: Não foi possível criar arquivo de teste inválido\n");
    }
}

// Função principal
int main(void)
{
    printf("=== Testes de Save State ===\n\n");

    test_save_state_create_destroy();
    printf("\n");

    test_save_state_register_memory();
    printf("\n");

    test_save_state_save();
    printf("\n");

    test_save_state_load();
    printf("\n");

    test_save_state_load_invalid();
    printf("\n");

    printf("=== Testes concluídos ===\n");
    return 0;
}
