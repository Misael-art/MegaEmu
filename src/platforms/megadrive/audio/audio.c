/**
 * @file audio.c
 * @brief Implementação do sistema de áudio do Mega Drive
 */

#include "audio.h"
#include <string.h>

/* Estado global do sistema de áudio */
static md_audio_state_t g_state;

/* Funções de inicialização e finalização */

emu_error_t md_audio_init(void) {
  /* Limpa o estado */
  memset(&g_state, 0, sizeof(g_state));

  /* Inicializa os flags de ativação */
  g_state.psg_enabled = true;
  g_state.fm_enabled = true;

  return EMU_ERROR_NONE;
}

void md_audio_shutdown(void) {
  /* Limpa o estado */
  memset(&g_state, 0, sizeof(g_state));
}

void md_audio_reset(void) {
  /* Limpa os registradores */
  memset(g_state.psg_registers, 0, sizeof(g_state.psg_registers));
  memset(g_state.fm_registers, 0, sizeof(g_state.fm_registers));

  /* Mantém os flags de ativação */
  g_state.psg_enabled = true;
  g_state.fm_enabled = true;
}

/* Funções de acesso aos registradores do PSG */

emu_error_t md_audio_write_psg(uint8_t reg, uint8_t value) {
  /* Verifica se o registrador é válido */
  if (reg >= sizeof(g_state.psg_registers)) {
    return EMU_ERROR_INVALID_PARAMETER;
  }

  /* Escreve o valor no registrador */
  g_state.psg_registers[reg] = value;

  return EMU_ERROR_NONE;
}

emu_error_t md_audio_read_psg(uint8_t reg, uint8_t *value) {
  /* Verifica se os parâmetros são válidos */
  if (reg >= sizeof(g_state.psg_registers) || value == NULL) {
    return EMU_ERROR_INVALID_PARAMETER;
  }

  /* Lê o valor do registrador */
  *value = g_state.psg_registers[reg];

  return EMU_ERROR_NONE;
}

/* Funções de acesso aos registradores do FM */

emu_error_t md_audio_write_fm(uint8_t port, uint8_t reg, uint8_t value) {
  /* Verifica se a porta é válida */
  if (port > 1) {
    return EMU_ERROR_INVALID_PARAMETER;
  }

  /* Calcula o offset do registrador */
  uint16_t offset = (port * 0x100) + reg;

  /* Verifica se o registrador é válido */
  if (offset >= sizeof(g_state.fm_registers)) {
    return EMU_ERROR_INVALID_PARAMETER;
  }

  /* Escreve o valor no registrador */
  g_state.fm_registers[offset] = value;

  return EMU_ERROR_NONE;
}

emu_error_t md_audio_read_fm(uint8_t port, uint8_t reg, uint8_t *value) {
  /* Verifica se os parâmetros são válidos */
  if (port > 1 || value == NULL) {
    return EMU_ERROR_INVALID_PARAMETER;
  }

  /* Calcula o offset do registrador */
  uint16_t offset = (port * 0x100) + reg;

  /* Verifica se o registrador é válido */
  if (offset >= sizeof(g_state.fm_registers)) {
    return EMU_ERROR_INVALID_PARAMETER;
  }

  /* Lê o valor do registrador */
  *value = g_state.fm_registers[offset];

  return EMU_ERROR_NONE;
}

/* Função de atualização */

void md_audio_update(uint32_t cycles) {
  /* TODO: Implementar a geração de áudio */
  EMU_UNUSED_PARAM(cycles);
}
