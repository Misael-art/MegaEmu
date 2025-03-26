/**
 * @file gui_types.h
 * @brief Definições de tipos básicos para a interface gráfica do Mega_Emu
 */
#ifndef EMU_GUI_TYPES_H
#define EMU_GUI_TYPES_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

// Constantes
#define GUI_MAX_TEXT 256
#define GUI_MAX_CHILDREN 16
#define GUI_MAX_PROPERTIES 32
#define GUI_INVALID_ID 0
#define GUI_MAX_PROPERTY_NAME 32
#define GUI_MAX_PROPERTY_VALUE 256
#define GUI_MAX_ELEMENTS 256
#define GUI_SUCCESS 0
#define GUI_MANAGER_MAX_ELEMENTS 256
#define GUI_MANAGER_MAX_CALLBACKS 128

// Códigos de erro
typedef enum {
    GUI_ERROR_SUCCESS = 0,
    GUI_ERROR_INIT_FAILED,
    GUI_ERROR_INVALID_PARAMETER,
    GUI_ERROR_OUT_OF_MEMORY,
    GUI_ERROR_NOT_FOUND,
    GUI_ERROR_INVALID_STATE,
    GUI_ERROR_NOT_HANDLED,
    GUI_ERROR_LIMIT_EXCEEDED,
    GUI_ERROR_MAX
} gui_error_t;

// Tipos de elementos
typedef enum {
    GUI_ELEMENT_NONE = 0,
    GUI_ELEMENT_WINDOW,
    GUI_ELEMENT_BUTTON,
    GUI_ELEMENT_LABEL,
    GUI_ELEMENT_TEXTBOX,
    GUI_ELEMENT_CHECKBOX,
    GUI_ELEMENT_LISTBOX,
    GUI_ELEMENT_DROPDOWN,
    GUI_ELEMENT_CONTAINER,
    GUI_ELEMENT_CUSTOM,
    GUI_ELEMENT_MAX
} gui_element_type_t;

// ID do elemento
typedef uint32_t gui_element_id_t;

// Retângulo
typedef struct {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
} gui_rect_t;

// Ponto
typedef struct {
    int32_t x;
    int32_t y;
} gui_point_t;

// Cor
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} gui_color_t;

// Tipos de propriedades
typedef enum {
    GUI_PROPERTY_NONE = 0,
    GUI_PROPERTY_INT,
    GUI_PROPERTY_FLOAT,
    GUI_PROPERTY_BOOL,
    GUI_PROPERTY_STRING,
    GUI_PROPERTY_COLOR,
    GUI_PROPERTY_POINTER,
    GUI_PROPERTY_MAX
} gui_property_type_t;

// Propriedade do elemento
typedef struct {
    char name[GUI_MAX_PROPERTY_NAME];
    gui_property_type_t type;
    union {
        int integer;
        float floating;
        bool boolean;
        char *string;
        gui_color_t color;
        void *pointer;
    } value;
} gui_property_t;

// Tipos de eventos
typedef enum {
    GUI_EVENT_NONE = 0,
    GUI_EVENT_MOUSE_MOVE,
    GUI_EVENT_MOUSE_DOWN,
    GUI_EVENT_MOUSE_UP,
    GUI_EVENT_MOUSE_ENTER,
    GUI_EVENT_MOUSE_LEAVE,
    GUI_EVENT_KEY_DOWN,
    GUI_EVENT_KEY_UP,
    GUI_EVENT_FOCUS,
    GUI_EVENT_BLUR,
    GUI_EVENT_CLICK,
    GUI_EVENT_DOUBLE_CLICK,
    GUI_EVENT_DRAG_START,
    GUI_EVENT_DRAG_MOVE,
    GUI_EVENT_DRAG_END,
    GUI_EVENT_VALUE_CHANGE,
    GUI_EVENT_CUSTOM,
    GUI_EVENT_MAX
} gui_event_type_t;

// Evento do mouse
typedef struct {
    gui_point_t position;
    int32_t button;
    int32_t clicks;
} gui_mouse_event_t;

// Evento do teclado
typedef struct {
    int32_t key_code;
    int32_t scan_code;
    uint16_t modifiers;
    bool repeat;
} gui_key_event_t;

// Evento de valor
typedef struct {
    void *old_value;
    void *new_value;
    gui_property_type_t value_type;
} gui_value_event_t;

// Evento personalizado
typedef struct {
    uint32_t type;
    void *data;
} gui_custom_event_t;

// Evento da GUI
typedef struct {
    gui_event_type_t type;
    gui_element_id_t target;
    union {
        gui_mouse_event_t mouse;
        gui_key_event_t key;
        gui_value_event_t value;
        gui_custom_event_t custom;
    };
} gui_event_t;

// Declaração avançada para estrutura do elemento
typedef struct gui_element_s gui_element_s;

// Estrutura do elemento
struct gui_element_s {
    gui_element_id_t id;
    gui_element_type_t type;
    gui_rect_t rect;
    char text[GUI_MAX_TEXT];
    bool visible;
    bool enabled;
    gui_element_id_t parent;
    gui_element_id_t children[GUI_MAX_CHILDREN];
    uint32_t child_count;
    gui_property_t properties[GUI_MAX_PROPERTIES];
    uint32_t property_count;
    
    // Funções virtuais para comportamento específico do tipo
    void (*render)(gui_element_s *element, SDL_Renderer *renderer);
    bool (*process_event)(gui_element_s *element, const gui_event_t *event);
    void (*update)(gui_element_s *element);
    void (*destroy)(gui_element_s *element);
};

#endif /* EMU_GUI_TYPES_H */
