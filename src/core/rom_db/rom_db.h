/**
 * @file rom_db.h
 * @brief Interface para o banco de dados de ROMs
 */

#ifndef MEGA_EMU_ROM_DB_H
#define MEGA_EMU_ROM_DB_H

#include "rom_db_types.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Inicializa o banco de dados de ROMs
 *
 * @param db_path Caminho para o arquivo do banco de dados
 * @return true Se inicializado com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_init(const char *db_path);

/**
 * @brief Finaliza o banco de dados de ROMs e libera recursos
 */
void mega_emu_rom_db_shutdown(void);

/**
 * @brief Verifica se o banco de dados está inicializado
 *
 * @return true Se estiver inicializado
 * @return false Se não estiver inicializado
 */
bool mega_emu_rom_db_is_initialized(void);

/**
 * @brief Obtém metadados do banco de dados
 *
 * @param metadata Ponteiro para estrutura que receberá os metadados
 * @return true Se os metadados foram obtidos com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_get_metadata(mega_emu_rom_db_metadata_t *metadata);

/**
 * @brief Pesquisa ROMs no banco de dados
 *
 * @param search Critérios de pesquisa
 * @param result Ponteiro para estrutura que receberá o resultado
 * @return true Se a pesquisa foi realizada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_search(const mega_emu_rom_db_search_t *search,
                            mega_emu_rom_db_search_result_t *result);

/**
 * @brief Libera os recursos de um resultado de pesquisa
 *
 * @param result Resultado de pesquisa a ser liberado
 */
void mega_emu_rom_db_free_search_result(
    mega_emu_rom_db_search_result_t *result);

/**
 * @brief Obtém informações de uma ROM pelo hash
 *
 * @param hash Hash da ROM
 * @param entry Ponteiro para estrutura que receberá as informações
 * @return true Se as informações foram obtidas com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_get_by_hash(const mega_emu_rom_db_hash_t *hash,
                                 mega_emu_rom_db_entry_t *entry);

/**
 * @brief Obtém informações de uma ROM pelo ID
 *
 * @param id ID da ROM no banco de dados
 * @param entry Ponteiro para estrutura que receberá as informações
 * @return true Se as informações foram obtidas com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_get_by_id(uint32_t id, mega_emu_rom_db_entry_t *entry);

/**
 * @brief Calcula os hashes de um arquivo de ROM
 *
 * @param file_path Caminho do arquivo de ROM
 * @param hash Estrutura que receberá os hashes calculados
 * @param callback Função de callback para notificar progresso (opcional)
 * @param user_data Dados do usuário para o callback
 * @return true Se os hashes foram calculados com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_calculate_hash(
    const char *file_path, mega_emu_rom_db_hash_t *hash,
    mega_emu_rom_db_progress_callback_t callback, void *user_data);

/**
 * @brief Adiciona uma nova entrada ao banco de dados
 *
 * @param entry Nova entrada a ser adicionada
 * @return true Se adicionada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_add_entry(const mega_emu_rom_db_entry_t *entry);

/**
 * @brief Atualiza uma entrada existente no banco de dados
 *
 * @param entry Entrada com informações atualizadas (o ID deve corresponder a
 * uma entrada existente)
 * @return true Se atualizada com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_update_entry(const mega_emu_rom_db_entry_t *entry);

/**
 * @brief Remove uma entrada do banco de dados
 *
 * @param id ID da entrada a ser removida
 * @return true Se removida com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_remove_entry(uint32_t id);

/**
 * @brief Importa entradas para o banco de dados a partir de um arquivo JSON
 *
 * @param json_path Caminho do arquivo JSON de importação
 * @param callback Função de callback para notificar progresso (opcional)
 * @param user_data Dados do usuário para o callback
 * @param entries_added Ponteiro para variável que receberá o número de entradas
 * adicionadas
 * @return true Se a importação foi realizada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_import_json(const char *json_path,
                                 mega_emu_rom_db_progress_callback_t callback,
                                 void *user_data, uint32_t *entries_added);

/**
 * @brief Exporta o banco de dados para um arquivo JSON
 *
 * @param json_path Caminho do arquivo JSON de exportação
 * @param callback Função de callback para notificar progresso (opcional)
 * @param user_data Dados do usuário para o callback
 * @return true Se a exportação foi realizada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_export_json(const char *json_path,
                                 mega_emu_rom_db_progress_callback_t callback,
                                 void *user_data);

/**
 * @brief Obtém estatísticas do banco de dados
 *
 * @param platform_count Vetor para armazenar o número de ROMs por plataforma
 * @param region_count Vetor para armazenar o número de ROMs por região
 * @param total_entries Ponteiro para variável que receberá o número total de
 * entradas
 * @return true Se as estatísticas foram obtidas com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_get_stats(uint32_t platform_count[ROM_DB_PLATFORM_COUNT],
                               uint32_t region_count[ROM_DB_REGION_COUNT],
                               uint32_t *total_entries);

/**
 * @brief Compacta o banco de dados (remove espaços não utilizados)
 *
 * @return true Se a compactação foi realizada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_compact(void);

/**
 * @brief Função de utilidade para converter string em enum de plataforma
 *
 * @param platform_str Nome da plataforma
 * @return mega_emu_rom_db_platform_t Enum correspondente
 */
mega_emu_rom_db_platform_t
mega_emu_rom_db_string_to_platform(const char *platform_str);

/**
 * @brief Função de utilidade para converter enum de plataforma em string
 *
 * @param platform Enum da plataforma
 * @return const char* Nome da plataforma
 */
const char *
mega_emu_rom_db_platform_to_string(mega_emu_rom_db_platform_t platform);

/**
 * @brief Função de utilidade para converter string em enum de região
 *
 * @param region_str Nome da região
 * @return mega_emu_rom_db_region_t Enum correspondente
 */
mega_emu_rom_db_region_t
mega_emu_rom_db_string_to_region(const char *region_str);

/**
 * @brief Função de utilidade para converter enum de região em string
 *
 * @param region Enum da região
 * @return const char* Nome da região
 */
const char *mega_emu_rom_db_region_to_string(mega_emu_rom_db_region_t region);

/**
 * @brief Função de utilidade para converter string em enum de gênero
 *
 * @param genre_str Nome do gênero
 * @return mega_emu_rom_db_genre_t Enum correspondente
 */
mega_emu_rom_db_genre_t mega_emu_rom_db_string_to_genre(const char *genre_str);

/**
 * @brief Função de utilidade para converter enum de gênero em string
 *
 * @param genre Enum do gênero
 * @return const char* Nome do gênero
 */
const char *mega_emu_rom_db_genre_to_string(mega_emu_rom_db_genre_t genre);

/**
 * @brief Função de utilidade para converter hash em formato hexadecimal para
 * string
 *
 * @param hash Hash a ser convertido
 * @param hash_str Buffer para receber a string
 * @param buffer_size Tamanho do buffer
 * @param hash_type Tipo de hash (0=CRC32, 1=MD5, 2=SHA1)
 * @return true Se convertido com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_hash_to_string(const mega_emu_rom_db_hash_t *hash,
                                    char *hash_str, size_t buffer_size,
                                    uint8_t hash_type);

/**
 * @brief Função de utilidade para converter string hexadecimal em hash
 *
 * @param hash_str String contendo o hash em formato hexadecimal
 * @param hash Estrutura que receberá o hash
 * @param hash_type Tipo de hash (0=CRC32, 1=MD5, 2=SHA1)
 * @return true Se convertido com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_string_to_hash(const char *hash_str,
                                    mega_emu_rom_db_hash_t *hash,
                                    uint8_t hash_type);

#endif // MEGA_EMU_ROM_DB_H
