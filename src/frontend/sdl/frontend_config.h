/**
 * @file frontend_config.h
 * @brief Configuração específica para o frontend SDL
 */
#ifndef SDL_FRONTEND_CONFIG_H
#define SDL_FRONTEND_CONFIG_H

#include "frontend/common/frontend_config.h"
#include <stdbool.h>
#include <stdint.h>

// Configuração padrão específica do SDL
extern const emu_frontend_config_t SDL_DEFAULT_FRONTEND_CONFIG;

// Funções de configuração específicas do SDL
void sdl_frontend_config_init(void);
emu_frontend_config_t *sdl_frontend_get_config(void);
bool sdl_frontend_save_config(void);
bool sdl_frontend_load_config(void);

#endif // SDL_FRONTEND_CONFIG_H
