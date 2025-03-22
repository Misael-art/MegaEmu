/**
 * @file m68k_adapter.h
 * @brief Adaptador do processador Motorola 68000 (M68K) para o Mega Drive
 */

#ifndef MEGA_EMU_MD_M68K_ADAPTER_H
#define MEGA_EMU_MD_M68K_ADAPTER_H

#include <stdint.h>
#include "../../../core/cpu/m68k/m68k.h"
#include "../md_core.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Tipo opaco para o adaptador M68K do Mega Drive
     */
    typedef struct md_m68k_s md_m68k_t;

    /**
     * @brief Cria uma nova instância do adaptador M68K para Mega Drive
     * @return Ponteiro para a instância do adaptador ou NULL em caso de erro
     */
    md_m68k_t *md_m68k_create(void);

    /**
     * @brief Destrói uma instância do adaptador M68K para Mega Drive
     * @param adapter Ponteiro para a instância do adaptador
     */
    void md_m68k_destroy(md_m68k_t *adapter);

    /**
     * @brief Inicializa o adaptador M68K para Mega Drive
     * @param adapter Ponteiro para a instância do adaptador
     * @param context Contexto do Mega Drive
     * @return 0 em caso de sucesso, -1 em caso de erro
     */
    int md_m68k_init(md_m68k_t *adapter, md_context_t *context);

    /**
     * @brief Reseta o adaptador M68K para Mega Drive
     * @param adapter Ponteiro para a instância do adaptador
     */
    void md_m68k_reset(md_m68k_t *adapter);

    /**
     * @brief Executa um número específico de ciclos no adaptador M68K para Mega Drive
     * @param adapter Ponteiro para a instância do adaptador
     * @param cycles Número de ciclos a executar
     * @return Número de ciclos executados
     */
    int md_m68k_execute_cycles(md_m68k_t *adapter, int cycles);

    /**
     * @brief Define o nível de interrupção no adaptador M68K para Mega Drive
     * @param adapter Ponteiro para a instância do adaptador
     * @param level Nível de interrupção (0-7)
     */
    void md_m68k_set_irq(md_m68k_t *adapter, int level);

    /**
     * @brief Obtém o valor de um registrador do adaptador M68K para Mega Drive
     * @param adapter Ponteiro para a instância do adaptador
     * @param reg Índice do registrador
     * @return Valor do registrador
     */
    uint32_t md_m68k_get_register(md_m68k_t *adapter, int reg);

    /**
     * @brief Define o valor de um registrador do adaptador M68K para Mega Drive
     * @param adapter Ponteiro para a instância do adaptador
     * @param reg Índice do registrador
     * @param value Valor a definir
     */
    void md_m68k_set_register(md_m68k_t *adapter, int reg, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_MD_M68K_ADAPTER_H */
