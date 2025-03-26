#include "m68k_timing.h"
#include "m68k.h"
#include <string.h>

// Tabela de ciclos para acessos à memória por região
static const uint8_t memory_timing_table[8] = {
    4, // ROM (0x000000-0x3FFFFF)
    2, // RAM (0xFF0000-0xFFFFFF)
    5, // VDP (0xC00000-0xC0001F)
    3, // Z80 (0xA00000-0xA0FFFF)
    4, // Cartridge Expansion (0x400000-0x7FFFFF)
    4, // Reserved (0x800000-0x9FFFFF)
    5, // I/O and Control (0xA10000-0xBFFFFF)
    4  // Reserved (0xC00020-0xFEFFFF)
};

void md_m68k_init_timing(md_m68k_timing_t *timing) {
  if (!timing)
    return;

  memset(timing, 0, sizeof(md_m68k_timing_t));
  timing->wait_states = 0;
  timing->prefetch_queue = 0;
  timing->is_halted = false;
  timing->target_cycles = 0;
  timing->sync.last_sync_cycle = 0;
  timing->sync.z80_sync_pending = 0;
  timing->sync.vdp_sync_pending = 0;
}

void md_m68k_reset_timing(md_m68k_timing_t *timing) {
  if (!timing)
    return;

  timing->current_cycles = 0;
  timing->target_cycles = 0;
  timing->wait_states = 0;
  timing->prefetch_queue = 0;
  timing->is_halted = false;
  timing->sync.last_sync_cycle = 0;
  timing->sync.z80_sync_pending = 0;
  timing->sync.vdp_sync_pending = 0;
  md_m68k_reset_timing_stats(timing);
}

uint8_t md_m68k_calculate_ea_timing(uint8_t mode, uint8_t reg, bool is_read) {
  uint8_t cycles;

  switch (mode) {
  case 0: // Data Register Direct
  case 1: // Address Register Direct
    cycles = 0;
    break;

  case 2: // Address Register Indirect
    cycles = M68K_EA_ADDR_INDIRECT;
    break;

  case 3: // Address Register Indirect with Post-increment
    cycles = M68K_EA_POSTINC;
    break;

  case 4: // Address Register Indirect with Pre-decrement
    cycles = M68K_EA_PREDEC;
    break;

  case 5: // Address Register Indirect with Displacement
    cycles = M68K_EA_DISP16;
    break;

  case 6: // Address Register Indirect with Index
    cycles = M68K_EA_INDEX;
    break;

  case 7: // Extended
    switch (reg) {
    case 0: // Absolute Short
      cycles = M68K_EA_ABS_SHORT;
      break;

    case 1: // Absolute Long
      cycles = M68K_EA_ABS_LONG;
      break;

    case 2: // Program Counter with Displacement
      cycles = M68K_EA_PC_DISP;
      break;

    case 3: // Program Counter with Index
      cycles = M68K_EA_PC_INDEX;
      break;

    case 4: // Immediate
      cycles = M68K_EA_IMMEDIATE;
      break;

    default:
      cycles = 4; // Valor padrão conservador
      break;
    }
    break;

  default:
    cycles = 4; // Valor padrão conservador
    break;
  }

  return cycles + (is_read ? 0 : 2); // Escrita leva 2 ciclos extras
}

uint8_t md_m68k_calculate_memory_timing(uint32_t address, bool is_write) {
  uint8_t region = (address >> 21) & 0x7;
  uint8_t base_cycles = memory_timing_table[region];

  // Adiciona wait states e ciclos extras para escrita
  return base_cycles + (is_write ? 2 : 0);
}

uint32_t md_m68k_get_instruction_timing(uint16_t opcode, uint8_t ea_mode,
                                        uint8_t ea_reg) {
  uint32_t cycles;
  uint8_t op_type = (opcode >> 12) & 0xF;

  switch (op_type) {
  case 0x1: // MOVE.B
    cycles = M68K_MOVE_BYTE_CYCLES;
    break;

  case 0x2: // MOVE.L
  case 0x3: // MOVE.W
    cycles = (op_type == 0x2) ? M68K_MOVE_LONG_CYCLES : M68K_MOVE_WORD_CYCLES;
    break;

  case 0x4:                            // Miscellaneous
  case 0x5:                            // ADDQ/SUBQ/Scc/DBcc
  case 0x6:                            // Bcc/BSR/BRA
    if ((opcode & 0xFF00) == 0x6000) { // BRA/BSR/Bcc
      cycles = ((opcode & 0xFF) == 0) ? M68K_BRANCH_NOT_TAKEN_CYCLES
                                      : M68K_BRANCH_TAKEN_CYCLES;
    } else {
      cycles = M68K_ALU_REG_CYCLES;
    }
    break;

  case 0x7: // MOVEQ
    cycles = 4;
    break;

  case 0x8:                            // OR/DIV/SBCD
  case 0x9:                            // SUB/SUBX
  case 0xB:                            // CMP/EOR
  case 0xC:                            // AND/MUL/ABCD
  case 0xD:                            // ADD/ADDX
    if ((opcode & 0x01C0) == 0x01C0) { // MUL/DIV
      if (opcode & 0x0100) {
        cycles = (opcode & 0x0080) ? M68K_DIVS_CYCLES : M68K_DIVU_CYCLES;
      } else {
        cycles = (opcode & 0x0080) ? M68K_MULS_CYCLES : M68K_MULU_CYCLES;
      }
    } else {
      cycles = (ea_mode == 0) ? M68K_ALU_REG_CYCLES : M68K_ALU_MEM_CYCLES;
    }
    break;

  case 0xE: // Shift/Rotate
    cycles = (ea_mode == 0) ? 6 : 8;
    break;

  default:
    cycles = 4; // Valor padrão conservador
    break;
  }

  // Adiciona ciclos do modo de endereçamento
  cycles += md_m68k_calculate_ea_timing(ea_mode, ea_reg, true);

  return cycles;
}

void md_m68k_add_cycles(md_m68k_timing_t *timing, uint32_t cycles) {
  if (!timing)
    return;

  timing->current_cycles += cycles;
  timing->stats.instruction_cycles += cycles;
  timing->stats.total_instructions++;
}

void md_m68k_sync_cycles(md_m68k_timing_t *timing) {
  if (!timing)
    return;

  if (timing->current_cycles >= timing->target_cycles) {
    timing->sync.last_sync_cycle = timing->current_cycles;
    timing->current_cycles = 0;
    timing->target_cycles = 0;
  }
}

void md_m68k_sync_with_z80(md_m68k_timing_t *timing) {
  if (!timing)
    return;

  if (timing->sync.z80_sync_pending) {
    timing->stats.wait_cycles += 3; // Espera típica para sincronização
    timing->current_cycles += 3;
    timing->sync.z80_sync_pending = 0;
  }
}

void md_m68k_wait_for_vdp(md_m68k_timing_t *timing) {
  if (!timing)
    return;

  if (timing->sync.vdp_sync_pending) {
    timing->stats.wait_cycles += 4; // Espera típica para acesso VDP
    timing->current_cycles += 4;
    timing->sync.vdp_sync_pending = 0;
  }
}

void md_m68k_get_timing_stats(const md_m68k_timing_t *timing,
                              uint32_t *instruction_cycles,
                              uint32_t *memory_cycles, uint32_t *wait_cycles,
                              uint32_t *total_instructions) {
  if (!timing)
    return;

  if (instruction_cycles)
    *instruction_cycles = timing->stats.instruction_cycles;
  if (memory_cycles)
    *memory_cycles = timing->stats.memory_cycles;
  if (wait_cycles)
    *wait_cycles = timing->stats.wait_cycles;
  if (total_instructions)
    *total_instructions = timing->stats.total_instructions;
}

void md_m68k_reset_timing_stats(md_m68k_timing_t *timing) {
  if (!timing)
    return;

  timing->stats.instruction_cycles = 0;
  timing->stats.memory_cycles = 0;
  timing->stats.wait_cycles = 0;
  timing->stats.total_instructions = 0;
}

void md_m68k_set_wait_states(md_m68k_timing_t *timing, uint8_t states) {
  if (!timing)
    return;
  timing->wait_states = states;
}

void md_m68k_request_z80_sync(md_m68k_timing_t *timing) {
  if (!timing)
    return;
  timing->sync.z80_sync_pending = 1;
}

void md_m68k_request_vdp_sync(md_m68k_timing_t *timing) {
  if (!timing)
    return;
  timing->sync.vdp_sync_pending = 1;
}

bool md_m68k_is_sync_pending(const md_m68k_timing_t *timing) {
  if (!timing)
    return false;
  return timing->sync.z80_sync_pending || timing->sync.vdp_sync_pending;
}

void md_m68k_add_memory_cycles(md_m68k_timing_t *timing, uint32_t address,
                               bool is_write) {
  if (!timing)
    return;

  uint8_t cycles = md_m68k_calculate_memory_timing(address, is_write);
  timing->current_cycles += cycles;
  timing->stats.memory_cycles += cycles;
}

bool md_m68k_should_sync(const md_m68k_timing_t *timing) {
  if (!timing)
    return false;
  return timing->current_cycles >= timing->target_cycles ||
         md_m68k_is_sync_pending(timing);
}
