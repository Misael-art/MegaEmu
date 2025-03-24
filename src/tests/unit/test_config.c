/**
 * @file test_config.c
 * @brief Testes unitários para o sistema de configuração
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/config/config_interface.h"

// Variáveis para testes
static int g_callback_called = 0;
static char g_callback_key[128] = {0};
static emu_config_value_t g_callback_value = {0};

// Callback de teste
static void test_config_callback(const char *key, const emu_config_value_t *value, void *userdata)
{
    g_callback_called = 1;
    strncpy(g_callback_key, key, sizeof(g_callback_key) - 1);

    // Copiar valor
    if (value)
    {
        g_callback_value.type = value->type;
        switch (value->type)
        {
        case EMU_CONFIG_TYPE_INT:
            g_callback_value.int_value = value->int_value;
            break;
        case EMU_CONFIG_TYPE_FLOAT:
            g_callback_value.float_value = value->float_value;
            break;
        case EMU_CONFIG_TYPE_BOOL:
            g_callback_value.bool_value = value->bool_value;
            break;
        case EMU_CONFIG_TYPE_STRING:
            if (g_callback_value.type == EMU_CONFIG_TYPE_STRING && g_callback_value.string_value)
            {
                free(g_callback_value.string_value);
            }
            if (value->string_value)
            {
                g_callback_value.string_value = strdup(value->string_value);
            }
            else
            {
                g_callback_value.string_value = NULL;
            }
            break;
        default:
            break;
        }
    }
}

void setUp(void)
{
    g_callback_called = 0;
    memset(g_callback_key, 0, sizeof(g_callback_key));
    if (g_callback_value.type == EMU_CONFIG_TYPE_STRING && g_callback_value.string_value)
    {
        free(g_callback_value.string_value);
    }
    memset(&g_callback_value, 0, sizeof(g_callback_value));
}

void tearDown(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    if (config)
    {
        config->shutdown();
    }
    if (g_callback_value.type == EMU_CONFIG_TYPE_STRING && g_callback_value.string_value)
    {
        free(g_callback_value.string_value);
        g_callback_value.string_value = NULL;
    }
}

void test_init(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);

    int result = config->init();
    TEST_ASSERT_EQUAL(0, result);

    // Tentar inicializar novamente
    result = config->init();
    TEST_ASSERT_EQUAL(0, result); // Deve retornar sucesso mesmo já inicializado
}

void test_int_values(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Definir valor
    int result = config->set_int("test.int", 42);
    TEST_ASSERT_EQUAL(0, result);

    // Obter valor
    int64_t value = 0;
    result = config->get_int("test.int", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(42, value);

    // Tentar obter valor inexistente
    result = config->get_int("test.nonexistent", &value);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar obter valor de tipo incorreto
    config->set_bool("test.bool", 1);
    result = config->get_int("test.bool", &value);
    TEST_ASSERT_EQUAL(-1, result);
}

void test_float_values(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Definir valor
    int result = config->set_float("test.float", 3.14159);
    TEST_ASSERT_EQUAL(0, result);

    // Obter valor
    double value = 0;
    result = config->get_float("test.float", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_DOUBLE(3.14159, value);

    // Tentar obter valor inexistente
    result = config->get_float("test.nonexistent", &value);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar obter valor de tipo incorreto
    config->set_int("test.int", 42);
    result = config->get_float("test.int", &value);
    TEST_ASSERT_EQUAL(-1, result);
}

void test_bool_values(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Definir valor
    int result = config->set_bool("test.bool", 1);
    TEST_ASSERT_EQUAL(0, result);

    // Obter valor
    int value = 0;
    result = config->get_bool("test.bool", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(value);

    // Definir valor falso
    result = config->set_bool("test.bool", 0);
    TEST_ASSERT_EQUAL(0, result);

    // Obter valor
    result = config->get_bool("test.bool", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_FALSE(value);

    // Tentar obter valor inexistente
    result = config->get_bool("test.nonexistent", &value);
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar obter valor de tipo incorreto
    config->set_int("test.int", 42);
    result = config->get_bool("test.int", &value);
    TEST_ASSERT_EQUAL(-1, result);
}

void test_string_values(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Definir valor
    int result = config->set_string("test.string", "Hello World");
    TEST_ASSERT_EQUAL(0, result);

    // Obter valor
    char value[128];
    result = config->get_string("test.string", value, sizeof(value));
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello World", value);

    // Tentar obter valor inexistente
    result = config->get_string("test.nonexistent", value, sizeof(value));
    TEST_ASSERT_EQUAL(-1, result);

    // Tentar obter valor de tipo incorreto
    config->set_int("test.int", 42);
    result = config->get_string("test.int", value, sizeof(value));
    TEST_ASSERT_EQUAL(-1, result);
}

void test_generic_values(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Definir valores de diferentes tipos
    config->set_int("test.int", 42);
    config->set_float("test.float", 3.14159);
    config->set_bool("test.bool", 1);
    config->set_string("test.string", "Hello World");

    // Obter valores
    emu_config_value_t value;
    int result;

    // Int
    result = config->get_value("test.int", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(EMU_CONFIG_TYPE_INT, value.type);
    TEST_ASSERT_EQUAL(42, value.int_value);

    // Float
    result = config->get_value("test.float", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(EMU_CONFIG_TYPE_FLOAT, value.type);
    TEST_ASSERT_EQUAL_DOUBLE(3.14159, value.float_value);

    // Bool
    result = config->get_value("test.bool", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(EMU_CONFIG_TYPE_BOOL, value.type);
    TEST_ASSERT_TRUE(value.bool_value);

    // String
    result = config->get_value("test.string", &value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(EMU_CONFIG_TYPE_STRING, value.type);
    TEST_ASSERT_EQUAL_STRING("Hello World", value.string_value);
    free(value.string_value); // Liberar string alocada por get_value
}

void test_callbacks(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Registrar callback
    int result = config->register_change_callback("test.int", test_config_callback, NULL);
    TEST_ASSERT_EQUAL(0, result);

    // Definir valor para acionar callback
    g_callback_called = 0;
    result = config->set_int("test.int", 42);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(g_callback_called);
    TEST_ASSERT_EQUAL_STRING("test.int", g_callback_key);
    TEST_ASSERT_EQUAL(EMU_CONFIG_TYPE_INT, g_callback_value.type);
    TEST_ASSERT_EQUAL(42, g_callback_value.int_value);

    // Definir outro valor
    g_callback_called = 0;
    result = config->set_int("test.int", 100);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(g_callback_called);
    TEST_ASSERT_EQUAL(100, g_callback_value.int_value);

    // Definir valor que não deve acionar o callback
    g_callback_called = 0;
    result = config->set_int("test.other", 50);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_FALSE(g_callback_called);

    // Registrar callback para todos os valores
    result = config->register_change_callback("*", test_config_callback, NULL);
    TEST_ASSERT_EQUAL(0, result);

    // Definir valor para acionar callback global
    g_callback_called = 0;
    result = config->set_int("test.other", 75);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(g_callback_called);
    TEST_ASSERT_EQUAL_STRING("test.other", g_callback_key);
    TEST_ASSERT_EQUAL(EMU_CONFIG_TYPE_INT, g_callback_value.type);
    TEST_ASSERT_EQUAL(75, g_callback_value.int_value);

    // Remover callback
    result = config->unregister_change_callback("test.int", test_config_callback);
    TEST_ASSERT_EQUAL(0, result);

    // Definir valor que não deve mais acionar o callback específico
    g_callback_called = 0;
    result = config->set_int("test.int", 200);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(g_callback_called); // Mas ainda deve acionar o callback global

    // Remover callback global
    result = config->unregister_change_callback("*", test_config_callback);
    TEST_ASSERT_EQUAL(0, result);

    // Definir valor que não deve acionar nenhum callback
    g_callback_called = 0;
    result = config->set_int("test.int", 300);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_FALSE(g_callback_called);
}

void test_defaults(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Carregar configurações padrão
    int result = config->load_defaults();
    TEST_ASSERT_EQUAL(0, result);

    // Verificar algumas configurações padrão
    int64_t int_value;
    double float_value;
    char string_value[128];

    // Verificar valores de vídeo
    result = config->get_int("video.width", &int_value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_GREATER_THAN(0, int_value);

    result = config->get_int("video.height", &int_value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_GREATER_THAN(0, int_value);

    // Verificar valores de áudio
    result = config->get_int("audio.sample_rate", &int_value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_GREATER_THAN(0, int_value);

    result = config->get_float("audio.volume", &float_value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 1.0, float_value);

    // Verificar caminhos
    result = config->get_string("paths.roms", string_value, sizeof(string_value));
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_GREATER_THAN(0, strlen(string_value));
}

void test_file_operations(void)
{
    const emu_config_interface_t *config = emu_config_get_interface();
    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_EQUAL(0, config->init());

    // Definir alguns valores
    config->set_int("test.int", 42);
    config->set_float("test.float", 3.14159);
    config->set_bool("test.bool", 1);
    config->set_string("test.string", "Hello World");

    // Salvar para arquivo
    int result = config->save_to_file("test_config.cfg");
    TEST_ASSERT_EQUAL(0, result);

    // Limpar configurações
    config->shutdown();
    config->init();

    // Carregar do arquivo
    result = config->load_from_file("test_config.cfg");
    TEST_ASSERT_EQUAL(0, result);

    // Verificar valores
    int64_t int_value;
    double float_value;
    int bool_value;
    char string_value[128];

    result = config->get_int("test.int", &int_value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(42, int_value);

    result = config->get_float("test.float", &float_value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_DOUBLE(3.14159, float_value);

    result = config->get_bool("test.bool", &bool_value);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(bool_value);

    result = config->get_string("test.string", string_value, sizeof(string_value));
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello World", string_value);

    // Limpar
    remove("test_config.cfg");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init);
    RUN_TEST(test_int_values);
    RUN_TEST(test_float_values);
    RUN_TEST(test_bool_values);
    RUN_TEST(test_string_values);
    RUN_TEST(test_generic_values);
    RUN_TEST(test_callbacks);
    RUN_TEST(test_defaults);
    RUN_TEST(test_file_operations);

    return UNITY_END();
}
