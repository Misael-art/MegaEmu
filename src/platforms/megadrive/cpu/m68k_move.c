/**
 * @file m68k_move.c
 * @brief Implementação das instruções de movimentação do M68000
 * @version 1.0
 * @date 2024-03-21
 */

#include "m68k_adapter.h"
#include "m68k_execute.h"
#include <string.h>

// Flags do Status Register
#define SR_N 0x8000 // Negative
#define SR_Z 0x4000 // Zero
#define SR_V 0x2000 // Overflow
#define SR_C 0x1000 // Carry

/**
 * @brief Atualiza as flags N e Z para instruções MOVE
 */
static void update_flags_move(megadrive_m68k_context_t *ctx, uint32_t result,
                              uint8_t size) {
  uint32_t mask = (size == 1) ? 0x80 : (size == 2) ? 0x8000 : 0x80000000;

  // Clear flags
  ctx->sr &= ~(SR_N | SR_Z | SR_V | SR_C);

  // Set Negative flag
  if (result & mask) {
    ctx->sr |= SR_N;
  }

  // Set Zero flag
  if (result == 0) {
    ctx->sr |= SR_Z;
  }
}

/**
 * @brief Implementa a instrução MOVE
 */
void m68k_execute_move(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t result = inst->src_value;

  // Escreve o resultado no destino
  write_value(ctx, inst->dst_addr, result, inst->size);

  // Atualiza flags
  update_flags_move(ctx, result, inst->size);

  // Atualiza ciclos
  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução MOVEA (Move Address)
 */
void m68k_execute_movea(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst) {
  uint32_t result = inst->src_value;

  // Para MOVEA.W, extende o sinal para 32 bits
  if (inst->size == 2) {
    result = (int16_t)result;
  }

  // Escreve no registrador de endereço
  ctx->address_registers[inst->dst_reg] = result;

  // MOVEA não afeta flags
  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução MOVEQ (Move Quick)
 */
void m68k_execute_moveq(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst) {
  // MOVEQ sempre move um valor de 8 bits com sinal extendido para D0-D7
  int32_t result = (int8_t)inst->src_value;

  ctx->data_registers[inst->dst_reg] = result;

  // Atualiza flags
  update_flags_move(ctx, result, 4);

  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução MOVEM (Move Multiple Registers)
 */
void m68k_execute_movem(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst) {
  uint16_t mask = inst->src_value;
  uint32_t addr = inst->dst_addr;
  int i;

  // Registrador para memória
  if (inst->src_mode == 0) {
    // Data registers D0-D7
    for (i = 0; i < 8; i++) {
      if (mask & (1 << i)) {
        write_value(ctx, addr, ctx->data_registers[i], inst->size);
        addr += inst->size;
      }
    }
    // Address registers A0-A7
    for (i = 0; i < 8; i++) {
      if (mask & (1 << (i + 8))) {
        write_value(ctx, addr, ctx->address_registers[i], inst->size);
        addr += inst->size;
      }
    }
  }
  // Memória para registrador
  else {
    // Data registers D0-D7
    for (i = 0; i < 8; i++) {
      if (mask & (1 << i)) {
        if (inst->size == 2) {
          ctx->data_registers[i] = (int16_t)read_long(ctx, addr);
        } else {
          ctx->data_registers[i] = read_long(ctx, addr);
        }
        addr += inst->size;
      }
    }
    // Address registers A0-A7
    for (i = 0; i < 8; i++) {
      if (mask & (1 << (i + 8))) {
        if (inst->size == 2) {
          ctx->address_registers[i] = (int16_t)read_long(ctx, addr);
        } else {
          ctx->address_registers[i] = read_long(ctx, addr);
        }
        addr += inst->size;
      }
    }
  }

  // MOVEM não afeta flags
  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução LEA (Load Effective Address)
 */
void m68k_execute_lea(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // LEA carrega o endereço efetivo no registrador de endereço
  ctx->address_registers[inst->dst_reg] = inst->src_addr;

  // LEA não afeta flags
  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução PEA (Push Effective Address)
 */
void m68k_execute_pea(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Decrementa SP
  ctx->address_registers[7] -= 4;

  // Escreve o endereço efetivo na pilha
  write_long(ctx, ctx->address_registers[7], inst->src_addr);

  // PEA não afeta flags
  ctx->cycles_left -= inst->cycles;
}
