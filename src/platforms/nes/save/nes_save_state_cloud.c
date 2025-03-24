/**
 * @file nes_save_state_cloud.c
 * @brief Implementação de integração com nuvem para save states do NES
 * @version 1.0
 * @date 2025-04-30
 *
 * Esta implementação se integra ao sistema unificado de nuvem
 * para fornecer sincronização e backup dos save states do NES.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nes_save_state_cloud.h"
#include "nes_save_state.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/error_handling.h"
#include "../../../utils/log_categories.h"
#include "../../../utils/file_utils.h"
#include "../../../utils/string_utils.h"
#include "../../../core/save_state_cloud.h"
#include "../nes.h"

/* Definição de categoria de log específica */
#define EMU_LOG_CAT_SAVE_CLOUD EMU_LOG_CAT_PLATFORM_NETWORK

/* Macros de log */
#define CLOUD_LOG_INFO(msg, ...) EMU_LOG_INFO(EMU_LOG_CAT_SAVE_CLOUD, msg, ##__VA_ARGS__)
#define CLOUD_LOG_ERROR(msg, ...) EMU_LOG_ERROR(EMU_LOG_CAT_SAVE_CLOUD, msg, ##__VA_ARGS__)
#define CLOUD_LOG_DEBUG(msg, ...) EMU_LOG_DEBUG(EMU_LOG_CAT_SAVE_CLOUD, msg, ##__VA_ARGS__)
#define CLOUD_LOG_WARN(msg, ...) EMU_LOG_WARN(EMU_LOG_CAT_SAVE_CLOUD, msg, ##__VA_ARGS__)

/* Estrutura privada para dados de nuvem específicos do NES */
typedef struct
{
    bool cloud_enabled;
    emu_cloud_provider_t provider;
    char remote_folder[256];
    char game_id[64];
    time_t last_sync_time;
    emu_cloud_conflict_strategy_t conflict_strategy;
    bool auto_backup_enabled;
    uint32_t backup_interval_minutes;
} nes_cloud_data_t;

/* Dados de nuvem globais para NES */
static nes_cloud_data_t g_nes_cloud_data = {0};

/* Função interna para obter o ID do jogo atual */
static bool get_current_game_id(char *buffer, size_t buffer_size)
{
    extern nes_state_t g_nes_state;

    if (!buffer || buffer_size == 0)
    {
        return false;
    }

    if (!g_nes_state.cartridge)
    {
        return false;
    }

    /* Tenta obter o título do jogo */
    if (g_nes_state.cartridge->game_title && g_nes_state.cartridge->game_title[0] != '\0')
    {
        /* Sanitiza o nome do jogo para uso em caminhos */
        sanitize_filename(g_nes_state.cartridge->game_title, buffer, buffer_size);
        return true;
    }

    /* Backup: usa o hash MD5 da ROM */
    char hash[33];
    if (g_nes_state.cartridge->rom_hash && strlen(g_nes_state.cartridge->rom_hash) == 32)
    {
        strncpy(hash, g_nes_state.cartridge->rom_hash, 32);
        hash[32] = '\0';
        snprintf(buffer, buffer_size, "nes_game_%s", hash);
        return true;
    }

    return false;
}

bool nes_save_state_enable_cloud(emu_save_state_t *state,
                                 emu_cloud_provider_t provider,
                                 const char *auth_token,
                                 bool auto_sync)
{
    if (!state || !auth_token)
    {
        CLOUD_LOG_ERROR("Parâmetros inválidos para ativação de nuvem");
        return false;
    }

    /* Configura a integração com nuvem */
    emu_cloud_config_t config;
    memset(&config, 0, sizeof(config));

    config.provider = provider;
    strncpy(config.auth_token, auth_token, sizeof(config.auth_token) - 1);

    /* Define a pasta remota com base no game ID */
    char game_id[64] = {0};
    if (get_current_game_id(game_id, sizeof(game_id)))
    {
        /* Armazena o ID do jogo para futuras operações */
        strncpy(g_nes_cloud_data.game_id, game_id, sizeof(g_nes_cloud_data.game_id) - 1);

        /* Define o caminho remoto */
        snprintf(config.folder_path, sizeof(config.folder_path),
                 "/MegaEmu/NES/SaveStates/%s", game_id);

        /* Armazena o caminho remoto */
        strncpy(g_nes_cloud_data.remote_folder, config.folder_path,
                sizeof(g_nes_cloud_data.remote_folder) - 1);
    }
    else
    {
        strcpy(config.folder_path, "/MegaEmu/NES/SaveStates");
        strcpy(g_nes_cloud_data.remote_folder, config.folder_path);
    }

    config.auto_sync = auto_sync;
    config.sync_interval = auto_sync ? 300 : 0; /* 5 minutos se auto_sync for true */
    config.conflict_resolution = EMU_CLOUD_CONFLICT_ASK;

    /* Armazena as configurações nos dados de nuvem do NES */
    g_nes_cloud_data.cloud_enabled = true;
    g_nes_cloud_data.provider = provider;
    g_nes_cloud_data.conflict_strategy = EMU_CLOUD_CONFLICT_ASK;
    g_nes_cloud_data.last_sync_time = time(NULL);

    /* Aplica a configuração ao contexto de save state */
    if (!emu_save_state_cloud_configure(state, &config))
    {
        CLOUD_LOG_ERROR("Falha ao configurar integração com nuvem");
        g_nes_cloud_data.cloud_enabled = false;
        return false;
    }

    CLOUD_LOG_INFO("Integração com nuvem ativada para save states do NES usando provedor %d", provider);
    return true;
}

bool nes_save_state_disable_cloud(emu_save_state_t *state)
{
    if (!state)
    {
        CLOUD_LOG_ERROR("Contexto de estado inválido");
        return false;
    }

    /* Configura para desativar a integração com nuvem */
    emu_cloud_config_t config;
    memset(&config, 0, sizeof(config));
    config.provider = EMU_CLOUD_NONE; // Sem provedor

    /* Aplica a configuração */
    if (!emu_save_state_cloud_configure(state, &config))
    {
        CLOUD_LOG_ERROR("Falha ao desativar integração com nuvem");
        return false;
    }

    /* Atualiza os dados de nuvem do NES */
    g_nes_cloud_data.cloud_enabled = false;

    CLOUD_LOG_INFO("Integração com nuvem desativada para save states do NES");
    return true;
}

bool nes_save_state_sync_with_cloud(emu_save_state_t *state)
{
    if (!state)
    {
        CLOUD_LOG_ERROR("Contexto de estado inválido");
        return false;
    }

    if (!g_nes_cloud_data.cloud_enabled)
    {
        CLOUD_LOG_ERROR("Integração com nuvem não está ativada");
        return false;
    }

    /* Verifica se o estado atual é válido para sincronização */
    if (!emu_save_state_is_valid(state))
    {
        CLOUD_LOG_ERROR("Estado atual não é válido para sincronização");
        return false;
    }

    /* Executa a sincronização com a nuvem */
    emu_cloud_sync_options_t options;
    memset(&options, 0, sizeof(options));
    options.conflict_strategy = g_nes_cloud_data.conflict_strategy;
    options.timeout_seconds = 30; // Timeout de 30 segundos

    bool result = emu_save_state_cloud_sync(state, &options);

    if (result)
    {
        /* Atualiza o timestamp da última sincronização */
        g_nes_cloud_data.last_sync_time = time(NULL);
        CLOUD_LOG_INFO("Sincronização com nuvem realizada com sucesso");
    }
    else
    {
        CLOUD_LOG_ERROR("Falha ao sincronizar com a nuvem");
    }

    return result;
}

emu_cloud_file_info_t *nes_save_state_list_cloud_saves(emu_save_state_t *state, uint32_t *count)
{
    if (!state || !count)
    {
        CLOUD_LOG_ERROR("Parâmetros inválidos");
        return NULL;
    }

    *count = 0;

    if (!g_nes_cloud_data.cloud_enabled)
    {
        CLOUD_LOG_ERROR("Integração com nuvem não está ativada");
        return NULL;
    }

    /* Lista os arquivos na pasta remota */
    emu_cloud_file_info_t *files = NULL;
    emu_cloud_list_options_t options;
    memset(&options, 0, sizeof(options));
    options.timeout_seconds = 30;
    options.file_pattern = "*.state"; // Apenas arquivos de save state

    *count = emu_save_state_cloud_list_files(state, g_nes_cloud_data.remote_folder, &files, &options);

    if (*count == 0)
    {
        CLOUD_LOG_INFO("Nenhum save state encontrado na nuvem");
        return NULL;
    }

    CLOUD_LOG_INFO("Encontrados %u save states na nuvem", *count);
    return files;
}

bool nes_save_state_download_from_cloud(emu_save_state_t *state,
                                        const char *cloud_id,
                                        const char *local_path)
{
    if (!state || !cloud_id || !local_path)
    {
        CLOUD_LOG_ERROR("Parâmetros inválidos");
        return false;
    }

    if (!g_nes_cloud_data.cloud_enabled)
    {
        CLOUD_LOG_ERROR("Integração com nuvem não está ativada");
        return false;
    }

    /* Configura as opções de download */
    emu_cloud_transfer_options_t options;
    memset(&options, 0, sizeof(options));
    options.timeout_seconds = 60; // 1 minuto de timeout
    options.overwrite_existing = true;

    /* Executa o download */
    bool result = emu_save_state_cloud_download_file(state, cloud_id, local_path, &options);

    if (result)
    {
        CLOUD_LOG_INFO("Save state baixado com sucesso da nuvem para %s", local_path);
    }
    else
    {
        CLOUD_LOG_ERROR("Falha ao baixar save state da nuvem");
    }

    return result;
}

bool nes_save_state_upload_to_cloud(emu_save_state_t *state,
                                    const char *local_path,
                                    const char *description)
{
    if (!state || !local_path)
    {
        CLOUD_LOG_ERROR("Parâmetros inválidos");
        return false;
    }

    if (!g_nes_cloud_data.cloud_enabled)
    {
        CLOUD_LOG_ERROR("Integração com nuvem não está ativada");
        return false;
    }

    /* Verifica se o arquivo existe localmente */
    if (!file_exists(local_path))
    {
        CLOUD_LOG_ERROR("Arquivo local não existe: %s", local_path);
        return false;
    }

    /* Extrai o nome do arquivo do caminho */
    char filename[256];
    extract_filename(local_path, filename, sizeof(filename));

    /* Constrói o caminho remoto */
    char remote_path[512];
    snprintf(remote_path, sizeof(remote_path), "%s/%s",
             g_nes_cloud_data.remote_folder, filename);

    /* Configura metadados adicionais */
    emu_cloud_metadata_t metadata;
    memset(&metadata, 0, sizeof(metadata));
    strncpy(metadata.title, "NES Save State", sizeof(metadata.title) - 1);

    if (description)
    {
        strncpy(metadata.description, description, sizeof(metadata.description) - 1);
    }
    else
    {
        strncpy(metadata.description, "Save state do emulador NES", sizeof(metadata.description) - 1);
    }

    /* Adiciona informações de jogo se disponíveis */
    if (g_nes_cloud_data.game_id[0] != '\0')
    {
        strncpy(metadata.tags, g_nes_cloud_data.game_id, sizeof(metadata.tags) - 1);
    }

    /* Configura as opções de upload */
    emu_cloud_transfer_options_t options;
    memset(&options, 0, sizeof(options));
    options.timeout_seconds = 60; // 1 minuto de timeout
    options.metadata = &metadata;
    options.overwrite_existing = true;

    /* Executa o upload */
    bool result = emu_save_state_cloud_upload_file(state, local_path, remote_path, &options);

    if (result)
    {
        CLOUD_LOG_INFO("Save state enviado com sucesso para a nuvem: %s", remote_path);
    }
    else
    {
        CLOUD_LOG_ERROR("Falha ao enviar save state para a nuvem");
    }

    return result;
}

bool nes_save_state_set_conflict_strategy(emu_save_state_t *state, emu_cloud_conflict_strategy_t strategy)
{
    if (!state)
    {
        CLOUD_LOG_ERROR("Contexto de estado inválido");
        return false;
    }

    if (!g_nes_cloud_data.cloud_enabled)
    {
        CLOUD_LOG_ERROR("Integração com nuvem não está ativada");
        return false;
    }

    /* Atualiza a estratégia de resolução de conflitos */
    g_nes_cloud_data.conflict_strategy = strategy;

    /* Atualiza a configuração no contexto */
    emu_cloud_config_t config;
    if (!emu_save_state_cloud_get_config(state, &config))
    {
        CLOUD_LOG_ERROR("Falha ao obter configuração de nuvem atual");
        return false;
    }

    config.conflict_resolution = strategy;

    if (!emu_save_state_cloud_configure(state, &config))
    {
        CLOUD_LOG_ERROR("Falha ao atualizar configuração de nuvem");
        return false;
    }

    CLOUD_LOG_INFO("Estratégia de resolução de conflitos atualizada para %d", strategy);
    return true;
}

bool nes_save_state_configure_auto_backup(emu_save_state_t *state, bool enable, uint32_t interval_minutes)
{
    if (!state)
    {
        CLOUD_LOG_ERROR("Contexto de estado inválido");
        return false;
    }

    if (!g_nes_cloud_data.cloud_enabled && enable)
    {
        CLOUD_LOG_ERROR("Não é possível habilitar backup automático sem integração com nuvem");
        return false;
    }

    /* Atualiza as configurações de backup automático */
    g_nes_cloud_data.auto_backup_enabled = enable;

    if (interval_minutes > 0)
    {
        g_nes_cloud_data.backup_interval_minutes = interval_minutes;
    }
    else
    {
        g_nes_cloud_data.backup_interval_minutes = 30; // Valor padrão: 30 minutos
    }

    /* Configura o backup automático no contexto */
    emu_cloud_config_t config;
    if (!emu_save_state_cloud_get_config(state, &config))
    {
        CLOUD_LOG_ERROR("Falha ao obter configuração de nuvem atual");
        return false;
    }

    config.auto_backup = enable;
    config.backup_interval = enable ? g_nes_cloud_data.backup_interval_minutes * 60 : 0;

    if (!emu_save_state_cloud_configure(state, &config))
    {
        CLOUD_LOG_ERROR("Falha ao atualizar configuração de nuvem");
        return false;
    }

    if (enable)
    {
        CLOUD_LOG_INFO("Backup automático habilitado com intervalo de %u minutos",
                       g_nes_cloud_data.backup_interval_minutes);
    }
    else
    {
        CLOUD_LOG_INFO("Backup automático desabilitado");
    }

    return true;
}
