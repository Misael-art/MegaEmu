/**
 * @file vdp_scroll.h
 * @brief Definições e protótipos para o sistema de scrolling do VDP do Mega
 * Drive
 */

#ifndef EMU_VDP_SCROLL_H
#define EMU_VDP_SCROLL_H

#include <stdint.h>

// Modos de scroll horizontal
typedef enum {
  VDP_HSCROLL_MODE_FULL, // Scroll de tela inteira
  VDP_HSCROLL_MODE_CELL, // Scroll por célula (8 pixels)
  VDP_HSCROLL_MODE_LINE  // Scroll por linha
} vdp_hscroll_mode_t;

// Modos de scroll vertical
typedef enum {
  VDP_VSCROLL_MODE_FULL, // Scroll de tela inteira
  VDP_VSCROLL_MODE_2CELL // Scroll por 2 células (16 pixels)
} vdp_vscroll_mode_t;

// Estrutura para armazenar o estado do scrolling
typedef struct {
  // Plano A
  uint16_t plane_a_base;     // Endereço base do plano A
  uint16_t plane_a_width;    // Largura do plano A em células
  uint16_t plane_a_height;   // Altura do plano A em células
  uint16_t plane_a_scroll_x; // Scroll horizontal do plano A
  uint16_t plane_a_scroll_y; // Scroll vertical do plano A
  uint8_t plane_a_enabled;   // Flag de habilitação do plano A

  // Plano B
  uint16_t plane_b_base;     // Endereço base do plano B
  uint16_t plane_b_width;    // Largura do plano B em células
  uint16_t plane_b_height;   // Altura do plano B em células
  uint16_t plane_b_scroll_x; // Scroll horizontal do plano B
  uint16_t plane_b_scroll_y; // Scroll vertical do plano B
  uint8_t plane_b_enabled;   // Flag de habilitação do plano B

  // Plano de janela
  uint16_t window_base;   // Endereço base do plano de janela
  uint16_t window_x;      // Posição X da janela
  uint16_t window_y;      // Posição Y da janela
  uint16_t window_width;  // Largura da janela em células
  uint16_t window_height; // Altura da janela em células
  uint8_t window_enabled; // Flag de habilitação da janela

  // Configurações gerais
  uint8_t hscroll_mode;  // Modo de scroll horizontal
  uint8_t vscroll_mode;  // Modo de scroll vertical
  uint16_t hscroll_base; // Endereço base da tabela de scroll horizontal
  uint16_t vscroll_base; // Endereço base da tabela de scroll vertical
} emu_vdp_scroll_state_t;

// Funções de inicialização e controle
void emu_vdp_scroll_init(void);
void emu_vdp_scroll_reset(void);

// Funções de configuração de planos
void emu_vdp_set_plane_a_base(uint16_t base);
void emu_vdp_set_plane_b_base(uint16_t base);
void emu_vdp_set_window_base(uint16_t base);

void emu_vdp_set_plane_a_size(uint16_t width, uint16_t height);
void emu_vdp_set_plane_b_size(uint16_t width, uint16_t height);
void emu_vdp_set_window_size(uint16_t width, uint16_t height);

void emu_vdp_set_window_position(uint16_t x, uint16_t y);

// Funções de scroll
void emu_vdp_set_plane_a_scroll(uint16_t x, uint16_t y);
void emu_vdp_set_plane_b_scroll(uint16_t x, uint16_t y);

void emu_vdp_set_hscroll_mode(uint8_t mode);
void emu_vdp_set_vscroll_mode(uint8_t mode);

void emu_vdp_set_hscroll_base(uint16_t base);
void emu_vdp_set_vscroll_base(uint16_t base);

// Funções de habilitação
void emu_vdp_set_plane_a_enable(uint8_t enable);
void emu_vdp_set_plane_b_enable(uint8_t enable);
void emu_vdp_set_window_enable(uint8_t enable);

// Funções de consulta
void emu_vdp_get_scroll_state(emu_vdp_scroll_state_t *state);

// Funções de renderização
void emu_vdp_render_plane_a(uint16_t line);
void emu_vdp_render_plane_b(uint16_t line);
void emu_vdp_render_window(uint16_t line);
void emu_vdp_render_line(uint16_t line);

#endif // EMU_VDP_SCROLL_H
