/**
 * @file vdp_scroll.c
 * @brief Implementação do sistema de scrolling do VDP do Mega Drive
 */

#include "vdp_scroll.h"
#include "utils/log_utils.h"
#include "vdp.h"
#include <string.h>

// Estado global do sistema de scrolling
static emu_vdp_scroll_state_t g_scroll_state;

// Cache de scroll por célula para otimização
static struct {
  uint16_t hscroll_values[41]; // 320/8 + 1 células horizontais
  uint16_t vscroll_values[21]; // 320/16 + 1 células verticais
  uint8_t hscroll_dirty;       // Flag para atualização do cache horizontal
  uint8_t vscroll_dirty;       // Flag para atualização do cache vertical
} g_scroll_cache;

// Cache de tiles para otimização
#define TILE_CACHE_SIZE 256
static struct {
  uint16_t tile_data[TILE_CACHE_SIZE]; // Dados dos tiles
  uint16_t tile_addr[TILE_CACHE_SIZE]; // Endereços dos tiles na VRAM
  uint8_t tile_valid[TILE_CACHE_SIZE]; // Flags de validade
  uint8_t next_entry;                  // Próxima entrada a ser substituída
} g_tile_cache;

// Funções de cache de tiles
static void invalidate_tile_cache(void) {
  memset(g_tile_cache.tile_valid, 0, TILE_CACHE_SIZE);
  g_tile_cache.next_entry = 0;
}

static uint16_t get_cached_tile(uint16_t addr) {
  // Procura o tile no cache
  for (int i = 0; i < TILE_CACHE_SIZE; i++) {
    if (g_tile_cache.tile_valid[i] && g_tile_cache.tile_addr[i] == addr) {
      return g_tile_cache.tile_data[i];
    }
  }

  // Tile não encontrado no cache, lê da VRAM
  uint16_t data = vdp_read_vram(addr);

  // Armazena no cache
  uint8_t entry = g_tile_cache.next_entry;
  g_tile_cache.tile_data[entry] = data;
  g_tile_cache.tile_addr[entry] = addr;
  g_tile_cache.tile_valid[entry] = 1;

  // Atualiza próxima entrada usando política circular
  g_tile_cache.next_entry = (entry + 1) % TILE_CACHE_SIZE;

  return data;
}

// Funções auxiliares internas otimizadas
static uint16_t get_tile_from_plane(uint16_t plane_base, uint16_t x, uint16_t y,
                                    uint16_t width) {
  uint32_t offset = (y * width + x) * 2;
  return get_cached_tile(plane_base + offset);
}

// Buffer de linha para otimização de renderização
static uint8_t
    g_line_buffer[320 * 4]; // 4 bytes por pixel para suportar composição

static void clear_line_buffer(void) {
  memset(g_line_buffer, 0, sizeof(g_line_buffer));
}

static void compose_line_buffer(uint16_t line) {
  // Transfere o buffer de linha para o framebuffer
  vdp_write_line_buffer(line, g_line_buffer);
}

static void render_tile_to_buffer(uint16_t pattern_index, int16_t x,
                                  uint8_t palette, uint8_t priority,
                                  uint8_t flip_h, uint8_t flip_v,
                                  uint16_t line_in_tile) {
  // Obtém dados do padrão do tile
  uint8_t *pattern_data = vdp_get_pattern_data(pattern_index);

  // Ajusta linha do tile para flip vertical
  if (flip_v) {
    line_in_tile = 7 - line_in_tile;
  }

  // Obtém linha do padrão
  uint8_t *line_data = &pattern_data[line_in_tile * 8];

  // Calcula posição no buffer
  uint8_t *dest = &g_line_buffer[x * 4];

  // Renderiza pixels do tile
  for (int i = 0; i < 8; i++) {
    uint8_t pixel = line_data[flip_h ? 7 - i : i];
    if (pixel) {                  // Pixel não transparente
      dest[i * 4 + 0] = pixel;    // Índice de cor
      dest[i * 4 + 1] = palette;  // Paleta
      dest[i * 4 + 2] = priority; // Prioridade
      dest[i * 4 + 3] = 1;        // Flag de pixel válido
    }
  }
}

static void render_tile(uint16_t tile_data, int16_t x, int16_t y,
                        uint8_t priority) {
  uint16_t pattern_index = tile_data & 0x07FF;
  uint8_t palette = (tile_data >> 13) & 0x03;
  uint8_t flip_h = (tile_data >> 11) & 0x01;
  uint8_t flip_v = (tile_data >> 12) & 0x01;

  // Calcula linha dentro do tile
  uint16_t line_in_tile = y & 7;

  render_tile_to_buffer(pattern_index, x, palette, priority, flip_h, flip_v,
                        line_in_tile);
}

// Funções de cache de scroll
static void update_hscroll_cache(void) {
  if (!g_scroll_cache.hscroll_dirty) {
    return;
  }

  // Atualiza cache de scroll horizontal
  for (int i = 0; i < 41; i++) {
    g_scroll_cache.hscroll_values[i] =
        vdp_read_vram(g_scroll_state.hscroll_base + i * 4);
  }

  g_scroll_cache.hscroll_dirty = 0;
}

static void update_vscroll_cache(void) {
  if (!g_scroll_cache.vscroll_dirty) {
    return;
  }

  // Atualiza cache de scroll vertical
  for (int i = 0; i < 21; i++) {
    g_scroll_cache.vscroll_values[i] = vdp_read_vsram(i * 2);
  }

  g_scroll_cache.vscroll_dirty = 0;
}

// Funções de scroll por célula otimizadas
static uint16_t get_cell_scroll_x(uint16_t cell) {
  if (g_scroll_cache.hscroll_dirty) {
    update_hscroll_cache();
  }
  return g_scroll_cache.hscroll_values[cell];
}

static uint16_t get_cell_scroll_y(uint16_t cell) {
  if (g_scroll_cache.vscroll_dirty) {
    update_vscroll_cache();
  }
  return g_scroll_cache.vscroll_values[cell];
}

// Funções de renderização com suporte a scroll por célula
static void render_plane_cell(uint16_t plane_base, uint16_t x, uint16_t y,
                              uint16_t scroll_x, uint16_t scroll_y,
                              uint16_t width, uint16_t height,
                              uint8_t priority) {
  // Calcula posição na tela com scroll
  int16_t screen_x = (x + scroll_x) % (width * 8);
  int16_t screen_y = (y + scroll_y) % (height * 8);

  // Calcula índices dos tiles
  uint16_t tile_x = screen_x / 8;
  uint16_t tile_y = screen_y / 8;

  // Obtém e renderiza o tile
  uint16_t tile_data = get_tile_from_plane(plane_base, tile_x, tile_y, width);
  render_tile(tile_data, x, y, priority);
}

// Funções de renderização
void emu_vdp_render_plane_a(uint16_t line) {
  if (!g_scroll_state.plane_a_enabled) {
    return;
  }

  uint16_t scroll_x = g_scroll_state.plane_a_scroll_x;
  uint16_t scroll_y = g_scroll_state.plane_a_scroll_y;

  // Processa scroll baseado no modo
  switch (g_scroll_state.hscroll_mode) {
  case VDP_HSCROLL_MODE_FULL:
    // Scroll de tela inteira - já temos os valores corretos
    break;

  case VDP_HSCROLL_MODE_CELL:
    // Renderiza cada célula com seu próprio scroll
    for (int16_t x = 0; x < 320; x += 8) {
      uint16_t cell = x / 8;
      scroll_x = get_cell_scroll_x(cell);

      render_plane_cell(g_scroll_state.plane_a_base, x, line, scroll_x,
                        scroll_y, g_scroll_state.plane_a_width,
                        g_scroll_state.plane_a_height, 0);
    }
    return;

  case VDP_HSCROLL_MODE_LINE:
    // Scroll por linha
    scroll_x = vdp_read_vram(g_scroll_state.hscroll_base + line * 4);
    break;
  }

  // Processa scroll vertical
  if (g_scroll_state.vscroll_mode == VDP_VSCROLL_MODE_2CELL) {
    // Renderiza cada coluna de 2 células com seu próprio scroll
    for (int16_t x = 0; x < 320; x += 16) {
      uint16_t cell = x / 16;
      scroll_y = get_cell_scroll_y(cell);

      // Renderiza as duas células da coluna
      render_plane_cell(g_scroll_state.plane_a_base, x, line, scroll_x,
                        scroll_y, g_scroll_state.plane_a_width,
                        g_scroll_state.plane_a_height, 0);

      render_plane_cell(g_scroll_state.plane_a_base, x + 8, line, scroll_x,
                        scroll_y, g_scroll_state.plane_a_width,
                        g_scroll_state.plane_a_height, 0);
    }
    return;
  }

  // Renderização padrão para scroll de tela inteira
  int16_t screen_y = (line + scroll_y) % (g_scroll_state.plane_a_height * 8);
  uint16_t tile_y = screen_y / 8;

  for (int16_t x = 0; x < 320; x += 8) {
    int16_t screen_x = (x + scroll_x) % (g_scroll_state.plane_a_width * 8);
    uint16_t tile_x = screen_x / 8;

    uint16_t tile_data =
        get_tile_from_plane(g_scroll_state.plane_a_base, tile_x, tile_y,
                            g_scroll_state.plane_a_width);

    render_tile(tile_data, x, line, 0);
  }
}

void emu_vdp_render_plane_b(uint16_t line) {
  if (!g_scroll_state.plane_b_enabled) {
    return;
  }

  uint16_t scroll_x = g_scroll_state.plane_b_scroll_x;
  uint16_t scroll_y = g_scroll_state.plane_b_scroll_y;

  // Processa scroll baseado no modo
  switch (g_scroll_state.hscroll_mode) {
  case VDP_HSCROLL_MODE_FULL:
    // Scroll de tela inteira - já temos os valores corretos
    break;

  case VDP_HSCROLL_MODE_CELL:
    // Renderiza cada célula com seu próprio scroll
    for (int16_t x = 0; x < 320; x += 8) {
      uint16_t cell = x / 8;
      scroll_x = get_cell_scroll_x(cell);

      render_plane_cell(g_scroll_state.plane_b_base, x, line, scroll_x,
                        scroll_y, g_scroll_state.plane_b_width,
                        g_scroll_state.plane_b_height, 1);
    }
    return;

  case VDP_HSCROLL_MODE_LINE:
    // Scroll por linha
    scroll_x = vdp_read_vram(g_scroll_state.hscroll_base + line * 4 + 2);
    break;
  }

  // Processa scroll vertical
  if (g_scroll_state.vscroll_mode == VDP_VSCROLL_MODE_2CELL) {
    // Renderiza cada coluna de 2 células com seu próprio scroll
    for (int16_t x = 0; x < 320; x += 16) {
      uint16_t cell = x / 16;
      scroll_y = get_cell_scroll_y(cell);

      // Renderiza as duas células da coluna
      render_plane_cell(g_scroll_state.plane_b_base, x, line, scroll_x,
                        scroll_y, g_scroll_state.plane_b_width,
                        g_scroll_state.plane_b_height, 1);

      render_plane_cell(g_scroll_state.plane_b_base, x + 8, line, scroll_x,
                        scroll_y, g_scroll_state.plane_b_width,
                        g_scroll_state.plane_b_height, 1);
    }
    return;
  }

  // Renderização padrão para scroll de tela inteira
  int16_t screen_y = (line + scroll_y) % (g_scroll_state.plane_b_height * 8);
  uint16_t tile_y = screen_y / 8;

  for (int16_t x = 0; x < 320; x += 8) {
    int16_t screen_x = (x + scroll_x) % (g_scroll_state.plane_b_width * 8);
    uint16_t tile_x = screen_x / 8;

    uint16_t tile_data =
        get_tile_from_plane(g_scroll_state.plane_b_base, tile_x, tile_y,
                            g_scroll_state.plane_b_width);

    render_tile(tile_data, x, line, 1);
  }
}

void emu_vdp_render_window(uint16_t line) {
  if (!g_scroll_state.window_enabled) {
    return;
  }

  // Verifica se a linha está dentro da área da janela
  if (line < g_scroll_state.window_y ||
      line >= g_scroll_state.window_y + g_scroll_state.window_height * 8) {
    return;
  }

  uint16_t tile_y = (line - g_scroll_state.window_y) / 8;

  // Renderiza tiles visíveis na linha
  for (uint16_t x = g_scroll_state.window_x;
       x < g_scroll_state.window_x + g_scroll_state.window_width * 8; x += 8) {
    uint16_t tile_x = (x - g_scroll_state.window_x) / 8;

    uint16_t tile_data =
        get_tile_from_plane(g_scroll_state.window_base, tile_x, tile_y,
                            g_scroll_state.window_width);

    render_tile(tile_data, x, line, 2);
  }
}

// Função principal de renderização por linha
void emu_vdp_render_line(uint16_t line) {
  // Limpa buffer de linha
  clear_line_buffer();

  // Renderiza os planos na ordem correta (B -> A -> Window)
  emu_vdp_render_plane_b(line);
  emu_vdp_render_plane_a(line);
  emu_vdp_render_window(line);

  // Compõe buffer final
  compose_line_buffer(line);
}

/**
 * @brief Inicializa o sistema de scrolling
 */
void emu_vdp_scroll_init(void) {
  // Inicializa o estado com valores padrão
  memset(&g_scroll_state, 0, sizeof(emu_vdp_scroll_state_t));
  memset(&g_scroll_cache, 0, sizeof(g_scroll_cache));

  // Inicializa cache de tiles
  invalidate_tile_cache();

  // Configura valores iniciais
  g_scroll_state.plane_a_width = 32;  // 32 células (256 pixels)
  g_scroll_state.plane_a_height = 32; // 32 células (256 pixels)
  g_scroll_state.plane_b_width = 32;
  g_scroll_state.plane_b_height = 32;
  g_scroll_state.window_width = 32;
  g_scroll_state.window_height = 32;

  // Habilita planos por padrão
  g_scroll_state.plane_a_enabled = 1;
  g_scroll_state.plane_b_enabled = 1;
  g_scroll_state.window_enabled = 0;

  // Configura modos de scroll padrão
  g_scroll_state.hscroll_mode = VDP_HSCROLL_MODE_FULL;
  g_scroll_state.vscroll_mode = VDP_VSCROLL_MODE_FULL;

  // Marca caches como sujos para forçar atualização
  g_scroll_cache.hscroll_dirty = 1;
  g_scroll_cache.vscroll_dirty = 1;

  LOG_INFO("Sistema de scrolling inicializado");
}

/**
 * @brief Reseta o estado do scrolling
 */
void emu_vdp_scroll_reset(void) {
  // Reseta para os valores iniciais
  emu_vdp_scroll_init();

  LOG_INFO("Estado do scrolling resetado");
}

/**
 * @brief Define o endereço base do plano A
 */
void emu_vdp_set_plane_a_base(uint16_t base) {
  g_scroll_state.plane_a_base = base;
  LOG_DEBUG("Base do plano A definida: 0x%04X", base);
}

/**
 * @brief Define o endereço base do plano B
 */
void emu_vdp_set_plane_b_base(uint16_t base) {
  g_scroll_state.plane_b_base = base;
  LOG_DEBUG("Base do plano B definida: 0x%04X", base);
}

/**
 * @brief Define o endereço base do plano de janela
 */
void emu_vdp_set_window_base(uint16_t base) {
  g_scroll_state.window_base = base;
  LOG_DEBUG("Base da janela definida: 0x%04X", base);
}

/**
 * @brief Define as dimensões do plano A
 */
void emu_vdp_set_plane_a_size(uint16_t width, uint16_t height) {
  if (width > 128 || height > 128) {
    LOG_ERROR("Dimensões inválidas para plano A: %dx%d", width, height);
    return;
  }

  g_scroll_state.plane_a_width = width;
  g_scroll_state.plane_a_height = height;
  LOG_DEBUG("Dimensões do plano A definidas: %dx%d", width, height);
}

/**
 * @brief Define as dimensões do plano B
 */
void emu_vdp_set_plane_b_size(uint16_t width, uint16_t height) {
  if (width > 128 || height > 128) {
    LOG_ERROR("Dimensões inválidas para plano B: %dx%d", width, height);
    return;
  }

  g_scroll_state.plane_b_width = width;
  g_scroll_state.plane_b_height = height;
  LOG_DEBUG("Dimensões do plano B definidas: %dx%d", width, height);
}

/**
 * @brief Define as dimensões da janela
 */
void emu_vdp_set_window_size(uint16_t width, uint16_t height) {
  if (width > 64 || height > 64) {
    LOG_ERROR("Dimensões inválidas para janela: %dx%d", width, height);
    return;
  }

  g_scroll_state.window_width = width;
  g_scroll_state.window_height = height;
  LOG_DEBUG("Dimensões da janela definidas: %dx%d", width, height);
}

/**
 * @brief Define a posição da janela
 */
void emu_vdp_set_window_position(uint16_t x, uint16_t y) {
  g_scroll_state.window_x = x;
  g_scroll_state.window_y = y;
  LOG_DEBUG("Posição da janela definida: (%d,%d)", x, y);
}

/**
 * @brief Define o scroll do plano A
 */
void emu_vdp_set_plane_a_scroll(uint16_t x, uint16_t y) {
  g_scroll_state.plane_a_scroll_x = x;
  g_scroll_state.plane_a_scroll_y = y;
  LOG_DEBUG("Scroll do plano A definido: (%d,%d)", x, y);
}

/**
 * @brief Define o scroll do plano B
 */
void emu_vdp_set_plane_b_scroll(uint16_t x, uint16_t y) {
  g_scroll_state.plane_b_scroll_x = x;
  g_scroll_state.plane_b_scroll_y = y;
  LOG_DEBUG("Scroll do plano B definido: (%d,%d)", x, y);
}

/**
 * @brief Define o modo de scroll horizontal
 */
void emu_vdp_set_hscroll_mode(uint8_t mode) {
  if (mode > VDP_HSCROLL_MODE_LINE) {
    LOG_ERROR("Modo de scroll horizontal inválido: %d", mode);
    return;
  }

  g_scroll_state.hscroll_mode = mode;
  g_scroll_cache.hscroll_dirty = 1;
  LOG_DEBUG("Modo de scroll horizontal definido: %d", mode);
}

/**
 * @brief Define o modo de scroll vertical
 */
void emu_vdp_set_vscroll_mode(uint8_t mode) {
  if (mode > VDP_VSCROLL_MODE_2CELL) {
    LOG_ERROR("Modo de scroll vertical inválido: %d", mode);
    return;
  }

  g_scroll_state.vscroll_mode = mode;
  g_scroll_cache.vscroll_dirty = 1;
  LOG_DEBUG("Modo de scroll vertical definido: %d", mode);
}

/**
 * @brief Define o endereço base da tabela de scroll horizontal
 */
void emu_vdp_set_hscroll_base(uint16_t base) {
  g_scroll_state.hscroll_base = base;
  g_scroll_cache.hscroll_dirty = 1;
  LOG_DEBUG("Base da tabela de scroll horizontal definida: 0x%04X", base);
}

/**
 * @brief Define o endereço base da tabela de scroll vertical
 */
void emu_vdp_set_vscroll_base(uint16_t base) {
  g_scroll_state.vscroll_base = base;
  g_scroll_cache.vscroll_dirty = 1;
  LOG_DEBUG("Base da tabela de scroll vertical definida: 0x%04X", base);
}

/**
 * @brief Habilita/desabilita o plano A
 */
void emu_vdp_set_plane_a_enable(uint8_t enable) {
  g_scroll_state.plane_a_enabled = enable ? 1 : 0;
  LOG_DEBUG("Plano A %s", enable ? "habilitado" : "desabilitado");
}

/**
 * @brief Habilita/desabilita o plano B
 */
void emu_vdp_set_plane_b_enable(uint8_t enable) {
  g_scroll_state.plane_b_enabled = enable ? 1 : 0;
  LOG_DEBUG("Plano B %s", enable ? "habilitado" : "desabilitado");
}

/**
 * @brief Habilita/desabilita a janela
 */
void emu_vdp_set_window_enable(uint8_t enable) {
  g_scroll_state.window_enabled = enable ? 1 : 0;
  LOG_DEBUG("Janela %s", enable ? "habilitada" : "desabilitada");
}

/**
 * @brief Obtém o estado atual do scrolling
 */
void emu_vdp_get_scroll_state(emu_vdp_scroll_state_t *state) {
  if (state) {
    memcpy(state, &g_scroll_state, sizeof(emu_vdp_scroll_state_t));
  }
}
