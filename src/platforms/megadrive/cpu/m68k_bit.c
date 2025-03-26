/**
 * @file m68k_bit.c
 * @brief Implementação das instruções de manipulação de bits do M68000
 * @version 1.0
 * @date 2024-03-21
 */

#include "m68k_adapter.h"
#include "m68k_execute.h"
#include <string.h>

// Flags do Status Register
#define SR_N 0x8000 // Negative
#define SR_Z 0x4000 // Zero

/**
 * @brief Obtém o número do bit a ser manipulado
 */
static uint8_t get_bit_number(megadrive_m68k_context_t *ctx,
                              m68k_instruction_t *inst) {
  // Se o bit número vem de um registrador de dados, usa os 5 bits menos
  // significativos
  if (inst->src_mode == 0) {
    return inst->src_value & 0x1F;
  }
  // Se é imediato, usa os 7 bits menos significativos para byte, ou 31 bits
  // para long
  else {
    return inst->src_value & ((inst->size == 1) ? 0x07 : 0x1F);
  }
}

/**
 * @brief Implementa a instrução BTST (Bit Test)
 */
void m68k_execute_btst(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t data;
  uint8_t bit_num = get_bit_number(ctx, inst);

  // Lê o dado do destino
  if (inst->dst_mode == 0) { // Registrador
    data = ctx->data_registers[inst->dst_reg];
  } else { // Memória
    data = read_long(ctx, inst->dst_addr);
  }

  // Testa o bit e atualiza a flag Z
  ctx->sr &= ~SR_Z;
  if (!(data & (1 << bit_num))) {
    ctx->sr |= SR_Z;
  }

  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução BCHG (Bit Change)
 */
void m68k_execute_bchg(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t data;
  uint8_t bit_num = get_bit_number(ctx, inst);

  // Lê o dado do destino
  if (inst->dst_mode == 0) { // Registrador
    data = ctx->data_registers[inst->dst_reg];
  } else { // Memória
    data = read_long(ctx, inst->dst_addr);
  }

  // Testa o bit e atualiza a flag Z
  ctx->sr &= ~SR_Z;
  if (!(data & (1 << bit_num))) {
    ctx->sr |= SR_Z;
  }

  // Inverte o bit
  data ^= (1 << bit_num);

  // Escreve o resultado
  if (inst->dst_mode == 0) { // Registrador
    ctx->data_registers[inst->dst_reg] = data;
  } else { // Memória
    write_value(ctx, inst->dst_addr, data, inst->size);
  }

  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução BCLR (Bit Clear)
 */
void m68k_execute_bclr(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t data;
  uint8_t bit_num = get_bit_number(ctx, inst);

  // Lê o dado do destino
  if (inst->dst_mode == 0) { // Registrador
    data = ctx->data_registers[inst->dst_reg];
  } else { // Memória
    data = read_long(ctx, inst->dst_addr);
  }

  // Testa o bit e atualiza a flag Z
  ctx->sr &= ~SR_Z;
  if (!(data & (1 << bit_num))) {
    ctx->sr |= SR_Z;
  }

  // Limpa o bit
  data &= ~(1 << bit_num);

  // Escreve o resultado
  if (inst->dst_mode == 0) { // Registrador
    ctx->data_registers[inst->dst_reg] = data;
  } else { // Memória
    write_value(ctx, inst->dst_addr, data, inst->size);
  }

  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução BSET (Bit Set)
 */
void m68k_execute_bset(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t data;
  uint8_t bit_num = get_bit_number(ctx, inst);

  // Lê o dado do destino
  if (inst->dst_mode == 0) { // Registrador
    data = ctx->data_registers[inst->dst_reg];
  } else { // Memória
    data = read_long(ctx, inst->dst_addr);
  }

  // Testa o bit e atualiza a flag Z
  ctx->sr &= ~SR_Z;
  if (!(data & (1 << bit_num))) {
    ctx->sr |= SR_Z;
  }

  // Seta o bit
  data |= (1 << bit_num);

  // Escreve o resultado
  if (inst->dst_mode == 0) { // Registrador
    ctx->data_registers[inst->dst_reg] = data;
  } else { // Memória
    write_value(ctx, inst->dst_addr, data, inst->size);
  }

  ctx->cycles_left -= inst->cycles;
}
