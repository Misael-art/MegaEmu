/**
 * @file nes_save_state_crypto.h
 * @brief Interface para criptografia de save states do NES
 * @version 1.0
 * @date 2025-04-30
 *
 * Este arquivo define as funções para habilitar a criptografia AES-256
 * para save states do NES, permitindo integração com o sistema unificado
 * de criptografia.
 */

#ifndef NES_SAVE_STATE_CRYPTO_H
#define NES_SAVE_STATE_CRYPTO_H

#include <stdbool.h>
#include "../../../core/save_state.h"
#include "../../../core/save_state_crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ativa a criptografia para os save states do NES
 *
 * Esta função configura a criptografia para o sistema de save states do NES,
 * usando o algoritmo AES-256 com a senha fornecida. A senha será usada para
 * derivar uma chave criptográfica segura através de PBKDF2.
 *
 * @param state Contexto do save state
 * @param password Senha para criptografia
 * @return true se a criptografia foi ativada com sucesso, false caso contrário
 */
bool nes_save_state_enable_encryption(emu_save_state_t *state, const char *password);

/**
 * @brief Desativa a criptografia para os save states do NES
 *
 * @param state Contexto do save state
 * @return true se a criptografia foi desativada com sucesso, false caso contrário
 */
bool nes_save_state_disable_encryption(emu_save_state_t *state);

/**
 * @brief Verifica se um save state está criptografado
 *
 * @param filepath Caminho para o arquivo de save state
 * @return true se o arquivo está criptografado, false caso contrário
 */
bool nes_save_state_is_encrypted(const char *filepath);

/**
 * @brief Exporta uma chave de criptografia para um arquivo
 *
 * Esta função exporta a chave de criptografia para um arquivo protegido por senha.
 * Isso permite transferir a capacidade de descriptografia para outro dispositivo
 * sem comprometer a senha original.
 *
 * @param state Contexto do save state
 * @param key_file Caminho para o arquivo de chave a ser criado
 * @param key_password Senha para proteger o arquivo de chave
 * @return true se a exportação foi bem-sucedida, false caso contrário
 */
bool nes_save_state_export_key(emu_save_state_t *state, const char *key_file, const char *key_password);

/**
 * @brief Importa uma chave de criptografia de um arquivo
 *
 * Esta função importa uma chave de criptografia de um arquivo protegido por senha
 * e a configura para o contexto de save state fornecido.
 *
 * @param state Contexto do save state
 * @param key_file Caminho para o arquivo de chave
 * @param key_password Senha do arquivo de chave
 * @return true se a importação foi bem-sucedida, false caso contrário
 */
bool nes_save_state_import_key(emu_save_state_t *state, const char *key_file, const char *key_password);

#ifdef __cplusplus
}
#endif

#endif /* NES_SAVE_STATE_CRYPTO_H */
