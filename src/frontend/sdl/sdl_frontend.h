/**
 * @file sdl_frontend.h
 * @brief Interface do frontend SDL
 */
#ifndef SDL_FRONTEND_H
#define SDL_FRONTEND_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

// Estrutura do frontend SDL
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    bool running;
    int scale;
} SDLFrontend;

// Funções do frontend SDL
bool sdl_frontend_init(SDLFrontend* frontend, const char* title, int width, int height, int scale);
void sdl_frontend_update(SDLFrontend* frontend, uint32_t* framebuffer, int width, int height);
bool sdl_frontend_handle_events(SDLFrontend* frontend);
void sdl_frontend_destroy(SDLFrontend* frontend);

#endif  // SDL_FRONTEND_H
