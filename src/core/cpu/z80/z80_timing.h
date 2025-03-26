/**
 * @file z80_timing.h
 * @brief Sistema refinado de timing para o processador Z80
 */

#ifndef MEGA_EMU_Z80_TIMING_H
#define MEGA_EMU_Z80_TIMING_H

#include "z80.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Modo de contagem de ciclos
     */
    typedef enum
    {
        Z80_TIMING_STANDARD, /**< Contagem de ciclos padrão */
        Z80_TIMING_ACCURATE, /**< Contagem de ciclos precisa (T states) */
        Z80_TIMING_CMOS,     /**< Timing específico para Z80 CMOS */
        Z80_TIMING_CUSTOM    /**< Timing personalizado */
    } z80_timing_mode_t;

    /**
     * @brief Estrutura de configuração de timing para cada instrução
     */
    typedef struct
    {
        uint8_t opcode;         /**< Opcode principal */
        uint8_t opcode_ext;     /**< Opcode extendido (para prefixos CB/DD/ED/FD) */
        uint8_t prefix;         /**< Prefixo (0=nenhum, CB=0xCB, DD=0xDD, etc) */
        uint8_t base_cycles;    /**< Ciclos base */
        uint8_t branch_taken;   /**< Ciclos adicionais quando branch é tomado */
        uint8_t mem_contention; /**< Ciclos adicionais por contenção de memória */
        uint8_t io_contention;  /**< Ciclos adicionais por contenção de I/O */
    } z80_timing_entry_t;

    /**
     * @brief Referência opaca para o contexto de timing
     */
    typedef struct z80_timing_context z80_timing_t;

    /**
     * @brief Inicializar subsistema de timing
     *
     * @param cpu Ponteiro para a instância do Z80
     * @param mode Modo de timing a utilizar
     * @return Ponteiro para o contexto de timing ou NULL em caso de erro
     */
    z80_timing_t *z80_timing_create(z80_t *cpu, z80_timing_mode_t mode);

    /**
     * @brief Destruir subsistema de timing
     *
     * @param timing Ponteiro para o contexto de timing
     */
    void z80_timing_destroy(z80_timing_t *timing);

    /**
     * @brief Definir modo de timing
     *
     * @param timing Ponteiro para o contexto de timing
     * @param mode Novo modo de timing
     * @return true se configurado com sucesso, false caso contrário
     */
    bool z80_timing_set_mode(z80_timing_t *timing, z80_timing_mode_t mode);

    /**
     * @brief Obter ciclos para uma instrução específica
     *
     * @param timing Ponteiro para o contexto de timing
     * @param opcode Opcode da instrução
     * @param prefix Prefixo (0, 0xCB, 0xDD, 0xED, 0xFD)
     * @param branch_taken Flag indicando se um branch foi tomado
     * @return Número de ciclos
     */
    int z80_timing_get_cycles(z80_timing_t *timing, uint8_t opcode, uint8_t prefix, bool branch_taken);

    /**
     * @brief Definir entrada de timing personalizada
     *
     * @param timing Ponteiro para o contexto de timing
     * @param entry Ponteiro para a entrada de timing
     * @return true se configurado com sucesso, false caso contrário
     */
    bool z80_timing_set_custom_entry(z80_timing_t *timing, const z80_timing_entry_t *entry);

    /**
     * @brief Obter entrada de timing para uma instrução
     *
     * @param timing Ponteiro para o contexto de timing
     * @param opcode Opcode da instrução
     * @param prefix Prefixo (0, 0xCB, 0xDD, 0xED, 0xFD)
     * @param entry Ponteiro para armazenar a entrada de timing
     * @return true se a entrada foi encontrada, false caso contrário
     */
    bool z80_timing_get_entry(z80_timing_t *timing, uint8_t opcode, uint8_t prefix, z80_timing_entry_t *entry);

    /**
     * @brief Substituir a tabela completa de timing
     *
     * @param timing Ponteiro para o contexto de timing
     * @param entries Array de entradas de timing
     * @param count Número de entradas no array
     * @return true se configurado com sucesso, false caso contrário
     */
    bool z80_timing_set_custom_table(z80_timing_t *timing, const z80_timing_entry_t *entries, int count);

    /**
     * @brief Ajustar os ciclos de acordo com o tipo de memória
     *
     * @param timing Ponteiro para o contexto de timing
     * @param address Endereço de memória acessado
     * @param cycles Número de ciclos base
     * @return Número de ciclos ajustado
     */
    int z80_timing_adjust_memory_cycles(z80_timing_t *timing, uint16_t address, int cycles);

    /**
     * @brief Ajustar os ciclos de acordo com o tipo de I/O
     *
     * @param timing Ponteiro para o contexto de timing
     * @param port Porta de I/O acessada
     * @param cycles Número de ciclos base
     * @return Número de ciclos ajustado
     */
    int z80_timing_adjust_io_cycles(z80_timing_t *timing, uint16_t port, int cycles);

    /**
     * @brief Definir callback para contenção de memória
     *
     * @param timing Ponteiro para o contexto de timing
     * @param callback Função de callback
     * @param user_data Dados de usuário passados para o callback
     */
    void z80_timing_set_memory_contention_callback(z80_timing_t *timing,
                                                   int (*callback)(void *user_data, uint16_t address, int cycles),
                                                   void *user_data);

    /**
     * @brief Definir callback para contenção de I/O
     *
     * @param timing Ponteiro para o contexto de timing
     * @param callback Função de callback
     * @param user_data Dados de usuário passados para o callback
     */
    void z80_timing_set_io_contention_callback(z80_timing_t *timing,
                                               int (*callback)(void *user_data, uint16_t port, int cycles),
                                               void *user_data);

    /**
     * @brief Obter o total de ciclos executados
     *
     * @param timing Ponteiro para o contexto de timing
     * @return Total de ciclos
     */
    uint64_t z80_timing_get_total_cycles(z80_timing_t *timing);

    /**
     * @brief Resetar contador de ciclos
     *
     * @param timing Ponteiro para o contexto de timing
     */
    void z80_timing_reset_counter(z80_timing_t *timing);

    /**
     * @brief Obter estatísticas de contenção de memória
     *
     * @param timing Ponteiro para o contexto de timing
     * @param contention Ponteiro para armazenar ciclos de contenção de memória
     */
    void z80_timing_get_contention_stats(z80_timing_t *timing, uint64_t *contention);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_Z80_TIMING_H */
