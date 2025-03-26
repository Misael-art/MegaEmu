/**
 * @file m68k_instructions.c
 * @brief Implementação das instruções do processador Motorola 68000 (M68K)
 */

#include <stdlib.h>
#include <string.h>
#include "m68k.h"
#include "m68k_internal.h"
#include "m68k_instructions.h"
#include <stdio.h>

/* Macros auxiliares */
#define GET_EA_MODE(op) (((op) >> 3) & 0x7)
#define GET_EA_REG(op) ((op) & 0x7)
#define GET_CONDITION(op) (((op) >> 8) & 0xF)

/* Tabelas de instruções */
static m68k_instruction_handler g_instruction_table[65536];

/* Rotinas auxiliares para cálculo de operandos e flags */
static uint32_t m68k_get_ea_value(m68k_t *ctx, uint8_t mode, uint8_t reg, uint8_t size)
{
    uint32_t ea = 0;
    uint32_t value = 0;

    switch (mode)
    {
    case 0: // Dn - Registro de dados
        value = ctx->regs.d[reg];
        break;

    case 1: // An - Registro de endereço
        value = ctx->regs.a[reg];
        break;

    case 2: // (An) - Indireto
        ea = ctx->regs.a[reg];
        value = m68k_read_memory(ctx, ea, size);
        break;

    case 3: // (An)+ - Indireto com pós-incremento
        ea = ctx->regs.a[reg];
        value = m68k_read_memory(ctx, ea, size);
        ctx->regs.a[reg] += (reg == 7 && size == 1) ? 2 : size;
        break;

    case 4: // -(An) - Indireto com pré-decremento
        ctx->regs.a[reg] -= (reg == 7 && size == 1) ? 2 : size;
        ea = ctx->regs.a[reg];
        value = m68k_read_memory(ctx, ea, size);
        break;

    case 5: // d16(An) - Indireto com deslocamento
    {
        int16_t disp = (int16_t)m68k_fetch_word(ctx);
        ea = ctx->regs.a[reg] + disp;
        value = m68k_read_memory(ctx, ea, size);
    }
    break;

    case 7: // Modos de endereçamento estendidos
        if (reg == 0)
        {
            // Endereçamento absoluto (curto)
            ea = (uint32_t)m68k_fetch_word(ctx);
            value = m68k_read_memory(ctx, ea, size);
        }
        else if (reg == 1)
        {
            // Endereçamento absoluto (longo)
            ea = m68k_fetch_long(ctx);
            value = m68k_read_memory(ctx, ea, size);
        }
        else if (reg == 4)
        {
            // Valor imediato
            if (size == 1)
                value = m68k_fetch_word(ctx) & 0xFF;
            else if (size == 2)
                value = m68k_fetch_word(ctx);
            else
                value = m68k_fetch_long(ctx);
        }
        break;
    }

    return value;
}

static void m68k_set_ea_value(m68k_t *ctx, uint8_t mode, uint8_t reg, uint8_t size, uint32_t value)
{
    uint32_t ea = 0;

    switch (mode)
    {
    case 0: // Dn - Registro de dados
        if (size == 1)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFFFF00) | (value & 0xFF);
        else if (size == 2)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFF0000) | (value & 0xFFFF);
        else
            ctx->regs.d[reg] = value;
        break;

    case 1: // An - Registro de endereço
        if (size == 2)
            ctx->regs.a[reg] = (int16_t)value; // Extensão de sinal
        else
            ctx->regs.a[reg] = value;
        break;

    case 2: // (An) - Indireto
        ea = ctx->regs.a[reg];
        m68k_write_memory(ctx, ea, size, value);
        break;

    case 3: // (An)+ - Indireto com pós-incremento
        ea = ctx->regs.a[reg];
        m68k_write_memory(ctx, ea, size, value);
        ctx->regs.a[reg] += (reg == 7 && size == 1) ? 2 : size;
        break;

    case 4: // -(An) - Indireto com pré-decremento
        ctx->regs.a[reg] -= (reg == 7 && size == 1) ? 2 : size;
        ea = ctx->regs.a[reg];
        m68k_write_memory(ctx, ea, size, value);
        break;

    case 5: // d16(An) - Indireto com deslocamento
    {
        int16_t disp = (int16_t)m68k_fetch_word(ctx);
        ea = ctx->regs.a[reg] + disp;
        m68k_write_memory(ctx, ea, size, value);
    }
    break;

    case 7: // Modos de endereçamento estendidos
        if (reg == 0)
        {
            // Endereçamento absoluto (curto)
            ea = (uint32_t)m68k_fetch_word(ctx);
            m68k_write_memory(ctx, ea, size, value);
        }
        else if (reg == 1)
        {
            // Endereçamento absoluto (longo)
            ea = m68k_fetch_long(ctx);
            m68k_write_memory(ctx, ea, size, value);
        }
        break;
    }
}

static void m68k_update_flags_logical(m68k_t *ctx, uint32_t result, uint8_t size)
{
    // Limpar flags V e C
    ctx->regs.sr &= ~(M68K_SR_V | M68K_SR_C);

    // Atualizar flag Z
    if ((size == 1 && (result & 0xFF) == 0) ||
        (size == 2 && (result & 0xFFFF) == 0) ||
        (size == 4 && result == 0))
    {
        ctx->regs.sr |= M68K_SR_Z;
    }
    else
    {
        ctx->regs.sr &= ~M68K_SR_Z;
    }

    // Atualizar flag N
    if ((size == 1 && (result & 0x80)) ||
        (size == 2 && (result & 0x8000)) ||
        (size == 4 && (result & 0x80000000)))
    {
        ctx->regs.sr |= M68K_SR_N;
    }
    else
    {
        ctx->regs.sr &= ~M68K_SR_N;
    }
}

static void m68k_update_flags_arithmetic(m68k_t *ctx, uint32_t src, uint32_t dst, uint32_t result, uint8_t size, uint8_t is_add)
{
    uint32_t mask = (size == 1) ? 0xFF : (size == 2) ? 0xFFFF
                                                     : 0xFFFFFFFF;
    uint32_t sign_bit = (size == 1) ? 0x80 : (size == 2) ? 0x8000
                                                         : 0x80000000;

    // Resultado truncado ao tamanho da operação
    result &= mask;

    // Flag Z
    if (result == 0)
    {
        ctx->regs.sr |= M68K_SR_Z;
    }
    else
    {
        ctx->regs.sr &= ~M68K_SR_Z;
    }

    // Flag N
    if (result & sign_bit)
    {
        ctx->regs.sr |= M68K_SR_N;
    }
    else
    {
        ctx->regs.sr &= ~M68K_SR_N;
    }

    // Flag C
    if (is_add)
    {
        if ((src & mask) + (dst & mask) > mask)
        {
            ctx->regs.sr |= M68K_SR_C;
        }
        else
        {
            ctx->regs.sr &= ~M68K_SR_C;
        }
    }
    else
    {
        if ((src & mask) > (dst & mask))
        {
            ctx->regs.sr |= M68K_SR_C;
        }
        else
        {
            ctx->regs.sr &= ~M68K_SR_C;
        }
    }

    // Flag V - Overflow
    src &= sign_bit;
    dst &= sign_bit;
    result &= sign_bit;

    if (is_add)
    {
        if ((src == dst) && (result != src))
        {
            ctx->regs.sr |= M68K_SR_V;
        }
        else
        {
            ctx->regs.sr &= ~M68K_SR_V;
        }
    }
    else
    {
        if ((src != dst) && (result == dst))
        {
            ctx->regs.sr |= M68K_SR_V;
        }
        else
        {
            ctx->regs.sr &= ~M68K_SR_V;
        }
    }
}

// Implementação das instruções

// MOVE
static void m68k_instr_move(m68k_t *ctx, uint16_t opcode)
{
    uint8_t size = 0;
    switch ((opcode >> 12) & 0x3)
    {
    case 1:
        size = 1;
        break; // MOVE.B
    case 3:
        size = 2;
        break; // MOVE.W
    case 2:
        size = 4;
        break; // MOVE.L
    }

    uint8_t src_mode = (opcode >> 3) & 0x7;
    uint8_t src_reg = opcode & 0x7;
    uint8_t dst_mode = (opcode >> 6) & 0x7;
    uint8_t dst_reg = (opcode >> 9) & 0x7;

    uint32_t src_value = m68k_get_ea_value(ctx, src_mode, src_reg, size);
    m68k_set_ea_value(ctx, dst_mode, dst_reg, size, src_value);

    // Atualizar flags
    m68k_update_flags_logical(ctx, src_value, size);
}

// MOVEQ - Move Quick
static void m68k_instr_moveq(m68k_t *ctx, uint16_t opcode)
{
    if ((opcode & 0xF100) == 0x7000)
    {
        uint8_t reg = (opcode >> 9) & 0x7;
        int8_t data = opcode & 0xFF;

        ctx->regs.d[reg] = (int32_t)data; // Extensão de sinal para 32 bits

        // Atualizar flags
        m68k_update_flags_logical(ctx, ctx->regs.d[reg], 4);
    }
}

// ADD
static void m68k_instr_add(m68k_t *ctx, uint16_t opcode)
{
    uint8_t reg = (opcode >> 9) & 0x7;
    uint8_t size = 0;

    switch ((opcode >> 6) & 0x3)
    {
    case 0:
        size = 1;
        break; // Byte
    case 1:
        size = 2;
        break; // Word
    case 2:
        size = 4;
        break; // Long
    }

    uint8_t mode = (opcode >> 3) & 0x7;
    uint8_t rm = opcode & 0x7;

    if (opcode & 0x100)
    {
        // EA + Dn -> EA
        uint32_t src_value = ctx->regs.d[reg];
        uint32_t dst_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t result = src_value + dst_value;

        m68k_set_ea_value(ctx, mode, rm, size, result);
        m68k_update_flags_arithmetic(ctx, src_value, dst_value, result, size, 1);
    }
    else
    {
        // Dn + EA -> Dn
        uint32_t src_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t dst_value = ctx->regs.d[reg];
        uint32_t result = src_value + dst_value;

        if (size == 1)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFFFF00) | (result & 0xFF);
        else if (size == 2)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFF0000) | (result & 0xFFFF);
        else
            ctx->regs.d[reg] = result;

        m68k_update_flags_arithmetic(ctx, src_value, dst_value, result, size, 1);
    }
}

// SUB
static void m68k_instr_sub(m68k_t *ctx, uint16_t opcode)
{
    uint8_t reg = (opcode >> 9) & 0x7;
    uint8_t size = 0;

    switch ((opcode >> 6) & 0x3)
    {
    case 0:
        size = 1;
        break; // Byte
    case 1:
        size = 2;
        break; // Word
    case 2:
        size = 4;
        break; // Long
    }

    uint8_t mode = (opcode >> 3) & 0x7;
    uint8_t rm = opcode & 0x7;

    if (opcode & 0x100)
    {
        // EA - Dn -> EA
        uint32_t src_value = ctx->regs.d[reg];
        uint32_t dst_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t result = dst_value - src_value;

        m68k_set_ea_value(ctx, mode, rm, size, result);
        m68k_update_flags_arithmetic(ctx, src_value, dst_value, result, size, 0);
    }
    else
    {
        // Dn - EA -> Dn
        uint32_t src_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t dst_value = ctx->regs.d[reg];
        uint32_t result = dst_value - src_value;

        if (size == 1)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFFFF00) | (result & 0xFF);
        else if (size == 2)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFF0000) | (result & 0xFFFF);
        else
            ctx->regs.d[reg] = result;

        m68k_update_flags_arithmetic(ctx, src_value, dst_value, result, size, 0);
    }
}

// AND
static void m68k_instr_and(m68k_t *ctx, uint16_t opcode)
{
    uint8_t reg = (opcode >> 9) & 0x7;
    uint8_t size = 0;

    switch ((opcode >> 6) & 0x3)
    {
    case 0:
        size = 1;
        break; // Byte
    case 1:
        size = 2;
        break; // Word
    case 2:
        size = 4;
        break; // Long
    }

    uint8_t mode = (opcode >> 3) & 0x7;
    uint8_t rm = opcode & 0x7;

    if (opcode & 0x100)
    {
        // EA & Dn -> EA
        uint32_t src_value = ctx->regs.d[reg];
        uint32_t dst_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t result = src_value & dst_value;

        m68k_set_ea_value(ctx, mode, rm, size, result);
        m68k_update_flags_logical(ctx, result, size);
    }
    else
    {
        // Dn & EA -> Dn
        uint32_t src_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t result = ctx->regs.d[reg] & src_value;

        if (size == 1)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFFFF00) | (result & 0xFF);
        else if (size == 2)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFF0000) | (result & 0xFFFF);
        else
            ctx->regs.d[reg] = result;

        m68k_update_flags_logical(ctx, result, size);
    }
}

// OR
static void m68k_instr_or(m68k_t *ctx, uint16_t opcode)
{
    uint8_t reg = (opcode >> 9) & 0x7;
    uint8_t size = 0;

    switch ((opcode >> 6) & 0x3)
    {
    case 0:
        size = 1;
        break; // Byte
    case 1:
        size = 2;
        break; // Word
    case 2:
        size = 4;
        break; // Long
    }

    uint8_t mode = (opcode >> 3) & 0x7;
    uint8_t rm = opcode & 0x7;

    if (opcode & 0x100)
    {
        // EA | Dn -> EA
        uint32_t src_value = ctx->regs.d[reg];
        uint32_t dst_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t result = src_value | dst_value;

        m68k_set_ea_value(ctx, mode, rm, size, result);
        m68k_update_flags_logical(ctx, result, size);
    }
    else
    {
        // Dn | EA -> Dn
        uint32_t src_value = m68k_get_ea_value(ctx, mode, rm, size);
        uint32_t result = ctx->regs.d[reg] | src_value;

        if (size == 1)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFFFF00) | (result & 0xFF);
        else if (size == 2)
            ctx->regs.d[reg] = (ctx->regs.d[reg] & 0xFFFF0000) | (result & 0xFFFF);
        else
            ctx->regs.d[reg] = result;

        m68k_update_flags_logical(ctx, result, size);
    }
}

// EOR - Exclusive OR
static void m68k_instr_eor(m68k_t *ctx, uint16_t opcode)
{
    uint8_t reg = (opcode >> 9) & 0x7;
    uint8_t size = 0;

    switch ((opcode >> 6) & 0x3)
    {
    case 0:
        size = 1;
        break; // Byte
    case 1:
        size = 2;
        break; // Word
    case 2:
        size = 4;
        break; // Long
    }

    uint8_t mode = (opcode >> 3) & 0x7;
    uint8_t rm = opcode & 0x7;

    uint32_t src_value = ctx->regs.d[reg];
    uint32_t dst_value = m68k_get_ea_value(ctx, mode, rm, size);
    uint32_t result = src_value ^ dst_value;

    m68k_set_ea_value(ctx, mode, rm, size, result);
    m68k_update_flags_logical(ctx, result, size);
}

// JMP - Jump
static void m68k_instr_jmp(m68k_t *ctx, uint16_t opcode)
{
    uint8_t mode = (opcode >> 3) & 0x7;
    uint8_t reg = opcode & 0x7;
    uint32_t target_addr = 0;

    // Calcular endereço efetivo
    switch (mode)
    {
    case 2: // (An)
        target_addr = ctx->regs.a[reg];
        break;

    case 5: // d16(An)
    {
        int16_t disp = (int16_t)m68k_fetch_word(ctx);
        target_addr = ctx->regs.a[reg] + disp;
    }
    break;

    case 7: // Modos de endereçamento estendidos
        if (reg == 0)
        {
            // Endereçamento absoluto (curto)
            target_addr = (uint32_t)m68k_fetch_word(ctx);
        }
        else if (reg == 1)
        {
            // Endereçamento absoluto (longo)
            target_addr = m68k_fetch_long(ctx);
        }
        break;
    }

    ctx->regs.pc = target_addr;
}

// JSR - Jump to Subroutine
static void m68k_instr_jsr(m68k_t *ctx, uint16_t opcode)
{
    uint8_t mode = (opcode >> 3) & 0x7;
    uint8_t reg = opcode & 0x7;
    uint32_t target_addr = 0;

    // Calcular endereço efetivo
    switch (mode)
    {
    case 2: // (An)
        target_addr = ctx->regs.a[reg];
        break;

    case 5: // d16(An)
    {
        int16_t disp = (int16_t)m68k_fetch_word(ctx);
        target_addr = ctx->regs.a[reg] + disp;
    }
    break;

    case 7: // Modos de endereçamento estendidos
        if (reg == 0)
        {
            // Endereçamento absoluto (curto)
            target_addr = (uint32_t)m68k_fetch_word(ctx);
        }
        else if (reg == 1)
        {
            // Endereçamento absoluto (longo)
            target_addr = m68k_fetch_long(ctx);
        }
        break;
    }

    // Empilhar endereço de retorno
    ctx->regs.a[7] -= 4;
    m68k_write_memory(ctx, ctx->regs.a[7], 4, ctx->regs.pc);

    // Saltar para o destino
    ctx->regs.pc = target_addr;
}

// RTS - Return from Subroutine
static void m68k_instr_rts(m68k_t *ctx, uint16_t opcode)
{
    // Desempilhar endereço de retorno
    uint32_t return_addr = m68k_read_memory(ctx, ctx->regs.a[7], 4);
    ctx->regs.a[7] += 4;

    // Saltar para o endereço de retorno
    ctx->regs.pc = return_addr;
}

// Bcc - Branch Conditionally
static void m68k_instr_bcc(m68k_t *ctx, uint16_t opcode)
{
    uint8_t condition = (opcode >> 8) & 0xF;
    int8_t displacement = opcode & 0xFF;
    int32_t full_disp = 0;

    if (displacement == 0)
    {
        // Deslocamento de 16 bits
        full_disp = (int16_t)m68k_fetch_word(ctx);
    }
    else
    {
        // Deslocamento de 8 bits
        full_disp = (int8_t)displacement;
    }

    bool take_branch = false;

    // Avaliar condição
    switch (condition)
    {
    case 0: // BRA - Branch Always
        take_branch = true;
        break;

    case 1: // BSR - Branch to Subroutine
        // Empilhar endereço de retorno
        ctx->regs.a[7] -= 4;
        m68k_write_memory(ctx, ctx->regs.a[7], 4, ctx->regs.pc);
        take_branch = true;
        break;

    case 2: // BHI - Branch if Higher
        take_branch = !((ctx->regs.sr & M68K_SR_C) || (ctx->regs.sr & M68K_SR_Z));
        break;

    case 3: // BLS - Branch if Lower or Same
        take_branch = (ctx->regs.sr & M68K_SR_C) || (ctx->regs.sr & M68K_SR_Z);
        break;

    case 4: // BCC/BHS - Branch if Carry Clear/Higher or Same
        take_branch = !(ctx->regs.sr & M68K_SR_C);
        break;

    case 5: // BCS/BLO - Branch if Carry Set/Lower
        take_branch = (ctx->regs.sr & M68K_SR_C);
        break;

    case 6: // BNE - Branch if Not Equal
        take_branch = !(ctx->regs.sr & M68K_SR_Z);
        break;

    case 7: // BEQ - Branch if Equal
        take_branch = (ctx->regs.sr & M68K_SR_Z);
        break;

    case 8: // BVC - Branch if Overflow Clear
        take_branch = !(ctx->regs.sr & M68K_SR_V);
        break;

    case 9: // BVS - Branch if Overflow Set
        take_branch = (ctx->regs.sr & M68K_SR_V);
        break;

    case 10: // BPL - Branch if Plus
        take_branch = !(ctx->regs.sr & M68K_SR_N);
        break;

    case 11: // BMI - Branch if Minus
        take_branch = (ctx->regs.sr & M68K_SR_N);
        break;

    case 12: // BGE - Branch if Greater or Equal
        take_branch = ((ctx->regs.sr & M68K_SR_N) && (ctx->regs.sr & M68K_SR_V)) ||
                      (!(ctx->regs.sr & M68K_SR_N) && !(ctx->regs.sr & M68K_SR_V));
        break;

    case 13: // BLT - Branch if Less Than
        take_branch = ((ctx->regs.sr & M68K_SR_N) && !(ctx->regs.sr & M68K_SR_V)) ||
                      (!(ctx->regs.sr & M68K_SR_N) && (ctx->regs.sr & M68K_SR_V));
        break;

    case 14: // BGT - Branch if Greater Than
        take_branch = ((ctx->regs.sr & M68K_SR_N) && (ctx->regs.sr & M68K_SR_V) && !(ctx->regs.sr & M68K_SR_Z)) ||
                      (!(ctx->regs.sr & M68K_SR_N) && !(ctx->regs.sr & M68K_SR_V) && !(ctx->regs.sr & M68K_SR_Z));
        break;

    case 15: // BLE - Branch if Less Than or Equal
        take_branch = (ctx->regs.sr & M68K_SR_Z) ||
                      ((ctx->regs.sr & M68K_SR_N) && !(ctx->regs.sr & M68K_SR_V)) ||
                      (!(ctx->regs.sr & M68K_SR_N) && (ctx->regs.sr & M68K_SR_V));
        break;
    }

    if (take_branch)
    {
        // Aplicar o deslocamento (PC já aponta para a próxima instrução)
        ctx->regs.pc += full_disp - 2;
    }
}

// NOP - No Operation
static void m68k_instr_nop(m68k_t *ctx, uint16_t opcode)
{
    // Não faz nada
}

// Inicialização da tabela de instruções
void m68k_init_instructions(m68k_t *ctx)
{
    // Limpar tabela de instruções
    memset(g_instruction_table, 0, sizeof(g_instruction_table));

    // MOVE
    for (int i = 0x1000; i < 0x4000; i++)
    {
        if ((i & 0xF000) == 0x1000 || (i & 0xF000) == 0x2000 || (i & 0xF000) == 0x3000)
        {
            g_instruction_table[i] = m68k_instr_move;
        }
    }

    // MOVEQ
    for (int i = 0x7000; i < 0x8000; i++)
    {
        if ((i & 0xF100) == 0x7000)
        {
            g_instruction_table[i] = m68k_instr_moveq;
        }
    }

    // ADD
    for (int i = 0xD000; i < 0xE000; i++)
    {
        g_instruction_table[i] = m68k_instr_add;
    }

    // SUB
    for (int i = 0x9000; i < 0xA000; i++)
    {
        g_instruction_table[i] = m68k_instr_sub;
    }

    // AND
    for (int i = 0xC000; i < 0xD000; i++)
    {
        g_instruction_table[i] = m68k_instr_and;
    }

    // OR
    for (int i = 0x8000; i < 0x9000; i++)
    {
        g_instruction_table[i] = m68k_instr_or;
    }

    // EOR
    for (int i = 0xB000; i < 0xC000; i++)
    {
        if ((i & 0xF138) >= 0xB100)
        {
            g_instruction_table[i] = m68k_instr_eor;
        }
    }

    // JMP
    for (int i = 0x4EC0; i < 0x4EE0; i++)
    {
        g_instruction_table[i] = m68k_instr_jmp;
    }

    // JSR
    for (int i = 0x4E80; i < 0x4EA0; i++)
    {
        g_instruction_table[i] = m68k_instr_jsr;
    }

    // RTS
    g_instruction_table[0x4E75] = m68k_instr_rts;

    // Bcc
    for (int i = 0x6000; i < 0x7000; i++)
    {
        g_instruction_table[i] = m68k_instr_bcc;
    }

    // NOP
    g_instruction_table[0x4E71] = m68k_instr_nop;
}

// Execução de instruções
void m68k_execute_instruction(m68k_t *ctx)
{
    uint16_t opcode = m68k_fetch_word(ctx);

    // Executar a instrução correspondente ao opcode
    m68k_instruction_handler handler = g_instruction_table[opcode];

    if (handler)
    {
        // Instrução implementada
        handler(ctx, opcode);
    }
    else
    {
        // Instrução não implementada
        printf("M68K: Instrução não implementada: 0x%04X em PC=0x%08X\n", opcode, ctx->regs.pc - 2);
    }
}
