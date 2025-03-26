/**
 * @file audio_control.h
 * @brief Interface principal para controle de canais de áudio
 */

#ifndef MEGA_EMU_AUDIO_CONTROL_H
#define MEGA_EMU_AUDIO_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "audio_control_types.h"
#include "../audio/audio_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa o sistema de controle de canais de áudio
 *
 * @param audio Interface de áudio do emulador
 * @param platform Plataforma alvo
 * @return true se inicializado com sucesso, false caso contrário
 */
bool mega_emu_audio_control_init(void* audio, mega_emu_audio_platform_t platform);

/**
 * @brief Finaliza o sistema de controle de canais de áudio e libera recursos
 */
void mega_emu_audio_control_shutdown(void);

/**
 * @brief Registra um callback para ser notificado sobre mudanças nos canais
 *
 * @param callback Função de callback
 * @param user_data Dados do usuário passados para o callback
 * @return int ID do callback ou -1 em caso de erro
 */
int mega_emu_audio_control_register_callback(
    mega_emu_audio_channel_callback_t callback,
    void* user_data
);

/**
 * @brief Remove um callback registrado
 *
 * @param callback_id ID do callback
 * @return true se removido com sucesso, false caso contrário
 */
bool mega_emu_audio_control_unregister_callback(int callback_id);

/**
 * @brief Habilita ou desabilita um canal de áudio
 *
 * @param channel Canal de áudio
 * @param enabled Estado desejado (true para habilitado, false para desabilitado)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_audio_control_set_channel_enabled(
    mega_emu_audio_channel_t channel,
    bool enabled
);

/**
 * @brief Verifica se um canal está habilitado
 *
 * @param channel Canal de áudio
 * @return true se o canal está habilitado, false caso contrário
 */
bool mega_emu_audio_control_is_channel_enabled(mega_emu_audio_channel_t channel);

/**
 * @brief Define o volume de um canal de áudio
 *
 * @param channel Canal de áudio
 * @param volume Volume (0-255)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_audio_control_set_channel_volume(
    mega_emu_audio_channel_t channel,
    uint8_t volume
);

/**
 * @brief Obtém o volume atual de um canal de áudio
 *
 * @param channel Canal de áudio
 * @return Volume atual (0-255) ou 0 em caso de erro
 */
uint8_t mega_emu_audio_control_get_channel_volume(mega_emu_audio_channel_t channel);

/**
 * @brief Silencia ou remove o silenciamento de um canal de áudio
 *
 * @param channel Canal de áudio
 * @param muted Estado desejado (true para silenciado, false para não silenciado)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_audio_control_set_channel_muted(
    mega_emu_audio_channel_t channel,
    bool muted
);

/**
 * @brief Verifica se um canal está silenciado
 *
 * @param channel Canal de áudio
 * @return true se o canal está silenciado, false caso contrário
 */
bool mega_emu_audio_control_is_channel_muted(mega_emu_audio_channel_t channel);

/**
 * @brief Ativa ou desativa o modo solo para um canal de áudio
 *
 * No modo solo, apenas o canal especificado é tocado, todos os outros
 * são silenciados temporariamente.
 *
 * @param channel Canal de áudio
 * @param solo Estado desejado (true para solo, false para normal)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_audio_control_set_channel_solo(
    mega_emu_audio_channel_t channel,
    bool solo
);

/**
 * @brief Verifica se um canal está em modo solo
 *
 * @param channel Canal de áudio
 * @return true se o canal está em modo solo, false caso contrário
 */
bool mega_emu_audio_control_is_channel_solo(mega_emu_audio_channel_t channel);

/**
 * @brief Obtém informações sobre um canal de áudio
 *
 * @param channel Canal de áudio
 * @param state Ponteiro para estrutura que receberá as informações
 * @return true se obtido com sucesso, false caso contrário
 */
bool mega_emu_audio_control_get_channel_state(
    mega_emu_audio_channel_t channel,
    mega_emu_audio_channel_state_t* state
);

/**
 * @brief Atualiza o buffer de forma de onda para um canal
 *
 * Esta função deve ser chamada pelo emulador para cada canal de áudio
 * quando novos dados de áudio são gerados.
 *
 * @param channel Canal de áudio
 * @param samples Ponteiro para array de samples
 * @param num_samples Número de samples
 * @return true se atualizado com sucesso, false caso contrário
 */
bool mega_emu_audio_control_update_wave_buffer(
    mega_emu_audio_channel_t channel,
    const int16_t* samples,
    uint32_t num_samples
);

/**
 * @brief Obtém o nome legível de um canal de áudio
 *
 * @param channel Canal de áudio
 * @return Ponteiro para string com o nome ou NULL em caso de erro
 */
const char* mega_emu_audio_control_get_channel_name(mega_emu_audio_channel_t channel);

/**
 * @brief Obtém a lista de canais disponíveis para uma plataforma
 *
 * @param channels Array para armazenar os canais disponíveis
 * @param max_channels Tamanho máximo do array
 * @return Número de canais disponíveis
 */
uint32_t mega_emu_audio_control_get_available_channels(
    mega_emu_audio_channel_t* channels,
    uint32_t max_channels
);

/**
 * @brief Reseta o estado de todos os canais para seus valores padrão
 *
 * @return true se resetado com sucesso, false caso contrário
 */
bool mega_emu_audio_control_reset_all_channels(void);

/**
 * @brief Salva configurações de canais de áudio em um arquivo
 *
 * @param filename Caminho para o arquivo
 * @return true se salvo com sucesso, false caso contrário
 */
bool mega_emu_audio_control_save_config(const char* filename);

/**
 * @brief Carrega configurações de canais de áudio a partir de um arquivo
 *
 * @param filename Caminho para o arquivo
 * @return true se carregado com sucesso, false caso contrário
 */
bool mega_emu_audio_control_load_config(const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_AUDIO_CONTROL_H */
