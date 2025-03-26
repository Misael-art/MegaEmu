/**
 * @file z80_instructions.c
 * @brief Implementação das instruções do Z80
 */

#include "z80_instructions.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o Z80
#define EMU_LOG_CAT_Z80 EMU_LOG_CAT_CPU

// Macros de log específicas para o Z80
#define Z80_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_Z80, __VA_ARGS__)
#define Z80_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_Z80, __VA_ARGS__)
#define Z80_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_Z80, __VA_ARGS__)
#define Z80_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_Z80, __VA_ARGS__)

// Declarações forward das funções handler
static void z80_nop(z80_cpu_t* cpu);
static void z80_halt(z80_cpu_t* cpu);
static void z80_di(z80_cpu_t* cpu);
static void z80_ei(z80_cpu_t* cpu);

// Tabelas de instruções (apenas algumas iniciais)
static z80_instruction_t z80_base_instructions[256] = {
    // 0x00: NOP
    {0x00, "NOP", 1, 4, 0, Z80_ADDR_MODE_NONE, Z80_ADDR_MODE_NONE, z80_nop},
    // 0x76: HALT
    {0x76, "HALT", 1, 4, 0, Z80_ADDR_MODE_NONE, Z80_ADDR_MODE_NONE, z80_halt},
    // 0xF3: DI
    {0xF3, "DI", 1, 4, 0, Z80_ADDR_MODE_NONE, Z80_ADDR_MODE_NONE, z80_di},
    // 0xFB: EI
    {0xFB, "EI", 1, 4, 0, Z80_ADDR_MODE_NONE, Z80_ADDR_MODE_NONE, z80_ei}
};

// Demais tabelas (CB, ED, DD, FD, DDCB, FDCB) a serem implementadas
static z80_instruction_t z80_cb_instructions[256];
static z80_instruction_t z80_ed_instructions[256];
static z80_instruction_t z80_dd_instructions[256];
static z80_instruction_t z80_fd_instructions[256];
static z80_instruction_t z80_ddcb_instructions[256];
static z80_instruction_t z80_fdcb_instructions[256];

/**
 * @brief Inicializa as tabelas de instruções
 */
bool z80_instructions_init(void) {
    // Inicializar tabela base (já tem algumas instruções definidas)

    // Inicializar outras tabelas (por enquanto vazias)
    memset(z80_cb_instructions, 0, sizeof(z80_cb_instructions));
    memset(z80_ed_instructions, 0, sizeof(z80_ed_instructions));
    memset(z80_dd_instructions, 0, sizeof(z80_dd_instructions));
    memset(z80_fd_instructions, 0, sizeof(z80_fd_instructions));
    memset(z80_ddcb_instructions, 0, sizeof(z80_ddcb_instructions));
    memset(z80_fdcb_instructions, 0, sizeof(z80_fdcb_instructions));

    return true;
}

/**
 * @brief Libera recursos das tabelas de instruções
 */
void z80_instructions_shutdown(void) {
    // Nada para liberar por enquanto, talvez no futuro
}

/**
 * @brief Busca a instrução correspondente a um opcode
 */
const z80_instruction_t* z80_find_instruction(
    uint8_t opcode,
    bool prefix_cb,
    bool prefix_ed,
    bool prefix_dd,
    bool prefix_fd
) {
    if (prefix_cb) {
        return z80_get_cb_instruction(opcode);
    } else if (prefix_ed) {
        return z80_get_ed_instruction(opcode);
    } else if (prefix_dd) {
        return z80_get_dd_instruction(opcode);
    } else if (prefix_fd) {
        return z80_get_fd_instruction(opcode);
    } else {
        return z80_get_base_instruction(opcode);
    }
}

/**
 * @brief Obtém uma instrução da tabela de instruções base
 */
const z80_instruction_t* z80_get_base_instruction(uint8_t opcode) {
    return &z80_base_instructions[opcode];
}

/**
 * @brief Obtém uma instrução da tabela de instruções CB
 */
const z80_instruction_t* z80_get_cb_instruction(uint8_t opcode) {
    return &z80_cb_instructions[opcode];
}

/**
 * @brief Obtém uma instrução da tabela de instruções ED
 */
const z80_instruction_t* z80_get_ed_instruction(uint8_t opcode) {
    return &z80_ed_instructions[opcode];
}

/**
 * @brief Obtém uma instrução da tabela de instruções DD
 */
const z80_instruction_t* z80_get_dd_instruction(uint8_t opcode) {
    return &z80_dd_instructions[opcode];
}

/**
 * @brief Obtém uma instrução da tabela de instruções FD
 */
const z80_instruction_t* z80_get_fd_instruction(uint8_t opcode) {
    return &z80_fd_instructions[opcode];
}

/**
 * @brief Obtém uma instrução da tabela de instruções DDCB
 */
const z80_instruction_t* z80_get_ddcb_instruction(uint8_t opcode) {
    return &z80_ddcb_instructions[opcode];
}

/**
 * @brief Obtém uma instrução da tabela de instruções FDCB
 */
const z80_instruction_t* z80_get_fdcb_instruction(uint8_t opcode) {
    return &z80_fdcb_instructions[opcode];
}

// Implementações das funções handler

/**
 * @brief Implementação da instrução NOP
 */
static void z80_nop(z80_cpu_t* cpu) {
    // Não faz nada
}

/**
 * @brief Implementação da instrução HALT
 */
static void z80_halt(z80_cpu_t* cpu) {
    cpu->halt = true;
}

/**
 * @brief Implementação da instrução DI
 */
static void z80_di(z80_cpu_t* cpu) {
    cpu->iff1 = false;
    cpu->iff2 = false;
}

/**
 * @brief Implementação da instrução EI
 */
static void z80_ei(z80_cpu_t* cpu) {
    cpu->iff1 = true;
    cpu->iff2 = true;
}
