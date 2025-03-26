/**
 * @file vdp_memory.c
 * @brief Implementação das funções de acesso à memória do VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-28
 */

#include "vdp_memory.h"
#include "vdp_registers.h"
#include <string.h>

/**
 * @brief Lê um byte da VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @return Valor lido
 */
uint8_t vdp_read_vram_byte(vdp_context_t *vdp_ctx, uint32_t addr) {
  if (!vdp_ctx) {
    return 0;
  }

  addr &= 0xFFFF; // Máscara para 64K
  return vdp_ctx->vram[addr];
}

/**
 * @brief Lê uma palavra (16 bits) da VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @return Valor lido
 */
uint16_t vdp_read_vram_word(vdp_context_t *vdp_ctx, uint32_t addr) {
  if (!vdp_ctx) {
    return 0;
  }

  addr &= 0xFFFE; // Alinha para endereço par

  // Lê dois bytes consecutivos e combina em uma palavra de 16 bits
  uint16_t value = (vdp_ctx->vram[addr] << 8) | vdp_ctx->vram[addr + 1];
  return value;
}

/**
 * @brief Escreve um byte na VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @param value Valor a escrever
 */
void vdp_write_vram_byte(vdp_context_t *vdp_ctx, uint32_t addr, uint8_t value) {
  if (!vdp_ctx) {
    return;
  }

  addr &= 0xFFFF; // Máscara para 64K
  vdp_ctx->vram[addr] = value;
}

/**
 * @brief Escreve uma palavra (16 bits) na VRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VRAM (0x0000-0xFFFF)
 * @param value Valor a escrever
 */
void vdp_write_vram_word(vdp_context_t *vdp_ctx, uint32_t addr,
                         uint16_t value) {
  if (!vdp_ctx) {
    return;
  }

  addr &= 0xFFFE; // Alinha para endereço par

  // Escreve a palavra separada em dois bytes
  vdp_ctx->vram[addr] = (value >> 8) & 0xFF;
  vdp_ctx->vram[addr + 1] = value & 0xFF;
}

/**
 * @brief Lê um valor da CRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na CRAM (0x00-0x3F)
 * @return Valor lido
 */
uint16_t vdp_read_cram(vdp_context_t *vdp_ctx, uint16_t addr) {
  if (!vdp_ctx) {
    return 0;
  }

  addr &= 0x3F; // Limita ao tamanho da CRAM (64 entradas)
  return vdp_ctx->cram[addr] & VDP_CRAM_COLOR_MASK;
}

/**
 * @brief Escreve um valor na CRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na CRAM (0x00-0x3F)
 * @param value Valor a escrever
 */
void vdp_write_cram(vdp_context_t *vdp_ctx, uint16_t addr, uint16_t value) {
  if (!vdp_ctx) {
    return;
  }

  addr &= 0x3F; // Limita ao tamanho da CRAM (64 entradas)

  // Apenas os 12 bits menos significativos são usados (formato 0x0RGB)
  vdp_ctx->cram[addr] = value & VDP_CRAM_COLOR_MASK;
}

/**
 * @brief Lê um valor da VSRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VSRAM (0x00-0x3F)
 * @return Valor lido
 */
uint16_t vdp_read_vsram(vdp_context_t *vdp_ctx, uint16_t addr) {
  if (!vdp_ctx) {
    return 0;
  }

  addr &= 0x3F; // Limita ao tamanho da VSRAM (64 entradas)
  return vdp_ctx->vsram[addr] & 0x03FF; // Apenas 10 bits são usados
}

/**
 * @brief Escreve um valor na VSRAM
 *
 * @param vdp_ctx Contexto do VDP
 * @param addr Endereço na VSRAM (0x00-0x3F)
 * @param value Valor a escrever
 */
void vdp_write_vsram(vdp_context_t *vdp_ctx, uint16_t addr, uint16_t value) {
  if (!vdp_ctx) {
    return;
  }

  addr &= 0x3F; // Limita ao tamanho da VSRAM (64 entradas)

  // Apenas 10 bits são usados para scroll
  vdp_ctx->vsram[addr] = value & 0x03FF;
}

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
                      uint16_t length, uint8_t increment) {
  if (!vdp_ctx || length == 0) {
    return;
  }

  // Para uma implementação real, aqui seria necessário ler da memória externa
  // Nesta versão simplificada, assumimos que temos acesso direto à memória
  // externa

  // Marca DMA como ativo
  vdp_ctx->dma_active = true;

  // Atualiza parâmetros de DMA
  vdp_ctx->dma_source = source;
  vdp_ctx->dma_length = length;

  // Em uma implementação completa, precisaríamos de um callback para ler da
  // memória externa Para fins de ilustração, preenchemos a VRAM com valores de
  // teste
  for (uint16_t i = 0; i < length; i++) {
    // Em uma implementação real, usaríamos algo como:
    // uint8_t value = memory_read_callback(source + i);
    uint8_t value = (source + i) & 0xFF; // Valor de teste

    vdp_write_vram_byte(vdp_ctx, dest, value);
    dest += increment;
  }

  // Marca DMA como inativo
  vdp_ctx->dma_active = false;
}

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
                           uint8_t mode) {
  if (!vdp_ctx || length == 0) {
    return;
  }

  // Marca DMA como ativo
  vdp_ctx->dma_active = true;

  // Modo fill
  if (mode == 1) {
    // Preenche a região com um valor constante
    for (uint16_t i = 0; i < length; i++) {
      vdp_write_vram_byte(vdp_ctx, dest + i, fill_data & 0xFF);
    }
  }
  // Modo copy
  else {
    // Copia dados de uma região para outra
    // Cuidado com sobreposição de regiões
    if (dest > source) {
      // Copia de trás para frente se as regiões se sobrepõem e destino > fonte
      for (int16_t i = length - 1; i >= 0; i--) {
        uint8_t value = vdp_read_vram_byte(vdp_ctx, source + i);
        vdp_write_vram_byte(vdp_ctx, dest + i, value);
      }
    } else {
      // Copia normal se não há sobreposição ou destino < fonte
      for (uint16_t i = 0; i < length; i++) {
        uint8_t value = vdp_read_vram_byte(vdp_ctx, source + i);
        vdp_write_vram_byte(vdp_ctx, dest + i, value);
      }
    }
  }

  // Marca DMA como inativo
  vdp_ctx->dma_active = false;
}
