/**
 * @file vdp_memory.h
 * @brief Definições para acesso à memória do VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-28
 */

#ifndef MEGA_EMU_VDP_MEMORY_H
#define MEGA_EMU_VDP_MEMORY_H

#include "vdp_types.h"
#include <stdint.h>

/**
 * @brief Lê um byte da VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @return Valor lido
 */
uint8_t vdp_read_vram_byte(vdp_context_t *vdp_ctx, uint32_t addr);

/**
 * @brief Lê uma palavra (16 bits) da VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @return Valor lido
 */
uint16_t vdp_read_vram_word(vdp_context_t *vdp_ctx, uint32_t addr);

/**
 * @brief Escreve um byte na VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @param value Valor a escrever
 */
void vdp_write_vram_byte(vdp_context_t *vdp_ctx, uint32_t addr, uint8_t value);

/**
 * @brief Escreve uma palavra (16 bits) na VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @param value Valor a escrever
 */
void vdp_write_vram_word(vdp_context_t *vdp_ctx, uint32_t addr, uint16_t value);

/**
 * @brief Lê um valor da CRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na CRAM (0x00-0x3F)
 * @return Valor lido
 */
uint16_t vdp_read_cram(vdp_context_t *vdp_ctx, uint16_t addr);

/**
 * @brief Escreve um valor na CRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na CRAM (0x00-0x3F)
 * @param value Valor a escrever
 */
void vdp_write_cram(vdp_context_t *vdp_ctx, uint16_t addr, uint16_t value);

/**
 * @brief Lê um valor da VSRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VSRAM (0x00-0x3F)
 * @return Valor lido
 */
uint16_t vdp_read_vsram(vdp_context_t *vdp_ctx, uint16_t addr);

/**
 * @brief Escreve um valor na VSRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VSRAM (0x00-0x3F)
 * @param value Valor a escrever
 */
void vdp_write_vsram(vdp_context_t *vdp_ctx, uint16_t addr, uint16_t value);

/**
 * @brief Transfere dados via DMA de uma fonte para a VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param source Endereço fonte (na RAM ou ROM)
 * @param dest Endereço destino na VRAM
 * @param length Comprimento da transferência em bytes
 * @param increment Valor de incremento do endereço
 */
void vdp_dma_transfer(vdp_context_t *vdp_ctx, uint32_t source, uint16_t dest,
                      uint16_t length, uint8_t increment);

/**
 * @brief Transfere dados via DMA de VRAM para VRAM (fill/copy)
 *
 * @param vdp_ctx Contexto do VDP
 * @param source Endereço fonte na VRAM (usado apenas para copy)
 * @param dest Endereço destino na VRAM
 * @param length Comprimento da transferência em bytes
 * @param fill_data Dado para preenchimento (usado apenas para fill)
 * @param mode Modo: 0 para copy, 1 para fill
 */
void vdp_dma_vram_transfer(vdp_context_t *vdp_ctx, uint16_t source,
                           uint16_t dest, uint16_t length, uint16_t fill_data,
                           uint8_t mode);

#endif // MEGA_EMU_VDP_MEMORY_H
