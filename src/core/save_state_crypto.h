/**
 * @file save_state_crypto.h
 * @brief API para criptografia e segurança de save states
 * @version 1.0
 * @date 2025-03-18
 *
 * Este arquivo define a API pública para a criptografia e recursos de
 * segurança para save states, incluindo criptografia AES-256,
 * verificação de integridade e geração de chaves seguras.
 */

#ifndef EMU_SAVE_STATE_CRYPTO_H
#define EMU_SAVE_STATE_CRYPTO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "save_state.h"

/**
 * @brief Algoritmos de criptografia suportados
 */
typedef enum {
    EMU_CRYPTO_NONE = 0,      /**< Sem criptografia */
    EMU_CRYPTO_AES_256_CBC,   /**< AES-256 no modo CBC com HMAC-SHA256 */
    EMU_CRYPTO_AES_256_GCM,   /**< AES-256 no modo GCM (AEAD) */
    EMU_CRYPTO_CHACHA20_POLY1305 /**< ChaCha20-Poly1305 (AEAD) */
} emu_crypto_algorithm_t;

/**
 * @brief Métodos de derivação de chave
 */
typedef enum {
    EMU_KEY_DERIVE_NONE = 0,   /**< Sem derivação (chave direta) */
    EMU_KEY_DERIVE_PBKDF2,     /**< PBKDF2 (senha) */
    EMU_KEY_DERIVE_ARGON2,     /**< Argon2id (senha) */
    EMU_KEY_DERIVE_KEY_FILE    /**< Arquivo de chave */
} emu_key_derivation_t;

/**
 * @brief Configuração de criptografia
 */
typedef struct {
    emu_crypto_algorithm_t algorithm;    /**< Algoritmo de criptografia */
    emu_key_derivation_t key_method;     /**< Método de derivação de chave */

    union {
        struct {
            char password[128];          /**< Senha para derivação */
            uint32_t iterations;         /**< Iterações para derivação */
        } password;

        struct {
            char key_file_path[256];     /**< Caminho do arquivo de chave */
        } key_file;

        uint8_t raw_key[32];             /**< Chave bruta (256 bits) */
    } key;

    bool encrypt_metadata;               /**< Criptografar metadados */
    bool encrypt_thumbnail;              /**< Criptografar thumbnail */
    bool verify_signature;               /**< Verificar assinatura */
    bool verify_integrity;               /**< Verificar integridade */
    bool store_key_hash;                 /**< Armazenar hash da chave */

    char key_identifier[64];             /**< Identificador para a chave */
} emu_crypto_config_t;

/**
 * @brief Informações de criptografia em um save state
 */
typedef struct {
    emu_crypto_algorithm_t algorithm;    /**< Algoritmo utilizado */
    emu_key_derivation_t key_method;     /**< Método de derivação */
    uint8_t salt[16];                    /**< Salt usado (se aplicável) */
    uint8_t iv[16];                      /**< Vetor de inicialização */
    uint32_t iterations;                 /**< Iterações (se PBKDF2/Argon2) */
    uint8_t key_hash[32];                /**< Hash da chave (verificação) */
    uint8_t signature[64];               /**< Assinatura digital */
    char key_identifier[64];             /**< Identificador da chave */
    bool metadata_encrypted;             /**< Metadados criptografados */
    bool thumbnail_encrypted;            /**< Thumbnail criptografado */
} emu_crypto_info_t;

/**
 * @brief Inicializa subsistema de criptografia
 *
 * @return true se inicializado com sucesso, false caso contrário
 */
bool emu_crypto_init(void);

/**
 * @brief Finaliza subsistema de criptografia
 */
void emu_crypto_shutdown(void);

/**
 * @brief Configura criptografia para um contexto de save state
 *
 * @param state Contexto de save state
 * @param config Configuração de criptografia
 * @return true se configurado com sucesso, false caso contrário
 */
bool emu_save_state_crypto_configure(emu_save_state_t* state, const emu_crypto_config_t* config);

/**
 * @brief Obtém configuração atual de criptografia
 *
 * @param state Contexto de save state
 * @param config Buffer para receber configuração
 * @return true se obtida com sucesso, false caso contrário
 */
bool emu_save_state_crypto_get_config(emu_save_state_t* state, emu_crypto_config_t* config);

/**
 * @brief Detecta se um arquivo de save state está criptografado
 *
 * @param filepath Caminho do arquivo
 * @param info Buffer para receber informações de criptografia (pode ser NULL)
 * @return true se criptografado, false caso contrário
 */
bool emu_save_state_is_encrypted(const char* filepath, emu_crypto_info_t* info);

/**
 * @brief Verifica se uma senha é válida para um save state criptografado
 *
 * @param filepath Caminho do arquivo
 * @param password Senha a verificar
 * @return true se senha válida, false caso contrário
 */
bool emu_save_state_verify_password(const char* filepath, const char* password);

/**
 * @brief Gera bytes aleatórios criptograficamente seguros
 *
 * @param buffer Buffer para receber bytes
 * @param length Número de bytes a gerar
 * @return true se gerados com sucesso, false caso contrário
 */
bool emu_crypto_random_bytes(void* buffer, size_t length);

/**
 * @brief Gera um arquivo de chave seguro
 *
 * @param filepath Caminho para salvar o arquivo
 * @param password Senha para proteger o arquivo (opcional)
 * @param key_id Identificador da chave (opcional)
 * @return true se gerado com sucesso, false caso contrário
 */
bool emu_crypto_generate_key_file(const char* filepath, const char* password, const char* key_id);

/**
 * @brief Carrega um arquivo de chave
 *
 * @param filepath Caminho do arquivo
 * @param password Senha para desbloquear (se protegido)
 * @param config Configuração a ser preenchida com a chave
 * @return true se carregado com sucesso, false caso contrário
 */
bool emu_crypto_load_key_file(const char* filepath, const char* password, emu_crypto_config_t* config);

/**
 * @brief Calcula hash HMAC-SHA256
 *
 * @param key Chave para HMAC
 * @param key_len Tamanho da chave
 * @param data Dados a calcular hash
 * @param data_len Tamanho dos dados
 * @param out Buffer para receber hash (32 bytes)
 * @return true se calculado com sucesso, false caso contrário
 */
bool emu_crypto_hmac_sha256(const void* key, size_t key_len,
                          const void* data, size_t data_len,
                          uint8_t out[32]);

/**
 * @brief Criptografa dados com AES-256-CBC
 *
 * @param key Chave AES-256 (32 bytes)
 * @param iv Vetor de inicialização (16 bytes)
 * @param plaintext Texto puro a criptografar
 * @param plaintext_len Tamanho do texto puro
 * @param ciphertext Buffer para texto cifrado (deve ter tamanho >= plaintext_len + 16)
 * @param ciphertext_len Ponteiro para receber tamanho do texto cifrado
 * @return true se criptografado com sucesso, false caso contrário
 */
bool emu_crypto_aes_256_cbc_encrypt(const uint8_t key[32], const uint8_t iv[16],
                                  const void* plaintext, size_t plaintext_len,
                                  void* ciphertext, size_t* ciphertext_len);

/**
 * @brief Descriptografa dados com AES-256-CBC
 *
 * @param key Chave AES-256 (32 bytes)
 * @param iv Vetor de inicialização (16 bytes)
 * @param ciphertext Texto cifrado a descriptografar
 * @param ciphertext_len Tamanho do texto cifrado
 * @param plaintext Buffer para texto puro
 * @param plaintext_len Ponteiro para receber tamanho do texto puro
 * @return true se descriptografado com sucesso, false caso contrário
 */
bool emu_crypto_aes_256_cbc_decrypt(const uint8_t key[32], const uint8_t iv[16],
                                  const void* ciphertext, size_t ciphertext_len,
                                  void* plaintext, size_t* plaintext_len);

/**
 * @brief Criptografa dados com AES-256-GCM (AEAD)
 *
 * @param key Chave AES-256 (32 bytes)
 * @param nonce Nonce (12 bytes recomendado)
 * @param nonce_len Tamanho do nonce
 * @param plaintext Texto puro a criptografar
 * @param plaintext_len Tamanho do texto puro
 * @param aad Dados autenticados adicionais (opcional)
 * @param aad_len Tamanho dos dados AAD
 * @param ciphertext Buffer para texto cifrado (deve ter tamanho >= plaintext_len)
 * @param tag Buffer para tag de autenticação (16 bytes)
 * @return true se criptografado com sucesso, false caso contrário
 */
bool emu_crypto_aes_256_gcm_encrypt(const uint8_t key[32],
                                  const uint8_t* nonce, size_t nonce_len,
                                  const void* plaintext, size_t plaintext_len,
                                  const void* aad, size_t aad_len,
                                  void* ciphertext, uint8_t tag[16]);

/**
 * @brief Descriptografa dados com AES-256-GCM (AEAD)
 *
 * @param key Chave AES-256 (32 bytes)
 * @param nonce Nonce (mesmo usado na criptografia)
 * @param nonce_len Tamanho do nonce
 * @param ciphertext Texto cifrado a descriptografar
 * @param ciphertext_len Tamanho do texto cifrado
 * @param aad Dados autenticados adicionais (opcional)
 * @param aad_len Tamanho dos dados AAD
 * @param tag Tag de autenticação (16 bytes)
 * @param plaintext Buffer para texto puro
 * @return true se descriptografado com sucesso, false caso contrário
 */
bool emu_crypto_aes_256_gcm_decrypt(const uint8_t key[32],
                                  const uint8_t* nonce, size_t nonce_len,
                                  const void* ciphertext, size_t ciphertext_len,
                                  const void* aad, size_t aad_len,
                                  const uint8_t tag[16], void* plaintext);

/**
 * @brief Deriva chave a partir de senha usando PBKDF2-HMAC-SHA256
 *
 * @param password Senha de entrada
 * @param password_len Tamanho da senha
 * @param salt Salt (recomendado 16+ bytes)
 * @param salt_len Tamanho do salt
 * @param iterations Número de iterações (recomendado 100000+)
 * @param key Buffer para receber chave derivada
 * @param key_len Tamanho da chave a derivar (32 para AES-256)
 * @return true se derivada com sucesso, false caso contrário
 */
bool emu_crypto_pbkdf2(const char* password, size_t password_len,
                     const uint8_t* salt, size_t salt_len,
                     uint32_t iterations,
                     uint8_t* key, size_t key_len);

/**
 * @brief Deriva chave a partir de senha usando Argon2id
 *
 * @param password Senha de entrada
 * @param password_len Tamanho da senha
 * @param salt Salt (recomendado 16+ bytes)
 * @param salt_len Tamanho do salt
 * @param memory_kb Memória em KB (recomendado 64MB+)
 * @param iterations Número de iterações (recomendado 3+)
 * @param parallelism Paralelismo (recomendado 4+)
 * @param key Buffer para receber chave derivada
 * @param key_len Tamanho da chave a derivar (32 para AES-256)
 * @return true se derivada com sucesso, false caso contrário
 */
bool emu_crypto_argon2id(const char* password, size_t password_len,
                       const uint8_t* salt, size_t salt_len,
                       uint32_t memory_kb, uint32_t iterations, uint32_t parallelism,
                       uint8_t* key, size_t key_len);

/**
 * @brief Muda a senha de um save state criptografado
 *
 * @param filepath Caminho do arquivo
 * @param old_password Senha atual
 * @param new_password Nova senha
 * @param iterations Iterações para nova senha (0 para padrão)
 * @return true se alterada com sucesso, false caso contrário
 */
bool emu_save_state_change_password(const char* filepath,
                                  const char* old_password,
                                  const char* new_password,
                                  uint32_t iterations);

/**
 * @brief Altera método de criptografia de um save state
 *
 * @param filepath Caminho do arquivo
 * @param current_config Configuração atual (para autenticação)
 * @param new_config Nova configuração
 * @return true se alterado com sucesso, false caso contrário
 */
bool emu_save_state_reencrypt(const char* filepath,
                            const emu_crypto_config_t* current_config,
                            const emu_crypto_config_t* new_config);

/**
 * @brief Define callback para solicitação de senha
 *
 * @param callback Função de callback
 */
void emu_crypto_set_password_callback(bool (*callback)(const char* filepath,
                                                     char* password,
                                                     size_t max_length,
                                                     bool verify));

/**
 * @brief Verifica integridade criptográfica de um save state
 *
 * @param filepath Caminho do arquivo
 * @param config Configuração de criptografia (se NULL, tentará detectar)
 * @return true se íntegro, false caso contrário
 */
bool emu_save_state_verify_integrity(const char* filepath, const emu_crypto_config_t* config);

/**
 * @brief Sanitiza dados sensíveis de criptografia da memória
 *
 * @param data Ponteiro para dados a sanitizar
 * @param length Tamanho dos dados
 */
void emu_crypto_sanitize_memory(void* data, size_t length);

/**
 * @brief Exporta metadados de criptografia de um save state
 *
 * @param filepath Caminho do arquivo
 * @param info Buffer para receber informações
 * @return true se exportado com sucesso, false caso contrário
 */
bool emu_save_state_export_crypto_info(const char* filepath, emu_crypto_info_t* info);

#endif /* EMU_SAVE_STATE_CRYPTO_H */
