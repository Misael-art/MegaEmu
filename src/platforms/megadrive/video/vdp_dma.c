/**
 * @file vdp_dma.c
 * @brief Implementação das funções de DMA do VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "vdp_adapter.h"
#include <stdlib.h>
#include <string.h>

// Constantes do DMA
#define DMA_FILL_MODE 0x80
#define DMA_COPY_MODE 0xC0

// Tipos de DMA
typedef enum {
  DMA_TYPE_VRAM_FILL,
  DMA_TYPE_VRAM_COPY,
  DMA_TYPE_MEMORY_TO_VRAM,
  DMA_TYPE_MEMORY_TO_CRAM,
  DMA_TYPE_MEMORY_TO_VSRAM
} dma_type_t;

// Estrutura para configuração do DMA
typedef struct {
  dma_type_t type;
  uint32_t source;
  uint16_t dest;
  uint16_t length;
  uint8_t fill_data;
} dma_config_t;

// Funções auxiliares estáticas
static void decode_dma_config(megadrive_vdp_context_t *ctx,
                              dma_config_t *config) {
  // Obtém tipo de DMA e endereço de destino
  uint8_t dma_mode = ctx->code & 0xC0;
  uint32_t dest_addr = ((uint32_t)ctx->code << 14) | ctx->address;
  config->dest = dest_addr & 0xFFFF;

  // Configura tipo de DMA
  if (dma_mode == DMA_FILL_MODE) {
    config->type = DMA_TYPE_VRAM_FILL;
    config->fill_data = ctx->dma_source & 0xFF;
  } else if (dma_mode == DMA_COPY_MODE) {
    config->type = DMA_TYPE_VRAM_COPY;
    config->source = ctx->dma_source & 0xFFFF;
  } else {
    // DMA da memória
    config->source = ctx->dma_source;
    switch ((dest_addr >> 16) & 0x7) {
    case 1:
      config->type = DMA_TYPE_MEMORY_TO_VRAM;
      break;
    case 3:
      config->type = DMA_TYPE_MEMORY_TO_CRAM;
      break;
    case 5:
      config->type = DMA_TYPE_MEMORY_TO_VSRAM;
      break;
    default:
      return; // Tipo inválido
    }
  }

  // Configura tamanho da transferência
  config->length = ctx->dma_length;
}

static void dma_fill_vram(megadrive_vdp_context_t *ctx,
                          const dma_config_t *config) {
  uint16_t dest = config->dest & (MD_VDP_VRAM_SIZE - 1);
  uint8_t data = config->fill_data;
  uint16_t length = config->length;

  // Preenche VRAM com o valor especificado
  while (length-- > 0) {
    ctx->vram[dest++] = data;
    dest &= (MD_VDP_VRAM_SIZE - 1);
  }
}

static void dma_copy_vram(megadrive_vdp_context_t *ctx,
                          const dma_config_t *config) {
  uint16_t src = config->source & (MD_VDP_VRAM_SIZE - 1);
  uint16_t dest = config->dest & (MD_VDP_VRAM_SIZE - 1);
  uint16_t length = config->length;

  // Copia dados dentro da VRAM
  while (length-- > 0) {
    ctx->vram[dest++] = ctx->vram[src++];
    src &= (MD_VDP_VRAM_SIZE - 1);
    dest &= (MD_VDP_VRAM_SIZE - 1);
  }
}

static void dma_memory_to_vram(megadrive_vdp_context_t *ctx,
                               const dma_config_t *config) {
  uint32_t src = config->source;
  uint16_t dest = config->dest & (MD_VDP_VRAM_SIZE - 1);
  uint16_t length = config->length;

  // Transfere dados da memória para VRAM
  while (length-- > 0) {
    if (ctx->memory_read_callback) {
      ctx->vram[dest++] = ctx->memory_read_callback(src++, ctx->callback_data);
    } else {
      break; // Sem callback, não pode ler memória
    }
    dest &= (MD_VDP_VRAM_SIZE - 1);
  }
}

static void dma_memory_to_cram(megadrive_vdp_context_t *ctx,
                               const dma_config_t *config) {
  uint32_t src = config->source;
  uint16_t dest = config->dest & (MD_VDP_CRAM_SIZE - 1);
  uint16_t length = config->length;

  // Transfere dados da memória para CRAM
  while (length-- > 0) {
    if (ctx->memory_read_callback) {
      ctx->cram[dest++] = ctx->memory_read_callback(src++, ctx->callback_data);
    } else {
      break; // Sem callback, não pode ler memória
    }
    dest &= (MD_VDP_CRAM_SIZE - 1);
  }
}

static void dma_memory_to_vsram(megadrive_vdp_context_t *ctx,
                                const dma_config_t *config) {
  uint32_t src = config->source;
  uint16_t dest = config->dest & (MD_VDP_VSRAM_SIZE - 1);
  uint16_t length = config->length;

  // Transfere dados da memória para VSRAM
  while (length-- > 0) {
    if (ctx->memory_read_callback) {
      ctx->vsram[dest++] = ctx->memory_read_callback(src++, ctx->callback_data);
    } else {
      break; // Sem callback, não pode ler memória
    }
    dest &= (MD_VDP_VSRAM_SIZE - 1);
  }
}

// Funções públicas
void vdp_dma_execute(megadrive_vdp_context_t *ctx) {
  if (!ctx || !ctx->dma_enabled || ctx->dma_length == 0)
    return;

  // Decodifica configuração do DMA
  dma_config_t config;
  decode_dma_config(ctx, &config);

  // Executa DMA de acordo com o tipo
  switch (config.type) {
  case DMA_TYPE_VRAM_FILL:
    dma_fill_vram(ctx, &config);
    break;

  case DMA_TYPE_VRAM_COPY:
    dma_copy_vram(ctx, &config);
    break;

  case DMA_TYPE_MEMORY_TO_VRAM:
    dma_memory_to_vram(ctx, &config);
    break;

  case DMA_TYPE_MEMORY_TO_CRAM:
    dma_memory_to_cram(ctx, &config);
    break;

  case DMA_TYPE_MEMORY_TO_VSRAM:
    dma_memory_to_vsram(ctx, &config);
    break;
  }

  // Finaliza DMA
  ctx->dma_enabled = false;
  ctx->dma_length = 0;
}

void vdp_dma_start(megadrive_vdp_context_t *ctx, uint8_t code, uint16_t address,
                   uint32_t source, uint16_t length) {
  if (!ctx)
    return;

  // Configura DMA
  ctx->code = code;
  ctx->address = address;
  ctx->dma_source = source;
  ctx->dma_length = length;
  ctx->dma_enabled = true;
}

bool vdp_dma_is_active(const megadrive_vdp_context_t *ctx) {
  return ctx && ctx->dma_enabled && ctx->dma_length > 0;
}

void vdp_dma_set_memory_callback(megadrive_vdp_context_t *ctx,
                                 uint8_t (*read_cb)(uint32_t addr,
                                                    void *user_data),
                                 void *user_data) {
  if (!ctx)
    return;

  ctx->memory_read_callback = read_cb;
  ctx->callback_data = user_data;
}
