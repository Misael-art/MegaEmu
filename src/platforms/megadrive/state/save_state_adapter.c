/**
 * @file save_state_adapter.c
 * @brief Adaptador do sistema de save states para o Mega Drive
 * @version 1.0
 * @date 2024-03-25
 */

#include "save_state_adapter.h"
#include "../../../core/state/emu_save_state.h"
#include "../../../utils/enhanced_log.h"
#include "../megadrive.h"
#include "../cpu/m68k_adapter.h"
#include "../cpu/z80_adapter.h"
#include "../video/vdp_adapter.h"
#include "../audio/audio_adapter.h"
#include "../memory/memory_adapter.h"
#include "../timer/timer_adapter.h"

#include <stdlib.h>
#include <string.h>

// Identificadores de regiões do save state
#define MD_STATE_REGION_CPU_M68K       "md_m68k"
#define MD_STATE_REGION_CPU_Z80        "md_z80"
#define MD_STATE_REGION_VDP            "md_vdp"
#define MD_STATE_REGION_VDP_VRAM       "md_vdp_vram"
#define MD_STATE_REGION_VDP_CRAM       "md_vdp_cram"
#define MD_STATE_REGION_VDP_VSRAM      "md_vdp_vsram"
#define MD_STATE_REGION_AUDIO_YM2612   "md_ym2612"
#define MD_STATE_REGION_AUDIO_PSG      "md_psg"
#define MD_STATE_REGION_MEMORY         "md_memory"
#define MD_STATE_REGION_TIMER          "md_timer"
#define MD_STATE_REGION_SYSTEM         "md_system"

// Metadados do save state
#define MD_STATE_META_GAME_TITLE       "game_title"
#define MD_STATE_META_GAME_REGION      "game_region"
#define MD_STATE_META_TIMESTAMP        "timestamp"
#define MD_STATE_META_EMULATOR_VERSION "emulator_version"
#define MD_STATE_META_SAVE_COMMENT     "comment"
#define MD_STATE_META_SAVE_VERSION     "save_version"

// Callbacks de preparação para salvar/carregar regiões específicas
static void m68k_pre_save(void *data, size_t size, void *user_data);
static void m68k_post_load(void *data, size_t size, void *user_data);
static void z80_pre_save(void *data, size_t size, void *user_data);
static void z80_post_load(void *data, size_t size, void *user_data);
static void vdp_pre_save(void *data, size_t size, void *user_data);
static void vdp_post_load(void *data, size_t size, void *user_data);
static void audio_pre_save(void *data, size_t size, void *user_data);
static void audio_post_load(void *data, size_t size, void *user_data);
static void memory_pre_save(void *data, size_t size, void *user_data);
static void memory_post_load(void *data, size_t size, void *user_data);

// Callbacks gerais do sistema de save state
static void pre_save_callback(emu_save_state_t *state, void *user_data);
static void post_save_callback(emu_save_state_t *state, const char *filename, void *user_data);
static void pre_load_callback(emu_save_state_t *state, const char *filename, void *user_data);
static void post_load_callback(emu_save_state_t *state, void *user_data);
static void error_callback(emu_save_state_t *state, int32_t error_code, const char *message, void *user_data);

/**
 * @brief Inicializa o sistema de save state para o Mega Drive
 * @param md Contexto do Mega Drive
 * @return Contexto de save state ou NULL em caso de erro
 */
emu_save_state_t *md_save_state_init(megadrive_t *md) {
    if (!md || !md->rom_data || md->rom_size == 0) {
        emu_log_error("Não foi possível inicializar save state: dados da ROM inválidos");
        return NULL;
    }

    // Configurar callbacks do sistema de save state
    emu_save_callbacks_t callbacks = {
        .pre_save = pre_save_callback,
        .post_save = post_save_callback,
        .pre_load = pre_load_callback,
        .post_load = post_load_callback,
        .error = error_callback,
        .user_data = md
    };

    // Inicializar contexto de save state
    emu_save_state_t *state = emu_save_state_init_ex(
        EMU_PLATFORM_MEGA_DRIVE,
        md->rom_data,
        md->rom_size,
        &callbacks
    );

    if (!state) {
        emu_log_error("Falha ao criar contexto de save state");
        return NULL;
    }

    // Registrar regiões (componentes) do emulador
    emu_log_info("Registrando regiões do sistema de save state para o Mega Drive");

    // M68K
    emu_save_state_register_region_ex(
        state,
        MD_STATE_REGION_CPU_M68K,
        m68k_adapter_get_context(md->m68k),
        sizeof(m68k_context_t),
        m68k_pre_save,
        m68k_post_load,
        md
    );

    // Z80
    emu_save_state_register_region_ex(
        state,
        MD_STATE_REGION_CPU_Z80,
        z80_adapter_get_context(md->z80),
        sizeof(z80_context_t),
        z80_pre_save,
        z80_post_load,
        md
    );

    // VDP (registradores e estado)
    emu_save_state_register_region_ex(
        state,
        MD_STATE_REGION_VDP,
        md->vdp,
        sizeof(megadrive_vdp_context_t),
        vdp_pre_save,
        vdp_post_load,
        md
    );

    // VDP (memórias)
    megadrive_vdp_context_t *vdp = md->vdp;
    emu_save_state_register_region(state, MD_STATE_REGION_VDP_VRAM, vdp->vram, MD_VDP_VRAM_SIZE);
    emu_save_state_register_region(state, MD_STATE_REGION_VDP_CRAM, vdp->cram, MD_VDP_CRAM_SIZE);
    emu_save_state_register_region(state, MD_STATE_REGION_VDP_VSRAM, vdp->vsram, MD_VDP_VSRAM_SIZE);

    // Áudio (YM2612)
    emu_save_state_register_region_ex(
        state,
        MD_STATE_REGION_AUDIO_YM2612,
        audio_adapter_get_ym2612_context(md->audio),
        sizeof(ym2612_context_t),
        audio_pre_save,
        audio_post_load,
        md
    );

    // Áudio (PSG)
    emu_save_state_register_region_ex(
        state,
        MD_STATE_REGION_AUDIO_PSG,
        audio_adapter_get_psg_context(md->audio),
        sizeof(psg_context_t),
        NULL,
        NULL,
        md
    );

    // Memória
    emu_save_state_register_region_ex(
        state,
        MD_STATE_REGION_MEMORY,
        md->memory,
        sizeof(md_memory_context_t),
        memory_pre_save,
        memory_post_load,
        md
    );

    // Timer
    emu_save_state_register_region(
        state,
        MD_STATE_REGION_TIMER,
        md->timer,
        sizeof(md_timer_context_t)
    );

    // Estado do sistema
    emu_save_state_register_region(
        state,
        MD_STATE_REGION_SYSTEM,
        md,
        sizeof(megadrive_t)
    );

    // Definir metadados básicos
    char rom_title[49] = {0};
    memcpy(rom_title, md->rom_data + 0x150, 48);
    emu_save_state_set_metadata(state, MD_STATE_META_GAME_TITLE, rom_title);

    char region = md->rom_data[0x1F0];
    char region_str[2] = {region, 0};
    emu_save_state_set_metadata(state, MD_STATE_META_GAME_REGION, region_str);
    emu_save_state_set_metadata(state, MD_STATE_META_EMULATOR_VERSION, "Mega_Emu 1.0");
    emu_save_state_set_metadata(state, MD_STATE_META_SAVE_VERSION, "3.0");

    emu_log_info("Sistema de save state inicializado com sucesso");
    return state;
}

/**
 * @brief Salva o estado do Mega Drive
 * @param md Contexto do Mega Drive
 * @param filename Nome do arquivo para salvar
 * @param options Opções para salvar (NULL para usar opções padrão)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_save(megadrive_t *md, const char *filename, const emu_save_options_t *options) {
    if (!md || !md->save_state || !filename) {
        emu_log_error("Parâmetros inválidos para salvar o estado");
        return EMU_ERROR_INVALID_PARAMETER;
    }

    emu_log_info("Salvando estado para o arquivo: %s", filename);

    // Capturar thumbnail atual
    if (!options || (options->flags & EMU_SAVE_OPT_THUMBNAIL)) {
        uint8_t *framebuffer = NULL;
        uint32_t width = 0, height = 0;

        // Obter framebuffer atual do VDP
        vdp_adapter_get_framebuffer(md->vdp, &framebuffer, &width, &height);

        if (framebuffer && width > 0 && height > 0) {
            emu_save_state_capture_thumbnail(
                md->save_state,
                framebuffer,
                width,
                height,
                EMU_THUMB_FORMAT_RGB565
            );
        }
    }

    // Se não tiver opções, usar padrão
    emu_save_options_t default_options;
    if (!options) {
        memset(&default_options, 0, sizeof(default_options));
        default_options.flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL | EMU_SAVE_OPT_VERIFY;
        default_options.compression_level = 6;
        sprintf(default_options.description, "Auto-save: %s", md->game_title);
        options = &default_options;
    }

    // Salvar estado
    int32_t result = emu_save_state_save(md->save_state, filename, options);

    if (result == EMU_SUCCESS) {
        emu_log_info("Estado salvo com sucesso: %s", filename);
    } else {
        emu_log_error("Falha ao salvar estado: código %d", result);
    }

    return result;
}

/**
 * @brief Carrega um estado salvo
 * @param md Contexto do Mega Drive
 * @param filename Nome do arquivo para carregar
 * @param options Opções para carregamento (NULL para usar opções padrão)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_load(megadrive_t *md, const char *filename, const emu_save_options_t *options) {
    if (!md || !md->save_state || !filename) {
        emu_log_error("Parâmetros inválidos para carregar o estado");
        return EMU_ERROR_INVALID_PARAMETER;
    }

    emu_log_info("Carregando estado do arquivo: %s", filename);

    // Verificar informações do arquivo
    emu_save_info_t info;
    int32_t result = emu_save_state_get_info(filename, &info);

    if (result != EMU_SUCCESS) {
        emu_log_error("Falha ao obter informações do arquivo de save state: %s", filename);
        return result;
    }

    // Verificar compatibilidade
    if (info.platform_id != EMU_PLATFORM_MEGA_DRIVE) {
        emu_log_error("Arquivo de save state não é compatível com o Mega Drive");
        return EMU_ERROR_SAVE_WRONG_PLATFORM;
    }

    // Calcular hash da ROM atual
    char current_hash[65];
    md_memory_calculate_rom_hash(md->memory, current_hash, sizeof(current_hash));

    // Verificar compatibilidade da ROM
    if (strcmp(info.rom_hash, current_hash) != 0) {
        emu_log_warning("O save state foi criado para uma ROM diferente");
        // Nesse caso não retornamos erro, apenas um aviso
    }

    // Carregar estado
    result = emu_save_state_load(md->save_state, filename, options);

    if (result == EMU_SUCCESS) {
        emu_log_info("Estado carregado com sucesso: %s", filename);
    } else {
        emu_log_error("Falha ao carregar estado: código %d", result);
    }

    return result;
}

/**
 * @brief Configura o sistema de rewind
 * @param md Contexto do Mega Drive
 * @param frames Número máximo de frames a armazenar
 * @param interval Intervalo entre capturas (em frames)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_config_rewind(megadrive_t *md, uint32_t frames, uint32_t interval) {
    if (!md || !md->save_state) {
        emu_log_error("Contexto de save state inválido para configurar rewind");
        return EMU_ERROR_INVALID_PARAMETER;
    }

    emu_log_info("Configurando sistema de rewind: %d frames, intervalo %d", frames, interval);
    return emu_save_state_config_rewind(md->save_state, frames, interval);
}

/**
 * @brief Captura o estado atual para o buffer de rewind
 * @param md Contexto do Mega Drive
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_rewind_capture(megadrive_t *md) {
    if (!md || !md->save_state) {
        return EMU_ERROR_INVALID_PARAMETER;
    }

    return emu_save_state_rewind_capture(md->save_state);
}

/**
 * @brief Retrocede ou avança no buffer de rewind
 * @param md Contexto do Mega Drive
 * @param steps Número de passos (negativo para retroceder, positivo para avançar)
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_rewind_step(megadrive_t *md, int32_t steps) {
    if (!md || !md->save_state) {
        emu_log_error("Contexto de save state inválido para rewind");
        return EMU_ERROR_INVALID_PARAMETER;
    }

    emu_log_debug("Executando rewind de %d passos", steps);
    return emu_save_state_rewind_step(md->save_state, steps);
}

/**
 * @brief Obtém informações sobre o estado atual do buffer de rewind
 * @param md Contexto do Mega Drive
 * @param total_frames Ponteiro para receber o número total de frames armazenados
 * @param current_position Ponteiro para receber a posição atual no buffer
 * @param memory_usage Ponteiro para receber o uso de memória em bytes
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_rewind_get_info(megadrive_t *md, uint32_t *total_frames, uint32_t *current_position, uint32_t *memory_usage) {
    if (!md || !md->save_state) {
        return EMU_ERROR_INVALID_PARAMETER;
    }

    return emu_save_state_rewind_get_info(md->save_state, total_frames, current_position, memory_usage);
}

/**
 * @brief Limpa todos os recursos do sistema de save state
 * @param md Contexto do Mega Drive
 */
void md_save_state_shutdown(megadrive_t *md) {
    if (!md || !md->save_state) {
        return;
    }

    emu_log_info("Desligando sistema de save state");
    emu_save_state_shutdown(md->save_state);
    md->save_state = NULL;
}

/**
 * @brief Configura a criptografia para salvar estados
 * @param md Contexto do Mega Drive
 * @param config Configuração de criptografia
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_set_encryption(megadrive_t *md, const emu_encryption_config_t *config) {
    if (!md || !md->save_state || !config) {
        return EMU_ERROR_INVALID_PARAMETER;
    }

    emu_log_info("Configurando criptografia para save states (método: %d)", config->method);
    return emu_save_state_set_encryption(md->save_state, config);
}

/**
 * @brief Configura a integração com serviços de nuvem
 * @param md Contexto do Mega Drive
 * @param config Configuração para a nuvem
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_cloud_configure(megadrive_t *md, const emu_cloud_config_t *config) {
    if (!md || !md->save_state || !config) {
        return EMU_ERROR_INVALID_PARAMETER;
    }

    emu_log_info("Configurando integração com nuvem (provedor: %d)", config->provider);
    return emu_save_state_cloud_configure(md->save_state, config);
}

/**
 * @brief Sincroniza um save state com a nuvem
 * @param md Contexto do Mega Drive
 * @param filename Caminho do arquivo local
 * @param upload true para upload, false para download
 * @return EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int32_t md_save_state_cloud_sync(megadrive_t *md, const char *filename, bool upload) {
    if (!md || !md->save_state || !filename) {
        return EMU_ERROR_INVALID_PARAMETER;
    }

    emu_log_info("Sincronizando save state com a nuvem: %s (upload: %d)", filename, upload);
    return emu_save_state_cloud_sync(md->save_state, filename, upload);
}

// =============================================================================
// Implementações dos Callbacks
// =============================================================================

// Callbacks de CPU M68K
static void m68k_pre_save(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Qualquer preparação específica para salvar o estado do M68K
    emu_log_debug("Preparando para salvar estado do M68K");

    // Exemplo: sincronizar estado para garantir consistência
    m68k_adapter_sync_state(md->m68k);
}

static void m68k_post_load(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Qualquer processamento após carregar o estado do M68K
    emu_log_debug("Processando estado carregado do M68K");

    // Exemplo: reinicializar cache após o carregamento
    m68k_adapter_reset_cache(md->m68k);
}

// Callbacks de CPU Z80
static void z80_pre_save(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Preparação para salvar estado do Z80
    emu_log_debug("Preparando para salvar estado do Z80");
}

static void z80_post_load(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Processamento após carregar estado do Z80
    emu_log_debug("Processando estado carregado do Z80");
}

// Callbacks de VDP
static void vdp_pre_save(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Preparação para salvar estado do VDP
    emu_log_debug("Preparando para salvar estado do VDP");

    // Exemplo: sincronizar estado para garantir consistência
    vdp_adapter_sync_state(md->vdp);
}

static void vdp_post_load(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Processamento após carregar estado do VDP
    emu_log_debug("Processando estado carregado do VDP");

    // Exemplo: recomputar sprites após o carregamento
    vdp_adapter_update_pattern_cache(md->vdp);
}

// Callbacks de Áudio
static void audio_pre_save(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Preparação para salvar estado do YM2612
    emu_log_debug("Preparando para salvar estado do YM2612");
}

static void audio_post_load(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Processamento após carregar estado do YM2612
    emu_log_debug("Processando estado carregado do YM2612");

    // Exemplo: reinicializar tabelas de lookup
    audio_adapter_reset_fm_tables(md->audio);
}

// Callbacks de Memória
static void memory_pre_save(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Preparação para salvar estado da memória
    emu_log_debug("Preparando para salvar estado da memória");
}

static void memory_post_load(void *data, size_t size, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md || !data) return;

    // Processamento após carregar estado da memória
    emu_log_debug("Processando estado carregado da memória");

    // Exemplo: invalidar caches após carregar
    md_memory_invalidate_cache(md->memory);
}

// Callbacks gerais do sistema de save state
static void pre_save_callback(emu_save_state_t *state, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md) return;

    emu_log_info("Preparando para salvar estado completo");

    // Pausar a emulação durante o salvamento
    md->is_paused = true;

    // Adicionar timestamp ao metadado
    char timestamp[32];
    sprintf(timestamp, "%llu", (unsigned long long)time(NULL));
    emu_save_state_set_metadata(state, MD_STATE_META_TIMESTAMP, timestamp);
}

static void post_save_callback(emu_save_state_t *state, const char *filename, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md) return;

    emu_log_info("Estado completo salvo com sucesso: %s", filename);

    // Retomar emulação após salvamento
    md->is_paused = false;
}

static void pre_load_callback(emu_save_state_t *state, const char *filename, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md) return;

    emu_log_info("Preparando para carregar estado completo: %s", filename);

    // Pausar a emulação durante o carregamento
    md->is_paused = true;
}

static void post_load_callback(emu_save_state_t *state, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;
    if (!md) return;

    emu_log_info("Estado completo carregado com sucesso");

    // Reinicializar componentes após carregamento
    megadrive_post_state_load(md);

    // Retomar emulação
    md->is_paused = false;
}

static void error_callback(emu_save_state_t *state, int32_t error_code, const char *message, void *user_data) {
    megadrive_t *md = (megadrive_t *)user_data;

    emu_log_error("Erro no sistema de save state: [%d] %s", error_code, message);

    // Garantir que a emulação seja retomada mesmo em caso de erro
    if (md) {
        md->is_paused = false;
    }
}
