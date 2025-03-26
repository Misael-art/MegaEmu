/**
 * @file z80_debug.h
 * @brief Sistema avançado de depuração para o processador Z80
 */

#ifndef MEGA_EMU_Z80_DEBUG_H
#define MEGA_EMU_Z80_DEBUG_H

#include "z80.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Tipos de breakpoint
     */
    typedef enum
    {
        Z80_BREAK_EXECUTION,    /**< Break na execução */
        Z80_BREAK_MEMORY_READ,  /**< Break na leitura de memória */
        Z80_BREAK_MEMORY_WRITE, /**< Break na escrita de memória */
        Z80_BREAK_IO_READ,      /**< Break na leitura de I/O */
        Z80_BREAK_IO_WRITE,     /**< Break na escrita de I/O */
        Z80_BREAK_INTERRUPT     /**< Break em interrupção */
    } z80_breakpoint_type_t;

    /**
     * @brief Condição de breakpoint
     */
    typedef enum
    {
        Z80_CONDITION_ALWAYS,     /**< Sempre quebrar */
        Z80_CONDITION_EQUALS,     /**< Se valor == valor específico */
        Z80_CONDITION_NOT_EQUALS, /**< Se valor != valor específico */
        Z80_CONDITION_GREATER,    /**< Se valor > valor específico */
        Z80_CONDITION_LESS,       /**< Se valor < valor específico */
        Z80_CONDITION_MASK_MATCH  /**< Se (valor & mask) == valor específico */
    } z80_breakpoint_condition_t;

    /**
     * @brief Estrutura de breakpoint
     */
    typedef struct
    {
        int id;                               /**< ID único do breakpoint */
        z80_breakpoint_type_t type;           /**< Tipo de breakpoint */
        uint16_t address;                     /**< Endereço (para breakpoints de memória/execução) */
        uint16_t address_end;                 /**< Endereço final (para ranges) */
        z80_breakpoint_condition_t condition; /**< Condição */
        uint16_t condition_value;             /**< Valor para a condição */
        uint16_t condition_mask;              /**< Máscara para condição */
        bool enabled;                         /**< Se o breakpoint está ativo */
        bool temporary;                       /**< Se o breakpoint é temporário */
        char description[64];                 /**< Descrição opcional */
    } z80_breakpoint_t;

    /**
     * @brief Trace de execução - armazena histórico de instruções
     */
    typedef struct
    {
        uint16_t pc;            /**< PC onde a instrução foi executada */
        uint8_t opcode[4];      /**< Bytes da instrução */
        uint8_t opcode_length;  /**< Comprimento da instrução */
        uint16_t registers[12]; /**< Estado dos registradores antes da execução */
        uint8_t flags;          /**< Flags antes da execução */
        int cycles;             /**< Ciclos consumidos */
        char disassembly[32];   /**< Instrução desassemblada */
    } z80_trace_entry_t;

    /**
     * @brief Referência opaca para o contexto de debug
     */
    typedef struct z80_debug_context z80_debug_t;

    /**
     * @brief Inicializar subsistema de debug
     *
     * @param cpu Ponteiro para a instância do Z80
     * @return Ponteiro para o contexto de debug ou NULL em caso de erro
     */
    z80_debug_t *z80_debug_create(z80_t *cpu);

    /**
     * @brief Destruir subsistema de debug
     *
     * @param debug Ponteiro para o contexto de debug
     */
    void z80_debug_destroy(z80_debug_t *debug);

    /**
     * @brief Adicionar um breakpoint simples
     *
     * @param debug Ponteiro para o contexto de debug
     * @param type Tipo de breakpoint
     * @param address Endereço do breakpoint
     * @return ID do breakpoint ou -1 em caso de erro
     */
    int z80_debug_add_breakpoint(z80_debug_t *debug, z80_breakpoint_type_t type, uint16_t address);

    /**
     * @brief Adicionar um breakpoint avançado com condição
     *
     * @param debug Ponteiro para o contexto de debug
     * @param type Tipo de breakpoint
     * @param address Endereço do breakpoint
     * @param condition Condição do breakpoint
     * @param value Valor para a condição
     * @return ID do breakpoint ou -1 em caso de erro
     */
    int z80_debug_add_breakpoint_ex(z80_debug_t *debug, z80_breakpoint_type_t type, uint16_t address,
                                    z80_breakpoint_condition_t condition, uint16_t value);

    /**
     * @brief Remover um breakpoint
     *
     * @param debug Ponteiro para o contexto de debug
     * @param id ID do breakpoint a remover
     * @return true se removido com sucesso, false caso contrário
     */
    bool z80_debug_remove_breakpoint(z80_debug_t *debug, int id);

    /**
     * @brief Habilitar ou desabilitar um breakpoint
     *
     * @param debug Ponteiro para o contexto de debug
     * @param id ID do breakpoint
     * @param enable true para habilitar, false para desabilitar
     */
    void z80_debug_enable_breakpoint(z80_debug_t *debug, int id, bool enable);

    /**
     * @brief Continuar execução após um breakpoint
     *
     * @param debug Ponteiro para o contexto de debug
     */
    void z80_debug_continue(z80_debug_t *debug);

    /**
     * @brief Executar a próxima instrução (step into)
     *
     * @param debug Ponteiro para o contexto de debug
     */
    void z80_debug_step_into(z80_debug_t *debug);

    /**
     * @brief Executar até retornar da instrução atual (step over)
     *
     * @param debug Ponteiro para o contexto de debug
     */
    void z80_debug_step_over(z80_debug_t *debug);

    /**
     * @brief Executar até sair da sub-rotina atual (step out)
     *
     * @param debug Ponteiro para o contexto de debug
     */
    void z80_debug_step_out(z80_debug_t *debug);

    /**
     * @brief Habilitar ou desabilitar trace de execução
     *
     * @param debug Ponteiro para o contexto de debug
     * @param enable true para habilitar, false para desabilitar
     */
    void z80_debug_enable_trace(z80_debug_t *debug, bool enable);

    /**
     * @brief Obter uma entrada específica do trace
     *
     * @param debug Ponteiro para o contexto de debug
     * @param index Índice da entrada no buffer de trace
     * @return Ponteiro para a entrada de trace ou NULL se índice inválido
     */
    z80_trace_entry_t *z80_debug_get_trace(z80_debug_t *debug, int index);

    /**
     * @brief Limpar o buffer de trace
     *
     * @param debug Ponteiro para o contexto de debug
     */
    void z80_debug_clear_trace(z80_debug_t *debug);

    /**
     * @brief Dumpar o estado atual do Z80
     *
     * @param debug Ponteiro para o contexto de debug
     * @param buffer Buffer para armazenar o resultado
     * @param buffer_size Tamanho do buffer
     */
    void z80_debug_dump_state(z80_debug_t *debug, char *buffer, int buffer_size);

    /**
     * @brief Dumpar uma região de memória
     *
     * @param debug Ponteiro para o contexto de debug
     * @param address Endereço inicial
     * @param size Tamanho da região
     * @param buffer Buffer para armazenar o resultado
     * @param buffer_size Tamanho do buffer
     */
    void z80_debug_dump_memory(z80_debug_t *debug, uint16_t address, uint16_t size, char *buffer, int buffer_size);

    /**
     * @brief Desassemblar um range de código
     *
     * @param debug Ponteiro para o contexto de debug
     * @param start Endereço inicial
     * @param end Endereço final
     * @param buffer Buffer para armazenar o resultado
     * @param buffer_size Tamanho do buffer
     */
    void z80_debug_disassemble_range(z80_debug_t *debug, uint16_t start, uint16_t end, char *buffer, int buffer_size);

    /**
     * @brief Verificar breakpoints antes da execução
     *
     * @param debug Ponteiro para o contexto de debug
     * @param cpu Ponteiro para a instância do Z80
     * @return true se um breakpoint foi atingido, false caso contrário
     */
    bool z80_debug_check_execution_breakpoint(z80_debug_t *debug, z80_t *cpu);

    /**
     * @brief Verificar breakpoints de memória
     *
     * @param debug Ponteiro para o contexto de debug
     * @param cpu Ponteiro para a instância do Z80
     * @param address Endereço acessado
     * @param is_write Flag indicando se é escrita (true) ou leitura (false)
     * @param value Valor escrito/lido
     * @return true se um breakpoint foi atingido, false caso contrário
     */
    bool z80_debug_check_memory_breakpoint(z80_debug_t *debug, z80_t *cpu, uint16_t address, bool is_write, uint8_t value);

    /**
     * @brief Verificar breakpoints de I/O
     *
     * @param debug Ponteiro para o contexto de debug
     * @param cpu Ponteiro para a instância do Z80
     * @param port Porta acessada
     * @param is_write Flag indicando se é escrita (true) ou leitura (false)
     * @param value Valor escrito/lido
     * @return true se um breakpoint foi atingido, false caso contrário
     */
    bool z80_debug_check_io_breakpoint(z80_debug_t *debug, z80_t *cpu, uint16_t port, bool is_write, uint8_t value);

    /**
     * @brief Registrar callback para eventos de breakpoint
     *
     * @param debug Ponteiro para o contexto de debug
     * @param callback Função de callback
     * @param user_data Dados de usuário passados para o callback
     */
    void z80_debug_set_breakpoint_callback(z80_debug_t *debug,
                                           void (*callback)(z80_t *cpu, z80_breakpoint_t *breakpoint, void *user_data),
                                           void *user_data);

    /**
     * @brief Registrar callback para eventos de trace
     *
     * @param debug Ponteiro para o contexto de debug
     * @param callback Função de callback
     * @param user_data Dados de usuário passados para o callback
     */
    void z80_debug_set_trace_callback(z80_debug_t *debug,
                                      void (*callback)(z80_t *cpu, z80_trace_entry_t *entry, void *user_data),
                                      void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_Z80_DEBUG_H */
