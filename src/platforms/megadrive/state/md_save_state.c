/**
 * @file md_save_state.c
 * @brief Implementação do sistema de save state para o Mega Drive/Genesis
 * @author Mega_Emu Team
 * @version 1.3.0
 * @date 2025-04-01
 */

#include "md_save_state.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../../../utils/string_utils.h"
#include "../../../utils/file_utils.h"
#include "../../../utils/crypto_utils.h"
#include "../../../core/delta_compression.h"
#include "../../../core/thumbnail_generator.h"
#include "../../../core/rewind_buffer.h"
#include "../../../core/core.h"
#include "../memory/memory.h"
#include "../memory/md_mapper.h"
#include "../cpu/m68k_adapter.h"
#include "../cpu/z80_adapter.h"
#include "../video/vdp.h"
#include "../audio/audio_system.h"
#include "../io/controller.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definir categoria de log para o sistema de save state do Mega Drive
#define LOG_CAT_MD_SAVE_STATE EMU_LOG_CAT_MEGADRIVE

// Macros de log específicas
#define MD_SAVE_STATE_LOG_ERROR(...) EMU_LOG_ERROR(LOG_CAT_MD_SAVE_STATE, __VA_ARGS__)
#define MD_SAVE_STATE_LOG_WARN(...) EMU_LOG_WARN(LOG_CAT_MD_SAVE_STATE, __VA_ARGS__)
#define MD_SAVE_STATE_LOG_INFO(...) EMU_LOG_INFO(LOG_CAT_MD_SAVE_STATE, __VA_ARGS__)
#define MD_SAVE_STATE_LOG_DEBUG(...) EMU_LOG_DEBUG(LOG_CAT_MD_SAVE_STATE, __VA_ARGS__)
#define MD_SAVE_STATE_LOG_TRACE(...) EMU_LOG_TRACE(LOG_CAT_MD_SAVE_STATE, __VA_ARGS__)

// Versão do formato de save state
#define MD_SAVE_STATE_VERSION 0x00010300 // 1.3.0

// Contadores globais
static uint32_t g_save_count = 0;
static uint32_t g_load_count = 0;
static uint32_t g_play_time_seconds = 0;
static time_t g_last_play_time_update = 0;

// Flag para indicar se o sistema está inicializado
static bool g_is_initialized = false;

/**
 * @brief Inicializa o sistema de save state do Mega Drive
 */
int32_t md_save_state_init(void)
{
    if (g_is_initialized)
    {
        MD_SAVE_STATE_LOG_WARN("Sistema de save state do Mega Drive já está inicializado");
        return SAVE_STATE_ERROR_NONE;
    }

    // Inicializar o sistema de compressão delta
    int32_t result = delta_compression_init();
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao inicializar sistema de compressão delta: %d", result);
        return result;
    }

    // Inicializar o buffer de rewind com configuração padrão
    result = rewind_buffer_init(100, 5); // 100 estados, 5 frames por snapshot
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao inicializar buffer de rewind: %d", result);
        delta_compression_shutdown();
        return result;
    }

    // Inicializar contadores
    g_save_count = 0;
    g_load_count = 0;
    g_play_time_seconds = 0;
    g_last_play_time_update = time(NULL);

    g_is_initialized = true;
    MD_SAVE_STATE_LOG_INFO("Sistema de save state do Mega Drive inicializado com sucesso");

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Finaliza o sistema de save state do Mega Drive
 */
void md_save_state_shutdown(void)
{
    if (!g_is_initialized)
    {
        return;
    }

    // Finalizar o buffer de rewind
    rewind_buffer_shutdown();

    // Finalizar o sistema de compressão delta
    delta_compression_shutdown();

    g_is_initialized = false;
    MD_SAVE_STATE_LOG_INFO("Sistema de save state do Mega Drive finalizado");
}

/**
 * @brief Atualiza o contador de tempo de jogo
 */
static void update_play_time(void)
{
    time_t current_time = time(NULL);
    g_play_time_seconds += (uint32_t)difftime(current_time, g_last_play_time_update);
    g_last_play_time_update = current_time;
}

/**
 * @brief Registra os componentes do Mega Drive no save state
 */
static int32_t register_components(save_state_t *state, emu_platform_t *platform)
{
    if (!state || !platform || !platform->platform_data)
    {
        return SAVE_STATE_ERROR_INVALID;
    }

    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;

    // Registrar versão do save state
    uint32_t version = MD_SAVE_STATE_VERSION;
    save_state_register_field(state, "md_save_state_version", &version, sizeof(uint32_t));

    // Registrar dados da plataforma
    save_state_register_field(state, "md_platform_data", data, sizeof(md_platform_data_t));

    // Registrar ROM
    if (data->cart_rom && data->cart_rom_size > 0)
    {
        // Para a ROM, registramos apenas um checksum, não os dados completos
        uint32_t rom_crc32 = calculate_crc32(data->cart_rom, data->cart_rom_size);
        save_state_register_field(state, "md_rom_crc32", &rom_crc32, sizeof(uint32_t));
    }

    // Registrar RAM
    if (data->ram)
    {
        save_state_register_field(state, "md_ram", data->ram, data->ram_size);
    }

    // Registrar cabeçalho da ROM
    save_state_register_field(state, "md_rom_header", &data->rom_header, sizeof(md_rom_header_t));

    // Registrar CPUs
    if (data->m68k_cpu)
    {
        md_m68k_adapter_register_save_state(data->m68k_cpu, state);
    }

    if (data->z80_cpu)
    {
        md_z80_adapter_register_save_state(data->z80_cpu, state);
    }

    // Registrar componentes adicionais (VDP, áudio, etc.)
    if (data->vdp)
    {
        md_vdp_register_save_state(data->vdp, state);
    }

    if (data->audio)
    {
        md_audio_register_save_state(data->audio, state);
    }

    // Registrar estado dos controles
    md_controller_register_save_state(state);

    // Registrar estado do mapper
    md_mapper_register_save_state(state);

    // Registrar metadados
    update_play_time();

    md_save_state_metadata_t metadata;
    memset(&metadata, 0, sizeof(md_save_state_metadata_t));

    // Preencher metadados com informações do jogo
    strncpy(metadata.game_title, data->rom_header.overseas_name, sizeof(metadata.game_title) - 1);
    strncpy(metadata.game_region, data->rom_header.region, sizeof(metadata.game_region) - 1);
    strncpy(metadata.game_serial, data->rom_header.serial_number, sizeof(metadata.game_serial) - 1);
    metadata.rom_crc32 = calculate_crc32(data->cart_rom, data->cart_rom_size);
    metadata.save_count = g_save_count;
    metadata.load_count = g_load_count;
    metadata.play_time_seconds = g_play_time_seconds;
    strncpy(metadata.emu_version, "1.3.0", sizeof(metadata.emu_version) - 1);
    metadata.timestamp = (uint64_t)time(NULL);

    save_state_register_field(state, "md_save_state_metadata", &metadata, sizeof(md_save_state_metadata_t));

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Cria um novo save state
 */
save_state_t *md_save_state_create(
    emu_platform_t *platform,
    const uint8_t *screenshot_data,
    uint32_t width,
    uint32_t height,
    uint32_t stride,
    bool with_thumbnail,
    const char *description,
    const char *tags)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return NULL;
    }

    if (!platform || !platform->platform_data)
    {
        MD_SAVE_STATE_LOG_ERROR("Plataforma inválida");
        return NULL;
    }

    // Criar um novo save state
    save_state_t *state = save_state_create("md_save_state");
    if (!state)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao criar save state");
        return NULL;
    }

    // Configurar o save state
    save_state_config_t config;
    save_state_get_config(state, &config);

    config.format_version = MD_SAVE_STATE_VERSION;
    config.platform_id = PLATFORM_MEGADRIVE;
    config.use_delta_compression = true;
    config.thumbnail_width = 160;
    config.thumbnail_height = 120;
    config.thumbnail_quality = 90;

    save_state_set_config(state, &config);

    // Registrar componentes
    int32_t result = register_components(state, platform);
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao registrar componentes: %d", result);
        save_state_destroy(state);
        return NULL;
    }

    // Incrementar contador de saves
    g_save_count++;

    // Atualizar metadados com descrição e tags
    if (description || tags)
    {
        md_save_state_metadata_t metadata;

        if (save_state_read_field(state, "md_save_state_metadata", &metadata, sizeof(md_save_state_metadata_t)) == SAVE_STATE_ERROR_NONE)
        {
            if (description)
            {
                strncpy(metadata.save_description, description, sizeof(metadata.save_description) - 1);
            }

            if (tags)
            {
                strncpy(metadata.user_tags, tags, sizeof(metadata.user_tags) - 1);
            }

            save_state_write_field(state, "md_save_state_metadata", &metadata, sizeof(md_save_state_metadata_t));
        }
    }

    // Gerar thumbnail se necessário
    if (with_thumbnail && screenshot_data)
    {
        result = save_state_generate_thumbnail(
            state,
            screenshot_data,
            width,
            height,
            stride,
            true,  // com tarja "Save"
            NULL); // texto padrão

        if (result != SAVE_STATE_ERROR_NONE)
        {
            MD_SAVE_STATE_LOG_WARN("Falha ao gerar thumbnail: %d", result);
            // Continuamos mesmo sem thumbnail
        }
    }

    MD_SAVE_STATE_LOG_INFO("Save state criado com sucesso");
    return state;
}

/**
 * @brief Salva um save state em um arquivo
 */
int32_t md_save_state_save(save_state_t *state, const char *filename)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!state || !filename)
    {
        MD_SAVE_STATE_LOG_ERROR("Parâmetros inválidos");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Salvar o save state
    int32_t result = save_state_save(state, filename);
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao salvar save state: %d", result);
        return result;
    }

    MD_SAVE_STATE_LOG_INFO("Save state salvo com sucesso: %s", filename);
    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Carrega um save state de um arquivo
 */
save_state_t *md_save_state_load(const char *filename, emu_platform_t *platform)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return NULL;
    }

    if (!filename || !platform)
    {
        MD_SAVE_STATE_LOG_ERROR("Parâmetros inválidos");
        return NULL;
    }

    // Carregar o save state
    save_state_t *state = save_state_load(filename);
    if (!state)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao carregar save state: %s", filename);
        return NULL;
    }

    // Verificar versão do save state
    save_state_config_t config;
    save_state_get_config(state, &config);

    if (config.platform_id != PLATFORM_MEGADRIVE)
    {
        MD_SAVE_STATE_LOG_ERROR("Save state não é do Mega Drive (platform_id: %d)", config.platform_id);
        save_state_destroy(state);
        return NULL;
    }

    // Verificar versão do formato
    uint32_t version;
    if (save_state_read_field(state, "md_save_state_version", &version, sizeof(uint32_t)) != SAVE_STATE_ERROR_NONE ||
        version > MD_SAVE_STATE_VERSION)
    {
        MD_SAVE_STATE_LOG_ERROR("Versão do save state incompatível: 0x%08X (atual: 0x%08X)",
                                version, MD_SAVE_STATE_VERSION);
        save_state_destroy(state);
        return NULL;
    }

    // Verificar compatibilidade da ROM (se uma ROM estiver carregada)
    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;
    if (data->cart_rom && data->cart_rom_size > 0)
    {
        uint32_t current_rom_crc32 = calculate_crc32(data->cart_rom, data->cart_rom_size);
        uint32_t save_rom_crc32;

        if (save_state_read_field(state, "md_rom_crc32", &save_rom_crc32, sizeof(uint32_t)) == SAVE_STATE_ERROR_NONE &&
            current_rom_crc32 != save_rom_crc32)
        {
            MD_SAVE_STATE_LOG_WARN("ROM do save state difere da ROM carregada!");
            // Continuamos, mas avisamos o usuário
        }
    }

    // Incrementar contador de loads
    g_load_count++;

    MD_SAVE_STATE_LOG_INFO("Save state carregado com sucesso: %s", filename);
    return state;
}

/**
 * @brief Aplica um save state à plataforma
 */
int32_t md_save_state_apply(save_state_t *state, emu_platform_t *platform)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!state || !platform || !platform->platform_data)
    {
        MD_SAVE_STATE_LOG_ERROR("Parâmetros inválidos");
        return SAVE_STATE_ERROR_INVALID;
    }

    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;

    // Pausar emulação
    bool was_running = data->is_running;
    data->is_running = false;

    // Aplicar dados da plataforma (exceto ponteiros)
    md_platform_data_t temp_data;
    if (save_state_read_field(state, "md_platform_data", &temp_data, sizeof(md_platform_data_t)) == SAVE_STATE_ERROR_NONE)
    {
        // Preservar ponteiros importantes
        void *cart_rom = data->cart_rom;
        void *ram = data->ram;
        void *m68k_cpu = data->m68k_cpu;
        void *z80_cpu = data->z80_cpu;
        void *vdp = data->vdp;
        void *audio = data->audio;
        void *memory = data->memory;

        // Copiar campos gerais
        memcpy(data, &temp_data, sizeof(md_platform_data_t));

        // Restaurar ponteiros
        data->cart_rom = cart_rom;
        data->ram = ram;
        data->m68k_cpu = m68k_cpu;
        data->z80_cpu = z80_cpu;
        data->vdp = vdp;
        data->audio = audio;
        data->memory = memory;
    }

    // Aplicar cabeçalho da ROM
    save_state_read_field(state, "md_rom_header", &data->rom_header, sizeof(md_rom_header_t));

    // Aplicar RAM
    if (data->ram)
    {
        save_state_read_field(state, "md_ram", data->ram, data->ram_size);
    }

    // Aplicar CPUs
    if (data->m68k_cpu)
    {
        md_m68k_adapter_restore_save_state(data->m68k_cpu, state);
    }

    if (data->z80_cpu)
    {
        md_z80_adapter_restore_save_state(data->z80_cpu, state);
    }

    // Aplicar componentes adicionais
    if (data->vdp)
    {
        md_vdp_restore_save_state(data->vdp, state);
    }

    if (data->audio)
    {
        md_audio_restore_save_state(data->audio, state);
    }

    // Aplicar estado dos controles
    md_controller_restore_save_state(state);

    // Aplicar estado do mapper
    md_mapper_restore_save_state(state);

    // Restaurar estado de execução
    data->is_running = was_running;

    MD_SAVE_STATE_LOG_INFO("Save state aplicado com sucesso");
    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Configura o sistema de rewind
 */
int32_t md_save_state_config_rewind(uint32_t capacity, uint32_t frames_per_snapshot)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    int32_t result = rewind_buffer_init(capacity, frames_per_snapshot);
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao configurar sistema de rewind: %d", result);
        return result;
    }

    MD_SAVE_STATE_LOG_INFO("Sistema de rewind configurado: %u estados, %u frames por snapshot",
                           capacity, frames_per_snapshot);
    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Captura um snapshot para o sistema de rewind
 */
int32_t md_save_state_capture_rewind(emu_platform_t *platform)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!platform)
    {
        MD_SAVE_STATE_LOG_ERROR("Plataforma inválida");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Criar save state temporário
    save_state_t *state = save_state_create("md_rewind");
    if (!state)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao criar save state para rewind");
        return SAVE_STATE_ERROR_MEMORY;
    }

    // Registrar componentes
    int32_t result = register_components(state, platform);
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao registrar componentes para rewind: %d", result);
        save_state_destroy(state);
        return result;
    }

    // Serializar o estado
    void *data;
    uint32_t size;
    result = save_state_serialize(state, &data, &size);
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao serializar save state para rewind: %d", result);
        save_state_destroy(state);
        return result;
    }

    // Adicionar ao buffer de rewind
    result = rewind_buffer_push(data, size);

    // Limpar memória
    free(data);
    save_state_destroy(state);

    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao adicionar save state ao buffer de rewind: %d", result);
        return result;
    }

    MD_SAVE_STATE_LOG_TRACE("Snapshot de rewind capturado com sucesso");
    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Aplica rewind de um estado
 */
int32_t md_save_state_rewind(emu_platform_t *platform)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!platform)
    {
        MD_SAVE_STATE_LOG_ERROR("Plataforma inválida");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Obter dados do estado anterior
    void *data;
    uint32_t size;
    int32_t result = rewind_buffer_pop(&data, &size);
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao obter estado anterior do buffer de rewind: %d", result);
        return result;
    }

    // Deserializar o estado
    save_state_t *state = save_state_deserialize(data, size);

    // Limpar memória
    free(data);

    if (!state)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao deserializar save state para rewind");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Aplicar o estado
    result = md_save_state_apply(state, platform);
    save_state_destroy(state);

    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao aplicar save state de rewind: %d", result);
        return result;
    }

    MD_SAVE_STATE_LOG_DEBUG("Rewind aplicado com sucesso");
    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Obtém metadados de um save state
 */
int32_t md_save_state_get_metadata(save_state_t *state, md_save_state_metadata_t *metadata)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!state || !metadata)
    {
        MD_SAVE_STATE_LOG_ERROR("Parâmetros inválidos");
        return SAVE_STATE_ERROR_INVALID;
    }

    int32_t result = save_state_read_field(state, "md_save_state_metadata", metadata, sizeof(md_save_state_metadata_t));
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao ler metadados do save state: %d", result);
        return result;
    }

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Define metadados para um save state
 */
int32_t md_save_state_set_metadata(save_state_t *state, const md_save_state_metadata_t *metadata)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!state || !metadata)
    {
        MD_SAVE_STATE_LOG_ERROR("Parâmetros inválidos");
        return SAVE_STATE_ERROR_INVALID;
    }

    int32_t result = save_state_write_field(state, "md_save_state_metadata", metadata, sizeof(md_save_state_metadata_t));
    if (result != SAVE_STATE_ERROR_NONE)
    {
        MD_SAVE_STATE_LOG_ERROR("Falha ao escrever metadados no save state: %d", result);
        return result;
    }

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Ativa/desativa a compressão delta para save states
 */
int32_t md_save_state_use_delta_compression(bool enable)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    MD_SAVE_STATE_LOG_INFO("%s compressão delta para save states",
                           enable ? "Ativando" : "Desativando");

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Configura o sistema de thumbnails para save states
 */
int32_t md_save_state_config_thumbnails(uint32_t width, uint32_t height, uint32_t quality)
{
    if (!g_is_initialized)
    {
        MD_SAVE_STATE_LOG_ERROR("Sistema de save state não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (width == 0 || height == 0 || quality > 100)
    {
        MD_SAVE_STATE_LOG_ERROR("Parâmetros inválidos para configuração de thumbnails");
        return SAVE_STATE_ERROR_INVALID;
    }

    MD_SAVE_STATE_LOG_INFO("Configurando thumbnails: %ux%u, qualidade %u%%",
                           width, height, quality);

    return SAVE_STATE_ERROR_NONE;
}
