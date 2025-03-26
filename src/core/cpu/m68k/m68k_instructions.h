/**
 * @file m68k_instructions.h
 * @brief Declarações de funções de instruções para o processador Motorola 68000 (M68K)
 */

#ifndef MEGA_EMU_M68K_INSTRUCTIONS_H
#define MEGA_EMU_M68K_INSTRUCTIONS_H

#include "m68k.h"
#include "m68k_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* Handlers para grupos de instruções */

    /**
     * @brief Handler para instrução ilegal
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_illegal_instruction(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief Handler para instrução não implementada
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_unimplemented_instruction(m68k_t *cpu, uint16_t opcode);

    /* Instruções de movimentação de dados */

    /**
     * @brief MOVE - Move data
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_move(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief MOVEA - Move address
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_movea(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief MOVEQ - Move quick
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_moveq(m68k_t *cpu, uint16_t opcode);

    /* Instruções aritméticas */

    /**
     * @brief ADD - Add
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_add(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief ADDA - Add address
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_adda(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief ADDQ - Add quick
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_addq(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief SUB - Subtract
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_sub(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief SUBA - Subtract address
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_suba(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief SUBQ - Subtract quick
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_subq(m68k_t *cpu, uint16_t opcode);

    /* Instruções lógicas */

    /**
     * @brief AND - Logical AND
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_and(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief OR - Logical OR
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_or(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief EOR - Logical Exclusive OR
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_eor(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief NOT - Logical complement
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_not(m68k_t *cpu, uint16_t opcode);

    /* Instruções de deslocamento e rotação */

    /**
     * @brief LSL - Logical shift left
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_lsl(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief LSR - Logical shift right
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_lsr(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief ASL - Arithmetic shift left
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_asl(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief ASR - Arithmetic shift right
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_asr(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief ROL - Rotate left
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_rol(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief ROR - Rotate right
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_ror(m68k_t *cpu, uint16_t opcode);

    /* Instruções de controle */

    /**
     * @brief JMP - Jump
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_jmp(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief JSR - Jump to subroutine
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_jsr(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief RTS - Return from subroutine
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_rts(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief BCC - Branch conditionally
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_bcc(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief DBCC - Decrement and branch conditionally
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_dbcc(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief RTE - Return from exception
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_rte(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief STOP - Stop program execution
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_stop(m68k_t *cpu, uint16_t opcode);

    /* Instruções de manipulação de sistema */

    /**
     * @brief RESET - Reset external devices
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_reset_instruction(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief TRAP - Trap
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_trap(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief TRAPV - Trap on overflow
     * @param cpu Ponteiro para a instância do M68K
     * @param opcode Opcode da instrução
     * @return Número de ciclos consumidos
     */
    int m68k_trapv(m68k_t *cpu, uint16_t opcode);

    /* Utilitários para instruções */

    /**
     * @brief Atualiza flags para operações lógicas
     * @param cpu Ponteiro para a instância do M68K
     * @param result Resultado da operação
     * @param size Tamanho do operando (1=byte, 2=word, 4=long)
     */
    void m68k_update_logic_flags(m68k_t *cpu, uint32_t result, int size);

    /**
     * @brief Atualiza flags para operações aritméticas de adição
     * @param cpu Ponteiro para a instância do M68K
     * @param src Operando fonte
     * @param dst Operando destino
     * @param result Resultado da operação
     * @param size Tamanho do operando (1=byte, 2=word, 4=long)
     */
    void m68k_update_add_flags(m68k_t *cpu, uint32_t src, uint32_t dst, uint32_t result, int size);

    /**
     * @brief Atualiza flags para operações aritméticas de subtração
     * @param cpu Ponteiro para a instância do M68K
     * @param src Operando fonte
     * @param dst Operando destino
     * @param result Resultado da operação
     * @param size Tamanho do operando (1=byte, 2=word, 4=long)
     */
    void m68k_update_sub_flags(m68k_t *cpu, uint32_t src, uint32_t dst, uint32_t result, int size);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_M68K_INSTRUCTIONS_H */
