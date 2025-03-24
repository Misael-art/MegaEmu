/**
 * @file save_state_cloud_tests.c
 * @brief Testes unitários para o sistema de integração com nuvem de save states
 * @version 1.0
 * @date 2025-03-18
 *
 * Este arquivo contém testes unitários para validar a implementação
 * de integração com serviços de nuvem para o sistema de save states.
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "core/save_state.h"
#include "core/save_state_cloud.h"
#include "utils/file_utils.h"
#include "utils/mock_http.h"

/* Diretório temporário para arquivos de teste */
#define TEST_DIR "./test_temp/cloud"

/* Tamanho do buffer de teste */
#define TEST_BUFFER_SIZE 8192

/* Dados para testes */
static uint8_t test_buffer[TEST_BUFFER_SIZE];

/* Contexto de save state para testes */
static emu_save_state_t *test_state = NULL;

/* Configuração de nuvem para testes */
static emu_cloud_config_t test_config;

/* Servidor mock para testes */
static mock_http_server mock_server;

/* Callbacks personalizados para testes */
static bool test_upload_callback(const char *local_path, const char *remote_path, void *user_data)
{
    char remote_file_path[512];
    snprintf(remote_file_path, sizeof(remote_file_path), "%s/remote%s", TEST_DIR, remote_path);

    char dir_path[512];
    strncpy(dir_path, remote_file_path, sizeof(dir_path));

    char *last_slash = strrchr(dir_path, '/');
    if (last_slash)
    {
        *last_slash = '\0';
        create_directory_recursive(dir_path);
    }

    return copy_file(local_path, remote_file_path);
}

static bool test_download_callback(const char *remote_path, const char *local_path, void *user_data)
{
    char remote_file_path[512];
    snprintf(remote_file_path, sizeof(remote_file_path), "%s/remote%s", TEST_DIR, remote_path);

    if (!file_exists(remote_file_path))
    {
        return false;
    }

    char dir_path[512];
    strncpy(dir_path, local_path, sizeof(dir_path));

    char *last_slash = strrchr(dir_path, '/');
    if (last_slash)
    {
        *last_slash = '\0';
        create_directory_recursive(dir_path);
    }

    return copy_file(remote_file_path, local_path);
}

static bool test_list_callback(const char *remote_path, void *user_data, char *results, size_t max_results)
{
    char remote_dir_path[512];
    snprintf(remote_dir_path, sizeof(remote_dir_path), "%s/remote%s", TEST_DIR, remote_path);

    if (!directory_exists(remote_dir_path))
    {
        return false;
    }

    char **files = list_files(remote_dir_path);
    if (!files)
    {
        return false;
    }

    size_t offset = 0;
    for (int i = 0; files[i] != NULL && offset < max_results - 1; i++)
    {
        size_t len = strlen(files[i]);
        if (offset + len + 1 < max_results)
        {
            memcpy(results + offset, files[i], len);
            offset += len;
            results[offset++] = '\n';
        }
    }

    if (offset < max_results)
    {
        results[offset] = '\0';
    }

    for (int i = 0; files[i] != NULL; i++)
    {
        free(files[i]);
    }
    free(files);

    return true;
}

static bool test_timestamp_callback(const char *remote_path, void *user_data, uint64_t *timestamp)
{
    char remote_file_path[512];
    snprintf(remote_file_path, sizeof(remote_file_path), "%s/remote%s", TEST_DIR, remote_path);

    if (!file_exists(remote_file_path))
    {
        return false;
    }

    *timestamp = get_file_modification_time(remote_file_path);
    return true;
}

void setUp(void)
{
    /* Inicializa o subsistema de nuvem */
    TEST_ASSERT_TRUE(emu_cloud_init());

    /* Cria diretório temporário para testes */
    TEST_ASSERT_TRUE(create_directory(TEST_DIR));

    /* Preenche buffer de teste com dados pseudoaleatórios */
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        test_buffer[i] = (uint8_t)((i * 7) & 0xFF);
    }

    /* Inicializa o contexto de save state */
    test_state = emu_save_state_init(EMU_PLATFORM_TEST, test_buffer, 1024);
    TEST_ASSERT_NOT_NULL(test_state);

    /* Registra algumas regiões para teste */
    TEST_ASSERT_TRUE(emu_save_state_register_region(test_state, 1, "TestRegion1", test_buffer, 1024));
    TEST_ASSERT_TRUE(emu_save_state_register_region(test_state, 2, "TestRegion2", test_buffer + 1024, 1024));

    /* Inicializa servidor mock */
    TEST_ASSERT_TRUE(mock_http_server_init(&mock_server, 8080));
}

void tearDown(void)
{
    /* Finaliza o servidor mock */
    mock_http_server_shutdown(&mock_server);

    /* Libera o contexto de save state */
    if (test_state)
    {
        emu_save_state_shutdown(test_state);
        test_state = NULL;
    }

    /* Finaliza o subsistema de nuvem */
    emu_cloud_shutdown();

    /* Remove arquivos temporários */
    remove_directory_recursive(TEST_DIR);
}

void test_cloud_config(void)
{
    /* Inicializa configuração para provedor personalizado */
    memset(&test_config, 0, sizeof(test_config));
    test_config.provider = EMU_CLOUD_CUSTOM;
    test_config.custom_upload = test_upload_callback;
    test_config.custom_download = test_download_callback;
    test_config.custom_list = test_list_callback;
    test_config.custom_timestamp = test_timestamp_callback;

    /* Configura o provedor de nuvem */
    TEST_ASSERT_TRUE(emu_cloud_configure(&test_config));

    /* Verifica se a configuração foi aplicada */
    emu_cloud_config_t current_config;
    TEST_ASSERT_TRUE(emu_cloud_get_config(&current_config));
    TEST_ASSERT_EQUAL(EMU_CLOUD_CUSTOM, current_config.provider);
    TEST_ASSERT_NOT_NULL(current_config.custom_upload);
    TEST_ASSERT_NOT_NULL(current_config.custom_download);
    TEST_ASSERT_NOT_NULL(current_config.custom_list);
    TEST_ASSERT_NOT_NULL(current_config.custom_timestamp);
}

void test_save_and_sync(void)
{
    const char *test_file = TEST_DIR "/test_save.sav";
    const char *cloud_path = "/saves/test_save.sav";

    /* Salva o estado localmente */
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, test_file));

    /* Sincroniza com a nuvem */
    TEST_ASSERT_TRUE(emu_cloud_sync_file(test_file, cloud_path));

    /* Verifica se o arquivo existe no armazenamento remoto simulado */
    char remote_file_path[512];
    snprintf(remote_file_path, sizeof(remote_file_path), "%s/remote%s", TEST_DIR, cloud_path);
    TEST_ASSERT_TRUE(file_exists(remote_file_path));

    /* Verifica se os arquivos são idênticos */
    TEST_ASSERT_TRUE(files_are_equal(test_file, remote_file_path));
}

void test_cloud_download(void)
{
    const char *cloud_path = "/saves/test_save.sav";
    const char *local_path = TEST_DIR "/downloaded_save.sav";

    /* Primeiro faz upload de um arquivo para teste */
    const char *test_file = TEST_DIR "/test_save.sav";
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, test_file));
    TEST_ASSERT_TRUE(emu_cloud_sync_file(test_file, cloud_path));

    /* Tenta baixar o arquivo */
    TEST_ASSERT_TRUE(emu_cloud_download_file(cloud_path, local_path));

    /* Verifica se o arquivo foi baixado corretamente */
    TEST_ASSERT_TRUE(file_exists(local_path));
    TEST_ASSERT_TRUE(files_are_equal(test_file, local_path));
}

void test_cloud_list(void)
{
    const char *test_dir = "/saves";
    char list_buffer[1024] = {0};

    /* Cria alguns arquivos de teste */
    const char *test_files[] = {
        "/saves/save1.sav",
        "/saves/save2.sav",
        "/saves/subdir/save3.sav"};

    for (int i = 0; i < 3; i++)
    {
        char local_path[512];
        snprintf(local_path, sizeof(local_path), "%s/test%d.sav", TEST_DIR, i);
        TEST_ASSERT_TRUE(emu_save_state_save(test_state, local_path));
        TEST_ASSERT_TRUE(emu_cloud_sync_file(local_path, test_files[i]));
    }

    /* Lista os arquivos */
    TEST_ASSERT_TRUE(emu_cloud_list_files(test_dir, list_buffer, sizeof(list_buffer)));

    /* Verifica se todos os arquivos estão listados */
    TEST_ASSERT_NOT_NULL(strstr(list_buffer, "save1.sav"));
    TEST_ASSERT_NOT_NULL(strstr(list_buffer, "save2.sav"));
    TEST_ASSERT_NOT_NULL(strstr(list_buffer, "subdir/save3.sav"));
}

void test_check_updates(void)
{
    const char *cloud_path = "/saves/test_save.sav";
    const char *local_path = TEST_DIR "/test_save.sav";

    /* Cria um save state inicial */
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, local_path));
    TEST_ASSERT_TRUE(emu_cloud_sync_file(local_path, cloud_path));

    /* Modifica o arquivo remoto */
    char remote_path[512];
    snprintf(remote_path, sizeof(remote_path), "%s/remote%s", TEST_DIR, cloud_path);
    FILE *f = fopen(remote_path, "wb");
    TEST_ASSERT_NOT_NULL(f);
    uint8_t modified_data[1024] = {0xFF};
    TEST_ASSERT_EQUAL(1024, fwrite(modified_data, 1, 1024, f));
    fclose(f);

    /* Verifica se há atualizações */
    bool has_update = false;
    TEST_ASSERT_TRUE(emu_cloud_check_update(cloud_path, local_path, &has_update));
    TEST_ASSERT_TRUE(has_update);
}

void test_conflict_resolution(void)
{
    const char *cloud_path = "/saves/test_save.sav";
    const char *local_path = TEST_DIR "/test_save.sav";
    const char *backup_path = TEST_DIR "/backup_save.sav";

    /* Cria um save state inicial */
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, local_path));
    TEST_ASSERT_TRUE(emu_cloud_sync_file(local_path, cloud_path));

    /* Modifica o arquivo local */
    TEST_ASSERT_TRUE(copy_file(local_path, backup_path));
    FILE *f = fopen(local_path, "wb");
    TEST_ASSERT_NOT_NULL(f);
    uint8_t local_data[1024] = {0xAA};
    TEST_ASSERT_EQUAL(1024, fwrite(local_data, 1, 1024, f));
    fclose(f);

    /* Modifica o arquivo remoto */
    char remote_path[512];
    snprintf(remote_path, sizeof(remote_path), "%s/remote%s", TEST_DIR, cloud_path);
    f = fopen(remote_path, "wb");
    TEST_ASSERT_NOT_NULL(f);
    uint8_t remote_data[1024] = {0xBB};
    TEST_ASSERT_EQUAL(1024, fwrite(remote_data, 1, 1024, f));
    fclose(f);

    /* Tenta sincronizar - deve detectar conflito */
    emu_cloud_conflict_t conflict;
    TEST_ASSERT_TRUE(emu_cloud_detect_conflict(cloud_path, local_path, &conflict));
    TEST_ASSERT_EQUAL(EMU_CLOUD_CONFLICT_DIVERGED, conflict.type);

    /* Resolve o conflito mantendo a versão local */
    conflict.resolution = EMU_CLOUD_RESOLVE_KEEP_LOCAL;
    TEST_ASSERT_TRUE(emu_cloud_resolve_conflict(&conflict));

    /* Verifica se a versão local foi mantida */
    uint8_t verify_data[1024];
    f = fopen(remote_path, "rb");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL(1024, fread(verify_data, 1, 1024, f));
    fclose(f);
    TEST_ASSERT_EQUAL_MEMORY(local_data, verify_data, 1024);
}

void test_oauth_authentication(void)
{
    /* Configura credenciais OAuth */
    emu_cloud_oauth_config_t oauth_config = {
        .client_id = "test_client_id",
        .client_secret = "test_client_secret",
        .redirect_uri = "http://localhost:8080/oauth/callback"};

    /* Configura respostas mock para o servidor */
    mock_http_server_add_response(&mock_server, "/oauth/token",
                                  "{"
                                  "\"access_token\":\"test_access_token\","
                                  "\"refresh_token\":\"test_refresh_token\","
                                  "\"expires_in\":3600"
                                  "}");

    /* Inicia o fluxo OAuth */
    TEST_ASSERT_TRUE(emu_cloud_start_oauth(&oauth_config));

    /* Simula callback do OAuth */
    TEST_ASSERT_TRUE(emu_cloud_handle_oauth_callback("test_code"));

    /* Verifica se os tokens foram obtidos */
    emu_cloud_tokens_t tokens;
    TEST_ASSERT_TRUE(emu_cloud_get_tokens(&tokens));
    TEST_ASSERT_EQUAL_STRING("test_access_token", tokens.access_token);
    TEST_ASSERT_EQUAL_STRING("test_refresh_token", tokens.refresh_token);
    TEST_ASSERT_EQUAL(3600, tokens.expires_in);
}

void test_async_operations(void)
{
    const char *cloud_path = "/saves/test_save.sav";
    const char *local_path = TEST_DIR "/test_save.sav";

    /* Cria um save state para teste */
    TEST_ASSERT_TRUE(emu_save_state_save(test_state, local_path));

    /* Inicia upload assíncrono */
    emu_cloud_async_t async;
    TEST_ASSERT_TRUE(emu_cloud_async_upload(local_path, cloud_path, &async));

    /* Verifica status até completar */
    emu_cloud_status_t status;
    do
    {
        TEST_ASSERT_TRUE(emu_cloud_async_status(&async, &status));
    } while (status.state == EMU_CLOUD_STATE_IN_PROGRESS);

    TEST_ASSERT_EQUAL(EMU_CLOUD_STATE_COMPLETED, status.state);
    TEST_ASSERT_EQUAL(100, status.progress);
}

void test_auto_backup(void)
{
    /* Configura backup automático */
    emu_cloud_backup_config_t backup_config = {
        .enabled = true,
        .interval = 300, // 5 minutos
        .max_backups = 3,
        .backup_dir = "/saves/backups"};

    TEST_ASSERT_TRUE(emu_cloud_configure_backup(&backup_config));

    /* Verifica se a configuração foi aplicada */
    emu_cloud_backup_config_t current_config;
    TEST_ASSERT_TRUE(emu_cloud_get_backup_config(&current_config));
    TEST_ASSERT_TRUE(current_config.enabled);
    TEST_ASSERT_EQUAL(300, current_config.interval);
    TEST_ASSERT_EQUAL(3, current_config.max_backups);
    TEST_ASSERT_EQUAL_STRING("/saves/backups", current_config.backup_dir);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_cloud_config);
    RUN_TEST(test_save_and_sync);
    RUN_TEST(test_cloud_download);
    RUN_TEST(test_cloud_list);
    RUN_TEST(test_check_updates);
    RUN_TEST(test_conflict_resolution);
    RUN_TEST(test_oauth_authentication);
    RUN_TEST(test_async_operations);
    RUN_TEST(test_auto_backup);

    return UNITY_END();
}
