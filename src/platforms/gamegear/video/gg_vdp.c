/**
 * @file gg_vdp.c
 * @brief Implementação das funções específicas do VDP do Game Gear
 */

#include "gg_vdp.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../../mastersystem/video/sms_vdp.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG_VDP EMU_LOG_CAT_VIDEO

// Macros de log
#define GG_VDP_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG_VDP, __VA_ARGS__)
#define GG_VDP_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG_VDP, __VA_ARGS__)
#define GG_VDP_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG_VDP, __VA_ARGS__)
#define GG_VDP_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG_VDP, __VA_ARGS__)
#define GG_VDP_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG_VDP, __VA_ARGS__)

/**
 * @brief Estrutura para extensão do VDP do Game Gear
 */
typedef struct {
  uint16_t cram[GG_TOTAL_COLORS]; // Memória de cores do Game Gear
  uint16_t screen_buffer[GG_SCREEN_WIDTH * GG_SCREEN_HEIGHT]; // Buffer da tela
  bool cram_latch;   // Latch para escrita na CRAM
  uint8_t cram_byte; // Byte temporário para escrita na CRAM
} gg_vdp_ext_t;

/**
 * @brief Inicializa a extensão do VDP do Game Gear
 * @param vdp Ponteiro para o VDP do Master System
 * @return Ponteiro para a extensão ou NULL em caso de erro
 */
void *gg_vdp_init(void) {
  gg_vdp_ext_t *ext = (gg_vdp_ext_t *)malloc(sizeof(gg_vdp_ext_t));
  if (!ext) {
    GG_VDP_LOG_ERROR("Falha ao alocar memória para extensão do VDP");
    return NULL;
  }

  // Inicializa estrutura
  memset(ext, 0, sizeof(gg_vdp_ext_t));

  GG_VDP_LOG_INFO("Extensão do VDP do Game Gear inicializada");
  return ext;
}

/**
 * @brief Libera a extensão do VDP do Game Gear
 * @param ext Ponteiro para a extensão
 */
void gg_vdp_shutdown(void *ext) {
  if (!ext)
    return;
  free(ext);
  GG_VDP_LOG_INFO("Extensão do VDP do Game Gear finalizada");
}

/**
 * @brief Reseta a extensão do VDP do Game Gear
 * @param ext Ponteiro para a extensão
 */
void gg_vdp_reset(void *ext) {
  if (!ext)
    return;
  gg_vdp_ext_t *gg_ext = (gg_vdp_ext_t *)ext;

  // Limpa memória de cores e buffer
  memset(gg_ext->cram, 0, sizeof(gg_ext->cram));
  memset(gg_ext->screen_buffer, 0, sizeof(gg_ext->screen_buffer));
  gg_ext->cram_latch = false;
  gg_ext->cram_byte = 0;

  GG_VDP_LOG_INFO("Extensão do VDP do Game Gear resetada");
}

/**
 * @brief Processa escrita na CRAM do Game Gear
 * @param ext Ponteiro para a extensão
 * @param value Valor a ser escrito
 */
void gg_vdp_write_cram(void *ext, uint8_t value) {
  if (!ext)
    return;
  gg_vdp_ext_t *gg_ext = (gg_vdp_ext_t *)ext;

  if (!gg_ext->cram_latch) {
    // Primeiro byte
    gg_ext->cram_byte = value;
    gg_ext->cram_latch = true;
  } else {
    // Segundo byte - completa a cor de 12 bits
    uint16_t color = ((value & 0x0F) << 8) | gg_ext->cram_byte;
    uint16_t addr = (value >> 4) & 0x3F;

    if (addr < GG_TOTAL_COLORS) {
      gg_ext->cram[addr] = color & GG_COLOR_MASK;
      GG_VDP_LOG_TRACE("CRAM[%02X] = %03X", addr, color & GG_COLOR_MASK);
    }

    gg_ext->cram_latch = false;
  }
}

/**
 * @brief Lê da CRAM do Game Gear
 * @param ext Ponteiro para a extensão
 * @param addr Endereço da CRAM
 * @return Valor lido
 */
uint16_t gg_vdp_read_cram(void *ext, uint8_t addr) {
  if (!ext)
    return 0;
  gg_vdp_ext_t *gg_ext = (gg_vdp_ext_t *)ext;

  if (addr < GG_TOTAL_COLORS) {
    return gg_ext->cram[addr];
  }

  return 0;
}

/**
 * @brief Converte o buffer do VDP do Master System para o formato do Game Gear
 * @param ext Ponteiro para a extensão
 * @param sms_buffer Buffer do VDP do Master System
 */
void gg_vdp_convert_buffer(void *ext, const uint8_t *sms_buffer) {
  if (!ext || !sms_buffer)
    return;
  gg_vdp_ext_t *gg_ext = (gg_vdp_ext_t *)ext;

  // Converte buffer linha por linha
  for (int y = 0; y < GG_SCREEN_HEIGHT; y++) {
    int sms_y = y + GG_SCREEN_Y_OFFSET;

    for (int x = 0; x < GG_SCREEN_WIDTH; x++) {
      int sms_x = x + GG_SCREEN_X_OFFSET;

      // Obtém índice de cor do buffer SMS
      int sms_idx = sms_y * GG_VDP_BUFFER_WIDTH + sms_x;
      uint8_t color_idx = sms_buffer[sms_idx];

      // Obtém cor da CRAM e converte para RGB565
      uint16_t gg_color = gg_ext->cram[color_idx];
      uint16_t rgb565 = gg_color_to_rgb565(gg_color);

      // Armazena no buffer do Game Gear
      gg_ext->screen_buffer[y * GG_SCREEN_WIDTH + x] = rgb565;
    }
  }
}

/**
 * @brief Obtém o buffer de tela do Game Gear
 * @param ext Ponteiro para a extensão
 * @return Ponteiro para o buffer de tela
 */
const uint16_t *gg_vdp_get_screen_buffer(void *ext) {
  if (!ext)
    return NULL;
  gg_vdp_ext_t *gg_ext = (gg_vdp_ext_t *)ext;
  return gg_ext->screen_buffer;
}

/**
 * @brief Registra campos da extensão no sistema de save state
 * @param ext Ponteiro para a extensão
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_vdp_register_save_state(void *ext, save_state_t *state) {
  if (!ext || !state)
    return -1;
  gg_vdp_ext_t *gg_ext = (gg_vdp_ext_t *)ext;

  save_state_register_field(state, "gg_vdp_cram", gg_ext->cram,
                            sizeof(gg_ext->cram));
  save_state_register_field(state, "gg_vdp_cram_latch", &gg_ext->cram_latch,
                            sizeof(gg_ext->cram_latch));
  save_state_register_field(state, "gg_vdp_cram_byte", &gg_ext->cram_byte,
                            sizeof(gg_ext->cram_byte));

  return 0;
}

// Interface de extensão do VDP
const sms_vdp_ext_t gg_vdp_ext = {.init = gg_vdp_init,
                                  .shutdown = gg_vdp_shutdown,
                                  .reset = gg_vdp_reset,
                                  .write_cram = gg_vdp_write_cram,
                                  .read_cram = gg_vdp_read_cram,
                                  .convert_buffer = gg_vdp_convert_buffer,
                                  .get_screen_buffer = gg_vdp_get_screen_buffer,
                                  .register_save_state =
                                      gg_vdp_register_save_state};
