/**
 * @file sn76489.h
 * @brief Emulação do chip de som SN76489 (PSG) do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 * 
 * O SN76489 é um chip de som PSG (Programmable Sound Generator) de 4 canais
 * usado no Mega Drive/Genesis para gerar sons simples. Este arquivo define
 * a interface para a emulação deste chip no Mega_Emu.
 */

#ifndef EMU_SN76489_H
#define EMU_SN76489_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../utils/common_types.h"

// Definição de constantes
#define SN76489_CLOCK_FREQ 3579545  // Frequência do clock em Hz
#define SN76489_NUM_CHANNELS 4      // Número de canais (3 tons + 1 ruído)
#define SN76489_SAMPLE_RATE 44100   // Taxa de amostragem padrão
#define SN76489_OUTPUTS 2           // Saídas estéreo (L/R)

// Estrutura para cada canal de tom
typedef struct {
    uint16_t tone_reg;      // Registrador de tom (10 bits)
    uint8_t attenuation;    // Atenuação (4 bits, 0=máximo volume, 15=silêncio)
    uint32_t counter;       // Contador para geração de onda
    uint8_t output;         // Estado atual da saída (0 ou 1)
    int16_t out_value;      // Valor de saída atual
} sn76489_tone_channel_t;

// Estrutura para o canal de ruído
typedef struct {
    uint8_t shift_rate;     // Taxa de deslocamento (2 bits)
    bool fb_type;           // Tipo de feedback (0=periódico, 1=ruído branco)
    uint8_t attenuation;    // Atenuação (4 bits, 0=máximo volume, 15=silêncio)
    uint32_t counter;       // Contador para geração de ruído
    uint16_t shift_reg;     // Registrador de deslocamento
    int16_t out_value;      // Valor de saída atual
} sn76489_noise_channel_t;

// Estrutura geral para o chip SN76489
typedef struct {
    sn76489_tone_channel_t tone_channels[3];  // 3 canais de tom
    sn76489_noise_channel_t noise_channel;    // 1 canal de ruído
    
    // Estado geral
    uint8_t latch;          // Registrador de latch atual
    uint8_t stereo;         // Configuração estéreo (para versão estéreo do chip)
    
    // Estado interno
    uint32_t rate;          // Taxa de amostragem
    uint32_t clock;         // Frequência do clock
    float clock_ratio;      // Razão clock/sample rate
    
    // Contadores
    uint32_t cycles;        // Ciclos acumulados
    uint32_t samples_generated; // Amostras geradas
    
    // Tabela de volume
    int16_t volume_table[16]; // Tabela de conversão de atenuação para amplitude
} sn76489_t;

/**
 * @brief Inicializa o chip SN76489
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @param clock Frequência do clock em Hz
 * @param rate Taxa de amostragem em Hz
 * @return emu_error_t EMU_ERROR_NONE se bem-sucedido
 */
emu_error_t sn76489_init(sn76489_t *chip, uint32_t clock, uint32_t rate);

/**
 * @brief Reseta o estado do chip SN76489
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @return emu_error_t EMU_ERROR_NONE se bem-sucedido
 */
emu_error_t sn76489_reset(sn76489_t *chip);

/**
 * @brief Libera recursos utilizados pelo chip SN76489
 * 
 * @param chip Ponteiro para a estrutura do chip
 */
void sn76489_shutdown(sn76489_t *chip);

/**
 * @brief Escreve um valor no chip
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @param data Byte de dados a ser escrito
 */
void sn76489_write(sn76489_t *chip, uint8_t data);

/**
 * @brief Atualiza o estado do chip e gera amostras
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @param buffer_left Buffer para o canal esquerdo
 * @param buffer_right Buffer para o canal direito
 * @param num_samples Número de amostras a gerar
 * @return int32_t Número de amostras geradas
 */
int32_t sn76489_update(sn76489_t *chip, int16_t *buffer_left, int16_t *buffer_right, int32_t num_samples);

/**
 * @brief Define a taxa de amostragem
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @param rate Nova taxa de amostragem em Hz
 */
void sn76489_set_sample_rate(sn76489_t *chip, uint32_t rate);

/**
 * @brief Define a frequência do clock
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @param clock Nova frequência do clock em Hz
 */
void sn76489_set_clock(sn76489_t *chip, uint32_t clock);

/**
 * @brief Avança o SN76489 pelo número especificado de ciclos
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @param cycles Número de ciclos para avançar
 */
void sn76489_advance(sn76489_t *chip, uint32_t cycles);

/**
 * @brief Define a configuração estéreo (para versão estéreo do chip)
 * 
 * @param chip Ponteiro para a estrutura do chip
 * @param stereo_byte Byte de configuração estéreo
 */
void sn76489_set_stereo(sn76489_t *chip, uint8_t stereo_byte);

#endif // EMU_SN76489_H
