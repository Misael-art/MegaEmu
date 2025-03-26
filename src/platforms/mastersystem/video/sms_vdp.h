/**
 * @file sms_vdp.h
 * @brief VDP do Master System com suporte a extensões
 */

#ifndef SMS_VDP_H
#define SMS_VDP_H

#include "../../../core/save_state.h"
#include "sms_vdp_ext.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Dimensões da tela do Master System
 */
#define SMS_SCREEN_WIDTH 256
#define SMS_SCREEN_HEIGHT 192
#define SMS_GG_SCREEN_WIDTH 160
#define SMS_GG_SCREEN_HEIGHT 144

// Dimensões do buffer do VDP
#define SMS_VDP_BUFFER_WIDTH 256
#define SMS_VDP_BUFFER_HEIGHT 192

// Registradores do VDP
#define SMS_VDP_REGISTERS 16

// Tamanho da VRAM e CRAM
#define SMS_VDP_VRAM_SIZE 0x4000
#define SMS_VDP_CRAM_SIZE 32

// Flags de status do VDP
#define SMS_VDP_STATUS_VBLANK 0x80
#define SMS_VDP_STATUS_SPRITE_OVERFLOW 0x40
#define SMS_VDP_STATUS_SPRITE_COLLISION 0x20
#define SMS_VDP_STATUS_FRAME_INTERRUPT 0x80

// Modos de escrita
#define SMS_VDP_CODE_REGISTER 0x80
#define SMS_VDP_CODE_CRAM 0xC0
#define SMS_VDP_CODE_VRAM 0x40
#define SMS_VDP_CODE_PALETTE 0x00

/**
 * @brief Opaque handle para o VDP
 */
typedef struct sms_vdp_t sms_vdp_t;

/**
 * @brief Cria uma nova instância do VDP
 * @param ext Ponteiro para interface de extensão (opcional)
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_vdp_t *sms_vdp_create(const sms_vdp_ext_t *ext);

/**
 * @brief Destrói uma instância do VDP e libera recursos
 *
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_destroy(sms_vdp_t *vdp);

/**
 * @brief Reseta o VDP para o estado inicial
 *
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_reset(sms_vdp_t *vdp);

/**
 * @brief Conecta o VDP à CPU
 *
 * @param vdp Ponteiro para a instância
 * @param cpu Ponteiro para a CPU
 */
void sms_vdp_connect_cpu(sms_vdp_t *vdp, void *cpu);

/**
 * @brief Inicia um novo frame no VDP
 *
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_start_frame(sms_vdp_t *vdp);

/**
 * @brief Atualiza o estado do VDP com base nos ciclos executados
 *
 * @param vdp Ponteiro para a instância
 * @param cycles Número de ciclos executados
 */
void sms_vdp_update(sms_vdp_t *vdp, uint8_t cycles);

/**
 * @brief Finaliza o frame atual e renderiza para o buffer
 *
 * @param vdp Ponteiro para a instância
 * @param frame_buffer Buffer para receber os dados do frame (32 bits RGBA)
 */
void sms_vdp_end_frame(sms_vdp_t *vdp, uint32_t *frame_buffer);

/**
 * @brief Verifica se o VDP está gerando uma interrupção
 *
 * @param vdp Ponteiro para a instância
 * @return 1 se há interrupção, 0 caso contrário
 */
uint8_t sms_vdp_check_interrupt(sms_vdp_t *vdp);

/**
 * @brief Lê um byte da porta de dados do VDP
 *
 * @param vdp Ponteiro para a instância do VDP
 * @return Valor lido da porta de dados
 */
uint8_t sms_vdp_read_data_port(sms_vdp_t *vdp);

/**
 * @brief Lê um byte da porta de status do VDP
 *
 * @param vdp Ponteiro para a instância do VDP
 * @return Valor lido da porta de status
 */
uint8_t sms_vdp_read_status_port(sms_vdp_t *vdp);

/**
 * @brief Escreve um byte na porta de dados do VDP
 *
 * @param vdp Ponteiro para a instância do VDP
 * @param value Valor a ser escrito
 */
void sms_vdp_write_data_port(sms_vdp_t *vdp, uint8_t value);

/**
 * @brief Escreve um byte na porta de controle do VDP
 *
 * @param vdp Ponteiro para a instância do VDP
 * @param value Valor a ser escrito
 */
void sms_vdp_write_control_port(sms_vdp_t *vdp, uint8_t value);

/**
 * @brief Atualiza o estado interno do VDP após um carregamento de estado
 *
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_update_after_state_load(sms_vdp_t *vdp);

/**
 * @brief Registra o estado do VDP no sistema de save state
 *
 * @param vdp Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_vdp_register_save_state(sms_vdp_t *vdp, save_state_t *state);

/**
 * @brief Escreve um valor em um registrador do VDP
 * @param vdp Ponteiro para a instância
 * @param reg Número do registrador
 * @param value Valor a ser escrito
 */
void sms_vdp_write_register(sms_vdp_t *vdp, uint8_t reg, uint8_t value);

/**
 * @brief Lê um registrador do VDP
 * @param vdp Ponteiro para a instância
 * @param reg Número do registrador
 * @return Valor do registrador
 */
uint8_t sms_vdp_read_register(sms_vdp_t *vdp, uint8_t reg);

/**
 * @brief Escreve um valor na porta de dados do VDP
 * @param vdp Ponteiro para a instância
 * @param value Valor a ser escrito
 */
void sms_vdp_write_data(sms_vdp_t *vdp, uint8_t value);

/**
 * @brief Lê um valor da porta de dados do VDP
 * @param vdp Ponteiro para a instância
 * @return Valor lido
 */
uint8_t sms_vdp_read_data(sms_vdp_t *vdp);

/**
 * @brief Escreve um valor na porta de controle do VDP
 * @param vdp Ponteiro para a instância
 * @param value Valor a ser escrito
 */
void sms_vdp_write_control(sms_vdp_t *vdp, uint8_t value);

/**
 * @brief Lê o registrador de status do VDP
 * @param vdp Ponteiro para a instância
 * @return Valor do registrador de status
 */
uint8_t sms_vdp_read_status(sms_vdp_t *vdp);

/**
 * @brief Executa um frame do VDP
 * @param vdp Ponteiro para a instância
 * @return true se uma interrupção vertical foi gerada
 */
bool sms_vdp_run_frame(sms_vdp_t *vdp);

/**
 * @brief Obtém o buffer de tela do VDP
 * @param vdp Ponteiro para a instância
 * @return Ponteiro para o buffer de tela
 */
const uint16_t *sms_vdp_get_screen_buffer(sms_vdp_t *vdp);

/**
 * @brief Configura o modo interlace
 * @param vdp Ponteiro para a instância
 * @param enabled true para ativar, false para desativar
 */
void sms_vdp_set_interlace_mode(sms_vdp_t *vdp, bool enabled);

/**
 * @brief Configura o scroll para uma linha específica
 * @param vdp Ponteiro para a instância
 * @param line Número da linha (0-191)
 * @param scroll_x Valor do scroll horizontal
 * @param scroll_y Valor do scroll vertical (se suportado)
 */
void sms_vdp_set_line_scroll(sms_vdp_t *vdp, uint8_t line, uint8_t scroll_x,
                             uint8_t scroll_y);

/**
 * @brief Configura flags de efeitos especiais
 * @param vdp Ponteiro para a instância
 * @param flags Bits de flag conforme definidos por SMS_VDP_EXT_FLAG_*
 */
void sms_vdp_set_special_effects(sms_vdp_t *vdp, uint16_t flags);

/**
 * @brief Atualiza o timing do VDP de forma precisa
 * @param vdp Ponteiro para a instância
 * @param cycles Número de ciclos a avançar
 * @return Número de ciclos até o próximo evento
 */
uint8_t sms_vdp_update_timing(sms_vdp_t *vdp, uint8_t cycles);

#endif /* SMS_VDP_H */
