/**
 * @file sdl_frontend_adapter.h
 * @brief Interface do adaptador SDL para o frontend do Mega_Emu
 */
#ifndef EMU_SDL_FRONTEND_ADAPTER_H
#define EMU_SDL_FRONTEND_ADAPTER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../gui/core/gui_manager.h"

/**
 * @brief Inicializa o adaptador SDL
 * 
 * @param title Título da janela
 * @param width Largura da janela
 * @param height Altura da janela
 * @return true se a inicialização foi bem-sucedida, false caso contrário
 */
bool sdl_frontend_init(const char* title, int32_t width, int32_t height);

/**
 * @brief Finaliza o adaptador SDL e libera recursos
 */
void sdl_frontend_shutdown(void);

/**
 * @brief Processa eventos SDL
 * 
 * @return true se deve continuar executando, false para sair
 */
bool sdl_frontend_process_events(void);

/**
 * @brief Atualiza o estado do frontend
 */
void sdl_frontend_update(void);

/**
 * @brief Renderiza o frontend
 */
void sdl_frontend_render(void);

/**
 * @brief Obtém o gerenciador de GUI
 * 
 * @return gui_manager_t Gerenciador de GUI
 */
gui_manager_t sdl_frontend_get_gui_manager(void);

/**
 * @brief Obtém o renderizador SDL
 * 
 * @return SDL_Renderer* Renderizador SDL
 */
SDL_Renderer* sdl_frontend_get_renderer(void);

/**
 * @brief Obtém a janela SDL
 * 
 * @return SDL_Window* Janela SDL
 */
SDL_Window* sdl_frontend_get_window(void);

/**
 * @brief Define a cor de fundo
 * 
 * @param r Componente vermelho (0-255)
 * @param g Componente verde (0-255)
 * @param b Componente azul (0-255)
 * @param a Componente alfa (0-255)
 */
void sdl_frontend_set_background_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

#endif /* EMU_SDL_FRONTEND_ADAPTER_H */
