/**
 * @file gg_psg.c
 * @brief Implementação do sistema de áudio do Game Gear (PSG)
 */

#include "gg_psg.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG_PSG EMU_LOG_CAT_AUDIO

// Macros de log
#define GG_PSG_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG_PSG, __VA_ARGS__)
#define GG_PSG_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG_PSG, __VA_ARGS__)
#define GG_PSG_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG_PSG, __VA_ARGS__)
#define GG_PSG_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG_PSG, __VA_ARGS__)
#define GG_PSG_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG_PSG, __VA_ARGS__)

// Constantes do PSG
#define PSG_CLOCK_DIVIDER 16
#define PSG_VOLUME_LEVELS 16
#define PSG_MAX_AMPLITUDE 0x7FFF

// Estrutura para canal de tom
typedef struct {
    uint16_t freq;         // Frequência do canal
    uint8_t volume;        // Volume do canal (0-15)
    uint32_t counter;      // Contador de ciclos
    bool output;           // Estado atual da onda quadrada
    bool enabled;          // Canal habilitado
} psg_tone_channel_t;

// Estrutura para canal de ruído
typedef struct {
    uint8_t shift_rate;    // Taxa de deslocamento
    uint8_t volume;        // Volume do canal (0-15)
    uint32_t counter;      // Contador de ciclos
    uint16_t lfsr;         // Registrador de deslocamento
    bool output;           // Estado atual do ruído
    bool enabled;          // Canal habilitado
} psg_noise_channel_t;

// Estrutura para envelope
typedef struct {
    uint16_t freq;         // Frequência do envelope
    uint8_t shape;         // Forma do envelope
    uint32_t counter;      // Contador de ciclos
    uint8_t volume;        // Volume atual do envelope
    bool enabled;          // Envelope habilitado
} psg_envelope_t;

/**
 * @brief Estrutura do PSG do Game Gear
 */
struct gg_psg_t {
    psg_tone_channel_t tone[3];      // Canais de tom
    psg_noise_channel_t noise;       // Canal de ruído
    psg_envelope_t envelope;         // Gerador de envelope
    uint8_t latch;                   // Registrador de latch
    bool is_reg_select;              // Flag de seleção de registrador
    int16_t buffer[GG_PSG_BUFFER_SIZE]; // Buffer de áudio
    size_t buffer_pos;               // Posição atual no buffer
    uint32_t sample_counter;         // Contador de amostras
    uint32_t cycles_per_sample;      // Ciclos por amostra
};

// Tabela de volumes (logarítmica)
static const int16_t volume_table[PSG_VOLUME_LEVELS] = {
    0x0000, 0x0055, 0x0079, 0x00AB, 0x00F1, 0x0155, 0x01E3, 0x02AA,
    0x03C5, 0x0555, 0x079A, 0x0AAB, 0x0F16, 0x1555, 0x1E2B, 0x2AAA
};

/**
 * @brief Cria uma nova instância do PSG
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_psg_t *gg_psg_create(void) {
    gg_psg_t *psg = (gg_psg_t *)malloc(sizeof(gg_psg_t));
    if (!psg) {
        GG_PSG_LOG_ERROR("Falha ao alocar memória para PSG");
        return NULL;
    }

    // Inicializa estrutura
    memset(psg, 0, sizeof(gg_psg_t));

    // Configura ciclos por amostra
    psg->cycles_per_sample = GG_PSG_CLOCK / GG_PSG_SAMPLE_RATE;

    // Inicializa LFSR do ruído
    psg->noise.lfsr = 0x8000;

    GG_PSG_LOG_INFO("PSG do Game Gear criado");
    return psg;
}

/**
 * @brief Destrói uma instância do PSG
 * @param psg Ponteiro para a instância
 */
void gg_psg_destroy(gg_psg_t *psg) {
    if (!psg) return;
    free(psg);
    GG_PSG_LOG_INFO("PSG do Game Gear destruído");
}

/**
 * @brief Reseta o PSG
 * @param psg Ponteiro para a instância
 */
void gg_psg_reset(gg_psg_t *psg) {
    if (!psg) return;

    // Reseta canais de tom
    for (int i = 0; i < 3; i++) {
        psg->tone[i].freq = 0;
        psg->tone[i].volume = 0;
        psg->tone[i].counter = 0;
        psg->tone[i].output = false;
        psg->tone[i].enabled = true;
    }

    // Reseta canal de ruído
    psg->noise.shift_rate = 0;
    psg->noise.volume = 0;
    psg->noise.counter = 0;
    psg->noise.lfsr = 0x8000;
    psg->noise.output = false;
    psg->noise.enabled = true;

    // Reseta envelope
    psg->envelope.freq = 0;
    psg->envelope.shape = 0;
    psg->envelope.counter = 0;
    psg->envelope.volume = 0;
    psg->envelope.enabled = false;

    // Reseta estado
    psg->latch = 0;
    psg->is_reg_select = true;
    psg->buffer_pos = 0;
    psg->sample_counter = 0;

    GG_PSG_LOG_INFO("PSG do Game Gear resetado");
}

/**
 * @brief Atualiza um canal de tom
 * @param channel Ponteiro para o canal
 * @param cycles Número de ciclos
 * @return Amplitude atual do canal
 */
static int16_t update_tone_channel(psg_tone_channel_t *channel, uint32_t cycles) {
    if (!channel->enabled || channel->freq == 0) {
        return 0;
    }

    channel->counter += cycles;
    uint32_t period = (uint32_t)channel->freq * PSG_CLOCK_DIVIDER;

    while (channel->counter >= period) {
        channel->counter -= period;
        channel->output = !channel->output;
    }

    return channel->output ? volume_table[channel->volume] : -volume_table[channel->volume];
}

/**
 * @brief Atualiza o canal de ruído
 * @param noise Ponteiro para o canal de ruído
 * @param cycles Número de ciclos
 * @return Amplitude atual do ruído
 */
static int16_t update_noise_channel(psg_noise_channel_t *noise, uint32_t cycles) {
    if (!noise->enabled) {
        return 0;
    }

    noise->counter += cycles;
    uint32_t period = (uint32_t)(16 << noise->shift_rate) * PSG_CLOCK_DIVIDER;

    while (noise->counter >= period) {
        noise->counter -= period;

        // Atualiza LFSR
        bool bit = ((noise->lfsr & 0x0001) ^ ((noise->lfsr & 0x0002) >> 1)) != 0;
        noise->lfsr = (noise->lfsr >> 1) | (bit << 15);
        noise->output = (noise->lfsr & 0x0001) != 0;
    }

    return noise->output ? volume_table[noise->volume] : -volume_table[noise->volume];
}

/**
 * @brief Atualiza o envelope
 * @param envelope Ponteiro para o envelope
 * @param cycles Número de ciclos
 */
static void update_envelope(psg_envelope_t *envelope, uint32_t cycles) {
    if (!envelope->enabled || envelope->freq == 0) {
        return;
    }

    envelope->counter += cycles;
    uint32_t period = (uint32_t)envelope->freq * PSG_CLOCK_DIVIDER;

    while (envelope->counter >= period) {
        envelope->counter -= period;

        // Atualiza volume baseado na forma do envelope
        switch (envelope->shape) {
            case 0x00: // \___
            case 0x04:
            case 0x08:
            case 0x0C:
                if (envelope->volume > 0) envelope->volume--;
                break;

            case 0x0B: // /|
            case 0x0D:
            case 0x0F:
                if (envelope->volume < 15) envelope->volume++;
                else envelope->enabled = false;
                break;

            case 0x09: // \\|
            case 0x0E:
                if (envelope->volume > 0) envelope->volume--;
                else envelope->enabled = false;
                break;

            case 0x0A: // \/\/
                envelope->volume = (envelope->volume - 1) & 0x0F;
                break;

            case 0x05: // /\/\
                envelope->volume = (envelope->volume + 1) & 0x0F;
                break;
        }
    }
}

/**
 * @brief Escreve um valor em um registrador do PSG
 * @param psg Ponteiro para a instância
 * @param value Valor a ser escrito
 */
void gg_psg_write(gg_psg_t *psg, uint8_t value) {
    if (!psg) return;

    if (value & 0x80) {
        // Seleção de registrador
        psg->latch = value;
        psg->is_reg_select = false;
        uint8_t reg = (value >> 4) & 0x07;
        uint8_t data = value & 0x0F;

        switch (reg) {
            case 0: // Frequência canal 0 (bits baixos)
            case 2: // Frequência canal 1 (bits baixos)
            case 4: // Frequência canal 2 (bits baixos)
                psg->tone[reg >> 1].freq = (psg->tone[reg >> 1].freq & 0x0F00) | data;
                break;

            case 1: // Frequência canal 0 (bits altos)
            case 3: // Frequência canal 1 (bits altos)
            case 5: // Frequência canal 2 (bits altos)
                psg->tone[(reg - 1) >> 1].freq = (psg->tone[(reg - 1) >> 1].freq & 0x000F) | (data << 8);
                break;

            case 6: // Controle de ruído
                psg->noise.shift_rate = data & 0x03;
                break;

            case 7: // Controle do mixer
                for (int i = 0; i < 3; i++) {
                    psg->tone[i].enabled = !(data & (1 << i));
                }
                psg->noise.enabled = !(data & (1 << 3));
                break;
        }
    } else if (!psg->is_reg_select) {
        // Dados para registrador selecionado
        uint8_t reg = (psg->latch >> 4) & 0x07;
        uint8_t data = value & 0x0F;

        switch (reg) {
            case 0: // Volume canal 0
            case 1: // Volume canal 1
            case 2: // Volume canal 2
                if (data & 0x10) {
                    psg->tone[reg].volume = psg->envelope.volume;
                } else {
                    psg->tone[reg].volume = data & 0x0F;
                }
                break;

            case 3: // Volume do ruído
                if (data & 0x10) {
                    psg->noise.volume = psg->envelope.volume;
                } else {
                    psg->noise.volume = data & 0x0F;
                }
                break;

            case 4: // Frequência do envelope (bits baixos)
                psg->envelope.freq = (psg->envelope.freq & 0xFF00) | data;
                break;

            case 5: // Frequência do envelope (bits altos)
                psg->envelope.freq = (psg->envelope.freq & 0x00FF) | (data << 8);
                break;

            case 6: // Forma do envelope
                psg->envelope.shape = data & 0x0F;
                psg->envelope.volume = 0x0F;
                psg->envelope.counter = 0;
                psg->envelope.enabled = true;
                break;
        }
    }
}

/**
 * @brief Atualiza o estado do PSG
 * @param psg Ponteiro para a instância
 * @param cycles Número de ciclos a processar
 */
void gg_psg_update(gg_psg_t *psg, uint32_t cycles) {
    if (!psg) return;

    psg->sample_counter += cycles;

    // Gera amostras quando necessário
    while (psg->sample_counter >= psg->cycles_per_sample) {
        psg->sample_counter -= psg->cycles_per_sample;

        // Atualiza envelope
        update_envelope(&psg->envelope, psg->cycles_per_sample);

        // Mixa saída dos canais
        int32_t output = 0;

        // Processa canais de tom
        for (int i = 0; i < 3; i++) {
            output += update_tone_channel(&psg->tone[i], psg->cycles_per_sample);
        }

        // Processa canal de ruído
        output += update_noise_channel(&psg->noise, psg->cycles_per_sample);

        // Limita amplitude e armazena no buffer
        output = (output * 3) >> 2; // Reduz volume para evitar clipping
        if (output > PSG_MAX_AMPLITUDE) output = PSG_MAX_AMPLITUDE;
        if (output < -PSG_MAX_AMPLITUDE) output = -PSG_MAX_AMPLITUDE;

        psg->buffer[psg->buffer_pos++] = (int16_t)output;

        // Reinicia buffer se cheio
        if (psg->buffer_pos >= GG_PSG_BUFFER_SIZE) {
            psg->buffer_pos = 0;
        }
    }
}

/**
 * @brief Obtém o buffer de áudio do PSG
 * @param psg Ponteiro para a instância
 * @param size Ponteiro para armazenar o tamanho do buffer
 * @return Ponteiro para o buffer de áudio
 */
const int16_t *gg_psg_get_buffer(gg_psg_t *psg, size_t *size) {
    if (!psg || !size) return NULL;
    *size = psg->buffer_pos;
    psg->buffer_pos = 0;
    return psg->buffer;
}

/**
 * @brief Registra campos do PSG no sistema de save state
 * @param psg Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_psg_register_save_state(gg_psg_t *psg, save_state_t *state) {
    if (!psg || !state) return -1;

    // Registra canais de tom
    for (int i = 0; i < 3; i++) {
        char name[32];
        snprintf(name, sizeof(name), "gg_psg_tone%d_freq", i);
        save_state_register_field(state, name, &psg->tone[i].freq,
                                sizeof(psg->tone[i].freq));

        snprintf(name, sizeof(name), "gg_psg_tone%d_volume", i);
        save_state_register_field(state, name, &psg->tone[i].volume,
                                sizeof(psg->tone[i].volume));

        snprintf(name, sizeof(name), "gg_psg_tone%d_enabled", i);
        save_state_register_field(state, name, &psg->tone[i].enabled,
                                sizeof(psg->tone[i].enabled));
    }

    // Registra canal de ruído
    save_state_register_field(state, "gg_psg_noise_shift_rate",
                            &psg->noise.shift_rate,
                            sizeof(psg->noise.shift_rate));
    save_state_register_field(state, "gg_psg_noise_volume",
                            &psg->noise.volume,
                            sizeof(psg->noise.volume));
    save_state_register_field(state, "gg_psg_noise_lfsr",
                            &psg->noise.lfsr,
                            sizeof(psg->noise.lfsr));
    save_state_register_field(state, "gg_psg_noise_enabled",
                            &psg->noise.enabled,
                            sizeof(psg->noise.enabled));

    // Registra envelope
    save_state_register_field(state, "gg_psg_env_freq",
                            &psg->envelope.freq,
                            sizeof(psg->envelope.freq));
    save_state_register_field(state, "gg_psg_env_shape",
                            &psg->envelope.shape,
                            sizeof(psg->envelope.shape));
    save_state_register_field(state, "gg_psg_env_volume",
                            &psg->envelope.volume,
                            sizeof(psg->envelope.volume));
    save_state_register_field(state, "gg_psg_env_enabled",
                            &psg->envelope.enabled,
                            sizeof(psg->envelope.enabled));

    // Registra estado geral
    save_state_register_field(state, "gg_psg_latch",
                            &psg->latch,
                            sizeof(psg->latch));
    save_state_register_field(state, "gg_psg_is_reg_select",
                            &psg->is_reg_select,
                            sizeof(psg->is_reg_select));

    return 0;
}
