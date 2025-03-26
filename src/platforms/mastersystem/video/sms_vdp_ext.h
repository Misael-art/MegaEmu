/**
 * @file sms_vdp_ext.h
 * @brief Interface de extensão do VDP do Master System
 */

#ifndef SMS_VDP_EXT_H
#define SMS_VDP_EXT_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/save_state.h"

// Flags para controlar efeitos especiais
#define SMS_VDP_EXT_FLAG_INTERLACE    0x01
#define SMS_VDP_EXT_FLAG_LINE_SCROLL  0x02
#define SMS_VDP_EXT_FLAG_CELL_SCROLL  0x04
#define SMS_VDP_EXT_FLAG_SPRITE_ZOOM  0x08
#define SMS_VDP_EXT_FLAG_HIRES_MODE   0x10

/**
 * @brief Interface de extensão do VDP do Master System
 *
 * Esta estrutura define a interface que deve ser implementada por extensões
 * do VDP do Master System, como por exemplo o VDP do Game Gear.
 */
typedef struct {
    /**
     * @brief Inicializa a extensão
     * @return Ponteiro para dados da extensão ou NULL em caso de erro
     */
    void *(*init)(void);

    /**
     * @brief Finaliza a extensão
     * @param ext Ponteiro para dados da extensão
     */
    void (*shutdown)(void *ext);

    /**
     * @brief Reseta a extensão
     * @param ext Ponteiro para dados da extensão
     */
    void (*reset)(void *ext);

    /**
     * @brief Processa escrita na CRAM
     * @param ext Ponteiro para dados da extensão
     * @param value Valor a ser escrito
     */
    void (*write_cram)(void *ext, uint8_t value);

    /**
     * @brief Lê da CRAM
     * @param ext Ponteiro para dados da extensão
     * @param addr Endereço da CRAM
     * @return Valor lido
     */
    uint16_t (*read_cram)(void *ext, uint8_t addr);

    /**
     * @brief Converte buffer do VDP do Master System
     * @param ext Ponteiro para dados da extensão
     * @param sms_buffer Buffer do VDP do Master System
     */
    void (*convert_buffer)(void *ext, const uint8_t *sms_buffer);

    /**
     * @brief Obtém buffer de tela da extensão
     * @param ext Ponteiro para dados da extensão
     * @return Ponteiro para o buffer de tela
     */
    const uint16_t *(*get_screen_buffer)(void *ext);

    /**
     * @brief Registra campos da extensão no sistema de save state
     * @param ext Ponteiro para dados da extensão
     * @param state Ponteiro para o save state
     * @return 0 se sucesso, -1 caso contrário
     */
    int (*register_save_state)(void *ext, save_state_t *state);

    /**
     * @brief Configura o modo interlace
     * @param ext Ponteiro para dados da extensão
     * @param enabled true para ativar, false para desativar
     */
    void (*set_interlace_mode)(void *ext, bool enabled);

    /**
     * @brief Configura valores de scroll para linhas específicas
     * @param ext Ponteiro para dados da extensão
     * @param line Número da linha (0-191)
     * @param scroll_x Valor do scroll horizontal
     * @param scroll_y Valor do scroll vertical (se suportado)
     */
    void (*set_line_scroll)(void *ext, uint8_t line, uint8_t scroll_x, uint8_t scroll_y);

    /**
     * @brief Configura flags de efeitos especiais
     * @param ext Ponteiro para dados da extensão
     * @param flags Bits de flag conforme definidos por SMS_VDP_EXT_FLAG_*
     */
    void (*set_special_effects)(void *ext, uint16_t flags);

    /**
     * @brief Ajusta o timing do VDP de forma precisa
     * @param ext Ponteiro para dados da extensão
     * @param h_counter Contador horizontal (0-255)
     * @param v_counter Contador vertical (0-255)
     * @param cycles Ciclos que se passaram
     * @return Número de ciclos até o próximo evento
     */
    uint8_t (*adjust_timing)(void *ext, uint8_t h_counter, uint8_t v_counter, uint8_t cycles);
} sms_vdp_ext_t;

#endif // SMS_VDP_EXT_H
