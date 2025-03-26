/**
 * @file rom_db_io.c
 * @brief Implementação das funções de importação e exportação para o banco de dados de ROMs
 */

#include "rom_db.h"
#include "../global_defines.h"
#include "../logging/log.h"
#include "../../utils/json/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Declarações de funções internas
static bool rom_db_entry_to_json(const mega_emu_rom_db_entry_t *entry, cJSON *json_entry);
static bool rom_db_json_to_entry(const cJSON *json_entry, mega_emu_rom_db_entry_t *entry);
static char* rom_db_read_file_content(const char *file_path);
static bool rom_db_write_file_content(const char *file_path, const char *content);

// Referência externa para o contexto do banco de dados
extern sqlite3 *g_rom_db_handle;

/**
 * @brief Importa entradas para o banco de dados a partir de um arquivo JSON
 *
 * @param json_path Caminho do arquivo JSON de importação
 * @param callback Função de callback para notificar progresso (opcional)
 * @param user_data Dados do usuário para o callback
 * @param entries_added Ponteiro para variável que receberá o número de entradas adicionadas
 * @return true Se a importação foi realizada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_import_json(const char *json_path,
                              mega_emu_rom_db_progress_callback_t callback,
                              void *user_data, uint32_t *entries_added) {
    if (json_path == NULL || entries_added == NULL) {
        return false;
    }

    // Inicializar contador
    *entries_added = 0;

    // Ler o conteúdo do arquivo JSON
    char *json_content = rom_db_read_file_content(json_path);
    if (json_content == NULL) {
        MEGA_LOG_ERROR("ROM Database: Falha ao ler arquivo JSON: %s", json_path);
        return false;
    }

    // Parsear o JSON
    cJSON *json = cJSON_Parse(json_content);
    free(json_content); // Liberar o conteúdo, já parseado

    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            MEGA_LOG_ERROR("ROM Database: Erro ao parsear JSON: %s", error_ptr);
        } else {
            MEGA_LOG_ERROR("ROM Database: Erro desconhecido ao parsear JSON");
        }
        return false;
    }

    // Verificar se o JSON é um array
    if (!cJSON_IsArray(json)) {
        MEGA_LOG_ERROR("ROM Database: Formato JSON inválido, esperava um array");
        cJSON_Delete(json);
        return false;
    }

    // Obter o número total de entradas para o progresso
    int total_entries = cJSON_GetArraySize(json);
    int successful_entries = 0;

    // Iniciar transação para melhor performance
    if (sqlite3_exec(g_rom_db_handle, "BEGIN TRANSACTION", NULL, NULL, NULL) != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Falha ao iniciar transação");
        cJSON_Delete(json);
        return false;
    }

    // Processar cada entrada do array
    for (int i = 0; i < total_entries; i++) {
        cJSON *json_entry = cJSON_GetArrayItem(json, i);
        if (json_entry == NULL || !cJSON_IsObject(json_entry)) {
            continue;
        }

        // Converter para estrutura de entrada
        mega_emu_rom_db_entry_t entry;
        memset(&entry, 0, sizeof(mega_emu_rom_db_entry_t));

        if (rom_db_json_to_entry(json_entry, &entry)) {
            // Adicionar ao banco de dados
            if (mega_emu_rom_db_add_entry(&entry)) {
                successful_entries++;
            }
        }

        // Notificar progresso
        if (callback != NULL) {
            callback(i + 1, total_entries, user_data);
        }
    }

    // Finalizar transação
    if (sqlite3_exec(g_rom_db_handle, "COMMIT", NULL, NULL, NULL) != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Falha ao finalizar transação");
        sqlite3_exec(g_rom_db_handle, "ROLLBACK", NULL, NULL, NULL);
        cJSON_Delete(json);
        return false;
    }

    // Atualizar contador
    *entries_added = successful_entries;

    // Limpar recursos
    cJSON_Delete(json);

    MEGA_LOG_INFO("ROM Database: Importação concluída. %d/%d entradas adicionadas.",
                 successful_entries, total_entries);

    return true;
}

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
                              void *user_data) {
    if (json_path == NULL) {
        return false;
    }

    // Obter o número total de entradas
    int total_entries = 0;
    sqlite3_stmt *count_stmt;

    int rc = sqlite3_prepare_v2(g_rom_db_handle, "SELECT COUNT(*) FROM rom_entries", -1, &count_stmt, NULL);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(count_stmt) == SQLITE_ROW) {
            total_entries = sqlite3_column_int(count_stmt, 0);
        }
        sqlite3_finalize(count_stmt);
    }

    if (total_entries == 0) {
        MEGA_LOG_WARN("ROM Database: Nenhuma entrada para exportar");
        return false;
    }

    // Criar array JSON
    cJSON *json_array = cJSON_CreateArray();
    if (json_array == NULL) {
        MEGA_LOG_ERROR("ROM Database: Falha ao criar array JSON");
        return false;
    }

    // Consulta para obter todas as entradas
    sqlite3_stmt *stmt;
    const char *query = "SELECT * FROM rom_entries";
    rc = sqlite3_prepare_v2(g_rom_db_handle, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Falha ao preparar consulta: %s", sqlite3_errmsg(g_rom_db_handle));
        cJSON_Delete(json_array);
        return false;
    }

    // Processar cada entrada
    int processed = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Extrair dados para a estrutura
        mega_emu_rom_db_entry_t entry;
        memset(&entry, 0, sizeof(mega_emu_rom_db_entry_t));

        entry.id = sqlite3_column_int(stmt, 0);

        const char *title = (const char*)sqlite3_column_text(stmt, 1);
        if (title) strncpy(entry.title, title, sizeof(entry.title) - 1);

        const char *alt_title = (const char*)sqlite3_column_text(stmt, 2);
        if (alt_title) strncpy(entry.alt_title, alt_title, sizeof(entry.alt_title) - 1);

        // ... continuar extraindo os campos restantes da mesma forma que na função de pesquisa

        // Converter para JSON e adicionar ao array
        cJSON *json_entry = cJSON_CreateObject();
        if (json_entry != NULL) {
            if (rom_db_entry_to_json(&entry, json_entry)) {
                cJSON_AddItemToArray(json_array, json_entry);
            } else {
                cJSON_Delete(json_entry);
            }
        }

        // Notificar progresso
        processed++;
        if (callback != NULL) {
            callback(processed, total_entries, user_data);
        }
    }

    sqlite3_finalize(stmt);

    // Converter o array JSON para string
    char *json_str = cJSON_Print(json_array);
    cJSON_Delete(json_array);

    if (json_str == NULL) {
        MEGA_LOG_ERROR("ROM Database: Falha ao serializar JSON");
        return false;
    }

    // Escrever no arquivo
    bool result = rom_db_write_file_content(json_path, json_str);
    free(json_str);

    if (result) {
        MEGA_LOG_INFO("ROM Database: Exportação concluída. %d entradas exportadas para %s",
                     processed, json_path);
    } else {
        MEGA_LOG_ERROR("ROM Database: Falha ao escrever arquivo JSON: %s", json_path);
    }

    return result;
}

/**
 * @brief Converte uma entrada do banco de dados para objeto JSON
 *
 * @param entry Entrada a ser convertida
 * @param json_entry Objeto JSON que receberá os dados
 * @return true Se convertido com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_entry_to_json(const mega_emu_rom_db_entry_t *entry, cJSON *json_entry) {
    if (entry == NULL || json_entry == NULL) {
        return false;
    }

    // Adicionar campos básicos
    cJSON_AddNumberToObject(json_entry, "id", entry->id);
    cJSON_AddStringToObject(json_entry, "title", entry->title);

    if (entry->alt_title[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "alt_title", entry->alt_title);
    }

    if (entry->developer[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "developer", entry->developer);
    }

    if (entry->publisher[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "publisher", entry->publisher);
    }

    if (entry->release_date[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "release_date", entry->release_date);
    }

    // Adicionar enums
    cJSON_AddNumberToObject(json_entry, "platform", entry->platform);
    cJSON_AddNumberToObject(json_entry, "region", entry->region);
    cJSON_AddNumberToObject(json_entry, "compatibility", entry->compatibility);
    cJSON_AddNumberToObject(json_entry, "media_type", entry->media_type);
    cJSON_AddNumberToObject(json_entry, "genre", entry->genre);
    cJSON_AddNumberToObject(json_entry, "input_type", entry->input_type);

    if (entry->description[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "description", entry->description);
    }

    // Adicionar hashes
    char md5_str[33] = {0};
    char sha1_str[41] = {0};
    char crc32_str[9] = {0};

    mega_emu_rom_db_hash_to_string(entry, md5_str, sizeof(md5_str), 1);
    mega_emu_rom_db_hash_to_string(entry, sha1_str, sizeof(sha1_str), 2);
    mega_emu_rom_db_hash_to_string(entry, crc32_str, sizeof(crc32_str), 0);

    cJSON_AddStringToObject(json_entry, "md5", md5_str);
    cJSON_AddStringToObject(json_entry, "sha1", sha1_str);
    cJSON_AddStringToObject(json_entry, "crc32", crc32_str);

    // Adicionar outros campos
    cJSON_AddNumberToObject(json_entry, "size", entry->size);
    cJSON_AddNumberToObject(json_entry, "players", entry->players);

    if (entry->serial[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "serial", entry->serial);
    }

    if (entry->version[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "version", entry->version);
    }

    if (entry->save_type[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "save_type", entry->save_type);
    }

    cJSON_AddBoolToObject(json_entry, "has_battery", entry->has_battery);
    cJSON_AddNumberToObject(json_entry, "flags", entry->flags);

    if (entry->extra_data[0] != '\0') {
        cJSON *extra_json = cJSON_Parse(entry->extra_data);
        if (extra_json != NULL) {
            cJSON_AddItemToObject(json_entry, "extra_data", extra_json);
        } else {
            cJSON_AddStringToObject(json_entry, "extra_data", entry->extra_data);
        }
    }

    cJSON_AddNumberToObject(json_entry, "db_revision", entry->db_revision);

    if (entry->added_date[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "added_date", entry->added_date);
    }

    if (entry->updated_date[0] != '\0') {
        cJSON_AddStringToObject(json_entry, "updated_date", entry->updated_date);
    }

    return true;
}

/**
 * @brief Converte um objeto JSON para entrada do banco de dados
 *
 * @param json_entry Objeto JSON com os dados
 * @param entry Estrutura que receberá os dados
 * @return true Se convertido com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_json_to_entry(const cJSON *json_entry, mega_emu_rom_db_entry_t *entry) {
    if (json_entry == NULL || entry == NULL) {
        return false;
    }

    // Extrair campos básicos
    cJSON *id_json = cJSON_GetObjectItem(json_entry, "id");
    if (cJSON_IsNumber(id_json)) {
        entry->id = id_json->valueint;
    }

    cJSON *title_json = cJSON_GetObjectItem(json_entry, "title");
    if (cJSON_IsString(title_json) && title_json->valuestring != NULL) {
        strncpy(entry->title, title_json->valuestring, sizeof(entry->title) - 1);
    } else {
        // Título é obrigatório
        return false;
    }

    cJSON *alt_title_json = cJSON_GetObjectItem(json_entry, "alt_title");
    if (cJSON_IsString(alt_title_json) && alt_title_json->valuestring != NULL) {
        strncpy(entry->alt_title, alt_title_json->valuestring, sizeof(entry->alt_title) - 1);
    }

    // Continuar extraindo os demais campos...

    // Extrair hashes
    cJSON *md5_json = cJSON_GetObjectItem(json_entry, "md5");
    cJSON *sha1_json = cJSON_GetObjectItem(json_entry, "sha1");
    cJSON *crc32_json = cJSON_GetObjectItem(json_entry, "crc32");

    if (cJSON_IsString(md5_json) && md5_json->valuestring != NULL) {
        mega_emu_rom_db_string_to_hash(md5_json->valuestring, entry, 1);
    }

    if (cJSON_IsString(sha1_json) && sha1_json->valuestring != NULL) {
        mega_emu_rom_db_string_to_hash(sha1_json->valuestring, entry, 2);
    }

    if (cJSON_IsString(crc32_json) && crc32_json->valuestring != NULL) {
        mega_emu_rom_db_string_to_hash(crc32_json->valuestring, entry, 0);
    }

    // Definir data atual para campos de data se não fornecidos
    char current_date[12];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_info);

    cJSON *added_date_json = cJSON_GetObjectItem(json_entry, "added_date");
    if (cJSON_IsString(added_date_json) && added_date_json->valuestring != NULL) {
        strncpy(entry->added_date, added_date_json->valuestring, sizeof(entry->added_date) - 1);
    } else {
        strncpy(entry->added_date, current_date, sizeof(entry->added_date) - 1);
    }

    cJSON *updated_date_json = cJSON_GetObjectItem(json_entry, "updated_date");
    if (cJSON_IsString(updated_date_json) && updated_date_json->valuestring != NULL) {
        strncpy(entry->updated_date, updated_date_json->valuestring, sizeof(entry->updated_date) - 1);
    } else {
        strncpy(entry->updated_date, current_date, sizeof(entry->updated_date) - 1);
    }

    return true;
}

/**
 * @brief Lê o conteúdo completo de um arquivo
 *
 * @param file_path Caminho do arquivo
 * @return char* Conteúdo do arquivo ou NULL em caso de erro (deve ser liberado pelo chamador)
 */
static char* rom_db_read_file_content(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        return NULL;
    }

    // Obter tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        return NULL;
    }

    // Alocar memória para o conteúdo
    char *content = (char*)malloc(file_size + 1);
    if (content == NULL) {
        fclose(file);
        return NULL;
    }

    // Ler o conteúdo
    size_t read_size = fread(content, 1, file_size, file);
    fclose(file);

    if (read_size != (size_t)file_size) {
        free(content);
        return NULL;
    }

    // Garantir que termine com null
    content[file_size] = '\0';

    return content;
}

/**
 * @brief Escreve conteúdo em um arquivo
 *
 * @param file_path Caminho do arquivo
 * @param content Conteúdo a ser escrito
 * @return true Se escrito com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_write_file_content(const char *file_path, const char *content) {
    if (file_path == NULL || content == NULL) {
        return false;
    }

    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        return false;
    }

    size_t content_len = strlen(content);
    size_t written = fwrite(content, 1, content_len, file);

    fclose(file);

    return written == content_len;
}
