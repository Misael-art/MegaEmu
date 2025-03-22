/**
 * @file mastersystem.c
 * @brief Implementação da plataforma Master System/Game Gear
 */

#include "mastersystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/core.h"
#include "../../utils/enhanced_log.h"
#include "../../utils/log_categories.h"
#include "../../utils/performance.h"
#include "core/save_state.h"

// Protótipos para componentes internos do Master System (serão implementados em arquivos separados)
#include "cpu/z80_adapter.h"
#include "video/sms_vdp.h"
#include "audio/sms_psg.h"
#include "memory/sms_memory.h"
#include "io/sms_io.h"

// Definir a categoria de log para a plataforma Master System
#define EMU_LOG_CAT_PLATFORM EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para a plataforma Master System
#define SMS_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)
#define SMS_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)
#define SMS_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)
#define SMS_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)
#define SMS_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)

/**
 * @brief Dimensões da tela do Master System
 */
#define SMS_SCREEN_WIDTH 256
#define SMS_SCREEN_HEIGHT 192
#define SMS_GG_SCREEN_WIDTH 160
#define SMS_GG_SCREEN_HEIGHT 144

/**
 * @brief Estado global do sistema Master System
 */
struct sms_state_s {
    sms_z80_adapter_t *cpu;     // Novo: adaptador Z80
    sms_vdp_t *vdp;
    sms_psg_t *psg;
    sms_memory_t *memory;
    sms_input_t *input;
    sms_cartridge_t *cartridge;
    sms_config_t config;
    uint32_t cycles_count;
    uint32_t frame_count;
    bool initialized;
    sms_rom_info_t rom_info;
};

struct sms_state_s g_sms_state = {0};

/**
 * @brief Contexto de save state para o Master System
 */
static save_state_t *g_sms_save_state = NULL;

/**
 * @brief Callback chamado quando o estado muda
 */
static void sms_save_state_callback(void *user_data) {
    SMS_LOG_INFO("Estado do Master System alterado");

    // Atualiza os componentes após um load
    if (g_sms_state.cpu) {
        // Possível reset de cache ou outras operações após load
        sms_z80_adapter_update_after_state_load(g_sms_state.cpu);
    }

    if (g_sms_state.vdp) {
        // Atualiza estado interno do VDP, se necessário
        sms_vdp_update_after_state_load(g_sms_state.vdp);
    }

    if (g_sms_state.psg) {
        // Reinicia buffers de áudio, se necessário
        sms_psg_update_after_state_load(g_sms_state.psg);
    }
}

/**
 * @brief Inicializa o sistema de save states para o Master System
 *
 * @return int Código de erro (0 para sucesso)
 */
static int sms_save_state_init(void) {
    if (!g_sms_state.initialized) {
        SMS_LOG_ERROR("Tentativa de inicializar save state com sistema Master System não inicializado");
        return SMS_ERROR_INITIALIZATION;
    }

    // Libera contexto anterior, se existir
    if (g_sms_save_state) {
        save_state_destroy(g_sms_save_state);
        g_sms_save_state = NULL;
    }

    // Calcula o CRC32 da ROM ou usa o já calculado pelo cartucho
    uint32_t rom_crc32 = g_sms_state.rom_info.checksum;

    // Cria o contexto de save state
    g_sms_save_state = save_state_create(
        2, // ID da plataforma Master System
        rom_crc32,
        g_sms_state.config.rom_path ? g_sms_state.config.rom_path : "unknown");

    if (!g_sms_save_state) {
        SMS_LOG_ERROR("Falha ao criar contexto de save state");
        return SMS_ERROR_MEMORY_ALLOCATION;
    }

    // Registra o callback
    save_state_set_callback(g_sms_save_state, sms_save_state_callback, NULL);
    SMS_LOG_INFO("Sistema de save state inicializado");

    return SMS_ERROR_NONE;
}

/**
 * @brief Registra os componentes do Master System no sistema de save state
 *
 * @return int Código de erro (0 para sucesso)
 */
static int sms_save_state_register_components(void) {
    if (!g_sms_save_state) {
        SMS_LOG_ERROR("Contexto de save state não inicializado");
        return SMS_ERROR_INITIALIZATION;
    }

    int result = SAVE_STATE_ERROR_NONE;

    // Registra CPU se disponível
    if (g_sms_state.cpu) {
        result = sms_z80_adapter_register_save_state(g_sms_state.cpu, g_sms_save_state);
        if (result != SAVE_STATE_ERROR_NONE) {
            SMS_LOG_ERROR("Falha ao registrar estado da CPU");
            return result;
        }
    }

    // Registra VDP se disponível
    if (g_sms_state.vdp) {
        result = sms_vdp_register_save_state(g_sms_state.vdp, g_sms_save_state);
        if (result != SAVE_STATE_ERROR_NONE) {
            SMS_LOG_ERROR("Falha ao registrar estado do VDP");
            return result;
        }
    }

    // Registra PSG se disponível
    if (g_sms_state.psg) {
        result = sms_psg_register_save_state(g_sms_state.psg, g_sms_save_state);
        if (result != SAVE_STATE_ERROR_NONE) {
            SMS_LOG_ERROR("Falha ao registrar estado do PSG");
            return result;
        }
    }

    // Registra memória se disponível
    if (g_sms_state.memory) {
        result = sms_memory_register_save_state(g_sms_state.memory, g_sms_save_state);
        if (result != SAVE_STATE_ERROR_NONE) {
            SMS_LOG_ERROR("Falha ao registrar estado da memória");
            return result;
        }
    }

    // Registra controladores se disponível
    if (g_sms_state.input) {
        result = sms_input_register_save_state(g_sms_state.input, g_sms_save_state);
        if (result != SAVE_STATE_ERROR_NONE) {
            SMS_LOG_ERROR("Falha ao registrar estado dos controladores");
            return result;
        }
    }

    SMS_LOG_INFO("Componentes do Master System registrados para save state");
    return SMS_ERROR_NONE;
}

/**
 * @brief Finaliza o sistema de save states para o Master System
 */
static void sms_save_state_shutdown(void) {
    if (g_sms_save_state) {
        save_state_destroy(g_sms_save_state);
        g_sms_save_state = NULL;
        SMS_LOG_INFO("Sistema de save state finalizado");
    }
}

/**
 * @brief Inicializa o sistema Master System com configurações padrão
 *
 * @return sms_config_t Configuração com valores padrão
 */
static sms_config_t sms_get_default_config(void) {
    sms_config_t config;
    memset(&config, 0, sizeof(config));

    config.ntsc_mode = 1;                  // NTSC por padrão
    config.audio_enabled = 1;              // Áudio habilitado
    config.audio_sample_rate = 44100;      // Taxa de amostragem padrão
    config.log_level = EMU_LOG_LEVEL_INFO; // Nível de log padrão: INFO
    config.system_type = 0;                // Master System por padrão
    config.rom_path = NULL;

    return config;
}

/**
 * @brief Inicializa o sistema Master System com configurações padrão se não forem fornecidas
 *
 * @param config Configuração inicial (pode ser NULL para usar padrões)
 * @return int Código de erro (0 para sucesso)
 */
int32_t sms_init(const sms_config_t *config) {
    // Verifica se já foi inicializado
    if (g_sms_state.initialized) {
        SMS_LOG_WARN("Sistema Master System já inicializado");
        return SMS_ERROR_ALREADY_INITIALIZED;
    }

    SMS_LOG_INFO("Inicializando sistema Master System");

    // Limpa o estado global
    memset(&g_sms_state, 0, sizeof(struct sms_state_s));

    // Usa configurações padrão se não forem fornecidas
    if (!config) {
        g_sms_state.config = sms_get_default_config();
    } else {
        g_sms_state.config = *config;
    }

    // Inicializa o Z80
    g_sms_state.cpu = sms_z80_adapter_create();
    if (!g_sms_state.cpu) {
        SMS_LOG_ERROR("Falha ao criar processador Z80");
        sms_shutdown();
        return SMS_ERROR_INITIALIZATION;
    }

    // Inicializa o VDP
    g_sms_state.vdp = sms_vdp_create();
    if (!g_sms_state.vdp) {
        SMS_LOG_ERROR("Falha ao criar VDP");
        sms_shutdown();
        return SMS_ERROR_INITIALIZATION;
    }

    // Inicializa o PSG
    g_sms_state.psg = sms_psg_create();
    if (!g_sms_state.psg) {
        SMS_LOG_ERROR("Falha ao criar PSG");
        sms_shutdown();
        return SMS_ERROR_INITIALIZATION;
    }

    // Inicializa a memória
    g_sms_state.memory = sms_memory_create();
    if (!g_sms_state.memory) {
        SMS_LOG_ERROR("Falha ao criar sistema de memória");
        sms_shutdown();
        return SMS_ERROR_INITIALIZATION;
    }

    // Inicializa os controladores
    g_sms_state.input = sms_input_create();
    if (!g_sms_state.input) {
        SMS_LOG_ERROR("Falha ao criar sistema de entrada");
        sms_shutdown();
        return SMS_ERROR_INITIALIZATION;
    }

    // Conecta os componentes
    sms_z80_adapter_connect_memory(g_sms_state.cpu, g_sms_state.memory);
    sms_z80_adapter_connect_vdp(g_sms_state.cpu, g_sms_state.vdp);
    sms_z80_adapter_connect_psg(g_sms_state.cpu, g_sms_state.psg);
    sms_z80_connect_bus(g_sms_state.cpu, g_sms_state.memory);
    sms_vdp_connect_cpu(g_sms_state.vdp, g_sms_state.cpu);
    sms_psg_connect_cpu(g_sms_state.psg, g_sms_state.cpu);
    sms_memory_connect_vdp(g_sms_state.memory, g_sms_state.vdp);
    sms_memory_connect_psg(g_sms_state.memory, g_sms_state.psg);
    sms_memory_connect_input(g_sms_state.memory, g_sms_state.input);

    g_sms_state.initialized = 1;
    SMS_LOG_INFO("Sistema Master System inicializado com sucesso");

    return SMS_ERROR_NONE;
}

/**
 * @brief Finaliza o sistema Master System e libera recursos
 */
void sms_shutdown(void) {
    if (!g_sms_state.initialized) {
        SMS_LOG_WARN("Sistema Master System não inicializado");
        return;
    }

    SMS_LOG_INFO("Finalizando sistema Master System");

    // Finaliza o sistema de save states
    sms_save_state_shutdown();

    // Libera os componentes
    if (g_sms_state.cpu) {
        sms_z80_destroy(g_sms_state.cpu);
        g_sms_state.cpu = NULL;
    }

    if (g_sms_state.vdp) {
        sms_vdp_destroy(g_sms_state.vdp);
        g_sms_state.vdp = NULL;
    }

    if (g_sms_state.psg) {
        sms_psg_destroy(g_sms_state.psg);
        g_sms_state.psg = NULL;
    }

    if (g_sms_state.memory) {
        sms_memory_destroy(g_sms_state.memory);
        g_sms_state.memory = NULL;
    }

    if (g_sms_state.input) {
        sms_input_destroy(g_sms_state.input);
        g_sms_state.input = NULL;
    }

    if (g_sms_state.cartridge) {
        sms_cartridge_destroy(g_sms_state.cartridge);
        g_sms_state.cartridge = NULL;
    }

    // Limpa o estado da ROM
    if (g_sms_state.rom_info.rom_data) {
        free(g_sms_state.rom_info.rom_data);
        g_sms_state.rom_info.rom_data = NULL;
    }

    // Reset do estado global
    memset(&g_sms_state, 0, sizeof(sms_state_t));

    SMS_LOG_INFO("Sistema Master System finalizado com sucesso");
}

/**
 * @brief Reseta o sistema Master System (equivalente a pressionar o botão RESET)
 *
 * @return int Código de erro (0 para sucesso)
 */
int32_t sms_reset(void) {
    if (!g_sms_state.initialized) {
        SMS_LOG_ERROR("Tentativa de resetar sistema Master System não inicializado");
        return SMS_ERROR_NOT_INITIALIZED;
    }

    SMS_LOG_INFO("Resetando sistema Master System");

    // Reseta os componentes
    if (g_sms_state.cpu)
        sms_z80_reset(g_sms_state.cpu);

    if (g_sms_state.vdp)
        sms_vdp_reset(g_sms_state.vdp);

    if (g_sms_state.psg)
        sms_psg_reset(g_sms_state.psg);

    if (g_sms_state.memory)
        sms_memory_reset(g_sms_state.memory);

    if (g_sms_state.input)
        sms_input_reset(g_sms_state.input);

    if (g_sms_state.cartridge)
        sms_cartridge_reset(g_sms_state.cartridge);

    // Reseta contadores
    g_sms_state.cycles_count = 0;
    g_sms_state.frame_count = 0;

    SMS_LOG_INFO("Sistema Master System resetado com sucesso");

    return SMS_ERROR_NONE;
}

/**
 * @brief Carrega uma ROM no sistema Master System
 *
 * @param rom_path Caminho para o arquivo ROM
 * @return int Código de erro (0 para sucesso)
 */
int32_t sms_load_rom(const char *rom_path) {
    if (!g_sms_state.initialized) {
        SMS_LOG_ERROR("Tentativa de carregar ROM com sistema Master System não inicializado");
        return SMS_ERROR_NOT_INITIALIZED;
    }

    if (!rom_path) {
        SMS_LOG_ERROR("Caminho da ROM inválido");
        return SMS_ERROR_INVALID_PARAMETER;
    }

    SMS_LOG_INFO("Carregando ROM: %s", rom_path);

    // Libera ROM anterior se existir
    if (g_sms_state.cartridge) {
        sms_cartridge_destroy(g_sms_state.cartridge);
        g_sms_state.cartridge = NULL;
    }

    if (g_sms_state.rom_info.rom_data) {
        free(g_sms_state.rom_info.rom_data);
        g_sms_state.rom_info.rom_data = NULL;
    }

    // Cria e carrega o cartucho
    g_sms_state.cartridge = sms_cartridge_create();
    if (!g_sms_state.cartridge) {
        SMS_LOG_ERROR("Falha ao criar cartucho");
        return SMS_ERROR_MEMORY_ALLOCATION;
    }

    int result = sms_cartridge_load_rom(g_sms_state.cartridge, rom_path);
    if (result != SMS_ERROR_NONE) {
        SMS_LOG_ERROR("Falha ao carregar ROM: %d", result);
        sms_cartridge_destroy(g_sms_state.cartridge);
        g_sms_state.cartridge = NULL;
        return result;
    }

    // Obtém informações da ROM
    sms_cartridge_get_info(g_sms_state.cartridge, &g_sms_state.rom_info);

    // Atualiza o caminho da ROM na configuração
    if (g_sms_state.config.rom_path) {
        free((void*)g_sms_state.config.rom_path);
    }
    g_sms_state.config.rom_path = strdup(rom_path);

    // Conecta o cartucho à memória
    sms_memory_connect_cartridge(g_sms_state.memory, g_sms_state.cartridge);

    // Inicializa o sistema de save states
    result = sms_save_state_init();
    if (result != SMS_ERROR_NONE) {
        SMS_LOG_ERROR("Falha ao inicializar sistema de save state: %d", result);
        // Não é um erro fatal, podemos continuar
    } else {
        // Registra os componentes no sistema de save state
        result = sms_save_state_register_components();
        if (result != SMS_ERROR_NONE) {
            SMS_LOG_ERROR("Falha ao registrar componentes no sistema de save state: %d", result);
            // Não é um erro fatal, podemos continuar
        }
    }

    // Reseta o sistema para iniciar com a nova ROM
    sms_reset();

    SMS_LOG_INFO("ROM carregada com sucesso: %s", g_sms_state.rom_info.title);

    return SMS_ERROR_NONE;
}

/**
 * @brief Executa um único frame do sistema Master System
 *
 * @param frame_buffer Buffer para receber dados do frame renderizado
 * @param audio_buffer Buffer para receber dados de áudio
 * @param audio_buffer_size Tamanho do buffer de áudio
 * @return int Código de erro (0 para sucesso)
 */
int32_t sms_run_frame(uint32_t *frame_buffer, int16_t *audio_buffer, int32_t audio_buffer_size) {
    if (!g_sms_state.initialized) {
        SMS_LOG_ERROR("Tentativa de executar frame com sistema Master System não inicializado");
        return SMS_ERROR_NOT_INITIALIZED;
    }

    if (!g_sms_state.cartridge) {
        SMS_LOG_ERROR("Tentativa de executar frame sem ROM carregada");
        return SMS_ERROR_ROM_LOAD;
    }

    // Verifica se os buffers são válidos
    if (!frame_buffer) {
        SMS_LOG_ERROR("Buffer de frame inválido");
        return SMS_ERROR_INVALID_PARAMETER;
    }

    if (g_sms_state.config.audio_enabled && (!audio_buffer || audio_buffer_size <= 0)) {
        SMS_LOG_ERROR("Buffer de áudio inválido");
        return SMS_ERROR_INVALID_PARAMETER;
    }

    // Inicia a medição de desempenho
    perf_start_section("sms_frame");

    // Define o número de ciclos para este frame
    uint32_t cycles_per_frame = g_sms_state.config.ntsc_mode ?
                               SMS_NTSC_CYCLES_PER_FRAME :
                               SMS_PAL_CYCLES_PER_FRAME;

    // Limpa o buffer de áudio
    if (g_sms_state.config.audio_enabled) {
        memset(audio_buffer, 0, audio_buffer_size * sizeof(int16_t));
    }

    // Prepara o VDP para o novo frame
    sms_vdp_start_frame(g_sms_state.vdp);

    // Prepara o PSG para o novo frame
    if (g_sms_state.config.audio_enabled) {
        sms_psg_start_frame(g_sms_state.psg, audio_buffer, audio_buffer_size);
    }

    // Ciclo principal de emulação
    uint32_t cycles_executed = 0;

    while (cycles_executed < cycles_per_frame) {
        // Executa um passo da CPU
        uint8_t cycles = sms_z80_step(g_sms_state.cpu);
        cycles_executed += cycles;

        // Atualiza o VDP
        sms_vdp_update(g_sms_state.vdp, cycles);

        // Atualiza o PSG
        if (g_sms_state.config.audio_enabled) {
            sms_psg_update(g_sms_state.psg, cycles);
        }

        // Processa interrupções
        if (sms_vdp_check_interrupt(g_sms_state.vdp)) {
            sms_z80_interrupt(g_sms_state.cpu);
        }
    }

    // Finaliza o frame no VDP e obtém o buffer de vídeo
    sms_vdp_end_frame(g_sms_state.vdp, frame_buffer);

    // Finaliza o frame no PSG
    if (g_sms_state.config.audio_enabled) {
        sms_psg_end_frame(g_sms_state.psg);
    }

    // Atualiza contadores
    g_sms_state.cycles_count += cycles_executed;
    g_sms_state.frame_count++;

    // Finaliza a medição de desempenho
    perf_end_section("sms_frame");

    return SMS_ERROR_NONE;
}

/**
 * @brief Define o estado dos botões do controlador 1
 *
 * @param button_state Estado dos botões
 */
void sms_set_controller1(uint8_t button_state) {
    if (g_sms_state.initialized && g_sms_state.input) {
        sms_input_set_controller1(g_sms_state.input, button_state);
    }
}

/**
 * @brief Define o estado dos botões do controlador 2
 *
 * @param button_state Estado dos botões
 */
void sms_set_controller2(uint8_t button_state) {
    if (g_sms_state.initialized && g_sms_state.input) {
        sms_input_set_controller2(g_sms_state.input, button_state);
    }
}

/**
 * @brief Salva o estado atual do sistema Master System
 *
 * @param state_path Caminho para o arquivo de estado
 * @return int Código de erro (0 para sucesso)
 */
int32_t sms_save_state(const char *state_path) {
    if (!g_sms_state.initialized) {
        SMS_LOG_ERROR("Tentativa de salvar estado com sistema Master System não inicializado");
        return SMS_ERROR_NOT_INITIALIZED;
    }

    if (!g_sms_save_state) {
        SMS_LOG_ERROR("Sistema de save state não inicializado");
        return SMS_ERROR_INITIALIZATION;
    }

    SMS_LOG_INFO("Salvando estado: %s", state_path);

    int result = save_state_save(g_sms_save_state, state_path);
    if (result != SAVE_STATE_ERROR_NONE) {
        SMS_LOG_ERROR("Falha ao salvar estado: %d", result);
        return SMS_ERROR_INITIALIZATION;
    }

    SMS_LOG_INFO("Estado salvo com sucesso");

    return SMS_ERROR_NONE;
}

/**
 * @brief Carrega um estado salvo para o sistema Master System
 *
 * @param state_path Caminho para o arquivo de estado
 * @return int Código de erro (0 para sucesso)
 */
int32_t sms_load_state(const char *state_path) {
    if (!g_sms_state.initialized) {
        SMS_LOG_ERROR("Tentativa de carregar estado com sistema Master System não inicializado");
        return SMS_ERROR_NOT_INITIALIZED;
    }

    if (!g_sms_save_state) {
        SMS_LOG_ERROR("Sistema de save state não inicializado");
        return SMS_ERROR_INITIALIZATION;
    }

    SMS_LOG_INFO("Carregando estado: %s", state_path);

    int result = save_state_load(g_sms_save_state, state_path);
    if (result != SAVE_STATE_ERROR_NONE) {
        SMS_LOG_ERROR("Falha ao carregar estado: %d", result);
        return SMS_ERROR_INITIALIZATION;
    }

    SMS_LOG_INFO("Estado carregado com sucesso");

    return SMS_ERROR_NONE;
}

/**
 * @brief Obtém o estado atual do sistema Master System
 *
 * @return sms_state_t* Ponteiro para o estado (não modificar diretamente)
 */
const sms_state_t *sms_get_state(void) {
    return &g_sms_state;
}

/**
 * @brief Cria uma instância da plataforma Master System/Game Gear
 *
 * @return Nova instância da plataforma ou NULL em caso de erro
 */
emu_platform_t* emu_platform_mastersystem_create(void) {
    // Cria a plataforma base
    emu_platform_t* platform = emu_platform_create("mastersystem");
    if (!platform) {
        SMS_LOG_ERROR("Falha ao criar plataforma base");
        return NULL;
    }

    // Inicializa o sistema Master System com configurações padrão
    sms_config_t config = sms_get_default_config();
    int result = sms_init(&config);
    if (result != SMS_ERROR_NONE) {
        SMS_LOG_ERROR("Falha ao inicializar sistema Master System: %d", result);
        emu_platform_destroy(platform);
        return NULL;
    }

    // Configura callbacks da plataforma
    // Estes serão implementados posteriormente quando a interface da plataforma for definida

    return platform;
}
