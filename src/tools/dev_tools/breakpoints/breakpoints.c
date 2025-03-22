/**
 * @file breakpoints.c
 * @brief Implementação da API para breakpoints condicionais avançados
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "breakpoints.h"

// Identificador inválido para breakpoints
#define INVALID_BREAKPOINT_ID 0

// Número padrão máximo de breakpoints se não especificado
#define DEFAULT_MAX_BREAKPOINTS 256

// Estrutura de implementação para contexto de breakpoints
struct breakpoint_context_t
{
    uint32_t platform_id;        // ID da plataforma
    uint32_t max_breakpoints;    // Máximo de breakpoints permitido
    uint32_t next_breakpoint_id; // Próximo ID a ser usado
    uint32_t breakpoint_count;   // Número atual de breakpoints
    breakpoint_t *breakpoints;   // Array de breakpoints

    // Armazenamento para valores anteriores (para detecção de mudanças)
    uint8_t *last_values;           // Último valor lido para cada endereço
    uint32_t *last_value_addresses; // Endereços correspondentes
    uint32_t max_last_values;       // Tamanho do buffer de valores anteriores
    uint32_t last_value_count;      // Número de endereços monitorados

    // Callback para notificação de breakpoints ativados
    breakpoint_callback_t callback; // Função de callback
    void *callback_user_data;       // Dados de usuário para o callback

    // Cache de última verificação (otimização)
    uint32_t last_pc_checked; // Último PC verificado
    int32_t last_pc_result;   // Resultado da última verificação de PC

    // Estado de execução
    uint32_t current_cycle_count; // Contador de ciclos atual
    bool had_breakpoint_hit;      // Se houve breakpoint atingido neste ciclo
};

// Utilitários internos

/**
 * @brief Encontra índice de um breakpoint pelo ID
 */
static int32_t find_breakpoint_index(breakpoint_context_t *context, int32_t id)
{
    if (!context || id == INVALID_BREAKPOINT_ID)
        return -1;

    for (uint32_t i = 0; i < context->breakpoint_count; i++)
    {
        if (context->breakpoints[i].id == id)
        {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Verifica se uma condição de breakpoint é satisfeita
 */
static bool check_breakpoint_condition(
    const breakpoint_t *bp,
    uint32_t value,
    uint32_t previous_value)
{
    if (!bp)
        return false;

    // Aplicar máscara se definida
    if (bp->mask != 0)
    {
        value &= bp->mask;
        previous_value &= bp->mask;
    }

    // Verificar condição
    switch (bp->cmp)
    {
    case BP_CMP_EQUAL:
        return value == bp->value;

    case BP_CMP_NOT_EQUAL:
        return value != bp->value;

    case BP_CMP_GREATER:
        return value > bp->value;

    case BP_CMP_GREATER_OR_EQUAL:
        return value >= bp->value;

    case BP_CMP_LESS:
        return value < bp->value;

    case BP_CMP_LESS_OR_EQUAL:
        return value <= bp->value;

    case BP_CMP_BITWISE_AND:
        return (value & bp->value) != 0;

    case BP_CMP_BITWISE_NAND:
        return (value & bp->value) == 0;

    case BP_CMP_BITWISE_OR:
        return (value | bp->value) != 0;

    case BP_CMP_BITWISE_NOR:
        return (value | bp->value) == 0;

    case BP_CMP_BITWISE_XOR:
        return (value ^ bp->value) != 0;

    case BP_CMP_BITWISE_XNOR:
        return (value ^ bp->value) == 0;

    case BP_CMP_CHANGED:
        return value != previous_value;

    case BP_CMP_CHANGED_TO:
        return value == bp->value && value != previous_value;

    case BP_CMP_CHANGED_FROM:
        return previous_value == bp->value && value != previous_value;

    case BP_CMP_IN_RANGE:
        return value >= bp->value && value <= bp->value_end;

    case BP_CMP_NOT_IN_RANGE:
        return value < bp->value || value > bp->value_end;

    default:
        return false;
    }
}

/**
 * @brief Busca ou adiciona um endereço no cache de valores anteriores
 */
static uint32_t find_or_add_address_index(
    breakpoint_context_t *context,
    uint32_t address)
{
    if (!context)
        return UINT32_MAX;

    // Buscar endereço existente
    for (uint32_t i = 0; i < context->last_value_count; i++)
    {
        if (context->last_value_addresses[i] == address)
        {
            return i;
        }
    }

    // Se não encontrar, adicionar novo se houver espaço
    if (context->last_value_count < context->max_last_values)
    {
        uint32_t index = context->last_value_count++;
        context->last_value_addresses[index] = address;
        context->last_values[index] = 0; // Valor inicial
        return index;
    }

    // Se não houver espaço, substituir o primeiro (mais antigo)
    // Em uma implementação mais robusta, usaríamos uma estratégia de cache melhor
    context->last_value_addresses[0] = address;
    context->last_values[0] = 0;
    return 0;
}

/**
 * @brief Atualiza o valor no cache para um endereço e retorna o valor anterior
 */
static uint8_t update_address_value(
    breakpoint_context_t *context,
    uint32_t address,
    uint8_t new_value)
{
    if (!context)
        return 0;

    uint32_t index = find_or_add_address_index(context, address);
    if (index == UINT32_MAX)
        return 0;

    uint8_t old_value = context->last_values[index];
    context->last_values[index] = new_value;
    return old_value;
}

/**
 * @brief Obtém o valor anterior para um endereço
 */
static uint8_t get_previous_value(
    breakpoint_context_t *context,
    uint32_t address)
{
    if (!context)
        return 0;

    for (uint32_t i = 0; i < context->last_value_count; i++)
    {
        if (context->last_value_addresses[i] == address)
        {
            return context->last_values[i];
        }
    }

    return 0; // Valor não encontrado
}

/**
 * @brief Cria e inicializa um novo breakpoint
 */
static bool init_breakpoint(
    breakpoint_t *bp,
    uint32_t id,
    breakpoint_type_t type,
    uint32_t address,
    uint32_t flags)
{
    if (!bp)
        return false;

    memset(bp, 0, sizeof(breakpoint_t));

    bp->id = id;
    bp->type = type;
    bp->address = address;
    bp->address_end = address;
    bp->flags = flags | BP_FLAG_ENABLED; // Habilitado por padrão
    bp->mask = 0xFFFFFFFF;               // Sem máscara (todos os bits)

    // Condições padrão dependendo do tipo
    switch (type)
    {
    case BP_TYPE_EXECUTION:
        bp->cmp = BP_CMP_EQUAL;
        break;

    case BP_TYPE_MEMORY_READ:
    case BP_TYPE_MEMORY_WRITE:
        bp->cmp = BP_CMP_EQUAL;
        break;

    case BP_TYPE_INTERRUPT:
        bp->cmp = BP_CMP_EQUAL;
        break;

    case BP_TYPE_REGISTER:
        bp->cmp = BP_CMP_EQUAL;
        break;

    case BP_TYPE_CYCLE_COUNT:
        bp->cmp = BP_CMP_GREATER_OR_EQUAL;
        break;

    case BP_TYPE_VALUE_CHANGE:
        bp->cmp = BP_CMP_CHANGED;
        break;

    case BP_TYPE_EXPRESSION:
        // Expressão é especial, será definida separadamente
        bp->condition[0] = '\0';
        break;

    default:
        return false;
    }

    return true;
}

// Implementação da API pública

breakpoint_context_t *breakpoint_create_context(
    uint32_t platform_id,
    uint32_t max_breakpoints)
{
    // Alocar estrutura de contexto
    breakpoint_context_t *context = calloc(1, sizeof(breakpoint_context_t));
    if (!context)
        return NULL;

    // Definir tamanho máximo
    context->max_breakpoints = max_breakpoints > 0 ? max_breakpoints : DEFAULT_MAX_BREAKPOINTS;

    // Alocar memória para breakpoints
    context->breakpoints = calloc(context->max_breakpoints, sizeof(breakpoint_t));
    if (!context->breakpoints)
    {
        free(context);
        return NULL;
    }

    // Alocar memória para cache de valores
    context->max_last_values = 1024; // Valor arbitrário, ajustar conforme necessário
    context->last_values = calloc(context->max_last_values, sizeof(uint8_t));
    context->last_value_addresses = calloc(context->max_last_values, sizeof(uint32_t));

    if (!context->last_values || !context->last_value_addresses)
    {
        free(context->breakpoints);
        free(context->last_values);
        free(context->last_value_addresses);
        free(context);
        return NULL;
    }

    // Inicializar contexto
    context->platform_id = platform_id;
    context->next_breakpoint_id = 1; // IDs começam em 1
    context->breakpoint_count = 0;
    context->last_value_count = 0;
    context->callback = NULL;
    context->callback_user_data = NULL;
    context->last_pc_checked = 0xFFFFFFFF;
    context->last_pc_result = -1;
    context->current_cycle_count = 0;
    context->had_breakpoint_hit = false;

    return context;
}

void breakpoint_destroy_context(breakpoint_context_t *context)
{
    if (!context)
        return;

    // Liberar memória alocada
    free(context->breakpoints);
    free(context->last_values);
    free(context->last_value_addresses);

    // Liberar o contexto
    free(context);
}

bool breakpoint_set_callback(
    breakpoint_context_t *context,
    breakpoint_callback_t callback,
    void *user_data)
{
    if (!context)
        return false;

    context->callback = callback;
    context->callback_user_data = user_data;

    return true;
}

int32_t breakpoint_add(
    breakpoint_context_t *context,
    breakpoint_type_t type,
    uint32_t address,
    uint32_t flags)
{
    if (!context)
        return BP_ERROR_INVALID_PARAMS;

    // Verificar se há espaço para mais breakpoints
    if (context->breakpoint_count >= context->max_breakpoints)
    {
        return BP_ERROR_LIMIT_REACHED;
    }

    // Criar novo breakpoint
    uint32_t index = context->breakpoint_count++;
    uint32_t id = context->next_breakpoint_id++;

    if (!init_breakpoint(&context->breakpoints[index], id, type, address, flags))
    {
        context->breakpoint_count--; // Reverter adição
        return BP_ERROR_INVALID_PARAMS;
    }

    return id;
}

int32_t breakpoint_add_conditional(
    breakpoint_context_t *context,
    breakpoint_type_t type,
    uint32_t address,
    breakpoint_compare_t cmp,
    uint32_t value,
    uint32_t flags)
{
    if (!context)
        return BP_ERROR_INVALID_PARAMS;

    // Adicionar breakpoint básico primeiro
    int32_t id = breakpoint_add(context, type, address, flags);
    if (id < 0)
        return id;

    // Encontrar o índice do breakpoint recém-adicionado
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return BP_ERROR_NOT_FOUND;

    // Configurar condição
    context->breakpoints[index].cmp = cmp;
    context->breakpoints[index].value = value;

    return id;
}

int32_t breakpoint_add_with_expression(
    breakpoint_context_t *context,
    breakpoint_type_t type,
    uint32_t address,
    const char *condition,
    uint32_t flags)
{
    if (!context || !condition)
        return BP_ERROR_INVALID_PARAMS;

    // Adicionar breakpoint básico primeiro
    int32_t id = breakpoint_add(context, type, address, flags | BP_FLAG_CONDITION);
    if (id < 0)
        return id;

    // Encontrar o índice do breakpoint recém-adicionado
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return BP_ERROR_NOT_FOUND;

    // Copiar expressão
    strncpy(context->breakpoints[index].condition, condition,
            sizeof(context->breakpoints[index].condition) - 1);
    context->breakpoints[index].condition[sizeof(context->breakpoints[index].condition) - 1] = '\0';

    return id;
}

int32_t breakpoint_add_range(
    breakpoint_context_t *context,
    breakpoint_type_t type,
    uint32_t address_start,
    uint32_t address_end,
    uint32_t flags)
{
    if (!context || address_start > address_end)
        return BP_ERROR_INVALID_PARAMS;

    // Adicionar breakpoint básico primeiro
    int32_t id = breakpoint_add(context, type, address_start, flags);
    if (id < 0)
        return id;

    // Encontrar o índice do breakpoint recém-adicionado
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return BP_ERROR_NOT_FOUND;

    // Configurar range
    context->breakpoints[index].address_end = address_end;

    return id;
}

bool breakpoint_remove(
    breakpoint_context_t *context,
    int32_t id)
{
    if (!context)
        return false;

    // Encontrar índice do breakpoint
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return false;

    // Mover último breakpoint para a posição do removido
    if (index < (int32_t)context->breakpoint_count - 1)
    {
        memcpy(&context->breakpoints[index],
               &context->breakpoints[context->breakpoint_count - 1],
               sizeof(breakpoint_t));
    }

    // Reduzir contador
    context->breakpoint_count--;

    return true;
}

bool breakpoint_enable(
    breakpoint_context_t *context,
    int32_t id,
    bool enable)
{
    if (!context)
        return false;

    // Encontrar índice do breakpoint
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return false;

    // Atualizar flag de habilitado
    if (enable)
    {
        context->breakpoints[index].flags |= BP_FLAG_ENABLED;
    }
    else
    {
        context->breakpoints[index].flags &= ~BP_FLAG_ENABLED;
    }

    return true;
}

bool breakpoint_check_execution(
    breakpoint_context_t *context,
    uint32_t pc,
    const void *regs,
    int32_t *id_out)
{
    if (!context)
        return false;

    // Verificar cache
    if (pc == context->last_pc_checked)
    {
        if (id_out && context->last_pc_result >= 0)
        {
            *id_out = context->last_pc_result;
        }
        return context->last_pc_result >= 0;
    }

    // Resetar resultado
    context->last_pc_checked = pc;
    context->last_pc_result = -1;

    // Verificar todos os breakpoints habilitados
    for (uint32_t i = 0; i < context->breakpoint_count; i++)
    {
        breakpoint_t *bp = &context->breakpoints[i];

        // Ignorar breakpoints desabilitados
        if (!(bp->flags & BP_FLAG_ENABLED))
            continue;

        // Verificar tipo e condição
        bool triggered = false;

        switch (bp->type)
        {
        case BP_TYPE_EXECUTION:
            // Verificar se PC está no range do breakpoint
            if (pc >= bp->address && pc <= bp->address_end)
            {
                triggered = true;
            }
            break;

        case BP_TYPE_CYCLE_COUNT:
            // Verificar contador de ciclos
            if (context->current_cycle_count >= bp->value)
            {
                triggered = true;
            }
            break;

        // Outros tipos não são pertinentes para verificação de execução
        default:
            continue;
        }

        // Se o breakpoint foi ativado
        if (triggered)
        {
            // Incrementar contador de hits
            bp->hit_count++;

            // Verificar contador de hits se necessário
            if ((bp->flags & BP_FLAG_COUNTER) && bp->hit_count < bp->hit_count_target)
            {
                continue; // Não atingiu o alvo ainda
            }

            // Verificar expressão condicional se presente
            if ((bp->flags & BP_FLAG_CONDITION) && bp->condition[0] != '\0')
            {
                // Aqui seria necessário um avaliador de expressões
                // Esta é uma implementação simplificada
                if (strcmp(bp->condition, "true") != 0)
                {
                    continue; // Condição não satisfeita
                }
            }

            // Breakpoint ativado!
            context->last_pc_result = bp->id;

            // Se for temporário, remover
            if (bp->flags & BP_FLAG_TEMPORARY)
            {
                breakpoint_remove(context, bp->id);
            }

            // Chamar callback se definido
            if (context->callback)
            {
                context->callback(context, bp, context->callback_user_data);
            }

            // Retornar ID se solicitado
            if (id_out)
            {
                *id_out = bp->id;
            }

            return true;
        }
    }

    return false;
}

bool breakpoint_check_memory(
    breakpoint_context_t *context,
    uint32_t address,
    uint32_t value,
    bool is_write,
    int32_t *id_out)
{
    if (!context)
        return false;

    // Obter valor anterior
    uint8_t previous_value = get_previous_value(context, address);

    // Atualizar valor armazenado (apenas se for escrita)
    if (is_write)
    {
        update_address_value(context, address, (uint8_t)value);
    }

    // Verificar breakpoints
    for (uint32_t i = 0; i < context->breakpoint_count; i++)
    {
        breakpoint_t *bp = &context->breakpoints[i];

        // Ignorar breakpoints desabilitados
        if (!(bp->flags & BP_FLAG_ENABLED))
            continue;

        // Verificar tipo correto
        if ((is_write && bp->type != BP_TYPE_MEMORY_WRITE) ||
            (!is_write && bp->type != BP_TYPE_MEMORY_READ && bp->type != BP_TYPE_VALUE_CHANGE))
        {
            continue;
        }

        // Verificar endereço no range
        if (address < bp->address || address > bp->address_end)
        {
            continue;
        }

        // Verificar condição
        if (!check_breakpoint_condition(bp, value, previous_value))
        {
            continue;
        }

        // Incrementar contador de hits
        bp->hit_count++;

        // Verificar contador de hits se necessário
        if ((bp->flags & BP_FLAG_COUNTER) && bp->hit_count < bp->hit_count_target)
        {
            continue; // Não atingiu o alvo ainda
        }

        // Verificar expressão condicional se presente
        if ((bp->flags & BP_FLAG_CONDITION) && bp->condition[0] != '\0')
        {
            // Aqui seria necessário um avaliador de expressões
            // Esta é uma implementação simplificada
            if (strcmp(bp->condition, "true") != 0)
            {
                continue; // Condição não satisfeita
            }
        }

        // Breakpoint ativado!

        // Se for temporário, remover
        if (bp->flags & BP_FLAG_TEMPORARY)
        {
            breakpoint_remove(context, bp->id);
        }

        // Chamar callback se definido
        if (context->callback)
        {
            context->callback(context, bp, context->callback_user_data);
        }

        // Retornar ID se solicitado
        if (id_out)
        {
            *id_out = bp->id;
        }

        return true;
    }

    return false;
}

// Implementações adicionais básicas

bool breakpoint_modify(
    breakpoint_context_t *context,
    int32_t id,
    const breakpoint_t *bp)
{
    if (!context || !bp)
        return false;

    // Encontrar índice do breakpoint
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return false;

    // Preservar ID
    uint32_t original_id = context->breakpoints[index].id;

    // Copiar nova configuração
    memcpy(&context->breakpoints[index], bp, sizeof(breakpoint_t));

    // Restaurar ID (para manter consistência)
    context->breakpoints[index].id = original_id;

    return true;
}

bool breakpoint_get_info(
    breakpoint_context_t *context,
    int32_t id,
    breakpoint_t *bp)
{
    if (!context || !bp)
        return false;

    // Encontrar índice do breakpoint
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return false;

    // Copiar informações
    memcpy(bp, &context->breakpoints[index], sizeof(breakpoint_t));

    return true;
}

uint32_t breakpoint_remove_all(breakpoint_context_t *context)
{
    if (!context)
        return 0;

    uint32_t count = context->breakpoint_count;
    context->breakpoint_count = 0;

    return count;
}

uint32_t breakpoint_disable_all(breakpoint_context_t *context)
{
    if (!context)
        return 0;

    uint32_t count = 0;

    for (uint32_t i = 0; i < context->breakpoint_count; i++)
    {
        if (context->breakpoints[i].flags & BP_FLAG_ENABLED)
        {
            context->breakpoints[i].flags &= ~BP_FLAG_ENABLED;
            count++;
        }
    }

    return count;
}

uint32_t breakpoint_list(
    breakpoint_context_t *context,
    breakpoint_t *bps,
    uint32_t max_bps)
{
    if (!context || !bps || max_bps == 0)
        return 0;

    uint32_t copy_count = context->breakpoint_count;
    if (copy_count > max_bps)
    {
        copy_count = max_bps;
    }

    memcpy(bps, context->breakpoints, copy_count * sizeof(breakpoint_t));

    return copy_count;
}

uint32_t breakpoint_get_count(breakpoint_context_t *context)
{
    if (!context)
        return 0;

    return context->breakpoint_count;
}

int32_t breakpoint_set_temporary(
    breakpoint_context_t *context,
    uint32_t address)
{
    if (!context)
        return BP_ERROR_INVALID_PARAMS;

    return breakpoint_add(context,
                          BP_TYPE_EXECUTION,
                          address,
                          BP_FLAG_TEMPORARY);
}

bool breakpoint_set_description(
    breakpoint_context_t *context,
    int32_t id,
    const char *description)
{
    if (!context || !description)
        return false;

    // Encontrar índice do breakpoint
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return false;

    // Copiar descrição
    strncpy(context->breakpoints[index].description,
            description,
            sizeof(context->breakpoints[index].description) - 1);
    context->breakpoints[index].description[sizeof(context->breakpoints[index].description) - 1] = '\0';

    return true;
}

bool breakpoint_set_log_format(
    breakpoint_context_t *context,
    int32_t id,
    const char *format)
{
    if (!context || !format)
        return false;

    // Encontrar índice do breakpoint
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return false;

    // Copiar formato
    strncpy(context->breakpoints[index].log_format,
            format,
            sizeof(context->breakpoints[index].log_format) - 1);
    context->breakpoints[index].log_format[sizeof(context->breakpoints[index].log_format) - 1] = '\0';

    // Ativar flag de log
    context->breakpoints[index].flags |= BP_FLAG_LOG;

    return true;
}

bool breakpoint_set_hit_count(
    breakpoint_context_t *context,
    int32_t id,
    uint32_t count)
{
    if (!context || count == 0)
        return false;

    // Encontrar índice do breakpoint
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return false;

    // Configurar contador
    context->breakpoints[index].hit_count_target = count;
    context->breakpoints[index].hit_count = 0;
    context->breakpoints[index].flags |= BP_FLAG_COUNTER;

    return true;
}

// Funções especializadas
int32_t breakpoint_add_register(
    breakpoint_context_t *context,
    breakpoint_register_t reg,
    breakpoint_compare_t cmp,
    uint32_t value,
    uint32_t flags)
{
    if (!context)
        return BP_ERROR_INVALID_PARAMS;

    // Adicionar breakpoint básico primeiro
    int32_t id = breakpoint_add(context, BP_TYPE_REGISTER, (uint32_t)reg, flags);
    if (id < 0)
        return id;

    // Encontrar o índice do breakpoint recém-adicionado
    int32_t index = find_breakpoint_index(context, id);
    if (index < 0)
        return BP_ERROR_NOT_FOUND;

    // Configurar condição
    context->breakpoints[index].cmp = cmp;
    context->breakpoints[index].value = value;
    context->breakpoints[index].reg = reg;

    return id;
}

int32_t breakpoint_add_watchpoint(
    breakpoint_context_t *context,
    uint32_t address,
    breakpoint_compare_t cmp,
    uint32_t value,
    uint32_t flags)
{
    if (!context)
        return BP_ERROR_INVALID_PARAMS;

    // Adicionar breakpoint para qualquer tipo de acesso de memória
    int32_t read_id = breakpoint_add_conditional(
        context, BP_TYPE_MEMORY_READ, address, cmp, value, flags);

    int32_t write_id = breakpoint_add_conditional(
        context, BP_TYPE_MEMORY_WRITE, address, cmp, value, flags);

    // Se ambos falharem, retornar erro
    if (read_id < 0 && write_id < 0)
    {
        return BP_ERROR_LIMIT_REACHED;
    }

    // Retornar o ID do breakpoint de leitura (ou escrita se leitura falhou)
    return (read_id >= 0) ? read_id : write_id;
}

int32_t breakpoint_add_cycle_count(
    breakpoint_context_t *context,
    uint32_t cycles,
    uint32_t flags)
{
    if (!context)
        return BP_ERROR_INVALID_PARAMS;

    // Adicionar breakpoint para contador de ciclos
    return breakpoint_add_conditional(
        context, BP_TYPE_CYCLE_COUNT, 0, BP_CMP_GREATER_OR_EQUAL, cycles, flags);
}

// Exportação e importação (stubs básicos)
bool breakpoint_export(
    breakpoint_context_t *context,
    const char *filename)
{
    if (!context || !filename)
        return false;

    FILE *file = fopen(filename, "wb");
    if (!file)
        return false;

    // Salvar cabeçalho
    uint32_t header[3] = {
        0x42504558, // "BPEX" em ASCII
        context->platform_id,
        context->breakpoint_count};

    fwrite(header, sizeof(uint32_t), 3, file);

    // Salvar breakpoints
    fwrite(context->breakpoints, sizeof(breakpoint_t), context->breakpoint_count, file);

    fclose(file);
    return true;
}

int32_t breakpoint_import(
    breakpoint_context_t *context,
    const char *filename)
{
    if (!context || !filename)
        return BP_ERROR_INVALID_PARAMS;

    FILE *file = fopen(filename, "rb");
    if (!file)
        return BP_ERROR_FILE;

    // Ler cabeçalho
    uint32_t header[3];
    if (fread(header, sizeof(uint32_t), 3, file) != 3)
    {
        fclose(file);
        return BP_ERROR_FILE;
    }

    // Verificar assinatura
    if (header[0] != 0x42504558)
    { // "BPEX" em ASCII
        fclose(file);
        return BP_ERROR_INVALID_PARAMS;
    }

    // Verificar plataforma
    if (header[1] != context->platform_id)
    {
        fclose(file);
        return BP_ERROR_INVALID_PARAMS;
    }

    uint32_t count = header[2];
    if (count > context->max_breakpoints)
    {
        count = context->max_breakpoints;
    }

    // Limpar breakpoints existentes
    breakpoint_remove_all(context);

    // Ler breakpoints
    breakpoint_t *temp_breakpoints = malloc(count * sizeof(breakpoint_t));
    if (!temp_breakpoints)
    {
        fclose(file);
        return BP_ERROR_MEMORY;
    }

    if (fread(temp_breakpoints, sizeof(breakpoint_t), count, file) != count)
    {
        free(temp_breakpoints);
        fclose(file);
        return BP_ERROR_FILE;
    }

    // Ajustar IDs e adicionar
    uint32_t imported = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        // Usar um ID novo
        temp_breakpoints[i].id = context->next_breakpoint_id++;

        // Copiar para o contexto
        if (context->breakpoint_count < context->max_breakpoints)
        {
            memcpy(&context->breakpoints[context->breakpoint_count++],
                   &temp_breakpoints[i],
                   sizeof(breakpoint_t));
            imported++;
        }
        else
        {
            break;
        }
    }

    free(temp_breakpoints);
    fclose(file);

    return imported;
}
