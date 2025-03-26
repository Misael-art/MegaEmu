/**
 * @file gg_vdp.h
 * @brief Interface para o VDP do Game Gear
 */

#ifndef GG_VDP_H
#define GG_VDP_H

#include "../../../core/save_state.h"
#include "../../mastersystem/video/sms_vdp.h"
#include <stdbool.h>
#include <stdint.h>

// Dimensões da tela do Game Gear
#define GG_SCREEN_WIDTH 160
#define GG_SCREEN_HEIGHT 144

// Deslocamento da tela do Game Gear no buffer do SMS
#define GG_SCREEN_X_OFFSET 48
#define GG_SCREEN_Y_OFFSET 24

// Dimensões do buffer do VDP
#define GG_VDP_BUFFER_WIDTH 256
#define GG_VDP_BUFFER_HEIGHT 192

// Cores e paletas
#define GG_TOTAL_COLORS 32
#define GG_COLOR_MASK 0x0FFF  // Máscara para 12 bits (4096 cores)

// Componentes de cor RGB444
#define GG_COLOR_R_MASK 0x0F00
#define GG_COLOR_G_MASK 0x00F0
#define GG_COLOR_B_MASK 0x000F

#define GG_COLOR_R_SHIFT 8
#define GG_COLOR_G_SHIFT 4
#define GG_COLOR_B_SHIFT 0

// Efeitos visuais
#define GG_EFFECT_NONE 0x00
#define GG_EFFECT_SHADOW 0x01  // Sombra (diminui brilho)
#define GG_EFFECT_HIGHLIGHT 0x02  // Highlight (aumenta brilho)
#define GG_EFFECT_GRADIENT 0x04  // Efeito gradiente vertical
#define GG_EFFECT_SCANLINES 0x08  // Linhas de escaneamento visíveis

/**
 * @brief Obtém a interface de extensão do VDP do Game Gear para o VDP do SMS
 *
 * @return Interface de extensão
 */
const sms_vdp_ext_t *gg_vdp_get_extension(void);

/**
 * @brief Converte uma cor do formato Game Gear (RGB444) para RGB565
 *
 * @param color Cor no formato do Game Gear (12 bits)
 * @return Cor no formato RGB565 (16 bits)
 */
static inline uint16_t gg_color_to_rgb565(uint16_t color) {
    // Extrai componentes RGB444
    uint8_t r = (color & GG_COLOR_R_MASK) >> GG_COLOR_R_SHIFT;
    uint8_t g = (color & GG_COLOR_G_MASK) >> GG_COLOR_G_SHIFT;
    uint8_t b = (color & GG_COLOR_B_MASK) >> GG_COLOR_B_SHIFT;

    // Expande para RGB565
    // R: 0-15 -> 0-31 (aproximadamente)
    // G: 0-15 -> 0-63 (aproximadamente)
    // B: 0-15 -> 0-31 (aproximadamente)
    uint16_t r565 = (r << 1) | (r >> 3);
    uint16_t g565 = (g << 2) | (g >> 2);
    uint16_t b565 = (b << 1) | (b >> 3);

    return (r565 << 11) | (g565 << 5) | b565;
}

/**
 * @brief Converte uma cor do formato RGB565 para Game Gear (RGB444)
 *
 * @param rgb565 Cor no formato RGB565 (16 bits)
 * @return Cor no formato do Game Gear (12 bits)
 */
static inline uint16_t rgb565_to_gg_color(uint16_t rgb565) {
    // Extrai componentes RGB565
    uint8_t r = (rgb565 >> 11) & 0x1F;
    uint8_t g = (rgb565 >> 5) & 0x3F;
    uint8_t b = rgb565 & 0x1F;

    // Reduz para RGB444
    // R: 0-31 -> 0-15
    // G: 0-63 -> 0-15
    // B: 0-31 -> 0-15
    uint8_t r444 = r >> 1;
    uint8_t g444 = g >> 2;
    uint8_t b444 = b >> 1;

    return (r444 << GG_COLOR_R_SHIFT) |
           (g444 << GG_COLOR_G_SHIFT) |
           (b444 << GG_COLOR_B_SHIFT);
}

/**
 * @brief Aplica um efeito visual à cor
 *
 * @param color Cor no formato do Game Gear (12 bits)
 * @param effect Tipo de efeito (GG_EFFECT_*)
 * @param param Parâmetro adicional para o efeito, se necessário
 * @return Cor modificada pelo efeito
 */
static inline uint16_t gg_apply_effect(uint16_t color, uint8_t effect, uint8_t param) {
    // Extrai componentes
    uint8_t r = (color & GG_COLOR_R_MASK) >> GG_COLOR_R_SHIFT;
    uint8_t g = (color & GG_COLOR_G_MASK) >> GG_COLOR_G_SHIFT;
    uint8_t b = (color & GG_COLOR_B_MASK) >> GG_COLOR_B_SHIFT;

    // Aplica efeitos
    if (effect & GG_EFFECT_SHADOW) {
        // Escurece a cor (diminui brilho)
        r = r >> 1;
        g = g >> 1;
        b = b >> 1;
    } else if (effect & GG_EFFECT_HIGHLIGHT) {
        // Clareia a cor (aumenta brilho)
        r = r + ((15 - r) >> 1);
        g = g + ((15 - g) >> 1);
        b = b + ((15 - b) >> 1);
    }

    // Constrói a cor modificada
    return (r << GG_COLOR_R_SHIFT) |
           (g << GG_COLOR_G_SHIFT) |
           (b << GG_COLOR_B_SHIFT);
}

/**
 * @brief Interpola entre duas cores do Game Gear
 *
 * @param color1 Primeira cor (12 bits)
 * @param color2 Segunda cor (12 bits)
 * @param factor Fator de interpolação (0-255, onde 0 = color1 e 255 = color2)
 * @return Cor interpolada
 */
static inline uint16_t gg_color_blend(uint16_t color1, uint16_t color2, uint8_t factor) {
    // Extrai componentes da primeira cor
    uint8_t r1 = (color1 & GG_COLOR_R_MASK) >> GG_COLOR_R_SHIFT;
    uint8_t g1 = (color1 & GG_COLOR_G_MASK) >> GG_COLOR_G_SHIFT;
    uint8_t b1 = (color1 & GG_COLOR_B_MASK) >> GG_COLOR_B_SHIFT;

    // Extrai componentes da segunda cor
    uint8_t r2 = (color2 & GG_COLOR_R_MASK) >> GG_COLOR_R_SHIFT;
    uint8_t g2 = (color2 & GG_COLOR_G_MASK) >> GG_COLOR_G_SHIFT;
    uint8_t b2 = (color2 & GG_COLOR_B_MASK) >> GG_COLOR_B_SHIFT;

    // Interpola componentes
    uint8_t inv_factor = 255 - factor;
    uint8_t r = (r1 * inv_factor + r2 * factor) / 255;
    uint8_t g = (g1 * inv_factor + g2 * factor) / 255;
    uint8_t b = (b1 * inv_factor + b2 * factor) / 255;

    // Constrói a cor interpolada
    return (r << GG_COLOR_R_SHIFT) |
           (g << GG_COLOR_G_SHIFT) |
           (b << GG_COLOR_B_SHIFT);
}

/**
 * @brief Exporta funções disponíveis para o VDP do Game Gear
 */
const sms_vdp_ext_t *gg_vdp_get_extension(void);

#endif /* GG_VDP_H */
