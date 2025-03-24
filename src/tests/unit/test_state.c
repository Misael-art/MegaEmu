/**
 * @file test_state.c
 * @brief Testes unitários para o sistema de estado
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/state/state_interface.h"

// Variáveis para testes
static int g_progress_callback_called = 0;
static int g_progress_percentage = 0;
static char g_progress_message[256] = {0};
static int g_rom_verify_callback_called = 0;
static char g_rom_hash[64] = {0};
static int g_rom_verify_result = 1;

// Callback de progresso para testes
static void test_progress_callback(int percentage, const char *message, void *userdata)
{
    g_progress_callback_called = 1;
    g_progress_percentage = percentage;
    if (message)
    {
        strncpy(g_progress_message, message, sizeof(g_progress_message) - 1);
    }
    else
    {
        g_progress_message[0] = '\0';
    }
}

// Callback de verificação de ROM para testes
static int test_rom_verify_callback(const char *rom_hash, void *userdata)
{
    g_rom_verify_callback_called = 1;
    if (rom_hash)
    {
        strncpy(g_rom_hash, rom_hash, sizeof(g_rom_hash) - 1);
    }
    else
    {
        g_rom_hash[0] = '\0';
    }
    return g_rom_verify_result;
}

void setUp(void)
{
    g_progress_callback_called = 0;
    g_progress_percentage = 0;
    memset(g_progress_message, 0, sizeof(g_progress_message));
    g_rom_verify_callback_called = 0;
    memset(g_rom_hash, 0, sizeof(g_rom_hash));
    g_rom_verify_result = 1;
}

void tearDown(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    if (state)
    {
        state->shutdown();
    }
}

void test_init(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);

    int result = state->init();
    TEST_ASSERT_EQUAL(0, result);

    // Tentar inicializar novamente
    result = state->init();
    TEST_ASSERT_EQUAL(0, result); // Deve retornar sucesso mesmo já inicializado
}

void test_state_slot(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL(0, state->init());

    // Configurar callback de progresso
    int result = state->set_progress_callback(test_progress_callback, NULL);
    TEST_ASSERT_EQUAL(0, result);

    // Tentar salvar em slot sem plataforma (deve falhar)
    result = state->save_state(0, "Teste");
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar carregar de slot sem plataforma (deve falhar)
    result = state->load_state(0);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar usar slot inválido (deve falhar)
    result = state->save_state(-1, "Teste");
    TEST_ASSERT_EQUAL(-1, result);
    result = state->save_state(100, "Teste");
    TEST_ASSERT_EQUAL(-1, result);
}

void test_state_file(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL(0, state->init());

    // Configurar callback de progresso
    int result = state->set_progress_callback(test_progress_callback, NULL);
    TEST_ASSERT_EQUAL(0, result);

    // Tentar salvar para arquivo sem plataforma (deve falhar)
    result = state->save_state_to_file("test_state.dat", "Teste");
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar carregar de arquivo sem plataforma (deve falhar)
    result = state->load_state_from_file("test_state.dat");
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar carregar arquivo inexistente (deve falhar)
    result = state->load_state_from_file("nonexistent_file.dat");
    TEST_ASSERT_EQUAL(-1, result);
}

void test_snapshots(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL(0, state->init());

    // Tentar criar snapshot sem plataforma (deve falhar)
    int result = state->create_snapshot();
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar restaurar snapshot inexistente (deve falhar)
    result = state->restore_snapshot(0);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar restaurar snapshot com ID inválido (deve falhar)
    result = state->restore_snapshot(-1);
    TEST_ASSERT_EQUAL(-1, result);
    result = state->restore_snapshot(100);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar excluir snapshot inexistente (deve falhar)
    result = state->delete_snapshot(0);
    TEST_ASSERT_EQUAL(-1, result);

    // Verificar contagem de snapshots
    int count = state->get_snapshot_count();
    TEST_ASSERT_EQUAL(0, count);
}

void test_reset(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL(0, state->init());

    // Tentar reset sem plataforma (deve falhar)
    int result = state->reset(EMU_STATE_TYPE_RESET);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar reset com tipo inválido (deve falhar)
    result = state->reset(EMU_STATE_TYPE_MAX);
    TEST_ASSERT_EQUAL(-1, result);
}

void test_rewind(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL(0, state->init());

    // Tentar rebobinar sem habilitar (deve falhar)
    int result = state->rewind(10);
    TEST_ASSERT_EQUAL(-1, result);

    // Habilitar rebobinagem
    result = state->enable_rewind(1);
    TEST_ASSERT_EQUAL(0, result);

    // Tentar rebobinar sem plataforma (deve falhar)
    result = state->rewind(10);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar rebobinar com frames inválidos (deve falhar)
    result = state->rewind(0);
    TEST_ASSERT_EQUAL(-1, result);
    result = state->rewind(-10);
    TEST_ASSERT_EQUAL(-1, result);

    // Configurar buffer de rebobinagem
    result = state->set_rewind_buffer_frames(120);
    TEST_ASSERT_EQUAL(0, result);

    // Desabilitar rebobinagem
    result = state->enable_rewind(0);
    TEST_ASSERT_EQUAL(0, result);
}

void test_callbacks(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL(0, state->init());

    // Registrar callback de progresso
    int result = state->set_progress_callback(test_progress_callback, NULL);
    TEST_ASSERT_EQUAL(0, result);

    // Registrar callback de verificação de ROM
    result = state->set_rom_verify_callback(test_rom_verify_callback, NULL);
    TEST_ASSERT_EQUAL(0, result);
}

void test_autosave_config(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL(0, state->init());

    // Configurar intervalo de autosave
    int result = state->set_autosave_interval(60);
    TEST_ASSERT_EQUAL(0, result);

    // Tentar configurar intervalo inválido (deve falhar)
    result = state->set_autosave_interval(-1);
    TEST_ASSERT_EQUAL(-1, result);
}

void test_error_messages(void)
{
    const emu_state_interface_t *state = emu_state_get_interface();
    TEST_ASSERT_NOT_NULL(state);

    // Verificar se todas as mensagens de erro estão definidas
    for (int i = 0; i < EMU_STATE_ERROR_MAX; i++)
    {
        const char *message = state->get_error_string((emu_state_error_t)i);
        TEST_ASSERT_NOT_NULL(message);
        TEST_ASSERT_GREATER_THAN(0, strlen(message));
    }
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init);
    RUN_TEST(test_state_slot);
    RUN_TEST(test_state_file);
    RUN_TEST(test_snapshots);
    RUN_TEST(test_reset);
    RUN_TEST(test_rewind);
    RUN_TEST(test_callbacks);
    RUN_TEST(test_autosave_config);
    RUN_TEST(test_error_messages);

    return UNITY_END();
}
