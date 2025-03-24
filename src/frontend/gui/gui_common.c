/**
 * @file gui_common.c
 * @brief Implementação de funções utilitárias e constantes comuns para o sistema GUI
 */

#include "gui/gui_common.h"
#include <string.h>
#include <stdlib.h>

// Definições das cores padrão
const gui_color_t GUI_COLOR_BLACK = {0, 0, 0, 255};
const gui_color_t GUI_COLOR_WHITE = {255, 255, 255, 255};
const gui_color_t GUI_COLOR_RED = {255, 0, 0, 255};
const gui_color_t GUI_COLOR_GREEN = {0, 255, 0, 255};
const gui_color_t GUI_COLOR_BLUE = {0, 0, 255, 255};
const gui_color_t GUI_COLOR_GRAY = {128, 128, 128, 255};
const gui_color_t GUI_COLOR_LIGHT_GRAY = {192, 192, 192, 255};
const gui_color_t GUI_COLOR_DARK_GRAY = {64, 64, 64, 255};
const gui_color_t GUI_COLOR_TRANSPARENT = {0, 0, 0, 0};

/**
 * @brief Verifica se um ponto está dentro de um retângulo
 *
 * @param rect Retângulo a ser verificado
 * @param x Coordenada X do ponto
 * @param y Coordenada Y do ponto
 * @return true se o ponto está dentro do retângulo, false caso contrário
 */
bool gui_point_in_rect(const gui_rect_t *rect, int32_t x, int32_t y)
{
    if (!rect)
        return false;

    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

/**
 * @brief Define as propriedades de um retângulo
 *
 * @param rect Retângulo a ser modificado
 * @param x Coordenada X
 * @param y Coordenada Y
 * @param w Largura
 * @param h Altura
 */
void gui_rect_set(gui_rect_t *rect, int32_t x, int32_t y, int32_t w, int32_t h)
{
    if (!rect)
        return;

    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

/**
 * @brief Cria um novo retângulo com os valores especificados
 *
 * @param x Coordenada X
 * @param y Coordenada Y
 * @param w Largura
 * @param h Altura
 * @return gui_rect_t O retângulo criado
 */
gui_rect_t gui_rect_create(int32_t x, int32_t y, int32_t w, int32_t h)
{
    gui_rect_t rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    return rect;
}

/**
 * @brief Cria uma nova cor com os valores especificados
 *
 * @param r Componente vermelho (0-255)
 * @param g Componente verde (0-255)
 * @param b Componente azul (0-255)
 * @param a Componente alfa (0-255)
 * @return gui_color_t A cor criada
 */
gui_color_t gui_color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    gui_color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

/**
 * @brief Define os valores de uma cor
 *
 * @param color Ponteiro para a cor a ser modificada
 * @param r Componente vermelho (0-255)
 * @param g Componente verde (0-255)
 * @param b Componente azul (0-255)
 * @param a Componente alfa (0-255)
 */
void gui_color_set(gui_color_t *color, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (!color)
        return;

    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;
}

/**
 * @brief Cria uma nova propriedade do tipo inteiro
 */
gui_property_t *gui_make_property_int(const char *name, int32_t value)
{
    gui_property_t *prop = (gui_property_t *)malloc(sizeof(gui_property_t));
    if (!prop)
        return NULL;

    strncpy(prop->name, name, GUI_MAX_PROPERTY_NAME - 1);
    prop->name[GUI_MAX_PROPERTY_NAME - 1] = '\0';
    prop->type = GUI_PROPERTY_INT;
    prop->value.integer = value;
    return prop;
}

/**
 * @brief Cria uma nova propriedade do tipo float
 */
gui_property_t *gui_make_property_float(const char *name, float value)
{
    gui_property_t *prop = (gui_property_t *)malloc(sizeof(gui_property_t));
    if (!prop)
        return NULL;

    strncpy(prop->name, name, GUI_MAX_PROPERTY_NAME - 1);
    prop->name[GUI_MAX_PROPERTY_NAME - 1] = '\0';
    prop->type = GUI_PROPERTY_FLOAT;
    prop->value.floating = value;
    return prop;
}

/**
 * @brief Cria uma nova propriedade do tipo booleano
 */
gui_property_t *gui_make_property_bool(const char *name, bool value)
{
    gui_property_t *prop = (gui_property_t *)malloc(sizeof(gui_property_t));
    if (!prop)
        return NULL;

    strncpy(prop->name, name, GUI_MAX_PROPERTY_NAME - 1);
    prop->name[GUI_MAX_PROPERTY_NAME - 1] = '\0';
    prop->type = GUI_PROPERTY_BOOL;
    prop->value.boolean = value;
    return prop;
}

/**
 * @brief Cria uma nova propriedade do tipo string
 */
gui_property_t *gui_make_property_string(const char *name, const char *value)
{
    gui_property_t *prop = (gui_property_t *)malloc(sizeof(gui_property_t));
    if (!prop)
        return NULL;

    strncpy(prop->name, name, GUI_MAX_PROPERTY_NAME - 1);
    prop->name[GUI_MAX_PROPERTY_NAME - 1] = '\0';
    prop->type = GUI_PROPERTY_STRING;
    prop->value.string = strdup(value);
    return prop;
}

/**
 * @brief Cria uma nova propriedade do tipo cor
 */
gui_property_t *gui_make_property_color(const char *name, gui_color_t value)
{
    gui_property_t *prop = (gui_property_t *)malloc(sizeof(gui_property_t));
    if (!prop)
        return NULL;

    strncpy(prop->name, name, GUI_MAX_PROPERTY_NAME - 1);
    prop->name[GUI_MAX_PROPERTY_NAME - 1] = '\0';
    prop->type = GUI_PROPERTY_COLOR;
    prop->value.color = value;
    return prop;
}

/**
 * @brief Cria uma nova propriedade do tipo ponteiro
 */
gui_property_t *gui_make_property_pointer(const char *name, void *value)
{
    gui_property_t *prop = (gui_property_t *)malloc(sizeof(gui_property_t));
    if (!prop)
        return NULL;

    strncpy(prop->name, name, GUI_MAX_PROPERTY_NAME - 1);
    prop->name[GUI_MAX_PROPERTY_NAME - 1] = '\0';
    prop->type = GUI_PROPERTY_POINTER;
    prop->value.pointer = value;
    return prop;
}
