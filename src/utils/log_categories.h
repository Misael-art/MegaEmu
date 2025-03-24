#ifndef EMU_LOG_CATEGORIES_H
#define EMU_LOG_CATEGORIES_H

#include <stdint.h>

/**
 * @file log_categories.h
 * @brief Definição das categorias de log do emulador
 */

#ifdef __cplusplus
extern "C"
{
#endif

    /* Categorias de log */
    typedef enum
    {
        /* Categorias do sistema */
        EMU_LOG_CAT_CORE = 0,    /* Sistema principal */
        EMU_LOG_CAT_CPU,         /* CPU */
        EMU_LOG_CAT_MEMORY,      /* Memória */
        EMU_LOG_CAT_VIDEO,       /* Vídeo */
        EMU_LOG_CAT_AUDIO,       /* Áudio */
        EMU_LOG_CAT_INPUT,       /* Entrada */
        EMU_LOG_CAT_SAVE_STATE,  /* Save state */
        EMU_LOG_CAT_CONFIG,      /* Configuração */
        EMU_LOG_CAT_UTILS,       /* Utilitários */
        EMU_LOG_CAT_PERFORMANCE, /* Performance */
        EMU_LOG_CAT_GUI,         /* Interface gráfica */
        EMU_LOG_CAT_FRONTEND,    /* Frontend/UI */
        EMU_LOG_CAT_DEBUG,       /* Debugging */
        EMU_LOG_CAT_ERROR,       /* Error handling */
        /* Categorias específicas */
        EMU_LOG_CAT_ROM,       /* ROM */
        EMU_LOG_CAT_CARTRIDGE, /* Cartucho */
        EMU_LOG_CAT_DISK,      /* Disco */
        EMU_LOG_CAT_TAPE,      /* Fita */
        EMU_LOG_CAT_JOYSTICK,  /* Joystick */
        EMU_LOG_CAT_KEYBOARD,  /* Teclado */
        EMU_LOG_CAT_MOUSE,     /* Mouse */
        EMU_LOG_CAT_DEBUGGER,  /* Debugger */
        /* Plataformas */
        EMU_LOG_CAT_NES,       /* Nintendo Entertainment System */
        EMU_LOG_CAT_SNES,      /* Super Nintendo */
        EMU_LOG_CAT_MEGADRIVE, /* Sega Mega Drive/Genesis */
        EMU_LOG_CAT_SMS,       /* Sega Master System */
        EMU_LOG_CAT_APU,       /* Audio Processing Unit */
        /* Categoria de teste */
        EMU_LOG_CAT_IO,       /* Input/Output */
        EMU_LOG_CAT_PLATFORM, /* Platform specific */
        EMU_LOG_CAT_TEST,     /* Test category */
        /* Número máximo de categorias */
        EMU_LOG_CAT_MAX
    } emu_log_category_t;

    /* String representations of log categories */
    static const char *EMU_LOG_CATEGORY_STRINGS[EMU_LOG_CAT_MAX] = {
        "CORE",
        "CPU",
        "MEMORY",
        "VIDEO",
        "AUDIO",
        "INPUT",
        "SAVE_STATE",
        "CONFIG",
        "UTILS",
        "PERFORMANCE",
        "GUI",
        "FRONTEND",
        "DEBUG",
        "ERROR",
        "ROM",
        "CARTRIDGE",
        "DISK",
        "TAPE",
        "JOYSTICK",
        "KEYBOARD",
        "MOUSE",
        "DEBUGGER",
        "NES",
        "SNES",
        "MEGADRIVE",
        "SMS",
        "APU",
        "IO",
        "PLATFORM",
        "TEST"};

#ifdef __cplusplus
}
#endif

#endif /* EMU_LOG_CATEGORIES_H */
