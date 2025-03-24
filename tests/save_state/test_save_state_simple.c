#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definições simplificadas para teste
#define TEST_PASSED() printf("TESTE PASSOU!\n")
#define TEST_FAILED() printf("TESTE FALHOU!\n")
#define ASSERT(condition) do { if (!(condition)) { printf("Falha na linha %d\n", __LINE__); TEST_FAILED(); return 1; } } while(0)

// Inclusão das dependências para o teste
#include "../../src/core/save_state.h"
#include "../../src/core/memory/memory.h"

// Buffer de teste para simular memória
static unsigned char test_buffer[1024];

int test_save_state_creation() {
    printf("Testando criação do save state...\n");

    save_state_t* state = save_state_create();
    ASSERT(state != NULL);

    save_state_destroy(state);
    TEST_PASSED();
    return 0;
}

int test_save_state_register_memory() {
    printf("Testando registro de memória...\n");

    save_state_t* state = save_state_create();
    ASSERT(state != NULL);

    memset(test_buffer, 0xAA, sizeof(test_buffer));

    int result = save_state_register_memory(state, "TEST_REGION", test_buffer, sizeof(test_buffer));
    ASSERT(result == SAVE_STATE_OK);

    save_state_destroy(state);
    TEST_PASSED();
    return 0;
}

int test_save_state_save_load() {
    printf("Testando salvamento e carregamento...\n");

    save_state_t* state = save_state_create();
    ASSERT(state != NULL);

    // Preencher o buffer com um padrão conhecido
    memset(test_buffer, 0xAA, sizeof(test_buffer));

    // Registrar região de memória
    int result = save_state_register_memory(state, "TEST_REGION", test_buffer, sizeof(test_buffer));
    ASSERT(result == SAVE_STATE_OK);

    // Salvar o estado
    result = save_state_save(state, "test_save.bin");
    ASSERT(result == SAVE_STATE_OK);

    // Modificar o buffer
    memset(test_buffer, 0xBB, sizeof(test_buffer));

    // Verificar que o buffer foi modificado
    ASSERT(test_buffer[0] == 0xBB);

    // Carregar o estado
    result = save_state_load(state, "test_save.bin");
    ASSERT(result == SAVE_STATE_OK);

    // Verificar que o buffer foi restaurado ao valor original
    ASSERT(test_buffer[0] == 0xAA);

    save_state_destroy(state);
    TEST_PASSED();
    return 0;
}

int main() {
    printf("============ TESTES DE SAVE STATE ============\n");

    // Inicializar o subsistema de memória
    if (memory_init() != 0) {
        printf("Falha ao inicializar o subsistema de memória\n");
        return 1;
    }

    int failed = 0;

    failed += test_save_state_creation();
    failed += test_save_state_register_memory();
    failed += test_save_state_save_load();

    // Desligar o subsistema de memória
    memory_shutdown();

    printf("============ RESULTADOS DOS TESTES ============\n");
    if (failed == 0) {
        printf("TODOS OS TESTES PASSARAM!\n");
        return 0;
    } else {
        printf("%d TESTES FALHARAM!\n", failed);
        return 1;
    }
}
