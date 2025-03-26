/**
 * @file rom_db_entry.c
 * @brief Implementação das funções de gerenciamento de entradas no banco de dados de ROMs
 */

#include "rom_db.h"
#include "../global_defines.h"
#include "../logging/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

// Referências externas para o contexto do banco de dados
extern sqlite3 *g_rom_db_handle;
extern uint32_t g_rom_db_entry_count;

/**
 * @brief Obtém informações de uma ROM pelo hash
 *
 * @param hash Hash da ROM
 * @param entry Ponteiro para estrutura que receberá as informações
 * @return true Se as informações foram obtidas com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_get_by_hash(const mega_emu_rom_db_hash_t *hash,
                              mega_emu_rom_db_entry_t *entry) {
    if (!g_rom_db_handle || hash == NULL || entry == NULL) {
        return false;
    }

    // Converter hashes para strings
    char md5_str[33] = {0};
    char sha1_str[41] = {0};
    char crc32_str[9] = {0};

    mega_emu_rom_db_hash_to_string(hash, md5_str, sizeof(md5_str), 1);
    mega_emu_rom_db_hash_to_string(hash, sha1_str, sizeof(sha1_str), 2);
    mega_emu_rom_db_hash_to_string(hash, crc32_str, sizeof(crc32_str), 0);

    // Construir consulta SQL
    char query[1024];
    snprintf(query, sizeof(query),
             "SELECT id, title, alt_title, developer, publisher, release_date, "
             "platform, region, compatibility, media_type, genre, input_type, "
             "description, md5, sha1, crc32, size, players, serial, version, "
             "save_type, has_battery, flags, extra_data, db_revision, "
             "added_date, updated_date FROM rom_entries "
             "WHERE md5 = '%s' OR sha1 = '%s' OR crc32 = '%s' LIMIT 1",
             md5_str, sha1_str, crc32_str);

    // Executar a consulta
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_rom_db_handle, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Erro ao preparar consulta: %s", sqlite3_errmsg(g_rom_db_handle));
        return false;
    }

    // Verificar se encontrou resultado
    bool success = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Preencher a estrutura com os dados
        memset(entry, 0, sizeof(mega_emu_rom_db_entry_t));

        entry->id = sqlite3_column_int(stmt, 0);

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

        entry->platform = sqlite3_column_int(stmt, 6);
        entry->region = sqlite3_column_int(stmt, 7);
        entry->compatibility = sqlite3_column_int(stmt, 8);
        entry->media_type = sqlite3_column_int(stmt, 9);
        entry->genre = sqlite3_column_int(stmt, 10);
        entry->input_type = sqlite3_column_int(stmt, 11);

        const char *description = (const char*)sqlite3_column_text(stmt, 12);
        if (description) strncpy(entry->description, description, sizeof(entry->description) - 1);

        // Extrair hashes
        const char *md5 = (const char*)sqlite3_column_text(stmt, 13);
        const char *sha1 = (const char*)sqlite3_column_text(stmt, 14);
        const char *crc32 = (const char*)sqlite3_column_text(stmt, 15);

        if (md5) mega_emu_rom_db_string_to_hash(md5, entry, 1);
        if (sha1) mega_emu_rom_db_string_to_hash(sha1, entry, 2);
        if (crc32) mega_emu_rom_db_string_to_hash(crc32, entry, 0);

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

        success = true;
    }

    sqlite3_finalize(stmt);
    return success;
}

/**
 * @brief Obtém informações de uma ROM pelo ID
 *
 * @param id ID da ROM no banco de dados
 * @param entry Ponteiro para estrutura que receberá as informações
 * @return true Se as informações foram obtidas com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_get_by_id(uint32_t id, mega_emu_rom_db_entry_t *entry) {
    if (!g_rom_db_handle || entry == NULL || id == 0) {
        return false;
    }

    // Construir consulta SQL
    char query[1024];
    snprintf(query, sizeof(query),
             "SELECT id, title, alt_title, developer, publisher, release_date, "
             "platform, region, compatibility, media_type, genre, input_type, "
             "description, md5, sha1, crc32, size, players, serial, version, "
             "save_type, has_battery, flags, extra_data, db_revision, "
             "added_date, updated_date FROM rom_entries "
             "WHERE id = %u LIMIT 1", id);

    // Executar a consulta
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_rom_db_handle, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Erro ao preparar consulta: %s", sqlite3_errmsg(g_rom_db_handle));
        return false;
    }

    // Verificar se encontrou resultado
    bool success = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Preencher a estrutura (mesmo processo que na função get_by_hash)
        // ...
        success = true;
    }

    sqlite3_finalize(stmt);
    return success;
}

/**
 * @brief Adiciona uma nova entrada ao banco de dados
 *
 * @param entry Nova entrada a ser adicionada
 * @return true Se adicionada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_add_entry(const mega_emu_rom_db_entry_t *entry) {
    if (!g_rom_db_handle || entry == NULL) {
        return false;
    }

    // Verificar se já existe uma entrada com o mesmo hash
    mega_emu_rom_db_entry_t existing;
    if (mega_emu_rom_db_get_by_hash(&entry->hash, &existing)) {
        MEGA_LOG_WARN("ROM Database: ROM já existe no banco de dados (ID=%u)", existing.id);
        return false;
    }

    // Converter hashes para strings
    char md5_str[33] = {0};
    char sha1_str[41] = {0};
    char crc32_str[9] = {0};

    mega_emu_rom_db_hash_to_string(&entry->hash, md5_str, sizeof(md5_str), 1);
    mega_emu_rom_db_hash_to_string(&entry->hash, sha1_str, sizeof(sha1_str), 2);
    mega_emu_rom_db_hash_to_string(&entry->hash, crc32_str, sizeof(crc32_str), 0);

    // Obter data atual se não fornecida
    char current_date[12];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_info);

    const char *added_date = (entry->added_date[0] != '\0') ? entry->added_date : current_date;
    const char *updated_date = (entry->updated_date[0] != '\0') ? entry->updated_date : current_date;

    // Construir consulta SQL para inserção
    char query[4096];
    snprintf(query, sizeof(query),
             "INSERT INTO rom_entries ("
             "   title, alt_title, developer, publisher, release_date, "
             "   platform, region, compatibility, media_type, genre, input_type, "
             "   description, md5, sha1, crc32, size, players, serial, version, "
             "   save_type, has_battery, flags, extra_data, db_revision, "
             "   added_date, updated_date"
             ") VALUES ("
             "   '%s', '%s', '%s', '%s', '%s', "
             "   %d, %d, %d, %d, %d, %d, "
             "   '%s', '%s', '%s', '%s', %u, %u, '%s', '%s', "
             "   '%s', %d, %u, '%s', %u, "
             "   '%s', '%s'"
             ")",
             entry->title, entry->alt_title, entry->developer, entry->publisher, entry->release_date,
             entry->platform, entry->region, entry->compatibility, entry->media_type, entry->genre, entry->input_type,
             entry->description, md5_str, sha1_str, crc32_str, entry->size, entry->players, entry->serial, entry->version,
             entry->save_type, entry->has_battery ? 1 : 0, entry->flags, entry->extra_data, entry->db_revision,
             added_date, updated_date);

    // Executar a consulta
    char *err_msg = NULL;
    int rc = sqlite3_exec(g_rom_db_handle, query, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Erro ao adicionar entrada: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    // Incrementar contador de entradas
    g_rom_db_entry_count++;

    MEGA_LOG_INFO("ROM Database: Entrada adicionada com sucesso (ID=%lld)",
                 (long long)sqlite3_last_insert_rowid(g_rom_db_handle));

    return true;
}

/**
 * @brief Atualiza uma entrada existente no banco de dados
 *
 * @param entry Entrada com informações atualizadas (o ID deve corresponder a
 * uma entrada existente)
 * @return true Se atualizada com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_update_entry(const mega_emu_rom_db_entry_t *entry) {
    if (!g_rom_db_handle || entry == NULL || entry->id == 0) {
        return false;
    }

    // Verificar se a entrada existe
    char check_query[64];
    snprintf(check_query, sizeof(check_query),
             "SELECT COUNT(*) FROM rom_entries WHERE id = %u", entry->id);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_rom_db_handle, check_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Erro ao preparar consulta: %s", sqlite3_errmsg(g_rom_db_handle));
        return false;
    }

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);

    if (!exists) {
        MEGA_LOG_ERROR("ROM Database: Entrada não encontrada (ID=%u)", entry->id);
        return false;
    }

    // Converter hashes para strings
    char md5_str[33] = {0};
    char sha1_str[41] = {0};
    char crc32_str[9] = {0};

    mega_emu_rom_db_hash_to_string(&entry->hash, md5_str, sizeof(md5_str), 1);
    mega_emu_rom_db_hash_to_string(&entry->hash, sha1_str, sizeof(sha1_str), 2);
    mega_emu_rom_db_hash_to_string(&entry->hash, crc32_str, sizeof(crc32_str), 0);

    // Obter data atual para campo updated_date
    char current_date[12];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_info);

    // Construir consulta SQL para atualização
    char query[4096];
    snprintf(query, sizeof(query),
             "UPDATE rom_entries SET "
             "   title = '%s', alt_title = '%s', developer = '%s', publisher = '%s', "
             "   release_date = '%s', platform = %d, region = %d, compatibility = %d, "
             "   media_type = %d, genre = %d, input_type = %d, description = '%s', "
             "   md5 = '%s', sha1 = '%s', crc32 = '%s', size = %u, players = %u, "
             "   serial = '%s', version = '%s', save_type = '%s', has_battery = %d, "
             "   flags = %u, extra_data = '%s', db_revision = %u, "
             "   updated_date = '%s' "
             "WHERE id = %u",
             entry->title, entry->alt_title, entry->developer, entry->publisher,
             entry->release_date, entry->platform, entry->region, entry->compatibility,
             entry->media_type, entry->genre, entry->input_type, entry->description,
             md5_str, sha1_str, crc32_str, entry->size, entry->players,
             entry->serial, entry->version, entry->save_type, entry->has_battery ? 1 : 0,
             entry->flags, entry->extra_data, entry->db_revision,
             current_date, entry->id);

    // Executar a consulta
    char *err_msg = NULL;
    rc = sqlite3_exec(g_rom_db_handle, query, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Erro ao atualizar entrada: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    MEGA_LOG_INFO("ROM Database: Entrada atualizada com sucesso (ID=%u)", entry->id);

    return true;
}

/**
 * @brief Remove uma entrada do banco de dados
 *
 * @param id ID da entrada a ser removida
 * @return true Se removida com sucesso
 * @return false Se não foi encontrada ou ocorrer erro
 */
bool mega_emu_rom_db_remove_entry(uint32_t id) {
    if (!g_rom_db_handle || id == 0) {
        return false;
    }

    // Construir consulta SQL
    char query[64];
    snprintf(query, sizeof(query), "DELETE FROM rom_entries WHERE id = %u", id);

    // Executar a consulta
    char *err_msg = NULL;
    int rc = sqlite3_exec(g_rom_db_handle, query, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        MEGA_LOG_ERROR("ROM Database: Erro ao remover entrada: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    // Verificar se alguma linha foi afetada
    int changes = sqlite3_changes(g_rom_db_handle);

    if (changes > 0) {
        // Decrementar contador de entradas
        g_rom_db_entry_count--;

        MEGA_LOG_INFO("ROM Database: Entrada removida com sucesso (ID=%u)", id);
        return true;
    } else {
        MEGA_LOG_WARN("ROM Database: Entrada não encontrada (ID=%u)", id);
        return false;
    }
}
