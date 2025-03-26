/**
 * @file vdp.h
 * @brief Definições e interface do VDP (Video Display Processor) do Mega Drive
 */

#ifndef EMU_VDP_H
#define EMU_VDP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../core/interfaces/video_interface.h"
#include <stdbool.h>
#include <stdint.h>

// Constantes do VDP
#define VDP_VRAM_SIZE 0x10000
#define VDP_CRAM_SIZE 0x80
#define VDP_VSRAM_SIZE 0x40
#define VDP_REGISTERS_SIZE 0x20

// Modos de acesso ao VDP
typedef enum {
  VDP_ACCESS_VRAM_READ,
  VDP_ACCESS_VRAM_WRITE,
  VDP_ACCESS_CRAM_WRITE,
  VDP_ACCESS_VSRAM_WRITE,
  VDP_ACCESS_CRAM_READ,
  VDP_ACCESS_VSRAM_READ,
  VDP_ACCESS_REGISTER
} vdp_access_mode_t;

// Estrutura do VDP
typedef struct {
  // Memórias
  uint8_t vram[VDP_VRAM_SIZE];
  uint16_t cram[VDP_CRAM_SIZE / 2];
  uint16_t vsram[VDP_VSRAM_SIZE / 2];
  uint8_t registers[VDP_REGISTERS_SIZE];

  // Estado do VDP
  uint16_t status;
  uint16_t control;
  uint32_t address;
  vdp_access_mode_t access_mode;
  bool first_byte;
  uint8_t pending_byte;

  // Controle de interrupção
  bool hblank_pending;
  bool vblank_pending;
  uint16_t hblank_counter;
  uint16_t vblank_counter;

  // Planos e sprites
  uint16_t plane_a_base;
  uint16_t plane_b_base;
  uint16_t window_base;
  uint16_t sprite_table_base;
  uint16_t hscroll_base;
  uint8_t plane_width;
  uint8_t plane_height;

  // Referência ao sistema de vídeo
  emu_video_t video;
} vdp_t;

// Funções de gerenciamento do VDP
vdp_t *vdp_create(void);
void vdp_destroy(vdp_t *vdp);
bool vdp_init(vdp_t *vdp, emu_video_t video);
void vdp_reset(vdp_t *vdp);

// Funções de acesso ao VDP
uint8_t vdp_read_data(vdp_t *vdp);
void vdp_write_data(vdp_t *vdp, uint8_t value);
uint8_t vdp_read_control(vdp_t *vdp);
void vdp_write_control(vdp_t *vdp, uint8_t value);
uint8_t vdp_read_hv_counter(vdp_t *vdp);

// Funções de renderização
void vdp_render_line(vdp_t *vdp, int line);
void vdp_render_frame(vdp_t *vdp);
void vdp_update_sprites(vdp_t *vdp);
void vdp_update_planes(vdp_t *vdp);

// Funções de interrupção
bool vdp_check_interrupts(vdp_t *vdp);
void vdp_acknowledge_vblank(vdp_t *vdp);
void vdp_acknowledge_hblank(vdp_t *vdp);

// Funções auxiliares
void vdp_update_timing(vdp_t *vdp);
void vdp_update_scroll(vdp_t *vdp);
void vdp_update_window(vdp_t *vdp);
void vdp_update_display_mode(vdp_t *vdp);

#ifdef __cplusplus
}
#endif

#endif // EMU_VDP_H
