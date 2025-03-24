/**
 * @file nes_save_state_crypto.c
 * @brief Implementação da criptografia para save states do NES
 * @version 1.0
 * @date 2025-04-30
 *
 * Esta implementação se integra ao sistema unificado de criptografia
 * para fornecer proteção AES-256 aos save states do NES.
 */

#include <stdlib.h>
#include <string.h>
#include "nes_save_state_crypto.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/error_handling.h"
#include "../../../utils/log_categories.h"

/* Definição de categoria de log específica */
#define EMU_LOG_CAT_SAVE_CRYPTO EMU_LOG_CAT_PLATFORM_SECURITY

/* Macros de log */
#define CRYPTO_LOG_INFO(msg, ...) EMU_LOG_INFO(EMU_LOG_CAT_SAVE_CRYPTO, msg, ##__VA_ARGS__)
#define CRYPTO_LOG_ERROR(msg, ...) EMU_LOG_ERROR(EMU_LOG_CAT_SAVE_CRYPTO, msg, ##__VA_ARGS__)
#define CRYPTO_LOG_DEBUG(msg, ...) EMU_LOG_DEBUG(EMU_LOG_CAT_SAVE_CRYPTO, msg, ##__VA_ARGS__)
#define CRYPTO_LOG_WARN(msg, ...) EMU_LOG_WARN(EMU_LOG_CAT_SAVE_CRYPTO, msg, ##__VA_ARGS__)

bool nes_save_state_enable_encryption(emu_save_state_t *state, const char *password)
{
    if (!state || !password)
    {
        CRYPTO_LOG_ERROR("Parâmetros inválidos para ativação de criptografia");
        return false;
    }

    /* Configura a criptografia */
    emu_crypto_config_t config;
    memset(&config, 0, sizeof(config));

    config.algorithm = EMU_CRYPTO_AES_256_GCM;  // Usar modo GCM para autenticação
    config.derive_from_password = true;
    strncpy(config.password, password, sizeof(config.password) - 1);
    config.kdf_iterations = 10000;  // Padrão recomendado

    /* Tenta gerar salt aleatório */
    if (!emu_crypto_random_bytes(config.salt, sizeof(config.salt)))
    {
        CRYPTO_LOG_ERROR("Falha ao gerar salt para criptografia");
        return false;
    }

    /* Aplica a configuração ao contexto de save state */
    if (!emu_save_state_crypto_configure(state, &config))
    {
        CRYPTO_LOG_ERROR("Falha ao configurar criptografia para o contexto");
        return false;
    }

    CRYPTO_LOG_INFO("Criptografia AES-256-GCM ativada para save states do NES");
    return true;
}

bool nes_save_state_disable_encryption(emu_save_state_t *state)
{
    if (!state)
    {
        CRYPTO_LOG_ERROR("Contexto de estado inválido");
        return false;
    }

    /* Configura para desativar a criptografia */
    emu_crypto_config_t config;
    memset(&config, 0, sizeof(config));
    config.algorithm = EMU_CRYPTO_NONE;  // Sem criptografia

    /* Aplica a configuração */
    if (!emu_save_state_crypto_configure(state, &config))
    {
        CRYPTO_LOG_ERROR("Falha ao desativar criptografia");
        return false;
    }

    CRYPTO_LOG_INFO("Criptografia desativada para save states do NES");
    return true;
}

bool nes_save_state_is_encrypted(const char *filepath)
{
    if (!filepath)
    {
        CRYPTO_LOG_ERROR("Caminho de arquivo inválido");
        return false;
    }

    /* Verifica se o arquivo está criptografado */
    emu_crypto_info_t info;
    memset(&info, 0, sizeof(info));

    if (!emu_save_state_is_encrypted(filepath, &info))
    {
        return false;  // Não criptografado ou erro
    }

    CRYPTO_LOG_DEBUG("Arquivo de save state criptografado com %s",
                    info.algorithm == EMU_CRYPTO_AES_256_CBC ? "AES-256-CBC" :
                    info.algorithm == EMU_CRYPTO_AES_256_GCM ? "AES-256-GCM" :
                    info.algorithm == EMU_CRYPTO_CHACHA20_POLY1305 ? "ChaCha20-Poly1305" : "algoritmo desconhecido");

    return true;
}

bool nes_save_state_export_key(emu_save_state_t *state, const char *key_file, const char *key_password)
{
    if (!state || !key_file || !key_password)
    {
        CRYPTO_LOG_ERROR("Parâmetros inválidos para exportação de chave");
        return false;
    }

    /* Obter a configuração atual */
    emu_crypto_config_t config;
    if (!emu_save_state_crypto_get_config(state, &config))
    {
        CRYPTO_LOG_ERROR("Falha ao obter configuração de criptografia");
        return false;
    }

    /* Verificar se a criptografia está habilitada */
    if (config.algorithm == EMU_CRYPTO_NONE)
    {
        CRYPTO_LOG_ERROR("Criptografia não está habilitada, não há chave para exportar");
        return false;
    }

    /* Gerar ID exclusivo para a chave */
    char key_id[33];  // 32 hex chars + null
    snprintf(key_id, sizeof(key_id), "nes_save_key_%llx", (unsigned long long)time(NULL));

    /* Exportar a chave para o arquivo */
    if (!emu_crypto_generate_key_file(key_file, key_password, key_id))
    {
        CRYPTO_LOG_ERROR("Falha ao exportar chave para arquivo");
        return false;
    }

    CRYPTO_LOG_INFO("Chave de criptografia exportada para %s", key_file);
    return true;
}

bool nes_save_state_import_key(emu_save_state_t *state, const char *key_file, const char *key_password)
{
    if (!state || !key_file || !key_password)
    {
        CRYPTO_LOG_ERROR("Parâmetros inválidos para importação de chave");
        return false;
    }

    /* Carregar configuração de criptografia do arquivo */
    emu_crypto_config_t config;
    memset(&config, 0, sizeof(config));

    if (!emu_crypto_load_key_file(key_file, key_password, &config))
    {
        CRYPTO_LOG_ERROR("Falha ao carregar arquivo de chave");
        return false;
    }

    /* Aplicar a configuração carregada */
    if (!emu_save_state_crypto_configure(state, &config))
    {
        CRYPTO_LOG_ERROR("Falha ao aplicar configuração de chave importada");
        /* Limpar dados sensíveis */
        emu_crypto_sanitize_memory(&config, sizeof(config));
        return false;
    }

    /* Limpar dados sensíveis */
    emu_crypto_sanitize_memory(&config, sizeof(config));

    CRYPTO_LOG_INFO("Chave de criptografia importada de %s", key_file);
    return true;
}
