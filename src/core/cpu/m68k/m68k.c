/**
 * @file m68k.c
 * @brief Implementação principal do processador Motorola 68000 (M68K)
 */

#include <stdlib.h>
#include <string.h>
#include "m68k.h"
#include "m68k_internal.h"

/* Implementação da tabela de instruções */
m68k_instruction_t m68k_instruction_table[M68K_INSTRUCTION_TABLE_SIZE];

/**
 * @brief Cria uma nova instância do processador M68K
 * @return Ponteiro para a instância do M68K ou NULL em caso de erro
 */
m68k_t *m68k_create(void)
{
    m68k_t *cpu = (m68k_t *)malloc(sizeof(m68k_t));
    if (cpu)
    {
        memset(cpu, 0, sizeof(m68k_t));
    }
    return cpu;
}

/**
 * @brief Destrói uma instância do processador M68K
 * @param cpu Ponteiro para a instância do M68K
 */
void m68k_destroy(m68k_t *cpu)
{
    if (cpu)
    {
        free(cpu);
    }
}

/**
 * @brief Inicializa o processador M68K
 * @param cpu Ponteiro para a instância do M68K
 */
void m68k_init(m68k_t *cpu)
{
    if (!cpu)
    {
        return;
    }

    /* Inicializar registradores e estado */
    memset(cpu->d, 0, sizeof(cpu->d));
    memset(cpu->a, 0, sizeof(cpu->a));
    cpu->pc = 0;
    cpu->sr = 0x2700; /* Supervisor mode, interrupts disabled */
    cpu->stopped = false;
    cpu->pending_interrupt = 0;
    cpu->interrupt_level = 0;
    cpu->cycles_remaining = 0;
    cpu->cycles_executed = 0;

    /* Inicializar tabela de instruções (uma vez) */
    static int instruction_table_initialized = 0;
    if (!instruction_table_initialized)
    {
        m68k_init_instruction_table();
        instruction_table_initialized = 1;
    }
}

/**
 * @brief Reseta o processador M68K
 * @param cpu Ponteiro para a instância do M68K
 */
void m68k_reset(m68k_t *cpu)
{
    if (!cpu)
    {
        return;
    }

    /* Salvar valores de SP e PC */
    uint32_t sp = m68k_read_long_internal(cpu, 0);
    uint32_t pc = m68k_read_long_internal(cpu, 4);

    /* Reinicializar estado */
    m68k_init(cpu);

    /* Configurar SP e PC a partir dos vetores de reset */
    cpu->a[7] = sp;
    cpu->pc = pc;
}

/**
 * @brief Executa um número específico de ciclos no processador M68K
 * @param cpu Ponteiro para a instância do M68K
 * @param cycles Número de ciclos a executar
 * @return Número de ciclos executados
 */
int m68k_execute_cycles(m68k_t *cpu, int cycles)
{
    if (!cpu || cycles <= 0)
    {
        return 0;
    }

    int total_executed = 0;
    cpu->cycles_remaining = cycles;

    while (cpu->cycles_remaining > 0)
    {
        /* Verificar interrupções pendentes */
        if (cpu->pending_interrupt &&
            cpu->interrupt_level > ((cpu->sr >> 8) & 7))
        {
            int irq_cycles = m68k_process_interrupt(cpu);
            cpu->cycles_remaining -= irq_cycles;
            total_executed += irq_cycles;
            continue;
        }

        /* Se CPU em estado STOP, consumir ciclos e retornar */
        if (cpu->stopped)
        {
            total_executed += cpu->cycles_remaining;
            cpu->cycles_remaining = 0;
            break;
        }

        /* Buscar próxima instrução */
        uint16_t opcode = m68k_read_word_internal(cpu, cpu->pc);
        cpu->pc += 2;

        /* Decodificar e executar instrução */
        m68k_instruction_t *instruction = &m68k_instruction_table[opcode];
        if (instruction->handler)
        {
            cpu->cycles_executed = instruction->handler(cpu, opcode);
        }
        else
        {
            /* Instrução ilegal - implementar tratamento */
            cpu->cycles_executed = 4; /* Valor aproximado */
        }

        /* Atualizar contadores de ciclos */
        cpu->cycles_remaining -= cpu->cycles_executed;
        total_executed += cpu->cycles_executed;
    }

    return total_executed;
}

/**
 * @brief Define o nível de interrupção
 * @param cpu Ponteiro para a instância do M68K
 * @param level Nível de interrupção (0-7)
 */
void m68k_set_irq(m68k_t *cpu, int level)
{
    if (!cpu)
    {
        return;
    }

    /* Validar nível de interrupção */
    if (level < 0)
    {
        level = 0;
    }
    else if (level > 7)
    {
        level = 7;
    }

    /* Definir nível e flag de interrupção pendente */
    cpu->interrupt_level = level;
    cpu->pending_interrupt = (level > 0) ? 1 : 0;

    /* Se em estado STOP e há interrupção pendente, saia de STOP */
    if (cpu->stopped && cpu->pending_interrupt)
    {
        cpu->stopped = false;
    }
}

/**
 * @brief Obtém o valor de um registrador
 * @param cpu Ponteiro para a instância do M68K
 * @param reg Índice do registrador
 * @return Valor do registrador
 */
uint32_t m68k_get_register(m68k_t *cpu, int reg)
{
    if (!cpu)
    {
        return 0;
    }

    switch (reg)
    {
    case M68K_REG_D0:
    case M68K_REG_D1:
    case M68K_REG_D2:
    case M68K_REG_D3:
    case M68K_REG_D4:
    case M68K_REG_D5:
    case M68K_REG_D6:
    case M68K_REG_D7:
        return cpu->d[reg - M68K_REG_D0];

    case M68K_REG_A0:
    case M68K_REG_A1:
    case M68K_REG_A2:
    case M68K_REG_A3:
    case M68K_REG_A4:
    case M68K_REG_A5:
    case M68K_REG_A6:
    case M68K_REG_A7:
        return cpu->a[reg - M68K_REG_A0];

    case M68K_REG_PC:
        return cpu->pc;

    case M68K_REG_SR:
        return cpu->sr;

    default:
        return 0;
    }
}

/**
 * @brief Define o valor de um registrador
 * @param cpu Ponteiro para a instância do M68K
 * @param reg Índice do registrador
 * @param value Valor a definir
 */
void m68k_set_register(m68k_t *cpu, int reg, uint32_t value)
{
    if (!cpu)
    {
        return;
    }

    switch (reg)
    {
    case M68K_REG_D0:
    case M68K_REG_D1:
    case M68K_REG_D2:
    case M68K_REG_D3:
    case M68K_REG_D4:
    case M68K_REG_D5:
    case M68K_REG_D6:
    case M68K_REG_D7:
        cpu->d[reg - M68K_REG_D0] = value;
        break;

    case M68K_REG_A0:
    case M68K_REG_A1:
    case M68K_REG_A2:
    case M68K_REG_A3:
    case M68K_REG_A4:
    case M68K_REG_A5:
    case M68K_REG_A6:
    case M68K_REG_A7:
        cpu->a[reg - M68K_REG_A0] = value;
        break;

    case M68K_REG_PC:
        cpu->pc = value;
        break;

    case M68K_REG_SR:
        cpu->sr = value & 0xFFFF;
        break;
    }
}

/**
 * @brief Processa uma interrupção
 * @param cpu Ponteiro para a instância do M68K
 * @return Número de ciclos consumidos
 */
int m68k_process_interrupt(m68k_t *cpu)
{
    if (!cpu || !cpu->pending_interrupt)
    {
        return 0;
    }

    /* Verificar se o nível de interrupção é maior que a máscara atual */
    int current_mask = (cpu->sr >> 8) & 7;
    if (cpu->interrupt_level <= current_mask)
    {
        cpu->pending_interrupt = 0;
        return 0;
    }

    /* Sair do estado STOP se necessário */
    cpu->stopped = false;

    /* Salvar contexto */
    uint32_t old_pc = cpu->pc;
    uint16_t old_sr = cpu->sr;

    /* Entrar em modo supervisor */
    cpu->sr |= M68K_SR_S;

    /* Desativar tracing */
    cpu->sr &= ~M68K_SR_T;

    /* Empilhar PC e SR */
    cpu->a[7] -= 4;
    m68k_write_long_internal(cpu, cpu->a[7], old_pc);
    cpu->a[7] -= 2;
    m68k_write_word_internal(cpu, cpu->a[7], old_sr);

    /* Atualizar máscara de interrupção */
    cpu->sr = (cpu->sr & 0xF8FF) | ((cpu->interrupt_level & 7) << 8);

    /* Obter vetor de interrupção */
    uint32_t vector_addr = 0x60 + (cpu->interrupt_level * 4);
    uint32_t new_pc = m68k_read_long_internal(cpu, vector_addr);

    /* Definir novo PC */
    cpu->pc = new_pc;

    /* Resetar flag de interrupção pendente */
    cpu->pending_interrupt = 0;

    /* Retornar ciclos consumidos pelo processamento da interrupção */
    return 44; /* Aproximadamente 44 ciclos para processamento de interrupção */
}

/**
 * @brief Lê um byte da memória
 * @param cpu Ponteiro para a instância do M68K
 * @param address Endereço de memória
 * @return Valor lido
 */
uint8_t m68k_read_byte_internal(m68k_t *cpu, uint32_t address)
{
    if (!cpu || !cpu->read_byte)
    {
        return 0xFF;
    }
    return cpu->read_byte(cpu->context, address);
}

/**
 * @brief Lê uma palavra da memória
 * @param cpu Ponteiro para a instância do M68K
 * @param address Endereço de memória
 * @return Valor lido
 */
uint16_t m68k_read_word_internal(m68k_t *cpu, uint32_t address)
{
    if (!cpu)
    {
        return 0xFFFF;
    }

    if (cpu->read_word)
    {
        return cpu->read_word(cpu->context, address);
    }
    else if (cpu->read_byte)
    {
        uint16_t value = cpu->read_byte(cpu->context, address) << 8;
        value |= cpu->read_byte(cpu->context, address + 1);
        return value;
    }

    return 0xFFFF;
}

/**
 * @brief Lê uma palavra longa da memória
 * @param cpu Ponteiro para a instância do M68K
 * @param address Endereço de memória
 * @return Valor lido
 */
uint32_t m68k_read_long_internal(m68k_t *cpu, uint32_t address)
{
    if (!cpu)
    {
        return 0xFFFFFFFF;
    }

    if (cpu->read_long)
    {
        return cpu->read_long(cpu->context, address);
    }
    else if (cpu->read_word)
    {
        uint32_t value = cpu->read_word(cpu->context, address) << 16;
        value |= cpu->read_word(cpu->context, address + 2);
        return value;
    }
    else if (cpu->read_byte)
    {
        uint32_t value = cpu->read_byte(cpu->context, address) << 24;
        value |= cpu->read_byte(cpu->context, address + 1) << 16;
        value |= cpu->read_byte(cpu->context, address + 2) << 8;
        value |= cpu->read_byte(cpu->context, address + 3);
        return value;
    }

    return 0xFFFFFFFF;
}

/**
 * @brief Escreve um byte na memória
 * @param cpu Ponteiro para a instância do M68K
 * @param address Endereço de memória
 * @param value Valor a escrever
 */
void m68k_write_byte_internal(m68k_t *cpu, uint32_t address, uint8_t value)
{
    if (!cpu || !cpu->write_byte)
    {
        return;
    }
    cpu->write_byte(cpu->context, address, value);
}

/**
 * @brief Escreve uma palavra na memória
 * @param cpu Ponteiro para a instância do M68K
 * @param address Endereço de memória
 * @param value Valor a escrever
 */
void m68k_write_word_internal(m68k_t *cpu, uint32_t address, uint16_t value)
{
    if (!cpu)
    {
        return;
    }

    if (cpu->write_word)
    {
        cpu->write_word(cpu->context, address, value);
    }
    else if (cpu->write_byte)
    {
        cpu->write_byte(cpu->context, address, (value >> 8) & 0xFF);
        cpu->write_byte(cpu->context, address + 1, value & 0xFF);
    }
}

/**
 * @brief Escreve uma palavra longa na memória
 * @param cpu Ponteiro para a instância do M68K
 * @param address Endereço de memória
 * @param value Valor a escrever
 */
void m68k_write_long_internal(m68k_t *cpu, uint32_t address, uint32_t value)
{
    if (!cpu)
    {
        return;
    }

    if (cpu->write_long)
    {
        cpu->write_long(cpu->context, address, value);
    }
    else if (cpu->write_word)
    {
        cpu->write_word(cpu->context, address, (value >> 16) & 0xFFFF);
        cpu->write_word(cpu->context, address + 2, value & 0xFFFF);
    }
    else if (cpu->write_byte)
    {
        cpu->write_byte(cpu->context, address, (value >> 24) & 0xFF);
        cpu->write_byte(cpu->context, address + 1, (value >> 16) & 0xFF);
        cpu->write_byte(cpu->context, address + 2, (value >> 8) & 0xFF);
        cpu->write_byte(cpu->context, address + 3, value & 0xFF);
    }
}

/**
 * @brief Verifica se uma condição é verdadeira com base no código de condição
 * @param cpu Ponteiro para a instância do M68K
 * @param condition Código de condição (0-15)
 * @return true se a condição for verdadeira, false caso contrário
 */
bool m68k_test_condition(m68k_t *cpu, uint8_t condition)
{
    if (!cpu)
    {
        return false;
    }

    uint16_t sr = cpu->sr;
    bool c = (sr & M68K_SR_C) != 0;
    bool v = (sr & M68K_SR_V) != 0;
    bool z = (sr & M68K_SR_Z) != 0;
    bool n = (sr & M68K_SR_N) != 0;

    switch (condition)
    {
    case M68K_COND_TRUE:
        return true;
    case M68K_COND_FALSE:
        return false;
    case M68K_COND_HI:
        return !c && !z;
    case M68K_COND_LS:
        return c || z;
    case M68K_COND_CC:
        return !c;
    case M68K_COND_CS:
        return c;
    case M68K_COND_NE:
        return !z;
    case M68K_COND_EQ:
        return z;
    case M68K_COND_VC:
        return !v;
    case M68K_COND_VS:
        return v;
    case M68K_COND_PL:
        return !n;
    case M68K_COND_MI:
        return n;
    case M68K_COND_GE:
        return (n && v) || (!n && !v);
    case M68K_COND_LT:
        return (n && !v) || (!n && v);
    case M68K_COND_GT:
        return (n && v && !z) || (!n && !v && !z);
    case M68K_COND_LE:
        return z || (n && !v) || (!n && v);
    default:
        return false;
    }
}

/* Inicialização da tabela de instruções - stub que será implementado no arquivo m68k_instructions.c */
void m68k_init_instruction_table(void)
{
    /* Limpar a tabela */
    memset(m68k_instruction_table, 0, sizeof(m68k_instruction_table));

    /* Inicializar com handlers para instruções ilegais */
    /* A implementação completa será feita em m68k_instructions.c */
}
