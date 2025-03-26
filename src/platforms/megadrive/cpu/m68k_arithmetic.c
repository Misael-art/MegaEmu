/**
 * @file m68k_arithmetic.c
 * @brief Implementação das instruções aritméticas do M68000
 * @version 1.0
 * @date 2024-03-21
 */

#include "m68k_adapter.h"
#include <string.h>

// Flags do Status Register
#define SR_EXTEND 0x0010
#define SR_NEGATIVE 0x0008
#define SR_ZERO 0x0004
#define SR_OVERFLOW 0x0002
#define SR_CARRY 0x0001

// Funções auxiliares para atualização de flags
static void update_flags_add(megadrive_m68k_context_t *ctx, uint32_t src,
                             uint32_t dst, uint32_t result, uint8_t size) {
  uint32_t mask = (size == 1) ? 0xFF : (size == 2) ? 0xFFFF : 0xFFFFFFFF;
  uint32_t sign_bit = (size == 1) ? 0x80 : (size == 2) ? 0x8000 : 0x80000000;

  // Clear flags
  ctx->sr &= ~(SR_EXTEND | SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Set flags
  if ((result & mask) == 0)
    ctx->sr |= SR_ZERO;
  if (result & sign_bit)
    ctx->sr |= SR_NEGATIVE;

  // Carry e Extend
  if (((src & mask) + (dst & mask)) > mask) {
    ctx->sr |= SR_CARRY | SR_EXTEND;
  }

  // Overflow
  if (((src & sign_bit) == (dst & sign_bit)) &&
      ((result & sign_bit) != (src & sign_bit))) {
    ctx->sr |= SR_OVERFLOW;
  }
}

static void update_flags_sub(megadrive_m68k_context_t *ctx, uint32_t src,
                             uint32_t dst, uint32_t result, uint8_t size) {
  uint32_t mask = (size == 1) ? 0xFF : (size == 2) ? 0xFFFF : 0xFFFFFFFF;
  uint32_t sign_bit = (size == 1) ? 0x80 : (size == 2) ? 0x8000 : 0x80000000;

  // Clear flags
  ctx->sr &= ~(SR_EXTEND | SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Set flags
  if ((result & mask) == 0)
    ctx->sr |= SR_ZERO;
  if (result & sign_bit)
    ctx->sr |= SR_NEGATIVE;

  // Carry e Extend
  if ((src & mask) > (dst & mask)) {
    ctx->sr |= SR_CARRY | SR_EXTEND;
  }

  // Overflow
  if (((src & sign_bit) != (dst & sign_bit)) &&
      ((result & sign_bit) == (src & sign_bit))) {
    ctx->sr |= SR_OVERFLOW;
  }
}

// Implementação das instruções aritméticas
void m68k_execute_add(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t result = src + dst;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_add(ctx, src, dst, result, inst->size);

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    // Escrita em memória
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_addq(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t src = (inst->opcode >> 9) & 7;
  if (src == 0)
    src = 8; // Quick data é 1-8

  uint32_t dst = inst->dst_value;
  uint32_t result = src + dst;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags (exceto para An)
  if (inst->dst_mode != M68K_ADDR_MODE_ADDR_REG) {
    update_flags_add(ctx, src, dst, result, inst->size);
  }

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG ||
      inst->dst_mode == M68K_ADDR_MODE_ADDR_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_addx(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t x = (ctx->sr & SR_EXTEND) ? 1 : 0;
  uint32_t result = src + dst + x;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags (Z é cleared somente se resultado != 0)
  uint16_t old_sr = ctx->sr;
  update_flags_add(ctx, src, dst + x, result, inst->size);
  if (result != 0)
    ctx->sr &= ~SR_ZERO;
  else if (old_sr & SR_ZERO)
    ctx->sr |= SR_ZERO;

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_sub(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t result = dst - src;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_sub(ctx, src, dst, result, inst->size);

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_subq(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t src = (inst->opcode >> 9) & 7;
  if (src == 0)
    src = 8; // Quick data é 1-8

  uint32_t dst = inst->dst_value;
  uint32_t result = dst - src;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags (exceto para An)
  if (inst->dst_mode != M68K_ADDR_MODE_ADDR_REG) {
    update_flags_sub(ctx, src, dst, result, inst->size);
  }

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG ||
      inst->dst_mode == M68K_ADDR_MODE_ADDR_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_subx(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t x = (ctx->sr & SR_EXTEND) ? 1 : 0;
  uint32_t result = dst - src - x;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags (Z é cleared somente se resultado != 0)
  uint16_t old_sr = ctx->sr;
  update_flags_sub(ctx, src + x, dst, result, inst->size);
  if (result != 0)
    ctx->sr &= ~SR_ZERO;
  else if (old_sr & SR_ZERO)
    ctx->sr |= SR_ZERO;

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

// Funções de multiplicação e divisão
void m68k_execute_mulu(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint16_t src = inst->src_value & 0xFFFF;
  uint16_t dst = inst->dst_value & 0xFFFF;
  uint32_t result = (uint32_t)src * (uint32_t)dst;

  // Atualiza flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);
  if (result == 0)
    ctx->sr |= SR_ZERO;
  if (result & 0x80000000)
    ctx->sr |= SR_NEGATIVE;

  // Armazena resultado
  ctx->registers[inst->dst_reg] = result;

  // 38-70 ciclos, dependendo dos dados
  ctx->cycles += 38 + count_ones(src) * 2;
}

void m68k_execute_muls(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  int16_t src = (int16_t)(inst->src_value & 0xFFFF);
  int16_t dst = (int16_t)(inst->dst_value & 0xFFFF);
  int32_t result = (int32_t)src * (int32_t)dst;

  // Atualiza flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);
  if (result == 0)
    ctx->sr |= SR_ZERO;
  if (result < 0)
    ctx->sr |= SR_NEGATIVE;

  // Armazena resultado
  ctx->registers[inst->dst_reg] = (uint32_t)result;

  // 38-70 ciclos, dependendo dos dados
  ctx->cycles += 38 + count_ones(abs(src)) * 2;
}

void m68k_execute_divu(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint32_t dividend = inst->dst_value;
  uint16_t divisor = inst->src_value & 0xFFFF;

  // Verifica divisão por zero
  if (divisor == 0) {
    // Gera trap de divisão por zero
    m68k_trigger_interrupt(ctx, 5);
    return;
  }

  uint32_t quotient = dividend / divisor;
  uint16_t remainder = dividend % divisor;

  // Verifica overflow
  if (quotient > 0xFFFF) {
    ctx->sr |= SR_OVERFLOW;
    return;
  }

  // Atualiza flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);
  if (quotient == 0)
    ctx->sr |= SR_ZERO;
  if (quotient & 0x8000)
    ctx->sr |= SR_NEGATIVE;

  // Armazena resultado (quotient:remainder)
  ctx->registers[inst->dst_reg] = (quotient << 16) | remainder;

  // 76-140 ciclos
  ctx->cycles += 76 + count_leading_zeros(dividend) * 2;
}

void m68k_execute_divs(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  int32_t dividend = (int32_t)inst->dst_value;
  int16_t divisor = (int16_t)(inst->src_value & 0xFFFF);

  // Verifica divisão por zero
  if (divisor == 0) {
    // Gera trap de divisão por zero
    m68k_trigger_interrupt(ctx, 5);
    return;
  }

  int32_t quotient = dividend / divisor;
  int16_t remainder = dividend % divisor;

  // Verifica overflow
  if (quotient < -32768 || quotient > 32767) {
    ctx->sr |= SR_OVERFLOW;
    return;
  }

  // Atualiza flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);
  if (quotient == 0)
    ctx->sr |= SR_ZERO;
  if (quotient < 0)
    ctx->sr |= SR_NEGATIVE;

  // Armazena resultado (quotient:remainder)
  ctx->registers[inst->dst_reg] =
      ((quotient & 0xFFFF) << 16) | (remainder & 0xFFFF);

  // 76-156 ciclos
  ctx->cycles += 76 + count_leading_zeros(abs(dividend)) * 2;
}

// Funções auxiliares
static int count_ones(uint32_t value) {
  int count = 0;
  while (value) {
    count += value & 1;
    value >>= 1;
  }
  return count;
}

static int count_leading_zeros(uint32_t value) {
  int count = 0;
  if (value == 0)
    return 32;
  while (!(value & 0x80000000)) {
    count++;
    value <<= 1;
  }
  return count;
}
