/**
 * @file m68k_disasm.h
 * @brief Interface de desassemblagem para o processador Motorola 68000 (M68K)
 */

#ifndef MEGA_EMU_M68K_DISASM_H
#define MEGA_EMU_M68K_DISASM_H

#include <stdint.h>
#include <stdbool.h>
#include "m68k.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Tamanho máximo para uma string de instrução desassemblada
 */
#define M68K_DISASM_BUF_SIZE 128

    /**
     * @brief Desassembla uma única instrução no endereço especificado
     * @param cpu Ponteiro para a instância do M68K
     * @param address Endereço da instrução a desassemblar
     * @param buffer Buffer para armazenar a string desassemblada
     * @param size Tamanho do buffer
     * @return Tamanho da instrução em bytes (2, 4, 6, 8)
     */
    int m68k_disassemble(m68k_t *cpu, uint32_t address, char *buffer, int size);

    /**
     * @brief Desassembla uma instrução a partir de um opcode específico
     * @param cpu Ponteiro para a instância do M68K (pode ser NULL se não precisar acessar memória)
     * @param opcode Opcode da instrução
     * @param address Endereço da instrução (usado para instruções relativas ao PC)
     * @param buffer Buffer para armazenar a string desassemblada
     * @param size Tamanho do buffer
     * @return Tamanho da instrução em bytes (2, 4, 6, 8)
     */
    int m68k_disassemble_opcode(m68k_t *cpu, uint16_t opcode, uint32_t address, char *buffer, int size);

    /**
     * @brief Obtém o nome do registrador
     * @param reg Índice do registrador
     * @return String com o nome do registrador
     */
    const char *m68k_get_register_name(int reg);

    /**
     * @brief Obtém o nome da condição
     * @param condition Índice da condição (0-15)
     * @return String com o nome da condição
     */
    const char *m68k_get_condition_name(int condition);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_M68K_DISASM_H */
