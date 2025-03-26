/**
 * @file vdp_interrupts.c
 * @brief Implementação do sistema de interrupções do VDP do Mega Drive
 */

#include "vdp_interrupts.h"
#include "utils/log_utils.h"
#include "vdp.h"
#include <string.h>

// Estado das interrupções
static struct {
  uint8_t vint_enabled;        // Flag de habilitação de VINT
  uint8_t hint_enabled;        // Flag de habilitação de HINT
  uint8_t ext_enabled;         // Flag de habilitação de interrupção externa
  uint8_t vint_pending;        // Flag de VINT pendente
  uint8_t hint_pending;        // Flag de HINT pendente
  uint8_t ext_pending;         // Flag de interrupção externa pendente
  uint8_t hint_counter;        // Contador de linhas para HINT
  uint8_t hint_line;           // Linha para gerar HINT
  void (*vint_callback)(void); // Callback para VINT
  void (*hint_callback)(void); // Callback para HINT
  void (*ext_callback)(void);  // Callback para interrupção externa
} int_state;

/**
 * @brief Inicializa o sistema de interrupções
 */
void emu_vdp_interrupts_init(void) {
  memset(&int_state, 0, sizeof(int_state));
  LOG_INFO("Sistema de interrupções do VDP inicializado");
}

/**
 * @brief Reseta o estado das interrupções
 */
void emu_vdp_interrupts_reset(void) {
  memset(&int_state, 0, sizeof(int_state));
  LOG_INFO("Estado das interrupções do VDP resetado");
}

/**
 * @brief Habilita/desabilita a interrupção vertical (VINT)
 * @param enable Flag de habilitação
 */
void emu_vdp_set_vint_enable(uint8_t enable) {
  int_state.vint_enabled = enable ? 1 : 0;
  LOG_DEBUG("VINT %s", enable ? "habilitada" : "desabilitada");
}

/**
 * @brief Habilita/desabilita a interrupção horizontal (HINT)
 * @param enable Flag de habilitação
 */
void emu_vdp_set_hint_enable(uint8_t enable) {
  int_state.hint_enabled = enable ? 1 : 0;
  LOG_DEBUG("HINT %s", enable ? "habilitada" : "desabilitada");
}

/**
 * @brief Habilita/desabilita a interrupção externa
 * @param enable Flag de habilitação
 */
void emu_vdp_set_ext_enable(uint8_t enable) {
  int_state.ext_enabled = enable ? 1 : 0;
  LOG_DEBUG("Interrupção externa %s", enable ? "habilitada" : "desabilitada");
}

/**
 * @brief Define a linha para gerar HINT
 * @param line Número da linha
 */
void emu_vdp_set_hint_line(uint8_t line) {
  int_state.hint_line = line;
  LOG_DEBUG("Linha de HINT definida: %d", line);
}

/**
 * @brief Define o callback para VINT
 * @param callback Função de callback
 */
void emu_vdp_set_vint_callback(void (*callback)(void)) {
  int_state.vint_callback = callback;
}

/**
 * @brief Define o callback para HINT
 * @param callback Função de callback
 */
void emu_vdp_set_hint_callback(void (*callback)(void)) {
  int_state.hint_callback = callback;
}

/**
 * @brief Define o callback para interrupção externa
 * @param callback Função de callback
 */
void emu_vdp_set_ext_callback(void (*callback)(void)) {
  int_state.ext_callback = callback;
}

/**
 * @brief Processa interrupções para uma linha de varredura
 * @param line Número da linha
 */
void emu_vdp_process_interrupts(int line) {
  // Processar HINT
  if (int_state.hint_enabled) {
    if (int_state.hint_counter == 0) {
      int_state.hint_counter = int_state.hint_line;
      int_state.hint_pending = 1;

      if (int_state.hint_callback) {
        int_state.hint_callback();
      }

      LOG_DEBUG("HINT gerada na linha %d", line);
    }

    int_state.hint_counter--;
  }

  // Processar VINT no início do vblank
  if (line == 224 && int_state.vint_enabled) {
    int_state.vint_pending = 1;

    if (int_state.vint_callback) {
      int_state.vint_callback();
    }

    LOG_DEBUG("VINT gerada");
  }
}

/**
 * @brief Verifica se há VINT pendente
 * @return 1 se houver VINT pendente, 0 caso contrário
 */
int emu_vdp_check_vint(void) { return int_state.vint_pending; }

/**
 * @brief Verifica se há HINT pendente
 * @return 1 se houver HINT pendente, 0 caso contrário
 */
int emu_vdp_check_hint(void) { return int_state.hint_pending; }

/**
 * @brief Verifica se há interrupção externa pendente
 * @return 1 se houver interrupção externa pendente, 0 caso contrário
 */
int emu_vdp_check_ext(void) { return int_state.ext_pending; }

/**
 * @brief Limpa o flag de VINT pendente
 */
void emu_vdp_clear_vint(void) { int_state.vint_pending = 0; }

/**
 * @brief Limpa o flag de HINT pendente
 */
void emu_vdp_clear_hint(void) { int_state.hint_pending = 0; }

/**
 * @brief Limpa o flag de interrupção externa pendente
 */
void emu_vdp_clear_ext(void) { int_state.ext_pending = 0; }

/**
 * @brief Gera uma interrupção externa
 */
void emu_vdp_trigger_ext(void) {
  if (int_state.ext_enabled) {
    int_state.ext_pending = 1;

    if (int_state.ext_callback) {
      int_state.ext_callback();
    }

    LOG_DEBUG("Interrupção externa gerada");
  }
}

/**
 * @brief Obtém o estado atual das interrupções
 * @param state Estrutura para receber o estado
 */
void emu_vdp_get_interrupt_state(emu_vdp_interrupt_state_t *state) {
  if (state) {
    state->vint_enabled = int_state.vint_enabled;
    state->hint_enabled = int_state.hint_enabled;
    state->ext_enabled = int_state.ext_enabled;
    state->vint_pending = int_state.vint_pending;
    state->hint_pending = int_state.hint_pending;
    state->ext_pending = int_state.ext_pending;
    state->hint_counter = int_state.hint_counter;
    state->hint_line = int_state.hint_line;
  }
}
