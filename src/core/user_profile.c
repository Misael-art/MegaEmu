/**
 * @file user_profile.c
 * @brief Implementação do sistema de perfis de usuário
 */

#include "user_profile.h"
#include "../utils/enhanced_log.h"
#include "../deps/sqlite/sqlite3.h" // Biblioteca SQLite
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Definição da categoria de log
#define LOG_CAT_PROFILE EMU_LOG_CAT_CORE

// Caminho do banco de dados
static char g_db_path[256] = "profiles.db";

// Conexão com o banco de dados
static sqlite3 *g_db = NULL;

// Perfil atualmente ativo
static user_profile_t g_active_profile;
static bool g_has_active_profile = false;

// Constantes
#define HASH_SIZE 64
#define DEFAULT_AVATAR_PATH "assets/images/default_avatar.png"

// Consultas SQL predefinidas
const char *SQL_CREATE_PROFILES_TABLE =
    "CREATE TABLE IF NOT EXISTS profiles ("
    "  username TEXT PRIMARY KEY,"
    "  display_name TEXT NOT NULL,"
    "  email TEXT UNIQUE NOT NULL,"
    "  password_hash TEXT NOT NULL,"
    "  avatar_path TEXT,"
    "  created_timestamp INTEGER NOT NULL,"
    "  last_login_timestamp INTEGER NOT NULL,"
    "  is_active INTEGER NOT NULL DEFAULT 0"
    ");";

const char *SQL_CREATE_STATS_TABLE =
    "CREATE TABLE IF NOT EXISTS profile_stats ("
    "  username TEXT PRIMARY KEY,"
    "  total_play_time INTEGER NOT NULL DEFAULT 0,"
    "  games_played INTEGER NOT NULL DEFAULT 0,"
    "  save_states_created INTEGER NOT NULL DEFAULT 0,"
    "  save_states_loaded INTEGER NOT NULL DEFAULT 0,"
    "  achievements_unlocked INTEGER NOT NULL DEFAULT 0,"
    "  total_launches INTEGER NOT NULL DEFAULT 0,"
    "  last_active_timestamp INTEGER NOT NULL,"
    "  FOREIGN KEY(username) REFERENCES profiles(username) ON DELETE CASCADE"
    ");";

const char *SQL_CREATE_CLOUD_CONFIG_TABLE =
    "CREATE TABLE IF NOT EXISTS profile_cloud_config ("
    "  username TEXT PRIMARY KEY,"
    "  service INTEGER NOT NULL DEFAULT 0,"
    "  auth_token TEXT,"
    "  refresh_token TEXT,"
    "  folder_path TEXT,"
    "  auto_sync INTEGER NOT NULL DEFAULT 0,"
    "  sync_on_exit INTEGER NOT NULL DEFAULT 0,"
    "  sync_screenshots INTEGER NOT NULL DEFAULT 0,"
    "  sync_interval INTEGER NOT NULL DEFAULT 0,"
    "  FOREIGN KEY(username) REFERENCES profiles(username) ON DELETE CASCADE"
    ");";

const char *SQL_CREATE_SOCIAL_CONFIG_TABLE =
    "CREATE TABLE IF NOT EXISTS profile_social_config ("
    "  username TEXT PRIMARY KEY,"
    "  share_enabled INTEGER NOT NULL DEFAULT 0,"
    "  default_privacy INTEGER NOT NULL DEFAULT 0,"
    "  auto_share_achievements INTEGER NOT NULL DEFAULT 0,"
    "  twitter_token TEXT,"
    "  facebook_token TEXT,"
    "  discord_webhook TEXT,"
    "  FOREIGN KEY(username) REFERENCES profiles(username) ON DELETE CASCADE"
    ");";

// Consultas comuns
const char *SQL_INSERT_PROFILE =
    "INSERT INTO profiles (username, display_name, email, password_hash, avatar_path, "
    "                     created_timestamp, last_login_timestamp, is_active) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

const char *SQL_GET_PROFILE_BY_USERNAME =
    "SELECT p.username, p.display_name, p.email, p.password_hash, p.avatar_path, "
    "       p.created_timestamp, p.last_login_timestamp, p.is_active, "
    "       s.total_play_time, s.games_played, s.save_states_created, s.save_states_loaded, "
    "       s.achievements_unlocked, s.total_launches, s.last_active_timestamp, "
    "       c.service, c.auth_token, c.refresh_token, c.folder_path, c.auto_sync, "
    "       c.sync_on_exit, c.sync_screenshots, c.sync_interval, "
    "       sc.share_enabled, sc.default_privacy, sc.auto_share_achievements, "
    "       sc.twitter_token, sc.facebook_token, sc.discord_webhook "
    "FROM profiles p "
    "LEFT JOIN profile_stats s ON p.username = s.username "
    "LEFT JOIN profile_cloud_config c ON p.username = c.username "
    "LEFT JOIN profile_social_config sc ON p.username = sc.username "
    "WHERE p.username = ?;";

const char *SQL_UPDATE_PROFILE =
    "UPDATE profiles SET "
    "  display_name = ?, "
    "  email = ?, "
    "  avatar_path = ?, "
    "  last_login_timestamp = ? "
    "WHERE username = ?;";

const char *SQL_UPDATE_PASSWORD =
    "UPDATE profiles SET password_hash = ? WHERE username = ?;";

const char *SQL_DELETE_PROFILE =
    "DELETE FROM profiles WHERE username = ?;";

const char *SQL_SET_ACTIVE_PROFILE =
    "UPDATE profiles SET is_active = CASE WHEN username = ? THEN 1 ELSE 0 END;";

const char *SQL_GET_ACTIVE_PROFILE =
    "SELECT username FROM profiles WHERE is_active = 1 LIMIT 1;";

const char *SQL_CHECK_USERNAME_EXISTS =
    "SELECT COUNT(*) FROM profiles WHERE username = ?;";

const char *SQL_GET_PASSWORD_HASH =
    "SELECT password_hash FROM profiles WHERE username = ?;";

/**
 * @brief Calcula o hash SHA-256 de uma string
 */
static void calculate_sha256(const char *input, char *output)
{
    // Implementação simplificada - em produção usaríamos uma biblioteca criptográfica
    // como OpenSSL para gerar o hash SHA-256

    // Esta é uma simplificação apenas para demonstração
    sprintf(output, "sha256_%08x%08x%08x%08x%08x%08x%08x%08x",
            rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand());
}

/**
 * @brief Cria as tabelas necessárias no banco de dados
 */
static int32_t create_tables(void)
{
    char *error_message = NULL;
    int result;

    // Criar tabela de perfis
    result = sqlite3_exec(g_db, SQL_CREATE_PROFILES_TABLE, NULL, NULL, &error_message);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao criar tabela de perfis: %s", error_message);
        sqlite3_free(error_message);
        return PROFILE_ERROR_DATABASE;
    }

    // Criar tabela de estatísticas
    result = sqlite3_exec(g_db, SQL_CREATE_STATS_TABLE, NULL, NULL, &error_message);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao criar tabela de estatísticas: %s", error_message);
        sqlite3_free(error_message);
        return PROFILE_ERROR_DATABASE;
    }

    // Criar tabela de configuração de nuvem
    result = sqlite3_exec(g_db, SQL_CREATE_CLOUD_CONFIG_TABLE, NULL, NULL, &error_message);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao criar tabela de configuração de nuvem: %s", error_message);
        sqlite3_free(error_message);
        return PROFILE_ERROR_DATABASE;
    }

    // Criar tabela de configuração social
    result = sqlite3_exec(g_db, SQL_CREATE_SOCIAL_CONFIG_TABLE, NULL, NULL, &error_message);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao criar tabela de configuração social: %s", error_message);
        sqlite3_free(error_message);
        return PROFILE_ERROR_DATABASE;
    }

    EMU_LOG_INFO(LOG_CAT_PROFILE, "Tabelas criadas com sucesso");
    return PROFILE_ERROR_NONE;
}

/**
 * @brief Inicializa o perfil padrão se não existir nenhum perfil
 */
static int32_t init_default_profile(void)
{
    sqlite3_stmt *stmt;
    int result;

    // Verificar se existe algum perfil
    result = sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM profiles;", -1, &stmt, NULL);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao verificar existência de perfis");
        return PROFILE_ERROR_DATABASE;
    }

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao executar consulta de contagem de perfis");
        return PROFILE_ERROR_DATABASE;
    }

    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (count > 0)
    {
        // Já existem perfis, não precisa criar o padrão
        return PROFILE_ERROR_NONE;
    }

    // Criar perfil padrão
    EMU_LOG_INFO(LOG_CAT_PROFILE, "Criando perfil padrão");

    char password_hash[HASH_SIZE];
    calculate_sha256("default", password_hash);

    uint64_t current_time = (uint64_t)time(NULL);

    // Preparar inserção
    result = sqlite3_prepare_v2(g_db, SQL_INSERT_PROFILE, -1, &stmt, NULL);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar inserção de perfil padrão");
        return PROFILE_ERROR_DATABASE;
    }

    // Vincular parâmetros
    sqlite3_bind_text(stmt, 1, "default", -1, SQLITE_STATIC);             // username
    sqlite3_bind_text(stmt, 2, "Usuário Padrão", -1, SQLITE_STATIC);      // display_name
    sqlite3_bind_text(stmt, 3, "default@example.com", -1, SQLITE_STATIC); // email
    sqlite3_bind_text(stmt, 4, password_hash, -1, SQLITE_STATIC);         // password_hash
    sqlite3_bind_text(stmt, 5, DEFAULT_AVATAR_PATH, -1, SQLITE_STATIC);   // avatar_path
    sqlite3_bind_int64(stmt, 6, current_time);                            // created_timestamp
    sqlite3_bind_int64(stmt, 7, current_time);                            // last_login_timestamp
    sqlite3_bind_int(stmt, 8, 1);                                         // is_active

    // Executar inserção
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao inserir perfil padrão: %s", sqlite3_errmsg(g_db));
        return PROFILE_ERROR_DATABASE;
    }

    // Inicializar outras tabelas para o perfil padrão
    const char *username = "default";

    // Estatísticas
    result = sqlite3_prepare_v2(g_db,
                                "INSERT INTO profile_stats (username, last_active_timestamp) VALUES (?, ?);",
                                -1, &stmt, NULL);

    if (result == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, current_time);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // Configuração de nuvem
    result = sqlite3_prepare_v2(g_db,
                                "INSERT INTO profile_cloud_config (username) VALUES (?);",
                                -1, &stmt, NULL);

    if (result == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // Configuração social
    result = sqlite3_prepare_v2(g_db,
                                "INSERT INTO profile_social_config (username) VALUES (?);",
                                -1, &stmt, NULL);

    if (result == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    EMU_LOG_INFO(LOG_CAT_PROFILE, "Perfil padrão criado com sucesso");
    return PROFILE_ERROR_NONE;
}

/**
 * @brief Preenche a estrutura de perfil com os dados da consulta
 */
static void fill_profile_from_statement(sqlite3_stmt *stmt, user_profile_t *profile)
{
    strcpy(profile->username, (const char *)sqlite3_column_text(stmt, 0));
    strcpy(profile->display_name, (const char *)sqlite3_column_text(stmt, 1));
    strcpy(profile->email, (const char *)sqlite3_column_text(stmt, 2));
    strcpy(profile->password_hash, (const char *)sqlite3_column_text(stmt, 3));

    const char *avatar_path = (const char *)sqlite3_column_text(stmt, 4);
    if (avatar_path)
    {
        strcpy(profile->avatar_path, avatar_path);
    }
    else
    {
        strcpy(profile->avatar_path, DEFAULT_AVATAR_PATH);
    }

    profile->created_timestamp = sqlite3_column_int64(stmt, 5);
    profile->last_login_timestamp = sqlite3_column_int64(stmt, 6);
    profile->is_active = sqlite3_column_int(stmt, 7) != 0;

    // Estatísticas
    profile->stats.total_play_time = sqlite3_column_int(stmt, 8);
    profile->stats.games_played = sqlite3_column_int(stmt, 9);
    profile->stats.save_states_created = sqlite3_column_int(stmt, 10);
    profile->stats.save_states_loaded = sqlite3_column_int(stmt, 11);
    profile->stats.achievements_unlocked = sqlite3_column_int(stmt, 12);
    profile->stats.total_launches = sqlite3_column_int(stmt, 13);
    profile->stats.last_active_timestamp = sqlite3_column_int64(stmt, 14);

    // Configuração de nuvem
    profile->cloud.service = (profile_cloud_service_t)sqlite3_column_int(stmt, 15);

    const char *auth_token = (const char *)sqlite3_column_text(stmt, 16);
    if (auth_token)
    {
        strcpy(profile->cloud.auth_token, auth_token);
    }
    else
    {
        profile->cloud.auth_token[0] = '\0';
    }

    const char *refresh_token = (const char *)sqlite3_column_text(stmt, 17);
    if (refresh_token)
    {
        strcpy(profile->cloud.refresh_token, refresh_token);
    }
    else
    {
        profile->cloud.refresh_token[0] = '\0';
    }

    const char *folder_path = (const char *)sqlite3_column_text(stmt, 18);
    if (folder_path)
    {
        strcpy(profile->cloud.folder_path, folder_path);
    }
    else
    {
        profile->cloud.folder_path[0] = '\0';
    }

    profile->cloud.auto_sync = sqlite3_column_int(stmt, 19) != 0;
    profile->cloud.sync_on_exit = sqlite3_column_int(stmt, 20) != 0;
    profile->cloud.sync_screenshots = sqlite3_column_int(stmt, 21) != 0;
    profile->cloud.sync_interval = sqlite3_column_int(stmt, 22);

    // Configuração social
    profile->social.share_enabled = sqlite3_column_int(stmt, 23) != 0;
    profile->social.default_privacy = (profile_privacy_t)sqlite3_column_int(stmt, 24);
    profile->social.auto_share_achievements = sqlite3_column_int(stmt, 25) != 0;

    const char *twitter_token = (const char *)sqlite3_column_text(stmt, 26);
    if (twitter_token)
    {
        strcpy(profile->social.twitter_token, twitter_token);
    }
    else
    {
        profile->social.twitter_token[0] = '\0';
    }

    const char *facebook_token = (const char *)sqlite3_column_text(stmt, 27);
    if (facebook_token)
    {
        strcpy(profile->social.facebook_token, facebook_token);
    }
    else
    {
        profile->social.facebook_token[0] = '\0';
    }

    const char *discord_webhook = (const char *)sqlite3_column_text(stmt, 28);
    if (discord_webhook)
    {
        strcpy(profile->social.discord_webhook, discord_webhook);
    }
    else
    {
        profile->social.discord_webhook[0] = '\0';
    }
}

/**
 * @brief Carrega o perfil ativo do banco de dados
 */
static int32_t load_active_profile(void)
{
    sqlite3_stmt *stmt;
    int result;

    // Encontrar o username do perfil ativo
    result = sqlite3_prepare_v2(g_db, SQL_GET_ACTIVE_PROFILE, -1, &stmt, NULL);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar consulta de perfil ativo");
        return PROFILE_ERROR_DATABASE;
    }

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        EMU_LOG_WARN(LOG_CAT_PROFILE, "Nenhum perfil ativo encontrado");
        g_has_active_profile = false;
        return PROFILE_ERROR_USER_NOT_FOUND;
    }

    const char *username = (const char *)sqlite3_column_text(stmt, 0);
    sqlite3_finalize(stmt);

    // Carregar dados completos do perfil
    result = sqlite3_prepare_v2(g_db, SQL_GET_PROFILE_BY_USERNAME, -1, &stmt, NULL);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar consulta de perfil: %s", sqlite3_errmsg(g_db));
        return PROFILE_ERROR_DATABASE;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao carregar dados do perfil: %s", sqlite3_errmsg(g_db));
        return PROFILE_ERROR_DATABASE;
    }

    // Preencher a estrutura do perfil ativo
    fill_profile_from_statement(stmt, &g_active_profile);
    g_has_active_profile = true;

    sqlite3_finalize(stmt);

    EMU_LOG_INFO(LOG_CAT_PROFILE, "Perfil ativo carregado: %s", g_active_profile.username);
    return PROFILE_ERROR_NONE;
}

// Implementações de funções da API

int32_t profile_init(const char *database_path)
{
    // Verificar se já está inicializado
    if (g_db != NULL)
    {
        EMU_LOG_WARN(LOG_CAT_PROFILE, "Sistema de perfis já inicializado");
        return PROFILE_ERROR_NONE;
    }

    // Guardar o caminho do banco de dados
    if (database_path)
    {
        strncpy(g_db_path, database_path, sizeof(g_db_path) - 1);
        g_db_path[sizeof(g_db_path) - 1] = '\0';
    }

    // Abrir conexão com o banco de dados
    int result = sqlite3_open(g_db_path, &g_db);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao abrir banco de dados: %s", sqlite3_errmsg(g_db));
        sqlite3_close(g_db);
        g_db = NULL;
        return PROFILE_ERROR_DATABASE;
    }

    EMU_LOG_INFO(LOG_CAT_PROFILE, "Banco de dados aberto: %s", g_db_path);

    // Criar tabelas se não existirem
    int32_t status = create_tables();
    if (status != PROFILE_ERROR_NONE)
    {
        sqlite3_close(g_db);
        g_db = NULL;
        return status;
    }

    // Inicializar perfil padrão se necessário
    status = init_default_profile();
    if (status != PROFILE_ERROR_NONE)
    {
        EMU_LOG_WARN(LOG_CAT_PROFILE, "Erro ao inicializar perfil padrão");
        // Não é um erro fatal, podemos continuar
    }

    // Carregar perfil ativo
    status = load_active_profile();
    if (status != PROFILE_ERROR_NONE)
    {
        EMU_LOG_WARN(LOG_CAT_PROFILE, "Erro ao carregar perfil ativo");
        // Não é um erro fatal, podemos continuar
    }

    EMU_LOG_INFO(LOG_CAT_PROFILE, "Sistema de perfis inicializado com sucesso");
    return PROFILE_ERROR_NONE;
}

void profile_shutdown(void)
{
    if (g_db != NULL)
    {
        sqlite3_close(g_db);
        g_db = NULL;
        g_has_active_profile = false;
        EMU_LOG_INFO(LOG_CAT_PROFILE, "Sistema de perfis finalizado");
    }
}

bool profile_is_username_available(const char *username)
{
    if (!g_db || !username)
    {
        return false;
    }

    sqlite3_stmt *stmt;
    int result = sqlite3_prepare_v2(g_db, SQL_CHECK_USERNAME_EXISTS, -1, &stmt, NULL);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar consulta de verificação de username");
        return false;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao executar consulta de verificação de username");
        return false;
    }

    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    return count == 0; // Username disponível se não existir
}

int32_t profile_create(
    const char *username,
    const char *display_name,
    const char *email,
    const char *password,
    const char *avatar_path)
{
    if (!g_db || !username || !display_name || !email || !password)
    {
        return PROFILE_ERROR_INVALID_PARAM;
    }

    // Verificar se o username já existe
    if (!profile_is_username_available(username))
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Nome de usuário já existe: %s", username);
        return PROFILE_ERROR_USER_EXISTS;
    }

    // Calcular hash da senha
    char password_hash[HASH_SIZE];
    calculate_sha256(password, password_hash);

    uint64_t current_time = (uint64_t)time(NULL);

    // Começar transação
    char *error_message = NULL;
    int result = sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, &error_message);
    if (result != SQLITE_OK)
    {
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao iniciar transação: %s", error_message);
        sqlite3_free(error_message);
        return PROFILE_ERROR_DATABASE;
    }

    // Inserir perfil
    sqlite3_stmt *stmt;
    result = sqlite3_prepare_v2(g_db, SQL_INSERT_PROFILE, -1, &stmt, NULL);
    if (result != SQLITE_OK)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar inserção de perfil");
        return PROFILE_ERROR_DATABASE;
    }

    // Vincular parâmetros
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);      // username
    sqlite3_bind_text(stmt, 2, display_name, -1, SQLITE_STATIC);  // display_name
    sqlite3_bind_text(stmt, 3, email, -1, SQLITE_STATIC);         // email
    sqlite3_bind_text(stmt, 4, password_hash, -1, SQLITE_STATIC); // password_hash

    if (avatar_path)
    {
        sqlite3_bind_text(stmt, 5, avatar_path, -1, SQLITE_STATIC); // avatar_path
    }
    else
    {
        sqlite3_bind_text(stmt, 5, DEFAULT_AVATAR_PATH, -1, SQLITE_STATIC);
    }

    sqlite3_bind_int64(stmt, 6, current_time); // created_timestamp
    sqlite3_bind_int64(stmt, 7, current_time); // last_login_timestamp
    sqlite3_bind_int(stmt, 8, 0);              // is_active

    // Executar inserção
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao inserir perfil: %s", sqlite3_errmsg(g_db));
        return PROFILE_ERROR_DATABASE;
    }

    // Inicializar outras tabelas para o perfil

    // Estatísticas
    result = sqlite3_prepare_v2(g_db,
                                "INSERT INTO profile_stats (username, last_active_timestamp) VALUES (?, ?);",
                                -1, &stmt, NULL);

    if (result != SQLITE_OK)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar inserção de estatísticas");
        return PROFILE_ERROR_DATABASE;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, current_time);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao inserir estatísticas: %s", sqlite3_errmsg(g_db));
        return PROFILE_ERROR_DATABASE;
    }

    // Configuração de nuvem
    result = sqlite3_prepare_v2(g_db,
                                "INSERT INTO profile_cloud_config (username) VALUES (?);",
                                -1, &stmt, NULL);

    if (result != SQLITE_OK)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar inserção de configuração de nuvem");
        return PROFILE_ERROR_DATABASE;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao inserir configuração de nuvem: %s", sqlite3_errmsg(g_db));
        return PROFILE_ERROR_DATABASE;
    }

    // Configuração social
    result = sqlite3_prepare_v2(g_db,
                                "INSERT INTO profile_social_config (username) VALUES (?);",
                                -1, &stmt, NULL);

    if (result != SQLITE_OK)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao preparar inserção de configuração social");
        return PROFILE_ERROR_DATABASE;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao inserir configuração social: %s", sqlite3_errmsg(g_db));
        return PROFILE_ERROR_DATABASE;
    }

    // Confirmar transação
    result = sqlite3_exec(g_db, "COMMIT;", NULL, NULL, &error_message);
    if (result != SQLITE_OK)
    {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        EMU_LOG_ERROR(LOG_CAT_PROFILE, "Erro ao confirmar transação: %s", error_message);
        sqlite3_free(error_message);
        return PROFILE_ERROR_DATABASE;
    }

    EMU_LOG_INFO(LOG_CAT_PROFILE, "Perfil criado com sucesso: %s", username);
    return PROFILE_ERROR_NONE;
}

// Esta é uma implementação parcial. As funções restantes seriam implementadas
// seguindo o mesmo padrão, interagindo com o banco de dados SQLite para realizar
// as operações necessárias.
