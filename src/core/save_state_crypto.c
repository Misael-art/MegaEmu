/**
 * @file save_state_crypto.c
 * @brief Implementação da criptografia para o sistema de Save States
 * @version 1.0
 * @date 2025-03-18
 *
 * Este arquivo implementa a funcionalidade de criptografia para save states
 * usando o algoritmo AES-256 nos modos CBC e GCM.
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "save_state_crypto.h"
#include "save_state_private.h"

/* Biblioteca de criptografia (OpenSSL ou similar) */
#include "crypto/aes.h"
#include "crypto/sha256.h"
#include "crypto/hmac.h"
#include "crypto/rand.h"
#include "crypto/modes.h"

/* Definições privadas */
#define AES_BLOCK_SIZE 16
#define HMAC_SIZE 32
#define GCM_TAG_SIZE 16
#define PBKDF2_MIN_ITERATIONS 10000

/* Estrutura privada para contexto de criptografia */
typedef struct {
    emu_encryption_config_t config;
    bool is_configured;
    uint8_t derived_key[32];
    bool key_derived;
} emu_crypto_context_t;

/**
 * @brief Inicializa o subsistema de criptografia
 */
bool emu_crypto_init(void) {
    /* Inicializa o gerador de números aleatórios */
    srand(time(NULL));

    /* Inicializa a biblioteca de criptografia */
    return crypto_library_init();
}

/**
 * @brief Finaliza o subsistema de criptografia
 */
void emu_crypto_shutdown(void) {
    /* Limpa recursos da biblioteca de criptografia */
    crypto_library_cleanup();
}

/**
 * @brief Gera bytes aleatórios seguros
 */
bool emu_generate_random_bytes(uint8_t* buffer, size_t size) {
    if (!buffer || size == 0) {
        return false;
    }

    /* Utiliza gerador de números aleatórios criptograficamente seguro */
    return crypto_rand_bytes(buffer, size) == 1;
}

/**
 * @brief Implementação de PBKDF2 com HMAC-SHA256
 */
bool emu_crypto_derive_key(const char* password, size_t password_len,
                         const uint8_t* salt, uint32_t iterations,
                         uint8_t* key) {
    if (!password || !salt || !key) {
        return false;
    }

    /* Garante um número mínimo de iterações */
    if (iterations < PBKDF2_MIN_ITERATIONS) {
        iterations = PBKDF2_MIN_ITERATIONS;
    }

    /* Deriva a chave usando PBKDF2 */
    return crypto_pbkdf2_hmac(password, password_len,
                             salt, 16,
                             iterations,
                             key, 32) == 1;
}

/**
 * @brief Calcula um HMAC-SHA256
 */
bool emu_crypto_calculate_hmac(const uint8_t* data, size_t size,
                             const uint8_t* key, size_t key_size,
                             uint8_t* hmac) {
    if (!data || !key || !hmac) {
        return false;
    }

    unsigned int hmac_len = HMAC_SIZE;
    return crypto_hmac(key, key_size, data, size, hmac, &hmac_len) == 1;
}

/**
 * @brief Verifica um HMAC-SHA256
 */
bool emu_crypto_verify_hmac(const uint8_t* data, size_t size,
                          const uint8_t* key, size_t key_size,
                          const uint8_t* expected_hmac) {
    if (!data || !key || !expected_hmac) {
        return false;
    }

    uint8_t calculated_hmac[HMAC_SIZE];
    if (!emu_crypto_calculate_hmac(data, size, key, key_size, calculated_hmac)) {
        return false;
    }

    /* Comparação em tempo constante para evitar timing attacks */
    return crypto_constant_time_cmp(calculated_hmac, expected_hmac, HMAC_SIZE) == 0;
}

/**
 * @brief Prepara o contexto de criptografia derivando a chave se necessário
 */
static bool prepare_crypto_context(emu_encryption_config_t* config) {
    if (!config) {
        return false;
    }

    /* Gera IV aleatório se não fornecido */
    if (config->iv[0] == 0 && config->iv[1] == 0 && config->iv[2] == 0) {
        if (!emu_generate_random_bytes(config->iv, AES_BLOCK_SIZE)) {
            return false;
        }
    }

    /* Deriva a chave a partir da senha se necessário */
    if (config->derive_from_password) {
        /* Gera salt aleatório se não fornecido */
        if (config->salt[0] == 0 && config->salt[1] == 0 && config->salt[2] == 0) {
            if (!emu_generate_random_bytes(config->salt, 16)) {
                return false;
            }
        }

        uint8_t derived_key[32];
        if (!emu_crypto_derive_key(config->password, strlen(config->password),
                                 config->salt, config->kdf_iterations,
                                 derived_key)) {
            return false;
        }

        /* Limpa a senha da memória e substitui pela chave derivada */
        memset(config->password, 0, sizeof(config->password));
        config->derive_from_password = false;
        memcpy(config->key, derived_key, 32);

        /* Limpa a chave derivada temporária */
        memset(derived_key, 0, sizeof(derived_key));
    }

    return true;
}

/**
 * @brief Criptografa usando AES-256-CBC
 */
static bool encrypt_aes_cbc(const uint8_t* input, uint8_t* output, size_t size,
                           const uint8_t* key, const uint8_t* iv) {
    if (!input || !output || !key || !iv) {
        return false;
    }

    /* Configura a chave AES */
    AES_KEY aes_key;
    if (crypto_aes_set_encrypt_key(key, 256, &aes_key) != 0) {
        return false;
    }

    /* Copia o IV para uma área de trabalho */
    uint8_t iv_copy[AES_BLOCK_SIZE];
    memcpy(iv_copy, iv, AES_BLOCK_SIZE);

    /* Executa a criptografia em modo CBC */
    crypto_aes_cbc_encrypt(input, output, size, &aes_key, iv_copy, 1);

    return true;
}

/**
 * @brief Descriptografa usando AES-256-CBC
 */
static bool decrypt_aes_cbc(const uint8_t* input, uint8_t* output, size_t size,
                          const uint8_t* key, const uint8_t* iv) {
    if (!input || !output || !key || !iv) {
        return false;
    }

    /* Configura a chave AES */
    AES_KEY aes_key;
    if (crypto_aes_set_decrypt_key(key, 256, &aes_key) != 0) {
        return false;
    }

    /* Copia o IV para uma área de trabalho */
    uint8_t iv_copy[AES_BLOCK_SIZE];
    memcpy(iv_copy, iv, AES_BLOCK_SIZE);

    /* Executa a descriptografia em modo CBC */
    crypto_aes_cbc_encrypt(input, output, size, &aes_key, iv_copy, 0);

    return true;
}

/**
 * @brief Criptografa usando AES-256-GCM
 */
static bool encrypt_aes_gcm(const uint8_t* input, uint8_t* output, size_t size,
                           const uint8_t* key, const uint8_t* iv, uint8_t* tag) {
    if (!input || !output || !key || !iv || !tag) {
        return false;
    }

    /* Configura o contexto GCM */
    GCM128_CONTEXT gcm_ctx;
    AES_KEY aes_key;

    if (crypto_aes_set_encrypt_key(key, 256, &aes_key) != 0) {
        return false;
    }

    crypto_gcm128_init(&gcm_ctx, &aes_key, (block128_f)crypto_aes_encrypt);

    /* Inicializa com IV */
    crypto_gcm128_setiv(&gcm_ctx, iv, 12);

    /* Sem dados autenticados adicionais (AAD) */
    crypto_gcm128_aad(&gcm_ctx, NULL, 0);

    /* Criptografa os dados */
    if (crypto_gcm128_encrypt(&gcm_ctx, input, output, size) != 0) {
        return false;
    }

    /* Obtém a tag de autenticação */
    crypto_gcm128_tag(&gcm_ctx, tag, GCM_TAG_SIZE);

    return true;
}

/**
 * @brief Descriptografa usando AES-256-GCM
 */
static bool decrypt_aes_gcm(const uint8_t* input, uint8_t* output, size_t size,
                          const uint8_t* key, const uint8_t* iv,
                          const uint8_t* tag) {
    if (!input || !output || !key || !iv || !tag) {
        return false;
    }

    /* Configura o contexto GCM */
    GCM128_CONTEXT gcm_ctx;
    AES_KEY aes_key;

    if (crypto_aes_set_encrypt_key(key, 256, &aes_key) != 0) {
        return false;
    }

    crypto_gcm128_init(&gcm_ctx, &aes_key, (block128_f)crypto_aes_encrypt);

    /* Inicializa com IV */
    crypto_gcm128_setiv(&gcm_ctx, iv, 12);

    /* Sem dados autenticados adicionais (AAD) */
    crypto_gcm128_aad(&gcm_ctx, NULL, 0);

    /* Descriptografa os dados */
    if (crypto_gcm128_decrypt(&gcm_ctx, input, output, size) != 0) {
        return false;
    }

    /* Verifica a tag de autenticação */
    uint8_t calculated_tag[GCM_TAG_SIZE];
    crypto_gcm128_tag(&gcm_ctx, calculated_tag, GCM_TAG_SIZE);

    if (crypto_constant_time_cmp(calculated_tag, tag, GCM_TAG_SIZE) != 0) {
        /* Falha na verificação da tag, dados corrompidos ou chave incorreta */
        memset(output, 0, size);  /* Limpa dados sensíveis */
        return false;
    }

    return true;
}

/**
 * @brief Criptografa um buffer usando AES-256
 */
bool emu_crypto_encrypt(const uint8_t* input, uint8_t* output, size_t size,
                      emu_encryption_config_t* config) {
    if (!input || !output || !config || size == 0) {
        return false;
    }

    /* Prepara o contexto de criptografia */
    if (!prepare_crypto_context(config)) {
        return false;
    }

    /* Escolhe o método de criptografia */
    bool result = false;

    switch (config->method) {
        case EMU_CRYPT_AES256_CBC:
            result = encrypt_aes_cbc(input, output, size, config->key, config->iv);
            break;

        case EMU_CRYPT_AES256_GCM:
            result = encrypt_aes_gcm(input, output, size, config->key, config->iv,
                                   config->auth_tag);
            break;

        default:
            /* Método não suportado */
            result = false;
    }

    return result;
}

/**
 * @brief Descriptografa um buffer usando AES-256
 */
bool emu_crypto_decrypt(const uint8_t* input, uint8_t* output, size_t size,
                      const emu_encryption_config_t* config) {
    if (!input || !output || !config || size == 0) {
        return false;
    }

    /* Escolhe o método de criptografia */
    bool result = false;

    switch (config->method) {
        case EMU_CRYPT_AES256_CBC:
            result = decrypt_aes_cbc(input, output, size, config->key, config->iv);
            break;

        case EMU_CRYPT_AES256_GCM:
            result = decrypt_aes_gcm(input, output, size, config->key, config->iv,
                                   config->auth_tag);
            break;

        default:
            /* Método não suportado */
            result = false;
    }

    return result;
}

/**
 * @brief Configura a criptografia para um contexto de save state
 */
bool emu_save_state_set_encryption(emu_save_state_t* state, const emu_encryption_config_t* config) {
    if (!state || !config) {
        return false;
    }

    /* Aloca ou reutiliza o contexto de criptografia */
    emu_crypto_context_t* ctx = (emu_crypto_context_t*)state->crypto_context;

    if (!ctx) {
        ctx = (emu_crypto_context_t*)malloc(sizeof(emu_crypto_context_t));
        if (!ctx) {
            return false;
        }
        memset(ctx, 0, sizeof(emu_crypto_context_t));
        state->crypto_context = ctx;
    }

    /* Copia a configuração */
    memcpy(&ctx->config, config, sizeof(emu_encryption_config_t));
    ctx->is_configured = true;
    ctx->key_derived = false;

    return true;
}

/**
 * @brief Obtém a configuração de criptografia atual de um contexto
 */
bool emu_save_state_get_encryption(const emu_save_state_t* state, emu_encryption_config_t* config) {
    if (!state || !config) {
        return false;
    }

    /* Verifica se o contexto de criptografia existe */
    emu_crypto_context_t* ctx = (emu_crypto_context_t*)state->crypto_context;

    if (!ctx || !ctx->is_configured) {
        return false;
    }

    /* Copia a configuração */
    memcpy(config, &ctx->config, sizeof(emu_encryption_config_t));

    return true;
}

/**
 * @brief Verifica se um arquivo de save state está criptografado
 */
bool emu_save_state_is_encrypted(const char* filepath, emu_crypto_method_t* method) {
    if (!filepath) {
        return false;
    }

    /* Abre o arquivo para leitura */
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return false;
    }

    /* Lê o cabeçalho para verificar a assinatura e flags */
    uint8_t header[64];
    size_t header_size = fread(header, 1, sizeof(header), file);
    fclose(file);

    if (header_size < 32) {
        return false;
    }

    /* Verifica a assinatura do save state */
    if (memcmp(header, "MEGA_EMU_SAVE", 13) != 0) {
        return false;
    }

    /* Verifica as flags de criptografia */
    uint32_t flags = 0;
    memcpy(&flags, header + 24, 4);

    bool is_encrypted = (flags & 0x00000100) != 0;  /* Bit de criptografia */

    /* Se solicitado, identifica o método de criptografia */
    if (method && is_encrypted) {
        uint8_t crypto_method = header[28];  /* Método de criptografia no cabeçalho */

        switch (crypto_method) {
            case 1:
                *method = EMU_CRYPT_AES256_CBC;
                break;
            case 2:
                *method = EMU_CRYPT_AES256_GCM;
                break;
            default:
                *method = EMU_CRYPT_NONE;
                break;
        }
    }

    return is_encrypted;
}

/**
 * @brief Importa uma chave de criptografia de um arquivo
 */
bool emu_crypto_import_key(const char* filepath, emu_encryption_config_t* config) {
    if (!filepath || !config) {
        return false;
    }

    /* Abre o arquivo para leitura */
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return false;
    }

    /* Lê a chave e metadados */
    bool result = false;
    uint8_t buffer[512];

    if (fread(buffer, 1, sizeof(buffer), file) > 64) {
        /* Verifica a assinatura do arquivo de chave */
        if (memcmp(buffer, "MEGA_EMU_KEY", 12) == 0) {
            /* Obtém o método de criptografia */
            uint8_t method = buffer[12];
            config->method = (emu_crypto_method_t)method;

            /* Obtém a chave */
            config->derive_from_password = false;
            memcpy(config->key, buffer + 16, 32);

            /* Obtém o IV se disponível */
            if (buffer[13] & 0x01) {
                memcpy(config->iv, buffer + 48, 16);
            }

            result = true;
        }
    }

    fclose(file);
    return result;
}

/**
 * @brief Exporta uma chave de criptografia para um arquivo
 */
bool emu_crypto_export_key(const char* filepath, const emu_encryption_config_t* config,
                         bool include_private_key) {
    if (!filepath || !config) {
        return false;
    }

    /* Verifica se há uma chave para exportar */
    if (config->method == EMU_CRYPT_NONE) {
        return false;
    }

    /* Abre o arquivo para escrita */
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        return false;
    }

    /* Escreve o cabeçalho do arquivo de chave */
    uint8_t buffer[512];
    memset(buffer, 0, sizeof(buffer));

    /* Assinatura */
    memcpy(buffer, "MEGA_EMU_KEY", 12);

    /* Método e flags */
    buffer[12] = (uint8_t)config->method;
    buffer[13] = 0x00;

    if (include_private_key) {
        /* Inclui a chave privada */
        buffer[13] |= 0x02;
        memcpy(buffer + 16, config->key, 32);
    }

    /* Inclui o IV */
    buffer[13] |= 0x01;
    memcpy(buffer + 48, config->iv, 16);

    /* Escreve os dados no arquivo */
    size_t written = fwrite(buffer, 1, 64, file);
    fclose(file);

    return (written == 64);
}

/**
 * @brief Converte um save state não criptografado para criptografado
 */
bool emu_save_state_encrypt_file(const char* input_path, const char* output_path,
                               const emu_encryption_config_t* config) {
    /* Implementação omitida por brevidade */
    /* Esta função abriria o arquivo de entrada, aplicaria a criptografia e salvaria no arquivo de saída */
    return false;
}

/**
 * @brief Converte um save state criptografado para não criptografado
 */
bool emu_save_state_decrypt_file(const char* input_path, const char* output_path,
                               const emu_encryption_config_t* config) {
    /* Implementação omitida por brevidade */
    /* Esta função abriria o arquivo de entrada, aplicaria a descriptografia e salvaria no arquivo de saída */
    return false;
}
