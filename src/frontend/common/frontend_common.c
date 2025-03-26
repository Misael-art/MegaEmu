/**
 * @file frontend_common.c
 * @brief Implementação de funções comuns a todos os frontends
 *
 * Este arquivo contém a implementação de funções comuns que são utilizadas
 * por todos os frontends, independentemente da sua implementação específica
 * (SDL, Qt, etc.). Estas funções servem como uma camada de abstração para
 * garantir operações consistentes em diferentes sistemas.
 */

#include "frontend.h"
#include "frontend_config.h"
#include "frontend_internal.h"
#include "utils/enhanced_log.h"
#include "utils/log_categories.h"
#include "utils/log_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Diretórios padrão
static char g_rom_directory[512] = "./roms";
static char g_save_directory[512] = "./saves";
static char g_screenshots_directory[512] = "./screenshots";
static char g_states_directory[512] = "./states";

// Estado global do frontend
typedef struct {
    bool is_initialized;
    bool is_running;
    bool is_paused;
    int frontend_type;  // 0=SDL, 1=Qt, etc.
    emu_platform_t current_platform;
    char current_rom_path[512];
    void* platform_instance;
} frontend_state_t;

static frontend_state_t g_frontend_state = {
    .is_initialized = false,
    .is_running = false,
    .is_paused = false,
    .frontend_type = 0,
    .current_platform = EMU_PLATFORM_NONE,
    .current_rom_path = "",
    .platform_instance = NULL
};

// Callbacks gerais (serão definidos pelas implementações específicas)
static frontend_callbacks_t g_callbacks = {0};

/**
 * @brief Inicializa o sistema de frontend comum
 *
 * @param type Tipo de frontend: 0=SDL, 1=Qt, etc.
 * @return true se inicializado com sucesso
 */
bool emu_frontend_common_init(int type) {
    if (g_frontend_state.is_initialized) {
        EMU_LOG_WARN(EMU_LOG_CAT_FRONTEND, "Frontend já inicializado");
        return true;
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Inicializando frontend comum (tipo: %d)", type);

    // Inicializa a configuração
    emu_frontend_config_init();

    // Configura o estado inicial
    g_frontend_state.is_initialized = true;
    g_frontend_state.is_running = false;
    g_frontend_state.is_paused = false;
    g_frontend_state.frontend_type = type;
    g_frontend_state.current_platform = EMU_PLATFORM_NONE;
    g_frontend_state.current_rom_path[0] = '\0';
    g_frontend_state.platform_instance = NULL;

    return true;
}

/**
 * @brief Finaliza o sistema de frontend comum
 */
void emu_frontend_common_shutdown(void) {
    if (!g_frontend_state.is_initialized) {
        return;
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Finalizando frontend comum");

    // Libera recursos se necessário
    if (g_frontend_state.platform_instance) {
        if (g_callbacks.platform_shutdown) {
            g_callbacks.platform_shutdown(g_frontend_state.platform_instance);
        }
        g_frontend_state.platform_instance = NULL;
    }

    g_frontend_state.is_initialized = false;
    g_frontend_state.is_running = false;
    g_frontend_state.is_paused = false;
}

/**
 * @brief Registra os callbacks do frontend específico
 *
 * @param callbacks Estrutura com ponteiros para funções de callback
 */
void emu_frontend_register_callbacks(const frontend_callbacks_t* callbacks) {
    if (!callbacks) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Ponteiro de callbacks inválido");
        return;
    }

    memcpy(&g_callbacks, callbacks, sizeof(frontend_callbacks_t));
    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Callbacks do frontend registrados");
}

/**
 * @brief Define o diretório de ROMs
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_rom_directory(const char* directory) {
    if (!directory) {
        return;
    }

    strncpy(g_rom_directory, directory, sizeof(g_rom_directory) - 1);
    g_rom_directory[sizeof(g_rom_directory) - 1] = '\0';
    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Diretório de ROMs definido: %s", g_rom_directory);
}

/**
 * @brief Obtém o diretório de ROMs atual
 *
 * @return const char* Caminho para o diretório
 */
const char* emu_frontend_get_rom_directory(void) {
    return g_rom_directory;
}

/**
 * @brief Define o diretório de saves
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_save_directory(const char* directory) {
    if (!directory) {
        return;
    }

    strncpy(g_save_directory, directory, sizeof(g_save_directory) - 1);
    g_save_directory[sizeof(g_save_directory) - 1] = '\0';
    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Diretório de saves definido: %s", g_save_directory);
}

/**
 * @brief Obtém o diretório de saves atual
 *
 * @return const char* Caminho para o diretório
 */
const char* emu_frontend_get_save_directory(void) {
    return g_save_directory;
}

/**
 * @brief Define o diretório de screenshots
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_screenshots_directory(const char* directory) {
    if (!directory) {
        return;
    }

    strncpy(g_screenshots_directory, directory, sizeof(g_screenshots_directory) - 1);
    g_screenshots_directory[sizeof(g_screenshots_directory) - 1] = '\0';
    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Diretório de screenshots definido: %s", g_screenshots_directory);
}

/**
 * @brief Obtém o diretório de screenshots atual
 *
 * @return const char* Caminho para o diretório
 */
const char* emu_frontend_get_screenshots_directory(void) {
    return g_screenshots_directory;
}

/**
 * @brief Define o diretório de save states
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_states_directory(const char* directory) {
    if (!directory) {
        return;
    }

    strncpy(g_states_directory, directory, sizeof(g_states_directory) - 1);
    g_states_directory[sizeof(g_states_directory) - 1] = '\0';
    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Diretório de save states definido: %s", g_states_directory);
}

/**
 * @brief Obtém o diretório de save states atual
 *
 * @return const char* Caminho para o diretório
 */
const char* emu_frontend_get_states_directory(void) {
    return g_states_directory;
}

/**
 * @brief Verifica se o frontend está inicializado
 *
 * @return true se estiver inicializado
 */
bool emu_frontend_is_initialized(void) {
    return g_frontend_state.is_initialized;
}

/**
 * @brief Verifica se o emulador está em execução
 *
 * @return true se estiver rodando
 */
bool emu_frontend_is_running(void) {
    return g_frontend_state.is_running;
}

/**
 * @brief Verifica se o emulador está pausado
 *
 * @return true se estiver pausado
 */
bool emu_frontend_is_paused(void) {
    return g_frontend_state.is_paused;
}

/**
 * @brief Pausa ou retoma a emulação
 *
 * @param paused true para pausar, false para retomar
 */
void emu_frontend_set_paused(bool paused) {
    if (g_frontend_state.is_paused == paused) {
        return;
    }

    g_frontend_state.is_paused = paused;

    if (g_callbacks.pause_changed) {
        g_callbacks.pause_changed(paused);
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Emulação %s", paused ? "pausada" : "retomada");
}

/**
 * @brief Obtém a plataforma atualmente carregada
 *
 * @return emu_platform_t Tipo de plataforma
 */
emu_platform_t emu_frontend_get_current_platform(void) {
    return g_frontend_state.current_platform;
}

/**
 * @brief Obtém o caminho para a ROM atualmente carregada
 *
 * @return const char* Caminho da ROM
 */
const char* emu_frontend_get_current_rom_path(void) {
    return g_frontend_state.current_rom_path;
}

/**
 * @brief Carrega uma ROM e inicia a emulação
 *
 * @param rom_path Caminho para o arquivo da ROM
 * @param platform Plataforma para emular (se AUTO, será detectado)
 * @return true se a ROM foi carregada com sucesso
 */
bool emu_frontend_load_rom(const char* rom_path, emu_platform_t platform) {
    if (!g_frontend_state.is_initialized) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Frontend não inicializado");
        return false;
    }

    if (!rom_path) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Caminho de ROM inválido");
        return false;
    }

    // Finalizar instância anterior se existir
    if (g_frontend_state.platform_instance) {
        if (g_callbacks.platform_shutdown) {
            g_callbacks.platform_shutdown(g_frontend_state.platform_instance);
        }
        g_frontend_state.platform_instance = NULL;
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Carregando ROM: %s", rom_path);

    // Auto-detecção de plataforma se necessário
    if (platform == EMU_PLATFORM_AUTO) {
        if (g_callbacks.detect_platform) {
            platform = g_callbacks.detect_platform(rom_path);
        } else {
            EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Callback de detecção de plataforma não registrado");
            return false;
        }
    }

    if (platform == EMU_PLATFORM_NONE || platform == EMU_PLATFORM_AUTO) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Não foi possível detectar a plataforma para: %s", rom_path);
        return false;
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Plataforma detectada: %d", platform);

    // Iniciar a plataforma
    if (g_callbacks.platform_init) {
        g_frontend_state.platform_instance = g_callbacks.platform_init(platform);
        if (!g_frontend_state.platform_instance) {
            EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao inicializar a plataforma");
            return false;
        }
    } else {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Callback de inicialização de plataforma não registrado");
        return false;
    }

    // Carregar a ROM
    if (g_callbacks.load_rom) {
        if (!g_callbacks.load_rom(g_frontend_state.platform_instance, rom_path)) {
            EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao carregar a ROM");
            if (g_callbacks.platform_shutdown) {
                g_callbacks.platform_shutdown(g_frontend_state.platform_instance);
            }
            g_frontend_state.platform_instance = NULL;
            return false;
        }
    } else {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Callback de carregamento de ROM não registrado");
        if (g_callbacks.platform_shutdown) {
            g_callbacks.platform_shutdown(g_frontend_state.platform_instance);
        }
        g_frontend_state.platform_instance = NULL;
        return false;
    }

    // Atualizar estado
    g_frontend_state.current_platform = platform;
    strncpy(g_frontend_state.current_rom_path, rom_path, sizeof(g_frontend_state.current_rom_path) - 1);
    g_frontend_state.current_rom_path[sizeof(g_frontend_state.current_rom_path) - 1] = '\0';
    g_frontend_state.is_running = true;
    g_frontend_state.is_paused = false;

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "ROM carregada com sucesso");
    return true;
}

/**
 * @brief Descarrega a ROM atual e para a emulação
 */
void emu_frontend_unload_rom(void) {
    if (!g_frontend_state.is_initialized || !g_frontend_state.is_running) {
        return;
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Descarregando ROM atual");

    // Finalizar a plataforma
    if (g_frontend_state.platform_instance) {
        if (g_callbacks.platform_shutdown) {
            g_callbacks.platform_shutdown(g_frontend_state.platform_instance);
        }
        g_frontend_state.platform_instance = NULL;
    }

    // Atualizar estado
    g_frontend_state.is_running = false;
    g_frontend_state.is_paused = false;
    g_frontend_state.current_platform = EMU_PLATFORM_NONE;
    g_frontend_state.current_rom_path[0] = '\0';

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "ROM descarregada");
}

/**
 * @brief Reinicia a emulação da ROM atual
 *
 * @return true se a ROM foi reiniciada com sucesso
 */
bool emu_frontend_reset_current_rom(void) {
    if (!g_frontend_state.is_initialized || !g_frontend_state.is_running) {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Nenhuma ROM em execução para reiniciar");
        return false;
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Reiniciando emulação");

    if (g_callbacks.reset) {
        if (!g_callbacks.reset(g_frontend_state.platform_instance)) {
            EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Falha ao reiniciar a emulação");
            return false;
        }
    } else {
        EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Callback de reset não registrado");
        return false;
    }

    EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, "Emulação reiniciada com sucesso");
    return true;
}

/**
 * @brief Obtém a instância da plataforma atual
 *
 * @return void* Ponteiro para a instância da plataforma
 */
void* emu_frontend_get_platform_instance(void) {
    return g_frontend_state.platform_instance;
}
