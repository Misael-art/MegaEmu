/**
 * @file z80_optimized.c
 * @brief Implementação otimizada do Z80 para casos específicos
 */

#include <stdlib.h>
#include <string.h>
#include "z80.h"
#include "z80_internal.h"

// Tamanho do cache de instruções
#define Z80_INSTRUCTION_CACHE_SIZE 64

// Cache de instruções para evitar decodificação repetida
typedef struct
{
    uint8_t opcode[4];                 // Opcode com até 4 bytes (prefixos)
    uint8_t length;                    // Comprimento da instrução em bytes
    uint8_t cycles;                    // Ciclos base
    int (*handler)(z80_t *, uint16_t); // Função manipuladora
    uint32_t execution_count;          // Contador de execuções (para estatísticas)
    uint16_t last_pc;                  // Último PC onde esta instrução foi executada
} z80_instruction_cache_entry_t;

// Estrutura de dados para otimizações
typedef struct
{
    // Cache circular de instruções
    z80_instruction_cache_entry_t instruction_cache[Z80_INSTRUCTION_CACHE_SIZE];
    int cache_index;

    // Estatísticas
    uint32_t cache_hits;
    uint32_t cache_misses;

    // Flags para otimizações específicas
    bool enable_audio_fast_path;   // Otimização para processamento de áudio
    bool enable_instruction_cache; // Cache de instruções

    // Contexto de áudio para otimizações específicas
    void *audio_context;

    // Funções de callback otimizadas para áudio
    uint8_t (*fast_read_audio_reg)(void *context, uint16_t address);
    void (*fast_write_audio_reg)(void *context, uint16_t address, uint8_t value);
} z80_optimizations_t;

// Obter o contexto de otimização
static z80_optimizations_t *z80_get_optimizations(z80_t *cpu)
{
    if (!cpu || !cpu->optimization_context)
    {
        return NULL;
    }
    return (z80_optimizations_t *)cpu->optimization_context;
}

/**
 * @brief Inicializar otimizações do Z80
 *
 * @param cpu Ponteiro para instância do Z80
 * @return true se inicializado com sucesso, false caso contrário
 */
bool z80_optimizations_init(z80_t *cpu)
{
    if (!cpu)
    {
        return false;
    }

    // Alocar e inicializar contexto de otimização
    z80_optimizations_t *opts = (z80_optimizations_t *)malloc(sizeof(z80_optimizations_t));
    if (!opts)
    {
        return false;
    }

    // Limpar toda a estrutura
    memset(opts, 0, sizeof(z80_optimizations_t));

    // Configurações iniciais
    opts->enable_instruction_cache = true;
    opts->enable_audio_fast_path = false;
    opts->cache_index = 0;

    // Armazenar no contexto da CPU
    cpu->optimization_context = opts;

    return true;
}

/**
 * @brief Liberar recursos das otimizações
 *
 * @param cpu Ponteiro para instância do Z80
 */
void z80_optimizations_shutdown(z80_t *cpu)
{
    if (cpu && cpu->optimization_context)
    {
        free(cpu->optimization_context);
        cpu->optimization_context = NULL;
    }
}

/**
 * @brief Configurar otimizações para processamento de áudio
 *
 * @param cpu Ponteiro para instância do Z80
 * @param audio_context Contexto específico para otimizações de áudio
 * @param fast_read_audio_reg Callback otimizado para leitura de registradores de áudio
 * @param fast_write_audio_reg Callback otimizado para escrita em registradores de áudio
 * @return true se configurado com sucesso, false caso contrário
 */
bool z80_configure_audio_optimizations(z80_t *cpu, void *audio_context,
                                       uint8_t (*fast_read_audio_reg)(void *context, uint16_t address),
                                       void (*fast_write_audio_reg)(void *context, uint16_t address, uint8_t value))
{
    z80_optimizations_t *opts = z80_get_optimizations(cpu);
    if (!opts)
    {
        return false;
    }

    opts->audio_context = audio_context;
    opts->fast_read_audio_reg = fast_read_audio_reg;
    opts->fast_write_audio_reg = fast_write_audio_reg;
    opts->enable_audio_fast_path = (audio_context != NULL &&
                                    fast_read_audio_reg != NULL &&
                                    fast_write_audio_reg != NULL);

    return true;
}

/**
 * @brief Obter estatísticas de otimização
 *
 * @param cpu Ponteiro para instância do Z80
 * @param cache_hits Ponteiro para armazenar número de cache hits
 * @param cache_misses Ponteiro para armazenar número de cache misses
 * @return true se estatísticas foram obtidas com sucesso, false caso contrário
 */
bool z80_get_optimization_stats(z80_t *cpu, uint32_t *cache_hits, uint32_t *cache_misses)
{
    z80_optimizations_t *opts = z80_get_optimizations(cpu);
    if (!opts)
    {
        return false;
    }

    if (cache_hits)
    {
        *cache_hits = opts->cache_hits;
    }

    if (cache_misses)
    {
        *cache_misses = opts->cache_misses;
    }

    return true;
}

/**
 * @brief Limpar o cache de instruções
 *
 * @param cpu Ponteiro para instância do Z80
 */
void z80_clear_instruction_cache(z80_t *cpu)
{
    z80_optimizations_t *opts = z80_get_optimizations(cpu);
    if (!opts)
    {
        return;
    }

    // Limpar o cache
    memset(opts->instruction_cache, 0, sizeof(opts->instruction_cache));
    opts->cache_index = 0;
}

/**
 * @brief Versão otimizada de z80_execute para casos específicos
 *
 * @param cpu Ponteiro para instância do Z80
 * @param cycles Número de ciclos a executar
 * @return Número de ciclos executados
 */
int z80_execute_optimized(z80_t *cpu, int cycles)
{
    if (!cpu || cycles <= 0)
    {
        return 0;
    }

    z80_optimizations_t *opts = z80_get_optimizations(cpu);
    if (!opts)
    {
        // Sem contexto de otimização, voltar para implementação padrão
        return z80_execute(cpu, cycles);
    }

    int total_executed = 0;

    // Loop principal de execução
    while (cycles > 0)
    {
        // Verificar interrupções usando fast path
        if (cpu->iff1 && cpu->irq_pending)
        {
            int irq_cycles = z80_process_interrupt(cpu);
            total_executed += irq_cycles;
            cycles -= irq_cycles;
            continue;
        }

        // Otimização: Cache de instruções
        if (opts->enable_instruction_cache)
        {
            uint16_t pc = cpu->pc;
            uint8_t opcode = cpu->read_byte(cpu->context, pc);

            // Verificar cache para esta instrução
            bool cache_hit = false;
            for (int i = 0; i < Z80_INSTRUCTION_CACHE_SIZE; i++)
            {
                z80_instruction_cache_entry_t *entry = &opts->instruction_cache[i];
                if (entry->length > 0 && entry->opcode[0] == opcode && entry->last_pc == pc)
                {
                    // Potencial hit no cache, verificar opcode completo para prefixos
                    bool full_match = true;
                    for (int j = 1; j < entry->length; j++)
                    {
                        if (pc + j >= 0x10000)
                        {
                            full_match = false;
                            break;
                        }
                        uint8_t next_byte = cpu->read_byte(cpu->context, pc + j);
                        if (next_byte != entry->opcode[j])
                        {
                            full_match = false;
                            break;
                        }
                    }

                    if (full_match)
                    {
                        // Cache hit! Executar instrução diretamente
                        cpu->pc += entry->length;
                        int instr_cycles = entry->handler(cpu, opcode);

                        // Atualizar estatísticas
                        entry->execution_count++;
                        opts->cache_hits++;

                        total_executed += instr_cycles;
                        cycles -= instr_cycles;
                        cache_hit = true;
                        break;
                    }
                }
            }

            if (cache_hit)
            {
                continue;
            }

            // Cache miss
            opts->cache_misses++;
        }

        // Otimização: Fast path para processamento de áudio
        if (opts->enable_audio_fast_path)
        {
            uint16_t pc = cpu->pc;
            uint8_t opcode = cpu->read_byte(cpu->context, pc);

            // Verificar padrões comuns de acesso a registradores de áudio
            // Por exemplo, instruções de acesso a portas I/O específicas
            if ((opcode == 0xD3 || opcode == 0xDB) && // OUT (n),A ou IN A,(n)
                (pc + 1 < 0x10000))
            {
                uint8_t port = cpu->read_byte(cpu->context, pc + 1);

                // Verificar se é um acesso a região de áudio
                // Exemplo: no Mega Drive, acessos aos registradores YM2612/PSG
                if ((port >= 0x40 && port <= 0x5F) || // YM2612
                    (port >= 0x70 && port <= 0x7F))
                { // PSG

                    if (opcode == 0xD3)
                    { // OUT (n),A
                        // Escrever diretamente no registrador de áudio
                        opts->fast_write_audio_reg(opts->audio_context, port, cpu->regs.a);
                    }
                    else
                    { // IN A,(n)
                        // Ler diretamente do registrador de áudio
                        cpu->regs.a = opts->fast_read_audio_reg(opts->audio_context, port);
                    }

                    // Avançar PC e atualizar ciclos
                    cpu->pc += 2;
                    int instr_cycles = 11; // Ciclos típicos para IN/OUT
                    total_executed += instr_cycles;
                    cycles -= instr_cycles;
                    continue;
                }
            }
        }

        // Fallback para execução padrão
        int instr_cycles = z80_execute_instruction(cpu);

        // Se o cache de instruções estiver habilitado, adicionar ao cache
        if (opts->enable_instruction_cache)
        {
            uint16_t old_pc = cpu->pc - cpu->last_instruction_size;

            // Alocar nova entrada no cache
            z80_instruction_cache_entry_t *entry = &opts->instruction_cache[opts->cache_index];
            entry->length = cpu->last_instruction_size;
            entry->cycles = instr_cycles;
            entry->handler = cpu->last_instruction_handler;
            entry->execution_count = 1;
            entry->last_pc = old_pc;

            // Armazenar bytes do opcode
            for (int i = 0; i < entry->length && i < 4; i++)
            {
                entry->opcode[i] = cpu->read_byte(cpu->context, old_pc + i);
            }

            // Avançar índice do cache circular
            opts->cache_index = (opts->cache_index + 1) % Z80_INSTRUCTION_CACHE_SIZE;
        }

        total_executed += instr_cycles;
        cycles -= instr_cycles;
    }

    return total_executed;
}
