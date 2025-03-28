#ifndef MEGA_EMU_GUI_COMMON_H
#define MEGA_EMU_GUI_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "gui_types.h"

// Constantes para o sistema de GUI
#define GUI_MAX_PATH 256
#define GUI_MAX_NAME 32

// Nomes padrão para propriedades comuns
#define GUI_PROP_BACKGROUND_COLOR "bg_color"
#define GUI_PROP_FOREGROUND_COLOR "fg_color"
#define GUI_PROP_BORDER_COLOR "border_color"
#define GUI_PROP_HOVER_COLOR "hover_color"
#define GUI_PROP_SELECTED_COLOR "selected_color"
#define GUI_PROP_TEXT_COLOR "text_color"
#define GUI_PROP_SAVE_DIR "save_dir"
#define GUI_PROP_PLATFORM_ID "platform_id"
#define GUI_PROP_GAME_TITLE "game_title"

// Margens e espaçamentos padrão
#define GUI_DEFAULT_MARGIN 10
#define GUI_DEFAULT_PADDING 5
#define GUI_DEFAULT_SPACING 8

// Definição das macros de log
// Estas devem estar alinhadas com as definições condicionais em gui_types.h
#ifdef GUI_LOG_DEBUG
#undef GUI_LOG_DEBUG
#endif
#define GUI_LOG_DEBUG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)

#ifdef GUI_LOG_INFO
#undef GUI_LOG_INFO
#endif
#define GUI_LOG_INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)

#ifdef GUI_LOG_ERROR
#undef GUI_LOG_ERROR
#endif
#define GUI_LOG_ERROR(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)

    // Cores padrão
    extern const gui_color_t GUI_COLOR_BLACK;
    extern const gui_color_t GUI_COLOR_WHITE;
    extern const gui_color_t GUI_COLOR_RED;
    extern const gui_color_t GUI_COLOR_GREEN;
    extern const gui_color_t GUI_COLOR_BLUE;
    extern const gui_color_t GUI_COLOR_GRAY;
    extern const gui_color_t GUI_COLOR_LIGHT_GRAY;
    extern const gui_color_t GUI_COLOR_DARK_GRAY;
    extern const gui_color_t GUI_COLOR_TRANSPARENT;

    // Funções utilitárias
    bool gui_point_in_rect(const gui_rect_t *rect, int32_t x, int32_t y);
    void gui_rect_set(gui_rect_t *rect, int32_t x, int32_t y, int32_t w, int32_t h);
    gui_rect_t gui_rect_create(int32_t x, int32_t y, int32_t w, int32_t h);
    gui_color_t gui_color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void gui_color_set(gui_color_t *color, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // Funções de propriedade
    gui_property_t *gui_make_property_int(const char *name, int32_t value);
    gui_property_t *gui_make_property_float(const char *name, float value);
    gui_property_t *gui_make_property_bool(const char *name, bool value);
    gui_property_t *gui_make_property_string(const char *name, const char *value);
    gui_property_t *gui_make_property_color(const char *name, gui_color_t value);
    gui_property_t *gui_make_property_pointer(const char *name, void *value);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_GUI_COMMON_H */
