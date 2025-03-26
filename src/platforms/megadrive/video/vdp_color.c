#include "vdp_color.h"
#include "core/video/color.h"
#include "vdp_registers.h"
#include <stdlib.h>
#include <string.h>

// Tabela para rápida conversão de cores Mega Drive para RGB
static uint32_t cram_to_rgb_table[4096];
static uint32_t shadow_color_table[4096];
static uint32_t highlight_color_table[4096];
static bool tables_initialized = false;

// Flag global indicando se o modo Shadow/Highlight está ativo
static bool shadow_highlight_enabled = false;

// Bit masks para detecção de Shadow/Highlight
#define PRIORITY_BIT_MASK 0x8000
#define SHADOW_BIT_MASK 0x4000
#define HIGHLIGHT_BIT_MASK 0x2000
#define PALETTE_MASK 0x0030
#define COLOR_INDEX_MASK 0x000F

// Definições para formato de cores do Mega Drive
#define VDP_CRAM_COLOR_MASK 0x0FFF // Máscara para 12 bits (formato 0x0RGB)

/**
 * @brief Inicializa as tabelas de conversão de cores
 *
 * Pré-calcula todas as possíveis conversões de cores do formato CRAM do Mega
 * Drive para o formato RGB usado pelo emulador, incluindo transformações
 * shadow/highlight.
 */
void vdp_color_init(void) {
  if (tables_initialized) {
    return;
  }

  // Processo de inicialização: calcular todas as possíveis cores para acesso
  // rápido
  for (int i = 0; i < 4096; i++) {
    // Extração de componentes RGB (4 bits cada)
    uint8_t r = (i >> 8) & 0xF;
    uint8_t g = (i >> 4) & 0xF;
    uint8_t b = i & 0xF;

    // Conversão para 8 bits por canal (0-255)
    uint8_t r8 = (r << 4) | r;
    uint8_t g8 = (g << 4) | g;
    uint8_t b8 = (b << 4) | b;

    // Cor normal
    cram_to_rgb_table[i] = (r8 << 16) | (g8 << 8) | b8;

    // Cor em modo shadow (50% da intensidade)
    uint8_t rs = r8 >> 1;
    uint8_t gs = g8 >> 1;
    uint8_t bs = b8 >> 1;
    shadow_color_table[i] = (rs << 16) | (gs << 8) | bs;

    // Cor em modo highlight (125% da intensidade, limitado a 255)
    uint8_t rh = r8 + (r8 >> 2);
    uint8_t gh = g8 + (g8 >> 2);
    uint8_t bh = b8 + (b8 >> 2);

    if (rh > 255)
      rh = 255;
    if (gh > 255)
      gh = 255;
    if (bh > 255)
      bh = 255;

    highlight_color_table[i] = (rh << 16) | (gh << 8) | bh;
  }

  tables_initialized = true;
}

/**
 * @brief Atualiza o estado do modo Shadow/Highlight baseado no registro de modo
 * 4
 *
 * @param mode_register_4 Valor atual do registro de modo 4 do VDP
 */
void vdp_update_shadow_highlight_mode(uint8_t mode_register_4) {
  // Bit 3 do registro de modo 4 controla o modo Shadow/Highlight
  shadow_highlight_enabled =
      (mode_register_4 & VDP_REG4_SHADOW_HIGHLIGHT_ENABLE) != 0;
}

/**
 * @brief Verifica se o modo Shadow/Highlight está habilitado
 *
 * @return true se o modo está habilitado, false caso contrário
 */
bool vdp_is_shadow_highlight_enabled(void) { return shadow_highlight_enabled; }

/**
 * @brief Converte uma cor do formato CRAM para RGB, aplicando transformações
 * shadow/highlight se necessário
 *
 * @param cram_color A cor no formato de 12 bits do Mega Drive (0x0RGB)
 * @param color_attributes Atributos da cor (bits de prioridade, shadow,
 * highlight)
 * @return Cor no formato RGB de 24 bits (0xRRGGBB)
 */
uint32_t vdp_calculate_color(uint16_t cram_color, uint16_t color_attributes) {
  uint16_t color_index = cram_color & 0xFFF; // 12 bits de cor efetivos

  // Se o modo Shadow/Highlight não está habilitado, retorna a cor normal
  if (!shadow_highlight_enabled) {
    return cram_to_rgb_table[color_index];
  }

  // Determina o tipo de renderização baseado nos atributos
  if (color_attributes & SHADOW_BIT_MASK) {
    // Modo shadow (50% da intensidade)
    return shadow_color_table[color_index];
  } else if (color_attributes & HIGHLIGHT_BIT_MASK) {
    // Modo highlight (125% da intensidade)
    return highlight_color_table[color_index];
  } else {
    // Cor normal
    return cram_to_rgb_table[color_index];
  }
}

/**
 * @brief Cria um tipo de pixel com informações de prioridade e shadow/highlight
 *
 * @param color_index Índice da cor na CRAM
 * @param priority Flag de prioridade (0 ou 1)
 * @param palette Número da paleta a usar (0-3)
 * @param is_shadow Flag indicando se é um pixel em modo shadow
 * @param is_highlight Flag indicando se é um pixel em modo highlight
 * @return Valor de pixel com todos os metadados necessários para renderização
 */
uint16_t vdp_create_pixel_type(uint8_t color_index, uint8_t priority,
                               uint8_t palette, bool is_shadow,
                               bool is_highlight) {
  uint16_t pixel_type = color_index & COLOR_INDEX_MASK;

  // Adiciona informação de paleta - bits 4-5
  pixel_type |= (palette & 0x3) << 4;

  // Adiciona flags para shadow/highlight - usando bits 12-13
  if (is_shadow) {
    pixel_type |= SHADOW_BIT_MASK;
  } else if (is_highlight) {
    pixel_type |= HIGHLIGHT_BIT_MASK;
  }

  // Adiciona bit de prioridade - bit 15
  if (priority) {
    pixel_type |= PRIORITY_BIT_MASK;
  }

  return pixel_type;
}

/**
 * @brief Extrai o índice de cor de um pixel
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Índice de cor (0-15)
 */
uint8_t vdp_get_color_index(uint16_t pixel_type) {
  return pixel_type & COLOR_INDEX_MASK;
}

/**
 * @brief Verifica se um pixel tem prioridade
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se o pixel tem prioridade, false caso contrário
 */
bool vdp_has_priority(uint16_t pixel_type) {
  return (pixel_type & PRIORITY_BIT_MASK) != 0;
}

/**
 * @brief Verifica se um pixel está em modo shadow
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se o pixel está no modo shadow, false caso contrário
 */
bool vdp_is_shadow(uint16_t pixel_type) {
  return (pixel_type & SHADOW_BIT_MASK) != 0;
}

/**
 * @brief Verifica se um pixel está em modo highlight
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se o pixel está no modo highlight, false caso contrário
 */
bool vdp_is_highlight(uint16_t pixel_type) {
  return (pixel_type & HIGHLIGHT_BIT_MASK) != 0;
}

/**
 * @brief Obtém a paleta associada a um pixel
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Número da paleta (0-3)
 */
uint8_t vdp_get_palette(uint16_t pixel_type) {
  return (pixel_type & PALETTE_MASK) >> 4;
}

/**
 * @brief Converte um pixel completo para RGB
 *
 * @param pixel_type Valor de pixel com metadados
 * @param cram Ponteiro para a CRAM (Color RAM)
 * @return Cor no formato RGB de 24 bits (0xRRGGBB)
 */
uint32_t vdp_pixel_to_rgb(uint16_t pixel_type, const uint16_t *cram) {
  uint8_t color_index = vdp_get_color_index(pixel_type);
  uint8_t palette = vdp_get_palette(pixel_type);

  // Se o índice de cor é 0, é transparente/fundo
  if (color_index == 0) {
    // Índice 0 sempre vem da paleta 0 (background)
    uint16_t cram_color = cram[0];
    // Para cor de fundo, usamos regras especiais para shadow/highlight
    if (vdp_is_shadow(pixel_type)) {
      return shadow_color_table[cram_color & 0xFFF];
    } else if (vdp_is_highlight(pixel_type)) {
      return highlight_color_table[cram_color & 0xFFF];
    } else {
      return cram_to_rgb_table[cram_color & 0xFFF];
    }
  }

  // Para outros índices, usamos a paleta e o índice para obter a cor da CRAM
  uint16_t cram_color = cram[palette * 16 + color_index];

  // Aplicamos shadow/highlight conforme necessário
  return vdp_calculate_color(cram_color, pixel_type);
}

/**
 * @brief Adiciona o efeito shadow a um pixel existente
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Novo pixel com efeito shadow aplicado
 */
uint16_t vdp_apply_shadow_to_pixel(uint16_t pixel_type) {
  // Remove qualquer flag de highlight existente e aplica shadow
  return (pixel_type & ~HIGHLIGHT_BIT_MASK) | SHADOW_BIT_MASK;
}

/**
 * @brief Adiciona o efeito highlight a um pixel existente
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Novo pixel com efeito highlight aplicado
 */
uint16_t vdp_apply_highlight_to_pixel(uint16_t pixel_type) {
  // Remove qualquer flag de shadow existente e aplica highlight
  return (pixel_type & ~SHADOW_BIT_MASK) | HIGHLIGHT_BIT_MASK;
}

/**
 * @brief Remove os efeitos shadow/highlight de um pixel
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Novo pixel sem efeitos shadow/highlight
 */
uint16_t vdp_remove_shadow_highlight(uint16_t pixel_type) {
  // Remove flags de shadow e highlight
  return pixel_type & ~(SHADOW_BIT_MASK | HIGHLIGHT_BIT_MASK);
}

/**
 * @brief Verifica se o pixel é do tipo que gera o efeito shadow em outros
 * pixels
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se é um pixel que gera efeito shadow, false caso contrário
 */
bool vdp_is_shadow_effect_sprite(uint16_t pixel_type) {
    // Na paleta 3, os índices de cor 14 e 15 geram efeito shadow
    // mas não são renderizados (sprite de efeito especial)
    return vdp_get_palette(pixel_type) == 3 &&
           (vdp_get_color_index(pixel_type) == 14);
}

/**
 * @brief Verifica se o pixel é do tipo que gera o efeito highlight em outros
 * pixels
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se é um pixel que gera efeito highlight, false caso contrário
 */
bool vdp_is_highlight_effect_sprite(uint16_t pixel_type) {
    // Na paleta 3, o índice de cor 15 gera efeito highlight
    // mas não é renderizado (sprite de efeito especial)
    return vdp_get_palette(pixel_type) == 3 &&
           (vdp_get_color_index(pixel_type) == 15);
}

/**
 * @brief Verifica se o pixel é de um sprite que deve ser renderizado normalmente
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se é um pixel normal que deve ser renderizado, false caso contrário
 */
bool vdp_is_normal_sprite_pixel(uint16_t pixel_type) {
    uint8_t palette = vdp_get_palette(pixel_type);
    uint8_t color_index = vdp_get_color_index(pixel_type);

    // Apenas os índices 14 e 15 da paleta 3 são especiais
    // todos os outros são renderizados normalmente
    if (palette == 3 && (color_index == 14 || color_index == 15)) {
        return false;
    }

    return true;
}
