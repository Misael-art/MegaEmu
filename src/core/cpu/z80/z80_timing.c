/**
 * @file z80_timing.c
 * @brief Implementação do sistema de timing refinado para o processador Z80
 */

#include <stdlib.h>
#include <string.h>
#include "z80_timing.h"
#include "z80_internal.h"

// Estado de timing interno para o Z80
static z80_timing_state_t *z80_get_timing_state(z80_t *cpu)
{
    // Acessar o estado de timing armazenado no contexto extendido do Z80
    if (cpu && cpu->extended_context)
    {
        return (z80_timing_state_t *)cpu->extended_context;
    }
    return NULL;
}

/**
 * @brief Inicializar configuração de timing
 */
void z80_timing_init(z80_t *cpu, z80_timing_config_t *config)
{
    if (!cpu || !config)
    {
        return;
    }

    // Alocar ou reutilizar o estado de timing
    z80_timing_state_t *timing_state = z80_get_timing_state(cpu);
    if (!timing_state)
    {
        timing_state = (z80_timing_state_t *)malloc(sizeof(z80_timing_state_t));
        if (!timing_state)
        {
            return; // Falha na alocação de memória
        }
        memset(timing_state, 0, sizeof(z80_timing_state_t));
        cpu->extended_context = timing_state;
    }

    // Copiar configuração
    memcpy(&timing_state->config, config, sizeof(z80_timing_config_t));

    // Inicializar contadores
    timing_state->total_cycles = 0;
    timing_state->last_sync_cycles = 0;
    timing_state->timing_enabled = true;
}

/**
 * @brief Calcular ciclos exatos para instrução específica com base na plataforma
 */
int z80_calculate_instruction_cycles(z80_t *cpu, uint8_t opcode, uint8_t *operands, int operand_count)
{
    z80_timing_state_t *timing_state = z80_get_timing_state(cpu);
    if (!timing_state || !timing_state->timing_enabled)
    {
        // Se timing não estiver configurado, usar valores padrão
        return z80_get_instruction_cycles(opcode);
    }

    // Ciclos base da instrução
    int base_cycles = z80_get_instruction_cycles(opcode);

    // Ajustar com base na plataforma específica
    switch (timing_state->config.platform_type)
    {
    case Z80_PLATFORM_MASTER_SYSTEM:
        // Master System: possível ajuste para sincronização com VDP
        if (timing_state->config.sync_with_vdp)
        {
            // Ajustes específicos do Master System, se necessário
        }
        break;

    case Z80_PLATFORM_MEGA_DRIVE:
        // Mega Drive: possíveis estados de espera ao acessar YM2612/PSG
        // Por exemplo, adicionar ciclos extras se opcode acessa região específica
        break;

    case Z80_PLATFORM_NEO_GEO:
        // Neo Geo: possíveis ajustes para YM2610 e ADPCM
        break;

    default:
        // Plataforma genérica: sem ajustes adicionais
        break;
    }

    return base_cycles;
}

/**
 * @brief Calcular timing específico para acesso à memória
 */
int z80_memory_access_timing(z80_t *cpu, uint16_t address, bool is_read, bool is_opcode_fetch)
{
    z80_timing_state_t *timing_state = z80_get_timing_state(cpu);
    if (!timing_state || !timing_state->timing_enabled)
    {
        return 0; // Sem ciclos adicionais
    }

    int additional_cycles = 0;

    // Verificar estados de espera específicos da plataforma
    if (timing_state->config.calculate_wait_states)
    {
        additional_cycles += timing_state->config.calculate_wait_states(
            timing_state->config.platform_context, address, is_read);
    }
    else
    {
        // Estados de espera padrão, se configurados
        additional_cycles += timing_state->config.memory_wait_states;
    }

    // Verificar contention de memória, se aplicável
    if (timing_state->config.has_memory_contention)
    {
        if (timing_state->config.calculate_contention)
        {
            additional_cycles += timing_state->config.calculate_contention(
                timing_state->config.platform_context, address, additional_cycles);
        }
        else if (timing_state->config.contention_mask[address] != 0)
        {
            // Usar mapa de contention
            additional_cycles += timing_state->config.contention_mask[address];
        }
    }

    return additional_cycles;
}

/**
 * @brief Sincronizar com subsistema de vídeo
 */
void z80_sync_with_vdp(z80_t *cpu, int executed_cycles)
{
    z80_timing_state_t *timing_state = z80_get_timing_state(cpu);
    if (!timing_state || !timing_state->timing_enabled || !timing_state->config.sync_with_vdp)
    {
        return;
    }

    // Atualizar contadores de ciclos
    timing_state->total_cycles += executed_cycles;

    // Calcular ciclos desde a última sincronização
    int64_t cycles_since_sync = timing_state->total_cycles - timing_state->last_sync_cycles;

    // Se for necessário sincronizar com o VDP
    if (cycles_since_sync >= 100)
    { // Valor arbitrário para exemplo; ajustar conforme necessário
        // Aqui seria chamada uma função do subsistema de vídeo para sincronização
        // Por exemplo: vdp_update(timing_state->config.platform_context, cycles_since_sync);

        // Atualizar timestamp da última sincronização
        timing_state->last_sync_cycles = timing_state->total_cycles;
    }
}

/**
 * @brief Obter os ciclos de uma instrução específica
 *
 * Função helper para obter os ciclos base de uma instrução
 */
int z80_get_instruction_cycles(uint8_t opcode)
{
    // Esta é uma tabela simplificada; em um emulador real, seria muito mais complexa
    // e provavelmente incluiria tabelas separadas para instruções com prefixos CB, DD, ED, FD
    static const uint8_t cycles_table[256] = {
        4, 10, 7, 6, 4, 4, 7, 4, 4, 11, 7, 6, 4, 4, 7, 4,          // 0x00-0x0F
        8, 10, 7, 6, 4, 4, 7, 4, 12, 11, 7, 6, 4, 4, 7, 4,         // 0x10-0x1F
        7, 10, 16, 6, 4, 4, 7, 4, 7, 11, 16, 6, 4, 4, 7, 4,        // 0x20-0x2F
        7, 10, 13, 6, 11, 11, 10, 4, 7, 11, 13, 6, 4, 4, 7, 4,     // 0x30-0x3F
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 0x40-0x4F
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 0x50-0x5F
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 0x60-0x6F
        7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,            // 0x70-0x7F
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 0x80-0x8F
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 0x90-0x9F
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 0xA0-0xAF
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 0xB0-0xBF
        5, 10, 10, 10, 10, 11, 7, 11, 5, 10, 10, 0, 10, 17, 7, 11, // 0xC0-0xCF
        5, 10, 10, 11, 10, 11, 7, 11, 5, 4, 10, 11, 10, 0, 7, 11,  // 0xD0-0xDF
        5, 10, 10, 19, 10, 11, 7, 11, 5, 4, 10, 4, 10, 0, 7, 11,   // 0xE0-0xEF
        5, 10, 10, 4, 10, 11, 7, 11, 5, 6, 10, 4, 10, 0, 7, 11     // 0xF0-0xFF
    };

    return cycles_table[opcode];
}

/**
 * @brief Liberar recursos do timing
 */
void z80_timing_shutdown(z80_t *cpu)
{
    if (cpu && cpu->extended_context)
    {
        free(cpu->extended_context);
        cpu->extended_context = NULL;
    }
}
