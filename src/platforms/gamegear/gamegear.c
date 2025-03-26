/**
 * @file gamegear.c
 * @brief Implementação do emulador Game Gear
 */

#include "gamegear.h"
#include "../../core/save_state.h"
#include "../../utils/enhanced_log.h"
#include "../../utils/log_categories.h"
#include "../mastersystem/audio/sms_psg.h"
#include "../mastersystem/cpu/z80_adapter.h"
#include "../mastersystem/memory/sms_memory.h"
#include "../mastersystem/video/sms_vdp.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG EMU_LOG_CAT_CORE

// Macros de log
#define GG_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG, __VA_ARGS__)
#define GG_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG, __VA_ARGS__)
#define GG_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG, __VA_ARGS__)
#define GG_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG, __VA_ARGS__)
#define GG_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG, __VA_ARGS__)

// Constantes do Game Gear
#define GG_SCREEN_WIDTH 160
#define GG_SCREEN_HEIGHT 144
#define GG_AUDIO_BUFFER_SIZE 2048
#define GG_CYCLES_PER_LINE 228
#define GG_LINES_PER_FRAME 262

/**
 * @brief Estrutura principal do Game Gear
 */
struct gamegear_t {
    // Componentes do sistema
    sms_memory_t *memory;       // Sistema de memória
    sms_vdp_t *vdp;            // Video Display Processor
    sms_psg_t *psg;            // Programmable Sound Generator
    sms_z80_adapter_t *cpu;     // Adaptador do Z80

    // Estado do sistema
    bool is_running;            // Sistema está rodando
    bool is_paused;             // Sistema está pausado

    // Buffers
    uint16_t *video_buffer;     // Buffer de vídeo (formato RGB565)
    int16_t *audio_buffer;      // Buffer de áudio
    size_t audio_buffer_size;   // Tamanho do buffer de áudio

    // Input
    uint8_t input_ports[2];     // Estado das portas de input

    // Configuração
    config_t *config;           // Configuração do emulador

    // Portas de controle específicas do Game Gear
    uint8_t lcd_control;        // Controle do LCD
    uint8_t stereo_control;     // Controle estéreo
    uint8_t lcd_contrast;       // Nível de contraste
    uint8_t power_save_mode;    // Modo de economia de energia

    // Adaptador de Master System
    gg_sms_adapter_t sms_adapter; // Configuração do adaptador SMS
};

/**
 * @brief Cria uma nova instância do emulador Game Gear
 */
gamegear_t *gamegear_create(config_t *config) {
    if (!config) {
        GG_LOG_ERROR("Configuração inválida");
        return NULL;
    }

    gamegear_t *gg = (gamegear_t *)malloc(sizeof(gamegear_t));
    if (!gg) {
        GG_LOG_ERROR("Falha ao alocar memória para Game Gear");
        return NULL;
    }

    // Inicializa a estrutura
    memset(gg, 0, sizeof(gamegear_t));
    gg->config = config;

    // Cria componentes
    gg->memory = sms_memory_create();
    gg->vdp = sms_vdp_create();
    gg->psg = sms_psg_create();
    gg->cpu = sms_z80_adapter_create();

    if (!gg->memory || !gg->vdp || !gg->psg || !gg->cpu) {
        GG_LOG_ERROR("Falha ao criar componentes do Game Gear");
        gamegear_destroy(gg);
        return NULL;
    }

    // Configura VDP para modo Game Gear
    sms_vdp_set_mode(gg->vdp, VDP_MODE_GG);

    // Aloca buffers
    gg->video_buffer = (uint16_t *)malloc(GG_SCREEN_WIDTH * GG_SCREEN_HEIGHT * sizeof(uint16_t));
    gg->audio_buffer = (int16_t *)malloc(GG_AUDIO_BUFFER_SIZE * sizeof(int16_t));

    if (!gg->video_buffer || !gg->audio_buffer) {
        GG_LOG_ERROR("Falha ao alocar buffers");
        gamegear_destroy(gg);
        return NULL;
    }

    gg->audio_buffer_size = GG_AUDIO_BUFFER_SIZE;

    // Conecta componentes
    sms_z80_adapter_connect(gg->cpu, gg->memory, gg->vdp, gg->psg, NULL);

    // Configura timing
    sms_z80_adapter_set_timing(gg->cpu, false); // NTSC

    // Inicializa controles específicos do Game Gear
    gg->lcd_control = GG_LCD_BACKLIGHT_ON | GG_LCD_ENABLE | GG_LCD_NORMAL_MODE;
    gg->stereo_control = 0xFF; // Ambos os canais ativos
    gg->lcd_contrast = 16;     // Contraste médio (0-31)
    gg->power_save_mode = 0;   // Sem economia de energia

    // Inicializa adaptador SMS
    gg->sms_adapter.enabled = false;
    gg->sms_adapter.force_sms_mode = false;
    gg->sms_adapter.stretch_display = false;
    gg->sms_adapter.apply_palette_filter = true;

    GG_LOG_INFO("Game Gear criado com sucesso");
    return gg;
}

/**
 * @brief Destrói uma instância do emulador Game Gear
 */
void gamegear_destroy(gamegear_t *gg) {
    if (!gg) return;

    // Libera componentes
    if (gg->cpu) sms_z80_adapter_destroy(gg->cpu);
    if (gg->vdp) sms_vdp_destroy(gg->vdp);
    if (gg->psg) sms_psg_destroy(gg->psg);
    if (gg->memory) sms_memory_destroy(gg->memory);

    // Libera buffers
    free(gg->video_buffer);
    free(gg->audio_buffer);

    free(gg);
    GG_LOG_INFO("Game Gear destruído");
}

/**
 * @brief Reseta o emulador Game Gear
 */
void gamegear_reset(gamegear_t *gg) {
    if (!gg) return;

    sms_z80_adapter_reset(gg->cpu);
    sms_vdp_reset(gg->vdp);
    sms_psg_reset(gg->psg);
    sms_memory_reset(gg->memory);

    memset(gg->input_ports, 0, sizeof(gg->input_ports));
    gg->is_paused = false;

    // Reseta controles específicos do Game Gear
    gg->lcd_control = GG_LCD_BACKLIGHT_ON | GG_LCD_ENABLE | GG_LCD_NORMAL_MODE;
    gg->stereo_control = 0xFF;
    gg->lcd_contrast = 16;
    gg->power_save_mode = 0;

    GG_LOG_INFO("Game Gear resetado");
}

/**
 * @brief Carrega uma ROM no Game Gear
 */
bool gamegear_load_rom(gamegear_t *gg, const uint8_t *rom_data, size_t rom_size) {
    if (!gg || !rom_data || !rom_size) {
        GG_LOG_ERROR("Parâmetros inválidos para carregar ROM");
        return false;
    }

    // Reseta o sistema antes de carregar nova ROM
    gamegear_reset(gg);

    // Carrega a ROM na memória
    if (!sms_memory_load_rom(gg->memory, rom_data, rom_size)) {
        GG_LOG_ERROR("Falha ao carregar ROM");
        return false;
    }

    GG_LOG_INFO("ROM carregada com sucesso (tamanho: %zu bytes)", rom_size);
    return true;
}

/**
 * @brief Executa um frame do Game Gear
 */
void gamegear_run_frame(gamegear_t *gg) {
    if (!gg || gg->is_paused) return;

    // Executa um frame completo
    for (int line = 0; line < GG_LINES_PER_FRAME; line++) {
        // Executa ciclos da CPU para uma linha
        for (int cycles = 0; cycles < GG_CYCLES_PER_LINE; cycles++) {
            sms_z80_adapter_update(gg->cpu, 1);
        }

        // Atualiza VDP para a linha atual
        sms_vdp_update_line(gg->vdp);
    }

    // Atualiza PSG e gera samples de áudio
    sms_psg_end_frame(gg->psg);

    // Copia buffer de vídeo do VDP
    const uint16_t *vdp_buffer = sms_vdp_get_screen_buffer(gg->vdp);
    memcpy(gg->video_buffer, vdp_buffer, GG_SCREEN_WIDTH * GG_SCREEN_HEIGHT * sizeof(uint16_t));
}

/**
 * @brief Obtém o buffer de vídeo atual
 */
const uint16_t *gamegear_get_video_buffer(gamegear_t *gg) {
    return gg ? gg->video_buffer : NULL;
}

/**
 * @brief Obtém o buffer de áudio atual
 */
const int16_t *gamegear_get_audio_buffer(gamegear_t *gg, size_t *size) {
    if (!gg || !size) return NULL;
    *size = gg->audio_buffer_size;
    return gg->audio_buffer;
}

/**
 * @brief Processa input do Game Gear
 */
void gamegear_set_input(gamegear_t *gg, int port, uint8_t value) {
    if (!gg || port < 0 || port > 1) return;
    gg->input_ports[port] = value;
}

/**
 * @brief Registra o estado para save state
 */
int gamegear_register_save_state(gamegear_t *gg, save_state_t *state) {
    if (!gg || !state) return -1;

    // Registra estado dos componentes
    if (sms_z80_adapter_register_save_state(gg->cpu, state) < 0 ||
        sms_vdp_register_save_state(gg->vdp, state) < 0 ||
        sms_psg_register_save_state(gg->psg, state) < 0 ||
        sms_memory_register_save_state(gg->memory, state) < 0) {
        return -1;
    }

    // Registra estado do Game Gear
    save_state_register_field(state, "gg_input_ports", gg->input_ports,
                            sizeof(gg->input_ports));
    save_state_register_field(state, "gg_is_paused", &gg->is_paused,
                            sizeof(gg->is_paused));
    save_state_register_field(state, "gg_lcd_control", &gg->lcd_control, sizeof(gg->lcd_control));
    save_state_register_field(state, "gg_stereo_control", &gg->stereo_control, sizeof(gg->stereo_control));
    save_state_register_field(state, "gg_lcd_contrast", &gg->lcd_contrast, sizeof(gg->lcd_contrast));
    save_state_register_field(state, "gg_power_save_mode", &gg->power_save_mode, sizeof(gg->power_save_mode));
    save_state_register_field(state, "gg_sms_adapter", &gg->sms_adapter, sizeof(gg->sms_adapter));

    return 0;
}

/**
 * @brief Atualiza o estado após carregar um save state
 */
void gamegear_update_state(gamegear_t *gg) {
    if (!gg) return;

    sms_z80_adapter_update_state(gg->cpu);
    sms_vdp_update_state(gg->vdp);
    sms_psg_update_state(gg->psg);
    sms_memory_update_state(gg->memory);

    GG_LOG_INFO("Estado do Game Gear atualizado");
}

/**
 * @brief Pausa ou retoma a emulação
 */
void gamegear_set_paused(gamegear_t *gg, bool paused) {
    if (!gg) return;
    gg->is_paused = paused;
    GG_LOG_INFO("Game Gear %s", paused ? "pausado" : "retomado");
}

/**
 * @brief Verifica se o emulador está pausado
 */
bool gamegear_is_paused(gamegear_t *gg) {
    if (!gg) return true;
    return gg->is_paused;
}

/**
 * @brief Configura o nível de contraste do LCD
 */
void gamegear_set_lcd_contrast(gamegear_t *gg, uint8_t level) {
    if (!gg) return;

    // Limita o contraste a 0-31
    if (level > 31) level = 31;

    gg->lcd_contrast = level;
    GG_LOG_DEBUG("Contraste do LCD configurado para %d", level);

    // Aplica efeito visual no buffer de vídeo com base no contraste
    // (quanto menor o contraste, mais escura fica a imagem)
    // Este efeito é apenas visual e não afeta o comportamento real do hardware
    if (gg->video_buffer) {
        float contrast_factor = (float)level / 31.0f;

        // Aplica contraste apenas se o LCD estiver ligado
        if (gg->lcd_control & GG_LCD_ENABLE) {
            for (int i = 0; i < GG_SCREEN_WIDTH * GG_SCREEN_HEIGHT; i++) {
                uint16_t color = gg->video_buffer[i];

                // Extrai componentes RGB565
                uint8_t r = (color >> 11) & 0x1F;
                uint8_t g = (color >> 5) & 0x3F;
                uint8_t b = color & 0x1F;

                // Aplica contraste
                r = (uint8_t)((float)r * contrast_factor);
                g = (uint8_t)((float)g * contrast_factor);
                b = (uint8_t)((float)b * contrast_factor);

                // Recompõe a cor
                gg->video_buffer[i] = (r << 11) | (g << 5) | b;
            }
        }
    }
}

/**
 * @brief Configura o modo de economia de energia
 */
void gamegear_set_power_mode(gamegear_t *gg, uint8_t mode) {
    if (!gg) return;

    uint8_t old_mode = gg->power_save_mode;
    gg->power_save_mode = mode;

    GG_LOG_DEBUG("Modo de economia de energia configurado: %02X", mode);

    // Implementa efeitos reais de economia de energia
    if ((mode & GG_POWER_LCD_OFF) && !(old_mode & GG_POWER_LCD_OFF)) {
        // Desliga o LCD
        gg->lcd_control &= ~GG_LCD_ENABLE;
        GG_LOG_INFO("LCD desligado para economia de energia");
    } else if (!(mode & GG_POWER_LCD_OFF) && (old_mode & GG_POWER_LCD_OFF)) {
        // Liga o LCD
        gg->lcd_control |= GG_LCD_ENABLE;
        GG_LOG_INFO("LCD ligado");
    }

    if ((mode & GG_POWER_PSG_OFF) && !(old_mode & GG_POWER_PSG_OFF)) {
        // Silencia o PSG
        if (gg->psg) {
            // Não há uma API direta para silenciar o PSG, então usamos volumes máximos
            // O valor 0x0F é o máximo de atenuação no PSG (silêncio)
            for (int i = 0; i < 4; i++) {
                sms_psg_write_port(gg->psg, 0x90 | (i << 5) | 0x0F);
            }
        }
        GG_LOG_INFO("PSG silenciado para economia de energia");
    }

    if ((mode & GG_POWER_Z80_SLOW) != (old_mode & GG_POWER_Z80_SLOW)) {
        // Altera a velocidade do Z80
        // Em um hardware real, isso reduziria o clock para economizar energia
        // No emulador, simplesmente registramos a mudança
        GG_LOG_INFO("Modo de clock Z80: %s",
                   (mode & GG_POWER_Z80_SLOW) ? "Reduzido" : "Normal");
    }

    if ((mode & GG_POWER_DEEP_SLEEP) && !(old_mode & GG_POWER_DEEP_SLEEP)) {
        // Entra em modo de sono profundo
        gg->is_paused = true;
        GG_LOG_INFO("Entrando em modo de sono profundo");
    } else if (!(mode & GG_POWER_DEEP_SLEEP) && (old_mode & GG_POWER_DEEP_SLEEP)) {
        // Sai do modo de sono profundo
        gg->is_paused = false;
        GG_LOG_INFO("Saindo do modo de sono profundo");
    }
}

/**
 * @brief Configura o adaptador de Master System
 */
void gamegear_set_sms_adapter(gamegear_t *gg, const gg_sms_adapter_t *adapter) {
    if (!gg || !adapter) return;

    // Copia configuração do adaptador
    memcpy(&gg->sms_adapter, adapter, sizeof(gg_sms_adapter_t));

    GG_LOG_INFO("Adaptador SMS %s", gg->sms_adapter.enabled ? "ativado" : "desativado");

    if (gg->sms_adapter.enabled) {
        // Ajusta VDP para modo SMS se necessário
        if (gg->sms_adapter.force_sms_mode) {
            sms_vdp_set_mode(gg->vdp, VDP_MODE_SMS);
            GG_LOG_INFO("VDP configurado para modo SMS");
        }

        // Configura filtro de cores para jogos SMS
        // Isso faz com que jogos SMS que usam a paleta original de 64 cores
        // sejam exibidos corretamente na tela do Game Gear com sua paleta de 4096 cores
        if (gg->sms_adapter.apply_palette_filter) {
            sms_vdp_set_palette_filter(gg->vdp, true);
            GG_LOG_INFO("Filtro de paleta SMS ativado");
        } else {
            sms_vdp_set_palette_filter(gg->vdp, false);
            GG_LOG_INFO("Filtro de paleta SMS desativado");
        }
    } else {
        // Restaura modo Game Gear
        sms_vdp_set_mode(gg->vdp, VDP_MODE_GG);
        sms_vdp_set_palette_filter(gg->vdp, false);
        GG_LOG_INFO("VDP restaurado para modo Game Gear padrão");
    }
}
