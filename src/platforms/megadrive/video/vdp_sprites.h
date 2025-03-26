/**
 * @file vdp_sprites.h
 * @brief Definições e protótipos para o sistema de sprites do VDP do Mega Drive
 */

#ifndef EMU_VDP_SPRITES_H
#define EMU_VDP_SPRITES_H

#include <stdint.h>

// Constantes
#define VDP_MAX_SPRITES 80
#define VDP_MAX_SPRITE_SIZE 4
#define VDP_SPRITE_CELL_SIZE 8

// Estrutura para informações de sprite
typedef struct {
  uint16_t x;       // Posição X
  uint16_t y;       // Posição Y
  uint8_t width;    // Largura em células (8 pixels)
  uint8_t height;   // Altura em células (8 pixels)
  uint16_t pattern; // Padrão base na VRAM
  uint8_t priority; // Prioridade de renderização
  uint8_t palette;  // Índice da paleta
  uint8_t flip_h;   // Flip horizontal
  uint8_t flip_v;   // Flip vertical
  uint8_t visible;  // Visibilidade do sprite
} emu_vdp_sprite_info_t;

// Funções de inicialização e controle
void emu_vdp_sprites_init(void);
void emu_vdp_sprites_reset(void);

// Funções de configuração de sprites
int emu_vdp_sprite_set(uint8_t index, uint16_t x, uint16_t y, uint8_t width,
                       uint8_t height, uint16_t pattern, uint8_t priority,
                       uint8_t palette, uint8_t flip_h, uint8_t flip_v);
int emu_vdp_sprite_set_visible(uint8_t index, uint8_t visible);
int emu_vdp_sprite_set_position(uint8_t index, uint16_t x, uint16_t y);
int emu_vdp_sprite_set_pattern(uint8_t index, uint16_t pattern);
int emu_vdp_sprite_set_palette(uint8_t index, uint8_t palette);
int emu_vdp_sprite_set_priority(uint8_t index, uint8_t priority);
int emu_vdp_sprite_set_flip(uint8_t index, uint8_t flip_h, uint8_t flip_v);
int emu_vdp_sprite_set_link(uint8_t index, uint8_t link);

// Funções de consulta
int emu_vdp_sprite_get(uint8_t index, emu_vdp_sprite_info_t *sprite);
int emu_vdp_sprites_get_count(void);
int emu_vdp_sprites_check_overflow(void);
int emu_vdp_sprites_check_collision(void);
void emu_vdp_sprites_clear_collision(void);

// Funções de processamento
int emu_vdp_sprites_process_line(int line, int max_sprites);

#endif // EMU_VDP_SPRITES_H
