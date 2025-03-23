/**
 * @file sms_psg.c
 * @brief Implementação do Programmable Sound Generator (PSG) do Master System
 */

#include "sms_psg.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../../../core/save_state.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Definir a categoria de log para o PSG do Master System
#define EMU_LOG_CAT_SMS_PSG EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para o PSG do Master System
#define SMS_PSG_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_SMS_PSG, __VA_ARGS__)
#define SMS_PSG_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_SMS_PSG, __VA_ARGS__)
#define SMS_PSG_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_SMS_PSG, __VA_ARGS__)
#define SMS_PSG_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_SMS_PSG, __VA_ARGS__)
#define SMS_PSG_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_SMS_PSG, __VA_ARGS__)

// Configurações do PSG
#define PSG_CLOCK 3579545     // Frequência do clock do PSG (Hz)
#define PSG_CHANNELS 4        // Número de canais (3 tons, 1 ruído)
#define PSG_SAMPLE_RATE 44100 // Taxa de amostragem para a saída (Hz)
#define PSG_BUFFER_SIZE 2048  // Tamanho do buffer de saída

// Tamanho da tabela de volume
#define PSG_VOLUME_LEVELS 16

// Tipos de registradores PSG
#define PSG_REG_TONE0_FREQ_L 0x00
#define PSG_REG_TONE0_FREQ_H 0x01
#define PSG_REG_TONE1_FREQ_L 0x02
#define PSG_REG_TONE1_FREQ_H 0x03
#define PSG_REG_TONE2_FREQ_L 0x04
#define PSG_REG_TONE2_FREQ_H 0x05
#define PSG_REG_NOISE_CTRL 0x06
#define PSG_REG_MIXER 0x07
#define PSG_REG_VOL0 0x08
#define PSG_REG_VOL1 0x09
#define PSG_REG_VOL2 0x0A
#define PSG_REG_VOL3 0x0B
#define PSG_REG_ENV_FREQ_L 0x0C
#define PSG_REG_ENV_FREQ_H 0x0D
#define PSG_REG_ENV_SHAPE 0x0E

/**
 * @brief Estrutura interna do PSG do Master System
 */
struct sms_psg_t
{
    // Registradores
    uint16_t tone_periods[3]; // Períodos de tom para canais 0-2
    uint8_t noise_period;     // Período de ruído
    uint8_t mixer;            // Registrador de mixer (controla quais canais estão ativos)
    uint8_t volumes[4];       // Volumes para canais 0-3
    uint16_t envelope_period; // Período do envelope
    uint8_t envelope_shape;   // Forma do envelope

    // Estado interno
    uint16_t tone_counters[3]; // Contadores de tom para canais 0-2
    uint8_t tone_states[3];    // Estados de tom para canais 0-2 (0 ou 1)
    uint8_t noise_counter;     // Contador de ruído
    uint16_t noise_shift;      // Registrador de deslocamento para geração de ruído
    uint8_t noise_output;      // Saída atual do gerador de ruído
    uint16_t envelope_counter; // Contador do envelope
    uint8_t envelope_step;     // Passo atual do envelope

    // Buffer de saída
    int16_t *output_buffer; // Buffer para amostras de saída
    uint32_t buffer_size;   // Tamanho do buffer
    uint32_t buffer_pos;    // Posição atual no buffer

    // Tabela de volume (atenuação logarítmica)
    int16_t volume_table[PSG_VOLUME_LEVELS];

    // Ciclos e temporização
    uint32_t cycles_per_sample; // Ciclos de CPU por amostra
    uint32_t cycle_counter;     // Contador de ciclos atual

    // Valor do registrador de latch para comandos
    uint8_t latch_register; // Registrador a ser atualizado
    bool latch_valid;       // Flag para latch válido
};

// Forward declarations
static void sms_psg_generate_sample(sms_psg_t *psg);
static void sms_psg_init_volume_table(sms_psg_t *psg);
static void sms_psg_update_tone(sms_psg_t *psg, uint8_t channel);
static void sms_psg_update_noise(sms_psg_t *psg);
static void sms_psg_update_envelope(sms_psg_t *psg);

/**
 * @brief Cria uma nova instância do PSG
 */
sms_psg_t *sms_psg_create(void)
{
    sms_psg_t *psg = (sms_psg_t *)malloc(sizeof(sms_psg_t));
    if (!psg)
    {
        SMS_PSG_LOG_ERROR("Falha ao alocar memória para o PSG");
        return NULL;
    }

    // Inicializa a estrutura
    memset(psg, 0, sizeof(sms_psg_t));

    // Aloca buffer de saída
    psg->output_buffer = (int16_t *)malloc(PSG_BUFFER_SIZE * sizeof(int16_t));
    if (!psg->output_buffer)
    {
        SMS_PSG_LOG_ERROR("Falha ao alocar buffer de saída do PSG");
        free(psg);
        return NULL;
    }

    psg->buffer_size = PSG_BUFFER_SIZE;

    // Inicializa a tabela de volume
    sms_psg_init_volume_table(psg);

    // Inicializa o gerador de ruído
    psg->noise_shift = 0x8000; // Valor inicial do registrador de deslocamento

    // Calcula ciclos por amostra
    psg->cycles_per_sample = PSG_CLOCK / PSG_SAMPLE_RATE;

    SMS_PSG_LOG_INFO("PSG do Master System criado com sucesso");
    return psg;
}

/**
 * @brief Destrói uma instância do PSG e libera recursos
 */
void sms_psg_destroy(sms_psg_t *psg)
{
    if (!psg)
    {
        return;
    }

    // Libera recursos
    if (psg->output_buffer)
    {
        free(psg->output_buffer);
    }

    free(psg);
    SMS_PSG_LOG_INFO("PSG do Master System destruído");
}

/**
 * @brief Reseta o PSG para o estado inicial
 */
void sms_psg_reset(sms_psg_t *psg)
{
    if (!psg)
    {
        return;
    }

    // Reseta registradores
    for (int i = 0; i < 3; i++)
    {
        psg->tone_periods[i] = 0;
        psg->tone_counters[i] = 0;
        psg->tone_states[i] = 0;
        psg->volumes[i] = 0;
    }

    psg->noise_period = 0;
    psg->noise_counter = 0;
    psg->noise_shift = 0x8000;
    psg->noise_output = 0;
    psg->mixer = 0xFF; // Todos os canais desativados
    psg->volumes[3] = 0;

    psg->envelope_period = 0;
    psg->envelope_counter = 0;
    psg->envelope_shape = 0;
    psg->envelope_step = 0;

    psg->latch_register = 0;
    psg->latch_valid = false;

    // Reseta o buffer de saída
    memset(psg->output_buffer, 0, psg->buffer_size * sizeof(int16_t));
    psg->buffer_pos = 0;
    psg->cycle_counter = 0;

    SMS_PSG_LOG_INFO("PSG do Master System resetado");
}

/**
 * @brief Inicializa a tabela de volumes (atenuação logarítmica)
 */
static void sms_psg_init_volume_table(sms_psg_t *psg)
{
    // O PSG usa atenuação logarítmica de 2dB por nível
    // Volume máximo é 0 (sem atenuação), mínimo é 15 (silêncio)

    for (int i = 0; i < PSG_VOLUME_LEVELS; i++)
    {
        // Fórmula: volume = 32767 * 10^(-(i * 2)/20)
        if (i == 15)
        {
            // Volume 15 é silêncio
            psg->volume_table[i] = 0;
        }
        else
        {
            double db_attenuation = -(i * 2.0);
            double amplitude = pow(10.0, db_attenuation / 20.0);
            psg->volume_table[i] = (int16_t)(32767.0 * amplitude);
        }
    }
}

/**
 * @brief Atualiza o estado do PSG com base nos ciclos executados
 */
void sms_psg_update(sms_psg_t *psg, uint8_t cycles)
{
    if (!psg)
    {
        return;
    }

    // Acumula ciclos
    psg->cycle_counter += cycles;

    // Gera amostras conforme necessário
    while (psg->cycle_counter >= psg->cycles_per_sample)
    {
        psg->cycle_counter -= psg->cycles_per_sample;

        // Gera uma amostra
        sms_psg_generate_sample(psg);
    }
}

/**
 * @brief Gera uma amostra para o buffer de saída
 */
static void sms_psg_generate_sample(sms_psg_t *psg)
{
    if (!psg || !psg->output_buffer)
    {
        return;
    }

    // Atualiza os geradores de tom
    for (int i = 0; i < 3; i++)
    {
        sms_psg_update_tone(psg, i);
    }

    // Atualiza o gerador de ruído
    sms_psg_update_noise(psg);

    // Atualiza o envelope
    sms_psg_update_envelope(psg);

    // Calcula a saída combinada
    int16_t sample = 0;

    // Para cada canal
    for (int i = 0; i < 3; i++)
    {
        // Verifica se o canal de tom está ativo
        if (!(psg->mixer & (1 << i)))
        {
            // Volume baseado no estado do tom
            if (psg->tone_states[i])
            {
                sample += psg->volume_table[psg->volumes[i]];
            }
        }
    }

    // Canal de ruído
    if (!(psg->mixer & 0x08) && psg->noise_output)
    {
        sample += psg->volume_table[psg->volumes[3]];
    }

    // Escala para evitar clipping
    sample = (int16_t)(sample / 4);

    // Adiciona ao buffer
    if (psg->buffer_pos < psg->buffer_size)
    {
        psg->output_buffer[psg->buffer_pos++] = sample;
    }
}

/**
 * @brief Atualiza o gerador de tom para um canal
 */
static void sms_psg_update_tone(sms_psg_t *psg, uint8_t channel)
{
    if (!psg || channel >= 3)
    {
        return;
    }

    // Período 0 é tratado como 1
    uint16_t period = psg->tone_periods[channel];
    if (period == 0)
    {
        period = 1;
    }

    // Incrementa o contador
    psg->tone_counters[channel]++;

    // Verifica se o contador atingiu o período
    if (psg->tone_counters[channel] >= period)
    {
        // Reinicia o contador
        psg->tone_counters[channel] = 0;

        // Inverte o estado do tom
        psg->tone_states[channel] = !psg->tone_states[channel];
    }
}

/**
 * @brief Atualiza o gerador de ruído
 */
static void sms_psg_update_noise(sms_psg_t *psg)
{
    if (!psg)
    {
        return;
    }

    // Determina o período do ruído
    uint8_t period;

    switch (psg->noise_period & 0x03)
    {
    case 0:
        period = 0x10;
        break;
    case 1:
        period = 0x20;
        break;
    case 2:
        period = 0x40;
        break;
    case 3:
        period = psg->tone_periods[2] & 0xFF;
        break;
    default:
        period = 0x10;
        break;
    }

    // Período 0 é tratado como 1
    if (period == 0)
    {
        period = 1;
    }

    // Incrementa o contador
    psg->noise_counter++;

    // Verifica se o contador atingiu o período
    if (psg->noise_counter >= period)
    {
        // Reinicia o contador
        psg->noise_counter = 0;

        // Executa o LFSR (Linear Feedback Shift Register)
        // Bit de feedback depende do modo de ruído
        bool feedback;

        if (psg->noise_period & 0x04)
        {
            // Modo periódico (white noise)
            feedback = ((psg->noise_shift & 0x0001) != 0) ^ ((psg->noise_shift & 0x0008) != 0);
        }
        else
        {
            // Modo periódico (periodic noise)
            feedback = (psg->noise_shift & 0x0001) != 0;
        }

        // Desloca e insere o bit de feedback
        psg->noise_shift = (psg->noise_shift >> 1) | (feedback ? 0x8000 : 0);

        // Atualiza a saída do ruído
        psg->noise_output = psg->noise_shift & 0x0001;
    }
}

/**
 * @brief Atualiza o envelope
 */
static void sms_psg_update_envelope(sms_psg_t *psg)
{
    // Implementação simplificada - em uma versão completa,
    // isso gerenciaria o envelope de acordo com a forma especificada
}

/**
 * @brief Finaliza o frame atual e gera amostras de áudio
 */
void sms_psg_end_frame(sms_psg_t *psg)
{
    if (!psg)
    {
        return;
    }

    // Reseta o contador de ciclos para o próximo frame
    psg->cycle_counter = 0;

    SMS_PSG_LOG_TRACE("Frame de áudio finalizado, amostras geradas: %d", psg->buffer_pos);
}

/**
 * @brief Obtém amostras de áudio geradas pelo PSG
 */
uint32_t sms_psg_get_samples(sms_psg_t *psg, int16_t *buffer, uint32_t num_samples)
{
    if (!psg || !buffer || !psg->output_buffer)
    {
        return 0;
    }

    // Número real de amostras a serem copiadas
    uint32_t samples_to_copy = (psg->buffer_pos < num_samples) ? psg->buffer_pos : num_samples;

    // Copia as amostras para o buffer de destino
    memcpy(buffer, psg->output_buffer, samples_to_copy * sizeof(int16_t));

    // Reseta o buffer de saída
    psg->buffer_pos = 0;

    return samples_to_copy;
}

/**
 * @brief Escreve no registrador do PSG
 */
void sms_psg_write_port(sms_psg_t *psg, uint8_t value)
{
    if (!psg)
    {
        return;
    }

    // Verifica se é um comando de latch ou dados
    if (value & 0x80)
    {
        // Comando de latch
        psg->latch_register = (value >> 4) & 0x07;
        psg->latch_valid = true;

        // Se o bit 4 estiver definido, também contém dados
        if (value & 0x10)
        {
            // Atualiza os 4 bits inferiores do registrador
            uint8_t data = value & 0x0F;

            switch (psg->latch_register)
            {
            case 0: // Tone 0 frequency (low)
                psg->tone_periods[0] = (psg->tone_periods[0] & 0x3F0) | data;
                break;

            case 1: // Tone 0 frequency (high)
                psg->tone_periods[0] = (psg->tone_periods[0] & 0x00F) | ((data & 0x3F) << 4);
                break;

            case 2: // Tone 1 frequency (low)
                psg->tone_periods[1] = (psg->tone_periods[1] & 0x3F0) | data;
                break;

            case 3: // Tone 1 frequency (high)
                psg->tone_periods[1] = (psg->tone_periods[1] & 0x00F) | ((data & 0x3F) << 4);
                break;

            case 4: // Tone 2 frequency (low)
                psg->tone_periods[2] = (psg->tone_periods[2] & 0x3F0) | data;
                break;

            case 5: // Tone 2 frequency (high)
                psg->tone_periods[2] = (psg->tone_periods[2] & 0x00F) | ((data & 0x3F) << 4);
                break;

            case 6: // Noise control
                psg->noise_period = data & 0x07;
                break;

            case 7: // Mixer control/IO enable
                psg->mixer = data & 0x3F;
                break;

            default:
                break;
            }
        }
    }
    else if (psg->latch_valid)
    {
        // Dados para o registrador previamente selecionado
        uint8_t data = value & 0x0F;

        switch (psg->latch_register)
        {
        case 0: // Tone 0 frequency (low)
            psg->tone_periods[0] = (psg->tone_periods[0] & 0x3F0) | data;
            break;

        case 1: // Tone 0 frequency (high)
            psg->tone_periods[0] = (psg->tone_periods[0] & 0x00F) | ((data & 0x3F) << 4);
            break;

        case 2: // Tone 1 frequency (low)
            psg->tone_periods[1] = (psg->tone_periods[1] & 0x3F0) | data;
            break;

        case 3: // Tone 1 frequency (high)
            psg->tone_periods[1] = (psg->tone_periods[1] & 0x00F) | ((data & 0x3F) << 4);
            break;

        case 4: // Tone 2 frequency (low)
            psg->tone_periods[2] = (psg->tone_periods[2] & 0x3F0) | data;
            break;

        case 5: // Tone 2 frequency (high)
            psg->tone_periods[2] = (psg->tone_periods[2] & 0x00F) | ((data & 0x3F) << 4);
            break;

        case 6: // Noise control
            psg->noise_period = data & 0x07;
            break;

        case 7: // Mixer control/IO enable
            psg->mixer = data & 0x3F;
            break;

        default:
            break;
        }
    }

    SMS_PSG_LOG_TRACE("Escrita no PSG: 0x%02X", value);
}

/**
 * @brief Registra o estado do PSG no sistema de save state
 */
int sms_psg_register_save_state(sms_psg_t *psg, save_state_t *state)
{
    if (!psg || !state)
    {
        return -1;
    }

    // Implementação simplificada - em uma implementação real, registraria todos os campos necessários
    save_state_register_section(state, "SMS_PSG");

    save_state_register_field(state, psg->tone_periods, sizeof(psg->tone_periods));
    save_state_register_field(state, &psg->noise_period, sizeof(psg->noise_period));
    save_state_register_field(state, &psg->mixer, sizeof(psg->mixer));
    save_state_register_field(state, psg->volumes, sizeof(psg->volumes));
    save_state_register_field(state, &psg->envelope_period, sizeof(psg->envelope_period));
    save_state_register_field(state, &psg->envelope_shape, sizeof(psg->envelope_shape));
    save_state_register_field(state, psg->tone_counters, sizeof(psg->tone_counters));
    save_state_register_field(state, psg->tone_states, sizeof(psg->tone_states));
    save_state_register_field(state, &psg->noise_counter, sizeof(psg->noise_counter));
    save_state_register_field(state, &psg->noise_shift, sizeof(psg->noise_shift));
    save_state_register_field(state, &psg->noise_output, sizeof(psg->noise_output));
    save_state_register_field(state, &psg->envelope_counter, sizeof(psg->envelope_counter));
    save_state_register_field(state, &psg->envelope_step, sizeof(psg->envelope_step));
    save_state_register_field(state, &psg->latch_register, sizeof(psg->latch_register));
    save_state_register_field(state, &psg->latch_valid, sizeof(psg->latch_valid));

    save_state_end_section(state);

    SMS_PSG_LOG_INFO("Estado do PSG registrado no sistema de save state");
    return 0;
}
