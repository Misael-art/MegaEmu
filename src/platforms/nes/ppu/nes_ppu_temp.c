#include <stdint.h>#include <stdbool.h>#include "nes_ppu.h"#include "nes_ppu_internal.h"#include "../../../utils/enhanced_log.h"// Paleta de cores do NESstatic const uint32_t NES_PALETTE[64] = {    // ... cores da paleta ...};/** * @brief Aplica a máscara de cores aos pixels do frame * * @param ppu Ponteiro para a PPU */void apply_color_mask(nes_ppu_t *ppu){    if (!ppu)    {        LOG_ERROR("[PPU] PPU é nulo");        return;    }    // Verifica se há alguma ênfase de cor ativa    if (!(ppu->reg_mask & (NES_PPUMASK_EMPHASIZE_RED | NES_PPUMASK_EMPHASIZE_GREEN | NES_PPUMASK_EMPHASIZE_BLUE)))    {        return;    }    LOG_DEBUG("[PPU] Aplicando máscara de cores: reg_mask=0x%02X", ppu->reg_mask);    // Itera sobre todas as cores da paleta    for (int i = 0; i < 64; i++)    {        uint32_t color = NES_PALETTE[i];        // Extrai componentes ARGB        uint8_t alpha = (color >> 24) & 0xFF;        uint8_t red = (color >> 16) & 0xFF;        uint8_t green = (color >> 8) & 0xFF;        uint8_t blue = color & 0xFF;        // Aplica ênfase de cores        if (ppu->reg_mask & NES_PPUMASK_EMPHASIZE_RED)        {            red = (uint8_t)(red * 1.1f);            green = (uint8_t)(green * 0.9f);            blue = (uint8_t)(blue * 0.9f);        }        if (ppu->reg_mask & NES_PPUMASK_EMPHASIZE_GREEN)        {            red = (uint8_t)(red * 0.9f);            green = (uint8_t)(green * 1.1f);            blue = (uint8_t)(blue * 0.9f);        }        if (ppu->reg_mask & NES_PPUMASK_EMPHASIZE_BLUE)        {            red = (uint8_t)(red * 0.9f);            green = (uint8_t)(green * 0.9f);            blue = (uint8_t)(blue * 1.1f);        }        // Recompõe a cor com os novos valores        ppu->palette[i] = (alpha << 24) | (red << 16) | (green << 8) | blue;    }    LOG_DEBUG("[PPU] Máscara de cores aplicada com sucesso");}