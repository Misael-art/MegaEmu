/**
 * @file breakpoints.h
 * @brief API para breakpoints condicionais avançados
 */
#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "core/core.h"

    // Tipos de breakpoints
    typedef enum
    {
        BP_TYPE_EXECUTION,    // Parar quando PC alcançar o endereço
        BP_TYPE_MEMORY_READ,  // Parar quando ler da memória
        BP_TYPE_MEMORY_WRITE, // Parar quando escrever na memória
        BP_TYPE_INTERRUPT,    // Parar ao receber interrupção específica
        BP_TYPE_REGISTER,     // Parar quando um registrador atender uma condição
        BP_TYPE_CYCLE_COUNT,  // Parar após um número específico de ciclos
        BP_TYPE_VALUE_CHANGE, // Parar quando valor num endereço mudar
        BP_TYPE_EXPRESSION    // Parar quando uma expressão for avaliada como verdadeira
    } breakpoint_type_t;

    // Operadores de comparação
    typedef enum
    {
        BP_CMP_EQUAL,            // ==
        BP_CMP_NOT_EQUAL,        // !=
        BP_CMP_GREATER,          // >
        BP_CMP_GREATER_OR_EQUAL, // >=
        BP_CMP_LESS,             // <
        BP_CMP_LESS_OR_EQUAL,    // <=
        BP_CMP_BITWISE_AND,      // & (não-zero = verdadeiro)
        BP_CMP_BITWISE_NAND,     // ~& (zero = verdadeiro)
        BP_CMP_BITWISE_OR,       // | (não-zero = verdadeiro)
        BP_CMP_BITWISE_NOR,      // ~| (zero = verdadeiro)
        BP_CMP_BITWISE_XOR,      // ^ (não-zero = verdadeiro)
        BP_CMP_BITWISE_XNOR,     // ~^ (zero = verdadeiro)
        BP_CMP_CHANGED,          // qualquer mudança
        BP_CMP_CHANGED_TO,       // mudança para valor específico
        BP_CMP_CHANGED_FROM,     // mudança de valor específico
        BP_CMP_IN_RANGE,         // dentro de um intervalo
        BP_CMP_NOT_IN_RANGE      // fora de um intervalo
    } breakpoint_compare_t;

    // Tipos especiais de registrador
    typedef enum
    {
        BP_REG_PC,    // Program Counter
        BP_REG_SP,    // Stack Pointer
        BP_REG_A,     // Acumulador (A ou semelhante)
        BP_REG_X,     // Registrador X (ou semelhante)
        BP_REG_Y,     // Registrador Y (ou semelhante)
        BP_REG_BC,    // Par BC (Z80, semelhantes)
        BP_REG_DE,    // Par DE (Z80, semelhantes)
        BP_REG_HL,    // Par HL (Z80, semelhantes)
        BP_REG_IX,    // Registrador IX (Z80)
        BP_REG_IY,    // Registrador IY (Z80)
        BP_REG_SR,    // Status Register (Flags)
        BP_REG_CYCLE, // Contador de ciclos
        // etc... outros conforme necessidade
        BP_REG_CUSTOM = 0x100 // Base para registradores específicos da plataforma
    } breakpoint_register_t;

    // Flags de um breakpoint
    typedef enum
    {
        BP_FLAG_ENABLED = (1 << 0),   // Breakpoint está ativo
        BP_FLAG_TEMPORARY = (1 << 1), // Breakpoint de uso único (auto-remove)
        BP_FLAG_SILENT = (1 << 2),    // Não notifica o usuário
        BP_FLAG_TRACE = (1 << 3),     // Registra no trace, mas não para execução
        BP_FLAG_CONDITION = (1 << 4), // Possui condição adicional
        BP_FLAG_LOG = (1 << 5),       // Gera log quando ativado
        BP_FLAG_SKIP = (1 << 6),      // Pular a instrução ao invés de parar
        BP_FLAG_COUNTER = (1 << 7)    // Possui contador (precisa ativar N vezes)
    } breakpoint_flags_t;

    // Códigos de retorno
    typedef enum
    {
        BP_SUCCESS = 0,
        BP_ERROR_INVALID_PARAMS = -1,
        BP_ERROR_NOT_FOUND = -2,
        BP_ERROR_MEMORY = -3,
        BP_ERROR_LIMIT_REACHED = -4,
        BP_ERROR_PARSE = -5
    } breakpoint_error_t;

    // Estrutura de dados do breakpoint
    typedef struct
    {
        uint32_t id;               // ID único do breakpoint
        breakpoint_type_t type;    // Tipo do breakpoint
        uint32_t address;          // Endereço (para BP_TYPE_EXECUTION, MEMORY_*)
        uint32_t address_end;      // Endereço final (para ranges)
        uint32_t value;            // Valor para comparação
        uint32_t value_end;        // Valor final (para BP_CMP_IN_RANGE)
        breakpoint_compare_t cmp;  // Operador de comparação
        uint32_t mask;             // Máscara de bits para comparação
        breakpoint_register_t reg; // Registrador (para BP_TYPE_REGISTER)
        uint32_t hit_count;        // Número de vezes que foi atingido
        uint32_t hit_count_target; // Número de vezes para ativar (se BP_FLAG_COUNTER)
        uint32_t flags;            // Flags (BP_FLAG_*)
        char condition[256];       // Expressão condicional (se BP_FLAG_CONDITION)
        char log_format[256];      // Formato de log (se BP_FLAG_LOG)
        char description[256];     // Descrição opcional
    } breakpoint_t;

    // Contexto de breakpoints
    typedef struct breakpoint_context_t breakpoint_context_t;

    // Função de callback para breakpoints
    typedef void (*breakpoint_callback_t)(
        breakpoint_context_t *context,
        const breakpoint_t *breakpoint,
        void *user_data);

    /**
     * @brief Cria um novo contexto de breakpoints
     *
     * @param platform_id ID da plataforma
     * @param max_breakpoints Número máximo de breakpoints (0 = ilimitado)
     * @return breakpoint_context_t* Ponteiro para o contexto ou NULL em caso de erro
     */
    breakpoint_context_t *breakpoint_create_context(
        uint32_t platform_id,
        uint32_t max_breakpoints);

    /**
     * @brief Destrói um contexto de breakpoints
     *
     * @param context Ponteiro para o contexto
     */
    void breakpoint_destroy_context(breakpoint_context_t *context);

    /**
     * @brief Define a função de callback para notificação de breakpoints
     *
     * @param context Ponteiro para o contexto
     * @param callback Função de callback
     * @param user_data Dados de usuário para o callback
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_set_callback(
        breakpoint_context_t *context,
        breakpoint_callback_t callback,
        void *user_data);

    /**
     * @brief Adiciona um novo breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param type Tipo do breakpoint
     * @param address Endereço (ou registrador)
     * @param flags Flags do breakpoint
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_add(
        breakpoint_context_t *context,
        breakpoint_type_t type,
        uint32_t address,
        uint32_t flags);

    /**
     * @brief Adiciona um novo breakpoint com condição
     *
     * @param context Ponteiro para o contexto
     * @param type Tipo do breakpoint
     * @param address Endereço (ou registrador)
     * @param cmp Operador de comparação
     * @param value Valor para comparação
     * @param flags Flags do breakpoint
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_add_conditional(
        breakpoint_context_t *context,
        breakpoint_type_t type,
        uint32_t address,
        breakpoint_compare_t cmp,
        uint32_t value,
        uint32_t flags);

    /**
     * @brief Adiciona um novo breakpoint com expressão (complexa)
     *
     * @param context Ponteiro para o contexto
     * @param type Tipo do breakpoint
     * @param address Endereço (ou registrador)
     * @param condition Expressão condicional
     * @param flags Flags do breakpoint
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_add_with_expression(
        breakpoint_context_t *context,
        breakpoint_type_t type,
        uint32_t address,
        const char *condition,
        uint32_t flags);

    /**
     * @brief Adiciona um novo breakpoint com range (intervalo)
     *
     * @param context Ponteiro para o contexto
     * @param type Tipo do breakpoint
     * @param address_start Endereço inicial
     * @param address_end Endereço final
     * @param flags Flags do breakpoint
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_add_range(
        breakpoint_context_t *context,
        breakpoint_type_t type,
        uint32_t address_start,
        uint32_t address_end,
        uint32_t flags);

    /**
     * @brief Remove um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param id ID do breakpoint
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_remove(
        breakpoint_context_t *context,
        int32_t id);

    /**
     * @brief Habilita ou desabilita um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param id ID do breakpoint
     * @param enable true para habilitar, false para desabilitar
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_enable(
        breakpoint_context_t *context,
        int32_t id,
        bool enable);

    /**
     * @brief Modifica um breakpoint existente
     *
     * @param context Ponteiro para o contexto
     * @param id ID do breakpoint
     * @param bp Nova configuração do breakpoint
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_modify(
        breakpoint_context_t *context,
        int32_t id,
        const breakpoint_t *bp);

    /**
     * @brief Obtém informações sobre um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param id ID do breakpoint
     * @param bp Ponteiro para receber informações
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_get_info(
        breakpoint_context_t *context,
        int32_t id,
        breakpoint_t *bp);

    /**
     * @brief Remove todos os breakpoints
     *
     * @param context Ponteiro para o contexto
     * @return uint32_t Número de breakpoints removidos
     */
    uint32_t breakpoint_remove_all(breakpoint_context_t *context);

    /**
     * @brief Desabilita todos os breakpoints
     *
     * @param context Ponteiro para o contexto
     * @return uint32_t Número de breakpoints desabilitados
     */
    uint32_t breakpoint_disable_all(breakpoint_context_t *context);

    /**
     * @brief Lista todos os breakpoints
     *
     * @param context Ponteiro para o contexto
     * @param bps Array para receber os breakpoints
     * @param max_bps Tamanho máximo do array
     * @return uint32_t Número de breakpoints copiados
     */
    uint32_t breakpoint_list(
        breakpoint_context_t *context,
        breakpoint_t *bps,
        uint32_t max_bps);

    /**
     * @brief Verifica se um endereço de execução (PC) corresponde a um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param pc Valor atual do PC (Program Counter)
     * @param regs Ponteiro para os registradores atuais (pode ser NULL)
     * @param id_out Ponteiro para receber o ID do breakpoint ativado (pode ser NULL)
     * @return bool true se algum breakpoint foi ativado, false caso contrário
     */
    bool breakpoint_check_execution(
        breakpoint_context_t *context,
        uint32_t pc,
        const void *regs,
        int32_t *id_out);

    /**
     * @brief Verifica se um acesso de memória corresponde a um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param address Endereço de memória acessado
     * @param value Valor lido ou escrito
     * @param is_write true se escrita, false se leitura
     * @param id_out Ponteiro para receber o ID do breakpoint ativado (pode ser NULL)
     * @return bool true se algum breakpoint foi ativado, false caso contrário
     */
    bool breakpoint_check_memory(
        breakpoint_context_t *context,
        uint32_t address,
        uint32_t value,
        bool is_write,
        int32_t *id_out);

    /**
     * @brief Define um breakpoint temporário para uma única execução
     *
     * @param context Ponteiro para o contexto
     * @param address Endereço de memória
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_set_temporary(
        breakpoint_context_t *context,
        uint32_t address);

    /**
     * @brief Define a descrição de um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param id ID do breakpoint
     * @param description Descrição
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_set_description(
        breakpoint_context_t *context,
        int32_t id,
        const char *description);

    /**
     * @brief Define o formato de log para um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param id ID do breakpoint
     * @param format Formato de log
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_set_log_format(
        breakpoint_context_t *context,
        int32_t id,
        const char *format);

    /**
     * @brief Define contador para um breakpoint
     *
     * @param context Ponteiro para o contexto
     * @param id ID do breakpoint
     * @param count Número de ativações antes de parar a execução
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_set_hit_count(
        breakpoint_context_t *context,
        int32_t id,
        uint32_t count);

    /**
     * @brief Obtém o número de breakpoints no contexto
     *
     * @param context Ponteiro para o contexto
     * @return uint32_t Número de breakpoints
     */
    uint32_t breakpoint_get_count(breakpoint_context_t *context);

    /**
     * @brief Adiciona um breakpoint para registrador
     *
     * @param context Ponteiro para o contexto
     * @param reg Registrador a monitorar
     * @param cmp Operador de comparação
     * @param value Valor para comparação
     * @param flags Flags do breakpoint
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_add_register(
        breakpoint_context_t *context,
        breakpoint_register_t reg,
        breakpoint_compare_t cmp,
        uint32_t value,
        uint32_t flags);

    /**
     * @brief Adiciona um watchpoint (breakpoint de valor de memória)
     *
     * @param context Ponteiro para o contexto
     * @param address Endereço de memória
     * @param cmp Operador de comparação
     * @param value Valor para comparação
     * @param flags Flags do breakpoint
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_add_watchpoint(
        breakpoint_context_t *context,
        uint32_t address,
        breakpoint_compare_t cmp,
        uint32_t value,
        uint32_t flags);

    /**
     * @brief Adiciona um breakpoint para contador de ciclos
     *
     * @param context Ponteiro para o contexto
     * @param cycles Número de ciclos para ativar o breakpoint
     * @param flags Flags do breakpoint
     * @return int32_t ID do breakpoint ou valor negativo em caso de erro
     */
    int32_t breakpoint_add_cycle_count(
        breakpoint_context_t *context,
        uint32_t cycles,
        uint32_t flags);

    /**
     * @brief Exporta breakpoints para arquivo
     *
     * @param context Ponteiro para o contexto
     * @param filename Nome do arquivo
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool breakpoint_export(
        breakpoint_context_t *context,
        const char *filename);

    /**
     * @brief Importa breakpoints de arquivo
     *
     * @param context Ponteiro para o contexto
     * @param filename Nome do arquivo
     * @return int32_t Número de breakpoints importados ou valor negativo em caso de erro
     */
    int32_t breakpoint_import(
        breakpoint_context_t *context,
        const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* BREAKPOINTS_H */
