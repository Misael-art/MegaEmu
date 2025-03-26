/**
 * @file gg_psg.h
 * @brief Sistema de áudio do Game Gear (PSG - Programmable Sound Generator)
 */

#ifndef GG_PSG_H
#define GG_PSG_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/save_state.h"

// Configurações do PSG
#define GG_PSG_CLOCK 3579545        // Clock do PSG em Hz
#define GG_PSG_CHANNELS 4           // Número de canais (3 tons + 1 ruído)
#define GG_PSG_BUFFER_SIZE 2048     // Tamanho do buffer de áudio
#define GG_PSG_SAMPLE_RATE 44100    // Taxa de amostragem em Hz

// Registradores do PSG
#define GG_PSG_TONE0_FREQ 0x00      // Frequência do canal 0
#define GG_PSG_TONE1_FREQ 0x02      // Frequência do canal 1
#define GG_PSG_TONE2_FREQ 0x04      // Frequência do canal 2
#define GG_PSG_NOISE_CTRL 0x06      // Controle de ruído
#define GG_PSG_MIXER_CTRL 0x07      // Controle do mixer
#define GG_PSG_TONE0_VOL 0x08       // Volume do canal 0
#define GG_PSG_TONE1_VOL 0x09       // Volume do canal 1
#define GG_PSG_TONE2_VOL 0x0A       // Volume do canal 2
#define GG_PSG_NOISE_VOL 0x0B       // Volume do ruído
#define GG_PSG_ENV_FREQ 0x0C        // Frequência do envelope
#define GG_PSG_ENV_SHAPE 0x0D       // Forma do envelope

// Forward declaration
typedef struct gg_psg_t gg_psg_t;

/**
 * @brief Cria uma nova instância do PSG
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_psg_t *gg_psg_create(void);

/**
 * @brief Destrói uma instância do PSG
 * @param psg Ponteiro para a instância
 */
void gg_psg_destroy(gg_psg_t *psg);

/**
 * @brief Reseta o PSG
 * @param psg Ponteiro para a instância
 */
void gg_psg_reset(gg_psg_t *psg);

/**
 * @brief Escreve um valor em um registrador do PSG
 * @param psg Ponteiro para a instância
 * @param value Valor a ser escrito
 */
void gg_psg_write(gg_psg_t *psg, uint8_t value);

/**
 * @brief Atualiza o estado do PSG
 * @param psg Ponteiro para a instância
 * @param cycles Número de ciclos a processar
 */
void gg_psg_update(gg_psg_t *psg, uint32_t cycles);

/**
 * @brief Obtém o buffer de áudio do PSG
 * @param psg Ponteiro para a instância
 * @param size Ponteiro para armazenar o tamanho do buffer
 * @return Ponteiro para o buffer de áudio
 */
const int16_t *gg_psg_get_buffer(gg_psg_t *psg, size_t *size);

/**
 * @brief Registra campos do PSG no sistema de save state
 * @param psg Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_psg_register_save_state(gg_psg_t *psg, save_state_t *state);

#endif // GG_PSG_H
