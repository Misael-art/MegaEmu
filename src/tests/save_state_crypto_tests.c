/**
 * @file save_state_crypto_tests.c
 * @brief Testes unitários para o sistema de criptografia de save states
 * @version 1.0
 * @date 2025-03-18
 *
 * Este arquivo contém testes unitários para validar a implementação
 * de criptografia AES-256 para o sistema de save states.
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "core/save_state.h"
#include "core/save_state_crypto.h"
#include "utils/file_utils.h"
#include "utils/rand_utils.h"

/* Diretório temporário para arquivos de teste */
#define TEST_DIR "./test_temp"

/* Tamanho do buffer de teste */
#define TEST_BUFFER_SIZE 16384

/* Dados para testes */
static uint8_t test_buffer[TEST_BUFFER_SIZE];
static uint8_t encrypted_buffer[TEST_BUFFER_SIZE + 256]; /* Espaço extra para padding */
static uint8_t decrypted_buffer[TEST_BUFFER_SIZE];

/* Configuração de teste */
static emu_encryption_config_t test_config;

/* Contexto de save state para testes */
static emu_save_state_t *test_state = NULL;

void setUp(void)
{
    /* Inicializa o subsistema de criptografia */
    TEST_ASSERT_TRUE(emu_crypto_init());

    /* Cria diretório temporário para testes */
    TEST_ASSERT_TRUE(create_directory(TEST_DIR));

    /* Preenche buffer de teste com dados pseudoaleatórios */
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        test_buffer[i] = (uint8_t)(i & 0xFF);
    }

    /* Inicializa o contexto de save state */
    test_state = emu_save_state_init(EMU_PLATFORM_TEST, test_buffer, 1024);
    TEST_ASSERT_NOT_NULL(test_state);
}

void tearDown(void)
{
    /* Libera o contexto de save state */
    if (test_state)
    {
        emu_save_state_shutdown(test_state);
        test_state = NULL;
    }

    /* Finaliza o subsistema de criptografia */
    emu_crypto_shutdown();

    /* Remove arquivos temporários */
    remove_directory_recursive(TEST_DIR);
}

void test_random_bytes(void)
{
    uint8_t buffer1[32];
    uint8_t buffer2[32];

    /* Gera dois conjuntos de bytes aleatórios */
    TEST_ASSERT_TRUE(emu_generate_random_bytes(buffer1, sizeof(buffer1)));
    TEST_ASSERT_TRUE(emu_generate_random_bytes(buffer2, sizeof(buffer2)));

    /* Verifica se os bytes são diferentes (improvável serem iguais) */
    TEST_ASSERT_NOT_EQUAL_MEMORY(buffer1, buffer2, sizeof(buffer1));

    /* Verifica se há entropia suficiente (contando bytes não nulos) */
    int non_zero1 = 0, non_zero2 = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++)
    {
        if (buffer1[i] != 0)
            non_zero1++;
        if (buffer2[i] != 0)
            non_zero2++;
    }

    TEST_ASSERT_GREATER_THAN(24, non_zero1);
    TEST_ASSERT_GREATER_THAN(24, non_zero2);
}

void test_key_derivation(void)
{
    const char *password = "senha_de_teste_123";
    uint8_t salt[16] = {0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5,
                        0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5};
    uint8_t key1[32];
    uint8_t key2[32];

    /* Deriva uma chave */
    TEST_ASSERT_TRUE(emu_crypto_derive_key(password, strlen(password),
                                           salt, 10000, key1));

    /* Deriva novamente com os mesmos parâmetros - deve dar o mesmo resultado */
    TEST_ASSERT_TRUE(emu_crypto_derive_key(password, strlen(password),
                                           salt, 10000, key2));

    /* As chaves devem ser idênticas quando derivadas com os mesmos parâmetros */
    TEST_ASSERT_EQUAL_MEMORY(key1, key2, sizeof(key1));

    /* Testa com um salt diferente */
    salt[0] = 0xFF;
    TEST_ASSERT_TRUE(emu_crypto_derive_key(password, strlen(password),
                                           salt, 10000, key2));

    /* As chaves devem ser diferentes com salt diferente */
    TEST_ASSERT_NOT_EQUAL_MEMORY(key1, key2, sizeof(key1));
}

void test_hmac(void)
{
    const uint8_t key[32] = {0x01, 0x02, 0x03, 0x04}; /* Resto preenchido com zeros */
    uint8_t hmac1[32];
    uint8_t hmac2[32];

    /* Calcula HMAC do buffer de teste */
    TEST_ASSERT_TRUE(emu_crypto_calculate_hmac(test_buffer, TEST_BUFFER_SIZE,
                                               key, sizeof(key), hmac1));

    /* Calcula novamente - deve ser idêntico */
    TEST_ASSERT_TRUE(emu_crypto_calculate_hmac(test_buffer, TEST_BUFFER_SIZE,
                                               key, sizeof(key), hmac2));

    /* Os HMACs devem ser idênticos */
    TEST_ASSERT_EQUAL_MEMORY(hmac1, hmac2, sizeof(hmac1));

    /* Testa verificação de HMAC */
    TEST_ASSERT_TRUE(emu_crypto_verify_hmac(test_buffer, TEST_BUFFER_SIZE,
                                            key, sizeof(key), hmac1));

    /* Modifica um byte do buffer e verifica se o HMAC falha */
    test_buffer[1000] ^= 0xFF;
    TEST_ASSERT_FALSE(emu_crypto_verify_hmac(test_buffer, TEST_BUFFER_SIZE,
                                             key, sizeof(key), hmac1));

    /* Restaura o buffer */
    test_buffer[1000] ^= 0xFF;
}

void test_aes_cbc(void)
{
    /* Inicializa configuração para CBC */
    memset(&test_config, 0, sizeof(test_config));
    test_config.method = EMU_CRYPT_AES256_CBC;
    test_config.derive_from_password = false;

    /* Gera chave aleatória */
    TEST_ASSERT_TRUE(emu_generate_random_bytes(test_config.key, 32));

    /* Gera IV aleatório */
    TEST_ASSERT_TRUE(emu_generate_random_bytes(test_config.iv, 16));

    /* Encripta os dados de teste */
    TEST_ASSERT_TRUE(emu_crypto_encrypt(test_buffer, encrypted_buffer,
                                        TEST_BUFFER_SIZE, &test_config));

    /* Verifica se os dados criptografados são diferentes dos originais */
    TEST_ASSERT_NOT_EQUAL_MEMORY(test_buffer, encrypted_buffer, TEST_BUFFER_SIZE);

    /* Descriptografa os dados */
    TEST_ASSERT_TRUE(emu_crypto_decrypt(encrypted_buffer, decrypted_buffer,
                                        TEST_BUFFER_SIZE, &test_config));

    /* Verifica se a descriptografia recuperou os dados originais */
    TEST_ASSERT_EQUAL_MEMORY(test_buffer, decrypted_buffer, TEST_BUFFER_SIZE);
}

void test_aes_gcm(void)
{
    /* Inicializa configuração para GCM */
    memset(&test_config, 0, sizeof(test_config));
    test_config.method = EMU_CRYPT_AES256_GCM;
    test_config.derive_from_password = false;

    /* Gera chave aleatória */
    TEST_ASSERT_TRUE(emu_generate_random_bytes(test_config.key, 32));

    /* Gera nonce aleatório */
    TEST_ASSERT_TRUE(emu_generate_random_bytes(test_config.nonce, 12));

    /* Encripta os dados de teste */
    TEST_ASSERT_TRUE(emu_crypto_encrypt(test_buffer, encrypted_buffer,
                                        TEST_BUFFER_SIZE, &test_config));

    /* Verifica se os dados criptografados são diferentes dos originais */
    TEST_ASSERT_NOT_EQUAL_MEMORY(test_buffer, encrypted_buffer, TEST_BUFFER_SIZE);

    /* Descriptografa os dados */
    TEST_ASSERT_TRUE(emu_crypto_decrypt(encrypted_buffer, decrypted_buffer,
                                        TEST_BUFFER_SIZE, &test_config));

    /* Verifica se a descriptografia recuperou os dados originais */
    TEST_ASSERT_EQUAL_MEMORY(test_buffer, decrypted_buffer, TEST_BUFFER_SIZE);

    /* Testa detecção de adulteração */
    encrypted_buffer[1000] ^= 0xFF;
    TEST_ASSERT_FALSE(emu_crypto_decrypt(encrypted_buffer, decrypted_buffer,
                                         TEST_BUFFER_SIZE, &test_config));
}

void test_password_encryption(void)
{
    const char *password = "senha_de_teste_123";

    /* Configura criptografia baseada em senha */
    memset(&test_config, 0, sizeof(test_config));
    test_config.method = EMU_CRYPT_AES256_GCM;
    test_config.derive_from_password = true;
    strncpy(test_config.password, password, sizeof(test_config.password) - 1);

    /* Encripta os dados de teste */
    TEST_ASSERT_TRUE(emu_crypto_encrypt(test_buffer, encrypted_buffer,
                                        TEST_BUFFER_SIZE, &test_config));

    /* Descriptografa os dados */
    TEST_ASSERT_TRUE(emu_crypto_decrypt(encrypted_buffer, decrypted_buffer,
                                        TEST_BUFFER_SIZE, &test_config));

    /* Verifica se a descriptografia recuperou os dados originais */
    TEST_ASSERT_EQUAL_MEMORY(test_buffer, decrypted_buffer, TEST_BUFFER_SIZE);
}

void test_save_state_encryption(void)
{
    const char *filename = TEST_DIR "/test_save.sav";
    const char *password = "senha_de_teste_123";

    /* Configura criptografia para o save state */
    TEST_ASSERT_TRUE(emu_save_state_enable_encryption(test_state, password));

    /* Salva o estado */
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, filename));

    /* Carrega o estado em um novo contexto */
    emu_save_state_t *loaded_state = emu_save_state_init(EMU_PLATFORM_TEST, NULL, 1024);
    TEST_ASSERT_NOT_NULL(loaded_state);

    /* Configura a senha antes de carregar */
    TEST_ASSERT_TRUE(emu_save_state_set_password(loaded_state, password));

    /* Carrega o arquivo */
    TEST_ASSERT_TRUE(emu_save_state_load(loaded_state, filename));

    /* Verifica se os dados foram preservados */
    TEST_ASSERT_EQUAL_MEMORY(test_state->data, loaded_state->data, test_state->size);

    /* Tenta carregar com senha errada */
    TEST_ASSERT_TRUE(emu_save_state_set_password(loaded_state, "senha_errada"));
    TEST_ASSERT_FALSE(emu_save_state_load(loaded_state, filename));

    /* Limpa */
    emu_save_state_shutdown(loaded_state);
}

void test_key_files(void)
{
    const char *key_file = TEST_DIR "/test.key";
    uint8_t key_data[32];
    uint8_t loaded_key[32];

    /* Gera dados aleatórios para a chave */
    TEST_ASSERT_TRUE(emu_generate_random_bytes(key_data, sizeof(key_data)));

    /* Salva a chave em arquivo */
    TEST_ASSERT_TRUE(emu_crypto_save_key_file(key_file, key_data, sizeof(key_data)));

    /* Carrega a chave do arquivo */
    TEST_ASSERT_TRUE(emu_crypto_load_key_file(key_file, loaded_key, sizeof(loaded_key)));

    /* Verifica se a chave foi preservada */
    TEST_ASSERT_EQUAL_MEMORY(key_data, loaded_key, sizeof(key_data));
}

void test_encryption_detection(void)
{
    const char *filename = TEST_DIR "/test_encrypted.sav";
    const char *unencrypted = TEST_DIR "/test_plain.sav";
    bool is_encrypted;

    /* Salva um arquivo criptografado */
    TEST_ASSERT_TRUE(emu_save_state_enable_encryption(test_state, "senha123"));
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, filename));

    /* Verifica detecção de criptografia */
    TEST_ASSERT_TRUE(emu_crypto_detect_encryption(filename, &is_encrypted));
    TEST_ASSERT_TRUE(is_encrypted);

    /* Salva um arquivo não criptografado */
    TEST_ASSERT_TRUE(emu_save_state_disable_encryption(test_state));
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, unencrypted));

    /* Verifica detecção de arquivo não criptografado */
    TEST_ASSERT_TRUE(emu_crypto_detect_encryption(unencrypted, &is_encrypted));
    TEST_ASSERT_FALSE(is_encrypted);

    /* Testa com arquivo inexistente */
    TEST_ASSERT_FALSE(emu_crypto_detect_encryption("arquivo_inexistente.sav", &is_encrypted));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_random_bytes);
    RUN_TEST(test_key_derivation);
    RUN_TEST(test_hmac);
    RUN_TEST(test_aes_cbc);
    RUN_TEST(test_aes_gcm);
    RUN_TEST(test_password_encryption);
    RUN_TEST(test_save_state_encryption);
    RUN_TEST(test_key_files);
    RUN_TEST(test_encryption_detection);

    return UNITY_END();
}
