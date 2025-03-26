#ifndef MEGA_EMU_VDP_COLOR_H
#define MEGA_EMU_VDP_COLOR_H

#include <stdbool.h>
#include <stdint.h>

// Constantes para as máscaras de bits do formato de pixel
#define PRIORITY_BIT_MASK 0x8000
#define SHADOW_BIT_MASK 0x4000
#define HIGHLIGHT_BIT_MASK 0x2000
#define PALETTE_MASK 0x0030
#define COLOR_INDEX_MASK 0x000F

// Máscara para 12 bits de cor na CRAM (formato 0x0RGB)
#define VDP_CRAM_COLOR_MASK 0x0FFF

/**
 * @brief Inicializa as tabelas de conversão de cores
 */
void vdp_color_init(void);

/**
 * @brief Atualiza o estado do modo Shadow/Highlight baseado no registro de modo
 * 4
 *
 * @param mode_register_4 Valor atual do registro de modo 4 do VDP
 */
void vdp_update_shadow_highlight_mode(uint8_t mode_register_4);

/**
 * @brief Verifica se o modo Shadow/Highlight está habilitado
 *
 * @return true se o modo está habilitado, false caso contrário
 */
bool vdp_is_shadow_highlight_enabled(void);

/**
 * @brief Converte uma cor do formato CRAM para RGB, aplicando transformações
 * shadow/highlight se necessário
 *
 * @param cram_color A cor no formato de 12 bits do Mega Drive (0x0RGB)
 * @param color_attributes Atributos da cor (bits de prioridade, shadow,
 * highlight)
 * @return Cor no formato RGB de 24 bits (0xRRGGBB)
 */
uint32_t vdp_calculate_color(uint16_t cram_color, uint16_t color_attributes);

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
                               bool is_highlight);

/**
 * @brief Extrai o índice de cor de um pixel
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Índice de cor (0-15)
 */
uint8_t vdp_get_color_index(uint16_t pixel_type);

/**
 * @brief Verifica se um pixel tem prioridade
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se o pixel tem prioridade, false caso contrário
 */
bool vdp_has_priority(uint16_t pixel_type);

/**
 * @brief Verifica se um pixel está em modo shadow
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se o pixel está no modo shadow, false caso contrário
 */
bool vdp_is_shadow(uint16_t pixel_type);

/**
 * @brief Verifica se um pixel está em modo highlight
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se o pixel está no modo highlight, false caso contrário
 */
bool vdp_is_highlight(uint16_t pixel_type);

/**
 * @brief Obtém a paleta associada a um pixel
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Número da paleta (0-3)
 */
uint8_t vdp_get_palette(uint16_t pixel_type);

/**
 * @brief Converte um pixel completo para RGB
 *
 * @param pixel_type Valor de pixel com metadados
 * @param cram Ponteiro para a CRAM (Color RAM)
 * @return Cor no formato RGB de 24 bits (0xRRGGBB)
 */
uint32_t vdp_pixel_to_rgb(uint16_t pixel_type, const uint16_t *cram);

/**
 * @brief Adiciona o efeito shadow a um pixel existente
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Novo pixel com efeito shadow aplicado
 */
uint16_t vdp_apply_shadow_to_pixel(uint16_t pixel_type);

/**
 * @brief Adiciona o efeito highlight a um pixel existente
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Novo pixel com efeito highlight aplicado
 */
uint16_t vdp_apply_highlight_to_pixel(uint16_t pixel_type);

/**
 * @brief Remove os efeitos shadow/highlight de um pixel
 *
 * @param pixel_type Valor de pixel com metadados
 * @return Novo pixel sem efeitos shadow/highlight
 */
uint16_t vdp_remove_shadow_highlight(uint16_t pixel_type);

/**
 * @brief Verifica se o pixel é do tipo que gera o efeito shadow em outros
 * pixels
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se é um pixel que gera efeito shadow, false caso contrário
 */
bool vdp_is_shadow_effect_sprite(uint16_t pixel_type);

/**
 * @brief Verifica se o pixel é do tipo que gera o efeito highlight em outros
 * pixels
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se é um pixel que gera efeito highlight, false caso contrário
 */
bool vdp_is_highlight_effect_sprite(uint16_t pixel_type);

/**
 * @brief Verifica se o pixel é de um sprite que deve ser renderizado normalmente
 *
 * @param pixel_type Valor de pixel com metadados
 * @return true se é um pixel normal que deve ser renderizado, false caso contrário
 */
bool vdp_is_normal_sprite_pixel(uint16_t pixel_type);

#endif // MEGA_EMU_VDP_COLOR_H
