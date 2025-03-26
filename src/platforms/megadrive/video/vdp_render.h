#ifndef MEGA_EMU_VDP_RENDER_H
#define MEGA_EMU_VDP_RENDER_H

#include "vdp_types.h"

/**
 * @brief Inicializa o sistema de renderização
 *
 * Configura buffers e estados iniciais para renderização
 *
 * @param vdp_ctx Contexto do VDP
 */
void vdp_render_init(vdp_context_t* vdp_ctx);

/**
 * @brief Atualiza configurações de renderização baseado em registros
 *
 * @param vdp_ctx Contexto do VDP
 */
void vdp_render_update_config(vdp_context_t* vdp_ctx);

/**
 * @brief Processa um scanline completo para renderização
 *
 * Esta função orquestra a renderização de um scanline, incluindo sprites,
 * planos A e B, janela, e aplicando efeitos de shadow/highlight conforme
 * necessário.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha a ser processada
 */
void vdp_process_scanline(vdp_context_t* vdp_ctx, int line);

/**
 * @brief Renderiza sprites em uma linha
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 * @param line_buffer Buffer de saída para a linha de sprites
 */
void vdp_render_sprites_line(vdp_context_t* vdp_ctx, int line, uint16_t* line_buffer);

/**
 * @brief Renderiza plano A em uma linha
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 * @param line_buffer Buffer de saída para a linha do plano A
 */
void vdp_render_plane_a_line(vdp_context_t* vdp_ctx, int line, uint16_t* line_buffer);

/**
 * @brief Renderiza plano B em uma linha
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 * @param line_buffer Buffer de saída para a linha do plano B
 */
void vdp_render_plane_b_line(vdp_context_t* vdp_ctx, int line, uint16_t* line_buffer);

/**
 * @brief Renderiza janela em uma linha
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 * @param line_buffer Buffer de saída para a linha da janela
 */
void vdp_render_window_line(vdp_context_t* vdp_ctx, int line, uint16_t* line_buffer);

/**
 * @brief Combina todos os elementos de um scanline com regras de prioridade
 *
 * Esta função implementa as regras de prioridade do VDP, incluindo o suporte
 * para o modo Shadow/Highlight.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 */
void vdp_combine_line(vdp_context_t* vdp_ctx, int line);

/**
 * @brief Renderiza a linha final para o framebuffer
 *
 * Converte a linha processada para o formato final RGB e coloca no framebuffer.
 *
 * @param vdp_ctx Contexto do VDP
 * @param line Número da linha
 */
void vdp_render_line_to_framebuffer(vdp_context_t* vdp_ctx, int line);

/**
 * @brief Verifica se um ponto está dentro da área da janela
 *
 * @param vdp_ctx Contexto do VDP
 * @param x Coordenada X
 * @param y Coordenada Y
 * @return true se o ponto está na janela, false caso contrário
 */
bool vdp_is_point_in_window(vdp_context_t* vdp_ctx, int x, int y);

#endif // MEGA_EMU_VDP_RENDER_H
