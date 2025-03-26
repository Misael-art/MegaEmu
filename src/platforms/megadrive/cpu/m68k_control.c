/**
 * @file m68k_control.c
 * @brief Implementação das instruções de controle do M68000
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

// Condições para desvios
typedef enum {
  COND_TRUE = 0x0,  // T
  COND_FALSE = 0x1, // F
  COND_HI = 0x2,    // Higher
  COND_LS = 0x3,    // Lower or Same
  COND_CC = 0x4,    // Carry Clear
  COND_CS = 0x5,    // Carry Set
  COND_NE = 0x6,    // Not Equal
  COND_EQ = 0x7,    // Equal
  COND_VC = 0x8,    // Overflow Clear
  COND_VS = 0x9,    // Overflow Set
  COND_PL = 0xA,    // Plus
  COND_MI = 0xB,    // Minus
  COND_GE = 0xC,    // Greater or Equal
  COND_LT = 0xD,    // Less Than
  COND_GT = 0xE,    // Greater Than
  COND_LE = 0xF     // Less or Equal
} m68k_condition_t;

// Verifica se uma condição é verdadeira
static bool check_condition(megadrive_m68k_context_t *ctx,
                            m68k_condition_t cond) {
  uint16_t sr = ctx->sr;
  bool n = (sr & SR_NEGATIVE) != 0;
  bool z = (sr & SR_ZERO) != 0;
  bool v = (sr & SR_OVERFLOW) != 0;
  bool c = (sr & SR_CARRY) != 0;

  switch (cond) {
  case COND_TRUE:
    return true;
  case COND_FALSE:
    return false;
  case COND_HI:
    return !c && !z;
  case COND_LS:
    return c || z;
  case COND_CC:
    return !c;
  case COND_CS:
    return c;
  case COND_NE:
    return !z;
  case COND_EQ:
    return z;
  case COND_VC:
    return !v;
  case COND_VS:
    return v;
  case COND_PL:
    return !n;
  case COND_MI:
    return n;
  case COND_GE:
    return n == v;
  case COND_LT:
    return n != v;
  case COND_GT:
    return !z && (n == v);
  case COND_LE:
    return z || (n != v);
  default:
    return false;
  }
}

// Implementação das instruções de desvio
void m68k_execute_bcc(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  m68k_condition_t cond = (m68k_condition_t)(inst->opcode & 0x0F);

  if (check_condition(ctx, cond)) {
    // Desvio é relativo ao PC atual
    ctx->pc += (int8_t)inst->src_value;
    ctx->cycles += inst->cycles;
  } else {
    // Se condição é falsa, apenas avança o PC
    ctx->cycles += 2;
  }
}

void m68k_execute_bra(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Desvio incondicional
  ctx->pc += (int8_t)inst->src_value;
  ctx->cycles += inst->cycles;
}

void m68k_execute_bsr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Salva endereço de retorno na pilha
  ctx->registers[15] -= 4; // SP -= 4
  write_long(ctx, ctx->registers[15], ctx->pc);

  // Realiza o desvio
  ctx->pc += (int8_t)inst->src_value;
  ctx->cycles += inst->cycles;
}

void m68k_execute_jmp(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Desvio absoluto
  ctx->pc = inst->src_addr;
  ctx->cycles += inst->cycles;
}

void m68k_execute_jsr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Salva endereço de retorno na pilha
  ctx->registers[15] -= 4; // SP -= 4
  write_long(ctx, ctx->registers[15], ctx->pc);

  // Realiza o desvio
  ctx->pc = inst->src_addr;
  ctx->cycles += inst->cycles;
}

void m68k_execute_rts(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Recupera endereço de retorno da pilha
  ctx->pc = read_long(ctx, ctx->registers[15]);
  ctx->registers[15] += 4; // SP += 4
  ctx->cycles += inst->cycles;
}

void m68k_execute_rte(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Recupera SR e PC da pilha
  ctx->sr = read_word(ctx, ctx->registers[15]);
  ctx->registers[15] += 2; // SP += 2

  ctx->pc = read_long(ctx, ctx->registers[15]);
  ctx->registers[15] += 4; // SP += 4

  ctx->cycles += inst->cycles;
}

void m68k_execute_trap(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  uint8_t vector = inst->src_value & 0x0F;

  // Salva contexto atual na pilha
  ctx->registers[15] -= 4; // SP -= 4
  write_long(ctx, ctx->registers[15], ctx->pc);

  ctx->registers[15] -= 2; // SP -= 2
  write_word(ctx, ctx->registers[15], ctx->sr);

  // Carrega novo PC do vetor de exceção
  ctx->pc = read_long(ctx, vector * 4);
  ctx->cycles += inst->cycles;
}

void m68k_execute_dbcc(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  m68k_condition_t cond = (m68k_condition_t)(inst->opcode & 0x0F);
  uint8_t reg = inst->dst_reg;

  if (!check_condition(ctx, cond)) {
    // Decrementa contador
    uint16_t count = (uint16_t)ctx->registers[reg];
    count--;
    ctx->registers[reg] = (ctx->registers[reg] & 0xFFFF0000) | count;

    if (count != 0xFFFF) {
      // Se contador não expirou, realiza o desvio
      ctx->pc += (int16_t)inst->src_value;
      ctx->cycles += inst->cycles;
    } else {
      // Se contador expirou, apenas avança o PC
      ctx->cycles += 2;
    }
  } else {
    // Se condição é verdadeira, apenas avança o PC
    ctx->cycles += 2;
  }
}

void m68k_execute_nop(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst) {
  // Não faz nada, apenas consome ciclos
  ctx->cycles += inst->cycles;
}

void m68k_execute_reset(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst) {
  // Instrução privilegiada - verifica modo supervisor
  if (!(ctx->sr & 0x2000)) {
    // Gera exceção de privilégio
    m68k_instruction_t trap_inst = {.src_value = 8, // Vetor de privilégio
                                    .cycles = 34};
    m68k_execute_trap(ctx, &trap_inst);
    return;
  }

  // Reseta dispositivos externos
  if (ctx->reset_callback) {
    ctx->reset_callback(ctx->user_data);
  }

  ctx->cycles += inst->cycles;
}

void m68k_execute_stop(megadrive_m68k_context_t *ctx,
                       m68k_instruction_t *inst) {
  // Instrução privilegiada - verifica modo supervisor
  if (!(ctx->sr & 0x2000)) {
    // Gera exceção de privilégio
    m68k_instruction_t trap_inst = {.src_value = 8, // Vetor de privilégio
                                    .cycles = 34};
    m68k_execute_trap(ctx, &trap_inst);
    return;
  }

  // Atualiza SR e para o processador
  ctx->sr = inst->src_value;
  ctx->stopped = true;
  ctx->cycles += inst->cycles;
}

void m68k_execute_illegal(megadrive_m68k_context_t *ctx,
                          m68k_instruction_t *inst) {
  // Gera exceção de instrução ilegal
  m68k_instruction_t trap_inst = {.src_value = 4, // Vetor de instrução ilegal
                                  .cycles = 34};
  m68k_execute_trap(ctx, &trap_inst);
}
