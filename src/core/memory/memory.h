/**
 * @file memory.h
 * @brief Interface para o sistema de gerenciamento de memória
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-23
 */

#ifndef MEGA_EMU_MEMORY_H
#define MEGA_EMU_MEMORY_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa o subsistema de memória
 *
 * @return 0 em caso de sucesso ou -1 em caso de erro
 */
int memory_init(void);

/**
 * @brief Finaliza o subsistema de memória
 *
 * @return 0 em caso de sucesso ou -1 em caso de erro
 */
int memory_shutdown(void);

/**
 * @brief Aloca uma região de memória
 *
 * @param size Tamanho da região a ser alocada
 * @return Ponteiro para a região alocada ou NULL em caso de erro
 */
void *memory_alloc(uint32_t size);

/**
 * @brief Libera uma região de memória previamente alocada
 *
 * @param ptr Ponteiro para a região a ser liberada
 */
void memory_free(void *ptr);

/**
 * @brief Limpa uma região de memória
 *
 * @param ptr Ponteiro para o início da região
 * @param size Tamanho da região em bytes
 */
void memory_clear(void *ptr, uint32_t size);

#endif /* MEGA_EMU_MEMORY_H */
