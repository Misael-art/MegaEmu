/**
 * @file platform_config.h
 * @brief Configurações comuns e específicas para plataformas
 */

#ifndef EMU_PLATFORM_CONFIG_H
#define EMU_PLATFORM_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configuração comum para todas as plataformas
 */
typedef struct
{
    bool enable_audio;
    bool enable_video;
    bool enable_savestate;
    int audio_sample_rate;
    int audio_buffer_size;
    int video_scale;
    bool use_vsync;
    bool enable_debug;
    bool enable_logging;
} emu_platform_config_t;

/**
 * @brief Configuração específica para Mega Drive
 */
typedef struct
{
    emu_platform_config_t common;
    bool enable_region_autodetect;
    bool use_genesis_palette;
    bool enable_ym2612_accurate;
    bool enable_z80;
} md_platform_config_t;

/**
 * @brief Configuração específica para Master System
 */
typedef struct
{
    emu_platform_config_t common;
    bool enable_gg_compat;    // Compatibilidade com Game Gear
    bool enable_mark3_compat; // Compatibilidade com SG-1000 Mark III
    bool enable_fm_sound;     // Suporte ao chip FM (Japão)
} sms_platform_config_t;

/**
 * @brief Configuração específica para NES
 */
typedef struct
{
    emu_platform_config_t common;
    bool enable_ntsc_filter;  // Filtro de vídeo NTSC
    bool enable_4player;      // Suporte para 4 jogadores
    bool enable_pal_timing;   // Timing PAL vs NTSC
    bool enable_dendy_compat; // Compatibilidade com clone Dendy
} nes_platform_config_t;

/**
 * @brief Configuração específica para SNES
 */
typedef struct
{
    emu_platform_config_t common;
    bool enable_superfx; // Suporte ao chip SuperFX
    bool enable_sa1;     // Suporte ao chip SA-1
    bool enable_dsp;     // Suporte aos chips DSP
    bool enable_cx4;     // Suporte ao chip CX4
    bool enable_s_dd1;   // Suporte ao chip S-DD1
    bool enable_srtc;    // Suporte ao chip S-RTC
    bool enable_obc1;    // Suporte ao chip OBC1
    bool enable_spc7110; // Suporte ao chip SPC7110
    bool enable_st010;   // Suporte ao chip ST010/ST011
    bool enable_bs_x;    // Suporte ao sattellaview BS-X
} snes_platform_config_t;

// Funções de configuração
emu_platform_config_t emu_get_default_config(void);
void emu_apply_config(emu_platform_config_t *config);

#endif // EMU_PLATFORM_CONFIG_H
