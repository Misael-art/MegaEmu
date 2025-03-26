#ifndef EMU_LOG_CATEGORIES_H
#define EMU_LOG_CATEGORIES_H

#include <stdint.h>

/**
 * @file log_categories.h
 * @brief Definição das categorias de log do emulador
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Categorias de log disponíveis
 */
typedef enum {
  EMU_LOG_CAT_CORE = 0,     // Core do emulador
  EMU_LOG_CAT_CPU,          // CPU (Z80)
  EMU_LOG_CAT_VDP,          // Video Display Processor
  EMU_LOG_CAT_PSG,          // Programmable Sound Generator
  EMU_LOG_CAT_MEMORY,       // Sistema de memória
  EMU_LOG_CAT_INPUT,        // Sistema de input
  EMU_LOG_CAT_AUDIO,        // Sistema de áudio
  EMU_LOG_CAT_VIDEO,        // Sistema de vídeo
  EMU_LOG_CAT_CONFIG,       // Sistema de configuração
  EMU_LOG_CAT_SAVE_STATE,   // Sistema de save states
  EMU_LOG_CAT_CARTRIDGE,    // Sistema de cartuchos
  EMU_LOG_CAT_DEBUGGER,     // Debugger
  EMU_LOG_CAT_UI,           // Interface do usuário
  EMU_LOG_CAT_SHADER,       // Sistema de shaders
  EMU_LOG_CAT_REWIND,       // Sistema de rewind
  EMU_LOG_CAT_CHEATS,       // Sistema de cheats
  EMU_LOG_CAT_NETPLAY,      // Sistema de netplay
  EMU_LOG_CAT_ACHIEVEMENTS, // Sistema de achievements
  EMU_LOG_CAT_PLUGINS,      // Sistema de plugins
  EMU_LOG_CAT_SCRIPTING,    // Sistema de scripting
  EMU_LOG_CAT_COUNT         // Número total de categorias
} emu_log_category_t;

/* String representations of log categories */
static const char *EMU_LOG_CATEGORY_STRINGS[EMU_LOG_CAT_COUNT] = {
    "CORE",      "CPU",      "VDP",          "PSG",     "MEMORY",
    "INPUT",     "AUDIO",    "VIDEO",        "CONFIG",  "SAVE_STATE",
    "CARTRIDGE", "DEBUGGER", "UI",           "SHADER",  "REWIND",
    "CHEATS",    "NETPLAY",  "ACHIEVEMENTS", "PLUGINS", "SCRIPTING"};

#ifdef __cplusplus
}
#endif

#endif /* EMU_LOG_CATEGORIES_H */
