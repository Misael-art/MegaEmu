/**
 * @file vdp_registers.c
 * @brief Implementação das funções de processamento de registradores do VDP do
 * Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "vdp_adapter.h"
#include <stdlib.h>
#include <string.h>

// Constantes de registradores
#define REG_MODE1 0x00
#define REG_MODE2 0x01
#define REG_PLANE_A 0x02
#define REG_WINDOW 0x03
#define REG_PLANE_B 0x04
#define REG_SPRITE 0x05
#define REG_BG_COLOR 0x07
#define REG_HINT 0x0A
#define REG_MODE3 0x0B
#define REG_MODE4 0x0C
#define REG_HSCROLL 0x0D
#define REG_AUTOINC 0x0F
#define REG_SCROLL_SIZE 0x10

// Máscaras de bits para registradores
#define MODE1_VINT_EN (1 << 5)
#define MODE1_DMA_EN (1 << 4)
#define MODE1_V30_MODE (1 << 3)
#define MODE1_DISPLAY_EN (1 << 6)

#define MODE2_VINT_EN (1 << 5)
#define MODE2_DMA_EN (1 << 4)
#define MODE2_V30_MODE (1 << 3)
#define MODE2_128KB_EN (1 << 2)

#define MODE3_VSCROLL (1 << 0)
#define MODE3_HSCROLL (1 << 1)
#define MODE3_EXINT_EN (1 << 3)

#define MODE4_H40_MODE (1 << 0)
#define MODE4_SHADOW_EN (1 << 3)
#define MODE4_INTERLACE (1 << 2)

// Estrutura para configuração de plano
typedef struct {
  uint16_t base_addr;
  uint8_t width;
  uint8_t height;
  bool priority;
} plane_config_t;

// Funções auxiliares estáticas
static void update_display_mode(megadrive_vdp_context_t *ctx) {
  bool h40 = (ctx->regs[REG_MODE4] & MODE4_H40_MODE) != 0;
  bool v30 = (ctx->regs[REG_MODE2] & MODE2_V30_MODE) != 0;

  if (h40) {
    ctx->mode = v30 ? MD_VDP_MODE_H40_V30 : MD_VDP_MODE_H40_V28;
  } else {
    ctx->mode = v30 ? MD_VDP_MODE_H32_V30 : MD_VDP_MODE_H32_V28;
  }
}

static void update_interrupts(megadrive_vdp_context_t *ctx) {
  // Configura interrupção vertical
  bool vint_enabled = (ctx->regs[REG_MODE2] & MODE2_VINT_EN) != 0;
  if (vint_enabled && ctx->vint_pending && ctx->vint_callback) {
    ctx->vint_callback(ctx->callback_data);
  }

  // Configura interrupção horizontal
  if (ctx->regs[REG_MODE1] & MODE1_DMA_EN) {
    ctx->hint_value = ctx->regs[REG_HINT];
    ctx->hint_counter = ctx->hint_value;
  }
}

static void update_plane_config(megadrive_vdp_context_t *ctx, uint8_t reg,
                                plane_config_t *config) {
  switch (reg) {
  case REG_PLANE_A:
    config->base_addr = (ctx->regs[reg] << 10) & 0xE000;
    break;

  case REG_PLANE_B:
    config->base_addr = (ctx->regs[reg] << 13) & 0xE000;
    break;

  case REG_WINDOW:
    config->base_addr = (ctx->regs[reg] << 10) & 0xF000;
    break;

  case REG_SCROLL_SIZE:
    if (reg == REG_PLANE_A || reg == REG_WINDOW) {
      config->width = ((ctx->regs[REG_SCROLL_SIZE] >> 0) & 0x03) * 32;
      config->height = ((ctx->regs[REG_SCROLL_SIZE] >> 4) & 0x03) * 32;
    } else {
      config->width = ((ctx->regs[REG_SCROLL_SIZE] >> 2) & 0x03) * 32;
      config->height = ((ctx->regs[REG_SCROLL_SIZE] >> 6) & 0x03) * 32;
    }
    break;
  }
}

static void update_sprite_config(megadrive_vdp_context_t *ctx) {
  // Configura tabela de atributos de sprites
  ctx->sprite_table = (ctx->regs[REG_SPRITE] << 9) & 0xFC00;

  // Configura limites de sprites
  ctx->sprite_limit =
      (ctx->mode == MD_VDP_MODE_H40_V30 || ctx->mode == MD_VDP_MODE_H40_V28)
          ? 80
          : 64;
}

static void update_scroll_config(megadrive_vdp_context_t *ctx) {
  // Configura base da tabela de scroll horizontal
  ctx->hscroll_base = (ctx->regs[REG_HSCROLL] << 10) & 0xFC00;

  // Configura modo de scroll
  ctx->hscroll_mode = (ctx->regs[REG_MODE3] >> 0) & 0x03;
  ctx->vscroll_mode = (ctx->regs[REG_MODE3] >> 2) & 0x01;
}

// Funções públicas
void vdp_write_register(megadrive_vdp_context_t *ctx, uint8_t reg,
                        uint8_t value) {
  if (!ctx || reg >= MD_VDP_REG_COUNT)
    return;

  // Salva valor anterior para detectar mudanças
  uint8_t old_value = ctx->regs[reg];
  ctx->regs[reg] = value;

  // Processa mudanças específicas
  switch (reg) {
  case REG_MODE1:
  case REG_MODE2:
    if ((value ^ old_value) & (MODE1_V30_MODE | MODE2_V30_MODE)) {
      update_display_mode(ctx);
    }
    if ((value ^ old_value) & (MODE1_VINT_EN | MODE2_VINT_EN)) {
      update_interrupts(ctx);
    }
    break;

  case REG_MODE4:
    if ((value ^ old_value) & MODE4_H40_MODE) {
      update_display_mode(ctx);
    }
    break;

  case REG_PLANE_A:
  case REG_PLANE_B:
  case REG_WINDOW:
  case REG_SCROLL_SIZE: {
    plane_config_t config = {0};
    update_plane_config(ctx, reg, &config);
    // Atualiza configuração do plano específico
    if (reg == REG_PLANE_A) {
      ctx->plane_a_base = config.base_addr;
      ctx->plane_a_width = config.width;
      ctx->plane_a_height = config.height;
    } else if (reg == REG_PLANE_B) {
      ctx->plane_b_base = config.base_addr;
      ctx->plane_b_width = config.width;
      ctx->plane_b_height = config.height;
    } else {
      ctx->window_base = config.base_addr;
      ctx->window_width = config.width;
      ctx->window_height = config.height;
    }
    break;
  }

  case REG_SPRITE:
    update_sprite_config(ctx);
    break;

  case REG_MODE3:
  case REG_HSCROLL:
    update_scroll_config(ctx);
    break;

  case REG_HINT:
    ctx->hint_value = value;
    ctx->hint_counter = value;
    break;

  case REG_AUTOINC:
    ctx->addr_increment = value;
    break;
  }
}

uint8_t vdp_read_register(const megadrive_vdp_context_t *ctx, uint8_t reg) {
  if (!ctx || reg >= MD_VDP_REG_COUNT)
    return 0xFF;

  return ctx->regs[reg];
}

void vdp_get_plane_info(const megadrive_vdp_context_t *ctx, uint8_t plane,
                        uint16_t *base_addr, uint8_t *width, uint8_t *height) {
  if (!ctx || !base_addr || !width || !height)
    return;

  switch (plane) {
  case 0: // Plano A
    *base_addr = ctx->plane_a_base;
    *width = ctx->plane_a_width;
    *height = ctx->plane_a_height;
    break;

  case 1: // Plano B
    *base_addr = ctx->plane_b_base;
    *width = ctx->plane_b_width;
    *height = ctx->plane_b_height;
    break;

  case 2: // Window
    *base_addr = ctx->window_base;
    *width = ctx->window_width;
    *height = ctx->window_height;
    break;

  default:
    *base_addr = 0;
    *width = 0;
    *height = 0;
    break;
  }
}

void vdp_get_sprite_info(const megadrive_vdp_context_t *ctx,
                         uint16_t *table_addr, uint8_t *max_sprites) {
  if (!ctx || !table_addr || !max_sprites)
    return;

  *table_addr = ctx->sprite_table;
  *max_sprites = ctx->sprite_limit;
}

void vdp_get_scroll_info(const megadrive_vdp_context_t *ctx,
                         uint16_t *hscroll_addr, uint8_t *hscroll_mode,
                         uint8_t *vscroll_mode) {
  if (!ctx || !hscroll_addr || !hscroll_mode || !vscroll_mode)
    return;

  *hscroll_addr = ctx->hscroll_base;
  *hscroll_mode = ctx->hscroll_mode;
  *vscroll_mode = ctx->vscroll_mode;
}
