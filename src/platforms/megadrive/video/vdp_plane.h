/**
 * @file vdp_plane.h
 * @brief Definições para renderização de planos (A, B e Window) do VDP do Mega
 * Drive
 * @version 1.0
 * @date 2024-03-28
 */

#ifndef MEGA_EMU_VDP_PLANE_H
#define MEGA_EMU_VDP_PLANE_H

#include "vdp_types.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Renderiza plano A em uma linha
 *
 * Processa e renderiza uma linha do plano A, considerando scroll e prioridade.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha
 */
void vdp_render_plane_a_line(vdp_context_t *vdp_ctx, int line,
                             uint16_t *line_buffer);

/**
 * @brief Renderiza plano B em uma linha
 *
 * Processa e renderiza uma linha do plano B, considerando scroll e prioridade.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha
 */
void vdp_render_plane_b_line(vdp_context_t *vdp_ctx, int line,
                             uint16_t *line_buffer);

/**
 * @brief Renderiza janela em uma linha
 *
 * Processa e renderiza uma linha da janela, verificando se a linha está na área
 * da janela.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a processar
 * @param line_buffer Buffer de saída para a linha
 */
void vdp_render_window_line(vdp_context_t *vdp_ctx, int line,
                            uint16_t *line_buffer);

/**
 * @brief Verifica se um ponto está dentro da área da janela
 *
 * @param vdp_ctx Contexto do VDP
 * @param x Coordenada X
 * @param y Coordenada Y
 * @return true se o ponto está na janela, false caso contrário
 */
bool vdp_is_point_in_window(vdp_context_t *vdp_ctx, int x, int y);

#endif // MEGA_EMU_VDP_PLANE_H
