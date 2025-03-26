/**
 * @file m68k_execute.c
 * @brief Implementação do ciclo de execução do processador M68000
 * @version 1.0
 * @date 2024-03-21
 */

#include "m68k_adapter.h"
#include <string.h>

// Tipos de endereçamento
typedef enum {
  M68K_ADDR_MODE_DATA_REG,      // Dn
  M68K_ADDR_MODE_ADDR_REG,      // An
  M68K_ADDR_MODE_ADDR_INDIRECT, // (An)
  M68K_ADDR_MODE_POST_INC,      // (An)+
  M68K_ADDR_MODE_PRE_DEC,       // -(An)
  M68K_ADDR_MODE_DISP,          // (d16,An)
  M68K_ADDR_MODE_INDEX,         // (d8,An,Xn)
  M68K_ADDR_MODE_ABS_SHORT,     // (xxx).W
  M68K_ADDR_MODE_ABS_LONG,      // (xxx).L
  M68K_ADDR_MODE_PC_DISP,       // (d16,PC)
  M68K_ADDR_MODE_PC_INDEX,      // (d8,PC,Xn)
  M68K_ADDR_MODE_IMMEDIATE      // #<data>
} m68k_addr_mode_t;

// Estrutura da instrução decodificada
typedef struct {
  uint16_t opcode;
  uint8_t size;       // Tamanho em bytes (1, 2 ou 4)
  uint8_t src_mode;   // Modo de endereçamento fonte
  uint8_t src_reg;    // Registrador fonte
  uint8_t dst_mode;   // Modo de endereçamento destino
  uint8_t dst_reg;    // Registrador destino
  uint32_t src_value; // Valor fonte
  uint32_t dst_value; // Valor destino
  uint8_t cycles;     // Ciclos base da instrução
} m68k_instruction_t;

// Funções auxiliares de acesso à memória
static uint16_t fetch_word(megadrive_m68k_context_t *ctx) {
  uint16_t word = (ctx->read_callback(ctx->pc, ctx->callback_data) << 8) |
                  ctx->read_callback(ctx->pc + 1, ctx->callback_data);
  ctx->pc += 2;
  ctx->cycles += 4;
  return word;
}

static uint32_t fetch_long(megadrive_m68k_context_t *ctx) {
  uint32_t value = (fetch_word(ctx) << 16) | fetch_word(ctx);
  return value;
}

static uint32_t read_long(megadrive_m68k_context_t *ctx, uint32_t addr) {
  uint32_t value = (ctx->read_callback(addr, ctx->callback_data) << 24) |
                   (ctx->read_callback(addr + 1, ctx->callback_data) << 16) |
                   (ctx->read_callback(addr + 2, ctx->callback_data) << 8) |
                   ctx->read_callback(addr + 3, ctx->callback_data);
  ctx->cycles += 8;
  return value;
}

static void write_long(megadrive_m68k_context_t *ctx, uint32_t addr,
                       uint32_t value) {
  ctx->write_callback(addr, value >> 24, ctx->callback_data);
  ctx->write_callback(addr + 1, (value >> 16) & 0xFF, ctx->callback_data);
  ctx->write_callback(addr + 2, (value >> 8) & 0xFF, ctx->callback_data);
  ctx->write_callback(addr + 3, value & 0xFF, ctx->callback_data);
  ctx->cycles += 8;
}

// Decodificação de instruções
static void decode_instruction(megadrive_m68k_context_t *ctx,
                               m68k_instruction_t *inst) {
  inst->opcode = fetch_word(ctx);

  // Decodifica grupo principal
  switch (inst->opcode >> 12) {
  case 0x0:                                  // Bit manipulation/MOVEP/Immediate
    if ((inst->opcode & 0x0138) == 0x0108) { // MOVEP
      inst->size = 2 + ((inst->opcode >> 6) & 1) * 2; // Word ou Long
      inst->dst_reg = (inst->opcode >> 9) & 7;
      inst->dst_mode = M68K_ADDR_MODE_DATA_REG;
      inst->src_reg = inst->opcode & 7;
      inst->src_mode = M68K_ADDR_MODE_DISP;
      inst->cycles = (inst->size == 2) ? 16 : 24;
    }
    break;

  case 0x1: // Move.B
  case 0x2: // Move.L
  case 0x3: // Move.W
    inst->size = (inst->opcode >> 12) == 1   ? 1
                 : (inst->opcode >> 12) == 2 ? 4
                                             : 2;
    inst->dst_mode = (inst->opcode >> 6) & 7;
    inst->dst_reg = (inst->opcode >> 9) & 7;
    inst->src_mode = (inst->opcode >> 3) & 7;
    inst->src_reg = inst->opcode & 7;
    inst->cycles = 4; // Ciclos base, será ajustado pelo modo de endereçamento
    break;

  case 0x4:                                  // Miscellaneous
    if ((inst->opcode & 0xFFC0) == 0x4E40) { // TRAP
      inst->size = 0;
      inst->src_value = inst->opcode & 0xF;
      inst->cycles = 34;
    } else if ((inst->opcode & 0xFFF0) == 0x4E60) { // MOVE to/from USP
      inst->size = 4;
      inst->src_reg = inst->opcode & 7;
      inst->cycles = 4;
    }
    break;

  case 0x5:                                  // ADDQ/SUBQ/Scc/DBcc
    if ((inst->opcode & 0xF0C0) == 0x50C0) { // Scc
      inst->size = 1;
      inst->dst_mode = inst->opcode & 7;
      inst->dst_reg = (inst->opcode >> 9) & 7;
      inst->cycles = 4;
    }
    break;

  case 0x6: // Bcc/BSR/BRA
    inst->size = 1;
    if ((inst->opcode & 0xFF00) == 0x6100) { // BSR
      inst->src_value = (int8_t)(inst->opcode & 0xFF);
      inst->cycles = 18;
    }
    break;

  case 0x7: // MOVEQ
    if ((inst->opcode & 0xF100) == 0x7000) {
      inst->size = 4;
      inst->dst_reg = (inst->opcode >> 9) & 7;
      inst->src_value = (int8_t)(inst->opcode & 0xFF);
      inst->cycles = 4;
    }
    break;

  default:
    // Instrução não implementada
    ctx->stopped = true;
    break;
  }
}

// Execução de instruções
static void execute_instruction(megadrive_m68k_context_t *ctx,
                                m68k_instruction_t *inst) {
  switch (inst->opcode >> 12) {
  case 0x1: // Move.B
  case 0x2: // Move.L
  case 0x3: // Move.W
    // Implementa MOVE
    if (inst->dst_mode == M68K_ADDR_MODE_DATA_REG) {
      uint32_t mask = (inst->size == 1)   ? 0xFF
                      : (inst->size == 2) ? 0xFFFF
                                          : 0xFFFFFFFF;
      ctx->registers[inst->dst_reg] =
          (ctx->registers[inst->dst_reg] & ~mask) | (inst->src_value & mask);
    }
    break;

  case 0x7: // MOVEQ
    if ((inst->opcode & 0xF100) == 0x7000) {
      ctx->registers[inst->dst_reg] = inst->src_value;

      // Atualiza flags
      ctx->sr &= ~(SR_NEGATIVE | SR_ZERO | SR_OVERFLOW | SR_CARRY);
      if (inst->src_value == 0)
        ctx->sr |= SR_ZERO;
      if (inst->src_value & 0x80)
        ctx->sr |= SR_NEGATIVE;
    }
    break;
  }

  ctx->cycles += inst->cycles;
}

// Função principal de execução
int m68k_execute_cycles(megadrive_m68k_context_t *ctx, int target_cycles) {
  ctx->cycles = 0;

  while (ctx->cycles < target_cycles && !ctx->stopped) {
    // Verifica interrupções
    if (ctx->interrupt_pending) {
      uint8_t level = (ctx->sr & SR_INT_MASK) >> 8;
      if (ctx->interrupt_level > level) {
        // Salva contexto
        uint32_t old_pc = ctx->pc;
        uint16_t old_sr = ctx->sr;

        // Processa interrupção
        ctx->sr |= (ctx->interrupt_level << 8) & SR_INT_MASK;
        ctx->sr |= SR_SUPERVISOR;

        // Obtém vetor de interrupção
        uint32_t vector = 24 + ctx->interrupt_level;
        uint32_t vector_addr = vector * 4;

        // Empilha PC e SR
        ctx->registers[REG_A7] -= 4;
        write_long(ctx, ctx->registers[REG_A7], old_pc);
        ctx->registers[REG_A7] -= 2;
        ctx->write_callback(ctx->registers[REG_A7], old_sr >> 8,
                            ctx->callback_data);
        ctx->write_callback(ctx->registers[REG_A7] + 1, old_sr & 0xFF,
                            ctx->callback_data);

        // Carrega novo PC do vetor
        ctx->pc = read_long(ctx, vector_addr);

        ctx->interrupt_pending = false;
        ctx->cycles += 44;
        continue;
      }
    }

    // Decodifica e executa próxima instrução
    m68k_instruction_t inst = {0};
    decode_instruction(ctx, &inst);
    if (!ctx->stopped) {
      execute_instruction(ctx, &inst);
    }
  }

  return ctx->cycles;
}
