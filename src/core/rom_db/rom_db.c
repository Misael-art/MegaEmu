/**
 * @file rom_db.c
 * @brief Implementação do banco de dados de ROMs
 */

#include "rom_db.h"
#include "../global_defines.h"
#include "../logging/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <time.h>

// Bibliotecas para cálculo de hashes
#include "../../utils/md5.h"
#include "../../utils/sha1.h"
#include "../../utils/crc32.h"

// Constantes
#define ROM_DB_VERSION 1
#define ROM_DB_MAX_BUFFER 4096
#define ROM_DB_HASH_BUFFER_SIZE 8192
#define ROM_DB_DEFAULT_PATH "romdb.sqlite"

// Estrutura para armazenar o contexto do banco de dados
typedef struct {
    sqlite3 *db;                    // Ponteiro para o banco de dados SQLite
    char db_path[MAX_PATH_LENGTH];  // Caminho do arquivo de banco de dados
    bool initialized;               // Flag para indicar se foi inicializado
    uint32_t version;               // Versão do banco de dados
    uint32_t entry_count;           // Número total de entradas
} rom_db_context_t;

// Contexto global do banco de dados
static rom_db_context_t g_rom_db = {0};

// Declarações de funções internas
static bool rom_db_create_tables(void);
static bool rom_db_check_version(void);
static bool rom_db_update_schema(uint32_t current_version);
static char* rom_db_escape_string(const char *str);
static bool rom_db_extract_hash_from_row(sqlite3_stmt *stmt, int col_offset, mega_emu_rom_db_hash_t *hash);
static bool rom_db_execute_simple_query(const char *query);
static int rom_db_get_single_int(const char *query);

/**
 * @brief Inicializa o banco de dados de ROMs
 *
 * @param db_path Caminho para o arquivo do banco de dados
 * @return true Se inicializado com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_init(const char *db_path) {
    if (g_rom_db.initialized) {
        MEGA_LOG_INFO("ROM Database: Já inicializado.");
        return true;
    }

    // Usar caminho padrão se nenhum for fornecido
    if (db_path == NULL || db_path[0] == '\0') {
        db_path = ROM_DB_DEFAULT_PATH;
    }

    // Copiar o caminho do banco de dados
    strncpy(g_rom_db.db_path, db_path, MAX_PATH_LENGTH - 1);
    g_rom_db.db_path[MAX_PATH_LENGTH - 1] = '\0';

    // Abrir/criar o banco de dados
    int rc = sqlite3_open(g_rom_db.db_path, &g_rom_db.db);
    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Falha ao abrir/criar o banco de dados: %s", sqlite3_errmsg(g_rom_db.db));
        sqlite3_close(g_rom_db.db);
        g_rom_db.db = NULL;
        return false;
    }

    // Ativar chaves estrangeiras
    rc = sqlite3_exec(g_rom_db.db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        MEGA_LOG_WARN("ROM Database: Falha ao ativar chaves estrangeiras: %s", sqlite3_errmsg(g_rom_db.db));
    }

    // Verificar se as tabelas existem e criar se necessário
    if (!rom_db_create_tables()) {
        MEGA_LOG_ERROR("ROM Database: Falha ao criar tabelas.");
        sqlite3_close(g_rom_db.db);
        g_rom_db.db = NULL;
        return false;
    }

    // Verificar e atualizar o esquema do banco de dados
    if (!rom_db_check_version()) {
        MEGA_LOG_ERROR("ROM Database: Falha ao verificar versão do banco de dados.");
        sqlite3_close(g_rom_db.db);
        g_rom_db.db = NULL;
        return false;
    }

    // Obter contagem de entradas
    g_rom_db.entry_count = rom_db_get_single_int("SELECT COUNT(*) FROM rom_entries");

    g_rom_db.initialized = true;
    MEGA_LOG_INFO("ROM Database: Inicializado com sucesso. Versão %d, %d entradas.",
                 g_rom_db.version, g_rom_db.entry_count);

    return true;
}

/**
 * @brief Finaliza o banco de dados de ROMs e libera recursos
 */
void mega_emu_rom_db_shutdown(void) {
    if (!g_rom_db.initialized) {
        return;
    }

    if (g_rom_db.db != NULL) {
        sqlite3_close(g_rom_db.db);
        g_rom_db.db = NULL;
    }

    g_rom_db.initialized = false;
    g_rom_db.entry_count = 0;

    MEGA_LOG_INFO("ROM Database: Finalizado.");
}

/**
 * @brief Verifica se o banco de dados está inicializado
 *
 * @return true Se estiver inicializado
 * @return false Se não estiver inicializado
 */
bool mega_emu_rom_db_is_initialized(void) {
    return g_rom_db.initialized;
}

/**
 * @brief Obtém metadados do banco de dados
 *
 * @param metadata Ponteiro para estrutura que receberá os metadados
 * @return true Se os metadados foram obtidos com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_get_metadata(mega_emu_rom_db_metadata_t *metadata) {
    if (!g_rom_db.initialized || metadata == NULL) {
        return false;
    }

    memset(metadata, 0, sizeof(mega_emu_rom_db_metadata_t));

    metadata->version = g_rom_db.version;
    metadata->entry_count = g_rom_db.entry_count;

    // Obter data da última atualização (pode ser do arquivo ou do banco)
    struct stat st;
    if (stat(g_rom_db.db_path, &st) == 0) {
        struct tm *tm_info = localtime(&st.st_mtime);
        strftime(metadata->build_date, sizeof(metadata->build_date), "%Y-%m-%d", tm_info);
    } else {
        strcpy(metadata->build_date, "Desconhecido");
    }

    strcpy(metadata->description, "Banco de dados de ROMs do Mega_Emu");

    // Obter estatísticas de entradas por plataforma
    sqlite3_stmt *stmt;
    const char *query = "SELECT platform, COUNT(*) FROM rom_entries GROUP BY platform";
    int rc = sqlite3_prepare_v2(g_rom_db.db, query, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int platform = sqlite3_column_int(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);

            if (platform >= 0 && platform < ROM_DB_PLATFORM_COUNT) {
                metadata->entries_by_platform[platform] = count;
            }
        }
        sqlite3_finalize(stmt);
    }

    // Obter estatísticas de entradas por região
    query = "SELECT region, COUNT(*) FROM rom_entries GROUP BY region";
    rc = sqlite3_prepare_v2(g_rom_db.db, query, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int region = sqlite3_column_int(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);

            if (region >= 0 && region < ROM_DB_REGION_COUNT) {
                metadata->entries_by_region[region] = count;
            }
        }
        sqlite3_finalize(stmt);
    }

    return true;
}

/**
 * @brief Pesquisa ROMs no banco de dados
 *
 * @param search Critérios de pesquisa
 * @param result Ponteiro para estrutura que receberá o resultado
 * @return true Se a pesquisa foi realizada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_search(const mega_emu_rom_db_search_t *search,
                            mega_emu_rom_db_search_result_t *result) {
    if (!g_rom_db.initialized || search == NULL || result == NULL) {
        return false;
    }

    // Inicializar o resultado
    memset(result, 0, sizeof(mega_emu_rom_db_search_result_t));
    result->success = false;

    // Construir consulta SQL base
    char base_query[ROM_DB_MAX_BUFFER];
    snprintf(base_query, ROM_DB_MAX_BUFFER,
             "SELECT id, title, alt_title, developer, publisher, release_date, "
             "platform, region, compatibility, media_type, genre, input_type, "
             "description, md5, sha1, crc32, size, players, serial, version, "
             "save_type, has_battery, flags, extra_data, db_revision, "
             "added_date, updated_date FROM rom_entries WHERE 1=1");

    // Buffer para condições WHERE
    char where_clause[ROM_DB_MAX_BUFFER] = "";
    int offset = 0;

    // Adicionar condições de pesquisa
    if (search->title[0] != '\0') {
        char *escaped_title = rom_db_escape_string(search->title);
        offset += snprintf(where_clause + offset, ROM_DB_MAX_BUFFER - offset,
                          " AND (title LIKE '%%%s%%' OR alt_title LIKE '%%%s%%')",
                          escaped_title, escaped_title);
        free(escaped_title);
    }

    if (search->use_platform) {
        offset += snprintf(where_clause + offset, ROM_DB_MAX_BUFFER - offset,
                          " AND platform = %d", search->platform);
    }

    if (search->use_region) {
        offset += snprintf(where_clause + offset, ROM_DB_MAX_BUFFER - offset,
                          " AND region = %d", search->region);
    }

    if (search->use_genre) {
        offset += snprintf(where_clause + offset, ROM_DB_MAX_BUFFER - offset,
                          " AND genre = %d", search->genre);
    }

    if (search->use_hash) {
        char md5_str[33] = {0};
        char sha1_str[41] = {0};
        char crc_str[9] = {0};

        mega_emu_rom_db_hash_to_string(&search->hash, md5_str, sizeof(md5_str), 1);
        mega_emu_rom_db_hash_to_string(&search->hash, sha1_str, sizeof(sha1_str), 2);
        mega_emu_rom_db_hash_to_string(&search->hash, crc_str, sizeof(crc_str), 0);

        offset += snprintf(where_clause + offset, ROM_DB_MAX_BUFFER - offset,
                          " AND (md5 = '%s' OR sha1 = '%s' OR crc32 = '%s')",
                          md5_str, sha1_str, crc_str);
    }

    // Construir cláusula ORDER BY
    char order_clause[128] = " ORDER BY title ASC";

    if (search->sort_by == 0) {
        snprintf(order_clause, sizeof(order_clause), " ORDER BY title %s",
                 search->sort_ascending ? "ASC" : "DESC");
    } else if (search->sort_by == 1) {
        snprintf(order_clause, sizeof(order_clause), " ORDER BY release_date %s",
                 search->sort_ascending ? "ASC" : "DESC");
    } else if (search->sort_by == 2) {
        snprintf(order_clause, sizeof(order_clause), " ORDER BY developer %s",
                 search->sort_ascending ? "ASC" : "DESC");
    } else if (search->sort_by == 3) {
        snprintf(order_clause, sizeof(order_clause), " ORDER BY added_date %s",
                 search->sort_ascending ? "ASC" : "DESC");
    }

    // Construir cláusula LIMIT para paginação
    char limit_clause[64] = "";

    if (search->items_per_page > 0) {
        uint32_t offset_value = search->page * search->items_per_page;
        snprintf(limit_clause, sizeof(limit_clause), " LIMIT %u OFFSET %u",
                 search->items_per_page, offset_value);
    }

    // Construir consulta SQL completa
    char query[ROM_DB_MAX_BUFFER * 2];
    snprintf(query, sizeof(query), "%s%s%s%s", base_query, where_clause, order_clause, limit_clause);

    // Obter contagem total de correspondências (para paginação)
    char count_query[ROM_DB_MAX_BUFFER * 2];
    snprintf(count_query, sizeof(count_query), "SELECT COUNT(*) FROM rom_entries WHERE 1=1%s", where_clause);
    result->total_matches = rom_db_get_single_int(count_query);

    // Executar a consulta
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_rom_db.db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        snprintf(result->error_message, sizeof(result->error_message),
                 "Erro ao preparar consulta: %s", sqlite3_errmsg(g_rom_db.db));
        return false;
    }

    // Contar quantos resultados serão retornados
    uint32_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }
    sqlite3_reset(stmt);

    // Alocar memória para os resultados
    if (count > 0) {
        result->entries = (mega_emu_rom_db_entry_t*)malloc(count * sizeof(mega_emu_rom_db_entry_t));
        if (result->entries == NULL) {
            snprintf(result->error_message, sizeof(result->error_message), "Falha ao alocar memória para resultados");
            sqlite3_finalize(stmt);
            return false;
        }
        memset(result->entries, 0, count * sizeof(mega_emu_rom_db_entry_t));
    } else {
        result->entries = NULL;
    }

    // Preencher os resultados
    uint32_t index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && index < count) {
        mega_emu_rom_db_entry_t *entry = &result->entries[index];

        entry->id = sqlite3_column_int(stmt, 0);

        // Copiar strings com verificação para evitar NULL
        const char *title = (const char*)sqlite3_column_text(stmt, 1);
        if (title) strncpy(entry->title, title, sizeof(entry->title) - 1);

        const char *alt_title = (const char*)sqlite3_column_text(stmt, 2);
        if (alt_title) strncpy(entry->alt_title, alt_title, sizeof(entry->alt_title) - 1);

        const char *developer = (const char*)sqlite3_column_text(stmt, 3);
        if (developer) strncpy(entry->developer, developer, sizeof(entry->developer) - 1);

        const char *publisher = (const char*)sqlite3_column_text(stmt, 4);
        if (publisher) strncpy(entry->publisher, publisher, sizeof(entry->publisher) - 1);

        const char *release_date = (const char*)sqlite3_column_text(stmt, 5);
        if (release_date) strncpy(entry->release_date, release_date, sizeof(entry->release_date) - 1);

        // Enums
        entry->platform = sqlite3_column_int(stmt, 6);
        entry->region = sqlite3_column_int(stmt, 7);
        entry->compatibility = sqlite3_column_int(stmt, 8);
        entry->media_type = sqlite3_column_int(stmt, 9);
        entry->genre = sqlite3_column_int(stmt, 10);
        entry->input_type = sqlite3_column_int(stmt, 11);

        const char *description = (const char*)sqlite3_column_text(stmt, 12);
        if (description) strncpy(entry->description, description, sizeof(entry->description) - 1);

        // Extrair os hashes
        rom_db_extract_hash_from_row(stmt, 13, &entry->hash);

        entry->size = sqlite3_column_int(stmt, 16);
        entry->players = sqlite3_column_int(stmt, 17);

        const char *serial = (const char*)sqlite3_column_text(stmt, 18);
        if (serial) strncpy(entry->serial, serial, sizeof(entry->serial) - 1);

        const char *version = (const char*)sqlite3_column_text(stmt, 19);
        if (version) strncpy(entry->version, version, sizeof(entry->version) - 1);

        const char *save_type = (const char*)sqlite3_column_text(stmt, 20);
        if (save_type) strncpy(entry->save_type, save_type, sizeof(entry->save_type) - 1);

        entry->has_battery = sqlite3_column_int(stmt, 21) != 0;
        entry->flags = sqlite3_column_int(stmt, 22);

        const char *extra_data = (const char*)sqlite3_column_text(stmt, 23);
        if (extra_data) strncpy(entry->extra_data, extra_data, sizeof(entry->extra_data) - 1);

        entry->db_revision = sqlite3_column_int(stmt, 24);

        const char *added_date = (const char*)sqlite3_column_text(stmt, 25);
        if (added_date) strncpy(entry->added_date, added_date, sizeof(entry->added_date) - 1);

        const char *updated_date = (const char*)sqlite3_column_text(stmt, 26);
        if (updated_date) strncpy(entry->updated_date, updated_date, sizeof(entry->updated_date) - 1);

        index++;
    }

    result->count = index;
    result->success = true;

    sqlite3_finalize(stmt);
    return true;
}

/**
 * @brief Libera os recursos de um resultado de pesquisa
 *
 * @param result Resultado de pesquisa a ser liberado
 */
void mega_emu_rom_db_free_search_result(mega_emu_rom_db_search_result_t *result) {
    if (result == NULL) {
        return;
    }

    if (result->entries != NULL) {
        free(result->entries);
        result->entries = NULL;
    }

    result->count = 0;
    result->total_matches = 0;
    result->success = false;
    result->error_message[0] = '\0';
}

// Funções internas (utilitários)

/**
 * @brief Cria as tabelas necessárias no banco de dados
 *
 * @return true Se as tabelas foram criadas com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_create_tables(void) {
    // Tabela de metadados do banco de dados
    const char *metadata_sql =
        "CREATE TABLE IF NOT EXISTS db_metadata ("
        "   key TEXT PRIMARY KEY,"
        "   value TEXT NOT NULL"
        ");";

    // Tabela principal de entradas de ROMs
    const char *entries_sql =
        "CREATE TABLE IF NOT EXISTS rom_entries ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   title TEXT NOT NULL,"
        "   alt_title TEXT,"
        "   developer TEXT,"
        "   publisher TEXT,"
        "   release_date TEXT,"
        "   platform INTEGER NOT NULL,"
        "   region INTEGER NOT NULL,"
        "   compatibility INTEGER NOT NULL DEFAULT 0,"
        "   media_type INTEGER NOT NULL DEFAULT 0,"
        "   genre INTEGER NOT NULL DEFAULT 0,"
        "   input_type INTEGER NOT NULL DEFAULT 0,"
        "   description TEXT,"
        "   md5 TEXT NOT NULL,"
        "   sha1 TEXT NOT NULL,"
        "   crc32 TEXT NOT NULL,"
        "   size INTEGER NOT NULL DEFAULT 0,"
        "   players INTEGER NOT NULL DEFAULT 1,"
        "   serial TEXT,"
        "   version TEXT,"
        "   save_type TEXT,"
        "   has_battery INTEGER NOT NULL DEFAULT 0,"
        "   flags INTEGER NOT NULL DEFAULT 0,"
        "   extra_data TEXT,"
        "   db_revision INTEGER NOT NULL DEFAULT 1,"
        "   added_date TEXT NOT NULL,"
        "   updated_date TEXT NOT NULL"
        ");";

    // Índices para pesquisa rápida
    const char *indices_sql =
        "CREATE INDEX IF NOT EXISTS idx_title ON rom_entries (title);"
        "CREATE INDEX IF NOT EXISTS idx_platform ON rom_entries (platform);"
        "CREATE INDEX IF NOT EXISTS idx_md5 ON rom_entries (md5);"
        "CREATE INDEX IF NOT EXISTS idx_sha1 ON rom_entries (sha1);"
        "CREATE INDEX IF NOT EXISTS idx_crc32 ON rom_entries (crc32);";

    // Executar as criações de tabelas
    if (!rom_db_execute_simple_query(metadata_sql) ||
        !rom_db_execute_simple_query(entries_sql) ||
        !rom_db_execute_simple_query(indices_sql)) {
        return false;
    }

    return true;
}

/**
 * @brief Verifica a versão do banco de dados e atualiza se necessário
 *
 * @return true Se a versão foi verificada com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_check_version(void) {
    int version = 0;
    sqlite3_stmt *stmt;

    // Verificar se a tabela de metadados tem a versão
    int rc = sqlite3_prepare_v2(g_rom_db.db,
                             "SELECT value FROM db_metadata WHERE key = 'version'",
                             -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *value = (const char*)sqlite3_column_text(stmt, 0);
            version = atoi(value);
        }
        sqlite3_finalize(stmt);
    }

    // Se não existe versão, inicializar com a versão atual
    if (version == 0) {
        char query[128];
        snprintf(query, sizeof(query),
                "INSERT OR REPLACE INTO db_metadata (key, value) VALUES ('version', '%d')",
                ROM_DB_VERSION);

        if (!rom_db_execute_simple_query(query)) {
            return false;
        }

        version = ROM_DB_VERSION;
    }
    // Se a versão é antiga, atualizar o esquema
    else if (version < ROM_DB_VERSION) {
        if (!rom_db_update_schema(version)) {
            return false;
        }

        // Atualizar a versão no banco
        char query[128];
        snprintf(query, sizeof(query),
                "UPDATE db_metadata SET value = '%d' WHERE key = 'version'",
                ROM_DB_VERSION);

        if (!rom_db_execute_simple_query(query)) {
            return false;
        }

        version = ROM_DB_VERSION;
    }

    g_rom_db.version = version;
    return true;
}

/**
 * @brief Atualiza o esquema do banco de dados para a versão mais recente
 *
 * @param current_version Versão atual do banco de dados
 * @return true Se o esquema foi atualizado com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_update_schema(uint32_t current_version) {
    // Implementar migrações quando houver versões futuras
    if (current_version < 1) {
        // Migração da versão 0 para 1 (se necessário no futuro)
    }

    return true;
}

/**
 * @brief Escapa uma string para uso em consulta SQL
 *
 * @param str String a ser escapada
 * @return char* Nova string escapada (deve ser liberada pelo chamador)
 */
static char* rom_db_escape_string(const char *str) {
    if (str == NULL) {
        return strdup("");
    }

    size_t len = strlen(str);
    char *escaped = (char*)malloc(len * 2 + 1);

    if (escaped == NULL) {
        return strdup("");
    }

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\'') {
            escaped[j++] = '\'';
            escaped[j++] = '\'';
        } else {
            escaped[j++] = str[i];
        }
    }

    escaped[j] = '\0';
    return escaped;
}

/**
 * @brief Extrai os hashes de uma linha de resultado SQL
 *
 * @param stmt Statement SQL
 * @param col_offset Índice da coluna onde inicia o MD5
 * @param hash Estrutura para receber os hashes
 * @return true Se os hashes foram extraídos com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_extract_hash_from_row(sqlite3_stmt *stmt, int col_offset, mega_emu_rom_db_hash_t *hash) {
    const char *md5_str = (const char*)sqlite3_column_text(stmt, col_offset);
    const char *sha1_str = (const char*)sqlite3_column_text(stmt, col_offset + 1);
    const char *crc32_str = (const char*)sqlite3_column_text(stmt, col_offset + 2);

    // Converter as strings em bytes
    return
        mega_emu_rom_db_string_to_hash(md5_str, hash, 1) &&
        mega_emu_rom_db_string_to_hash(sha1_str, hash, 2) &&
        mega_emu_rom_db_string_to_hash(crc32_str, hash, 0);
}

/**
 * @brief Executa uma consulta SQL simples
 *
 * @param query Consulta SQL
 * @return true Se a consulta foi executada com sucesso
 * @return false Se ocorrer erro
 */
static bool rom_db_execute_simple_query(const char *query) {
    char *err_msg = NULL;
    int rc = sqlite3_exec(g_rom_db.db, query, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Erro SQL: %s\nQuery: %s", err_msg, query);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

/**
 * @brief Obtém um único valor inteiro de uma consulta SQL
 *
 * @param query Consulta SQL que retorna um único valor inteiro
 * @return int Valor inteiro retornado ou 0 em caso de erro
 */
static int rom_db_get_single_int(const char *query) {
    sqlite3_stmt *stmt;
    int result = 0;

    int rc = sqlite3_prepare_v2(g_rom_db.db, query, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return result;
}
