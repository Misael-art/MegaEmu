/**
 * @file sdl_screenshot.h
 * @brief Sistema de captura de tela para o frontend SDL
 */
#ifndef SDL_SCREENSHOT_H
#define SDL_SCREENSHOT_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sdl_game_renderer.h"

// Constantes para o sistema de screenshot
#define SDL_SCREENSHOT_MAX_PATH 1024
#define SDL_SCREENSHOT_FORMAT_PNG 0
#define SDL_SCREENSHOT_FORMAT_BMP 1
#define SDL_SCREENSHOT_FORMAT_JPG 2

// Estrutura de configuração para screenshots
typedef struct {
    char output_dir[SDL_SCREENSHOT_MAX_PATH];  // Diretório para salvar
    int format;                               // Formato (PNG, BMP, JPG)
    int quality;                              // Qualidade (0-100) para JPG
    bool include_timestamp;                   // Incluir timestamp no nome
    bool include_game_name;                   // Incluir nome do jogo no nome
    bool show_notification;                   // Mostrar notificação na tela
} sdl_screenshot_config_t;

// Estrutura principal para o sistema de screenshots
typedef struct {
    sdl_screenshot_config_t config;           // Configuração
    bool initialized;                         // Se está inicializado
    time_t last_screenshot_time;              // Timestamp da última captura
    char last_screenshot_path[SDL_SCREENSHOT_MAX_PATH]; // Caminho da última captura
    char current_game_name[64];               // Nome do jogo atual

    // Notificação na tela
    struct {
        bool visible;                          // Se a notificação está visível
        Uint32 start_time;                     // Quando a notificação começou
        Uint32 duration;                       // Duração da notificação (ms)
        SDL_Texture *icon;                     // Ícone da notificação
        SDL_Texture *message;                  // Mensagem renderizada
        SDL_Rect position;                     // Posição na tela
    } notification;

    // Referência ao renderizador
    sdl_game_renderer_t *renderer;            // Renderizador do jogo
} sdl_screenshot_t;

// Funções de inicialização e finalização
bool sdl_screenshot_init(sdl_screenshot_t *screenshot, sdl_game_renderer_t *renderer);
void sdl_screenshot_shutdown(sdl_screenshot_t *screenshot);

// Funções para capturar a tela
bool sdl_screenshot_capture(sdl_screenshot_t *screenshot);
bool sdl_screenshot_capture_to_file(sdl_screenshot_t *screenshot, const char *filepath);
bool sdl_screenshot_capture_framebuffer(sdl_screenshot_t *screenshot, const uint32_t *framebuffer,
                                       int width, int height);

// Funções para gerenciar configurações
void sdl_screenshot_set_config(sdl_screenshot_t *screenshot, const sdl_screenshot_config_t *config);
void sdl_screenshot_get_config(const sdl_screenshot_t *screenshot, sdl_screenshot_config_t *config);
void sdl_screenshot_set_default_config(sdl_screenshot_t *screenshot);
bool sdl_screenshot_save_config(const sdl_screenshot_t *screenshot, const char *filepath);
bool sdl_screenshot_load_config(sdl_screenshot_t *screenshot, const char *filepath);

// Funções para gerenciar o estado atual
void sdl_screenshot_set_game_name(sdl_screenshot_t *screenshot, const char *game_name);
const char* sdl_screenshot_get_last_path(const sdl_screenshot_t *screenshot);
time_t sdl_screenshot_get_last_time(const sdl_screenshot_t *screenshot);

// Funções para a notificação na tela
void sdl_screenshot_update_notification(sdl_screenshot_t *screenshot);
void sdl_screenshot_render_notification(sdl_screenshot_t *screenshot);
void sdl_screenshot_set_notification_duration(sdl_screenshot_t *screenshot, Uint32 duration_ms);

#endif /* SDL_SCREENSHOT_H */
