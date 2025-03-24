/**
 * @file nes_save_state_crypto_tests.c
 * @brief Testes unitários para criptografia AES-256 nos save states do NES
 * @version 1.0
 * @date 2025-04-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../test_framework/unity.h"
#include "../platforms/nes/save/nes_save_state.h"
#include "../platforms/nes/save/nes_save_state_crypto.h"
#include "../core/save_state.h"
#include "../core/save_state_crypto.h"
#include "../utils/file_utils.h"
#include "../utils/crypto/crypto_common.h"

/* Diretório temporário para testes */
#define TEST_DIR "./test_tmp/nes_crypto_tests"
#define TEST_BUFFER_SIZE 4096
#define SMALL_BUFFER_SIZE 256

/* Dados de teste */
static uint8_t test_data[TEST_BUFFER_SIZE];
static uint8_t encrypted_data[TEST_BUFFER_SIZE + 128]; /* Buffer extra para tags e IVs */
static uint8_t decrypted_data[TEST_BUFFER_SIZE];

/* Configuração de teste */
static emu_save_state_t test_state;
static emu_crypto_config_t crypto_config;
static const char *test_password = "TesteSenhaForte123!@#";
static char test_save_path[256];
static char test_key_path[256];

/* Configuração/inicialização para testes */
void setUp(void)
{
    /* Criar diretório de teste se não existir */
    create_directory(TEST_DIR);

    /* Inicializar dados de teste com padrão pseudo-aleatório */
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        test_data[i] = (uint8_t)((i * 17) % 256);
    }

    /* Limpar buffers */
    memset(encrypted_data, 0, sizeof(encrypted_data));
    memset(decrypted_data, 0, sizeof(decrypted_data));

    /* Configurar caminhos de teste */
    snprintf(test_save_path, sizeof(test_save_path), "%s/test_save.state", TEST_DIR);
    snprintf(test_key_path, sizeof(test_key_path), "%s/test_key.key", TEST_DIR);

    /* Inicializar contexto de save state */
    memset(&test_state, 0, sizeof(test_state));
    emu_save_state_init(&test_state);

    /* Inicializar subsistema de criptografia */
    emu_crypto_initialize();
}

/* Limpeza após testes */
void tearDown(void)
{
    /* Limpar recursos */
    emu_save_state_cleanup(&test_state);

    /* Remover arquivos de teste */
    remove_file(test_save_path);
    remove_file(test_key_path);
}

/* Testar a detecção de criptografia em arquivos */
void test_nes_save_state_encryption_detection(void)
{
    /* Criar um save state não criptografado */
    FILE *f = fopen(test_save_path, "wb");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Falha ao criar arquivo de teste");

    /* Gravar cabeçalho e alguns dados */
    const char header[] = "NES_SAVE_STATE_1.0";
    fwrite(header, 1, strlen(header), f);
    fwrite(test_data, 1, 100, f);
    fclose(f);

    /* Verificar que não está criptografado */
    TEST_ASSERT_FALSE(nes_save_state_is_encrypted(test_save_path));

    /* Habilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, test_password));

    /* Salvar estado com criptografia */
    test_state.filepath = test_save_path;
    test_state.buffer = test_data;
    test_state.buffer_size = TEST_BUFFER_SIZE;
    TEST_ASSERT_TRUE(emu_save_state_save(&test_state));

    /* Verificar que está criptografado */
    TEST_ASSERT_TRUE(nes_save_state_is_encrypted(test_save_path));
}

/* Testar a habilitação e desabilitação de criptografia */
void test_nes_save_state_crypto_enable_disable(void)
{
    /* Verificar estado inicial */
    TEST_ASSERT_FALSE(test_state.crypto_enabled);

    /* Habilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, test_password));
    TEST_ASSERT_TRUE(test_state.crypto_enabled);
    TEST_ASSERT_NOT_NULL(test_state.crypto_config);

    /* Verificar configuração */
    crypto_config_t *config = (crypto_config_t *)test_state.crypto_config;
    TEST_ASSERT_EQUAL(CRYPTO_ALG_AES_256_GCM, config->algorithm);

    /* Desabilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_disable_encryption(&test_state));
    TEST_ASSERT_FALSE(test_state.crypto_enabled);
}

/* Testar salvamento e carregamento com criptografia */
void test_nes_save_state_crypto_save_load(void)
{
    /* Preparar dados de teste */
    test_state.buffer = test_data;
    test_state.buffer_size = TEST_BUFFER_SIZE;
    test_state.filepath = test_save_path;

    /* Habilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, test_password));

    /* Salvar estado */
    TEST_ASSERT_TRUE(emu_save_state_save(&test_state));

    /* Limpar buffer para teste */
    uint8_t *new_buffer = malloc(TEST_BUFFER_SIZE);
    TEST_ASSERT_NOT_NULL(new_buffer);
    memset(new_buffer, 0, TEST_BUFFER_SIZE);

    /* Configurar load */
    test_state.buffer = new_buffer;
    test_state.buffer_size = TEST_BUFFER_SIZE;

    /* Carregar estado */
    TEST_ASSERT_TRUE(emu_save_state_load(&test_state));

    /* Verificar dados carregados */
    TEST_ASSERT_EQUAL_MEMORY(test_data, new_buffer, TEST_BUFFER_SIZE);

    /* Limpar */
    free(new_buffer);
}

/* Testar exportação e importação de chaves */
void test_nes_save_state_crypto_key_export_import(void)
{
    /* Habilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, test_password));

    /* Exportar chave */
    TEST_ASSERT_TRUE(nes_save_state_export_key(&test_state, test_key_path, "KeyPassword123"));

    /* Desabilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_disable_encryption(&test_state));
    TEST_ASSERT_FALSE(test_state.crypto_enabled);

    /* Importar chave */
    TEST_ASSERT_TRUE(nes_save_state_import_key(&test_state, test_key_path, "KeyPassword123"));
    TEST_ASSERT_TRUE(test_state.crypto_enabled);
}

/* Testar tentativa de importação com senha incorreta */
void test_nes_save_state_crypto_key_wrong_password(void)
{
    /* Habilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, test_password));

    /* Exportar chave */
    TEST_ASSERT_TRUE(nes_save_state_export_key(&test_state, test_key_path, "KeyPassword123"));

    /* Desabilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_disable_encryption(&test_state));

    /* Tentar importar chave com senha incorreta */
    TEST_ASSERT_FALSE(nes_save_state_import_key(&test_state, test_key_path, "SenhaIncorreta"));
    TEST_ASSERT_FALSE(test_state.crypto_enabled);
}

/* Testar mudança de senha */
void test_nes_save_state_crypto_change_password(void)
{
    /* Habilitar criptografia */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, test_password));

    /* Preparar para salvar */
    test_state.buffer = test_data;
    test_state.buffer_size = TEST_BUFFER_SIZE;
    test_state.filepath = test_save_path;

    /* Salvar com a senha atual */
    TEST_ASSERT_TRUE(emu_save_state_save(&test_state));

    /* Mudar para nova senha */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, "NovaSenha456!@#"));

    /* Reservar novo buffer */
    uint8_t *new_buffer = malloc(TEST_BUFFER_SIZE);
    TEST_ASSERT_NOT_NULL(new_buffer);
    memset(new_buffer, 0, TEST_BUFFER_SIZE);
    test_state.buffer = new_buffer;

    /* Carregar com nova senha */
    TEST_ASSERT_TRUE(emu_save_state_load(&test_state));

    /* Verificar dados */
    TEST_ASSERT_EQUAL_MEMORY(test_data, new_buffer, TEST_BUFFER_SIZE);

    /* Limpar */
    free(new_buffer);
}

/* Testar integração com sistema de cloud */
void test_nes_save_state_crypto_with_cloud(void)
{
    /* Este teste só verifica a interoperabilidade básica,
       já que os testes completos de cloud estão em outro arquivo */

    /* Simular estruturas de cloud */
    extern bool nes_save_state_enable_cloud(emu_save_state_t *state,
                                         emu_cloud_provider_t provider,
                                         const char *auth_token,
                                         bool auto_sync);

    /* Habilitar criptografia primeiro */
    TEST_ASSERT_TRUE(nes_save_state_enable_encryption(&test_state, test_password));

    /* Agora habilitar cloud - isso não deve desabilitar a criptografia */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, 1, "dummy_token", false));

    /* Verificar que a criptografia permanece ativa */
    TEST_ASSERT_TRUE(test_state.crypto_enabled);
}

/* Função principal dos testes */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_nes_save_state_encryption_detection);
    RUN_TEST(test_nes_save_state_crypto_enable_disable);
    RUN_TEST(test_nes_save_state_crypto_save_load);
    RUN_TEST(test_nes_save_state_crypto_key_export_import);
    RUN_TEST(test_nes_save_state_crypto_key_wrong_password);
    RUN_TEST(test_nes_save_state_crypto_change_password);
    RUN_TEST(test_nes_save_state_crypto_with_cloud);

    return UNITY_END();
}
