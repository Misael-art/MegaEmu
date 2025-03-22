/**
 * @file sn76489.c
 * @brief Implementação da emulação do chip de som SN76489 (PSG) do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include "sn76489.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../../../utils/log_utils.h"

// Constantes
#define SN76489_NOISE_FEEDBACK_PATTERN 0x0009  // Padrão de feedback para ruído periódico
#define SN76489_NOISE_FEEDBACK_WHITE   0x8000  // Padrão de feedback para ruído branco

// Definições de bits para registradores
#define SN76489_REG_LATCH_MASK    0x80  // Bit 7: 1=latch/data, 0=data
#define SN76489_REG_CHANNEL_MASK  0x60  // Bits 6-5: canal
#define SN76489_REG_TYPE_MASK     0x10  // Bit 4: 0=tom, 1=atenuação
#define SN76489_REG_DATA_MASK     0x0F  // Bits 3-0: dados

// Macros para extração de bits
#define SN76489_GET_CHANNEL(x)    (((x) & SN76489_REG_CHANNEL_MASK) >> 5)
#define SN76489_IS_LATCH(x)       (((x) & SN76489_REG_LATCH_MASK) != 0)
#define SN76489_IS_VOLUME(x)      (((x) & SN76489_REG_TYPE_MASK) != 0)
#define SN76489_GET_DATA(x)       ((x) & SN76489_REG_DATA_MASK)

/**
 * @brief Inicializa a tabela de volume
 * @param chip Ponteiro para a estrutura do chip
 */
static void init_volume_table(sn76489_t *chip)
{
    if (!chip) return;
    
    // Tabela de volume logarítmica
    // 0 = volume máximo, 15 = silêncio
    // Cada passo é aproximadamente -2dB
    
    for (int i = 0; i < 16; i++)
    {
        // Cálculo logarítmico para aproximar -2dB por passo
        // Volume máximo é 0x7FFF (16 bits signed)
        if (i == 15)
        {
            // Silêncio completo
            chip->volume_table[i] = 0;
        }
        else
        {
            double db = -2.0 * (double)i;
            double amplitude = pow(10.0, db / 20.0);
            chip->volume_table[i] = (int16_t)(amplitude * 0x7FFF);
        }
    }
    
    LOG_DEBUG("SN76489: Tabela de volume inicializada");
}

/**
 * @brief Inicializa um canal de tom
 * @param channel Ponteiro para o canal de tom
 */
static void init_tone_channel(sn76489_tone_channel_t *channel)
{
    if (!channel) return;
    
    channel->tone_reg = 0x400;     // Valor padrão para evitar divisão por zero
    channel->attenuation = 0x0F;   // Silêncio
    channel->counter = 0;
    channel->output = 0;
    channel->out_value = 0;
}

/**
 * @brief Inicializa o canal de ruído
 * @param channel Ponteiro para o canal de ruído
 */
static void init_noise_channel(sn76489_noise_channel_t *channel)
{
    if (!channel) return;
    
    channel->shift_rate = 0;       // Taxa de deslocamento padrão
    channel->fb_type = false;      // Ruído periódico por padrão
    channel->attenuation = 0x0F;   // Silêncio
    channel->counter = 0;
    channel->shift_reg = 0x8000;   // Valor inicial do registrador de deslocamento
    channel->out_value = 0;
}

/**
 * @brief Inicializa o chip SN76489
 */
emu_error_t sn76489_init(sn76489_t *chip, uint32_t clock, uint32_t rate)
{
    if (!chip)
    {
        LOG_ERROR("SN76489: Ponteiro nulo passado para inicialização");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Limpar estrutura
    memset(chip, 0, sizeof(sn76489_t));
    
    // Inicializar tabela de volume
    init_volume_table(chip);
    
    // Inicializar canais de tom
    for (int i = 0; i < 3; i++)
    {
        init_tone_channel(&chip->tone_channels[i]);
    }
    
    // Inicializar canal de ruído
    init_noise_channel(&chip->noise_channel);
    
    // Configurar clock e taxa de amostragem
    chip->clock = clock;
    chip->rate = rate;
    chip->clock_ratio = (float)clock / (float)rate;
    
    // Inicializar latch
    chip->latch = 0;
    
    // Inicializar configuração estéreo (todos os canais em ambos os lados)
    chip->stereo = 0xFF;
    
    LOG_INFO("SN76489 inicializado: clock=%u Hz, sample_rate=%u Hz", clock, rate);
    
    return EMU_ERROR_NONE;
}

/**
 * @brief Reseta o estado do chip SN76489
 */
emu_error_t sn76489_reset(sn76489_t *chip)
{
    if (!chip)
    {
        LOG_ERROR("SN76489: Ponteiro nulo passado para reset");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Salvar clock e taxa de amostragem
    uint32_t clock = chip->clock;
    uint32_t rate = chip->rate;
    float clock_ratio = chip->clock_ratio;
    int16_t volume_table[16];
    memcpy(volume_table, chip->volume_table, sizeof(volume_table));
    
    // Limpar estrutura
    memset(chip, 0, sizeof(sn76489_t));
    
    // Restaurar tabela de volume
    memcpy(chip->volume_table, volume_table, sizeof(volume_table));
    
    // Inicializar canais de tom
    for (int i = 0; i < 3; i++)
    {
        init_tone_channel(&chip->tone_channels[i]);
    }
    
    // Inicializar canal de ruído
    init_noise_channel(&chip->noise_channel);
    
    // Restaurar clock e taxa de amostragem
    chip->clock = clock;
    chip->rate = rate;
    chip->clock_ratio = clock_ratio;
    
    // Inicializar latch
    chip->latch = 0;
    
    // Inicializar configuração estéreo (todos os canais em ambos os lados)
    chip->stereo = 0xFF;
    
    LOG_INFO("SN76489 resetado");
    
    return EMU_ERROR_NONE;
}

/**
 * @brief Libera recursos utilizados pelo chip SN76489
 */
void sn76489_shutdown(sn76489_t *chip)
{
    // Não há recursos alocados dinamicamente para liberar
    if (chip)
    {
        LOG_INFO("SN76489 desligado");
    }
}

/**
 * @brief Escreve um valor no chip
 */
void sn76489_write(sn76489_t *chip, uint8_t data)
{
    if (!chip)
    {
        LOG_ERROR("SN76489: Ponteiro nulo passado para escrita");
        return;
    }
    
    LOG_DEBUG("SN76489: Escrita de valor 0x%02X", data);
    
    // Verificar se é um comando de latch/data
    if (SN76489_IS_LATCH(data))
    {
        // Atualizar registrador de latch
        chip->latch = data;
        
        uint8_t channel = SN76489_GET_CHANNEL(data);
        bool is_volume = SN76489_IS_VOLUME(data);
        uint8_t reg_data = SN76489_GET_DATA(data);
        
        if (channel < 3)  // Canais de tom (0, 1, 2)
        {
            if (!is_volume)  // Registrador de tom (frequência)
            {
                // Atualizar parte baixa do registrador de tom (4 bits)
                chip->tone_channels[channel].tone_reg = 
                    (chip->tone_channels[channel].tone_reg & 0x3F0) | reg_data;
                
                LOG_DEBUG("SN76489: Canal de tom %u, parte baixa do tom definida para %u", 
                         channel, reg_data);
            }
            else  // Registrador de volume
            {
                // Atualizar atenuação (4 bits)
                chip->tone_channels[channel].attenuation = reg_data;
                
                LOG_DEBUG("SN76489: Canal de tom %u, atenuação definida para %u", 
                         channel, reg_data);
            }
        }
        else if (channel == 3)  // Canal de ruído (3)
        {
            if (!is_volume)  // Registrador de controle de ruído
            {
                // Bits 0-1: taxa de deslocamento
                chip->noise_channel.shift_rate = reg_data & 0x03;
                
                // Bit 2: tipo de feedback (0=periódico, 1=ruído branco)
                chip->noise_channel.fb_type = (reg_data & 0x04) != 0;
                
                // Resetar registrador de deslocamento
                chip->noise_channel.shift_reg = 0x8000;
                
                LOG_DEBUG("SN76489: Canal de ruído, controle definido para %u (rate=%u, fb_type=%u)", 
                         reg_data, chip->noise_channel.shift_rate, chip->noise_channel.fb_type);
            }
            else  // Registrador de volume de ruído
            {
                // Atualizar atenuação (4 bits)
                chip->noise_channel.attenuation = reg_data;
                
                LOG_DEBUG("SN76489: Canal de ruído, atenuação definida para %u", 
                         reg_data);
            }
        }
    }
    else  // Comando de data (continuação de um latch anterior)
    {
        uint8_t channel = SN76489_GET_CHANNEL(chip->latch);
        bool is_volume = SN76489_IS_VOLUME(chip->latch);
        uint8_t reg_data = SN76489_GET_DATA(data);
        
        if (channel < 3 && !is_volume)  // Canais de tom, parte alta do tom
        {
            // Atualizar parte alta do registrador de tom (6 bits)
            chip->tone_channels[channel].tone_reg = 
                (chip->tone_channels[channel].tone_reg & 0x00F) | ((uint16_t)reg_data << 4);
            
            LOG_DEBUG("SN76489: Canal de tom %u, parte alta do tom definida para %u, valor completo=%u", 
                     channel, reg_data, chip->tone_channels[channel].tone_reg);
        }
        // Outros casos são ignorados, pois não há continuação para volume ou ruído
    }
}

/**
 * @brief Define a configuração estéreo (para versão estéreo do chip)
 */
void sn76489_set_stereo(sn76489_t *chip, uint8_t stereo_byte)
{
    if (!chip)
    {
        LOG_ERROR("SN76489: Ponteiro nulo passado para configuração estéreo");
        return;
    }
    
    chip->stereo = stereo_byte;
    
    LOG_DEBUG("SN76489: Configuração estéreo definida para 0x%02X", stereo_byte);
}

/**
 * @brief Define a taxa de amostragem
 */
void sn76489_set_sample_rate(sn76489_t *chip, uint32_t rate)
{
    if (!chip || rate == 0)
    {
        LOG_ERROR("SN76489: Parâmetros inválidos para definição de taxa de amostragem");
        return;
    }
    
    chip->rate = rate;
    chip->clock_ratio = (float)chip->clock / (float)rate;
    
    LOG_INFO("SN76489: Taxa de amostragem alterada para %u Hz", rate);
}

/**
 * @brief Define a frequência do clock
 */
void sn76489_set_clock(sn76489_t *chip, uint32_t clock)
{
    if (!chip || clock == 0)
    {
        LOG_ERROR("SN76489: Parâmetros inválidos para definição de clock");
        return;
    }
    
    chip->clock = clock;
    chip->clock_ratio = (float)clock / (float)chip->rate;
    
    LOG_INFO("SN76489: Clock alterado para %u Hz", clock);
}

/**
 * @brief Avança o SN76489 pelo número especificado de ciclos
 */
void sn76489_advance(sn76489_t *chip, uint32_t cycles)
{
    if (!chip)
    {
        return;
    }
    
    chip->cycles += cycles;
    
    // Calcular quantas amostras deveriam ter sido geradas
    uint32_t expected_samples = (uint32_t)((float)chip->cycles / chip->clock_ratio);
    
    // Atualizar contador de ciclos
    if (expected_samples > chip->samples_generated)
    {
        chip->samples_generated = expected_samples;
    }
}

/**
 * @brief Atualiza o canal de tom
 * @param channel Ponteiro para o canal de tom
 * @param clocks Número de clocks a avançar
 * @param volume_table Tabela de volume
 * @return Valor de saída do canal
 */
static int16_t update_tone_channel(sn76489_tone_channel_t *channel, uint32_t clocks, int16_t *volume_table)
{
    if (!channel || !volume_table)
    {
        return 0;
    }
    
    // Verificar se o tom é válido (evitar divisão por zero)
    if (channel->tone_reg < 2)
    {
        channel->tone_reg = 2;
    }
    
    // Avançar o contador
    channel->counter += clocks;
    
    // Verificar se é hora de alternar a saída
    while (channel->counter >= channel->tone_reg)
    {
        channel->counter -= channel->tone_reg;
        channel->output ^= 1;  // Alternar entre 0 e 1
    }
    
    // Calcular valor de saída baseado no estado atual e atenuação
    channel->out_value = channel->output ? volume_table[channel->attenuation] : -volume_table[channel->attenuation];
    
    return channel->out_value;
}

/**
 * @brief Atualiza o canal de ruído
 * @param channel Ponteiro para o canal de ruído
 * @param tone2_freq Frequência do canal de tom 2 (para taxa de deslocamento 3)
 * @param clocks Número de clocks a avançar
 * @param volume_table Tabela de volume
 * @return Valor de saída do canal
 */
static int16_t update_noise_channel(sn76489_noise_channel_t *channel, uint16_t tone2_freq, uint32_t clocks, int16_t *volume_table)
{
    if (!channel || !volume_table)
    {
        return 0;
    }
    
    // Determinar a frequência baseada na taxa de deslocamento
    uint16_t freq;
    switch (channel->shift_rate)
    {
        case 0:  // N/512
            freq = 0x10;
            break;
        case 1:  // N/1024
            freq = 0x20;
            break;
        case 2:  // N/2048
            freq = 0x40;
            break;
        case 3:  // Frequência do canal de tom 2
            freq = tone2_freq;
            break;
        default:
            freq = 0x10;  // Valor padrão
    }
    
    // Verificar se a frequência é válida (evitar divisão por zero)
    if (freq < 2)
    {
        freq = 2;
    }
    
    // Avançar o contador
    channel->counter += clocks;
    
    // Verificar se é hora de deslocar o registrador
    while (channel->counter >= freq)
    {
        channel->counter -= freq;
        
        // Calcular bit de feedback
        uint16_t feedback_bit;
        if (channel->fb_type)
        {
            // Ruído branco: XOR dos bits 0 e 3
            feedback_bit = ((channel->shift_reg & 0x0001) ^ ((channel->shift_reg & 0x0008) >> 3)) & 0x0001;
        }
        else
        {
            // Ruído periódico: apenas bit 0
            feedback_bit = channel->shift_reg & 0x0001;
        }
        
        // Deslocar o registrador
        channel->shift_reg = (channel->shift_reg >> 1) | (feedback_bit << 15);
    }
    
    // Calcular valor de saída baseado no bit 0 do registrador de deslocamento
    bool output = (channel->shift_reg & 0x0001) != 0;
    channel->out_value = output ? volume_table[channel->attenuation] : -volume_table[channel->attenuation];
    
    return channel->out_value;
}

/**
 * @brief Atualiza o estado do chip e gera amostras
 */
int32_t sn76489_update(sn76489_t *chip, int16_t *buffer_left, int16_t *buffer_right, int32_t num_samples)
{
    if (!chip || !buffer_left || !buffer_right || num_samples <= 0)
    {
        LOG_ERROR("SN76489: Parâmetros inválidos para atualização");
        return 0;
    }
    
    // Calcular número de clocks por amostra
    float clocks_per_sample = chip->clock_ratio;
    
    // Processar cada amostra
    for (int32_t i = 0; i < num_samples; i++)
    {
        // Número de clocks para esta amostra
        uint32_t clocks = (uint32_t)clocks_per_sample;
        
        // Atualizar canais de tom
        int16_t tone_outputs[3];
        for (int ch = 0; ch < 3; ch++)
        {
            tone_outputs[ch] = update_tone_channel(&chip->tone_channels[ch], clocks, chip->volume_table);
        }
        
        // Atualizar canal de ruído usando a frequência do canal de tom 2
        int16_t noise_output = update_noise_channel(&chip->noise_channel, chip->tone_channels[2].tone_reg, clocks, chip->volume_table);
        
        // Misturar saídas para os canais estéreo
        int32_t output_left = 0;
        int32_t output_right = 0;
        
        // Aplicar configuração estéreo para canais de tom
        for (int ch = 0; ch < 3; ch++)
        {
            if (chip->stereo & (1 << ch))
            {
                output_left += tone_outputs[ch];
            }
            
            if (chip->stereo & (1 << (ch + 4)))
            {
                output_right += tone_outputs[ch];
            }
        }
        
        // Aplicar configuração estéreo para canal de ruído
        if (chip->stereo & (1 << 3))
        {
            output_left += noise_output;
        }
        
        if (chip->stereo & (1 << 7))
        {
            output_right += noise_output;
        }
        
        // Limitar e converter para 16-bit
        output_left = (output_left > 32767) ? 32767 : ((output_left < -32768) ? -32768 : output_left);
        output_right = (output_right > 32767) ? 32767 : ((output_right < -32768) ? -32768 : output_right);
        
        buffer_left[i] = (int16_t)output_left;
        buffer_right[i] = (int16_t)output_right;
    }
    
    // Atualizar contador de amostras
    chip->samples_generated += num_samples;
    
    return num_samples;
}
