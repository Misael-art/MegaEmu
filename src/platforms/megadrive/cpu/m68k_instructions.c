/**
 * @file m68k_instructions.c
 * @brief Implementação das instruções da CPU Motorola 68000
 *
 * Este arquivo contém a implementação das funções para decodificação,
 * execução e manipulação de operandos para a CPU M68K do Mega Drive.
 */

#include "m68k_instructions.h"
#include "m68k.h"
#include "m68k_timing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tabelas de ciclos para instruções
static const uint32_t g_instruction_cycles[] = {
    6,   // M68K_INST_ABCD
    4,   // M68K_INST_ADD
    8,   // M68K_INST_ADDA
    8,   // M68K_INST_ADDI
    4,   // M68K_INST_ADDQ
    4,   // M68K_INST_ADDX
    4,   // M68K_INST_AND
    8,   // M68K_INST_ANDI
    6,   // M68K_INST_ASL
    6,   // M68K_INST_ASR
    10,  // M68K_INST_BCC
    8,   // M68K_INST_BCHG
    8,   // M68K_INST_BCLR
    10,  // M68K_INST_BRA
    8,   // M68K_INST_BSET
    18,  // M68K_INST_BSR
    4,   // M68K_INST_BTST
    10,  // M68K_INST_CHK
    4,   // M68K_INST_CLR
    4,   // M68K_INST_CMP
    6,   // M68K_INST_CMPA
    8,   // M68K_INST_CMPI
    12,  // M68K_INST_CMPM
    12,  // M68K_INST_DBCC
    158, // M68K_INST_DIVS
    140, // M68K_INST_DIVU
    4,   // M68K_INST_EOR
    8,   // M68K_INST_EORI
    6,   // M68K_INST_EXG
    4,   // M68K_INST_EXT
    4,   // M68K_INST_ILLEGAL
    8,   // M68K_INST_JMP
    16,  // M68K_INST_JSR
    4,   // M68K_INST_LEA
    16,  // M68K_INST_LINK
    6,   // M68K_INST_LSL
    6,   // M68K_INST_LSR
    4,   // M68K_INST_MOVE
    4,   // M68K_INST_MOVEA
    12,  // M68K_INST_MOVEM
    16,  // M68K_INST_MOVEP
    4,   // M68K_INST_MOVEQ
    70,  // M68K_INST_MULS
    70,  // M68K_INST_MULU
    6,   // M68K_INST_NBCD
    4,   // M68K_INST_NEG
    4,   // M68K_INST_NEGX
    4,   // M68K_INST_NOP
    4,   // M68K_INST_NOT
    4,   // M68K_INST_OR
    8,   // M68K_INST_ORI
    8,   // M68K_INST_PEA
    132, // M68K_INST_RESET
    6,   // M68K_INST_ROL
    6,   // M68K_INST_ROR
    8,   // M68K_INST_ROXL
    8,   // M68K_INST_ROXR
    20,  // M68K_INST_RTE
    20,  // M68K_INST_RTR
    16,  // M68K_INST_RTS
    6,   // M68K_INST_SBCD
    4,   // M68K_INST_SCC
    4,   // M68K_INST_STOP
    4,   // M68K_INST_SUB
    8,   // M68K_INST_SUBA
    8,   // M68K_INST_SUBI
    4,   // M68K_INST_SUBQ
    4,   // M68K_INST_SUBX
    4,   // M68K_INST_SWAP
    10,  // M68K_INST_TAS
    4,   // M68K_INST_TRAP
    4,   // M68K_INST_TRAPV
    4,   // M68K_INST_TST
    12,  // M68K_INST_UNLK
    4    // M68K_INST_INVALID
};

// Tabelas de ciclos para modos de endereçamento
static const uint32_t g_ea_cycles[] = {
    0,  // M68K_ADDR_MODE_DATA_REG_DIRECT
    0,  // M68K_ADDR_MODE_ADDR_REG_DIRECT
    4,  // M68K_ADDR_MODE_ADDR_REG_INDIRECT
    4,  // M68K_ADDR_MODE_ADDR_REG_INDIRECT_POST
    6,  // M68K_ADDR_MODE_ADDR_REG_INDIRECT_PRE
    8,  // M68K_ADDR_MODE_ADDR_REG_INDIRECT_DISP
    10, // M68K_ADDR_MODE_ADDR_REG_INDIRECT_INDEX
    8,  // M68K_ADDR_MODE_PC_INDIRECT_DISP
    10, // M68K_ADDR_MODE_PC_INDIRECT_INDEX
    8,  // M68K_ADDR_MODE_ABSOLUTE_SHORT
    12, // M68K_ADDR_MODE_ABSOLUTE_LONG
    4,  // M68K_ADDR_MODE_IMMEDIATE
    0,  // M68K_ADDR_MODE_IMPLIED
    0   // M68K_ADDR_MODE_INVALID
};

// Tabelas de ciclos para acessos à memória
static const uint32_t g_mem_cycles[] = {
    4, // M68K_SIZE_BYTE
    4, // M68K_SIZE_WORD
    8  // M68K_SIZE_LONG
};

// Tabelas de ciclos para branches
static const uint32_t g_branch_cycles[] = {
    10, // Branch taken (short)
    10, // Branch taken (long)
    8   // Branch not taken
};

int32_t md_m68k_decode_instruction(uint16_t opcode, uint32_t pc,
                                   md_m68k_instruction_t *instruction) {
  if (!instruction)
    return 0;

  // Limpa a estrutura
  memset(instruction, 0, sizeof(md_m68k_instruction_t));

  instruction->opcode = opcode;
  instruction->address = pc;

  // Decodifica o grupo principal do opcode
  uint8_t group = (opcode >> 12) & 0xF;

  switch (group) {
  case 0x0: // Bit manipulation/MOVEP/Immediate
    // TODO: Implementar decodificação
    break;

  case 0x1: // Move.B
    instruction->type = M68K_INST_MOVE;
    instruction->size = M68K_SIZE_BYTE;
    break;

  case 0x2: // Move.L
    instruction->type = M68K_INST_MOVE;
    instruction->size = M68K_SIZE_LONG;
    break;

  case 0x3: // Move.W
    instruction->type = M68K_INST_MOVE;
    instruction->size = M68K_SIZE_WORD;
    break;

    // TODO: Implementar outros grupos

  default:
    instruction->type = M68K_INST_INVALID;
    break;
  }

  // Calcula informações de timing
  instruction->timing.base_cycles = g_instruction_cycles[instruction->type];
  instruction->timing.ea_cycles =
      g_ea_cycles[instruction->src_mode] + g_ea_cycles[instruction->dst_mode];
  instruction->timing.mem_cycles = g_mem_cycles[instruction->size];

  // Configura flags de execução
  instruction->execution.needs_prefetch = true;
  instruction->execution.changes_pc = (instruction->type >= M68K_INST_BCC &&
                                       instruction->type <= M68K_INST_BSR) ||
                                      instruction->type == M68K_INST_JMP ||
                                      instruction->type == M68K_INST_JSR ||
                                      instruction->type == M68K_INST_RTE ||
                                      instruction->type == M68K_INST_RTR ||
                                      instruction->type == M68K_INST_RTS;

  instruction->execution.is_privileged =
      (instruction->type == M68K_INST_RESET ||
       instruction->type == M68K_INST_STOP ||
       instruction->type == M68K_INST_RTE);

  instruction->execution.affects_ccr = true;

  // Configura flags de timing
  instruction->timing.is_rmw = (instruction->type == M68K_INST_ASL ||
                                instruction->type == M68K_INST_ASR ||
                                instruction->type == M68K_INST_LSL ||
                                instruction->type == M68K_INST_LSR ||
                                instruction->type == M68K_INST_ROL ||
                                instruction->type == M68K_INST_ROR ||
                                instruction->type == M68K_INST_ROXL ||
                                instruction->type == M68K_INST_ROXR ||
                                instruction->type == M68K_INST_TAS);

  instruction->timing.uses_prefetch = true;

  return 2; // Tamanho base da instrução
}

// Funções de execução específicas para cada instrução
static int32_t execute_move(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t cycles = 0;

  // Lê o valor fonte
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 instruction->size, &src_value, timing);

  // Escreve no destino
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, src_value, timing);

  // Atualiza flags
  md_m68k_update_flags(src_value, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_add(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t result = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 instruction->size, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &dst_value, timing);

  // Realiza a operação
  result = dst_value + src_value;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_sub(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t result = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 instruction->size, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &dst_value, timing);

  // Realiza a operação
  result = dst_value - src_value;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_and(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t result = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 instruction->size, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &dst_value, timing);

  // Realiza a operação
  result = dst_value & src_value;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_or(const md_m68k_instruction_t *instruction,
                          md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t result = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 instruction->size, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &dst_value, timing);

  // Realiza a operação
  result = dst_value | src_value;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_eor(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t result = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 instruction->size, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &dst_value, timing);

  // Realiza a operação
  result = dst_value ^ src_value;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_not(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t result = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &value, timing);

  // Realiza a operação
  result = ~value;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_neg(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t result = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &value, timing);

  // Realiza a operação
  result = 0 - value;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_clr(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t cycles = 0;

  // Escreve zero no destino
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, 0, timing);

  // Atualiza flags
  md_m68k_update_flags(0, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_tst(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &value, timing);

  // Atualiza flags
  md_m68k_update_flags(value, instruction->size, 0x0F);

  return cycles;
}

static int32_t execute_jmp(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t address = 0;
  uint32_t cycles = 0;

  // Calcula o endereço efetivo
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_LONG, &address, timing);

  // Atualiza o PC
  md_m68k_set_pc(address);

  return cycles;
}

static int32_t execute_jsr(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t address = 0;
  uint32_t cycles = 0;

  // Calcula o endereço efetivo
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_LONG, &address, timing);

  // Empilha o endereço de retorno
  uint32_t return_address = md_m68k_get_pc() + 2;
  md_m68k_push_long(return_address);

  // Atualiza o PC
  md_m68k_set_pc(address);

  return cycles;
}

static int32_t execute_rts(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t address = 0;

  // Desempilha o endereço de retorno
  address = md_m68k_pop_long();

  // Atualiza o PC
  md_m68k_set_pc(address);

  return 16; // Ciclos fixos para RTS
}

static int32_t execute_bra(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  // Calcula o endereço de destino
  uint32_t target = md_m68k_get_pc() + instruction->displacement;

  // Atualiza o PC
  md_m68k_set_pc(target);

  // Retorna ciclos base + ciclos de branch
  return instruction->timing.base_cycles + instruction->timing.branch_cycles;
}

static int32_t execute_bsr(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  // Calcula o endereço de destino
  uint32_t target = md_m68k_get_pc() + instruction->displacement;

  // Empilha o endereço de retorno
  uint32_t return_address = md_m68k_get_pc() + 2;
  md_m68k_push_long(return_address);

  // Atualiza o PC
  md_m68k_set_pc(target);

  // Retorna ciclos base + ciclos de branch
  return instruction->timing.base_cycles + instruction->timing.branch_cycles;
}

static int32_t execute_bcc(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  // Avalia a condição
  bool condition_true = md_m68k_evaluate_condition(instruction->condition);

  if (condition_true) {
    // Calcula o endereço de destino
    uint32_t target = md_m68k_get_pc() + instruction->displacement;

    // Atualiza o PC
    md_m68k_set_pc(target);

    // Retorna ciclos para branch taken
    return instruction->timing.base_cycles +
           ((instruction->displacement > 127 ||
             instruction->displacement < -128)
                ? g_branch_cycles[1]
                : g_branch_cycles[0]);
  }

  // Retorna ciclos para branch not taken
  return instruction->timing.base_cycles + g_branch_cycles[2];
}

static int32_t execute_bchg(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t bit_num = 0;
  uint32_t operand = 0;
  uint32_t cycles = 0;

  // Lê o número do bit
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   instruction->size, &bit_num, timing);
    bit_num &= 0x1F; // Máscara para 32 bits
  } else {
    bit_num = instruction->immediate & 0x07; // Máscara para 8 bits
  }

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &operand, timing);

  // Testa o bit
  uint32_t mask = 1 << bit_num;
  bool bit_set = (operand & mask) != 0;

  // Inverte o bit
  operand ^= mask;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, operand, timing);

  // Atualiza flag Z (zero se o bit estava setado)
  if (bit_set) {
    timing->ccr &= ~M68K_SR_Z;
  } else {
    timing->ccr |= M68K_SR_Z;
  }

  return cycles;
}

static int32_t execute_bclr(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t bit_num = 0;
  uint32_t operand = 0;
  uint32_t cycles = 0;

  // Lê o número do bit
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   instruction->size, &bit_num, timing);
    bit_num &= 0x1F;
  } else {
    bit_num = instruction->immediate & 0x07;
  }

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &operand, timing);

  // Testa o bit
  uint32_t mask = 1 << bit_num;
  bool bit_set = (operand & mask) != 0;

  // Limpa o bit
  operand &= ~mask;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, operand, timing);

  // Atualiza flag Z (zero se o bit estava setado)
  if (bit_set) {
    timing->ccr &= ~M68K_SR_Z;
  } else {
    timing->ccr |= M68K_SR_Z;
  }

  return cycles;
}

static int32_t execute_bset(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t bit_num = 0;
  uint32_t operand = 0;
  uint32_t cycles = 0;

  // Lê o número do bit
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   instruction->size, &bit_num, timing);
    bit_num &= 0x1F;
  } else {
    bit_num = instruction->immediate & 0x07;
  }

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &operand, timing);

  // Testa o bit
  uint32_t mask = 1 << bit_num;
  bool bit_set = (operand & mask) != 0;

  // Seta o bit
  operand |= mask;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, operand, timing);

  // Atualiza flag Z (zero se o bit estava setado)
  if (bit_set) {
    timing->ccr &= ~M68K_SR_Z;
  } else {
    timing->ccr |= M68K_SR_Z;
  }

  return cycles;
}

static int32_t execute_tas(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t operand = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_BYTE, &operand, timing);

  // Atualiza flags
  md_m68k_update_flags(operand, M68K_SIZE_BYTE,
                       M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C);

  // Seta o bit 7
  operand |= 0x80;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_BYTE, operand, timing);

  return cycles;
}

static int32_t execute_swap(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t operand = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_LONG, &operand, timing);

  // Realiza o swap
  operand = ((operand & 0xFFFF) << 16) | ((operand >> 16) & 0xFFFF);

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_LONG, operand, timing);

  // Atualiza flags
  md_m68k_update_flags(operand, M68K_SIZE_LONG, M68K_SR_N | M68K_SR_Z);
  timing->ccr &= ~(M68K_SR_V | M68K_SR_C); // Limpa V e C

  return cycles;
}

static int32_t execute_ext(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t operand = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &operand, timing);

  // Realiza a extensão de sinal
  if (instruction->size == M68K_SIZE_WORD) {
    // Byte para Word
    if (operand & 0x80) {
      operand |= 0xFF00;
    }
  } else {
    // Word para Long
    if (operand & 0x8000) {
      operand |= 0xFFFF0000;
    }
  }

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  instruction->size, operand, timing);

  // Atualiza flags
  md_m68k_update_flags(operand, instruction->size, M68K_SR_N | M68K_SR_Z);
  timing->ccr &= ~(M68K_SR_V | M68K_SR_C); // Limpa V e C

  return cycles;
}

static int32_t execute_link(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t sp = 0;
  uint32_t an = 0;
  uint32_t displacement = (int16_t)instruction->immediate;
  uint32_t cycles = 0;

  // Lê SP
  cycles += md_m68k_read_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, 7,
                                 M68K_SIZE_LONG, &sp, timing);

  // Lê An
  cycles +=
      md_m68k_read_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, instruction->dst_reg,
                           M68K_SIZE_LONG, &an, timing);

  // Decrementa SP
  sp -= 4;

  // Salva An na pilha
  cycles += md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_INDIRECT_PRE, 7,
                                  M68K_SIZE_LONG, an, timing);

  // Move SP para An
  cycles +=
      md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT,
                            instruction->dst_reg, M68K_SIZE_LONG, sp, timing);

  // Adiciona deslocamento ao SP
  sp += displacement;

  // Atualiza SP
  cycles += md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, 7,
                                  M68K_SIZE_LONG, sp, timing);

  return cycles;
}

static int32_t execute_unlk(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t sp = 0;
  uint32_t an = 0;
  uint32_t cycles = 0;

  // Move An para SP
  cycles +=
      md_m68k_read_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, instruction->src_reg,
                           M68K_SIZE_LONG, &sp, timing);
  cycles += md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, 7,
                                  M68K_SIZE_LONG, sp, timing);

  // Restaura An da pilha
  cycles += md_m68k_read_operand(M68K_ADDR_MODE_ADDR_REG_INDIRECT_POST, 7,
                                 M68K_SIZE_LONG, &an, timing);
  cycles +=
      md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT,
                            instruction->src_reg, M68K_SIZE_LONG, an, timing);

  return cycles;
}

static int32_t execute_pea(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t ea = 0;
  uint32_t sp = 0;
  uint32_t cycles = 0;

  // Calcula endereço efetivo
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 M68K_SIZE_LONG, &ea, timing);

  // Lê SP
  cycles += md_m68k_read_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, 7,
                                 M68K_SIZE_LONG, &sp, timing);

  // Decrementa SP
  sp -= 4;
  cycles += md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, 7,
                                  M68K_SIZE_LONG, sp, timing);

  // Empilha endereço
  cycles += md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_INDIRECT, 7,
                                  M68K_SIZE_LONG, ea, timing);

  return cycles;
}

static int32_t execute_muls(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 M68K_SIZE_WORD, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_WORD, &dst_value, timing);

  // Converte para signed
  int16_t src_signed = (int16_t)src_value;
  int16_t dst_signed = (int16_t)dst_value;

  // Realiza a multiplicação com sinal
  int32_t result = (int32_t)src_signed * (int32_t)dst_signed;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_LONG, (uint32_t)result, timing);

  // Atualiza flags
  md_m68k_update_flags((uint32_t)result, M68K_SIZE_LONG, M68K_SR_N | M68K_SR_Z);
  timing->ccr &= ~(M68K_SR_V | M68K_SR_C); // Limpa V e C

  return cycles;
}

static int32_t execute_mulu(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 M68K_SIZE_WORD, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_WORD, &dst_value, timing);

  // Realiza a multiplicação sem sinal
  uint32_t result = (src_value & 0xFFFF) * (dst_value & 0xFFFF);

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_LONG, result, timing);

  // Atualiza flags
  md_m68k_update_flags(result, M68K_SIZE_LONG, M68K_SR_N | M68K_SR_Z);
  timing->ccr &= ~(M68K_SR_V | M68K_SR_C); // Limpa V e C

  return cycles;
}

static int32_t execute_divs(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 M68K_SIZE_WORD, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_LONG, &dst_value, timing);

  // Converte para signed
  int16_t divisor = (int16_t)src_value;
  int32_t dividend = (int32_t)dst_value;

  // Verifica divisão por zero
  if (divisor == 0) {
    // TODO: Gerar exceção de divisão por zero
    timing->ccr |= M68K_SR_V; // Seta flag de overflow
    return cycles;
  }

  // Realiza a divisão com sinal
  int16_t quotient = dividend / divisor;
  int16_t remainder = dividend % divisor;

  // Verifica overflow
  if (quotient < -32768 || quotient > 32767) {
    timing->ccr |= M68K_SR_V; // Seta flag de overflow
    return cycles;
  }

  // Combina quociente e resto
  uint32_t result =
      ((uint32_t)(remainder & 0xFFFF) << 16) | (quotient & 0xFFFF);

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_LONG, result, timing);

  // Atualiza flags
  md_m68k_update_flags(quotient, M68K_SIZE_WORD, M68K_SR_N | M68K_SR_Z);
  timing->ccr &= ~M68K_SR_C; // Limpa C

  return cycles;
}

static int32_t execute_divu(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 M68K_SIZE_WORD, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_LONG, &dst_value, timing);

  uint16_t divisor = src_value & 0xFFFF;
  uint32_t dividend = dst_value;

  // Verifica divisão por zero
  if (divisor == 0) {
    // TODO: Gerar exceção de divisão por zero
    timing->ccr |= M68K_SR_V; // Seta flag de overflow
    return cycles;
  }

  // Realiza a divisão sem sinal
  uint16_t quotient = dividend / divisor;
  uint16_t remainder = dividend % divisor;

  // Verifica overflow
  if ((dividend / divisor) > 0xFFFF) {
    timing->ccr |= M68K_SR_V; // Seta flag de overflow
    return cycles;
  }

  // Combina quociente e resto
  uint32_t result = ((uint32_t)remainder << 16) | quotient;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_LONG, result, timing);

  // Atualiza flags
  md_m68k_update_flags(quotient, M68K_SIZE_WORD, M68K_SR_N | M68K_SR_Z);
  timing->ccr &= ~(M68K_SR_V | M68K_SR_C); // Limpa V e C

  return cycles;
}

static int32_t execute_btst(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t bit_num = 0;
  uint32_t operand = 0;
  uint32_t cycles = 0;

  // Lê o número do bit
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   instruction->size, &bit_num, timing);
    bit_num &= 0x1F; // Máscara para 32 bits
  } else {
    bit_num = instruction->immediate & 0x07; // Máscara para 8 bits
  }

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &operand, timing);

  // Testa o bit
  uint32_t mask = 1 << bit_num;
  bool bit_set = (operand & mask) != 0;

  // Atualiza flag Z (zero se o bit estava setado)
  if (bit_set) {
    timing->ccr &= ~M68K_SR_Z;
  } else {
    timing->ccr |= M68K_SR_Z;
  }

  return cycles;
}

static int32_t execute_stop(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t new_sr = instruction->immediate;
  uint32_t cycles = 0;

  // Verifica se está em modo supervisor
  if (!(timing->sr & M68K_SR_S)) {
    // TODO: Gerar exceção de privilégio
    return cycles;
  }

  // Atualiza o SR
  timing->sr = new_sr;

  // Entra em estado de parada
  timing->stopped = true;

  return cycles;
}

static int32_t execute_trap(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t vector = instruction->immediate & 0x0F;
  uint32_t cycles = 0;

  // TODO: Implementar tratamento de exceção TRAP
  // Deve salvar o contexto e pular para o vetor de exceção apropriado

  return cycles;
}

static int32_t execute_trapv(const md_m68k_instruction_t *instruction,
                             md_m68k_timing_t *timing) {
  uint32_t cycles = 0;

  // Verifica flag de overflow
  if (timing->ccr & M68K_SR_V) {
    // TODO: Implementar tratamento de exceção TRAPV
    // Deve salvar o contexto e pular para o vetor de exceção
  }

  return cycles;
}

static int32_t execute_reset(const md_m68k_instruction_t *instruction,
                             md_m68k_timing_t *timing) {
  uint32_t cycles = 0;

  // Verifica se está em modo supervisor
  if (!(timing->sr & M68K_SR_S)) {
    // TODO: Gerar exceção de privilégio
    return cycles;
  }

  // TODO: Implementar reset externo
  // Deve gerar sinal de reset para dispositivos externos

  return cycles;
}

static int32_t execute_nop(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  // NOP não faz nada, apenas consome ciclos
  return 0;
}

static int32_t execute_illegal(const md_m68k_instruction_t *instruction,
                               md_m68k_timing_t *timing) {
  // TODO: Implementar tratamento de instrução ilegal
  // Deve gerar exceção de instrução ilegal
  return 0;
}

static int32_t execute_movem(const md_m68k_instruction_t *instruction,
                             md_m68k_timing_t *timing) {
  uint32_t cycles = 0;
  uint16_t mask = instruction->immediate;
  uint32_t address = 0;
  uint32_t value = 0;

  // Lê o endereço base
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_LONG, &address, timing);

  // Determina a direção (memória para registradores ou vice-versa)
  bool mem_to_reg =
      (instruction->dst_mode != M68K_ADDR_MODE_ADDR_REG_INDIRECT_PRE);

  if (mem_to_reg) {
    // Memória para registradores
    for (int i = 0; i < 16; i++) {
      if (mask & (1 << i)) {
        // Lê da memória
        cycles += md_m68k_read_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                       instruction->size, &value, timing);

        // Escreve no registrador
        if (i < 8) {
          // Registrador de dados
          cycles += md_m68k_write_operand(M68K_ADDR_MODE_DATA_REG_DIRECT, i,
                                          instruction->size, value, timing);
        } else {
          // Registrador de endereço
          cycles += md_m68k_write_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, i - 8,
                                          instruction->size, value, timing);
        }

        // Atualiza o endereço
        address += (instruction->size == M68K_SIZE_LONG) ? 4 : 2;
      }
    }
  } else {
    // Registradores para memória
    for (int i = 15; i >= 0; i--) {
      if (mask & (1 << i)) {
        // Lê do registrador
        if (i < 8) {
          // Registrador de dados
          cycles += md_m68k_read_operand(M68K_ADDR_MODE_DATA_REG_DIRECT, i,
                                         instruction->size, &value, timing);
        } else {
          // Registrador de endereço
          cycles += md_m68k_read_operand(M68K_ADDR_MODE_ADDR_REG_DIRECT, i - 8,
                                         instruction->size, &value, timing);
        }

        // Escreve na memória
        cycles += md_m68k_write_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                        instruction->size, value, timing);

        // Atualiza o endereço
        address -= (instruction->size == M68K_SIZE_LONG) ? 4 : 2;
      }
    }
  }

  return cycles;
}

static int32_t execute_movep(const md_m68k_instruction_t *instruction,
                             md_m68k_timing_t *timing) {
  uint32_t cycles = 0;
  uint32_t address = 0;
  uint32_t value = 0;
  uint32_t data = 0;

  // Lê o endereço base
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_LONG, &address, timing);

  if (instruction->direction == 0) {
    // Memória para registrador
    if (instruction->size == M68K_SIZE_WORD) {
      // Word
      data = 0;
      cycles += md_m68k_read_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                     M68K_SIZE_BYTE, &value, timing);
      data = (data << 8) | (value & 0xFF);
      address += 2;
      cycles += md_m68k_read_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                     M68K_SIZE_BYTE, &value, timing);
      data = (data << 8) | (value & 0xFF);
    } else {
      // Long
      data = 0;
      for (int i = 0; i < 4; i++) {
        cycles += md_m68k_read_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                       M68K_SIZE_BYTE, &value, timing);
        data = (data << 8) | (value & 0xFF);
        address += 2;
      }
    }
    // Escreve no registrador de dados
    cycles += md_m68k_write_operand(M68K_ADDR_MODE_DATA_REG_DIRECT,
                                    instruction->src_reg, instruction->size,
                                    data, timing);
  } else {
    // Registrador para memória
    cycles += md_m68k_read_operand(M68K_ADDR_MODE_DATA_REG_DIRECT,
                                   instruction->src_reg, instruction->size,
                                   &data, timing);

    if (instruction->size == M68K_SIZE_WORD) {
      // Word
      value = (data >> 8) & 0xFF;
      cycles += md_m68k_write_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                      M68K_SIZE_BYTE, value, timing);
      address += 2;
      value = data & 0xFF;
      cycles += md_m68k_write_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                      M68K_SIZE_BYTE, value, timing);
    } else {
      // Long
      for (int i = 0; i < 4; i++) {
        value = (data >> (24 - i * 8)) & 0xFF;
        cycles += md_m68k_write_operand(M68K_ADDR_MODE_ABSOLUTE_LONG, 0,
                                        M68K_SIZE_BYTE, value, timing);
        address += 2;
      }
    }
  }

  return cycles;
}

static int32_t execute_moveq(const md_m68k_instruction_t *instruction,
                             md_m68k_timing_t *timing) {
  uint32_t cycles = 0;
  int8_t data = (int8_t)instruction->immediate;

  // Extende o sinal para 32 bits
  int32_t value = (int32_t)data;

  // Escreve no registrador de dados
  cycles += md_m68k_write_operand(M68K_ADDR_MODE_DATA_REG_DIRECT,
                                  instruction->dst_reg, M68K_SIZE_LONG,
                                  (uint32_t)value, timing);

  // Atualiza flags
  md_m68k_update_flags((uint32_t)value, M68K_SIZE_LONG, M68K_SR_N | M68K_SR_Z);
  timing->ccr &= ~(M68K_SR_V | M68K_SR_C); // Limpa V e C

  return cycles;
}

static int32_t execute_scc(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t cycles = 0;
  bool condition_met = md_m68k_evaluate_condition(instruction->condition);

  // Escreve 0xFF se a condição for verdadeira, 0x00 caso contrário
  uint32_t value = condition_met ? 0xFF : 0x00;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_BYTE, value, timing);

  return cycles;
}

static int32_t execute_dbcc(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t cycles = 0;
  uint32_t counter = 0;
  bool condition_met = md_m68k_evaluate_condition(instruction->condition);

  // Lê o contador do registrador
  cycles +=
      md_m68k_read_operand(M68K_ADDR_MODE_DATA_REG_DIRECT, instruction->dst_reg,
                           M68K_SIZE_WORD, &counter, timing);

  if (!condition_met) {
    // Decrementa o contador
    counter = (counter - 1) & 0xFFFF;

    // Escreve o novo valor do contador
    cycles += md_m68k_write_operand(M68K_ADDR_MODE_DATA_REG_DIRECT,
                                    instruction->dst_reg, M68K_SIZE_WORD,
                                    counter, timing);

    if ((int16_t)counter != -1) {
      // Realiza o desvio
      timing->pc += (int16_t)instruction->immediate;
    }
  }

  return cycles;
}

static int32_t execute_abcd(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 M68K_SIZE_BYTE, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_BYTE, &dst_value, timing);

  // Realiza a adição BCD
  uint8_t src_lo = src_value & 0x0F;
  uint8_t src_hi = (src_value >> 4) & 0x0F;
  uint8_t dst_lo = dst_value & 0x0F;
  uint8_t dst_hi = (dst_value >> 4) & 0x0F;
  uint8_t carry = (timing->ccr & M68K_SR_X) ? 1 : 0;

  // Soma os dígitos menos significativos
  uint8_t result_lo = dst_lo + src_lo + carry;
  carry = (result_lo > 9) ? 1 : 0;
  if (carry)
    result_lo -= 10;

  // Soma os dígitos mais significativos
  uint8_t result_hi = dst_hi + src_hi + carry;
  carry = (result_hi > 9) ? 1 : 0;
  if (carry)
    result_hi -= 10;

  // Combina os resultados
  uint32_t result = (result_hi << 4) | result_lo;

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_BYTE, result, timing);

  // Atualiza flags
  if (carry) {
    timing->ccr |= (M68K_SR_X | M68K_SR_C);
  } else {
    timing->ccr &= ~(M68K_SR_X | M68K_SR_C);
  }

  if (result) {
    timing->ccr &= ~M68K_SR_Z;
  }
  // Note: Z flag is cleared only if result is non-zero, otherwise unchanged

  return cycles;
}

static int32_t execute_sbcd(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t src_value = 0;
  uint32_t dst_value = 0;
  uint32_t cycles = 0;

  // Lê os operandos
  cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                 M68K_SIZE_BYTE, &src_value, timing);
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_BYTE, &dst_value, timing);

  // Realiza a subtração BCD
  uint8_t src_lo = src_value & 0x0F;
  uint8_t src_hi = (src_value >> 4) & 0x0F;
  uint8_t dst_lo = dst_value & 0x0F;
  uint8_t dst_hi = (dst_value >> 4) & 0x0F;
  uint8_t borrow = (timing->ccr & M68K_SR_X) ? 1 : 0;

  // Subtrai os dígitos menos significativos
  int16_t result_lo = dst_lo - src_lo - borrow;
  if (result_lo < 0) {
    result_lo += 10;
    borrow = 1;
  } else {
    borrow = 0;
  }

  // Subtrai os dígitos mais significativos
  int16_t result_hi = dst_hi - src_hi - borrow;
  if (result_hi < 0) {
    result_hi += 10;
    borrow = 1;
  } else {
    borrow = 0;
  }

  // Combina os resultados
  uint32_t result = ((result_hi & 0x0F) << 4) | (result_lo & 0x0F);

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_BYTE, result, timing);

  // Atualiza flags
  if (borrow) {
    timing->ccr |= (M68K_SR_X | M68K_SR_C);
  } else {
    timing->ccr &= ~(M68K_SR_X | M68K_SR_C);
  }

  if (result) {
    timing->ccr &= ~M68K_SR_Z;
  }
  // Note: Z flag is cleared only if result is non-zero, otherwise unchanged

  return cycles;
}

static int32_t execute_nbcd(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 M68K_SIZE_BYTE, &value, timing);

  // Realiza a negação BCD
  uint8_t lo = value & 0x0F;
  uint8_t hi = (value >> 4) & 0x0F;
  uint8_t borrow = (timing->ccr & M68K_SR_X) ? 1 : 0;

  // Calcula o complemento de 10
  int16_t result_lo = 0 - lo - borrow;
  if (result_lo < 0) {
    result_lo += 10;
    borrow = 1;
  } else {
    borrow = 0;
  }

  int16_t result_hi = 0 - hi - borrow;
  if (result_hi < 0) {
    result_hi += 10;
    borrow = 1;
  } else {
    borrow = 0;
  }

  // Combina os resultados
  uint32_t result = ((result_hi & 0x0F) << 4) | (result_lo & 0x0F);

  // Escreve o resultado
  cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                  M68K_SIZE_BYTE, result, timing);

  // Atualiza flags
  if (borrow) {
    timing->ccr |= (M68K_SR_X | M68K_SR_C);
  } else {
    timing->ccr &= ~(M68K_SR_X | M68K_SR_C);
  }

  if (result) {
    timing->ccr &= ~M68K_SR_Z;
  }
  // Note: Z flag is cleared only if result is non-zero, otherwise unchanged

  return cycles;
}

static int32_t execute_rol(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t count = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &value, timing);

  // Determina a contagem de rotação
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   M68K_SIZE_LONG, &count, timing);
    count &= 0x3F; // Máscara para 6 bits
  } else {
    count = instruction->immediate & 0x07; // Imediato limitado a 8
    if (count == 0)
      count = 8;
  }

  // Determina a máscara baseada no tamanho
  uint32_t mask = (instruction->size == M68K_SIZE_BYTE)   ? 0xFF
                  : (instruction->size == M68K_SIZE_WORD) ? 0xFFFF
                                                          : 0xFFFFFFFF;
  uint32_t msb = (instruction->size == M68K_SIZE_BYTE)   ? 7
                 : (instruction->size == M68K_SIZE_WORD) ? 15
                                                         : 31;

  // Realiza a rotação
  count %= (msb + 1);
  if (count > 0) {
    value &= mask;
    uint32_t result = ((value << count) | (value >> (msb + 1 - count))) & mask;

    // Escreve o resultado
    cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                    instruction->size, result, timing);

    // Atualiza flags
    timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C);
    if (result & (1 << msb))
      timing->ccr |= M68K_SR_N;
    if (result == 0)
      timing->ccr |= M68K_SR_Z;
    if (value & (1 << (count - 1)))
      timing->ccr |= M68K_SR_C;
  }

  return cycles;
}

static int32_t execute_ror(const md_m68k_instruction_t *instruction,
                           md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t count = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &value, timing);

  // Determina a contagem de rotação
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   M68K_SIZE_LONG, &count, timing);
    count &= 0x3F; // Máscara para 6 bits
  } else {
    count = instruction->immediate & 0x07; // Imediato limitado a 8
    if (count == 0)
      count = 8;
  }

  // Determina a máscara baseada no tamanho
  uint32_t mask = (instruction->size == M68K_SIZE_BYTE)   ? 0xFF
                  : (instruction->size == M68K_SIZE_WORD) ? 0xFFFF
                                                          : 0xFFFFFFFF;
  uint32_t msb = (instruction->size == M68K_SIZE_BYTE)   ? 7
                 : (instruction->size == M68K_SIZE_WORD) ? 15
                                                         : 31;

  // Realiza a rotação
  count %= (msb + 1);
  if (count > 0) {
    value &= mask;
    uint32_t result = ((value >> count) | (value << (msb + 1 - count))) & mask;

    // Escreve o resultado
    cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                    instruction->size, result, timing);

    // Atualiza flags
    timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C);
    if (result & (1 << msb))
      timing->ccr |= M68K_SR_N;
    if (result == 0)
      timing->ccr |= M68K_SR_Z;
    if (value & (1 << (count - 1)))
      timing->ccr |= M68K_SR_C;
  }

  return cycles;
}

static int32_t execute_roxl(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t count = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &value, timing);

  // Determina a contagem de rotação
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   M68K_SIZE_LONG, &count, timing);
    count &= 0x3F; // Máscara para 6 bits
  } else {
    count = instruction->immediate & 0x07; // Imediato limitado a 8
    if (count == 0)
      count = 8;
  }

  // Determina a máscara baseada no tamanho
  uint32_t mask = (instruction->size == M68K_SIZE_BYTE)   ? 0xFF
                  : (instruction->size == M68K_SIZE_WORD) ? 0xFFFF
                                                          : 0xFFFFFFFF;
  uint32_t msb = (instruction->size == M68K_SIZE_BYTE)   ? 7
                 : (instruction->size == M68K_SIZE_WORD) ? 15
                                                         : 31;

  // Realiza a rotação com extend
  count %= (msb + 2); // +2 para incluir o bit X
  if (count > 0) {
    value &= mask;
    uint32_t x = (timing->ccr & M68K_SR_X) ? 1 : 0;
    uint32_t result = value;
    uint32_t new_x = x;

    for (uint32_t i = 0; i < count; i++) {
      uint32_t old_msb = (result >> msb) & 1;
      result = ((result << 1) | new_x) & mask;
      new_x = old_msb;
    }

    // Escreve o resultado
    cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                    instruction->size, result, timing);

    // Atualiza flags
    timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C | M68K_SR_X);
    if (result & (1 << msb))
      timing->ccr |= M68K_SR_N;
    if (result == 0)
      timing->ccr |= M68K_SR_Z;
    if (new_x)
      timing->ccr |= (M68K_SR_C | M68K_SR_X);
  }

  return cycles;
}

static int32_t execute_roxr(const md_m68k_instruction_t *instruction,
                            md_m68k_timing_t *timing) {
  uint32_t value = 0;
  uint32_t count = 0;
  uint32_t cycles = 0;

  // Lê o operando
  cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg,
                                 instruction->size, &value, timing);

  // Determina a contagem de rotação
  if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
    cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg,
                                   M68K_SIZE_LONG, &count, timing);
    count &= 0x3F; // Máscara para 6 bits
  } else {
    count = instruction->immediate & 0x07; // Imediato limitado a 8
    if (count == 0)
      count = 8;
  }

  // Determina a máscara baseada no tamanho
  uint32_t mask = (instruction->size == M68K_SIZE_BYTE)   ? 0xFF
                  : (instruction->size == M68K_SIZE_WORD) ? 0xFFFF
                                                          : 0xFFFFFFFF;
  uint32_t msb = (instruction->size == M68K_SIZE_BYTE)   ? 7
                 : (instruction->size == M68K_SIZE_WORD) ? 15
                                                         : 31;

  // Realiza a rotação com extend
  count %= (msb + 2); // +2 para incluir o bit X
  if (count > 0) {
    value &= mask;
    uint32_t x = (timing->ccr & M68K_SR_X) ? 1 : 0;
    uint32_t result = value;
    uint32_t new_x = x;

    for (uint32_t i = 0; i < count; i++) {
      uint32_t old_lsb = result & 1;
      result = (result >> 1) | (new_x << msb);
      new_x = old_lsb;
    }

    // Escreve o resultado
    cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg,
                                    instruction->size, result, timing);

    // Atualiza flags
    timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C | M68K_SR_X);
    if (result & (1 << msb))
      timing->ccr |= M68K_SR_N;
    if (result == 0)
      timing->ccr |= M68K_SR_Z;
    if (new_x)
      timing->ccr |= (M68K_SR_C | M68K_SR_X);
  }

  return cycles;
}

static int32_t execute_asl(const md_m68k_instruction_t* instruction, md_m68k_timing_t* timing) {
    uint32_t value = 0;
    uint32_t count = 0;
    uint32_t cycles = 0;

    // Lê o operando
    cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, &value, timing);

    // Determina a contagem de deslocamento
    if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
        cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg, M68K_SIZE_LONG, &count, timing);
        count &= 0x3F; // Máscara para 6 bits
    } else {
        count = instruction->immediate & 0x07; // Imediato limitado a 8
        if (count == 0) count = 8;
    }

    // Determina a máscara baseada no tamanho
    uint32_t mask = (instruction->size == M68K_SIZE_BYTE) ? 0xFF :
                    (instruction->size == M68K_SIZE_WORD) ? 0xFFFF : 0xFFFFFFFF;
    uint32_t msb = (instruction->size == M68K_SIZE_BYTE) ? 7 :
                   (instruction->size == M68K_SIZE_WORD) ? 15 : 31;

    // Realiza o deslocamento aritmético à esquerda
    if (count > 0) {
        value &= mask;
        uint32_t result = 0;
        bool overflow = false;
        uint32_t last_bit = 0;

        // Verifica overflow (mudança no bit de sinal durante o deslocamento)
        int32_t sign_bit = (value >> msb) & 1;

        for (uint32_t i = 0; i < count; i++) {
            last_bit = (value >> msb) & 1;
            value = (value << 1) & mask;
            if (((value >> msb) & 1) != sign_bit) {
                overflow = true;
            }
        }

        result = value;

        // Escreve o resultado
        cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, result, timing);

        // Atualiza flags
        timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C | M68K_SR_X);
        if (result & (1 << msb)) timing->ccr |= M68K_SR_N;
        if (result == 0) timing->ccr |= M68K_SR_Z;
        if (overflow) timing->ccr |= M68K_SR_V;
        if (last_bit) timing->ccr |= (M68K_SR_C | M68K_SR_X);
    }

    return cycles;
}

static int32_t execute_asr(const md_m68k_instruction_t* instruction, md_m68k_timing_t* timing) {
    uint32_t value = 0;
    uint32_t count = 0;
    uint32_t cycles = 0;

    // Lê o operando
    cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, &value, timing);

    // Determina a contagem de deslocamento
    if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
        cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg, M68K_SIZE_LONG, &count, timing);
        count &= 0x3F; // Máscara para 6 bits
    } else {
        count = instruction->immediate & 0x07; // Imediato limitado a 8
        if (count == 0) count = 8;
    }

    // Determina a máscara baseada no tamanho
    uint32_t mask = (instruction->size == M68K_SIZE_BYTE) ? 0xFF :
                    (instruction->size == M68K_SIZE_WORD) ? 0xFFFF : 0xFFFFFFFF;
    uint32_t msb = (instruction->size == M68K_SIZE_BYTE) ? 7 :
                   (instruction->size == M68K_SIZE_WORD) ? 15 : 31;

    // Realiza o deslocamento aritmético à direita
    if (count > 0) {
        value &= mask;
        uint32_t sign_bit = value & (1 << msb);
        uint32_t result = value;
        uint32_t last_bit = 0;

        for (uint32_t i = 0; i < count; i++) {
            last_bit = result & 1;
            result = (result >> 1) | sign_bit;
        }

        result &= mask;

        // Escreve o resultado
        cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, result, timing);

        // Atualiza flags
        timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C | M68K_SR_X);
        if (result & (1 << msb)) timing->ccr |= M68K_SR_N;
        if (result == 0) timing->ccr |= M68K_SR_Z;
        if (last_bit) timing->ccr |= (M68K_SR_C | M68K_SR_X);
    }

    return cycles;
}

static int32_t execute_lsl(const md_m68k_instruction_t* instruction, md_m68k_timing_t* timing) {
    uint32_t value = 0;
    uint32_t count = 0;
    uint32_t cycles = 0;

    // Lê o operando
    cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, &value, timing);

    // Determina a contagem de deslocamento
    if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
        cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg, M68K_SIZE_LONG, &count, timing);
        count &= 0x3F; // Máscara para 6 bits
    } else {
        count = instruction->immediate & 0x07; // Imediato limitado a 8
        if (count == 0) count = 8;
    }

    // Determina a máscara baseada no tamanho
    uint32_t mask = (instruction->size == M68K_SIZE_BYTE) ? 0xFF :
                    (instruction->size == M68K_SIZE_WORD) ? 0xFFFF : 0xFFFFFFFF;
    uint32_t msb = (instruction->size == M68K_SIZE_BYTE) ? 7 :
                   (instruction->size == M68K_SIZE_WORD) ? 15 : 31;

    // Realiza o deslocamento lógico à esquerda
    if (count > 0) {
        value &= mask;
        uint32_t last_bit = 0;

        for (uint32_t i = 0; i < count; i++) {
            last_bit = (value >> msb) & 1;
            value = (value << 1) & mask;
        }

        // Escreve o resultado
        cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, value, timing);

        // Atualiza flags
        timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C | M68K_SR_X);
        if (value & (1 << msb)) timing->ccr |= M68K_SR_N;
        if (value == 0) timing->ccr |= M68K_SR_Z;
        if (last_bit) timing->ccr |= (M68K_SR_C | M68K_SR_X);
    }

    return cycles;
}

static int32_t execute_lsr(const md_m68k_instruction_t* instruction, md_m68k_timing_t* timing) {
    uint32_t value = 0;
    uint32_t count = 0;
    uint32_t cycles = 0;

    // Lê o operando
    cycles += md_m68k_read_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, &value, timing);

    // Determina a contagem de deslocamento
    if (instruction->src_mode == M68K_ADDR_MODE_DATA_REG_DIRECT) {
        cycles += md_m68k_read_operand(instruction->src_mode, instruction->src_reg, M68K_SIZE_LONG, &count, timing);
        count &= 0x3F; // Máscara para 6 bits
    } else {
        count = instruction->immediate & 0x07; // Imediato limitado a 8
        if (count == 0) count = 8;
    }

    // Determina a máscara baseada no tamanho
    uint32_t mask = (instruction->size == M68K_SIZE_BYTE) ? 0xFF :
                    (instruction->size == M68K_SIZE_WORD) ? 0xFFFF : 0xFFFFFFFF;
    uint32_t msb = (instruction->size == M68K_SIZE_BYTE) ? 7 :
                   (instruction->size == M68K_SIZE_WORD) ? 15 : 31;

    // Realiza o deslocamento lógico à direita
    if (count > 0) {
        value &= mask;
        uint32_t last_bit = 0;

        for (uint32_t i = 0; i < count; i++) {
            last_bit = value & 1;
            value = (value >> 1) & mask;
        }

        // Escreve o resultado
        cycles += md_m68k_write_operand(instruction->dst_mode, instruction->dst_reg, instruction->size, value, timing);

        // Atualiza flags
        timing->ccr &= ~(M68K_SR_N | M68K_SR_Z | M68K_SR_V | M68K_SR_C | M68K_SR_X);
        if (value & (1 << msb)) timing->ccr |= M68K_SR_N;
        if (value == 0) timing->ccr |= M68K_SR_Z;
        if (last_bit) timing->ccr |= (M68K_SR_C | M68K_SR_X);
    }

    return cycles;
}

// Atualiza a função principal de execução
int32_t md_m68k_execute_instruction(const md_m68k_instruction_t *instruction,
                                    md_m68k_timing_t *timing) {
  if (!instruction || !timing)
    return 0;

  int32_t cycles = 0;

  switch (instruction->type) {
  case M68K_INST_MOVE:
    cycles = execute_move(instruction, timing);
    break;
  case M68K_INST_ADD:
    cycles = execute_add(instruction, timing);
    break;
  case M68K_INST_SUB:
    cycles = execute_sub(instruction, timing);
    break;
  case M68K_INST_AND:
    cycles = execute_and(instruction, timing);
    break;
  case M68K_INST_OR:
    cycles = execute_or(instruction, timing);
    break;
  case M68K_INST_EOR:
    cycles = execute_eor(instruction, timing);
    break;
  case M68K_INST_NOT:
    cycles = execute_not(instruction, timing);
    break;
  case M68K_INST_NEG:
    cycles = execute_neg(instruction, timing);
    break;
  case M68K_INST_CLR:
    cycles = execute_clr(instruction, timing);
    break;
  case M68K_INST_TST:
    cycles = execute_tst(instruction, timing);
    break;
  case M68K_INST_JMP:
    cycles = execute_jmp(instruction, timing);
    break;
  case M68K_INST_JSR:
    cycles = execute_jsr(instruction, timing);
    break;
  case M68K_INST_RTS:
    cycles = execute_rts(instruction, timing);
    break;
  case M68K_INST_BRA:
    cycles = execute_bra(instruction, timing);
    break;
  case M68K_INST_BSR:
    cycles = execute_bsr(instruction, timing);
    break;
  case M68K_INST_BCC:
    cycles = execute_bcc(instruction, timing);
    break;
  case M68K_INST_BCHG:
    cycles = execute_bchg(instruction, timing);
    break;
  case M68K_INST_BCLR:
    cycles = execute_bclr(instruction, timing);
    break;
  case M68K_INST_BSET:
    cycles = execute_bset(instruction, timing);
    break;
  case M68K_INST_TAS:
    cycles = execute_tas(instruction, timing);
    break;
  case M68K_INST_SWAP:
    cycles = execute_swap(instruction, timing);
    break;
  case M68K_INST_EXT:
    cycles = execute_ext(instruction, timing);
    break;
  case M68K_INST_LINK:
    cycles = execute_link(instruction, timing);
    break;
  case M68K_INST_UNLK:
    cycles = execute_unlk(instruction, timing);
    break;
  case M68K_INST_PEA:
    cycles = execute_pea(instruction, timing);
    break;
  case M68K_INST_MULS:
    cycles = execute_muls(instruction, timing);
    break;
  case M68K_INST_MULU:
    cycles = execute_mulu(instruction, timing);
    break;
  case M68K_INST_DIVS:
    cycles = execute_divs(instruction, timing);
    break;
  case M68K_INST_DIVU:
    cycles = execute_divu(instruction, timing);
    break;
  case M68K_INST_BTST:
    cycles = execute_btst(instruction, timing);
    break;
  case M68K_INST_STOP:
    cycles = execute_stop(instruction, timing);
    break;
  case M68K_INST_TRAP:
    cycles = execute_trap(instruction, timing);
    break;
  case M68K_INST_TRAPV:
    cycles = execute_trapv(instruction, timing);
    break;
  case M68K_INST_RESET:
    cycles = execute_reset(instruction, timing);
    break;
  case M68K_INST_NOP:
    cycles = execute_nop(instruction, timing);
    break;
  case M68K_INST_ILLEGAL:
    cycles = execute_illegal(instruction, timing);
    break;
  case M68K_INST_MOVEM:
    cycles = execute_movem(instruction, timing);
    break;
  case M68K_INST_MOVEP:
    cycles = execute_movep(instruction, timing);
    break;
  case M68K_INST_MOVEQ:
    cycles = execute_moveq(instruction, timing);
    break;
  case M68K_INST_SCC:
    cycles = execute_scc(instruction, timing);
    break;
  case M68K_INST_DBCC:
    cycles = execute_dbcc(instruction, timing);
    break;
  case M68K_INST_ABCD:
    cycles = execute_abcd(instruction, timing);
    break;
  case M68K_INST_SBCD:
    cycles = execute_sbcd(instruction, timing);
    break;
  case M68K_INST_NBCD:
    cycles = execute_nbcd(instruction, timing);
    break;
  case M68K_INST_ROL:
    cycles = execute_rol(instruction, timing);
    break;
  case M68K_INST_ROR:
    cycles = execute_ror(instruction, timing);
    break;
  case M68K_INST_ROXL:
    cycles = execute_roxl(instruction, timing);
    break;
  case M68K_INST_ROXR:
    cycles = execute_roxr(instruction, timing);
    break;
  case M68K_INST_ASL:
    cycles = execute_asl(instruction, timing);
    break;
  case M68K_INST_ASR:
    cycles = execute_asr(instruction, timing);
    break;
  case M68K_INST_LSL:
    cycles = execute_lsl(instruction, timing);
    break;
  case M68K_INST_LSR:
    cycles = execute_lsr(instruction, timing);
    break;
  default:
    // Instrução não implementada ou inválida
    cycles = 4; // Ciclos padrão para instruções não implementadas
    break;
  }

  return cycles + instruction->timing.base_cycles;
}

int32_t md_m68k_read_operand(md_m68k_addr_mode_t mode, uint8_t reg,
                             md_m68k_size_t size, uint32_t *value,
                             md_m68k_timing_t *timing) {
  if (!value || !timing)
    return 0;

  uint32_t cycles = 0;

  // Calcula ciclos do modo de endereçamento
  cycles += g_ea_cycles[mode];

  // Adiciona ciclos de acesso à memória se necessário
  if (mode != M68K_ADDR_MODE_DATA_REG_DIRECT &&
      mode != M68K_ADDR_MODE_ADDR_REG_DIRECT) {
    cycles += g_mem_cycles[size];
  }

  // Atualiza o timing
  md_m68k_add_cycles(timing, cycles);

  // TODO: Implementar leitura do operando

  return cycles;
}

int32_t md_m68k_write_operand(md_m68k_addr_mode_t mode, uint8_t reg,
                              md_m68k_size_t size, uint32_t value,
                              md_m68k_timing_t *timing) {
  if (!timing)
    return 0;

  uint32_t cycles = 0;

  // Calcula ciclos do modo de endereçamento
  cycles += g_ea_cycles[mode];

  // Adiciona ciclos de acesso à memória se necessário
  if (mode != M68K_ADDR_MODE_DATA_REG_DIRECT &&
      mode != M68K_ADDR_MODE_ADDR_REG_DIRECT) {
    cycles += g_mem_cycles[size];
  }

  // Atualiza o timing
  md_m68k_add_cycles(timing, cycles);

  // TODO: Implementar escrita do operando

  return cycles;
}

void md_m68k_update_flags(uint32_t result, md_m68k_size_t size,
                          uint16_t update_mask) {
  uint16_t ccr = md_m68k_get_sr() & 0xFF;
  uint32_t mask;

  // Determina a máscara com base no tamanho
  switch (size) {
  case M68K_SIZE_BYTE:
    mask = 0xFF;
    break;
  case M68K_SIZE_WORD:
    mask = 0xFFFF;
    break;
  case M68K_SIZE_LONG:
    mask = 0xFFFFFFFF;
    break;
  default:
    return;
  }

  // Atualiza cada flag se necessário
  if (update_mask & 0x01) { // N - Negative
    if (result & ((mask >> 1) + 1)) {
      ccr |= 0x08;
    } else {
      ccr &= ~0x08;
    }
  }

  if (update_mask & 0x02) { // Z - Zero
    if ((result & mask) == 0) {
      ccr |= 0x04;
    } else {
      ccr &= ~0x04;
    }
  }

  if (update_mask & 0x04) { // V - Overflow
    // Normalmente calculado durante a instrução específica
    ccr &= ~0x02;
  }

  if (update_mask & 0x08) { // C - Carry
    // Normalmente calculado durante a instrução específica
    ccr &= ~0x01;
  }

  // Atualiza o registrador de status
  md_m68k_set_sr((md_m68k_get_sr() & 0xFF00) | ccr);
}

int32_t md_m68k_evaluate_condition(uint8_t condition) {
  uint16_t sr = md_m68k_get_sr();
  bool n = (sr & 0x08) != 0; // Negative
  bool z = (sr & 0x04) != 0; // Zero
  bool v = (sr & 0x02) != 0; // Overflow
  bool c = (sr & 0x01) != 0; // Carry

  switch (condition) {
  case 0x0:
    return true; // T  - Always true
  case 0x1:
    return false; // F  - Always false
  case 0x2:
    return !c && !z; // HI - Higher
  case 0x3:
    return c || z; // LS - Lower or same
  case 0x4:
    return !c; // CC - Carry clear
  case 0x5:
    return c; // CS - Carry set
  case 0x6:
    return !z; // NE - Not equal
  case 0x7:
    return z; // EQ - Equal
  case 0x8:
    return !v; // VC - Overflow clear
  case 0x9:
    return v; // VS - Overflow set
  case 0xA:
    return !n; // PL - Plus
  case 0xB:
    return n; // MI - Minus
  case 0xC:
    return n == v; // GE - Greater or equal
  case 0xD:
    return n != v; // LT - Less than
  case 0xE:
    return !z && (n == v); // GT - Greater than
  case 0xF:
    return z || (n != v); // LE - Less or equal
  default:
    return false;
  }
}

uint32_t md_m68k_calculate_ea_timing(md_m68k_addr_mode_t mode, uint8_t reg,
                                     bool is_read, md_m68k_timing_t *timing) {
  if (!timing)
    return 0;

  uint32_t cycles = g_ea_cycles[mode];

  // Adiciona ciclos extras para escrita
  if (!is_read) {
    cycles += 2;
  }

  return cycles;
}

uint32_t
md_m68k_calculate_instruction_timing(const md_m68k_instruction_t *instruction) {
  if (!instruction)
    return 0;

  uint32_t total_cycles = instruction->timing.base_cycles;

  // Adiciona ciclos de EA
  total_cycles += instruction->timing.ea_cycles;

  // Adiciona ciclos de memória
  total_cycles += instruction->timing.mem_cycles;

  // Adiciona ciclos de branch
  if (instruction->execution.changes_pc) {
    if (instruction->type >= M68K_INST_BCC &&
        instruction->type <= M68K_INST_BSR) {
      bool condition_true = md_m68k_evaluate_condition(instruction->condition);
      if (condition_true) {
        total_cycles += (instruction->displacement > 127 ||
                         instruction->displacement < -128)
                            ? g_branch_cycles[1]
                            : g_branch_cycles[0];
      } else {
        total_cycles += g_branch_cycles[2];
      }
    }
  }

  return total_cycles;
}
