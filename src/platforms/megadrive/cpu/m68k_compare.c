/**
 * @file m68k_compare.c
 * @brief Implementação das instruções de comparação do M68000
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
#define SR_X 0x0010 // Extend

/**
 * @brief Atualiza as flags para instruções de comparação
 */
static void update_flags_cmp(megadrive_m68k_context_t *ctx, uint32_t src,
                             uint32_t dst, uint32_t result, uint8_t size) {
  uint32_t mask = (size == 1) ? 0x80 : (size == 2) ? 0x8000 : 0x80000000;
  uint32_t smask = (size == 1) ? 0xFF : (size == 2) ? 0xFFFF : 0xFFFFFFFF;

  // Clear flags
  ctx->sr &= ~(SR_N | SR_Z | SR_V | SR_C);

  // Set Negative flag
  if (result & mask) {
    ctx->sr |= SR_N;
  }

  // Set Zero flag
  if ((result & smask) == 0) {
    ctx->sr |= SR_Z;
  }

  // Set Overflow flag
  if (((src ^ dst) & (result ^ dst) & mask) != 0) {
    ctx->sr |= SR_V;
  }

  // Set Carry flag
  if ((src > dst) || ((result & ~smask) != 0)) {
    ctx->sr |= SR_C;
  }
}

/**
 * @brief Implementa a instrução CMP (Compare)
 */
void m68k_execute_cmp(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t result = dst - src;

  // Atualiza flags
  update_flags_cmp(ctx, src, dst, result, inst->size);

  // CMP não modifica o destino
  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução CMPA (Compare Address)
 */
void m68k_execute_cmpa(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = ctx->address_registers[inst->dst_reg];

  // Para CMPA.W, extende o sinal para 32 bits
  if (inst->size == 2) {
    src = (int16_t)src;
  }

  uint32_t result = dst - src;

  // Atualiza flags (sempre usa tamanho long para CMPA)
  update_flags_cmp(ctx, src, dst, result, 4);

  ctx->cycles_left -= inst->cycles;
}

/**
 * @brief Implementa a instrução CMPM (Compare Memory)
 */
void m68k_execute_cmpm(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t src_addr = ctx->address_registers[inst->src_reg];
  uint32_t dst_addr = ctx->address_registers[inst->dst_reg];

  // Lê os valores da memória
  uint32_t src = read_long(ctx, src_addr);
  uint32_t dst = read_long(ctx, dst_addr);

  // Mascara os valores de acordo com o tamanho
  if (inst->size == 1) {
    src &= 0xFF;
    dst &= 0xFF;
  } else if (inst->size == 2) {
    src &= 0xFFFF;
    dst &= 0xFFFF;
  }

  uint32_t result = dst - src;

  // Atualiza flags
  update_flags_cmp(ctx, src, dst, result, inst->size);

  // Incrementa os registradores de endereço
  ctx->address_registers[inst->src_reg] += inst->size;
  ctx->address_registers[inst->dst_reg] += inst->size;

  ctx->cycles_left -= inst->cycles;
}

void m68k_execute_cmpi(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t result = dst - src;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_cmp(ctx, src, dst, result, inst->size);

  ctx->cycles_left -= inst->cycles;
}

void m68k_execute_tst(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  uint32_t sign_bit = (size == 1) ? 0x80 : (size == 2) ? 0x8000 : 0x80000000;

  src &= mask;

  // Clear flags
  ctx->sr &= ~(SR_N | SR_Z | SR_V | SR_C);

  // Set flags
  if (src == 0)
    ctx->sr |= SR_Z;
  if (src & sign_bit)
    ctx->sr |= SR_N;

  ctx->cycles_left -= inst->cycles;
}
