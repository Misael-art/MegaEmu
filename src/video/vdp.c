/**
 * @file vdp.c
 * @brief Implementação do VDP (Video Display Processor) do Mega Drive
 */

#include "vdp.h"
#include <stdlib.h>
#include <string.h>

// Constantes internas
#define VDP_STATUS_FIFO_EMPTY 0x0200
#define VDP_STATUS_FIFO_FULL 0x0100
#define VDP_STATUS_VBLANK 0x0008
#define VDP_STATUS_HBLANK 0x0004
#define VDP_STATUS_DMA_BUSY 0x0002
#define VDP_STATUS_PAL 0x0001

#define VDP_REG_MODE1 0x00
#define VDP_REG_MODE2 0x01
#define VDP_REG_PLANE_A 0x02
#define VDP_REG_WINDOW 0x03
#define VDP_REG_PLANE_B 0x04
#define VDP_REG_SPRITE 0x05
#define VDP_REG_BGCOLOR 0x07
#define VDP_REG_HSCROLL 0x0D
#define VDP_REG_MODE3 0x0B
#define VDP_REG_MODE4 0x0C
#define VDP_REG_HSCROLL_BASE 0x0D
#define VDP_REG_AUTOINC 0x0F
#define VDP_REG_SCROLL_SIZE 0x10

// Funções de gerenciamento do VDP
vdp_t *vdp_create(void) {
  vdp_t *vdp = (vdp_t *)calloc(1, sizeof(vdp_t));
  if (vdp == NULL) {
    return NULL;
  }
  return vdp;
}

void vdp_destroy(vdp_t *vdp) {
  if (vdp != NULL) {
    free(vdp);
  }
}

bool vdp_init(vdp_t *vdp, emu_video_t video) {
  if (vdp == NULL || video == NULL) {
    return false;
  }

  // Limpa todas as memórias
  memset(vdp->vram, 0, VDP_VRAM_SIZE);
  memset(vdp->cram, 0, VDP_CRAM_SIZE);
  memset(vdp->vsram, 0, VDP_VSRAM_SIZE);
  memset(vdp->registers, 0, VDP_REGISTERS_SIZE);

  // Inicializa o estado
  vdp->status = VDP_STATUS_FIFO_EMPTY;
  vdp->control = 0;
  vdp->address = 0;
  vdp->access_mode = VDP_ACCESS_VRAM_READ;
  vdp->first_byte = true;
  vdp->pending_byte = 0;

  // Inicializa contadores de interrupção
  vdp->hblank_pending = false;
  vdp->vblank_pending = false;
  vdp->hblank_counter = 0;
  vdp->vblank_counter = 0;

  // Inicializa bases dos planos
  vdp->plane_a_base = 0;
  vdp->plane_b_base = 0;
  vdp->window_base = 0;
  vdp->sprite_table_base = 0;
  vdp->hscroll_base = 0;
  vdp->plane_width = 32;
  vdp->plane_height = 32;

  // Armazena referência ao sistema de vídeo
  vdp->video = video;

  return true;
}

void vdp_reset(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }

  // Reseta registradores
  memset(vdp->registers, 0, VDP_REGISTERS_SIZE);

  // Reseta estado
  vdp->status = VDP_STATUS_FIFO_EMPTY;
  vdp->control = 0;
  vdp->address = 0;
  vdp->access_mode = VDP_ACCESS_VRAM_READ;
  vdp->first_byte = true;
  vdp->pending_byte = 0;

  // Reseta interrupções
  vdp->hblank_pending = false;
  vdp->vblank_pending = false;
  vdp->hblank_counter = 0;
  vdp->vblank_counter = 0;

  // Reseta bases dos planos
  vdp->plane_a_base = 0;
  vdp->plane_b_base = 0;
  vdp->window_base = 0;
  vdp->sprite_table_base = 0;
  vdp->hscroll_base = 0;
  vdp->plane_width = 32;
  vdp->plane_height = 32;
}

// Funções de acesso ao VDP
uint8_t vdp_read_data(vdp_t *vdp) {
  if (vdp == NULL) {
    return 0;
  }

  uint8_t data = 0;

  switch (vdp->access_mode) {
  case VDP_ACCESS_VRAM_READ:
    data = vdp->vram[vdp->address & 0xFFFF];
    break;
  case VDP_ACCESS_CRAM_READ:
    data = (vdp->cram[(vdp->address >> 1) & 0x3F] >>
            ((vdp->address & 1) ? 8 : 0)) &
           0xFF;
    break;
  case VDP_ACCESS_VSRAM_READ:
    data = (vdp->vsram[(vdp->address >> 1) & 0x1F] >>
            ((vdp->address & 1) ? 8 : 0)) &
           0xFF;
    break;
  default:
    break;
  }

  vdp->address += vdp->registers[VDP_REG_AUTOINC];
  return data;
}

void vdp_write_data(vdp_t *vdp, uint8_t value) {
  if (vdp == NULL) {
    return;
  }

  switch (vdp->access_mode) {
  case VDP_ACCESS_VRAM_WRITE:
    vdp->vram[vdp->address & 0xFFFF] = value;
    break;
  case VDP_ACCESS_CRAM_WRITE:
    if (vdp->address & 1) {
      vdp->cram[(vdp->address >> 1) & 0x3F] =
          (vdp->cram[(vdp->address >> 1) & 0x3F] & 0x00FF) | (value << 8);
    } else {
      vdp->cram[(vdp->address >> 1) & 0x3F] =
          (vdp->cram[(vdp->address >> 1) & 0x3F] & 0xFF00) | value;
    }
    break;
  case VDP_ACCESS_VSRAM_WRITE:
    if (vdp->address & 1) {
      vdp->vsram[(vdp->address >> 1) & 0x1F] =
          (vdp->vsram[(vdp->address >> 1) & 0x1F] & 0x00FF) | (value << 8);
    } else {
      vdp->vsram[(vdp->address >> 1) & 0x1F] =
          (vdp->vsram[(vdp->address >> 1) & 0x1F] & 0xFF00) | value;
    }
    break;
  default:
    break;
  }

  vdp->address += vdp->registers[VDP_REG_AUTOINC];
}

uint8_t vdp_read_control(vdp_t *vdp) {
  if (vdp == NULL) {
    return 0;
  }

  uint8_t status = vdp->status & 0xFF;
  vdp->status &= ~(VDP_STATUS_VBLANK | VDP_STATUS_HBLANK);
  return status;
}

void vdp_write_control(vdp_t *vdp, uint8_t value) {
  if (vdp == NULL) {
    return;
  }

  if (vdp->first_byte) {
    vdp->pending_byte = value;
    vdp->first_byte = false;
  } else {
    uint8_t code = (value >> 6) & 0x03;
    uint16_t addr = ((value & 0x3F) << 8) | vdp->pending_byte;

    switch (code) {
    case 0: // VRAM Read
      vdp->access_mode = VDP_ACCESS_VRAM_READ;
      vdp->address = addr;
      break;
    case 1: // VRAM Write
      vdp->access_mode = VDP_ACCESS_VRAM_WRITE;
      vdp->address = addr;
      break;
    case 2: // Register Write
      if (addr & 0x1000) {
        vdp->access_mode = VDP_ACCESS_CRAM_WRITE;
        vdp->address = addr & 0x7F;
      } else {
        vdp->access_mode = VDP_ACCESS_VSRAM_WRITE;
        vdp->address = addr & 0x3F;
      }
      break;
    case 3: // Register Set
      if (addr < VDP_REGISTERS_SIZE) {
        vdp->registers[addr] = vdp->pending_byte;
        vdp_update_display_mode(vdp);
      }
      break;
    }
    vdp->first_byte = true;
  }
}

uint8_t vdp_read_hv_counter(vdp_t *vdp) {
  if (vdp == NULL) {
    return 0;
  }
  // Implementação simplificada - retorna apenas o contador H
  return vdp->hblank_counter & 0xFF;
}

// Funções de renderização
void vdp_render_line(vdp_t *vdp, int line) {
  if (vdp == NULL || line < 0 || line >= 224) {
    return;
  }

  // Atualiza os planos e sprites
  vdp_update_planes(vdp);
  vdp_update_sprites(vdp);

  // Incrementa contadores
  vdp->hblank_counter++;
  if (vdp->hblank_counter >= 342) {
    vdp->hblank_counter = 0;
    vdp->vblank_counter++;
    if (vdp->vblank_counter >= 262) {
      vdp->vblank_counter = 0;
    }
  }
}

void vdp_render_frame(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }

  // Renderiza todas as linhas
  for (int line = 0; line < 224; line++) {
    vdp_render_line(vdp, line);
  }

  // Atualiza o status
  vdp->status |= VDP_STATUS_VBLANK;
  vdp->vblank_pending = true;
}

void vdp_update_sprites(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }
  // TODO: Implementar atualização de sprites
}

void vdp_update_planes(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }
  // TODO: Implementar atualização de planos
}

// Funções de interrupção
bool vdp_check_interrupts(vdp_t *vdp) {
  if (vdp == NULL) {
    return false;
  }
  return vdp->vblank_pending || vdp->hblank_pending;
}

void vdp_acknowledge_vblank(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }
  vdp->vblank_pending = false;
}

void vdp_acknowledge_hblank(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }
  vdp->hblank_pending = false;
}

// Funções auxiliares
void vdp_update_timing(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }
  // TODO: Implementar atualização de timing
}

void vdp_update_scroll(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }
  // TODO: Implementar atualização de scroll
}

void vdp_update_window(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }
  // TODO: Implementar atualização de janela
}

void vdp_update_display_mode(vdp_t *vdp) {
  if (vdp == NULL) {
    return;
  }

  // Atualiza bases dos planos
  vdp->plane_a_base = (vdp->registers[VDP_REG_PLANE_A] & 0x38) << 10;
  vdp->plane_b_base = (vdp->registers[VDP_REG_PLANE_B] & 0x07) << 13;
  vdp->window_base = (vdp->registers[VDP_REG_WINDOW] & 0x3C) << 10;
  vdp->sprite_table_base = (vdp->registers[VDP_REG_SPRITE] & 0x7E) << 9;
  vdp->hscroll_base = (vdp->registers[VDP_REG_HSCROLL_BASE] & 0x3F) << 10;

  // Atualiza tamanho dos planos
  uint8_t size = vdp->registers[VDP_REG_SCROLL_SIZE] & 0x03;
  switch (size) {
  case 0:
    vdp->plane_width = 32;
    vdp->plane_height = 32;
    break;
  case 1:
    vdp->plane_width = 64;
    vdp->plane_height = 32;
    break;
  case 2:
    vdp->plane_width = 32;
    vdp->plane_height = 64;
    break;
  case 3:
    vdp->plane_width = 64;
    vdp->plane_height = 64;
    break;
  }
}
