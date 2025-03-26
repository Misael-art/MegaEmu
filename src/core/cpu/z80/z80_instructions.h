/**
 * @file z80_instructions.h
 * @brief Definições para as instruções do Z80
 *
 * Este arquivo contém as definições para instruções e tabelas
 * de decodificação do processador Z80.
 */

#ifndef Z80_INSTRUCTIONS_H
#define Z80_INSTRUCTIONS_H

#include <stdint.h>
#include "z80.h"

/**
 * @brief Identificadores para os registradores do Z80
 */
typedef enum {
    Z80_REG_A = 0,
    Z80_REG_F,
    Z80_REG_B,
    Z80_REG_C,
    Z80_REG_D,
    Z80_REG_E,
    Z80_REG_H,
    Z80_REG_L,
    Z80_REG_AF,
    Z80_REG_BC,
    Z80_REG_DE,
    Z80_REG_HL,
    Z80_REG_IX,
    Z80_REG_IY,
    Z80_REG_SP,
    Z80_REG_PC,
    Z80_REG_I,
    Z80_REG_R,
    Z80_REG_A_ALT,
    Z80_REG_F_ALT,
    Z80_REG_B_ALT,
    Z80_REG_C_ALT,
    Z80_REG_D_ALT,
    Z80_REG_E_ALT,
    Z80_REG_H_ALT,
    Z80_REG_L_ALT,
    Z80_REG_AF_ALT,
    Z80_REG_BC_ALT,
    Z80_REG_DE_ALT,
    Z80_REG_HL_ALT
} z80_register_id;

/**
 * @brief Identificadores para as flags do Z80 (no registrador F)
 */
typedef enum {
    Z80_FLAG_C = 0,  // Carry (bit 0)
    Z80_FLAG_N = 1,  // Add/Subtract (bit 1)
    Z80_FLAG_PV = 2, // Parity/Overflow (bit 2)
    Z80_FLAG_F3 = 3, // Undocumented (bit 3)
    Z80_FLAG_H = 4,  // Half Carry (bit 4)
    Z80_FLAG_F5 = 5, // Undocumented (bit 5)
    Z80_FLAG_Z = 6,  // Zero (bit 6)
    Z80_FLAG_S = 7   // Sign (bit 7)
} z80_flag_id;

/**
 * @brief Modos de endereçamento do Z80
 */
typedef enum {
    Z80_ADDR_MODE_NONE,
    Z80_ADDR_MODE_REG,       // Registrador
    Z80_ADDR_MODE_REG_PAIR,  // Par de registradores
    Z80_ADDR_MODE_IMM,       // Imediato (8-bit)
    Z80_ADDR_MODE_IMM16,     // Imediato (16-bit)
    Z80_ADDR_MODE_IND_REG,   // Indireto via registrador
    Z80_ADDR_MODE_IND_IMM,   // Indireto via imediato
    Z80_ADDR_MODE_IDX_IX,    // Indexado via IX
    Z80_ADDR_MODE_IDX_IY,    // Indexado via IY
    Z80_ADDR_MODE_BIT        // Operações de bit
} z80_addressing_mode;

/**
 * @brief Estrutura que representa uma instrução Z80
 */
typedef struct {
    uint8_t opcode;          // Código da operação
    const char* mnemonic;    // Mnemônico para debug
    uint8_t length;          // Tamanho em bytes
    uint8_t cycles;          // Ciclos base
    uint8_t alt_cycles;      // Ciclos alternativos
    z80_addressing_mode src_mode;    // Modo de endereçamento da fonte
    z80_addressing_mode dst_mode;    // Modo de endereçamento do destino
    void (*handler)(z80_cpu_t*);     // Função que implementa a instrução
} z80_instruction_t;

/**
 * @brief Inicializa as tabelas de instruções do Z80
 *
 * @return true se inicializado com sucesso, false caso contrário
 */
bool z80_instructions_init(void);

/**
 * @brief Executa uma instrução do Z80
 *
 * @param cpu Ponteiro para a instância do Z80
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t z80_execute_instruction(z80_t *cpu);

/**
 * @brief Executa uma instrução prefixada com CB
 *
 * @param cpu Ponteiro para a instância do Z80
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t z80_execute_cb_instruction(z80_t *cpu);

/**
 * @brief Executa uma instrução prefixada com DD (IX)
 *
 * @param cpu Ponteiro para a instância do Z80
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t z80_execute_dd_instruction(z80_t *cpu);

/**
 * @brief Executa uma instrução prefixada com FD (IY)
 *
 * @param cpu Ponteiro para a instância do Z80
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t z80_execute_fd_instruction(z80_t *cpu);

/**
 * @brief Executa uma instrução prefixada com ED
 *
 * @param cpu Ponteiro para a instância do Z80
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t z80_execute_ed_instruction(z80_t *cpu);

/**
 * @brief Executa uma instrução prefixada com DD CB (IX bit)
 *
 * @param cpu Ponteiro para a instância do Z80
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t z80_execute_ddcb_instruction(z80_t *cpu);

/**
 * @brief Executa uma instrução prefixada com FD CB (IY bit)
 *
 * @param cpu Ponteiro para a instância do Z80
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t z80_execute_fdcb_instruction(z80_t *cpu);

#endif /* Z80_INSTRUCTIONS_H */
