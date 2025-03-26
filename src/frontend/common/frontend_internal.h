/**
 * @file frontend_internal.h
 * @brief Definições e estruturas internas do frontend
 *
 * Este arquivo contém definições internas utilizadas pelo sistema de frontend
 * que não fazem parte da API pública.
 */

#ifndef FRONTEND_INTERNAL_H
#define FRONTEND_INTERNAL_H

#include <stdbool.h>
#include "frontend.h"

/**
 * @brief Tipos de plataforma suportados
 */
typedef enum {
    EMU_PLATFORM_NONE = 0,
    EMU_PLATFORM_AUTO,
    EMU_PLATFORM_MEGA_DRIVE,
    EMU_PLATFORM_MASTER_SYSTEM,
    EMU_PLATFORM_GAME_GEAR,
    EMU_PLATFORM_NES,
    EMU_PLATFORM_SNES,
    EMU_PLATFORM_GAME_BOY,
    EMU_PLATFORM_GAME_BOY_COLOR,
    EMU_PLATFORM_GAME_BOY_ADVANCE,
    EMU_PLATFORM_ATARI_2600,
    EMU_PLATFORM_ATARI_7800,
    EMU_PLATFORM_COLECOVISION,
    EMU_PLATFORM_PC_ENGINE,
    EMU_PLATFORM_NEO_GEO,
    // Futuras plataformas...
} emu_platform_t;

/**
 * @brief Callbacks para implementações específicas de frontend
 */
typedef struct {
    // Callbacks de plataforma
    void* (*platform_init)(emu_platform_t platform);
    void (*platform_shutdown)(void* platform_instance);
    bool (*load_rom)(void* platform_instance, const char* rom_path);
    bool (*reset)(void* platform_instance);

    // Callbacks de detecção
    emu_platform_t (*detect_platform)(const char* rom_path);

    // Callbacks de estado
    void (*pause_changed)(bool paused);

    // Callbacks de ciclo de frame
    bool (*begin_frame)(void* platform_instance);
    bool (*end_frame)(void* platform_instance);

    // Callbacks de menu/UI
    bool (*show_message)(const char* message, int duration_ms);
    bool (*show_menu)(void);
} frontend_callbacks_t;

// Funções internas
bool emu_frontend_common_init(int type);
void emu_frontend_common_shutdown(void);
void emu_frontend_register_callbacks(const frontend_callbacks_t* callbacks);

// Funções de gerenciamento de diretórios
void emu_frontend_set_rom_directory(const char* directory);
const char* emu_frontend_get_rom_directory(void);
void emu_frontend_set_save_directory(const char* directory);
const char* emu_frontend_get_save_directory(void);
void emu_frontend_set_screenshots_directory(const char* directory);
const char* emu_frontend_get_screenshots_directory(void);
void emu_frontend_set_states_directory(const char* directory);
const char* emu_frontend_get_states_directory(void);

// Funções de verificação de estado
bool emu_frontend_is_initialized(void);
bool emu_frontend_is_running(void);
bool emu_frontend_is_paused(void);
void emu_frontend_set_paused(bool paused);

// Funções de carregamento de ROM
bool emu_frontend_load_rom(const char* rom_path, emu_platform_t platform);
void emu_frontend_unload_rom(void);
bool emu_frontend_reset_current_rom(void);

// Funções de informação
emu_platform_t emu_frontend_get_current_platform(void);
const char* emu_frontend_get_current_rom_path(void);
void* emu_frontend_get_platform_instance(void);

#endif /* FRONTEND_INTERNAL_H */
