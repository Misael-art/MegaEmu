/**
 * @file memory_viewer.h
 * @brief Interface para o visualizador e editor de memória
 */
#ifndef MEMORY_VIEWER_H
#define MEMORY_VIEWER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "core/core.h"

    // Modos de visualização
    typedef enum
    {
        MEMORY_VIEW_HEX,         // Visualização hexadecimal (padrão)
        MEMORY_VIEW_DECIMAL,     // Visualização decimal
        MEMORY_VIEW_BINARY,      // Visualização binária
        MEMORY_VIEW_ASCII,       // Visualização ASCII
        MEMORY_VIEW_DISASSEMBLY, // Visualização em disassembly
        MEMORY_VIEW_CUSTOM       // Visualização personalizada
    } memory_view_mode_t;

    // Flags para recursos
    typedef enum
    {
        MEMORY_VIEWER_FLAG_EDITING_ENABLED = (1 << 0),    // Permite edição
        MEMORY_VIEWER_FLAG_FOLLOW_PC = (1 << 1),          // Segue PC quando em modo disassembly
        MEMORY_VIEWER_FLAG_HIGHLIGHT_CHANGES = (1 << 2),  // Destaca alterações recentes
        MEMORY_VIEWER_FLAG_SHOW_MEMORY_MAP = (1 << 3),    // Exibe mapa de memória
        MEMORY_VIEWER_FLAG_AUTO_REFRESH = (1 << 4),       // Atualiza automaticamente
        MEMORY_VIEWER_FLAG_SHOW_SYMBOL_NAMES = (1 << 5),  // Exibe nomes de símbolos
        MEMORY_VIEWER_FLAG_ALLOW_MULTI_REGION = (1 << 6), // Permite visualizar múltiplas regiões
        MEMORY_VIEWER_FLAG_SYNTAX_HIGHLIGHTING = (1 << 7) // Destaca sintaxe em disassembly
    } memory_viewer_flags_t;

    // Tipos de regiões de memória
    typedef enum
    {
        MEMORY_REGION_RAM,     // RAM do sistema
        MEMORY_REGION_ROM,     // ROM/cartridge
        MEMORY_REGION_VRAM,    // Memória de vídeo
        MEMORY_REGION_IO,      // Registradores de I/O
        MEMORY_REGION_SPRITE,  // Tabela de sprites
        MEMORY_REGION_PALETTE, // Paletas
        MEMORY_REGION_BANK,    // Banco de memória mapeado
        MEMORY_REGION_CUSTOM   // Região personalizada
    } memory_region_type_t;

    // Descrição de uma região de memória
    typedef struct
    {
        char name[32];             // Nome da região
        memory_region_type_t type; // Tipo da região
        uint32_t start_address;    // Endereço inicial
        uint32_t end_address;      // Endereço final
        uint32_t visible_start;    // Endereço de início para visualização
        uint8_t *direct_ptr;       // Ponteiro direto (NULL se não acessível diretamente)
        uint8_t access_flags;      // Flags de acesso (leitura/escrita)
        uint32_t bank_number;      // Número do banco (se aplicável)
        bool active;               // Se a região está ativa
        uint32_t platform_id;      // ID da plataforma relacionada
    } memory_region_desc_t;

    // Representação de marcadores e anotações
    typedef struct
    {
        uint32_t address;   // Endereço
        char label[64];     // Rótulo/nome
        char comment[256];  // Comentário
        uint32_t color;     // Cor para exibição
        bool is_bookmark;   // Se é um bookmark
        bool is_breakpoint; // Se é um breakpoint
        bool is_watchpoint; // Se é um watchpoint
    } memory_annotation_t;

    // Configuração do visualizador
    typedef struct
    {
        memory_view_mode_t default_view_mode; // Modo de visualização padrão
        uint32_t bytes_per_row;               // Bytes por linha
        uint32_t visible_rows;                // Número de linhas visíveis
        uint32_t refresh_interval_ms;         // Intervalo de atualização automática
        uint32_t flags;                       // Flags de configuração
        uint32_t highlight_duration_ms;       // Duração do destaque para alterações
        uint32_t history_size;                // Tamanho do histórico de edições
        bool follow_execution;                // Seguir execução (para debugger)
    } memory_viewer_config_t;

    // Informações sobre alterações na memória
    typedef struct
    {
        uint32_t address;   // Endereço alterado
        uint8_t old_value;  // Valor antigo
        uint8_t new_value;  // Novo valor
        uint64_t timestamp; // Timestamp da alteração
    } memory_change_info_t;

    // Funções de callback para acesso personalizado à memória
    typedef uint8_t (*memory_read_callback_t)(void *context, uint32_t address);
    typedef void (*memory_write_callback_t)(void *context, uint32_t address, uint8_t value);

    // Função de callback para notificação de alterações
    typedef void (*memory_change_callback_t)(void *context, const memory_change_info_t *change);

    // Função de callback para visualização personalizada
    typedef void (*memory_custom_view_callback_t)(
        void *context,
        uint32_t address,
        const uint8_t *data,
        uint32_t size,
        char *output_buffer,
        uint32_t buffer_size);

    // Opaque handle para o visualizador de memória
    typedef struct memory_viewer_t memory_viewer_t;

    /**
     * @brief Cria uma nova instância do visualizador de memória
     *
     * @param platform_id ID da plataforma (NES, SMS, etc.)
     * @param context Contexto para callbacks
     * @param read_callback Função para leitura de memória
     * @param write_callback Função para escrita em memória (NULL se somente leitura)
     * @return memory_viewer_t* Ponteiro para instância ou NULL em caso de erro
     */
    memory_viewer_t *memory_viewer_create(
        uint32_t platform_id,
        void *context,
        memory_read_callback_t read_callback,
        memory_write_callback_t write_callback);

    /**
     * @brief Destrói uma instância do visualizador de memória
     *
     * @param viewer Ponteiro para a instância
     */
    void memory_viewer_destroy(memory_viewer_t *viewer);

    /**
     * @brief Configura o visualizador de memória
     *
     * @param viewer Ponteiro para a instância
     * @param config Ponteiro para configuração
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_configure(memory_viewer_t *viewer, const memory_viewer_config_t *config);

    /**
     * @brief Obtém a configuração atual do visualizador
     *
     * @param viewer Ponteiro para a instância
     * @param config Ponteiro para receber configuração
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_get_config(memory_viewer_t *viewer, memory_viewer_config_t *config);

    /**
     * @brief Adiciona uma região de memória ao visualizador
     *
     * @param viewer Ponteiro para a instância
     * @param region Descrição da região
     * @return int32_t ID da região ou valor negativo em caso de erro
     */
    int32_t memory_viewer_add_region(memory_viewer_t *viewer, const memory_region_desc_t *region);

    /**
     * @brief Remove uma região de memória
     *
     * @param viewer Ponteiro para a instância
     * @param region_id ID da região
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_remove_region(memory_viewer_t *viewer, int32_t region_id);

    /**
     * @brief Define a região ativa para visualização
     *
     * @param viewer Ponteiro para a instância
     * @param region_id ID da região
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_set_active_region(memory_viewer_t *viewer, int32_t region_id);

    /**
     * @brief Define o endereço de visualização atual
     *
     * @param viewer Ponteiro para a instância
     * @param address Endereço na memória
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_set_address(memory_viewer_t *viewer, uint32_t address);

    /**
     * @brief Obtém o endereço de visualização atual
     *
     * @param viewer Ponteiro para a instância
     * @return uint32_t Endereço atual ou 0xFFFFFFFF em caso de erro
     */
    uint32_t memory_viewer_get_address(memory_viewer_t *viewer);

    /**
     * @brief Define o modo de visualização
     *
     * @param viewer Ponteiro para a instância
     * @param mode Modo de visualização
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_set_view_mode(memory_viewer_t *viewer, memory_view_mode_t mode);

    /**
     * @brief Adiciona uma anotação/bookmark para um endereço
     *
     * @param viewer Ponteiro para a instância
     * @param annotation Informações da anotação
     * @return int32_t ID da anotação ou valor negativo em caso de erro
     */
    int32_t memory_viewer_add_annotation(memory_viewer_t *viewer, const memory_annotation_t *annotation);

    /**
     * @brief Remove uma anotação
     *
     * @param viewer Ponteiro para a instância
     * @param annotation_id ID da anotação
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_remove_annotation(memory_viewer_t *viewer, int32_t annotation_id);

    /**
     * @brief Busca por um valor específico na memória
     *
     * @param viewer Ponteiro para a instância
     * @param region_id ID da região (ou -1 para todas)
     * @param value Valor a ser buscado
     * @param size Tamanho do valor em bytes
     * @param start_address Endereço inicial para busca
     * @param results Array para armazenar resultados
     * @param max_results Tamanho máximo do array de resultados
     * @return int32_t Número de ocorrências encontradas ou valor negativo em caso de erro
     */
    int32_t memory_viewer_search(
        memory_viewer_t *viewer,
        int32_t region_id,
        const uint8_t *value,
        uint32_t size,
        uint32_t start_address,
        uint32_t *results,
        uint32_t max_results);

    /**
     * @brief Escreve um valor na memória
     *
     * @param viewer Ponteiro para a instância
     * @param address Endereço na memória
     * @param value Valor a ser escrito
     * @param size Tamanho do valor em bytes
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_write(
        memory_viewer_t *viewer,
        uint32_t address,
        const uint8_t *value,
        uint32_t size);

    /**
     * @brief Lê um valor da memória
     *
     * @param viewer Ponteiro para a instância
     * @param address Endereço na memória
     * @param buffer Buffer para receber valor
     * @param size Tamanho do buffer em bytes
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_read(
        memory_viewer_t *viewer,
        uint32_t address,
        uint8_t *buffer,
        uint32_t size);

    /**
     * @brief Adiciona um callback para notificação de alterações
     *
     * @param viewer Ponteiro para a instância
     * @param callback Função de callback
     * @param context Contexto para o callback
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_add_change_callback(
        memory_viewer_t *viewer,
        memory_change_callback_t callback,
        void *context);

    /**
     * @brief Define um callback para visualização personalizada
     *
     * @param viewer Ponteiro para a instância
     * @param callback Função de callback
     * @param context Contexto para o callback
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_set_custom_view(
        memory_viewer_t *viewer,
        memory_custom_view_callback_t callback,
        void *context);

    /**
     * @brief Gera uma representação textual do estado atual
     *
     * @param viewer Ponteiro para a instância
     * @param lines Número de linhas a gerar
     * @param buffer Buffer para receber a representação
     * @param buffer_size Tamanho do buffer em bytes
     * @return int32_t Número de bytes escritos ou valor negativo em caso de erro
     */
    int32_t memory_viewer_render_text(
        memory_viewer_t *viewer,
        uint32_t lines,
        char *buffer,
        uint32_t buffer_size);

    /**
     * @brief Exporta o conteúdo de uma região para um arquivo
     *
     * @param viewer Ponteiro para a instância
     * @param region_id ID da região
     * @param start_address Endereço inicial
     * @param size Tamanho em bytes
     * @param filename Nome do arquivo
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_export_region(
        memory_viewer_t *viewer,
        int32_t region_id,
        uint32_t start_address,
        uint32_t size,
        const char *filename);

    /**
     * @brief Importa dados de um arquivo para a memória
     *
     * @param viewer Ponteiro para a instância
     * @param address Endereço de destino
     * @param filename Nome do arquivo
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_import_file(
        memory_viewer_t *viewer,
        uint32_t address,
        const char *filename);

    /**
     * @brief Adiciona uma região de memória ao visualizador usando um mapa de memória
     *
     * @param viewer Ponteiro para a instância
     * @param map_index Índice no mapa de memória
     * @return int32_t ID da região ou valor negativo em caso de erro
     */
    int32_t memory_viewer_add_region_from_map(
        memory_viewer_t *viewer,
        uint32_t map_index);

    /**
     * @brief Ativa ou desativa o modo de edição
     *
     * @param viewer Ponteiro para a instância
     * @param enable true para ativar, false para desativar
     * @return bool Estado anterior
     */
    bool memory_viewer_enable_editing(memory_viewer_t *viewer, bool enable);

    /**
     * @brief Verifica se um endereço é válido para a região atual
     *
     * @param viewer Ponteiro para a instância
     * @param address Endereço a verificar
     * @return bool true se válido, false caso contrário
     */
    bool memory_viewer_is_address_valid(memory_viewer_t *viewer, uint32_t address);

    /**
     * @brief Define uma função de pré-processamento para transformar visualizações
     *
     * @param viewer Ponteiro para a instância
     * @param callback Função de callback
     * @param context Contexto para o callback
     * @return bool true se bem-sucedido, false caso contrário
     */
    typedef void (*memory_preprocess_callback_t)(
        void *context,
        uint32_t address,
        uint8_t *value,
        uint32_t size);

    bool memory_viewer_set_preprocess_callback(
        memory_viewer_t *viewer,
        memory_preprocess_callback_t callback,
        void *context);

    /**
     * @brief Acessa e acompanha alterações em um endereço específico (watch)
     *
     * @param viewer Ponteiro para a instância
     * @param address Endereço a observar
     * @param name Nome para o watch
     * @return int32_t ID do watch ou valor negativo em caso de erro
     */
    int32_t memory_viewer_add_watch(
        memory_viewer_t *viewer,
        uint32_t address,
        const char *name);

    /**
     * @brief Remove um watch
     *
     * @param viewer Ponteiro para a instância
     * @param watch_id ID do watch
     * @return bool true se bem-sucedido, false caso contrário
     */
    bool memory_viewer_remove_watch(memory_viewer_t *viewer, int32_t watch_id);

    /**
     * @brief Cria um histórico das últimas alterações em um endereço
     *
     * @param viewer Ponteiro para a instância
     * @param address Endereço a analisar
     * @param history Array para receber histórico
     * @param max_entries Tamanho máximo do array
     * @return int32_t Número de entradas ou valor negativo em caso de erro
     */
    int32_t memory_viewer_get_address_history(
        memory_viewer_t *viewer,
        uint32_t address,
        memory_change_info_t *history,
        uint32_t max_entries);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_VIEWER_H */
