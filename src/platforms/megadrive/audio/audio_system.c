/**
 * @file audio_system.c
 * @brief Implementação do sistema de áudio completo do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include "audio_system.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../../../utils/log_utils.h"

/**
 * @brief Inicializa os buffers de áudio
 * @param audio Ponteiro para a estrutura do sistema de áudio
 * @param buffer_size Tamanho do buffer
 * @return emu_error_t EMU_ERROR_NONE se bem-sucedido
 */
static emu_error_t init_audio_buffers(md_audio_system_t *audio, uint32_t buffer_size)
{
    if (!audio || buffer_size == 0)
    {
        LOG_ERROR("Audio System: Parâmetros inválidos para inicialização de buffers");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Liberar buffers existentes, se houver
    if (audio->buffer_left)
    {
        free(audio->buffer_left);
        audio->buffer_left = NULL;
    }
    
    if (audio->buffer_right)
    {
        free(audio->buffer_right);
        audio->buffer_right = NULL;
    }
    
    if (audio->ym2612_buffer_left)
    {
        free(audio->ym2612_buffer_left);
        audio->ym2612_buffer_left = NULL;
    }
    
    if (audio->ym2612_buffer_right)
    {
        free(audio->ym2612_buffer_right);
        audio->ym2612_buffer_right = NULL;
    }
    
    if (audio->sn76489_buffer_left)
    {
        free(audio->sn76489_buffer_left);
        audio->sn76489_buffer_left = NULL;
    }
    
    if (audio->sn76489_buffer_right)
    {
        free(audio->sn76489_buffer_right);
        audio->sn76489_buffer_right = NULL;
    }
    
    // Alocar novos buffers
    audio->buffer_left = (int16_t*)malloc(buffer_size * sizeof(int16_t));
    audio->buffer_right = (int16_t*)malloc(buffer_size * sizeof(int16_t));
    audio->ym2612_buffer_left = (int16_t*)malloc(buffer_size * sizeof(int16_t));
    audio->ym2612_buffer_right = (int16_t*)malloc(buffer_size * sizeof(int16_t));
    audio->sn76489_buffer_left = (int16_t*)malloc(buffer_size * sizeof(int16_t));
    audio->sn76489_buffer_right = (int16_t*)malloc(buffer_size * sizeof(int16_t));
    
    // Verificar se a alocação foi bem-sucedida
    if (!audio->buffer_left || !audio->buffer_right || 
        !audio->ym2612_buffer_left || !audio->ym2612_buffer_right ||
        !audio->sn76489_buffer_left || !audio->sn76489_buffer_right)
    {
        LOG_ERROR("Audio System: Falha na alocação de buffers");
        
        // Liberar buffers alocados
        if (audio->buffer_left)
        {
            free(audio->buffer_left);
            audio->buffer_left = NULL;
        }
        
        if (audio->buffer_right)
        {
            free(audio->buffer_right);
            audio->buffer_right = NULL;
        }
        
        if (audio->ym2612_buffer_left)
        {
            free(audio->ym2612_buffer_left);
            audio->ym2612_buffer_left = NULL;
        }
        
        if (audio->ym2612_buffer_right)
        {
            free(audio->ym2612_buffer_right);
            audio->ym2612_buffer_right = NULL;
        }
        
        if (audio->sn76489_buffer_left)
        {
            free(audio->sn76489_buffer_left);
            audio->sn76489_buffer_left = NULL;
        }
        
        if (audio->sn76489_buffer_right)
        {
            free(audio->sn76489_buffer_right);
            audio->sn76489_buffer_right = NULL;
        }
        
        return EMU_ERROR_MEMORY;
    }
    
    // Limpar buffers
    memset(audio->buffer_left, 0, buffer_size * sizeof(int16_t));
    memset(audio->buffer_right, 0, buffer_size * sizeof(int16_t));
    memset(audio->ym2612_buffer_left, 0, buffer_size * sizeof(int16_t));
    memset(audio->ym2612_buffer_right, 0, buffer_size * sizeof(int16_t));
    memset(audio->sn76489_buffer_left, 0, buffer_size * sizeof(int16_t));
    memset(audio->sn76489_buffer_right, 0, buffer_size * sizeof(int16_t));
    
    // Atualizar tamanho do buffer
    audio->buffer_size = buffer_size;
    
    LOG_DEBUG("Audio System: Buffers inicializados com tamanho %u", buffer_size);
    
    return EMU_ERROR_NONE;
}

/**
 * @brief Inicializa o sistema de áudio do Mega Drive
 */
emu_error_t md_audio_init(md_audio_system_t *audio, uint32_t system_clock, uint32_t sample_rate)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para inicialização");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Limpar estrutura
    memset(audio, 0, sizeof(md_audio_system_t));
    
    // Configurar parâmetros
    audio->system_clock = system_clock;
    audio->sample_rate = sample_rate;
    audio->cycles_per_sample = (float)system_clock / (float)sample_rate;
    
    // Configurar volumes
    audio->ym2612_volume = 0.8f;    // 80% para YM2612
    audio->sn76489_volume = 0.6f;   // 60% para SN76489
    audio->master_volume = 1.0f;    // 100% para master
    
    // Inicializar buffers
    emu_error_t error = init_audio_buffers(audio, MD_AUDIO_BUFFER_SIZE);
    if (error != EMU_ERROR_NONE)
    {
        LOG_ERROR("Audio System: Falha na inicialização de buffers");
        return error;
    }
    
    // Inicializar chips de áudio
    error = ym2612_init(&audio->ym2612, system_clock, sample_rate);
    if (error != EMU_ERROR_NONE)
    {
        LOG_ERROR("Audio System: Falha na inicialização do YM2612");
        return error;
    }
    
    error = sn76489_init(&audio->sn76489, system_clock / 4, sample_rate); // SN76489 opera a 1/4 do clock principal
    if (error != EMU_ERROR_NONE)
    {
        LOG_ERROR("Audio System: Falha na inicialização do SN76489");
        return error;
    }
    
    // Habilitar sistema de áudio
    audio->enabled = true;
    
    LOG_INFO("Audio System: Inicializado com clock=%u Hz, sample_rate=%u Hz", system_clock, sample_rate);
    
    return EMU_ERROR_NONE;
}

/**
 * @brief Reseta o estado do sistema de áudio
 */
emu_error_t md_audio_reset(md_audio_system_t *audio)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para reset");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Resetar chips de áudio
    emu_error_t error = ym2612_reset(&audio->ym2612);
    if (error != EMU_ERROR_NONE)
    {
        LOG_ERROR("Audio System: Falha no reset do YM2612");
        return error;
    }
    
    error = sn76489_reset(&audio->sn76489);
    if (error != EMU_ERROR_NONE)
    {
        LOG_ERROR("Audio System: Falha no reset do SN76489");
        return error;
    }
    
    // Limpar buffers
    if (audio->buffer_left && audio->buffer_right && 
        audio->ym2612_buffer_left && audio->ym2612_buffer_right &&
        audio->sn76489_buffer_left && audio->sn76489_buffer_right)
    {
        memset(audio->buffer_left, 0, audio->buffer_size * sizeof(int16_t));
        memset(audio->buffer_right, 0, audio->buffer_size * sizeof(int16_t));
        memset(audio->ym2612_buffer_left, 0, audio->buffer_size * sizeof(int16_t));
        memset(audio->ym2612_buffer_right, 0, audio->buffer_size * sizeof(int16_t));
        memset(audio->sn76489_buffer_left, 0, audio->buffer_size * sizeof(int16_t));
        memset(audio->sn76489_buffer_right, 0, audio->buffer_size * sizeof(int16_t));
    }
    
    // Resetar contadores
    audio->samples_generated = 0;
    audio->cycles = 0;
    
    LOG_INFO("Audio System: Resetado");
    
    return EMU_ERROR_NONE;
}

/**
 * @brief Libera recursos utilizados pelo sistema de áudio
 */
void md_audio_shutdown(md_audio_system_t *audio)
{
    if (!audio)
    {
        return;
    }
    
    // Desligar chips de áudio
    ym2612_shutdown(&audio->ym2612);
    sn76489_shutdown(&audio->sn76489);
    
    // Liberar buffers
    if (audio->buffer_left)
    {
        free(audio->buffer_left);
        audio->buffer_left = NULL;
    }
    
    if (audio->buffer_right)
    {
        free(audio->buffer_right);
        audio->buffer_right = NULL;
    }
    
    if (audio->ym2612_buffer_left)
    {
        free(audio->ym2612_buffer_left);
        audio->ym2612_buffer_left = NULL;
    }
    
    if (audio->ym2612_buffer_right)
    {
        free(audio->ym2612_buffer_right);
        audio->ym2612_buffer_right = NULL;
    }
    
    if (audio->sn76489_buffer_left)
    {
        free(audio->sn76489_buffer_left);
        audio->sn76489_buffer_left = NULL;
    }
    
    if (audio->sn76489_buffer_right)
    {
        free(audio->sn76489_buffer_right);
        audio->sn76489_buffer_right = NULL;
    }
    
    LOG_INFO("Audio System: Desligado");
}

/**
 * @brief Escreve um valor no chip YM2612
 */
void md_audio_write_ym2612(md_audio_system_t *audio, uint8_t port, uint8_t address, uint8_t data)
{
    if (!audio || port > 1)
    {
        LOG_ERROR("Audio System: Parâmetros inválidos para escrita no YM2612");
        return;
    }
    
    ym2612_write(&audio->ym2612, port, address, data);
}

/**
 * @brief Lê um valor do chip YM2612
 */
uint8_t md_audio_read_ym2612(md_audio_system_t *audio, uint8_t port, uint8_t address)
{
    if (!audio || port > 1)
    {
        LOG_ERROR("Audio System: Parâmetros inválidos para leitura do YM2612");
        return 0;
    }
    
    return ym2612_read(&audio->ym2612, port, address);
}

/**
 * @brief Escreve um valor no chip SN76489
 */
void md_audio_write_sn76489(md_audio_system_t *audio, uint8_t data)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para escrita no SN76489");
        return;
    }
    
    sn76489_write(&audio->sn76489, data);
}

/**
 * @brief Define a configuração estéreo do SN76489
 */
void md_audio_set_sn76489_stereo(md_audio_system_t *audio, uint8_t stereo_byte)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para configuração estéreo");
        return;
    }
    
    sn76489_set_stereo(&audio->sn76489, stereo_byte);
}

/**
 * @brief Define a taxa de amostragem
 */
void md_audio_set_sample_rate(md_audio_system_t *audio, uint32_t sample_rate)
{
    if (!audio || sample_rate == 0)
    {
        LOG_ERROR("Audio System: Parâmetros inválidos para definição de taxa de amostragem");
        return;
    }
    
    // Atualizar taxa de amostragem
    audio->sample_rate = sample_rate;
    audio->cycles_per_sample = (float)audio->system_clock / (float)sample_rate;
    
    // Atualizar chips de áudio
    ym2612_set_sample_rate(&audio->ym2612, sample_rate);
    sn76489_set_sample_rate(&audio->sn76489, sample_rate);
    
    LOG_INFO("Audio System: Taxa de amostragem alterada para %u Hz", sample_rate);
}

/**
 * @brief Define o volume do YM2612
 */
void md_audio_set_ym2612_volume(md_audio_system_t *audio, float volume)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para definição de volume");
        return;
    }
    
    // Limitar volume entre 0.0 e 1.0
    if (volume < 0.0f)
    {
        volume = 0.0f;
    }
    else if (volume > 1.0f)
    {
        volume = 1.0f;
    }
    
    audio->ym2612_volume = volume;
    
    LOG_DEBUG("Audio System: Volume do YM2612 definido para %.2f", volume);
}

/**
 * @brief Define o volume do SN76489
 */
void md_audio_set_sn76489_volume(md_audio_system_t *audio, float volume)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para definição de volume");
        return;
    }
    
    // Limitar volume entre 0.0 e 1.0
    if (volume < 0.0f)
    {
        volume = 0.0f;
    }
    else if (volume > 1.0f)
    {
        volume = 1.0f;
    }
    
    audio->sn76489_volume = volume;
    
    LOG_DEBUG("Audio System: Volume do SN76489 definido para %.2f", volume);
}

/**
 * @brief Define o volume master
 */
void md_audio_set_master_volume(md_audio_system_t *audio, float volume)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para definição de volume");
        return;
    }
    
    // Limitar volume entre 0.0 e 1.0
    if (volume < 0.0f)
    {
        volume = 0.0f;
    }
    else if (volume > 1.0f)
    {
        volume = 1.0f;
    }
    
    audio->master_volume = volume;
    
    LOG_DEBUG("Audio System: Volume master definido para %.2f", volume);
}

/**
 * @brief Habilita ou desabilita o sistema de áudio
 */
void md_audio_set_enabled(md_audio_system_t *audio, bool enabled)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para habilitação");
        return;
    }
    
    audio->enabled = enabled;
    
    LOG_INFO("Audio System: %s", enabled ? "Habilitado" : "Desabilitado");
}

/**
 * @brief Avança o sistema de áudio pelo número especificado de ciclos
 */
void md_audio_advance(md_audio_system_t *audio, uint32_t cycles)
{
    if (!audio || !audio->enabled)
    {
        return;
    }
    
    // Avançar chips de áudio
    ym2612_advance(&audio->ym2612, cycles);
    sn76489_advance(&audio->sn76489, cycles / 4); // SN76489 opera a 1/4 do clock principal
    
    // Atualizar contador de ciclos
    audio->cycles += cycles;
    
    // Calcular quantas amostras deveriam ter sido geradas
    uint32_t expected_samples = (uint32_t)((float)audio->cycles / audio->cycles_per_sample);
    
    // Atualizar contador de amostras
    if (expected_samples > audio->samples_generated)
    {
        audio->samples_generated = expected_samples;
    }
}

/**
 * @brief Atualiza o sistema de áudio e gera amostras
 */
int32_t md_audio_update(md_audio_system_t *audio, int16_t *buffer_left, int16_t *buffer_right, int32_t num_samples)
{
    if (!audio || !buffer_left || !buffer_right || num_samples <= 0 || !audio->enabled)
    {
        LOG_ERROR("Audio System: Parâmetros inválidos para atualização");
        return 0;
    }
    
    // Verificar se o número de amostras é maior que o tamanho do buffer
    if ((uint32_t)num_samples > audio->buffer_size)
    {
        LOG_ERROR("Audio System: Número de amostras excede o tamanho do buffer");
        num_samples = audio->buffer_size;
    }
    
    // Limpar buffers temporários
    memset(audio->ym2612_buffer_left, 0, num_samples * sizeof(int16_t));
    memset(audio->ym2612_buffer_right, 0, num_samples * sizeof(int16_t));
    memset(audio->sn76489_buffer_left, 0, num_samples * sizeof(int16_t));
    memset(audio->sn76489_buffer_right, 0, num_samples * sizeof(int16_t));
    
    // Gerar amostras para cada chip
    ym2612_update(&audio->ym2612, audio->ym2612_buffer_left, audio->ym2612_buffer_right, num_samples);
    sn76489_update(&audio->sn76489, audio->sn76489_buffer_left, audio->sn76489_buffer_right, num_samples);
    
    // Mixar amostras com os volumes apropriados
    for (int32_t i = 0; i < num_samples; i++)
    {
        // Aplicar volumes individuais
        int32_t ym2612_left = (int32_t)((float)audio->ym2612_buffer_left[i] * audio->ym2612_volume);
        int32_t ym2612_right = (int32_t)((float)audio->ym2612_buffer_right[i] * audio->ym2612_volume);
        int32_t sn76489_left = (int32_t)((float)audio->sn76489_buffer_left[i] * audio->sn76489_volume);
        int32_t sn76489_right = (int32_t)((float)audio->sn76489_buffer_right[i] * audio->sn76489_volume);
        
        // Somar contribuições
        int32_t mixed_left = ym2612_left + sn76489_left;
        int32_t mixed_right = ym2612_right + sn76489_right;
        
        // Aplicar volume master
        mixed_left = (int32_t)((float)mixed_left * audio->master_volume);
        mixed_right = (int32_t)((float)mixed_right * audio->master_volume);
        
        // Limitar valores para o intervalo de 16 bits
        if (mixed_left > 32767)
        {
            mixed_left = 32767;
        }
        else if (mixed_left < -32768)
        {
            mixed_left = -32768;
        }
        
        if (mixed_right > 32767)
        {
            mixed_right = 32767;
        }
        else if (mixed_right < -32768)
        {
            mixed_right = -32768;
        }
        
        // Armazenar no buffer de saída
        buffer_left[i] = (int16_t)mixed_left;
        buffer_right[i] = (int16_t)mixed_right;
    }
    
    // Atualizar contador de amostras
    audio->samples_generated += num_samples;
    
    return num_samples;
}

/**
 * @brief Obtém o número de amostras geradas
 */
uint32_t md_audio_get_samples_generated(md_audio_system_t *audio)
{
    if (!audio)
    {
        LOG_ERROR("Audio System: Ponteiro nulo passado para obtenção de amostras");
        return 0;
    }
    
    return audio->samples_generated;
}

/**
 * @brief Redefine o tamanho do buffer de áudio
 */
emu_error_t md_audio_resize_buffer(md_audio_system_t *audio, uint32_t buffer_size)
{
    if (!audio || buffer_size == 0)
    {
        LOG_ERROR("Audio System: Parâmetros inválidos para redimensionamento de buffer");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Inicializar novos buffers
    return init_audio_buffers(audio, buffer_size);
}
