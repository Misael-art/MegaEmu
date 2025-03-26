/**
 * @file m68k_execute.h
 * @brief Definições para execução de instruções do M68000
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef M68K_EXECUTE_H
#define M68K_EXECUTE_H

#include "m68k_adapter.h"

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
  uint32_t src_addr;  // Endereço fonte (se aplicável)
  uint32_t dst_addr;  // Endereço destino (se aplicável)
  uint8_t cycles;     // Ciclos base da instrução
} m68k_instruction_t;

// Função principal de execução
int m68k_execute_cycles(megadrive_m68k_context_t *ctx, int target_cycles);

// Funções de execução de instruções aritméticas
void m68k_execute_add(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_addq(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_addx(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_sub(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_subq(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_subx(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_mulu(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_muls(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_divu(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_divs(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);

// Funções auxiliares de acesso à memória
uint16_t fetch_word(megadrive_m68k_context_t *ctx);
uint32_t fetch_long(megadrive_m68k_context_t *ctx);
uint32_t read_long(megadrive_m68k_context_t *ctx, uint32_t addr);
void write_long(megadrive_m68k_context_t *ctx, uint32_t addr, uint32_t value);
void write_value(megadrive_m68k_context_t *ctx, uint32_t addr, uint32_t value,
                 uint8_t size);

// Funções de instruções lógicas
void m68k_execute_and(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_or(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_eor(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_not(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_neg(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_clr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_tst(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);

// Funções de deslocamento e rotação
void m68k_execute_asl(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_asr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_lsl(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_lsr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_rol(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_ror(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);

// Funções de instruções de controle
void m68k_execute_bcc(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_bra(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_bsr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_jmp(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_jsr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_rts(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_rte(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_trap(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_dbcc(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_nop(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_reset(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst);
void m68k_execute_stop(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_illegal(megadrive_m68k_context_t *ctx,
                          m68k_instruction_t *inst);

// Funções de instruções de comparação
void m68k_execute_cmp(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_cmpa(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_cmpi(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_cmpm(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);

// Funções de instruções de movimentação
void m68k_execute_move(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_movea(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst);
void m68k_execute_moveq(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst);
void m68k_execute_movem(megadrive_m68k_context_t *ctx,
                        m68k_instruction_t *inst);
void m68k_execute_lea(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_pea(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);

// Funções de instruções de manipulação de bits
void m68k_execute_btst(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_bchg(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_bclr(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);
void m68k_execute_bset(megadrive_m68k_context_t *ctx, m68k_instruction_t *inst);

#endif // M68K_EXECUTE_H
