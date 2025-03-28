/**
 * @file gui_types.h
 * @brief Definições de tipos básicos para a interface gráfica do Mega_Emu
 */
#ifndef MEGA_EMU_GUI_TYPES_H
#define MEGA_EMU_GUI_TYPES_H

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
#define GUI_SUCCESS GUI_ERROR_SUCCESS
#define GUI_MANAGER_MAX_ELEMENTS 256
#define GUI_MANAGER_MAX_CALLBACKS 128

// Macros de log
// Estas são apenas declarações, as implementações devem estar em gui_common.h
#ifndef GUI_LOG_DEBUG
#define GUI_LOG_DEBUG(fmt, ...) fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#endif

#ifndef GUI_LOG_INFO
#define GUI_LOG_INFO(fmt, ...) fprintf(stderr, "[INFO] " fmt "\n", ##__VA_ARGS__)
#endif

#ifndef GUI_LOG_ERROR
#define GUI_LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#endif

// Códigos de erro
typedef enum gui_error_e
{
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
typedef enum gui_element_type_e
{
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
typedef struct gui_rect_s
{
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
} gui_rect_t;

// Ponto
typedef struct gui_point_s
{
    int32_t x;
    int32_t y;
} gui_point_t;

// Cor
typedef struct gui_color_s
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} gui_color_t;

// Tipos de propriedades
typedef enum gui_property_type_e
{
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
typedef struct gui_property_s
{
    char name[GUI_MAX_PROPERTY_NAME];
    gui_property_type_t type;
    union
    {
        int integer;
        float floating;
        bool boolean;
        char *string;
        gui_color_t color;
        void *pointer;
    } value;
} gui_property_t;

// Estado do botão
typedef enum gui_button_state_e
{
    GUI_BUTTON_RELEASED = 0,
    GUI_BUTTON_PRESSED
} gui_button_state_t;

// Botões do mouse
typedef enum gui_mouse_button_e
{
    GUI_MOUSE_BUTTON_LEFT = 0,
    GUI_MOUSE_BUTTON_MIDDLE,
    GUI_MOUSE_BUTTON_RIGHT,
    GUI_MOUSE_BUTTON_MAX
} gui_mouse_button_t;

// Tipos de eventos
typedef enum gui_event_type_e
{
    GUI_EVENT_NONE = 0,
    GUI_EVENT_MOUSE_BUTTON,
    GUI_EVENT_MOUSE_MOTION,
    GUI_EVENT_CLICK,
    GUI_EVENT_HOVER,
    GUI_EVENT_LEAVE,
    GUI_EVENT_KEY_DOWN,
    GUI_EVENT_KEY_UP,
    GUI_EVENT_MAX
} gui_event_type_t;

// Evento de botão do mouse
typedef struct gui_mouse_button_event_s
{
    gui_point_t point;
    gui_mouse_button_t button;
    gui_button_state_t state;
} gui_mouse_button_event_t;

// Evento de movimento do mouse
typedef struct gui_mouse_motion_event_s
{
    gui_point_t point;
    gui_point_t rel;
} gui_mouse_motion_event_t;

// Forward declarations
struct gui_element_s;
struct gui_event_s;

// Callbacks
typedef void (*gui_element_callback_t)(struct gui_element_s *element);
typedef bool (*gui_event_callback_t)(struct gui_element_s *element, const struct gui_event_s *event);
typedef void (*gui_update_callback_t)(struct gui_element_s *element, float delta_time);
typedef void (*gui_render_callback_t)(struct gui_element_s *element, SDL_Renderer *renderer);

// Definição de callback GUI
typedef bool (*gui_callback_fn)(struct gui_element_s *element, void *user_data);

// Estrutura de callback
typedef struct gui_callback_s
{
    gui_event_type_t event_type;
    gui_element_id_t element_id;
    gui_callback_fn callback;
    void *user_data;
} gui_callback_t;

// Estrutura de evento
typedef struct gui_event_s
{
    gui_event_type_t type;
    gui_element_id_t element_id;
    union
    {
        gui_mouse_button_event_t mouse_button;
        gui_mouse_motion_event_t mouse_motion;
        int key_code;
    } data;
} gui_event_t;

// Estrutura base de elemento
typedef struct gui_element_s
{
    gui_element_id_t id;
    gui_element_type_t type;
    gui_rect_t bounds;
    bool visible;
    bool enabled;
    char *text;

    // Campos de hierarquia
    struct gui_element_s *parent;
    uint32_t num_children;
    struct gui_element_s *children[GUI_MAX_CHILDREN];

    // Campos de propriedades
    uint32_t num_properties;
    gui_property_t properties[GUI_MAX_PROPERTIES];

    // Callbacks
    gui_element_callback_t on_click;
    gui_element_callback_t on_hover;
    gui_element_callback_t on_leave;
    gui_event_callback_t on_event;
    gui_update_callback_t on_update;
    gui_render_callback_t on_render;

    // Dados específicos do elemento
    void *data;
    void (*destroy_data)(void *data);
} gui_element_t;

// Estrutura do gerenciador GUI
typedef struct gui_manager_s
{
    bool initialized;
    int width;
    int height;
    int mouse_x;
    int mouse_y;
    bool mouse_buttons[GUI_MOUSE_BUTTON_MAX];
    gui_element_t *main_screen;
    gui_element_t *focused_element;
    gui_element_t *dragged_element;
} gui_manager_t;

// Ponteiro para o tipo gui_element_t
typedef gui_element_t *gui_element_ptr_t;

#endif /* MEGA_EMU_GUI_TYPES_H */
