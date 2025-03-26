/**
 * @file turbo.c
 * @brief Implementação do sistema de turbo/autofire
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "turbo.h"
#include "../logging/log.h"
#include "../utils/file_utils.h"

#define MAX_TURBO_CONFIG 32
#define MAX_CALLBACKS 8
#define MAX_CONTROLLER_PORTS 8

// Estrutura para callbacks registrados
typedef struct {
    int id;
    mega_emu_turbo_callback_t callback;
    void* user_data;
    bool active;
} turbo_callback_t;

// Estrutura interna do sistema de turbo
typedef struct {
    void* input_interface;
    mega_emu_turbo_platform_t platform;
    mega_emu_turbo_config_t configs[MAX_TURBO_CONFIG];
    uint32_t config_count;
    turbo_callback_t callbacks[MAX_CALLBACKS];
    uint32_t callback_count;
    bool initialized;

    // Mapeamento de botões para a plataforma atual
    mega_emu_turbo_button_mapping_t button_mappings[TURBO_BUTTON_COUNT];
    uint32_t mapping_count;

    // Estado atual dos botões
    uint32_t input_state[MAX_CONTROLLER_PORTS];
    uint32_t previous_state[MAX_CONTROLLER_PORTS];
} turbo_context_t;

// Contexto global
static turbo_context_t g_turbo_ctx = {0};

// Tabela de mapeamento de botões por plataforma
static const mega_emu_turbo_button_mapping_t g_button_mappings[] = {
    // Mega Drive
    {TURBO_BUTTON_MD_A, 0x00000001},
    {TURBO_BUTTON_MD_B, 0x00000002},
    {TURBO_BUTTON_MD_C, 0x00000004},
    {TURBO_BUTTON_MD_X, 0x00000008},
    {TURBO_BUTTON_MD_Y, 0x00000010},
    {TURBO_BUTTON_MD_Z, 0x00000020},
    {TURBO_BUTTON_MD_START, 0x00000040},
    {TURBO_BUTTON_MD_MODE, 0x00000080},

    // NES
    {TURBO_BUTTON_NES_A, 0x00000001},
    {TURBO_BUTTON_NES_B, 0x00000002},
    {TURBO_BUTTON_NES_START, 0x00000008},
    {TURBO_BUTTON_NES_SELECT, 0x00000004},

    // Master System
    {TURBO_BUTTON_SMS_1, 0x00000001},
    {TURBO_BUTTON_SMS_2, 0x00000002},

    // SNES
    {TURBO_BUTTON_SNES_A, 0x00000001},
    {TURBO_BUTTON_SNES_B, 0x00000002},
    {TURBO_BUTTON_SNES_X, 0x00000004},
    {TURBO_BUTTON_SNES_Y, 0x00000008},
    {TURBO_BUTTON_SNES_L, 0x00000010},
    {TURBO_BUTTON_SNES_R, 0x00000020},
    {TURBO_BUTTON_SNES_START, 0x00000040},
    {TURBO_BUTTON_SNES_SELECT, 0x00000080},

    // Botões genéricos (D-pad)
    {TURBO_BUTTON_UP, 0x00010000},
    {TURBO_BUTTON_DOWN, 0x00020000},
    {TURBO_BUTTON_LEFT, 0x00040000},
    {TURBO_BUTTON_RIGHT, 0x00080000},

    // Fim da tabela
    {TURBO_BUTTON_NONE, 0}
};

// Tabela de valores para presets de velocidade em Hz
static const uint8_t g_speed_preset_values[] = {
    6,   // TURBO_SPEED_SLOW (5-6 Hz)
    12,  // TURBO_SPEED_MEDIUM (10-12 Hz)
    20,  // TURBO_SPEED_FAST (15-20 Hz)
    30   // TURBO_SPEED_ULTRA (30 Hz)
};

// Protótipos de funções auxiliares
static void init_button_mappings(mega_emu_turbo_platform_t platform);
static int find_config_index(mega_emu_turbo_button_t button, uint8_t port);
static void calculate_period(mega_emu_turbo_config_t* config);
static void notify_turbo_event(mega_emu_turbo_button_t button, bool state);
static void update_input_state(void);
static uint32_t get_input_button_mask(mega_emu_turbo_button_t button);

// Implementação das funções públicas
bool mega_emu_turbo_init(void* input, mega_emu_turbo_platform_t platform) {
    // Já inicializado?
    if (g_turbo_ctx.initialized) {
        LOG_WARNING("Sistema de turbo já inicializado.");
        return false;
    }

    // Verifica parâmetros
    if (!input) {
        LOG_ERROR("Interface de entrada inválida.");
        return false;
    }

    // Inicializa contexto
    memset(&g_turbo_ctx, 0, sizeof(turbo_context_t));
    g_turbo_ctx.input_interface = input;
    g_turbo_ctx.platform = platform;

    // Inicializa mapeamento de botões
    init_button_mappings(platform);

    g_turbo_ctx.initialized = true;
    LOG_INFO("Sistema de turbo inicializado para plataforma %d", platform);

    return true;
}

void mega_emu_turbo_shutdown(void) {
    if (!g_turbo_ctx.initialized) {
        return;
    }

    g_turbo_ctx.initialized = false;
    LOG_INFO("Sistema de turbo finalizado.");
}

bool mega_emu_turbo_set_config(const mega_emu_turbo_config_t* config) {
    if (!g_turbo_ctx.initialized || !config || config->button == TURBO_BUTTON_NONE) {
        return false;
    }

    if (config->controller_port >= MAX_CONTROLLER_PORTS) {
        LOG_ERROR("Porta de controle inválida: %d", config->controller_port);
        return false;
    }

    // Procura se já existe configuração para este botão/porta
    int index = find_config_index(config->button, config->controller_port);

    // Se não existir e não há espaço, falha
    if (index < 0) {
        if (g_turbo_ctx.config_count >= MAX_TURBO_CONFIG) {
            LOG_ERROR("Número máximo de configurações de turbo atingido.");
            return false;
        }

        // Adiciona nova configuração
        index = g_turbo_ctx.config_count++;
    }

    // Copia a configuração
    memcpy(&g_turbo_ctx.configs[index], config, sizeof(mega_emu_turbo_config_t));

    // Calcula o período baseado na velocidade
    calculate_period(&g_turbo_ctx.configs[index]);

    LOG_INFO("Configuração de turbo definida para botão %d, porta %d",
            config->button, config->controller_port);

    return true;
}

bool mega_emu_turbo_get_config(mega_emu_turbo_button_t button, uint8_t port, mega_emu_turbo_config_t* config) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE || !config) {
        return false;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0) {
        return false;
    }

    // Copia a configuração
    memcpy(config, &g_turbo_ctx.configs[index], sizeof(mega_emu_turbo_config_t));
    return true;
}

bool mega_emu_turbo_remove_config(mega_emu_turbo_button_t button, uint8_t port) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE) {
        return false;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0) {
        return false;
    }

    // Remove a configuração deslocando as demais
    for (uint32_t i = index; i < g_turbo_ctx.config_count - 1; i++) {
        memcpy(&g_turbo_ctx.configs[i], &g_turbo_ctx.configs[i + 1], sizeof(mega_emu_turbo_config_t));
    }

    g_turbo_ctx.config_count--;
    LOG_INFO("Configuração de turbo removida para botão %d, porta %d", button, port);

    return true;
}

bool mega_emu_turbo_set_enabled(mega_emu_turbo_button_t button, uint8_t port, bool enabled) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE) {
        return false;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0) {
        return false;
    }

    // Atualiza estado
    g_turbo_ctx.configs[index].enabled = enabled;
    LOG_INFO("Turbo %s para botão %d, porta %d",
            enabled ? "habilitado" : "desabilitado", button, port);

    return true;
}

bool mega_emu_turbo_is_enabled(mega_emu_turbo_button_t button, uint8_t port) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE) {
        return false;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0) {
        return false;
    }

    return g_turbo_ctx.configs[index].enabled;
}

bool mega_emu_turbo_set_speed(mega_emu_turbo_button_t button, uint8_t port,
                            mega_emu_turbo_speed_preset_t speed_preset, uint8_t custom_speed) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE) {
        return false;
    }

    // Verifica preset
    if (speed_preset >= TURBO_SPEED_CUSTOM && custom_speed == 0) {
        LOG_ERROR("Velocidade personalizada inválida para botão %d, porta %d", button, port);
        return false;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0) {
        return false;
    }

    // Atualiza velocidade
    g_turbo_ctx.configs[index].speed_preset = speed_preset;
    if (speed_preset == TURBO_SPEED_CUSTOM) {
        g_turbo_ctx.configs[index].custom_speed = custom_speed;
    }

    // Recalcula o período
    calculate_period(&g_turbo_ctx.configs[index]);

    LOG_INFO("Velocidade de turbo definida para botão %d, porta %d: %d Hz",
            button, port,
            (speed_preset == TURBO_SPEED_CUSTOM) ?
                custom_speed : g_speed_preset_values[speed_preset]);

    return true;
}

bool mega_emu_turbo_set_duty_cycle(mega_emu_turbo_button_t button, uint8_t port, uint8_t duty_cycle) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE) {
        return false;
    }

    // Verifica ciclo de trabalho
    if (duty_cycle > 100) {
        duty_cycle = 100;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0) {
        return false;
    }

    // Atualiza ciclo de trabalho
    g_turbo_ctx.configs[index].duty_cycle = duty_cycle;

    LOG_INFO("Ciclo de trabalho de turbo definido para botão %d, porta %d: %d%%",
            button, port, duty_cycle);

    return true;
}

bool mega_emu_turbo_set_mode(mega_emu_turbo_button_t button, uint8_t port, mega_emu_turbo_mode_t mode) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE) {
        return false;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0) {
        return false;
    }

    // Atualiza modo
    g_turbo_ctx.configs[index].mode = mode;

    const char* mode_str = "desconhecido";
    switch (mode) {
        case TURBO_MODE_TOGGLE: mode_str = "toggle"; break;
        case TURBO_MODE_PULSE: mode_str = "pulse"; break;
        case TURBO_MODE_HOLD: mode_str = "hold"; break;
    }

    LOG_INFO("Modo de turbo definido para botão %d, porta %d: %s", button, port, mode_str);

    return true;
}

uint32_t mega_emu_turbo_get_configured_buttons(mega_emu_turbo_button_t* buttons,
                                             uint8_t* ports,
                                             uint32_t max_buttons) {
    if (!g_turbo_ctx.initialized || !buttons || !ports || max_buttons == 0) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < g_turbo_ctx.config_count && count < max_buttons; i++) {
        buttons[count] = g_turbo_ctx.configs[i].button;
        ports[count] = g_turbo_ctx.configs[i].controller_port;
        count++;
    }

    return count;
}

int mega_emu_turbo_register_callback(mega_emu_turbo_callback_t callback, void* user_data) {
    if (!g_turbo_ctx.initialized || !callback) {
        return -1;
    }

    // Procura por um slot livre
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!g_turbo_ctx.callbacks[i].active) {
            g_turbo_ctx.callbacks[i].id = i;
            g_turbo_ctx.callbacks[i].callback = callback;
            g_turbo_ctx.callbacks[i].user_data = user_data;
            g_turbo_ctx.callbacks[i].active = true;
            g_turbo_ctx.callback_count++;
            return i;
        }
    }

    LOG_ERROR("Não foi possível registrar callback, limite atingido.");
    return -1;
}

bool mega_emu_turbo_unregister_callback(int callback_id) {
    if (!g_turbo_ctx.initialized || callback_id < 0 || callback_id >= MAX_CALLBACKS) {
        return false;
    }

    if (!g_turbo_ctx.callbacks[callback_id].active) {
        return false;
    }

    g_turbo_ctx.callbacks[callback_id].active = false;
    g_turbo_ctx.callback_count--;
    return true;
}

uint32_t mega_emu_turbo_process(float frame_time) {
    if (!g_turbo_ctx.initialized) {
        return 0;
    }

    // Atualiza o estado de entrada atual
    update_input_state();

    uint32_t processed_count = 0;

    // Processa cada configuração
    for (uint32_t i = 0; i < g_turbo_ctx.config_count; i++) {
        mega_emu_turbo_config_t* config = &g_turbo_ctx.configs[i];

        if (!config->enabled) {
            continue;
        }

        // Verifica se o botão está pressionado
        uint32_t button_mask = get_input_button_mask(config->button);
        uint8_t port = config->controller_port;

        bool pressed = (g_turbo_ctx.input_state[port] & button_mask) != 0;
        bool was_pressed = (g_turbo_ctx.previous_state[port] & button_mask) != 0;

        // Atualiza o estado interno do botão
        if (pressed != was_pressed) {
            config->button_pressed = pressed;
        }

        // Processa de acordo com o modo
        switch (config->mode) {
            case TURBO_MODE_TOGGLE:
                // Alterna entre pressionado e não pressionado automaticamente
                config->counter += (uint32_t)(frame_time * 1000.0f);

                if (config->counter >= config->period) {
                    config->counter = 0;
                    config->state = !config->state;
                    notify_turbo_event(config->button, config->state);
                }
                break;

            case TURBO_MODE_PULSE:
                // Pulsa apenas enquanto o botão está pressionado
                if (config->button_pressed) {
                    config->counter += (uint32_t)(frame_time * 1000.0f);

                    if (config->counter >= config->period) {
                        config->counter = 0;
                        config->state = !config->state;
                        notify_turbo_event(config->button, config->state);
                    }
                } else {
                    if (config->state) {
                        config->state = false;
                        notify_turbo_event(config->button, false);
                    }
                    config->counter = 0;
                }
                break;

            case TURBO_MODE_HOLD:
                // Mantém pressionado até soltar
                if (config->button_pressed) {
                    if (!config->state) {
                        config->state = true;
                        notify_turbo_event(config->button, true);
                    }
                } else {
                    if (config->state) {
                        config->state = false;
                        notify_turbo_event(config->button, false);
                    }
                }
                break;
        }

        processed_count++;
    }

    return processed_count;
}

bool mega_emu_turbo_reset_all(void) {
    if (!g_turbo_ctx.initialized) {
        return false;
    }

    // Reseta o estado de todos os botões
    for (uint32_t i = 0; i < g_turbo_ctx.config_count; i++) {
        g_turbo_ctx.configs[i].counter = 0;
        g_turbo_ctx.configs[i].state = false;
        g_turbo_ctx.configs[i].button_pressed = false;
    }

    LOG_INFO("Todos os estados de turbo resetados.");
    return true;
}

bool mega_emu_turbo_is_button_active(mega_emu_turbo_button_t button, uint8_t port) {
    if (!g_turbo_ctx.initialized || button == TURBO_BUTTON_NONE) {
        return false;
    }

    // Procura a configuração
    int index = find_config_index(button, port);
    if (index < 0 || !g_turbo_ctx.configs[index].enabled) {
        return false;
    }

    return g_turbo_ctx.configs[index].state;
}

// Implementação de funções auxiliares
static void init_button_mappings(mega_emu_turbo_platform_t platform) {
    memset(g_turbo_ctx.button_mappings, 0, sizeof(g_turbo_ctx.button_mappings));
    g_turbo_ctx.mapping_count = 0;

    // Copia os mapeamentos relevantes para a plataforma
    for (int i = 0; g_button_mappings[i].turbo_button != TURBO_BUTTON_NONE; i++) {
        mega_emu_turbo_button_t button = g_button_mappings[i].turbo_button;

        // Verifica se o botão é relevante para a plataforma
        bool relevant = false;

        // Botões genéricos são relevantes para todas as plataformas
        if (button >= TURBO_BUTTON_A && button <= TURBO_BUTTON_RIGHT) {
            relevant = true;
        }

        // Botões específicos são relevantes apenas para suas plataformas
        else {
            switch (platform) {
                case TURBO_PLATFORM_MEGADRIVE:
                    relevant = (button >= TURBO_BUTTON_MD_A && button <= TURBO_BUTTON_MD_MODE);
                    break;

                case TURBO_PLATFORM_NES:
                    relevant = (button >= TURBO_BUTTON_NES_A && button <= TURBO_BUTTON_NES_SELECT);
                    break;

                case TURBO_PLATFORM_MASTERSYSTEM:
                case TURBO_PLATFORM_GAMEGEAR:
                    relevant = (button >= TURBO_BUTTON_SMS_1 && button <= TURBO_BUTTON_SMS_2);
                    break;

                case TURBO_PLATFORM_SNES:
                    relevant = (button >= TURBO_BUTTON_SNES_A && button <= TURBO_BUTTON_SNES_SELECT);
                    break;

                case TURBO_PLATFORM_GENERIC:
                    relevant = true;
                    break;

                default:
                    relevant = false;
            }
        }

        if (relevant) {
            g_turbo_ctx.button_mappings[g_turbo_ctx.mapping_count].turbo_button = button;
            g_turbo_ctx.button_mappings[g_turbo_ctx.mapping_count].input_button_mask =
                g_button_mappings[i].input_button_mask;
            g_turbo_ctx.mapping_count++;
        }
    }
}

static int find_config_index(mega_emu_turbo_button_t button, uint8_t port) {
    for (uint32_t i = 0; i < g_turbo_ctx.config_count; i++) {
        if (g_turbo_ctx.configs[i].button == button &&
            g_turbo_ctx.configs[i].controller_port == port) {
            return (int)i;
        }
    }

    return -1;
}

static void calculate_period(mega_emu_turbo_config_t* config) {
    if (!config) {
        return;
    }

    // Calcula o período em ms com base na velocidade
    uint8_t speed_hz;

    if (config->speed_preset == TURBO_SPEED_CUSTOM) {
        speed_hz = config->custom_speed;
    } else {
        speed_hz = g_speed_preset_values[config->speed_preset];
    }

    // Período = 1000 ms / frequência Hz
    if (speed_hz > 0) {
        config->period = 1000 / speed_hz;
    } else {
        // Padrão para 10 Hz se for zero por algum motivo
        config->period = 100;
    }
}

static void notify_turbo_event(mega_emu_turbo_button_t button, bool state) {
    // Notifica todos os callbacks registrados
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (g_turbo_ctx.callbacks[i].active && g_turbo_ctx.callbacks[i].callback) {
            g_turbo_ctx.callbacks[i].callback(
                button, state, g_turbo_ctx.callbacks[i].user_data);
        }
    }
}

static void update_input_state(void) {
    // Copia o estado atual para o anterior
    memcpy(g_turbo_ctx.previous_state, g_turbo_ctx.input_state, sizeof(g_turbo_ctx.input_state));

    // Atualiza o estado atual (depende da implementação específica da interface de entrada)
    // Implementação simplificada apenas para exemplo
    for (uint8_t port = 0; port < MAX_CONTROLLER_PORTS; port++) {
        g_turbo_ctx.input_state[port] = 0; // TO DO: Obter estado real da interface de entrada
    }
}

static uint32_t get_input_button_mask(mega_emu_turbo_button_t button) {
    for (uint32_t i = 0; i < g_turbo_ctx.mapping_count; i++) {
        if (g_turbo_ctx.button_mappings[i].turbo_button == button) {
            return g_turbo_ctx.button_mappings[i].input_button_mask;
        }
    }

    return 0;
}

// Funções pendentes
bool mega_emu_turbo_save_config(const char* filename) {
    // TO DO: Implementar salvamento de configuração
    return false;
}

bool mega_emu_turbo_load_config(const char* filename) {
    // TO DO: Implementar carregamento de configuração
    return false;
}
