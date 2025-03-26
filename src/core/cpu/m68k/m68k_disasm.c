/**
 * @file m68k_disasm.c
 * @brief Implementação do desassemblador para o processador Motorola 68000 (M68K)
 */

#include <stdio.h>
#include <string.h>
#include "m68k.h"
#include "m68k_internal.h"
#include "m68k_disasm.h"

/* Nomes de registradores */
static const char *register_names[] = {
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7/SP",
    "PC", "SR"};

/* Nomes de condições */
static const char *condition_names[] = {
    "T", "F", "HI", "LS", "CC", "CS", "NE", "EQ",
    "VC", "VS", "PL", "MI", "GE", "LT", "GT", "LE"};

/* Tabela de mnemônicos para instruções */
static const char *mnemonic_table[M68K_INSTRUCTION_TABLE_SIZE];

/**
 * @brief Inicializa a tabela de mnemônicos
 */
static void init_mnemonic_table()
{
    static int initialized = 0;
    int i;

    if (initialized)
    {
        return;
    }

    /* Inicializar tudo como "???" */
    for (i = 0; i < M68K_INSTRUCTION_TABLE_SIZE; i++)
    {
        mnemonic_table[i] = "???";
    }

    /* Populer com mnemônicos conhecidos */
    for (i = 0x1000; i <= 0x3FFF; i++)
    {
        if ((i & 0xC000) == 0x0000)
            continue;

        uint8_t dst_mode = ((i >> 6) & 0x7);

        /* MOVEA */
        if (dst_mode == 1)
        {
            mnemonic_table[i] = "MOVEA";
        }
        else
        {
            mnemonic_table[i] = "MOVE";
        }
    }

    /* MOVEQ */
    for (i = 0x7000; i <= 0x7FFF; i++)
    {
        if ((i & 0xF100) != 0x7000)
            continue;
        mnemonic_table[i] = "MOVEQ";
    }

    /* ADD */
    for (i = 0xD000; i <= 0xDFFF; i++)
    {
        mnemonic_table[i] = "ADD";
    }

    /* SUB */
    for (i = 0x9000; i <= 0x9FFF; i++)
    {
        mnemonic_table[i] = "SUB";
    }

    /* AND */
    for (i = 0xC000; i <= 0xCFFF; i++)
    {
        mnemonic_table[i] = "AND";
    }

    /* OR */
    for (i = 0x8000; i <= 0x8FFF; i++)
    {
        mnemonic_table[i] = "OR";
    }

    /* JMP */
    for (i = 0x4EC0; i <= 0x4EDF; i++)
    {
        if ((i & 0xFFC0) != 0x4EC0)
            continue;
        mnemonic_table[i] = "JMP";
    }

    /* JSR */
    for (i = 0x4E80; i <= 0x4E9F; i++)
    {
        if ((i & 0xFFC0) != 0x4E80)
            continue;
        mnemonic_table[i] = "JSR";
    }

    /* RTS */
    mnemonic_table[0x4E75] = "RTS";

    /* BCC */
    mnemonic_table[0x6000] = "BRA";
    mnemonic_table[0x6100] = "BSR";
    mnemonic_table[0x6200] = "BHI";
    mnemonic_table[0x6300] = "BLS";
    mnemonic_table[0x6400] = "BCC";
    mnemonic_table[0x6500] = "BCS";
    mnemonic_table[0x6600] = "BNE";
    mnemonic_table[0x6700] = "BEQ";
    mnemonic_table[0x6800] = "BVC";
    mnemonic_table[0x6900] = "BVS";
    mnemonic_table[0x6A00] = "BPL";
    mnemonic_table[0x6B00] = "BMI";
    mnemonic_table[0x6C00] = "BGE";
    mnemonic_table[0x6D00] = "BLT";
    mnemonic_table[0x6E00] = "BGT";
    mnemonic_table[0x6F00] = "BLE";

    /* STOP */
    mnemonic_table[0x4E72] = "STOP";

    initialized = 1;
}

/**
 * @brief Formata uma operação de tamanho para string
 * @param size Tamanho (1=B, 2=W, 4=L)
 * @return String com o sufixo de tamanho
 */
static const char *size_suffix(int size)
{
    switch (size)
    {
    case 1:
        return ".B";
    case 2:
        return ".W";
    case 4:
        return ".L";
    default:
        return "";
    }
}

/**
 * @brief Formata um operando de endereço efetivo para string
 * @param cpu Ponteiro para a instância do M68K
 * @param mode Modo de endereçamento
 * @param reg Registrador
 * @param pc PC atual
 * @param buffer Buffer para armazenar a string
 * @param size Tamanho do buffer
 * @return Número de bytes adicionais usados pelo operando
 */
static int format_ea(m68k_t *cpu, uint8_t mode, uint8_t reg, uint32_t pc, char *buffer, int size)
{
    uint16_t extension;
    uint32_t value;
    int extra_bytes = 0;

    switch (mode)
    {
    case 0: /* Data Register Direct */
        snprintf(buffer, size, "D%d", reg);
        break;

    case 1: /* Address Register Direct */
        snprintf(buffer, size, "A%d", reg);
        break;

    case 2: /* Address Register Indirect */
        snprintf(buffer, size, "(A%d)", reg);
        break;

    case 3: /* Address Register Indirect with Post-increment */
        snprintf(buffer, size, "(A%d)+", reg);
        break;

    case 4: /* Address Register Indirect with Pre-decrement */
        snprintf(buffer, size, "-(A%d)", reg);
        break;

    case 5: /* Address Register Indirect with Displacement */
        if (cpu)
        {
            extension = m68k_read_word_internal(cpu, pc);
            snprintf(buffer, size, "$%04X(A%d)", (int16_t)extension, reg);
        }
        else
        {
            snprintf(buffer, size, "d16(A%d)", reg);
        }
        extra_bytes = 2;
        break;

    case 6: /* Address Register Indirect with Index */
        if (cpu)
        {
            extension = m68k_read_word_internal(cpu, pc);
            int8_t disp = (int8_t)(extension & 0xFF);
            uint8_t idx_reg = (extension >> 12) & 0x7;
            const char *idx_type = (extension & 0x8000) ? "A" : "D";
            snprintf(buffer, size, "%d(A%d,%s%d.%c)", disp, reg, idx_type, idx_reg,
                     (extension & 0x800) ? 'L' : 'W');
        }
        else
        {
            snprintf(buffer, size, "d8(A%d,Xn.x)", reg);
        }
        extra_bytes = 2;
        break;

    case 7: /* Extended modes */
        switch (reg)
        {
        case 0: /* Absolute Short */
            if (cpu)
            {
                extension = m68k_read_word_internal(cpu, pc);
                snprintf(buffer, size, "$%04X.W", extension);
            }
            else
            {
                snprintf(buffer, size, "abs.W");
            }
            extra_bytes = 2;
            break;

        case 1: /* Absolute Long */
            if (cpu)
            {
                value = m68k_read_long_internal(cpu, pc);
                snprintf(buffer, size, "$%08X", value);
            }
            else
            {
                snprintf(buffer, size, "abs.L");
            }
            extra_bytes = 4;
            break;

        case 2: /* PC with Displacement */
            if (cpu)
            {
                extension = m68k_read_word_internal(cpu, pc);
                int16_t disp = (int16_t)extension;
                snprintf(buffer, size, "$%04X(PC)", disp);
            }
            else
            {
                snprintf(buffer, size, "d16(PC)");
            }
            extra_bytes = 2;
            break;

        case 3: /* PC with Index */
            if (cpu)
            {
                extension = m68k_read_word_internal(cpu, pc);
                int8_t disp = (int8_t)(extension & 0xFF);
                uint8_t idx_reg = (extension >> 12) & 0x7;
                const char *idx_type = (extension & 0x8000) ? "A" : "D";
                snprintf(buffer, size, "%d(PC,%s%d.%c)", disp, idx_type, idx_reg,
                         (extension & 0x800) ? 'L' : 'W');
            }
            else
            {
                snprintf(buffer, size, "d8(PC,Xn.x)");
            }
            extra_bytes = 2;
            break;

        case 4: /* Immediate */
            snprintf(buffer, size, "#imm");
            /* O tamanho depende da operação - será tratado específicamente */
            break;

        default:
            snprintf(buffer, size, "???");
            break;
        }
        break;

    default:
        snprintf(buffer, size, "???");
        break;
    }

    return extra_bytes;
}

/**
 * @brief Formata uma instrução MOVE para string
 * @param cpu Ponteiro para a instância do M68K
 * @param opcode Opcode da instrução
 * @param address Endereço da instrução
 * @param buffer Buffer para armazenar a string
 * @param size Tamanho do buffer
 * @return Tamanho da instrução em bytes
 */
static int disassemble_move(m68k_t *cpu, uint16_t opcode, uint32_t address, char *buffer, int size)
{
    uint8_t src_mode, src_reg, dst_mode, dst_reg;
    int instruction_size;
    char src_buffer[64], dst_buffer[64];
    const char *sz_suffix;
    int src_extra, dst_extra;

    /* Determinar tamanho da operação */
    switch ((opcode >> 12) & 0x3)
    {
    case 1:
        sz_suffix = ".B";
        instruction_size = 1;
        break;
    case 3:
        sz_suffix = ".W";
        instruction_size = 2;
        break;
    case 2:
        sz_suffix = ".L";
        instruction_size = 4;
        break;
    default:
        return 2; /* Tamanho mínimo */
    }

    /* Extrair modos e registradores */
    src_mode = (opcode >> 3) & 0x7;
    src_reg = opcode & 0x7;
    dst_reg = ((opcode >> 9) & 0x7);
    dst_mode = ((opcode >> 6) & 0x7);

    /* Formatar operandos */
    address += 2;
    src_extra = format_ea(cpu, src_mode, src_reg, address, src_buffer, sizeof(src_buffer));
    address += src_extra;
    dst_extra = format_ea(cpu, dst_mode, dst_reg, address, dst_buffer, sizeof(dst_buffer));

    /* Escrever string completa */
    if (dst_mode == 1)
    {
        snprintf(buffer, size, "MOVEA%s %s,%s", sz_suffix, src_buffer, dst_buffer);
    }
    else
    {
        snprintf(buffer, size, "MOVE%s %s,%s", sz_suffix, src_buffer, dst_buffer);
    }

    return 2 + src_extra + dst_extra;
}

/**
 * @brief Formata uma instrução MOVEQ para string
 * @param cpu Ponteiro para a instância do M68K
 * @param opcode Opcode da instrução
 * @param address Endereço da instrução
 * @param buffer Buffer para armazenar a string
 * @param size Tamanho do buffer
 * @return Tamanho da instrução em bytes
 */
static int disassemble_moveq(m68k_t *cpu, uint16_t opcode, uint32_t address, char *buffer, int size)
{
    uint8_t reg;
    int8_t data;

    /* Extrair registrador e dados */
    reg = (opcode >> 9) & 0x7;
    data = (int8_t)(opcode & 0xFF);

    /* Formatar instrução */
    snprintf(buffer, size, "MOVEQ #$%02X,D%d", (uint8_t)data, reg);

    return 2; /* Tamanho fixo */
}

/**
 * @brief Formata uma instrução ALU (ADD/SUB/AND/OR) para string
 * @param cpu Ponteiro para a instância do M68K
 * @param opcode Opcode da instrução
 * @param address Endereço da instrução
 * @param buffer Buffer para armazenar a string
 * @param size Tamanho do buffer
 * @return Tamanho da instrução em bytes
 */
static int disassemble_alu(m68k_t *cpu, uint16_t opcode, uint32_t address, char *buffer, int size)
{
    const char *mnemonic;
    uint8_t op_size, reg, ea_mode, ea_reg;
    char ea_buffer[64], reg_buffer[64];
    const char *sz_suffix;
    int ea_extra = 0;

    /* Determinar mnemônico */
    switch ((opcode >> 12) & 0xF)
    {
    case 0x8:
        mnemonic = "OR";
        break;
    case 0x9:
        mnemonic = "SUB";
        break;
    case 0xC:
        mnemonic = "AND";
        break;
    case 0xD:
        mnemonic = "ADD";
        break;
    default:
        return 2; /* Tamanho mínimo */
    }

    /* Determinar tamanho da operação */
    op_size = ((opcode >> 6) & 0x3);
    switch (op_size)
    {
    case 0:
        sz_suffix = ".B";
        break;
    case 1:
        sz_suffix = ".W";
        break;
    case 2:
        sz_suffix = ".L";
        break;
    default:
        sz_suffix = "";
        break;
    }

    /* Extrair registrador e modo */
    reg = (opcode >> 9) & 0x7;
    ea_mode = (opcode >> 3) & 0x7;
    ea_reg = opcode & 0x7;

    /* Formatar operandos */
    address += 2;
    snprintf(reg_buffer, sizeof(reg_buffer), "D%d", reg);
    ea_extra = format_ea(cpu, ea_mode, ea_reg, address, ea_buffer, sizeof(ea_buffer));

    /* Determinar direção */
    if (opcode & 0x100)
    { /* EA,Dn -> EA */
        snprintf(buffer, size, "%s%s %s,%s", mnemonic, sz_suffix, reg_buffer, ea_buffer);
    }
    else
    { /* Dn,EA -> Dn */
        snprintf(buffer, size, "%s%s %s,%s", mnemonic, sz_suffix, ea_buffer, reg_buffer);
    }

    return 2 + ea_extra;
}

/**
 * @brief Formata uma instrução de controle (JMP/JSR) para string
 * @param cpu Ponteiro para a instância do M68K
 * @param opcode Opcode da instrução
 * @param address Endereço da instrução
 * @param buffer Buffer para armazenar a string
 * @param size Tamanho do buffer
 * @return Tamanho da instrução em bytes
 */
static int disassemble_control(m68k_t *cpu, uint16_t opcode, uint32_t address, char *buffer, int size)
{
    const char *mnemonic;
    uint8_t mode, reg;
    char ea_buffer[64];
    int ea_extra = 0;

    /* Determinar mnemônico */
    if ((opcode & 0xFFC0) == 0x4E80)
    {
        mnemonic = "JSR";
    }
    else if ((opcode & 0xFFC0) == 0x4EC0)
    {
        mnemonic = "JMP";
    }
    else
    {
        return 2; /* Tamanho mínimo */
    }

    /* Extrair modo e registrador */
    mode = (opcode >> 3) & 0x7;
    reg = opcode & 0x7;

    /* Formatar operando */
    address += 2;
    ea_extra = format_ea(cpu, mode, reg, address, ea_buffer, sizeof(ea_buffer));

    /* Formatar instrução */
    snprintf(buffer, size, "%s %s", mnemonic, ea_buffer);

    return 2 + ea_extra;
}

/**
 * @brief Formata uma instrução de branch (Bcc) para string
 * @param cpu Ponteiro para a instância do M68K
 * @param opcode Opcode da instrução
 * @param address Endereço da instrução
 * @param buffer Buffer para armazenar a string
 * @param size Tamanho do buffer
 * @return Tamanho da instrução em bytes
 */
static int disassemble_branch(m68k_t *cpu, uint16_t opcode, uint32_t address, char *buffer, int size)
{
    uint8_t condition;
    int8_t displacement;
    uint16_t extension;
    int32_t target;
    const char *mnemonic;

    /* Extrair condição e deslocamento */
    condition = (opcode >> 8) & 0xF;
    displacement = (int8_t)(opcode & 0xFF);

    /* Determinar mnemônico */
    switch (condition)
    {
    case 0:
        mnemonic = "BRA";
        break;
    case 1:
        mnemonic = "BSR";
        break;
    case 2:
        mnemonic = "BHI";
        break;
    case 3:
        mnemonic = "BLS";
        break;
    case 4:
        mnemonic = "BCC";
        break;
    case 5:
        mnemonic = "BCS";
        break;
    case 6:
        mnemonic = "BNE";
        break;
    case 7:
        mnemonic = "BEQ";
        break;
    case 8:
        mnemonic = "BVC";
        break;
    case 9:
        mnemonic = "BVS";
        break;
    case 10:
        mnemonic = "BPL";
        break;
    case 11:
        mnemonic = "BMI";
        break;
    case 12:
        mnemonic = "BGE";
        break;
    case 13:
        mnemonic = "BLT";
        break;
    case 14:
        mnemonic = "BGT";
        break;
    case 15:
        mnemonic = "BLE";
        break;
    default:
        mnemonic = "B??";
        break;
    }

    /* Se deslocamento for 0, usar word extendida */
    if (displacement == 0)
    {
        if (cpu)
        {
            extension = m68k_read_word_internal(cpu, address + 2);
            target = address + 2 + (int16_t)extension;
            snprintf(buffer, size, "%s $%08X", mnemonic, target);
        }
        else
        {
            snprintf(buffer, size, "%s <disp16>", mnemonic);
        }
        return 4; /* 2 bytes para opcode + 2 bytes para extensão */
    }
    else
    {
        target = address + 2 + displacement;
        snprintf(buffer, size, "%s $%08X", mnemonic, target);
        return 2; /* 2 bytes para opcode */
    }
}

/**
 * @brief Desassembla uma única instrução no endereço especificado
 * @param cpu Ponteiro para a instância do M68K
 * @param address Endereço da instrução a desassemblar
 * @param buffer Buffer para armazenar a string desassemblada
 * @param size Tamanho do buffer
 * @return Tamanho da instrução em bytes
 */
int m68k_disassemble(m68k_t *cpu, uint32_t address, char *buffer, int size)
{
    uint16_t opcode;

    if (!cpu || !buffer || size <= 0)
    {
        if (buffer && size > 0)
        {
            buffer[0] = '\0';
        }
        return 2; /* Tamanho mínimo */
    }

    /* Inicializar tabela de mnemônicos se necessário */
    init_mnemonic_table();

    /* Ler opcode */
    opcode = m68k_read_word_internal(cpu, address);

    /* Desassemblar baseado no opcode */
    return m68k_disassemble_opcode(cpu, opcode, address, buffer, size);
}

/**
 * @brief Desassembla uma instrução a partir de um opcode específico
 * @param cpu Ponteiro para a instância do M68K (pode ser NULL se não precisar acessar memória)
 * @param opcode Opcode da instrução
 * @param address Endereço da instrução (usado para instruções relativas ao PC)
 * @param buffer Buffer para armazenar a string desassemblada
 * @param size Tamanho do buffer
 * @return Tamanho da instrução em bytes
 */
int m68k_disassemble_opcode(m68k_t *cpu, uint16_t opcode, uint32_t address, char *buffer, int size)
{
    if (!buffer || size <= 0)
    {
        return 2; /* Tamanho mínimo */
    }

    /* Inicializar tabela de mnemônicos se necessário */
    init_mnemonic_table();

    /* Desassemblar baseado no opcode */
    switch (opcode & 0xF000)
    {
    case 0x1000: /* MOVE.B */
    case 0x2000: /* MOVE.L */
    case 0x3000: /* MOVE.W */
        return disassemble_move(cpu, opcode, address, buffer, size);

    case 0x7000:
        if ((opcode & 0xF100) == 0x7000)
        { /* MOVEQ */
            return disassemble_moveq(cpu, opcode, address, buffer, size);
        }
        break;

    case 0x8000: /* OR */
    case 0x9000: /* SUB */
    case 0xC000: /* AND */
    case 0xD000: /* ADD */
        return disassemble_alu(cpu, opcode, address, buffer, size);

    case 0x4000:
        if ((opcode & 0xFFC0) == 0x4E80 || (opcode & 0xFFC0) == 0x4EC0)
        { /* JSR / JMP */
            return disassemble_control(cpu, opcode, address, buffer, size);
        }
        else if (opcode == 0x4E75)
        { /* RTS */
            snprintf(buffer, size, "RTS");
            return 2;
        }
        else if (opcode == 0x4E72)
        { /* STOP */
            if (cpu)
            {
                uint16_t imm = m68k_read_word_internal(cpu, address + 2);
                snprintf(buffer, size, "STOP #$%04X", imm);
            }
            else
            {
                snprintf(buffer, size, "STOP #<imm>");
            }
            return 4;
        }
        break;

    case 0x6000: /* Bcc */
        return disassemble_branch(cpu, opcode, address, buffer, size);
    }

    /* Instrução não reconhecida - mostrar opcode */
    snprintf(buffer, size, "DC.W $%04X", opcode);
    return 2;
}

/**
 * @brief Obtém o nome do registrador
 * @param reg Índice do registrador
 * @return String com o nome do registrador
 */
const char *m68k_get_register_name(int reg)
{
    if (reg >= 0 && reg < sizeof(register_names) / sizeof(register_names[0]))
    {
        return register_names[reg];
    }
    return "???";
}

/**
 * @brief Obtém o nome da condição
 * @param condition Índice da condição (0-15)
 * @return String com o nome da condição
 */
const char *m68k_get_condition_name(int condition)
{
    if (condition >= 0 && condition < sizeof(condition_names) / sizeof(condition_names[0]))
    {
        return condition_names[condition];
    }
    return "???";
}
