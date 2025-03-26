/**
 * @file vdp_interrupts.h
 * @brief Definições e protótipos para o sistema de interrupções do VDP do Mega
 * Drive
 */

#ifndef EMU_VDP_INTERRUPTS_H
#define EMU_VDP_INTERRUPTS_H

#include <stdint.h>

// Estrutura para armazenar o estado das interrupções
typedef struct {
  uint8_t vint_enabled; // Flag de habilitação de VINT
  uint8_t hint_enabled; // Flag de habilitação de HINT
  uint8_t ext_enabled;  // Flag de habilitação de interrupção externa
  uint8_t vint_pending; // Flag de VINT pendente
  uint8_t hint_pending; // Flag de HINT pendente
  uint8_t ext_pending;  // Flag de interrupção externa pendente
  uint8_t hint_counter; // Contador de linhas para HINT
  uint8_t hint_line;    // Linha para gerar HINT
} emu_vdp_interrupt_state_t;

// Funções de inicialização e controle
void emu_vdp_interrupts_init(void);
void emu_vdp_interrupts_reset(void);

// Funções de configuração
void emu_vdp_set_vint_enable(uint8_t enable);
void emu_vdp_set_hint_enable(uint8_t enable);
void emu_vdp_set_ext_enable(uint8_t enable);
void emu_vdp_set_hint_line(uint8_t line);

// Funções de callback
void emu_vdp_set_vint_callback(void (*callback)(void));
void emu_vdp_set_hint_callback(void (*callback)(void));
void emu_vdp_set_ext_callback(void (*callback)(void));

// Funções de processamento
void emu_vdp_process_interrupts(int line);

// Funções de verificação
int emu_vdp_check_vint(void);
int emu_vdp_check_hint(void);
int emu_vdp_check_ext(void);

// Funções de limpeza de flags
void emu_vdp_clear_vint(void);
void emu_vdp_clear_hint(void);
void emu_vdp_clear_ext(void);

// Funções de controle externo
void emu_vdp_trigger_ext(void);
void emu_vdp_get_interrupt_state(emu_vdp_interrupt_state_t *state);

#endif // EMU_VDP_INTERRUPTS_H
