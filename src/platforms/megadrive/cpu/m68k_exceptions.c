#include "m68k_exceptions.h"
#include "m68k.h"
#include <string.h>
#include <stdio.h>

// Tabela de vetores de exceção
static md_m68k_exception_handler_t exception_handlers[64];

// Estado das interrupções e exceções
static struct {
    uint8_t mask;                    // Máscara de interrupção atual (IPL)
    uint8_t pending;                 // Interrupções pendentes
    uint8_t in_service;             // Interrupções em serviço
    uint32_t vector_base;           // Endereço base da tabela de vetores
    uint32_t total_exceptions;      // Total de exceções processadas
    uint32_t total_cycles;          // Total de ciclos gastos em exceções
    md_m68k_exception_timing_t timing_table[64];  // Tabela de timing para cada tipo
} exception_state;

// Cache de exceções para otimização
#define EXCEPTION_CACHE_SIZE 16
static struct {
    md_m68k_exception_info_t info;
    int valid;
    uint32_t timestamp;             // Timestamp para análise de timing
} exception_cache[EXCEPTION_CACHE_SIZE];

static int exception_cache_index = 0;

// Valores padrão de timing para diferentes grupos de exceções
static const md_m68k_exception_timing_t default_timings[] = {
    // Reset
    {4, 6, 4, 4},  // Total: 18 ciclos
    // Erros de bus/endereço
    {6, 8, 6, 4},  // Total: 24 ciclos
    // Instruções ilegais e privilégios
    {4, 6, 4, 4},  // Total: 18 ciclos
    // Interrupções
    {6, 6, 4, 4},  // Total: 20 ciclos
    // TRAPs
    {4, 4, 4, 4}   // Total: 16 ciclos
};

/**
 * @brief Inicializa o sistema de exceções
 */
void md_m68k_init_exceptions(void) {
    // Limpar handlers
    memset(exception_handlers, 0, sizeof(exception_handlers));

    // Inicializar estado
    memset(&exception_state, 0, sizeof(exception_state));
    exception_state.mask = 7; // Todas as interrupções mascaradas inicialmente

    // Configurar timings padrão
    for (int i = 0; i < 64; i++) {
        const md_m68k_exception_timing_t *timing;

        if (i == M68K_EXCEPTION_RESET) {
            timing = &default_timings[0];
        }
        else if (i == M68K_EXCEPTION_BUS_ERROR || i == M68K_EXCEPTION_ADDRESS_ERROR) {
            timing = &default_timings[1];
        }
        else if (i >= M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_1 &&
                 i <= M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_7) {
            timing = &default_timings[3];
        }
        else if (i >= M68K_EXCEPTION_TRAP_0 && i <= M68K_EXCEPTION_TRAP_15) {
            timing = &default_timings[4];
        }
        else {
            timing = &default_timings[2];
        }

        exception_state.timing_table[i] = *timing;
    }

    // Limpar cache
    memset(exception_cache, 0, sizeof(exception_cache));
    exception_cache_index = 0;
}

/**
 * @brief Define timing específico para um tipo de exceção
 */
void md_m68k_set_exception_timing(md_m68k_exception_t type, const md_m68k_exception_timing_t *timing) {
    if (type < 64 && timing != NULL) {
        exception_state.timing_table[type] = *timing;
    }
}

/**
 * @brief Calcula ciclos totais para uma exceção
 */
uint32_t md_m68k_get_exception_cycles(const md_m68k_exception_info_t *info) {
    if (!info) return 0;

    const md_m68k_exception_timing_t *timing = &exception_state.timing_table[info->type];
    return timing->cycles_to_acknowledge +
           timing->cycles_to_process +
           timing->cycles_stack_push +
           timing->cycles_vector_fetch;
}

/**
 * @brief Define um handler para um tipo de exceção
 */
void md_m68k_set_exception_handler(md_m68k_exception_t type, md_m68k_exception_handler_t handler) {
    if (type < 64) {
        exception_handlers[type] = handler;
    }
}

/**
 * @brief Gera uma exceção com timing preciso
 */
void md_m68k_raise_exception(md_m68k_exception_t type, uint32_t address, uint32_t data) {
    md_m68k_exception_info_t info;
    memset(&info, 0, sizeof(info));

    info.type = type;
    info.address = address;
    info.status_register = md_m68k_get_sr();
    info.data = data;
    info.instruction_address = md_m68k_get_pc();
    info.instruction_opcode = md_m68k_read_memory_16(info.instruction_address);

    // Determinar prioridade e grupo
    switch (type) {
        case M68K_EXCEPTION_RESET:
            info.priority = M68K_PRIORITY_RESET;
            info.group_priority = 7;
            break;

        case M68K_EXCEPTION_BUS_ERROR:
        case M68K_EXCEPTION_ADDRESS_ERROR:
            info.priority = M68K_PRIORITY_BUS_ERROR;
            info.group_priority = 6;
            break;

        case M68K_EXCEPTION_ILLEGAL_INSTRUCTION:
        case M68K_EXCEPTION_ZERO_DIVIDE:
        case M68K_EXCEPTION_CHK:
        case M68K_EXCEPTION_TRAPV:
        case M68K_EXCEPTION_PRIVILEGE_VIOLATION:
        case M68K_EXCEPTION_TRACE:
        case M68K_EXCEPTION_LINE_1010:
        case M68K_EXCEPTION_LINE_1111:
            info.priority = M68K_PRIORITY_ILLEGAL_INSTRUCTION;
            info.group_priority = 6;
            break;

        case M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_1:
            info.priority = M68K_PRIORITY_AUTOVECTOR_1;
            info.group_priority = 1;
            break;

        case M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_2:
            info.priority = M68K_PRIORITY_AUTOVECTOR_2;
            info.group_priority = 2;
            break;

        case M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_3:
            info.priority = M68K_PRIORITY_AUTOVECTOR_3;
            info.group_priority = 3;
            break;

        case M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_4:
            info.priority = M68K_PRIORITY_AUTOVECTOR_4;
            info.group_priority = 4;
            break;

        case M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_5:
            info.priority = M68K_PRIORITY_AUTOVECTOR_5;
            info.group_priority = 5;
            break;

        case M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_6:
            info.priority = M68K_PRIORITY_AUTOVECTOR_6;
            info.group_priority = 6;
            break;

        case M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_7:
            info.priority = M68K_PRIORITY_AUTOVECTOR_7;
            info.group_priority = 7;
            break;

        default:
            info.priority = M68K_PRIORITY_TRAP;
            info.group_priority = 6;
            break;
    }

    // Copiar timing específico
    info.timing = exception_state.timing_table[type];

    // Verificar se é uma interrupção mascarável
    if (type >= M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_1 &&
        type <= M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_7) {

        uint8_t level = type - M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_1 + 1;

        // Verificar máscara
        if (level <= exception_state.mask) {
            // Interrupção mascarada, marcar como pendente
            exception_state.pending |= (1 << (level - 1));
            return;
        }

        // Marcar interrupção como em serviço
        exception_state.in_service |= (1 << (level - 1));
    }

    // Adicionar ao cache
    exception_cache[exception_cache_index].info = info;
    exception_cache[exception_cache_index].valid = 1;
    exception_cache[exception_cache_index].timestamp = md_m68k_get_cycles();
    exception_cache_index = (exception_cache_index + 1) % EXCEPTION_CACHE_SIZE;

    // Atualizar estatísticas
    exception_state.total_exceptions++;
    exception_state.total_cycles += md_m68k_get_exception_cycles(&info);

    // Chamar handler se existir
    if (type < 64 && exception_handlers[type]) {
        // Salvar contexto
        uint32_t old_pc = md_m68k_get_pc();
        uint16_t old_sr = md_m68k_get_sr();

        // Empilhar PC e SR com timing preciso
        uint32_t sp = md_m68k_get_addr_reg(7) - 6;
        md_m68k_set_addr_reg(7, sp);

        // Adicionar ciclos de stack push
        md_m68k_add_cycles(info.timing.cycles_stack_push);

        md_m68k_write_memory_32(sp, old_pc);
        md_m68k_write_memory_16(sp + 4, old_sr);

        // Atualizar SR (modo supervisor e máscara de interrupção)
        uint16_t new_sr = old_sr | 0x2000; // Set supervisor bit
        if (info.priority > M68K_PRIORITY_AUTOVECTOR_1) {
            new_sr = (new_sr & 0xF8FF) | ((info.priority & 7) << 8);
        }
        md_m68k_set_sr(new_sr);

        // Calcular endereço do vetor
        uint32_t vector_address = exception_state.vector_base + (type * 4);

        // Adicionar ciclos de fetch do vetor
        md_m68k_add_cycles(info.timing.cycles_vector_fetch);

        // Ler novo PC do vetor
        uint32_t new_pc = md_m68k_read_memory_32(vector_address);
        md_m68k_set_pc(new_pc);

        // Adicionar ciclos de processamento
        md_m68k_add_cycles(info.timing.cycles_to_process);

        // Chamar handler
        exception_handlers[type](&info);
    }
}

/**
 * @brief Define a máscara de interrupção
 */
void md_m68k_set_interrupt_mask(uint8_t mask) {
    exception_state.mask = mask & 7;

    // Verificar interrupções pendentes que podem ser processadas agora
    if (exception_state.pending) {
        for (int i = 7; i > exception_state.mask; i--) {
            if (exception_state.pending & (1 << (i - 1))) {
                // Limpar flag pendente
                exception_state.pending &= ~(1 << (i - 1));

                // Gerar exceção
                md_m68k_raise_exception(
                    M68K_EXCEPTION_INTERRUPT_AUTOVECTOR_1 + i - 1,
                    md_m68k_get_pc(),
                    0
                );
                break;
            }
        }
    }
}

/**
 * @brief Obtém a máscara de interrupção atual
 */
uint8_t md_m68k_get_interrupt_mask(void) {
    return exception_state.mask;
}

/**
 * @brief Verifica se há interrupções pendentes
 */
int32_t md_m68k_check_interrupts(void) {
    if (!exception_state.pending) {
        return 0;
    }

    // Verificar cada nível de interrupção, do mais alto para o mais baixo
    for (int i = 7; i > exception_state.mask; i--) {
        if (exception_state.pending & (1 << (i - 1))) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Reconhece uma interrupção específica
 */
void md_m68k_acknowledge_interrupt(uint8_t level) {
    if (level > 0 && level <= 7) {
        // Limpar flag de serviço
        exception_state.in_service &= ~(1 << (level - 1));
    }
}

/**
 * @brief Define o endereço base dos vetores
 */
void md_m68k_set_vector_base(uint32_t address) {
    exception_state.vector_base = address;
}

/**
 * @brief Obtém o endereço base dos vetores
 */
uint32_t md_m68k_get_vector_base(void) {
    return exception_state.vector_base;
}

/**
 * @brief Obtém estatísticas de exceções
 */
void md_m68k_get_exception_stats(uint32_t *total_exceptions, uint32_t *cycles_spent) {
    if (total_exceptions) {
        *total_exceptions = exception_state.total_exceptions;
    }
    if (cycles_spent) {
        *cycles_spent = exception_state.total_cycles;
    }
}

/**
 * @brief Reseta estatísticas de exceções
 */
void md_m68k_reset_exception_stats(void) {
    exception_state.total_exceptions = 0;
    exception_state.total_cycles = 0;
}
