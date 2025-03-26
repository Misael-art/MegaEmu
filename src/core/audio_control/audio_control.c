/**
 * @file audio_control.c
 * @brief Implementação do sistema de controle de canais de áudio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_control.h"
#include "../logging/log.h"
#include "../utils/file_utils.h"

#define MAX_CALLBACKS 8
#define WAVE_BUFFER_SIZE 512

// Estrutura para callbacks registrados
typedef struct {
    int id;
    mega_emu_audio_channel_callback_t callback;
    void* user_data;
    bool active;
} audio_callback_t;

// Estrutura de contexto global do sistema de controle de áudio
typedef struct {
    void* audio_interface;
    mega_emu_audio_platform_t platform;
    mega_emu_audio_channel_state_t channels[AUDIO_CHANNEL_COUNT];
    audio_callback_t callbacks[MAX_CALLBACKS];
    uint32_t callback_count;
    bool initialized;
    bool solo_active;
    mega_emu_audio_channel_t solo_channel;
} audio_control_context_t;

// Contexto global
static audio_control_context_t g_audio_ctx = {0};

// Tabela de nomes de canais por plataforma
static const struct {
    mega_emu_audio_platform_t platform;
    mega_emu_audio_channel_t channel;
    const char* name;
} g_channel_names[] = {
    // Nomes comuns
    {AUDIO_PLATFORM_GENERIC, AUDIO_CHANNEL_MASTER, "Master"},

    // Mega Drive
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_YM2612_FM1, "FM 1"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_YM2612_FM2, "FM 2"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_YM2612_FM3, "FM 3"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_YM2612_FM4, "FM 4"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_YM2612_FM5, "FM 5"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_YM2612_FM6, "FM 6"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_PSG1, "PSG 1"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_PSG2, "PSG 2"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_PSG3, "PSG 3"},
    {AUDIO_PLATFORM_MEGADRIVE, AUDIO_CHANNEL_PSG_NOISE, "PSG Noise"},

    // NES
    {AUDIO_PLATFORM_NES, AUDIO_CHANNEL_NES_PULSE1, "Pulse 1"},
    {AUDIO_PLATFORM_NES, AUDIO_CHANNEL_NES_PULSE2, "Pulse 2"},
    {AUDIO_PLATFORM_NES, AUDIO_CHANNEL_NES_TRIANGLE, "Triangle"},
    {AUDIO_PLATFORM_NES, AUDIO_CHANNEL_NES_NOISE, "Noise"},
    {AUDIO_PLATFORM_NES, AUDIO_CHANNEL_NES_DMC, "DMC"},

    // Master System
    {AUDIO_PLATFORM_MASTERSYSTEM, AUDIO_CHANNEL_SMS_PSG1, "PSG 1"},
    {AUDIO_PLATFORM_MASTERSYSTEM, AUDIO_CHANNEL_SMS_PSG2, "PSG 2"},
    {AUDIO_PLATFORM_MASTERSYSTEM, AUDIO_CHANNEL_SMS_PSG3, "PSG 3"},
    {AUDIO_PLATFORM_MASTERSYSTEM, AUDIO_CHANNEL_SMS_PSG_NOISE, "PSG Noise"},

    // Game Gear
    {AUDIO_PLATFORM_GAMEGEAR, AUDIO_CHANNEL_SMS_PSG1, "PSG 1"},
    {AUDIO_PLATFORM_GAMEGEAR, AUDIO_CHANNEL_SMS_PSG2, "PSG 2"},
    {AUDIO_PLATFORM_GAMEGEAR, AUDIO_CHANNEL_SMS_PSG3, "PSG 3"},
    {AUDIO_PLATFORM_GAMEGEAR, AUDIO_CHANNEL_SMS_PSG_NOISE, "PSG Noise"},

    // Game Boy
    {AUDIO_PLATFORM_GAMEBOY, AUDIO_CHANNEL_GB_PULSE1, "Pulse 1"},
    {AUDIO_PLATFORM_GAMEBOY, AUDIO_CHANNEL_GB_PULSE2, "Pulse 2"},
    {AUDIO_PLATFORM_GAMEBOY, AUDIO_CHANNEL_GB_WAVE, "Wave"},
    {AUDIO_PLATFORM_GAMEBOY, AUDIO_CHANNEL_GB_NOISE, "Noise"},

    // SNES
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE1, "Voice 1"},
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE2, "Voice 2"},
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE3, "Voice 3"},
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE4, "Voice 4"},
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE5, "Voice 5"},
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE6, "Voice 6"},
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE7, "Voice 7"},
    {AUDIO_PLATFORM_SNES, AUDIO_CHANNEL_SNES_VOICE8, "Voice 8"},

    // Fim da tabela
    {AUDIO_PLATFORM_GENERIC, AUDIO_CHANNEL_COUNT, NULL}
};

// Protótipos de funções auxiliares
static void initialize_channels_for_platform(mega_emu_audio_platform_t platform);
static void notify_channel_change(mega_emu_audio_channel_t channel, bool enabled);
static bool is_valid_channel(mega_emu_audio_channel_t channel);
static bool is_channel_available(mega_emu_audio_platform_t platform, mega_emu_audio_channel_t channel);

// Inicialização e finalização
bool mega_emu_audio_control_init(void* audio, mega_emu_audio_platform_t platform) {
    // Já inicializado?
    if (g_audio_ctx.initialized) {
        LOG_WARNING("Sistema de controle de áudio já inicializado.");
        return false;
    }

    // Verifica parâmetros
    if (!audio) {
        LOG_ERROR("Interface de áudio inválida.");
        return false;
    }

    // Inicializa contexto
    memset(&g_audio_ctx, 0, sizeof(audio_control_context_t));
    g_audio_ctx.audio_interface = audio;
    g_audio_ctx.platform = platform;

    // Inicializa canais de acordo com a plataforma
    initialize_channels_for_platform(platform);

    g_audio_ctx.initialized = true;
    LOG_INFO("Sistema de controle de áudio inicializado para plataforma %d", platform);

    return true;
}

void mega_emu_audio_control_shutdown(void) {
    if (!g_audio_ctx.initialized) {
        return;
    }

    // Libera recursos dos canais
    for (int i = 0; i < AUDIO_CHANNEL_COUNT; i++) {
        if (g_audio_ctx.channels[i].wave_buffer) {
            free(g_audio_ctx.channels[i].wave_buffer);
            g_audio_ctx.channels[i].wave_buffer = NULL;
        }
    }

    g_audio_ctx.initialized = false;
    LOG_INFO("Sistema de controle de áudio finalizado.");
}

// Gerenciamento de callbacks
int mega_emu_audio_control_register_callback(mega_emu_audio_channel_callback_t callback, void* user_data) {
    if (!g_audio_ctx.initialized || !callback) {
        return -1;
    }

    // Procura por um slot livre
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!g_audio_ctx.callbacks[i].active) {
            g_audio_ctx.callbacks[i].id = i;
            g_audio_ctx.callbacks[i].callback = callback;
            g_audio_ctx.callbacks[i].user_data = user_data;
            g_audio_ctx.callbacks[i].active = true;
            g_audio_ctx.callback_count++;
            return i;
        }
    }

    LOG_ERROR("Não foi possível registrar callback, limite atingido.");
    return -1;
}

bool mega_emu_audio_control_unregister_callback(int callback_id) {
    if (!g_audio_ctx.initialized || callback_id < 0 || callback_id >= MAX_CALLBACKS) {
        return false;
    }

    if (!g_audio_ctx.callbacks[callback_id].active) {
        return false;
    }

    g_audio_ctx.callbacks[callback_id].active = false;
    g_audio_ctx.callback_count--;
    return true;
}

// Gerenciamento de canais
bool mega_emu_audio_control_set_channel_enabled(mega_emu_audio_channel_t channel, bool enabled) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return false;
    }

    g_audio_ctx.channels[channel].enabled = enabled;
    notify_channel_change(channel, enabled);

    // Caso especial para o master
    if (channel == AUDIO_CHANNEL_MASTER) {
        // Quando o master é desabilitado, todos os canais ficam efetivamente desabilitados
        // mas mantemos seus estados individuais intactos
        LOG_INFO("Canal Master %s", enabled ? "habilitado" : "desabilitado");
    } else {
        LOG_INFO("Canal %s %s", g_audio_ctx.channels[channel].name,
                enabled ? "habilitado" : "desabilitado");
    }

    return true;
}

bool mega_emu_audio_control_is_channel_enabled(mega_emu_audio_channel_t channel) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return false;
    }

    // Leva em consideração o estado do canal master
    if (!g_audio_ctx.channels[AUDIO_CHANNEL_MASTER].enabled && channel != AUDIO_CHANNEL_MASTER) {
        return false;
    }

    return g_audio_ctx.channels[channel].enabled;
}

bool mega_emu_audio_control_set_channel_volume(mega_emu_audio_channel_t channel, uint8_t volume) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return false;
    }

    g_audio_ctx.channels[channel].volume = volume;

    LOG_INFO("Volume do canal %s definido para %d",
            g_audio_ctx.channels[channel].name, volume);

    return true;
}

uint8_t mega_emu_audio_control_get_channel_volume(mega_emu_audio_channel_t channel) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return 0;
    }

    return g_audio_ctx.channels[channel].volume;
}

bool mega_emu_audio_control_set_channel_muted(mega_emu_audio_channel_t channel, bool muted) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return false;
    }

    g_audio_ctx.channels[channel].muted = muted;

    LOG_INFO("Canal %s %s", g_audio_ctx.channels[channel].name,
            muted ? "silenciado" : "não silenciado");

    return true;
}

bool mega_emu_audio_control_is_channel_muted(mega_emu_audio_channel_t channel) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return false;
    }

    return g_audio_ctx.channels[channel].muted;
}

bool mega_emu_audio_control_set_channel_solo(mega_emu_audio_channel_t channel, bool solo) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return false;
    }

    // Desativa solo em todos os canais primeiro
    if (solo) {
        for (int i = 0; i < AUDIO_CHANNEL_COUNT; i++) {
            g_audio_ctx.channels[i].solo = false;
        }
    }

    g_audio_ctx.channels[channel].solo = solo;
    g_audio_ctx.solo_active = solo;
    g_audio_ctx.solo_channel = channel;

    LOG_INFO("Canal %s %s", g_audio_ctx.channels[channel].name,
            solo ? "em modo solo" : "não mais em modo solo");

    return true;
}

bool mega_emu_audio_control_is_channel_solo(mega_emu_audio_channel_t channel) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel)) {
        return false;
    }

    return g_audio_ctx.channels[channel].solo;
}

bool mega_emu_audio_control_get_channel_state(mega_emu_audio_channel_t channel,
                                             mega_emu_audio_channel_state_t* state) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel) || !state) {
        return false;
    }

    memcpy(state, &g_audio_ctx.channels[channel], sizeof(mega_emu_audio_channel_state_t));
    return true;
}

bool mega_emu_audio_control_update_wave_buffer(mega_emu_audio_channel_t channel,
                                             const int16_t* samples,
                                             uint32_t num_samples) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel) ||
        !is_channel_available(g_audio_ctx.platform, channel) || !samples || num_samples == 0) {
        return false;
    }

    // Verifica se o buffer de forma de onda já foi criado
    if (!g_audio_ctx.channels[channel].wave_buffer) {
        g_audio_ctx.channels[channel].wave_buffer = (int16_t*)malloc(WAVE_BUFFER_SIZE * sizeof(int16_t));
        if (!g_audio_ctx.channels[channel].wave_buffer) {
            LOG_ERROR("Falha ao alocar buffer de forma de onda para canal %s.",
                    g_audio_ctx.channels[channel].name);
            return false;
        }

        g_audio_ctx.channels[channel].buffer_size = WAVE_BUFFER_SIZE;
        g_audio_ctx.channels[channel].buffer_pos = 0;
    }

    // Copia os novos samples para o buffer (circular)
    for (uint32_t i = 0; i < num_samples; i++) {
        g_audio_ctx.channels[channel].wave_buffer[g_audio_ctx.channels[channel].buffer_pos] = samples[i];
        g_audio_ctx.channels[channel].buffer_pos =
            (g_audio_ctx.channels[channel].buffer_pos + 1) % g_audio_ctx.channels[channel].buffer_size;
    }

    g_audio_ctx.channels[channel].is_active = true;
    return true;
}

const char* mega_emu_audio_control_get_channel_name(mega_emu_audio_channel_t channel) {
    if (!g_audio_ctx.initialized || !is_valid_channel(channel)) {
        return NULL;
    }

    return g_audio_ctx.channels[channel].name;
}

uint32_t mega_emu_audio_control_get_available_channels(mega_emu_audio_channel_t* channels,
                                                     uint32_t max_channels) {
    if (!g_audio_ctx.initialized || !channels || max_channels == 0) {
        return 0;
    }

    uint32_t count = 0;

    // Sempre inclui o canal master
    if (count < max_channels) {
        channels[count++] = AUDIO_CHANNEL_MASTER;
    }

    // Adiciona canais específicos da plataforma
    for (int i = 1; i < AUDIO_CHANNEL_COUNT && count < max_channels; i++) {
        if (is_channel_available(g_audio_ctx.platform, i)) {
            channels[count++] = i;
        }
    }

    return count;
}

bool mega_emu_audio_control_reset_all_channels(void) {
    if (!g_audio_ctx.initialized) {
        return false;
    }

    // Reseta para valores padrão
    initialize_channels_for_platform(g_audio_ctx.platform);

    LOG_INFO("Todos os canais de áudio resetados para valores padrão.");
    return true;
}

// Funções auxiliares

static void initialize_channels_for_platform(mega_emu_audio_platform_t platform) {
    // Inicializa todos os canais com valores padrão
    for (int i = 0; i < AUDIO_CHANNEL_COUNT; i++) {
        g_audio_ctx.channels[i].id = i;
        g_audio_ctx.channels[i].enabled = true;
        g_audio_ctx.channels[i].volume = 255;
        g_audio_ctx.channels[i].muted = false;
        g_audio_ctx.channels[i].solo = false;
        g_audio_ctx.channels[i].is_active = false;
        g_audio_ctx.channels[i].wave_buffer = NULL;
        g_audio_ctx.channels[i].buffer_size = 0;
        g_audio_ctx.channels[i].buffer_pos = 0;

        // Busca o nome do canal na tabela
        const char* name = NULL;
        for (int j = 0; g_channel_names[j].name != NULL; j++) {
            if ((g_channel_names[j].platform == platform || g_channel_names[j].platform == AUDIO_PLATFORM_GENERIC) &&
                g_channel_names[j].channel == i) {
                name = g_channel_names[j].name;
                break;
            }
        }

        if (name) {
            strncpy(g_audio_ctx.channels[i].name, name, sizeof(g_audio_ctx.channels[i].name) - 1);
        } else {
            snprintf(g_audio_ctx.channels[i].name, sizeof(g_audio_ctx.channels[i].name), "Canal %d", i);
        }
    }

    g_audio_ctx.solo_active = false;
}

static void notify_channel_change(mega_emu_audio_channel_t channel, bool enabled) {
    // Notifica todos os callbacks registrados
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (g_audio_ctx.callbacks[i].active && g_audio_ctx.callbacks[i].callback) {
            g_audio_ctx.callbacks[i].callback(channel, enabled, g_audio_ctx.callbacks[i].user_data);
        }
    }
}

static bool is_valid_channel(mega_emu_audio_channel_t channel) {
    return (channel >= 0 && channel < AUDIO_CHANNEL_COUNT);
}

static bool is_channel_available(mega_emu_audio_platform_t platform, mega_emu_audio_channel_t channel) {
    // O canal Master está sempre disponível
    if (channel == AUDIO_CHANNEL_MASTER) {
        return true;
    }

    // Verifica se o canal é compatível com a plataforma atual
    switch (platform) {
        case AUDIO_PLATFORM_MEGADRIVE:
            return (channel >= AUDIO_CHANNEL_YM2612_FM1 && channel <= AUDIO_CHANNEL_PSG_NOISE);

        case AUDIO_PLATFORM_MASTERSYSTEM:
        case AUDIO_PLATFORM_GAMEGEAR:
            return (channel >= AUDIO_CHANNEL_SMS_PSG1 && channel <= AUDIO_CHANNEL_SMS_PSG_NOISE);

        case AUDIO_PLATFORM_NES:
            return (channel >= AUDIO_CHANNEL_NES_PULSE1 && channel <= AUDIO_CHANNEL_NES_DMC);

        case AUDIO_PLATFORM_SNES:
            return (channel >= AUDIO_CHANNEL_SNES_VOICE1 && channel <= AUDIO_CHANNEL_SNES_VOICE8);

        case AUDIO_PLATFORM_GAMEBOY:
        case AUDIO_PLATFORM_GAMEBOY_COLOR:
            return (channel >= AUDIO_CHANNEL_GB_PULSE1 && channel <= AUDIO_CHANNEL_GB_NOISE);

        case AUDIO_PLATFORM_GENERIC:
            return true;

        default:
            return false;
    }
}

// Funcionalidades pendentes
bool mega_emu_audio_control_save_config(const char* filename) {
    // TO DO: Implementar salvamento de configuração
    return false;
}

bool mega_emu_audio_control_load_config(const char* filename) {
    // TO DO: Implementar carregamento de configuração
    return false;
}
