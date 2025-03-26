/**
 * @file m68k_logical.c
 * @brief Implementação das instruções lógicas do M68000
 * @version 1.0
 * @date 2024-03-21
 */

#include "m68k_adapter.h"
#include <string.h>

// Flags do Status Register
#define SR_NEGATIVE 0x0008
#define SR_ZERO 0x0004
#define SR_OVERFLOW 0x0002
#define SR_CARRY 0x0001

// Funções auxiliares para atualização de flags
static void update_flags_logical(megadrive_m68k_context_t *ctx, uint32_t result,
                                 uint8_t size) {
  uint32_t mask = (size == 1) ? 0xFF : (size == 2) ? 0xFFFF : 0xFFFFFFFF;
  uint32_t sign_bit = (size == 1) ? 0x80 : (size == 2) ? 0x8000 : 0x80000000;

  // Clear flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Set flags
  if ((result & mask) == 0)
    ctx->sr |= SR_ZERO;
  if (result & sign_bit)
    ctx->sr |= SR_NEGATIVE;
}

// Implementação das instruções lógicas
void m68k_execute_and(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t result = src & dst;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_logical(ctx, result, inst->size);

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_or(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t result = src | dst;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_logical(ctx, result, inst->size);

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_eor(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t dst = inst->dst_value;
  uint32_t result = src ^ dst;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_logical(ctx, result, inst->size);

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_not(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t result = ~src;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_logical(ctx, result, inst->size);

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_neg(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;
  uint32_t result = 0 - src;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  result &= mask;

  // Atualiza flags
  update_flags_logical(ctx, result, inst->size);

  // Carry e Extend são setados se o resultado não é zero
  if (result != 0) {
    ctx->sr |= SR_CARRY;
  }

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_clr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;

  // Atualiza flags
  ctx->sr &= ~(SR_NEGATIVE | SR_OVERFLOW | SR_CARRY);
  ctx->sr |= SR_ZERO;

  // Armazena resultado (zero)
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] &= ~mask;
  } else {
    write_value(ctx, inst->dst_addr, 0, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_tst(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t src = inst->src_value;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  src &= mask;

  // Atualiza flags
  update_flags_logical(ctx, src, inst->size);

  ctx->cycles += inst->cycles;
}

// Funções de deslocamento e rotação
void m68k_execute_asl(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t count = inst->src_value & 63;
  uint32_t data = inst->dst_value;
  uint32_t result = data;
  uint32_t last_out = 0;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  uint32_t sign_bit = (inst->size == 1)   ? 0x80
                      : (inst->size == 2) ? 0x8000
                                          : 0x80000000;

  // Clear flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Executa deslocamento
  if (count > 0) {
    while (count--) {
      last_out = (result & sign_bit) ? 1 : 0;
      result <<= 1;
    }

    // Atualiza flags
    if (last_out)
      ctx->sr |= SR_CARRY;
    if (result & sign_bit)
      ctx->sr |= SR_NEGATIVE;
    if ((result & mask) == 0)
      ctx->sr |= SR_ZERO;

    // Overflow é setado se o bit de sinal mudou durante o deslocamento
    if ((data & sign_bit) != (result & sign_bit)) {
      ctx->sr |= SR_OVERFLOW;
    }
  }

  result &= mask;

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_asr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t count = inst->src_value & 63;
  uint32_t data = inst->dst_value;
  uint32_t result = data;
  uint32_t last_out = 0;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  uint32_t sign_bit = (inst->size == 1)   ? 0x80
                      : (inst->size == 2) ? 0x8000
                                          : 0x80000000;

  // Clear flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Executa deslocamento
  if (count > 0) {
    // Preserva o bit de sinal
    uint32_t sign = data & sign_bit;

    while (count--) {
      last_out = result & 1;
      result = (result >> 1) | sign;
    }

    // Atualiza flags
    if (last_out)
      ctx->sr |= SR_CARRY;
    if (result & sign_bit)
      ctx->sr |= SR_NEGATIVE;
    if ((result & mask) == 0)
      ctx->sr |= SR_ZERO;
  }

  result &= mask;

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_lsl(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t count = inst->src_value & 63;
  uint32_t data = inst->dst_value;
  uint32_t result = data;
  uint32_t last_out = 0;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;

  // Clear flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Executa deslocamento
  if (count > 0) {
    while (count--) {
      last_out = (result & mask) >> (inst->size * 8 - 1);
      result <<= 1;
    }

    // Atualiza flags
    if (last_out)
      ctx->sr |= SR_CARRY;
    if (result & (mask >> 1))
      ctx->sr |= SR_NEGATIVE;
    if ((result & mask) == 0)
      ctx->sr |= SR_ZERO;
  }

  result &= mask;

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_lsr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t count = inst->src_value & 63;
  uint32_t data = inst->dst_value;
  uint32_t result = data;
  uint32_t last_out = 0;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;

  // Clear flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Executa deslocamento
  if (count > 0) {
    while (count--) {
      last_out = result & 1;
      result >>= 1;
    }

    // Atualiza flags
    if (last_out)
      ctx->sr |= SR_CARRY;
    if ((result & mask) == 0)
      ctx->sr |= SR_ZERO;
  }

  result &= mask;

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_rol(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t count = inst->src_value & 63;
  uint32_t data = inst->dst_value;
  uint32_t result = data;
  uint32_t last_out = 0;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  uint32_t msb = (inst->size == 1) ? 7 : (inst->size == 2) ? 15 : 31;

  // Clear flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Executa rotação
  if (count > 0) {
    count %= msb + 1;
    while (count--) {
      last_out = (result & mask) >> msb;
      result = ((result << 1) | last_out) & mask;
    }

    // Atualiza flags
    if (last_out)
      ctx->sr |= SR_CARRY;
    if (result & (1 << msb))
      ctx->sr |= SR_NEGATIVE;
    if ((result & mask) == 0)
      ctx->sr |= SR_ZERO;
  }

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_ror(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  uint32_t count = inst->src_value & 63;
  uint32_t data = inst->dst_value;
  uint32_t result = data;
  uint32_t last_out = 0;

  // Aplica máscara baseada no tamanho
  uint32_t mask = (inst->size == 1)   ? 0xFF
                  : (inst->size == 2) ? 0xFFFF
                                      : 0xFFFFFFFF;
  uint32_t msb = (inst->size == 1) ? 7 : (inst->size == 2) ? 15 : 31;

  // Clear flags
  ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);

  // Executa rotação
  if (count > 0) {
    count %= msb + 1;
    while (count--) {
      last_out = result & 1;
      result = ((result >> 1) | (last_out << msb)) & mask;
    }

    // Atualiza flags
    if (last_out)
      ctx->sr |= SR_CARRY;
    if (result & (1 << msb))
      ctx->sr |= SR_NEGATIVE;
    if ((result & mask) == 0)
      ctx->sr |= SR_ZERO;
  }

  // Armazena resultado
  if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
    ctx->registers[inst->dst_reg] =
        (ctx->registers[inst->dst_reg] & ~mask) | result;
  } else {
    write_value(ctx, inst->dst_addr, result, inst->size);
  }

  ctx->cycles += inst->cycles;
}
