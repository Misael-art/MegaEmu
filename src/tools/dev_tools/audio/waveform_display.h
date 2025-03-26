/**
 * @file waveform_display.h
 * @brief Interface para exibição do framebuffer do visualizador
 */

#ifndef WAVEFORM_DISPLAY_H
#define WAVEFORM_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Estrutura que mantém o estado da janela de exibição
 */
typedef struct {
  uint32_t width;        // Largura da janela
  uint32_t height;       // Altura da janela
  bool fullscreen;       // Se está em tela cheia
  void *window_handle;   // Handle da janela (específico da plataforma)
  void *renderer_handle; // Handle do renderizador (específico da plataforma)
  void *texture_handle;  // Handle da textura (específico da plataforma)
} waveform_display_t;

/**
 * @brief Cria uma nova janela de exibição
 * @param width Largura inicial da janela
 * @param height Altura inicial da janela
 * @param title Título da janela
 * @return Ponteiro para o display criado ou NULL em caso de erro
 */
waveform_display_t *waveform_display_create(uint32_t width, uint32_t height,
                                            const char *title);

/**
 * @brief Destrói uma janela de exibição
 * @param display Ponteiro para o display
 */
void waveform_display_destroy(waveform_display_t *display);

/**
 * @brief Redimensiona a janela
 * @param display Ponteiro para o display
 * @param width Nova largura
 * @param height Nova altura
 */
void waveform_display_resize(waveform_display_t *display, uint32_t width,
                             uint32_t height);

/**
 * @brief Alterna entre modo janela e tela cheia
 * @param display Ponteiro para o display
 */
void waveform_display_toggle_fullscreen(waveform_display_t *display);

/**
 * @brief Atualiza o conteúdo da janela com o framebuffer
 * @param display Ponteiro para o display
 * @param framebuffer Ponteiro para o framebuffer RGBA
 * @param width Largura do framebuffer
 * @param height Altura do framebuffer
 */
void waveform_display_update(waveform_display_t *display,
                             const uint8_t *framebuffer, uint32_t width,
                             uint32_t height);

/**
 * @brief Processa eventos da janela
 * @param display Ponteiro para o display
 * @return true se a janela deve continuar aberta
 */
bool waveform_display_process_events(waveform_display_t *display);

/**
 * @brief Inicializa o subsistema de exibição
 * @return true se a inicialização foi bem sucedida
 */
bool waveform_display_init(void);

/**
 * @brief Finaliza o subsistema de exibição
 */
void waveform_display_quit(void);

#endif // WAVEFORM_DISPLAY_H
