/**
 * @file m68k_adapter.c
 * @brief Adaptador do processador Motorola 68000 (M68K) para o Mega Drive
 */

#include <stdlib.h>
#include <string.h>
#include "m68k_adapter.h"
#include "../memory/md_memory.h"

/**
 * @brief Estrutura do adaptador M68K para Mega Drive
 */
struct md_m68k_s
{
    m68k_t *cpu;           /* Instância do M68K */
    md_context_t *context; /* Contexto do Mega Drive */
};

/**
 * @brief Callback de leitura de byte
 * @param context Contexto do Mega Drive
 * @param address Endereço de memória
 * @return Valor lido
 */
static uint8_t md_m68k_read_byte(void *context, uint32_t address)
{
    md_context_t *md = (md_context_t *)context;
    return md_memory_read_byte(md, address);
}

/**
 * @brief Callback de leitura de palavra
 * @param context Contexto do Mega Drive
 * @param address Endereço de memória
 * @return Valor lido
 */
static uint16_t md_m68k_read_word(void *context, uint32_t address)
{
    md_context_t *md = (md_context_t *)context;
    return md_memory_read_word(md, address);
}

/**
 * @brief Callback de leitura de palavra longa
 * @param context Contexto do Mega Drive
 * @param address Endereço de memória
 * @return Valor lido
 */
static uint32_t md_m68k_read_long(void *context, uint32_t address)
{
    md_context_t *md = (md_context_t *)context;
    return md_memory_read_long(md, address);
}

/**
 * @brief Callback de escrita de byte
 * @param context Contexto do Mega Drive
 * @param address Endereço de memória
 * @param value Valor a escrever
 */
static void md_m68k_write_byte(void *context, uint32_t address, uint8_t value)
{
    md_context_t *md = (md_context_t *)context;
    md_memory_write_byte(md, address, value);
}

/**
 * @brief Callback de escrita de palavra
 * @param context Contexto do Mega Drive
 * @param address Endereço de memória
 * @param value Valor a escrever
 */
static void md_m68k_write_word(void *context, uint32_t address, uint16_t value)
{
    md_context_t *md = (md_context_t *)context;
    md_memory_write_word(md, address, value);
}

/**
 * @brief Callback de escrita de palavra longa
 * @param context Contexto do Mega Drive
 * @param address Endereço de memória
 * @param value Valor a escrever
 */
static void md_m68k_write_long(void *context, uint32_t address, uint32_t value)
{
    md_context_t *md = (md_context_t *)context;
    md_memory_write_long(md, address, value);
}

/**
 * @brief Cria uma nova instância do adaptador M68K para Mega Drive
 * @return Ponteiro para a instância do adaptador ou NULL em caso de erro
 */
md_m68k_t *md_m68k_create(void)
{
    md_m68k_t *adapter = (md_m68k_t *)malloc(sizeof(md_m68k_t));
    if (adapter)
    {
        memset(adapter, 0, sizeof(md_m68k_t));
        adapter->cpu = m68k_create();
        if (!adapter->cpu)
        {
            free(adapter);
            return NULL;
        }
    }
    return adapter;
}

/**
 * @brief Destrói uma instância do adaptador M68K para Mega Drive
 * @param adapter Ponteiro para a instância do adaptador
 */
void md_m68k_destroy(md_m68k_t *adapter)
{
    if (adapter)
    {
        if (adapter->cpu)
        {
            m68k_destroy(adapter->cpu);
        }
        free(adapter);
    }
}

/**
 * @brief Inicializa o adaptador M68K para Mega Drive
 * @param adapter Ponteiro para a instância do adaptador
 * @param context Contexto do Mega Drive
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int md_m68k_init(md_m68k_t *adapter, md_context_t *context)
{
    if (!adapter || !context)
    {
        return -1;
    }

    adapter->context = context;

    /* Configurar callbacks de acesso à memória */
    adapter->cpu->read_byte = md_m68k_read_byte;
    adapter->cpu->read_word = md_m68k_read_word;
    adapter->cpu->read_long = md_m68k_read_long;
    adapter->cpu->write_byte = md_m68k_write_byte;
    adapter->cpu->write_word = md_m68k_write_word;
    adapter->cpu->write_long = md_m68k_write_long;
    adapter->cpu->context = context;

    /* Inicializar CPU */
    m68k_init(adapter->cpu);

    return 0;
}

/**
 * @brief Reseta o adaptador M68K para Mega Drive
 * @param adapter Ponteiro para a instância do adaptador
 */
void md_m68k_reset(md_m68k_t *adapter)
{
    if (adapter && adapter->cpu)
    {
        m68k_reset(adapter->cpu);
    }
}

/**
 * @brief Executa um número específico de ciclos no adaptador M68K para Mega Drive
 * @param adapter Ponteiro para a instância do adaptador
 * @param cycles Número de ciclos a executar
 * @return Número de ciclos executados
 */
int md_m68k_execute_cycles(md_m68k_t *adapter, int cycles)
{
    if (!adapter || !adapter->cpu)
    {
        return 0;
    }
    return m68k_execute_cycles(adapter->cpu, cycles);
}

/**
 * @brief Define o nível de interrupção no adaptador M68K para Mega Drive
 * @param adapter Ponteiro para a instância do adaptador
 * @param level Nível de interrupção (0-7)
 */
void md_m68k_set_irq(md_m68k_t *adapter, int level)
{
    if (adapter && adapter->cpu)
    {
        m68k_set_irq(adapter->cpu, level);
    }
}

/**
 * @brief Obtém o valor de um registrador do adaptador M68K para Mega Drive
 * @param adapter Ponteiro para a instância do adaptador
 * @param reg Índice do registrador
 * @return Valor do registrador
 */
uint32_t md_m68k_get_register(md_m68k_t *adapter, int reg)
{
    if (!adapter || !adapter->cpu)
    {
        return 0;
    }
    return m68k_get_register(adapter->cpu, reg);
}

/**
 * @brief Define o valor de um registrador do adaptador M68K para Mega Drive
 * @param adapter Ponteiro para a instância do adaptador
 * @param reg Índice do registrador
 * @param value Valor a definir
 */
void md_m68k_set_register(md_m68k_t *adapter, int reg, uint32_t value)
{
    if (adapter && adapter->cpu)
    {
        m68k_set_register(adapter->cpu, reg, value);
    }
}
