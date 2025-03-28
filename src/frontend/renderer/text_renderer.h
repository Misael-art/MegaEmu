/**
 * @file text_renderer.h
 * @brief Sistema de renderização de texto para o emulador
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2024-07-22
 */

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "texture_cache.h"

#ifdef USE_SDL2_TTF
#include <SDL2/SDL_ttf.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Definição de cores padrão para texto
 */
#define TEXT_COLOR_WHITE {255, 255, 255, 255}
#define TEXT_COLOR_BLACK {0, 0, 0, 255}
#define TEXT_COLOR_RED {255, 0, 0, 255}
#define TEXT_COLOR_GREEN {0, 255, 0, 255}
#define TEXT_COLOR_BLUE {0, 0, 255, 255}
#define TEXT_COLOR_YELLOW {255, 255, 0, 255}
#define TEXT_COLOR_CYAN {0, 255, 255, 255}
#define TEXT_COLOR_MAGENTA {255, 0, 255, 255}
#define TEXT_COLOR_GRAY {128, 128, 128, 255}

    /**
     * @brief Alinhamentos de texto disponíveis
     */
    typedef enum
    {
        TEXT_ALIGN_LEFT,
        TEXT_ALIGN_CENTER,
        TEXT_ALIGN_RIGHT
    } text_align_t;

    /**
     * @brief Estilos de texto disponíveis
     */
    typedef enum
    {
        TEXT_STYLE_NORMAL = 0,
        TEXT_STYLE_BOLD = 1,
        TEXT_STYLE_ITALIC = 2,
        TEXT_STYLE_UNDERLINE = 4
    } text_style_flags_t;

    /**
     * @brief Tamanhos de fonte disponíveis
     */
    typedef enum
    {
        TEXT_SIZE_SMALL,
        TEXT_SIZE_NORMAL,
        TEXT_SIZE_LARGE
    } text_size_t;

    /**
     * @brief Estrutura que define o estilo do texto
     */
    typedef struct
    {
        text_size_t size;
        SDL_Color color;
        int flags;
        int alignment;
        int reserved;
    } text_style_t;

    /**
     * @brief Inicializa o sistema de renderização de texto
     *
     * @param renderer Ponteiro para o renderer SDL
     * @param cache Ponteiro para o cache de texturas
     * @return true se a inicialização foi bem-sucedida, false caso contrário
     */
    bool text_renderer_init(SDL_Renderer *renderer, texture_cache_t *cache);

    /**
     * @brief Finaliza o sistema de renderização de texto e libera recursos
     */
    void text_renderer_shutdown(void);

    /**
     * @brief Renderiza texto na tela
     *
     * @param text Texto a ser renderizado
     * @param x Posição X
     * @param y Posição Y
     * @param style Estilo do texto
     * @return true se a renderização foi bem-sucedida, false caso contrário
     */
    bool text_renderer_render(const char *text, int x, int y, text_style_t style);

    /**
     * @brief Renderiza texto na tela com quebra de linha automática
     *
     * @param text Texto a ser renderizado
     * @param x Posição X
     * @param y Posição Y
     * @param max_width Largura máxima para quebra de linha
     * @param style Estilo do texto
     * @return true se a renderização foi bem-sucedida, false caso contrário
     */
    bool text_renderer_draw_wrapped(const char *text, int x, int y,
                                    int max_width, text_style_t style);

    /**
     * @brief Verifica se o sistema de texto avançado está disponível
     *
     * @return true se SDL2_ttf está disponível, false caso contrário
     */
    bool text_renderer_has_ttf(void);

    /**
     * @brief Obtém o tamanho que um texto ocuparia se renderizado
     *
     * @param text Texto a ser medido
     * @param style Estilo do texto
     * @param width Ponteiro para receber a largura (pode ser NULL)
     * @param height Ponteiro para receber a altura (pode ser NULL)
     * @return true se a medição foi bem-sucedida, false caso contrário
     */
    bool text_renderer_measure(const char *text, text_style_t style, int *width, int *height);

    /**
     * @brief Obtém o tamanho que um texto ocuparia se renderizado
     *
     * @param text Texto a ser medido
     * @param style Estilo do texto
     * @param width Ponteiro para receber a largura (pode ser NULL)
     * @param height Ponteiro para receber a altura (pode ser NULL)
     * @return true se a medição foi bem-sucedida, false caso contrário
     */
    void text_renderer_get_size(const char *text, text_style_t style, int *width, int *height);

    /**
     * @brief Estilo padrão
     *
     * @return Estilo padrão
     */
    static inline text_style_t text_style_default(void)
    {
        text_style_t style;
        style.size = TEXT_SIZE_NORMAL;
        SDL_Color color = {255, 255, 255, 255}; // Cor branca
        style.color = color;
        style.flags = TEXT_STYLE_NORMAL;
        style.alignment = 0;
        style.reserved = 0;
        return style;
    }

#ifdef __cplusplus
}
#endif

#endif /* TEXT_RENDERER_H */
