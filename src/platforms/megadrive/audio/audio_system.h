/**
 * @file audio_system.h
 * @brief Sistema de áudio completo do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 * 
 * Este arquivo define a interface para o sistema de áudio completo do Mega Drive,
 * integrando os chips YM2612 (FM) e SN76489 (PSG).
 */

#ifndef EMU_MEGADRIVE_AUDIO_SYSTEM_H
#define EMU_MEGADRIVE_AUDIO_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../utils/common_types.h"
#include "ym2612.h"
#include "sn76489.h"

// Definição de constantes
#define MD_AUDIO_SAMPLE_RATE 44100   // Taxa de amostragem padrão
#define MD_AUDIO_BUFFER_SIZE 2048    // Tamanho do buffer de áudio
#define MD_AUDIO_CHANNELS 2          // Número de canais (estéreo)

// Estrutura para o sistema de áudio do Mega Drive
typedef struct {
    // Chips de áudio
    ym2612_t ym2612;     // Chip FM (YM2612)
    sn76489_t sn76489;   // Chip PSG (SN76489)
    
    // Configurações
    uint32_t sample_rate;            // Taxa de amostragem atual
    float ym2612_volume;             // Volume do YM2612 (0.0 - 1.0)
    float sn76489_volume;            // Volume do SN76489 (0.0 - 1.0)
    float master_volume;             // Volume master (0.0 - 1.0)
    
    // Buffers de áudio
    int16_t *buffer_left;            // Buffer para o canal esquerdo
    int16_t *buffer_right;           // Buffer para o canal direito
    int16_t *ym2612_buffer_left;     // Buffer temporário para YM2612 (esquerdo)
    int16_t *ym2612_buffer_right;    // Buffer temporário para YM2612 (direito)
    int16_t *sn76489_buffer_left;    // Buffer temporário para SN76489 (esquerdo)
    int16_t *sn76489_buffer_right;   // Buffer temporário para SN76489 (direito)
    uint32_t buffer_size;            // Tamanho atual do buffer
    
    // Estado
    bool enabled;                    // Sistema de áudio habilitado
    uint32_t samples_generated;      // Contador de amostras geradas
    uint32_t cycles;                 // Ciclos acumulados
    
    // Configurações de clock
    uint32_t system_clock;           // Clock do sistema em Hz
    float cycles_per_sample;         // Ciclos por amostra
} md_audio_system_t;

/**
 * @brief Inicializa o sistema de áudio do Mega Drive
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param system_clock Frequência do clock do sistema em Hz
 * @param sample_rate Taxa de amostragem em Hz
 * @return emu_error_t EMU_ERROR_NONE se bem-sucedido
 */
emu_error_t md_audio_init(md_audio_system_t *audio, uint32_t system_clock, uint32_t sample_rate);

/**
 * @brief Reseta o estado do sistema de áudio
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @return emu_error_t EMU_ERROR_NONE se bem-sucedido
 */
emu_error_t md_audio_reset(md_audio_system_t *audio);

/**
 * @brief Libera recursos utilizados pelo sistema de áudio
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 */
void md_audio_shutdown(md_audio_system_t *audio);

/**
 * @brief Escreve um valor no chip YM2612
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param port Porta (0 ou 1)
 * @param address Endereço do registrador
 * @param data Byte de dados a ser escrito
 */
void md_audio_write_ym2612(md_audio_system_t *audio, uint8_t port, uint8_t address, uint8_t data);

/**
 * @brief Lê um valor do chip YM2612
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param port Porta (0 ou 1)
 * @param address Endereço do registrador
 * @return uint8_t Valor lido
 */
uint8_t md_audio_read_ym2612(md_audio_system_t *audio, uint8_t port, uint8_t address);

/**
 * @brief Escreve um valor no chip SN76489
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param data Byte de dados a ser escrito
 */
void md_audio_write_sn76489(md_audio_system_t *audio, uint8_t data);

/**
 * @brief Define a configuração estéreo do SN76489
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param stereo_byte Byte de configuração estéreo
 */
void md_audio_set_sn76489_stereo(md_audio_system_t *audio, uint8_t stereo_byte);

/**
 * @brief Define a taxa de amostragem
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param sample_rate Nova taxa de amostragem em Hz
 */
void md_audio_set_sample_rate(md_audio_system_t *audio, uint32_t sample_rate);

/**
 * @brief Define o volume do YM2612
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param volume Volume (0.0 - 1.0)
 */
void md_audio_set_ym2612_volume(md_audio_system_t *audio, float volume);

/**
 * @brief Define o volume do SN76489
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param volume Volume (0.0 - 1.0)
 */
void md_audio_set_sn76489_volume(md_audio_system_t *audio, float volume);

/**
 * @brief Define o volume master
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param volume Volume (0.0 - 1.0)
 */
void md_audio_set_master_volume(md_audio_system_t *audio, float volume);

/**
 * @brief Habilita ou desabilita o sistema de áudio
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param enabled true para habilitar, false para desabilitar
 */
void md_audio_set_enabled(md_audio_system_t *audio, bool enabled);

/**
 * @brief Avança o sistema de áudio pelo número especificado de ciclos
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param cycles Número de ciclos para avançar
 */
void md_audio_advance(md_audio_system_t *audio, uint32_t cycles);

/**
 * @brief Atualiza o sistema de áudio e gera amostras
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param buffer_left Buffer para o canal esquerdo
 * @param buffer_right Buffer para o canal direito
 * @param num_samples Número de amostras a gerar
 * @return int32_t Número de amostras geradas
 */
int32_t md_audio_update(md_audio_system_t *audio, int16_t *buffer_left, int16_t *buffer_right, int32_t num_samples);

/**
 * @brief Obtém o número de amostras geradas
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @return uint32_t Número de amostras geradas
 */
uint32_t md_audio_get_samples_generated(md_audio_system_t *audio);

/**
 * @brief Redefine o tamanho do buffer de áudio
 * 
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param buffer_size Novo tamanho do buffer
 * @return emu_error_t EMU_ERROR_NONE se bem-sucedido
 */
emu_error_t md_audio_resize_buffer(md_audio_system_t *audio, uint32_t buffer_size);

#endif // EMU_MEGADRIVE_AUDIO_SYSTEM_H
