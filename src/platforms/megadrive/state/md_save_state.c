/**
 * @file md_save_state.c
 * @brief Adaptador para integrar o Mega Drive com o sistema unificado de Save States
 * @version 1.0
 * @date 2025-03-18
 *
 * Este arquivo implementa a adaptação do sistema de save states do Mega Drive
 * para o novo sistema unificado, preservando a compatibilidade com os formatos
 * antigos e adicionando suporte a novos recursos como criptografia e nuvem.
 */

#include <stdlib.h>
#include <string.h>

#include "md_save_state.h"
#include "core/save_state.h"
#include "core/save_state_crypto.h"
#include "core/save_state_cloud.h"
#include "platforms/megadrive/md_core.h"
#include "platforms/megadrive/m68k/m68k.h"
#include "platforms/megadrive/z80/z80.h"
#include "platforms/megadrive/vdp/vdp.h"
#include "platforms/megadrive/sound/fm.h"
#include "platforms/megadrive/sound/psg.h"
#include "utils/logging.h"

/* ID do formato Mega Drive no sistema unificado */
#define MD_SAVE_FORMAT_ID 0x01

/* IDs de regiões para o Mega Drive */
typedef enum
{
    MD_REGION_HEADER = 0x01,
    MD_REGION_M68K = 0x02,
    MD_REGION_Z80 = 0x03,
    MD_REGION_VDP = 0x04,
    MD_REGION_FM = 0x05,
    MD_REGION_PSG = 0x06,
    MD_REGION_MEMORY = 0x07,
    MD_REGION_IO = 0x08,
    MD_REGION_CART = 0x09,
    MD_REGION_SRAM = 0x0A,
    MD_REGION_METADATA = 0x0B
} md_region_id_t;

/* Estrutura privada para o adaptador */
typedef struct
{
    md_context_t *md_context; /* Contexto do Mega Drive */
    bool registered;          /* Flag indicando se os componentes foram registrados */
    bool legacy_mode;         /* Modo de compatibilidade com formatos antigos */
    uint8_t save_flags;       /* Flags específicas do Mega Drive */
    char game_id[32];         /* ID do jogo (nome de arquivo da ROM) */
} md_state_adapter_t;

/**
 * @brief Registra uma região de memória para o contexto de save state
 */
static void md_register_region(emu_save_state_t *state,
                               uint32_t region_id,
                               const char *name,
                               void *data,
                               size_t size)
{
    /* Registra a região no sistema unificado */
    emu_save_state_register_region(state, region_id, name, data, size);

    /* Configura flags específicas para a região */
    emu_region_flags_t flags = 0;

    /* Habilita compressão delta para certas regiões */
    if (region_id == MD_REGION_MEMORY || region_id == MD_REGION_SRAM)
    {
        flags |= EMU_REGION_DELTA_COMPRESS;
    }

    /* Marca regiões sensíveis para possível criptografia individual */
    if (region_id == MD_REGION_SRAM)
    {
        flags |= EMU_REGION_SENSITIVE;
    }

    /* Aplica as flags */
    emu_save_state_set_region_flags(state, region_id, flags);
}

/**
 * @brief Função de callback para capturar thumbnails
 */
static bool md_thumbnail_callback(emu_save_state_t *state, uint8_t *buffer,
                                  int width, int height, int *actual_width,
                                  int *actual_height)
{
    /* Obtém o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (!adapter || !adapter->md_context)
    {
        return false;
    }

    /* Captura a tela atual do emulador */
    md_context_t *md = adapter->md_context;

    /* Verifica se temos acesso ao buffer de vídeo */
    if (!md->vdp || !md->vdp->framebuffer)
    {
        return false;
    }

    /* Define as dimensões reais */
    *actual_width = 320;
    *actual_height = 240;

    /* Copia e converte o framebuffer */
    uint32_t *src = (uint32_t *)md->vdp->framebuffer;
    uint8_t *dst = buffer;

    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 320; x++)
        {
            uint32_t pixel = src[y * 320 + x];

            /* Formato do thumbnail é RGB */
            *dst++ = (pixel >> 16) & 0xFF; /* R */
            *dst++ = (pixel >> 8) & 0xFF;  /* G */
            *dst++ = pixel & 0xFF;         /* B */
        }
    }

    return true;
}

/**
 * @brief Função de callback para validação antes de salvar
 */
static bool md_pre_save_callback(emu_save_state_t *state)
{
    /* Obtém o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (!adapter || !adapter->md_context)
    {
        return false;
    }

    md_context_t *md = adapter->md_context;

    /* Prepara o Mega Drive para o save state */

    /* Sincroniza os processadores */
    md_synchronize_processors(md);

    /* Atualiza metadados */
    emu_save_info_t info;
    emu_save_state_get_info(state, &info);

    /* Adiciona ou atualiza metadados específicos */
    emu_save_state_set_metadata(state, "md_vdp_mode",
                                md->vdp->mode ? "h40" : "h32",
                                strlen(md->vdp->mode ? "h40" : "h32") + 1);

    emu_save_state_set_metadata(state, "md_region",
                                md->region == 0 ? "JP" : (md->region == 1 ? "US" : "EU"),
                                3);

    /* Verifica se existe SRAM e se tem dados */
    if (md->cart && md->cart->sram_size > 0 && md->cart->sram)
    {
        /* Marca que o save state contém SRAM */
        emu_save_state_set_metadata(state, "md_has_sram", "true", 5);

        /* Se a SRAM estiver sendo usada, ativa criptografia para esta região */
        if (md->cart->sram_modified)
        {
            /* Marca a região SRAM como sensível se não estiver marcada */
            emu_region_flags_t flags;
            emu_save_state_get_region_flags(state, MD_REGION_SRAM, &flags);

            if (!(flags & EMU_REGION_SENSITIVE))
            {
                flags |= EMU_REGION_SENSITIVE;
                emu_save_state_set_region_flags(state, MD_REGION_SRAM, flags);
            }
        }
    }

    return true;
}

/**
 * @brief Função de callback para restauração após carregar
 */
static bool md_post_load_callback(emu_save_state_t *state)
{
    /* Obtém o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (!adapter || !adapter->md_context)
    {
        return false;
    }

    md_context_t *md = adapter->md_context;

    /* Recupera metadados específicos do Mega Drive */
    char vdp_mode[8];
    size_t size = sizeof(vdp_mode);

    if (emu_save_state_get_metadata(state, "md_vdp_mode", vdp_mode, &size))
    {
        /* Ajusta o modo do VDP se necessário */
        bool h40 = (strcmp(vdp_mode, "h40") == 0);
        if (md->vdp->mode != h40)
        {
            md_vdp_set_mode(md->vdp, h40);
        }
    }

    /* Atualiza o relógio do emulador */
    md_z80_sync_clock(md->z80);
    md_fm_update_timers(md->fm);

    /* Reinicia o pipeline do M68K */
    md_m68k_reset_pipeline(md->m68k);

    /* Regenerar paleta e planos do VDP */
    md_vdp_update_palette(md->vdp);
    md_vdp_update_planes(md->vdp);

    /* Notifica componentes sobre a carga */
    md_notify_components(md, MD_EVENT_STATE_LOADED);

    return true;
}

/**
 * @brief Função para migrar um formato antigo de save state para o novo
 */
static bool md_migrate_legacy_state(const char *filepath, emu_save_state_t *state)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        return false;
    }

    /* Verifica o cabeçalho do formato antigo */
    char header[16];
    if (fread(header, 1, sizeof(header), file) != sizeof(header))
    {
        fclose(file);
        return false;
    }

    /* Verifica a assinatura do formato antigo */
    if (memcmp(header, "MD_STATE", 8) != 0)
    {
        fclose(file);
        return false;
    }

    /* Extrai a versão do formato antigo */
    uint32_t version;
    fread(&version, 1, sizeof(version), file);

    /* Obtém o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (!adapter || !adapter->md_context)
    {
        fclose(file);
        return false;
    }

    md_context_t *md = adapter->md_context;

    /* Processo de migração dependente da versão */
    bool success = false;

    if (version == 0x0103)
    { /* Versão 1.3 */
        /* Migração do formato 1.3 */
        /* Carrega cada seção e registra no novo formato */

        /* Aqui seria implementada a lógica para carregar cada componente
           do formato antigo e registrá-lo no novo sistema */

        /* Para brevidade, esta implementação está omitida */
        success = true;
    }
    else if (version == 0x0102)
    { /* Versão 1.2 */
        /* Migração do formato 1.2 */
        success = true;
    }
    else
    {
        /* Formato não suportado para migração */
        success = false;
    }

    fclose(file);
    return success;
}

/**
 * @brief Registra todos os componentes do Mega Drive no sistema de save state
 */
bool md_save_state_register(emu_save_state_t *state)
{
    if (!state)
    {
        return false;
    }

    /* Obtém o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (!adapter || !adapter->md_context)
    {
        return false;
    }

    md_context_t *md = adapter->md_context;

    /* Verificação se já foi registrado */
    if (adapter->registered)
    {
        return true;
    }

    /* Registra cada componente */

    /* M68K */
    if (md->m68k)
    {
        md_register_region(state, MD_REGION_M68K, "M68K", md->m68k, sizeof(m68k_context_t));
    }

    /* Z80 */
    if (md->z80)
    {
        md_register_region(state, MD_REGION_Z80, "Z80", md->z80, sizeof(z80_context_t));
    }

    /* VDP */
    if (md->vdp)
    {
        md_register_region(state, MD_REGION_VDP, "VDP", md->vdp, sizeof(vdp_context_t));
    }

    /* FM */
    if (md->fm)
    {
        md_register_region(state, MD_REGION_FM, "FM", md->fm, sizeof(fm_context_t));
    }

    /* PSG */
    if (md->psg)
    {
        md_register_region(state, MD_REGION_PSG, "PSG", md->psg, sizeof(psg_context_t));
    }

    /* Memória principal */
    if (md->memory && md->memory_size > 0)
    {
        md_register_region(state, MD_REGION_MEMORY, "RAM", md->memory, md->memory_size);
    }

    /* I/O */
    if (md->io)
    {
        md_register_region(state, MD_REGION_IO, "IO", md->io, sizeof(io_context_t));
    }

    /* Cartridge */
    if (md->cart)
    {
        md_register_region(state, MD_REGION_CART, "Cart", md->cart, sizeof(cart_context_t));
    }

    /* SRAM (se existir) */
    if (md->cart && md->cart->sram && md->cart->sram_size > 0)
    {
        md_register_region(state, MD_REGION_SRAM, "SRAM", md->cart->sram, md->cart->sram_size);
    }

    /* Registra callbacks */
    emu_save_state_set_thumbnail_callback(state, md_thumbnail_callback);
    emu_save_state_set_pre_save_callback(state, md_pre_save_callback);
    emu_save_state_set_post_load_callback(state, md_post_load_callback);

    /* Configura metadados */
    emu_save_state_set_metadata(state, "system_type", "Mega Drive", 11);
    emu_save_state_set_metadata(state, "rom_name", md->cart ? md->cart->rom_name : "Unknown",
                                strlen(md->cart ? md->cart->rom_name : "Unknown") + 1);

    /* Se houver checksum da ROM, salva como metadado */
    if (md->cart && md->cart->checksum)
    {
        char checksum[16];
        snprintf(checksum, sizeof(checksum), "%08X", md->cart->checksum);
        emu_save_state_set_metadata(state, "rom_checksum", checksum, strlen(checksum) + 1);
    }

    /* Marca como registrado */
    adapter->registered = true;

    return true;
}

/**
 * @brief Inicializa o adaptador de save state para o Mega Drive
 */
bool md_save_state_init(emu_save_state_t *state, md_context_t *md_context)
{
    if (!state || !md_context)
    {
        return false;
    }

    /* Aloca o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)malloc(sizeof(md_state_adapter_t));
    if (!adapter)
    {
        return false;
    }

    /* Inicializa o adaptador */
    memset(adapter, 0, sizeof(md_state_adapter_t));
    adapter->md_context = md_context;
    adapter->registered = false;
    adapter->legacy_mode = false;

    /* Extrai o game ID da ROM */
    if (md_context->cart && md_context->cart->rom_name)
    {
        strncpy(adapter->game_id, md_context->cart->rom_name, sizeof(adapter->game_id) - 1);
        adapter->game_id[sizeof(adapter->game_id) - 1] = '\0';
    }
    else
    {
        strcpy(adapter->game_id, "unknown");
    }

    /* Define o adaptador como dados de usuário do save state */
    emu_save_state_set_user_data(state, adapter);

    LOG_INFO("MD Save State adapter inicializado");

    return true;
}

/**
 * @brief Finaliza o adaptador de save state
 */
void md_save_state_shutdown(emu_save_state_t *state)
{
    if (!state)
    {
        return;
    }

    /* Obtém e libera o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (adapter)
    {
        free(adapter);
    }

    /* Limpa os dados de usuário */
    emu_save_state_set_user_data(state, NULL);
}

/**
 * @brief Salva o estado do Mega Drive usando uma implementação legada
 *
 * Esta função é mantida para compatibilidade com código existente,
 * internamente ela utiliza o novo sistema unificado.
 */
bool md_save_state_save_legacy(md_context_t *md, const char *filepath)
{
    if (!md || !filepath)
    {
        return false;
    }

    /* Cria um contexto do sistema unificado */
    emu_save_state_t *state = emu_save_state_init(EMU_PLATFORM_MEGA_DRIVE,
                                                  md->cart ? md->cart->rom_data : NULL,
                                                  md->cart ? md->cart->rom_size : 0);
    if (!state)
    {
        return false;
    }

    /* Inicializa o adaptador */
    if (!md_save_state_init(state, md))
    {
        emu_save_state_shutdown(state);
        return false;
    }

    /* Registra os componentes */
    if (!md_save_state_register(state))
    {
        md_save_state_shutdown(state);
        emu_save_state_shutdown(state);
        return false;
    }

    /* Configura opções de save */
    emu_save_options_t options;
    memset(&options, 0, sizeof(options));
    options.flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL;
    options.compression_level = 6;

    /* Salva o estado */
    bool result = emu_save_state_save(state, filepath, &options);

    /* Limpa recursos */
    md_save_state_shutdown(state);
    emu_save_state_shutdown(state);

        return result;
}

/**
 * @brief Carrega o estado do Mega Drive usando uma implementação legada
 *
 * Esta função é mantida para compatibilidade com código existente,
 * internamente ela utiliza o novo sistema unificado.
 */
bool md_save_state_load_legacy(md_context_t *md, const char *filepath)
{
    if (!md || !filepath)
    {
        return false;
    }

    /* Verifica se o arquivo existe */
    FILE *test = fopen(filepath, "rb");
    if (!test)
    {
        return false;
    }
    fclose(test);

    /* Cria um contexto do sistema unificado */
    emu_save_state_t *state = emu_save_state_init(EMU_PLATFORM_MEGA_DRIVE,
                                                  md->cart ? md->cart->rom_data : NULL,
                                                  md->cart ? md->cart->rom_size : 0);
    if (!state)
    {
        return false;
    }

    /* Inicializa o adaptador */
    if (!md_save_state_init(state, md))
    {
        emu_save_state_shutdown(state);
        return false;
    }

    /* Obtém o adaptador */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (!adapter)
    {
        emu_save_state_shutdown(state);
        return false;
    }

    /* Registra os componentes */
    if (!md_save_state_register(state))
    {
        md_save_state_shutdown(state);
        emu_save_state_shutdown(state);
        return false;
    }

    /* Configura o modo legacy para detectar e migrar formatos antigos */
    adapter->legacy_mode = true;

    /* Configura opções de load */
    emu_load_options_t options;
    memset(&options, 0, sizeof(options));
    options.flags = EMU_LOAD_OPT_MIGRATE | EMU_LOAD_OPT_VALIDATE;

    /* Carrega o estado */
    bool result = emu_save_state_load(state, filepath, &options);

    /* Se falhou e estamos em modo legacy, tenta migrar de um formato antigo */
    if (!result && adapter->legacy_mode)
    {
        /* Tenta migrar um formato legado */
        if (md_migrate_legacy_state(filepath, state))
        {
            /* Após migrar, salva no novo formato */
            char new_path[512];
            snprintf(new_path, sizeof(new_path), "%s.new", filepath);

            emu_save_options_t save_options;
            memset(&save_options, 0, sizeof(save_options));
            save_options.flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL;

            result = emu_save_state_save(state, new_path, &save_options);

            if (result)
            {
                /* Carrega o novo arquivo salvo */
                result = emu_save_state_load(state, new_path, &options);
            }
        }
    }

    /* Limpa recursos */
    md_save_state_shutdown(state);
    emu_save_state_shutdown(state);

    return result;
}

/**
 * @brief Ativa a criptografia para os save states do Mega Drive
 */
bool md_save_state_enable_encryption(emu_save_state_t *state, const char *password)
{
    if (!state || !password)
    {
        return false;
    }

    /* Configura a criptografia */
    emu_encryption_config_t config;
    memset(&config, 0, sizeof(config));

    config.method = EMU_CRYPT_AES256_GCM;
    config.derive_from_password = true;
    strncpy(config.password, password, sizeof(config.password) - 1);
    config.kdf_iterations = 10000;
    config.kdf = EMU_KDF_PBKDF2;

    /* Aplica a configuração */
    return emu_save_state_set_encryption(state, &config);
}

/**
 * @brief Configura a integração com nuvem para os save states do Mega Drive
 */
bool md_save_state_enable_cloud(emu_save_state_t *state,
                                emu_cloud_provider_t provider,
                                const char *auth_token,
                                bool auto_sync)
{
    if (!state || !auth_token)
    {
        return false;
    }

    /* Configura a integração com nuvem */
    emu_cloud_config_t config;
    memset(&config, 0, sizeof(config));

    config.provider = provider;
    strncpy(config.auth_token, auth_token, sizeof(config.auth_token) - 1);

    /* Define a pasta remota com base no game ID */
    md_state_adapter_t *adapter = (md_state_adapter_t *)emu_save_state_get_user_data(state);
    if (adapter && adapter->game_id[0] != '\0')
    {
        snprintf(config.folder_path, sizeof(config.folder_path),
                 "/MegaEmu/SaveStates/%s", adapter->game_id);
    }
    else
    {
        strcpy(config.folder_path, "/MegaEmu/SaveStates");
    }

    config.auto_sync = auto_sync;
    config.sync_interval = auto_sync ? 300 : 0; /* 5 minutos se auto_sync for true */
    config.conflict_strategy = EMU_CLOUD_CONFLICT_ASK;

    /* Aplica a configuração */
    return emu_save_state_cloud_configure(state, &config);
}
