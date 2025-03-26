/**
 * @file memory.c
 * @brief Implementação do sistema de gerenciamento de memória
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-23
 */

#include "memory.h"
#include <stdlib.h>
#include <string.h>

// Variável global para controlar o estado do subsistema
static bool g_memory_initialized = false;

int memory_init(void)
{
    if (g_memory_initialized)
    {
        return -1; // Já inicializado
    }

    g_memory_initialized = true;
    return 0;
}

int memory_shutdown(void)
{
    if (!g_memory_initialized)
    {
        return -1; // Não inicializado
    }

    g_memory_initialized = false;
    return 0;
}

void *memory_alloc(uint32_t size)
{
    if (!g_memory_initialized || size == 0)
    {
        return NULL;
    }

    return calloc(1, size);
}

void memory_free(void *ptr)
{
    if (!g_memory_initialized || !ptr)
    {
        return;
    }

    free(ptr);
}

void memory_clear(void *ptr, uint32_t size)
{
    if (!g_memory_initialized || !ptr || size == 0)
    {
        return;
    }

    memset(ptr, 0, size);
}
