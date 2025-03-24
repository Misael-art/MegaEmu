/**
 * @file nes_save_state_cloud_tests.c
 * @brief Testes unitários para integração com nuvem nos save states do NES
 * @version 1.0
 * @date 2025-04-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../test_framework/unity.h"
#include "../platforms/nes/save/nes_save_state.h"
#include "../platforms/nes/save/nes_save_state_cloud.h"
#include "../core/save_state.h"
#include "../core/save_state_cloud.h"
#include "../utils/file_utils.h"
#include "../utils/mock_http.h"

/* Diretório temporário para testes */
#define TEST_DIR "./test_tmp/nes_cloud_tests"
#define TEST_BUFFER_SIZE 4096
#define TEST_PORT 8980

/* Dados de teste */
static uint8_t test_data[TEST_BUFFER_SIZE];
static char test_save_path[256];
static char test_remote_path[256];

/* Configuração de teste */
static emu_save_state_t test_state;
static mock_http_server mock_server;
static const char *test_token = "TesteTokenAuth123!@#";

/* Preparar mocks de HTTP */
static void setup_mock_responses(void)
{
    /* Inicializar o servidor mock */
    mock_http_init(&mock_server, TEST_PORT);

    /* Configurar resposta para login */
    mock_http_response_t auth_response = {
        .endpoint = "/auth/token",
        .response_data = "{\"status\":\"success\",\"token\":\"valid_session_token\"}",
        .status_code = 200};
    mock_http_add_response(&mock_server, &auth_response);

    /* Configurar resposta para listagem de arquivos */
    mock_http_response_t list_response = {
        .endpoint = "/api/files/list",
        .response_data = "[{\"name\":\"save1.state\",\"id\":\"file1\",\"size\":1024,\"modified\":\"2025-04-30T12:00:00Z\"},"
                         "{\"name\":\"save2.state\",\"id\":\"file2\",\"size\":2048,\"modified\":\"2025-04-30T13:00:00Z\"}]",
        .status_code = 200};
    mock_http_add_response(&mock_server, &list_response);

    /* Configurar resposta para upload */
    mock_http_response_t upload_response = {
        .endpoint = "/api/files/upload",
        .response_data = "{\"status\":\"success\",\"file_id\":\"uploaded_file_id\"}",
        .status_code = 200};
    mock_http_add_response(&mock_server, &upload_response);

    /* Configurar resposta para download */
    mock_http_response_t download_response = {
        .endpoint = "/api/files/download",
        .response_data = "MOCK_FILE_CONTENT",
        .status_code = 200};
    mock_http_add_response(&mock_server, &download_response);

    /* Configurar resposta para sincronização */
    mock_http_response_t sync_response = {
        .endpoint = "/api/sync",
        .response_data = "{\"status\":\"success\",\"synced_files\":2}",
        .status_code = 200};
    mock_http_add_response(&mock_server, &sync_response);

    /* Iniciar o servidor */
    mock_http_start(&mock_server);
}

/* Configuração/inicialização para testes */
void setUp(void)
{
    /* Criar diretório de teste se não existir */
    create_directory(TEST_DIR);

    /* Inicializar dados de teste com padrão pseudo-aleatório */
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        test_data[i] = (uint8_t)((i * 23) % 256);
    }

    /* Configurar caminhos de teste */
    snprintf(test_save_path, sizeof(test_save_path), "%s/test_cloud_save.state", TEST_DIR);
    snprintf(test_remote_path, sizeof(test_remote_path), "/MegaEmu/NES/SaveStates/test_cloud_save.state");

    /* Inicializar contexto de save state */
    memset(&test_state, 0, sizeof(test_state));
    emu_save_state_init(&test_state);

    /* Configurar endpoint de API para apontar para nosso mock */
    extern void emu_cloud_set_api_endpoint(const char *url);
    char mock_url[64];
    snprintf(mock_url, sizeof(mock_url), "http://localhost:%d", TEST_PORT);
    emu_cloud_set_api_endpoint(mock_url);

    /* Configurar respostas mock */
    setup_mock_responses();
}

/* Limpeza após testes */
void tearDown(void)
{
    /* Limpar recursos */
    emu_save_state_cleanup(&test_state);

    /* Parar servidor mock */
    mock_http_stop(&mock_server);

    /* Remover arquivos de teste */
    remove_file(test_save_path);
}

/* Testar habilitação e desabilitação de cloud */
void test_nes_save_state_cloud_enable_disable(void)
{
    /* Verificar estado inicial */
    TEST_ASSERT_FALSE(test_state.cloud_enabled);

    /* Habilitar cloud */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, EMU_CLOUD_PROVIDER_MEGA_CLOUD, test_token, false));
    TEST_ASSERT_TRUE(test_state.cloud_enabled);

    /* Verificar que a requisição de autenticação foi feita */
    TEST_ASSERT_TRUE(mock_http_request_received(&mock_server, "/auth/token"));

    /* Desabilitar cloud */
    TEST_ASSERT_TRUE(nes_save_state_disable_cloud(&test_state));
    TEST_ASSERT_FALSE(test_state.cloud_enabled);
}

/* Testar listagem de saves da nuvem */
void test_nes_save_state_cloud_list(void)
{
    uint32_t count = 0;

    /* Habilitar cloud primeiro */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, EMU_CLOUD_PROVIDER_MEGA_CLOUD, test_token, false));

    /* Listar saves da nuvem */
    emu_cloud_file_info_t *files = nes_save_state_list_cloud_saves(&test_state, &count);

    /* Verificar resultados */
    TEST_ASSERT_NOT_NULL(files);
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL_STRING("save1.state", files[0].filename);
    TEST_ASSERT_EQUAL_STRING("file1", files[0].id);
    TEST_ASSERT_EQUAL_STRING("save2.state", files[1].filename);

    /* Verificar que a requisição foi feita */
    TEST_ASSERT_TRUE(mock_http_request_received(&mock_server, "/api/files/list"));

    /* Liberar memória */
    free(files);
}

/* Testar upload para a nuvem */
void test_nes_save_state_cloud_upload(void)
{
    /* Criar arquivo local para teste */
    FILE *f = fopen(test_save_path, "wb");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Falha ao criar arquivo de teste");
    fwrite(test_data, 1, TEST_BUFFER_SIZE, f);
    fclose(f);

    /* Habilitar cloud */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, EMU_CLOUD_PROVIDER_MEGA_CLOUD, test_token, false));

    /* Fazer upload */
    TEST_ASSERT_TRUE(nes_save_state_upload_to_cloud(&test_state, test_save_path, "Teste de upload"));

    /* Verificar que a requisição foi feita */
    TEST_ASSERT_TRUE(mock_http_request_received(&mock_server, "/api/files/upload"));
}

/* Testar download da nuvem */
void test_nes_save_state_cloud_download(void)
{
    /* Habilitar cloud */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, EMU_CLOUD_PROVIDER_MEGA_CLOUD, test_token, false));

    /* Fazer download */
    TEST_ASSERT_TRUE(nes_save_state_download_from_cloud(&test_state, "file1", test_save_path));

    /* Verificar que a requisição foi feita */
    TEST_ASSERT_TRUE(mock_http_request_received(&mock_server, "/api/files/download"));

    /* Verificar que o arquivo foi criado */
    TEST_ASSERT_TRUE(file_exists(test_save_path));
}

/* Testar sincronização */
void test_nes_save_state_cloud_sync(void)
{
    /* Habilitar cloud */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, EMU_CLOUD_PROVIDER_MEGA_CLOUD, test_token, false));

    /* Sincronizar */
    TEST_ASSERT_TRUE(nes_save_state_sync_with_cloud(&test_state));

    /* Verificar que a requisição foi feita */
    TEST_ASSERT_TRUE(mock_http_request_received(&mock_server, "/api/sync"));
}

/* Testar configuração de estratégia de conflito */
void test_nes_save_state_cloud_conflict_strategy(void)
{
    /* Habilitar cloud */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, EMU_CLOUD_PROVIDER_MEGA_CLOUD, test_token, false));

    /* Configurar estratégia de conflito */
    TEST_ASSERT_TRUE(nes_save_state_set_conflict_strategy(&test_state, EMU_CLOUD_CONFLICT_LOCAL_WINS));

    /* Verificar que a estratégia foi aplicada - isso testaria a API interna */
    emu_cloud_config_t config;
    TEST_ASSERT_TRUE(emu_save_state_cloud_get_config(&test_state, &config));
    TEST_ASSERT_EQUAL(EMU_CLOUD_CONFLICT_LOCAL_WINS, config.conflict_resolution);
}

/* Testar configuração de backup automático */
void test_nes_save_state_auto_backup(void)
{
    /* Habilitar cloud */
    TEST_ASSERT_TRUE(nes_save_state_enable_cloud(&test_state, EMU_CLOUD_PROVIDER_MEGA_CLOUD, test_token, false));

    /* Configurar backup automático */
    TEST_ASSERT_TRUE(nes_save_state_configure_auto_backup(&test_state, true, 15));

    /* Verificar que a configuração foi aplicada */
    emu_cloud_config_t config;
    TEST_ASSERT_TRUE(emu_save_state_cloud_get_config(&test_state, &config));
    TEST_ASSERT_TRUE(config.auto_backup);
    TEST_ASSERT_EQUAL(15 * 60, config.backup_interval);

    /* Desabilitar backup automático */
    TEST_ASSERT_TRUE(nes_save_state_configure_auto_backup(&test_state, false, 0));

    /* Verificar que foi desabilitado */
    TEST_ASSERT_TRUE(emu_save_state_cloud_get_config(&test_state, &config));
    TEST_ASSERT_FALSE(config.auto_backup);
}

/* Testar falha quando não há cloud habilitada */
void test_nes_save_state_cloud_fail_when_disabled(void)
{
    uint32_t count = 0;

    /* Tentar operações com cloud desabilitada */
    TEST_ASSERT_FALSE(nes_save_state_sync_with_cloud(&test_state));
    TEST_ASSERT_NULL(nes_save_state_list_cloud_saves(&test_state, &count));
    TEST_ASSERT_EQUAL(0, count);
    TEST_ASSERT_FALSE(nes_save_state_download_from_cloud(&test_state, "file1", test_save_path));
    TEST_ASSERT_FALSE(nes_save_state_upload_to_cloud(&test_state, test_save_path, "Teste"));
    TEST_ASSERT_FALSE(nes_save_state_set_conflict_strategy(&test_state, EMU_CLOUD_CONFLICT_LOCAL_WINS));
    TEST_ASSERT_FALSE(nes_save_state_configure_auto_backup(&test_state, true, 15));
}

/* Função principal dos testes */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_nes_save_state_cloud_enable_disable);
    RUN_TEST(test_nes_save_state_cloud_list);
    RUN_TEST(test_nes_save_state_cloud_upload);
    RUN_TEST(test_nes_save_state_cloud_download);
    RUN_TEST(test_nes_save_state_cloud_sync);
    RUN_TEST(test_nes_save_state_cloud_conflict_strategy);
    RUN_TEST(test_nes_save_state_cloud_auto_backup);
    RUN_TEST(test_nes_save_state_cloud_fail_when_disabled);

    return UNITY_END();
}
