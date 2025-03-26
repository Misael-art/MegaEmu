/**
 * @file waveform_display.c
 * @brief Implementação da interface de exibição usando SDL2
 */

#include "waveform_display.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// Converte handle genérico para SDL_Window
#define TO_SDL_WINDOW(handle) ((SDL_Window *)(handle))

// Converte handle genérico para SDL_Renderer
#define TO_SDL_RENDERER(handle) ((SDL_Renderer *)(handle))

// Converte handle genérico para SDL_Texture
#define TO_SDL_TEXTURE(handle) ((SDL_Texture *)(handle))

bool waveform_display_init(void) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Erro ao inicializar SDL: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

void waveform_display_quit(void) { SDL_Quit(); }

waveform_display_t *waveform_display_create(uint32_t width, uint32_t height,
                                            const char *title) {
  waveform_display_t *display = calloc(1, sizeof(waveform_display_t));
  if (!display) {
    return NULL;
  }

  display->width = width;
  display->height = height;
  display->fullscreen = false;

  // Cria a janela SDL
  SDL_Window *window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!window) {
    fprintf(stderr, "Erro ao criar janela: %s\n", SDL_GetError());
    free(display);
    return NULL;
  }
  display->window_handle = window;

  // Cria o renderizador
  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer) {
    fprintf(stderr, "Erro ao criar renderizador: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    free(display);
    return NULL;
  }
  display->renderer_handle = renderer;

  // Cria a textura para o framebuffer
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_STREAMING, width, height);

  if (!texture) {
    fprintf(stderr, "Erro ao criar textura: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(display);
    return NULL;
  }
  display->texture_handle = texture;

  return display;
}

void waveform_display_destroy(waveform_display_t *display) {
  if (!display)
    return;

  if (display->texture_handle) {
    SDL_DestroyTexture(TO_SDL_TEXTURE(display->texture_handle));
  }
  if (display->renderer_handle) {
    SDL_DestroyRenderer(TO_SDL_RENDERER(display->renderer_handle));
  }
  if (display->window_handle) {
    SDL_DestroyWindow(TO_SDL_WINDOW(display->window_handle));
  }
  free(display);
}

void waveform_display_resize(waveform_display_t *display, uint32_t width,
                             uint32_t height) {
  if (!display)
    return;

  // Atualiza as dimensões
  display->width = width;
  display->height = height;

  // Recria a textura com as novas dimensões
  SDL_Texture *old_texture = TO_SDL_TEXTURE(display->texture_handle);
  SDL_Texture *new_texture = SDL_CreateTexture(
      TO_SDL_RENDERER(display->renderer_handle), SDL_PIXELFORMAT_RGBA32,
      SDL_TEXTUREACCESS_STREAMING, width, height);

  if (!new_texture) {
    fprintf(stderr, "Erro ao recriar textura: %s\n", SDL_GetError());
    return;
  }

  SDL_DestroyTexture(old_texture);
  display->texture_handle = new_texture;
}

void waveform_display_toggle_fullscreen(waveform_display_t *display) {
  if (!display)
    return;

  display->fullscreen = !display->fullscreen;
  SDL_SetWindowFullscreen(TO_SDL_WINDOW(display->window_handle),
                          display->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                              : 0);
}

void waveform_display_update(waveform_display_t *display,
                             const uint8_t *framebuffer, uint32_t width,
                             uint32_t height) {
  if (!display || !framebuffer)
    return;

  SDL_Texture *texture = TO_SDL_TEXTURE(display->texture_handle);
  SDL_Renderer *renderer = TO_SDL_RENDERER(display->renderer_handle);

  // Atualiza a textura com o novo framebuffer
  void *pixels;
  int pitch;
  if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0) {
    fprintf(stderr, "Erro ao bloquear textura: %s\n", SDL_GetError());
    return;
  }

  // Copia o framebuffer para a textura
  const int bytes_per_pixel = 4; // RGBA
  const int row_bytes = width * bytes_per_pixel;
  uint8_t *dst = (uint8_t *)pixels;
  const uint8_t *src = framebuffer;

  for (uint32_t y = 0; y < height; y++) {
    memcpy(dst, src, row_bytes);
    dst += pitch;
    src += row_bytes;
  }

  SDL_UnlockTexture(texture);

  // Limpa o renderizador e desenha a textura
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

bool waveform_display_process_events(waveform_display_t *display) {
  if (!display)
    return false;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      return false;

    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
        waveform_display_resize(display, event.window.data1,
                                event.window.data2);
      }
      break;

    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_f) {
        waveform_display_toggle_fullscreen(display);
      }
      break;
    }
  }

  return true;
}
