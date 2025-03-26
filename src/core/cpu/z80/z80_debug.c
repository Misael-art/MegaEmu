/**
 * @file z80_debug.c
 * @brief Implementação do sistema avançado de depuração para o processador Z80
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "z80_debug.h"
#include "z80_internal.h"

// Número máximo de breakpoints suportados
#define MAX_BREAKPOINTS 64

// Tamanho do buffer de trace (quantas instruções armazenar)
#define TRACE_BUFFER_SIZE 1024

/**
 * @brief Contexto de depuração
 */
struct z80_debug_context
{
    z80_t *cpu;                                    // Referência para a CPU
    z80_breakpoint_t breakpoints[MAX_BREAKPOINTS]; // Array de breakpoints
    int breakpoint_count;                          // Contador de breakpoints
    int next_breakpoint_id;                        // Próximo ID de breakpoint

    // Buffer circular para trace de instruções
    z80_trace_entry_t trace_buffer[TRACE_BUFFER_SIZE];
    int trace_index;    // Índice atual no buffer circular
    int trace_count;    // Número de entradas válidas no buffer
    bool trace_enabled; // Flag indicando se o trace está ativo

    // Controle de step-over e step-out
    uint16_t step_over_pc; // PC para step-over
    uint16_t step_out_sp;  // SP para step-out
    bool step_mode;        // Flag indicando se está em modo de step

    // Callbacks
    void (*breakpoint_callback)(z80_t *cpu, z80_breakpoint_t *breakpoint, void *user_data);
    void *breakpoint_callback_data;

    void (*trace_callback)(z80_t *cpu, z80_trace_entry_t *entry, void *user_data);
    void *trace_callback_data;
};

/**
 * @brief Inicializar subsistema de debug
 */
z80_debug_t *z80_debug_create(z80_t *cpu)
{
    if (!cpu)
    {
        return NULL;
    }

    z80_debug_t *debug = (z80_debug_t *)malloc(sizeof(z80_debug_t));
    if (!debug)
    {
        return NULL;
    }

    // Inicializar estrutura
    memset(debug, 0, sizeof(z80_debug_t));
    debug->cpu = cpu;
    debug->breakpoint_count = 0;
    debug->next_breakpoint_id = 1;
    debug->trace_index = 0;
    debug->trace_count = 0;
    debug->trace_enabled = false;
    debug->step_mode = false;

    return debug;
}

/**
 * @brief Destruir subsistema de debug
 */
void z80_debug_destroy(z80_debug_t *debug)
{
    if (debug)
    {
        free(debug);
    }
}

/**
 * @brief Adicionar um breakpoint simples
 */
int z80_debug_add_breakpoint(z80_debug_t *debug, z80_breakpoint_type_t type, uint16_t address)
{
    return z80_debug_add_breakpoint_ex(debug, type, address, Z80_CONDITION_ALWAYS, 0);
}

/**
 * @brief Adicionar um breakpoint avançado com condição
 */
int z80_debug_add_breakpoint_ex(z80_debug_t *debug, z80_breakpoint_type_t type, uint16_t address,
                                z80_breakpoint_condition_t condition, uint16_t value)
{
    if (!debug || debug->breakpoint_count >= MAX_BREAKPOINTS)
    {
        return -1;
    }

    // Encontrar índice livre
    int index = debug->breakpoint_count;
    int id = debug->next_breakpoint_id++;

    // Configurar o breakpoint
    z80_breakpoint_t *bp = &debug->breakpoints[index];
    bp->id = id;
    bp->type = type;
    bp->address = address;
    bp->address_end = address; // Não é range ainda
    bp->condition = condition;
    bp->condition_value = value;
    bp->condition_mask = 0xFFFF;
    bp->enabled = true;
    bp->temporary = false;

    snprintf(bp->description, sizeof(bp->description), "Breakpoint %d em 0x%04X", id, address);

    debug->breakpoint_count++;

    return id;
}

/**
 * @brief Encontrar índice de um breakpoint pelo ID
 */
static int find_breakpoint_index(z80_debug_t *debug, int id)
{
    for (int i = 0; i < debug->breakpoint_count; i++)
    {
        if (debug->breakpoints[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Remover um breakpoint
 */
bool z80_debug_remove_breakpoint(z80_debug_t *debug, int id)
{
    if (!debug)
    {
        return false;
    }

    // Encontrar o breakpoint
    int index = find_breakpoint_index(debug, id);
    if (index < 0)
    {
        return false;
    }

    // Remover e compactar a lista
    if (index < debug->breakpoint_count - 1)
    {
        // Mover o último breakpoint para a posição do removido
        memcpy(&debug->breakpoints[index],
               &debug->breakpoints[debug->breakpoint_count - 1],
               sizeof(z80_breakpoint_t));
    }

    debug->breakpoint_count--;
    return true;
}

/**
 * @brief Habilitar ou desabilitar um breakpoint
 */
void z80_debug_enable_breakpoint(z80_debug_t *debug, int id, bool enable)
{
    if (!debug)
    {
        return;
    }

    int index = find_breakpoint_index(debug, id);
    if (index >= 0)
    {
        debug->breakpoints[index].enabled = enable;
    }
}

/**
 * @brief Continuar execução após um breakpoint
 */
void z80_debug_continue(z80_debug_t *debug)
{
    if (!debug)
    {
        return;
    }

    debug->step_mode = false;
    debug->step_over_pc = 0;
    debug->step_out_sp = 0;
}

/**
 * @brief Executar a próxima instrução (step into)
 */
void z80_debug_step_into(z80_debug_t *debug)
{
    if (!debug)
    {
        return;
    }

    debug->step_mode = true;
    debug->step_over_pc = 0;
    debug->step_out_sp = 0;
}

/**
 * @brief Executar até retornar da instrução atual (step over)
 */
void z80_debug_step_over(z80_debug_t *debug)
{
    if (!debug || !debug->cpu)
    {
        return;
    }

    // Obter o PC atual
    uint16_t pc = debug->cpu->pc;

    // Ler o opcode para determinar se é uma chamada
    uint8_t opcode = debug->cpu->read_byte(debug->cpu->context, pc);

    // Verificar se é CALL, RST ou outras instruções que fazem chamadas
    bool is_call = (opcode == 0xCD) ||                     // CALL nn
                   (opcode == 0xC4) || (opcode == 0xCC) || // CALL NZ/Z,nn
                   (opcode == 0xD4) || (opcode == 0xDC) || // CALL NC/C,nn
                   (opcode == 0xE4) || (opcode == 0xEC) || // CALL PO/PE,nn
                   (opcode == 0xF4) || (opcode == 0xFC) || // CALL P/M,nn
                   (opcode == 0xC7) || (opcode == 0xCF) || // RST 0/8
                   (opcode == 0xD7) || (opcode == 0xDF) || // RST 10/18
                   (opcode == 0xE7) || (opcode == 0xEF) || // RST 20/28
                   (opcode == 0xF7) || (opcode == 0xFF);   // RST 30/38

    if (is_call)
    {
        // Para CALLs, precisamos continuar até o PC após a instrução
        int length = z80_get_instruction_length(debug->cpu, pc);
        debug->step_mode = true;
        debug->step_over_pc = pc + length;
    }
    else
    {
        // Para outras instruções, fazemos um simples step into
        z80_debug_step_into(debug);
    }
}

/**
 * @brief Executar até sair da sub-rotina atual (step out)
 */
void z80_debug_step_out(z80_debug_t *debug)
{
    if (!debug || !debug->cpu)
    {
        return;
    }

    // Armazenar o SP atual
    debug->step_mode = true;
    debug->step_over_pc = 0;
    debug->step_out_sp = debug->cpu->regs.sp;
}

/**
 * @brief Habilitar ou desabilitar trace de execução
 */
void z80_debug_enable_trace(z80_debug_t *debug, bool enable)
{
    if (!debug)
    {
        return;
    }

    debug->trace_enabled = enable;
    if (enable && debug->trace_count == 0)
    {
        // Inicializar buffer de trace
        debug->trace_index = 0;
        debug->trace_count = 0;
    }
}

/**
 * @brief Obter uma entrada específica do trace
 */
z80_trace_entry_t *z80_debug_get_trace(z80_debug_t *debug, int index)
{
    if (!debug || index < 0 || index >= debug->trace_count)
    {
        return NULL;
    }

    // Calcular o índice real no buffer circular
    int real_index = (debug->trace_index - debug->trace_count + index) % TRACE_BUFFER_SIZE;
    if (real_index < 0)
    {
        real_index += TRACE_BUFFER_SIZE;
    }

    return &debug->trace_buffer[real_index];
}

/**
 * @brief Limpar o buffer de trace
 */
void z80_debug_clear_trace(z80_debug_t *debug)
{
    if (!debug)
    {
        return;
    }

    debug->trace_index = 0;
    debug->trace_count = 0;
}

/**
 * @brief Adicionar entrada ao trace
 */
static void add_trace_entry(z80_debug_t *debug, z80_t *cpu)
{
    if (!debug || !cpu || !debug->trace_enabled)
    {
        return;
    }

    // Alocar nova entrada
    z80_trace_entry_t *entry = &debug->trace_buffer[debug->trace_index];

    // Preencher com dados da instrução atual
    entry->pc = cpu->pc;

    // Determinar comprimento da instrução
    int length = z80_get_instruction_length(cpu, cpu->pc);
    entry->opcode_length = length > 4 ? 4 : length;

    // Ler bytes da instrução
    for (int i = 0; i < entry->opcode_length; i++)
    {
        entry->opcode[i] = cpu->read_byte(cpu->context, cpu->pc + i);
    }

    // Armazenar registradores
    entry->registers[0] = cpu->regs.af;
    entry->registers[1] = cpu->regs.bc;
    entry->registers[2] = cpu->regs.de;
    entry->registers[3] = cpu->regs.hl;
    entry->registers[4] = cpu->regs.ix;
    entry->registers[5] = cpu->regs.iy;
    entry->registers[6] = cpu->regs.sp;
    entry->registers[7] = cpu->regs.pc;
    entry->registers[8] = cpu->regs.af_prime;
    entry->registers[9] = cpu->regs.bc_prime;
    entry->registers[10] = cpu->regs.de_prime;
    entry->registers[11] = cpu->regs.hl_prime;

    // Flags
    entry->flags = cpu->regs.f;

    // Desassemblar a instrução
    z80_disassemble_instruction(cpu, cpu->pc, entry->disassembly, sizeof(entry->disassembly));

    // Avançar índice no buffer circular
    debug->trace_index = (debug->trace_index + 1) % TRACE_BUFFER_SIZE;
    if (debug->trace_count < TRACE_BUFFER_SIZE)
    {
        debug->trace_count++;
    }

    // Chamar callback se configurado
    if (debug->trace_callback)
    {
        debug->trace_callback(cpu, entry, debug->trace_callback_data);
    }
}

/**
 * @brief Verificar se a condição do breakpoint é satisfeita
 */
static bool check_breakpoint_condition(z80_breakpoint_t *bp, uint16_t value)
{
    switch (bp->condition)
    {
    case Z80_CONDITION_ALWAYS:
        return true;

    case Z80_CONDITION_EQUALS:
        return value == bp->condition_value;

    case Z80_CONDITION_NOT_EQUALS:
        return value != bp->condition_value;

    case Z80_CONDITION_GREATER:
        return value > bp->condition_value;

    case Z80_CONDITION_LESS:
        return value < bp->condition_value;

    case Z80_CONDITION_MASK_MATCH:
        return (value & bp->condition_mask) == bp->condition_value;

    default:
        return false;
    }
}

/**
 * @brief Verificar breakpoints antes da execução
 */
bool z80_debug_check_execution_breakpoint(z80_debug_t *debug, z80_t *cpu)
{
    if (!debug || !cpu)
    {
        return false;
    }

    uint16_t pc = cpu->pc;

    // Verificar breakpoints de execução
    for (int i = 0; i < debug->breakpoint_count; i++)
    {
        z80_breakpoint_t *bp = &debug->breakpoints[i];

        if (!bp->enabled)
        {
            continue;
        }

        if (bp->type == Z80_BREAK_EXECUTION &&
            pc >= bp->address && pc <= bp->address_end)
        {

            // Verificar a condição
            if (check_breakpoint_condition(bp, pc))
            {
                // Ativar modo step
                debug->step_mode = true;

                // Se o breakpoint é temporário, removê-lo
                if (bp->temporary)
                {
                    z80_debug_remove_breakpoint(debug, bp->id);
                }

                // Chamar callback se configurado
                if (debug->breakpoint_callback)
                {
                    debug->breakpoint_callback(cpu, bp, debug->breakpoint_callback_data);
                }

                return true;
            }
        }
    }

    // Verificar modo step
    if (debug->step_mode)
    {
        // Step over: verificar se chegamos ao PC desejado
        if (debug->step_over_pc != 0 && pc == debug->step_over_pc)
        {
            debug->step_over_pc = 0;
            return true;
        }

        // Step out: verificar se o SP aumentou (RET executado)
        if (debug->step_out_sp != 0 &&
            cpu->regs.sp > debug->step_out_sp &&
            (cpu->read_byte(cpu->context, pc - 1) == 0xC9 || // RET
             cpu->read_byte(cpu->context, pc - 1) == 0xC0 || // RET NZ
             cpu->read_byte(cpu->context, pc - 1) == 0xC8 || // RET Z
             cpu->read_byte(cpu->context, pc - 1) == 0xD0 || // RET NC
             cpu->read_byte(cpu->context, pc - 1) == 0xD8 || // RET C
             cpu->read_byte(cpu->context, pc - 1) == 0xE0 || // RET PO
             cpu->read_byte(cpu->context, pc - 1) == 0xE8 || // RET PE
             cpu->read_byte(cpu->context, pc - 1) == 0xF0 || // RET P
             cpu->read_byte(cpu->context, pc - 1) == 0xF8 || // RET M
             cpu->read_byte(cpu->context, pc - 1) == 0xED &&
                 cpu->read_byte(cpu->context, pc - 2) == 0x4D))
        { // RETI

            debug->step_out_sp = 0;
            return true;
        }

        // Step into: parar em qualquer instrução
        if (debug->step_over_pc == 0 && debug->step_out_sp == 0)
        {
            return true;
        }
    }

    // Adicionar ao trace se habilitado
    add_trace_entry(debug, cpu);

    return false;
}

/**
 * @brief Verificar breakpoints de memória
 */
bool z80_debug_check_memory_breakpoint(z80_debug_t *debug, z80_t *cpu, uint16_t address, bool is_write, uint8_t value)
{
    if (!debug || !cpu)
    {
        return false;
    }

    // Verificar breakpoints de memória
    for (int i = 0; i < debug->breakpoint_count; i++)
    {
        z80_breakpoint_t *bp = &debug->breakpoints[i];

        if (!bp->enabled)
        {
            continue;
        }

        if ((is_write && bp->type == Z80_BREAK_MEMORY_WRITE) ||
            (!is_write && bp->type == Z80_BREAK_MEMORY_READ))
        {

            if (address >= bp->address && address <= bp->address_end)
            {
                // Verificar a condição
                if (check_breakpoint_condition(bp, value))
                {
                    // Ativar modo step
                    debug->step_mode = true;

                    // Se o breakpoint é temporário, removê-lo
                    if (bp->temporary)
                    {
                        z80_debug_remove_breakpoint(debug, bp->id);
                    }

                    // Chamar callback se configurado
                    if (debug->breakpoint_callback)
                    {
                        debug->breakpoint_callback(cpu, bp, debug->breakpoint_callback_data);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

/**
 * @brief Verificar breakpoints de I/O
 */
bool z80_debug_check_io_breakpoint(z80_debug_t *debug, z80_t *cpu, uint16_t port, bool is_write, uint8_t value)
{
    if (!debug || !cpu)
    {
        return false;
    }

    // Verificar breakpoints de I/O
    for (int i = 0; i < debug->breakpoint_count; i++)
    {
        z80_breakpoint_t *bp = &debug->breakpoints[i];

        if (!bp->enabled)
        {
            continue;
        }

        if ((is_write && bp->type == Z80_BREAK_IO_WRITE) ||
            (!is_write && bp->type == Z80_BREAK_IO_READ))
        {

            if (port >= bp->address && port <= bp->address_end)
            {
                // Verificar a condição
                if (check_breakpoint_condition(bp, value))
                {
                    // Ativar modo step
                    debug->step_mode = true;

                    // Se o breakpoint é temporário, removê-lo
                    if (bp->temporary)
                    {
                        z80_debug_remove_breakpoint(debug, bp->id);
                    }

                    // Chamar callback se configurado
                    if (debug->breakpoint_callback)
                    {
                        debug->breakpoint_callback(cpu, bp, debug->breakpoint_callback_data);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

/**
 * @brief Registrar callback para eventos de breakpoint
 */
void z80_debug_set_breakpoint_callback(z80_debug_t *debug,
                                       void (*callback)(z80_t *cpu, z80_breakpoint_t *breakpoint, void *user_data),
                                       void *user_data)
{
    if (!debug)
    {
        return;
    }

    debug->breakpoint_callback = callback;
    debug->breakpoint_callback_data = user_data;
}

/**
 * @brief Registrar callback para eventos de trace
 */
void z80_debug_set_trace_callback(z80_debug_t *debug,
                                  void (*callback)(z80_t *cpu, z80_trace_entry_t *entry, void *user_data),
                                  void *user_data)
{
    if (!debug)
    {
        return;
    }

    debug->trace_callback = callback;
    debug->trace_callback_data = user_data;
}

/**
 * @brief Dumpar o estado atual do Z80
 */
void z80_debug_dump_state(z80_debug_t *debug, char *buffer, int buffer_size)
{
    if (!debug || !debug->cpu || !buffer || buffer_size <= 0)
    {
        if (buffer && buffer_size > 0)
        {
            buffer[0] = '\0';
        }
        return;
    }

    z80_t *cpu = debug->cpu;

    snprintf(buffer, buffer_size,
             "Z80 Estado:\n"
             "AF=%04X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X\n"
             "AF'=%04X BC'=%04X DE'=%04X HL'=%04X SP=%04X PC=%04X\n"
             "Flags: S=%d Z=%d H=%d P/V=%d N=%d C=%d\n"
             "IFF1=%d IFF2=%d IM=%d HALT=%d\n",
             cpu->regs.af, cpu->regs.bc, cpu->regs.de, cpu->regs.hl,
             cpu->regs.ix, cpu->regs.iy,
             cpu->regs.af_prime, cpu->regs.bc_prime,
             cpu->regs.de_prime, cpu->regs.hl_prime,
             cpu->regs.sp, cpu->regs.pc,
             (cpu->regs.f & Z80_S_FLAG) ? 1 : 0,
             (cpu->regs.f & Z80_Z_FLAG) ? 1 : 0,
             (cpu->regs.f & Z80_H_FLAG) ? 1 : 0,
             (cpu->regs.f & Z80_PV_FLAG) ? 1 : 0,
             (cpu->regs.f & Z80_N_FLAG) ? 1 : 0,
             (cpu->regs.f & Z80_C_FLAG) ? 1 : 0,
             cpu->iff1, cpu->iff2, cpu->im, cpu->halted);
}

/**
 * @brief Dumpar uma região de memória
 */
void z80_debug_dump_memory(z80_debug_t *debug, uint16_t address, uint16_t size, char *buffer, int buffer_size)
{
    if (!debug || !debug->cpu || !buffer || buffer_size <= 0)
    {
        if (buffer && buffer_size > 0)
        {
            buffer[0] = '\0';
        }
        return;
    }

    z80_t *cpu = debug->cpu;
    int offset = 0;

    // Limitar tamanho para evitar buffer overflow
    if (size > (buffer_size / 4))
    {
        size = buffer_size / 4;
    }

    // Cabeçalho
    offset += snprintf(buffer + offset, buffer_size - offset,
                       "Dump de memória a partir de 0x%04X:\n", address);

    // 16 bytes por linha
    for (uint16_t i = 0; i < size; i += 16)
    {
        // Endereço
        offset += snprintf(buffer + offset, buffer_size - offset, "%04X: ", address + i);

        // Bytes em hexadecimal
        for (int j = 0; j < 16 && (i + j) < size; j++)
        {
            uint8_t byte = cpu->read_byte(cpu->context, address + i + j);
            offset += snprintf(buffer + offset, buffer_size - offset, "%02X ", byte);
        }

        // Espaço entre hex e ASCII
        for (int j = 0; j < 16 && (i + j) < size; j++)
        {
            uint8_t byte = cpu->read_byte(cpu->context, address + i + j);
            // Mostrar caractere ASCII se for imprimível, ou ponto
            char c = (byte >= 32 && byte < 127) ? byte : '.';
            offset += snprintf(buffer + offset, buffer_size - offset, "%c", c);
        }

        offset += snprintf(buffer + offset, buffer_size - offset, "\n");

        // Verificar se ainda há espaço no buffer
        if (offset >= buffer_size - 32)
        {
            offset += snprintf(buffer + offset, buffer_size - offset, "...\n");
            break;
        }
    }
}

/**
 * @brief Desassemblar um range de código
 */
void z80_debug_disassemble_range(z80_debug_t *debug, uint16_t start, uint16_t end, char *buffer, int buffer_size)
{
    if (!debug || !debug->cpu || !buffer || buffer_size <= 0)
    {
        if (buffer && buffer_size > 0)
        {
            buffer[0] = '\0';
        }
        return;
    }

    z80_t *cpu = debug->cpu;
    int offset = 0;
    uint16_t address = start;

    offset += snprintf(buffer + offset, buffer_size - offset,
                       "Desassembly de 0x%04X até 0x%04X:\n", start, end);

    while (address <= end)
    {
        char disasm[32];
        uint8_t bytes[4];
        int length = z80_get_instruction_length(cpu, address);

        // Ler bytes da instrução
        for (int i = 0; i < length && i < 4; i++)
        {
            bytes[i] = cpu->read_byte(cpu->context, address + i);
        }

        // Desassemblar
        z80_disassemble_instruction(cpu, address, disasm, sizeof(disasm));

        // Formatar saída
        offset += snprintf(buffer + offset, buffer_size - offset, "%04X: ", address);

        // Bytes em hexadecimal
        for (int i = 0; i < length && i < 4; i++)
        {
            offset += snprintf(buffer + offset, buffer_size - offset, "%02X ", bytes[i]);
        }

        // Preencher espaços para alinhar
        for (int i = length; i < 4; i++)
        {
            offset += snprintf(buffer + offset, buffer_size - offset, "   ");
        }

        // Disassembly
        offset += snprintf(buffer + offset, buffer_size - offset, "%s\n", disasm);

        // Avançar para próxima instrução
        address += length;

        // Verificar se ainda há espaço no buffer
        if (offset >= buffer_size - 64)
        {
            offset += snprintf(buffer + offset, buffer_size - offset, "...\n");
            break;
        }
    }
}
