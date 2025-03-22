/** * @file gui_common.c * @brief Implementação de funções utilitárias e constantes comuns para o sistema GUI */#include "gui/gui_common.h"#include <string.h>// Definições das cores padrãoconst gui_color_t GUI_COLOR_BLACK = {0, 0, 0, 255};const gui_color_t GUI_COLOR_WHITE = {255, 255, 255, 255};const gui_color_t GUI_COLOR_RED = {255, 0, 0, 255};const gui_color_t GUI_COLOR_GREEN = {0, 255, 0, 255};const gui_color_t GUI_COLOR_BLUE = {0, 0, 255, 255};const gui_color_t GUI_COLOR_GRAY = {128, 128, 128, 255};const gui_color_t GUI_COLOR_LIGHT_GRAY = {192, 192, 192, 255};const gui_color_t GUI_COLOR_DARK_GRAY = {64, 64, 64, 255};const gui_color_t GUI_COLOR_TRANSPARENT = {0, 0, 0, 0};/** * @brief Verifica se um ponto está dentro de um retângulo * * @param rect Retângulo a ser verificado * @param x Coordenada X do ponto * @param y Coordenada Y do ponto * @return true se o ponto está dentro do retângulo, false caso contrário */bool gui_point_in_rect(const gui_rect_t *rect, int32_t x, int32_t y){    if (!rect)        return false;    return (x >= rect->x && x < rect->x + rect->width &&            y >= rect->y && y < rect->y + rect->height);}/** * @brief Define as propriedades de um retângulo * * @param rect Retângulo a ser modificado * @param x Coordenada X * @param y Coordenada Y * @param w Largura * @param h Altura */void gui_rect_set(gui_rect_t *rect, int32_t x, int32_t y, int32_t w, int32_t h){    if (!rect)        return;    rect->x = x;    rect->y = y;    rect->width = w;    rect->height = h;}/** * @brief Cria um novo retângulo com os valores especificados * * @param x Coordenada X * @param y Coordenada Y * @param w Largura * @param h Altura * @return gui_rect_t O retângulo criado */gui_rect_t gui_rect_create(int32_t x, int32_t y, int32_t w, int32_t h){    gui_rect_t rect;    rect.x = x;    rect.y = y;    rect.width = w;    rect.height = h;    return rect;}/** * @brief Cria uma nova cor com os valores especificados * * @param r Componente vermelho (0-255) * @param g Componente verde (0-255) * @param b Componente azul (0-255) * @param a Componente alfa (0-255) * @return gui_color_t A cor criada */gui_color_t gui_color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a){    gui_color_t color;    color.r = r;    color.g = g;    color.b = b;    color.a = a;    return color;}/** * @brief Define os valores de uma cor * * @param color Ponteiro para a cor a ser modificada * @param r Componente vermelho (0-255) * @param g Componente verde (0-255) * @param b Componente azul (0-255) * @param a Componente alfa (0-255) */void gui_color_set(gui_color_t *color, uint8_t r, uint8_t g, uint8_t b, uint8_t a){    if (!color)        return;    color->r = r;    color->g = g;    color->b = b;    color->a = a;}/** * @brief Cria uma propriedade de tipo inteiro * * @param name Nome da propriedade * @param value Valor inteiro * @return gui_element_property_t A propriedade criada */gui_element_property_t gui_make_property_int(const char *name, int32_t value){    gui_element_property_t prop;    strncpy(prop.name, name, GUI_MAX_PROPERTY_NAME - 1);    prop.name[GUI_MAX_PROPERTY_NAME - 1] = '\0';    prop.value.int_val = value;    return prop;}/** * @brief Cria uma propriedade de tipo ponto flutuante * * @param name Nome da propriedade * @param value Valor de ponto flutuante * @return gui_element_property_t A propriedade criada */gui_element_property_t gui_make_property_float(const char *name, float value){    gui_element_property_t prop;    strncpy(prop.name, name, GUI_MAX_PROPERTY_NAME - 1);    prop.name[GUI_MAX_PROPERTY_NAME - 1] = '\0';    prop.value.float_val = value;    return prop;}/** * @brief Cria uma propriedade de tipo booleano * * @param name Nome da propriedade * @param value Valor booleano * @return gui_element_property_t A propriedade criada */gui_element_property_t gui_make_property_bool(const char *name, bool value){    gui_element_property_t prop;    strncpy(prop.name, name, GUI_MAX_PROPERTY_NAME - 1);    prop.name[GUI_MAX_PROPERTY_NAME - 1] = '\0';    prop.value.bool_val = value;    return prop;}/** * @brief Cria uma propriedade de tipo string * * @param name Nome da propriedade * @param value Valor string * @return gui_element_property_t A propriedade criada */gui_element_property_t gui_make_property_string(const char *name, const char *value){    gui_element_property_t prop;    strncpy(prop.name, name, GUI_MAX_PROPERTY_NAME - 1);    prop.name[GUI_MAX_PROPERTY_NAME - 1] = '\0';    if (value)    {        strncpy(prop.value.string_val, value, GUI_MAX_PROPERTY_VALUE - 1);        prop.value.string_val[GUI_MAX_PROPERTY_VALUE - 1] = '\0';    }    else    {        prop.value.string_val[0] = '\0';    }    return prop;}/** * @brief Cria uma propriedade de tipo cor * * @param name Nome da propriedade * @param value Valor de cor * @return gui_element_property_t A propriedade criada */gui_element_property_t gui_make_property_color(const char *name, gui_color_t value){    gui_element_property_t prop;    strncpy(prop.name, name, GUI_MAX_PROPERTY_NAME - 1);    prop.name[GUI_MAX_PROPERTY_NAME - 1] = '\0';    prop.value.color_val = value;    return prop;}/** * @brief Cria uma propriedade de tipo ponteiro * * @param name Nome da propriedade * @param value Valor de ponteiro * @return gui_element_property_t A propriedade criada */gui_element_property_t gui_make_property_pointer(const char *name, void *value){    gui_element_property_t prop;    strncpy(prop.name, name, GUI_MAX_PROPERTY_NAME - 1);    prop.name[GUI_MAX_PROPERTY_NAME - 1] = '\0';    prop.value.ptr_val = value;    return prop;}