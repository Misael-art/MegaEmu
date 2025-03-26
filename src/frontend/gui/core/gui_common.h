/**
 * @file gui_common.h
 * @brief Funções e macros comuns para o sistema de GUI do Mega_Emu
 */
#ifndef EMU_GUI_COMMON_H
#define EMU_GUI_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../core/logging/log.h"

// Categorias de log
#define EMU_LOG_CAT_GUI "GUI"

// Macros de log
#define GUI_LOG_DEBUG(fmt, ...) emu_log_debug(EMU_LOG_CAT_GUI, fmt, ##__VA_ARGS__)
#define GUI_LOG_INFO(fmt, ...) emu_log_info(EMU_LOG_CAT_GUI, fmt, ##__VA_ARGS__)
#define GUI_LOG_WARN(fmt, ...) emu_log_warn(EMU_LOG_CAT_GUI, fmt, ##__VA_ARGS__)
#define GUI_LOG_ERROR(fmt, ...) emu_log_error(EMU_LOG_CAT_GUI, fmt, ##__VA_ARGS__)

// Funções utilitárias comuns

/**
 * @brief Verifica se um ponto está dentro de um retângulo
 * 
 * @param x Coordenada X do ponto
 * @param y Coordenada Y do ponto
 * @param rect Retângulo a ser verificado
 * @return true se o ponto está dentro do retângulo, false caso contrário
 */
static inline bool gui_point_in_rect(int32_t x, int32_t y, const gui_rect_t *rect) {
    return (x >= rect->x && x < rect->x + rect->w && 
            y >= rect->y && y < rect->y + rect->h);
}

/**
 * @brief Cria uma cópia de uma string
 * 
 * @param str String a ser copiada
 * @return char* Ponteiro para a nova string ou NULL em caso de erro
 */
static inline char *gui_strdup(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char *new_str = (char *)malloc(len);
    
    if (new_str) {
        memcpy(new_str, str, len);
    }
    
    return new_str;
}

/**
 * @brief Verifica se dois retângulos se intersectam
 * 
 * @param a Primeiro retângulo
 * @param b Segundo retângulo
 * @return true se os retângulos se intersectam, false caso contrário
 */
static inline bool gui_rect_intersect(const gui_rect_t *a, const gui_rect_t *b) {
    return (a->x < b->x + b->w && a->x + a->w > b->x &&
            a->y < b->y + b->h && a->y + a->h > b->y);
}

/**
 * @brief Calcula a interseção entre dois retângulos
 * 
 * @param a Primeiro retângulo
 * @param b Segundo retângulo
 * @param result Retângulo resultante da interseção
 * @return true se há interseção, false caso contrário
 */
static inline bool gui_rect_intersection(const gui_rect_t *a, const gui_rect_t *b, gui_rect_t *result) {
    if (!gui_rect_intersect(a, b)) {
        return false;
    }
    
    result->x = (a->x > b->x) ? a->x : b->x;
    result->y = (a->y > b->y) ? a->y : b->y;
    result->w = ((a->x + a->w) < (b->x + b->w)) ? (a->x + a->w) - result->x : (b->x + b->w) - result->x;
    result->h = ((a->y + a->h) < (b->y + b->h)) ? (a->y + a->h) - result->y : (b->y + b->h) - result->y;
    
    return true;
}

#endif /* EMU_GUI_COMMON_H */
