/**
 * @file audio.h
 * @brief Interface para o sistema de áudio do Mega Drive
 */

#ifndef MD_AUDIO_H
#define MD_AUDIO_H

#include "../../../utils/common_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Estrutura que representa o estado do sistema de áudio do Mega Drive
 */
typedef struct {
  uint8_t psg_registers[8];    /**< Registradores do PSG */
  uint8_t fm_registers[0x200]; /**< Registradores do FM */
  bool psg_enabled;            /**< Estado de ativação do PSG */
  bool fm_enabled;             /**< Estado de ativação do FM */
} md_audio_state_t;

/**
 * @brief Inicializa o sistema de áudio
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_audio_init(void);

/**
 * @brief Finaliza o sistema de áudio
 */
void md_audio_shutdown(void);

/**
 * @brief Reseta o sistema de áudio
 */
void md_audio_reset(void);

/**
 * @brief Escreve um valor em um registrador do PSG
 * @param reg Número do registrador
 * @param value Valor a ser escrito
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_audio_write_psg(uint8_t reg, uint8_t value);

/**
 * @brief Escreve um valor em um registrador do FM
 * @param port Porta do FM (0 ou 1)
 * @param reg Número do registrador
 * @param value Valor a ser escrito
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_audio_write_fm(uint8_t port, uint8_t reg, uint8_t value);

/**
 * @brief Lê um valor de um registrador do PSG
 * @param reg Número do registrador
 * @param value Ponteiro para armazenar o valor lido
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_audio_read_psg(uint8_t reg, uint8_t *value);

/**
 * @brief Lê um valor de um registrador do FM
 * @param port Porta do FM (0 ou 1)
 * @param reg Número do registrador
 * @param value Ponteiro para armazenar o valor lido
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_audio_read_fm(uint8_t port, uint8_t reg, uint8_t *value);

/**
 * @brief Atualiza o estado do sistema de áudio
 * @param cycles Número de ciclos a serem processados
 */
void md_audio_update(uint32_t cycles);

#ifdef __cplusplus
}
#endif

#endif /* MD_AUDIO_H */
