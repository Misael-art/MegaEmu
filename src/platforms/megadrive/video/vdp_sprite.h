/**
 * @file vdp_sprite.h
 * @brief Definições para renderização de sprites do VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-28
 */

#ifndef MEGA_EMU_VDP_SPRITE_H
#define MEGA_EMU_VDP_SPRITE_H

#include "vdp_types.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Renderiza sprites em uma linha
 *
 * Esta função processa os sprites visíveis em uma linha específica e os
 * renderiza no buffer de saída, considerando prioridades e efeitos de
 * Shadow/Highlight.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha de sprites
 */
void vdp_render_sprites_line(vdp_context_t *vdp_ctx, int line,
                             uint16_t *line_buffer);

/**
 * @brief Verifica se houve colisão de sprites
 *
 * @param vdp_ctx Contexto do VDP
 * @return true se houve colisão, false caso contrário
 */
bool vdp_sprite_collision_occurred(vdp_context_t *vdp_ctx);

/**
 * @brief Verifica se houve overflow de sprites
 *
 * @param vdp_ctx Contexto do VDP
 * @return true se houve overflow, false caso contrário
 */
bool vdp_sprite_overflow_occurred(vdp_context_t *vdp_ctx);

/**
 * @brief Reseta flags de sprite
 *
 * @param vdp_ctx Contexto do VDP
 */
void vdp_sprite_reset_flags(vdp_context_t *vdp_ctx);

/**
 * @brief Obtém o número de sprites processados na última linha
 *
 * @param vdp_ctx Contexto do VDP
 * @return Número de sprites
 */
int vdp_sprite_get_count(vdp_context_t *vdp_ctx);

#endif // MEGA_EMU_VDP_SPRITE_H
