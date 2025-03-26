/**
 * @file log_categories.c
 * @brief Implementação das categorias de log
 */

#include "log_categories.h"

// Nomes das categorias de log
const char *emu_log_category_names[EMU_LOG_CAT_COUNT] = {
    "CORE",         // Core do emulador
    "CPU",          // CPU (Z80)
    "VDP",          // Video Display Processor
    "PSG",          // Programmable Sound Generator
    "MEMORY",       // Sistema de memória
    "INPUT",        // Sistema de input
    "AUDIO",        // Sistema de áudio
    "VIDEO",        // Sistema de vídeo
    "CONFIG",       // Sistema de configuração
    "SAVE_STATE",   // Sistema de save states
    "CARTRIDGE",    // Sistema de cartuchos
    "DEBUGGER",     // Debugger
    "UI",           // Interface do usuário
    "SHADER",       // Sistema de shaders
    "REWIND",       // Sistema de rewind
    "CHEATS",       // Sistema de cheats
    "NETPLAY",      // Sistema de netplay
    "ACHIEVEMENTS", // Sistema de achievements
    "PLUGINS",      // Sistema de plugins
    "SCRIPTING"     // Sistema de scripting
};
