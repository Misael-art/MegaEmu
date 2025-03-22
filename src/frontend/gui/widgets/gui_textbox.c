/**
 * @file gui_textbox.c
 * @brief Implementação do widget de caixa de texto da GUI
 */
#include "gui_textbox.h"
#include "../core/gui_element.h"
#include "../core/gui_common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Definição de propriedades específicas da caixa de texto
#define GUI_PROP_TEXTBOX_TEXT_COLOR      "text_color"
#define GUI_PROP_TEXTBOX_BG_COLOR        "bg_color"
#define GUI_PROP_TEXTBOX_BORDER_COLOR    "border_color"
#define GUI_PROP_TEXTBOX_BORDER_WIDTH    "border_width"
#define GUI_PROP_TEXTBOX_MAX_LENGTH      "max_length"
#define GUI_PROP_TEXTBOX_READ_ONLY       "read_only"
#define GUI_PROP_TEXTBOX_FOCUSED         "focused"
#define GUI_PROP_TEXTBOX_CURSOR_POS      "cursor_pos"
#define GUI_PROP_TEXTBOX_ON_TEXT_CHANGED "on_text_changed"
#define GUI_PROP_TEXTBOX_ON_FOCUS_LOST   "on_focus_lost"
#define GUI_PROP_TEXTBOX_USER_DATA       "user_data"

// Valores padrão
static const gui_color_t DEFAULT_TEXT_COLOR = {0, 0, 0, 255};
static const gui_color_t DEFAULT_BG_COLOR = {255, 255, 255, 255};
static const gui_color_t DEFAULT_BORDER_COLOR = {128, 128, 128, 255};
static const int DEFAULT_BORDER_WIDTH = 1;
static const size_t DEFAULT_MAX_LENGTH = 256;
static const bool DEFAULT_READ_ONLY = false;
static const bool DEFAULT_FOCUSED = false;
static const int DEFAULT_CURSOR_POS = 0;

// Estrutura para armazenar callbacks
typedef struct {
    void (*on_text_changed)(gui_element_id_t, const char*, void*);
    void (*on_focus_lost)(gui_element_id_t, void*);
    void* user_data;
} textbox_callbacks_t;

// Função para processar entrada de texto
static void process_text_input(gui_element_t element, const char* input) {
    if (!element || !input) {
        return;
    }
    
    // Verificar se está em modo somente leitura
    bool read_only = DEFAULT_READ_ONLY;
    gui_element_get_property_bool(element, GUI_PROP_TEXTBOX_READ_ONLY, &read_only);
    if (read_only) {
        return;
    }
    
    // Obter texto atual
    char current_text[GUI_MAX_TEXT] = {0};
    strncpy(current_text, gui_element_get_text(element), GUI_MAX_TEXT - 1);
    
    // Obter posição do cursor
    int cursor_pos = DEFAULT_CURSOR_POS;
    gui_element_get_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, &cursor_pos);
    
    // Obter tamanho máximo
    size_t max_length = DEFAULT_MAX_LENGTH;
    gui_element_get_property_size_t(element, GUI_PROP_TEXTBOX_MAX_LENGTH, &max_length);
    
    // Verificar se há espaço para adicionar o texto
    size_t current_length = strlen(current_text);
    size_t input_length = strlen(input);
    
    if (current_length + input_length >= max_length) {
        return;
    }
    
    // Inserir texto na posição do cursor
    char new_text[GUI_MAX_TEXT] = {0};
    strncpy(new_text, current_text, cursor_pos);
    strncpy(new_text + cursor_pos, input, GUI_MAX_TEXT - cursor_pos - 1);
    strncpy(new_text + cursor_pos + input_length, current_text + cursor_pos, 
            GUI_MAX_TEXT - cursor_pos - input_length - 1);
    
    // Atualizar texto
    gui_element_set_text(element, new_text);
    
    // Atualizar posição do cursor
    cursor_pos += input_length;
    gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, cursor_pos);
    
    // Chamar callback de texto alterado
    textbox_callbacks_t* callbacks = NULL;
    if (gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_TEXT_CHANGED, (void**)&callbacks) && 
        callbacks && callbacks->on_text_changed) {
        callbacks->on_text_changed(gui_element_get_id(element), new_text, callbacks->user_data);
    }
}

// Função para processar tecla de backspace
static void process_backspace(gui_element_t element) {
    if (!element) {
        return;
    }
    
    // Verificar se está em modo somente leitura
    bool read_only = DEFAULT_READ_ONLY;
    gui_element_get_property_bool(element, GUI_PROP_TEXTBOX_READ_ONLY, &read_only);
    if (read_only) {
        return;
    }
    
    // Obter texto atual
    char current_text[GUI_MAX_TEXT] = {0};
    strncpy(current_text, gui_element_get_text(element), GUI_MAX_TEXT - 1);
    
    // Obter posição do cursor
    int cursor_pos = DEFAULT_CURSOR_POS;
    gui_element_get_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, &cursor_pos);
    
    // Verificar se há caracteres para apagar
    if (cursor_pos <= 0 || strlen(current_text) == 0) {
        return;
    }
    
    // Remover caractere antes do cursor
    char new_text[GUI_MAX_TEXT] = {0};
    strncpy(new_text, current_text, cursor_pos - 1);
    strncpy(new_text + cursor_pos - 1, current_text + cursor_pos, GUI_MAX_TEXT - cursor_pos);
    
    // Atualizar texto
    gui_element_set_text(element, new_text);
    
    // Atualizar posição do cursor
    cursor_pos--;
    gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, cursor_pos);
    
    // Chamar callback de texto alterado
    textbox_callbacks_t* callbacks = NULL;
    if (gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_TEXT_CHANGED, (void**)&callbacks) && 
        callbacks && callbacks->on_text_changed) {
        callbacks->on_text_changed(gui_element_get_id(element), new_text, callbacks->user_data);
    }
}

// Função para processar tecla de delete
static void process_delete(gui_element_t element) {
    if (!element) {
        return;
    }
    
    // Verificar se está em modo somente leitura
    bool read_only = DEFAULT_READ_ONLY;
    gui_element_get_property_bool(element, GUI_PROP_TEXTBOX_READ_ONLY, &read_only);
    if (read_only) {
        return;
    }
    
    // Obter texto atual
    char current_text[GUI_MAX_TEXT] = {0};
    strncpy(current_text, gui_element_get_text(element), GUI_MAX_TEXT - 1);
    
    // Obter posição do cursor
    int cursor_pos = DEFAULT_CURSOR_POS;
    gui_element_get_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, &cursor_pos);
    
    // Verificar se há caracteres para apagar
    size_t text_length = strlen(current_text);
    if (cursor_pos >= text_length) {
        return;
    }
    
    // Remover caractere após o cursor
    char new_text[GUI_MAX_TEXT] = {0};
    strncpy(new_text, current_text, cursor_pos);
    strncpy(new_text + cursor_pos, current_text + cursor_pos + 1, GUI_MAX_TEXT - cursor_pos - 1);
    
    // Atualizar texto
    gui_element_set_text(element, new_text);
    
    // Chamar callback de texto alterado
    textbox_callbacks_t* callbacks = NULL;
    if (gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_TEXT_CHANGED, (void**)&callbacks) && 
        callbacks && callbacks->on_text_changed) {
        callbacks->on_text_changed(gui_element_get_id(element), new_text, callbacks->user_data);
    }
}

// Função para mover o cursor
static void move_cursor(gui_element_t element, int direction) {
    if (!element) {
        return;
    }
    
    // Obter posição atual do cursor
    int cursor_pos = DEFAULT_CURSOR_POS;
    gui_element_get_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, &cursor_pos);
    
    // Obter comprimento do texto
    const char* text = gui_element_get_text(element);
    size_t text_length = text ? strlen(text) : 0;
    
    // Calcular nova posição
    cursor_pos += direction;
    
    // Limitar posição
    if (cursor_pos < 0) {
        cursor_pos = 0;
    } else if (cursor_pos > text_length) {
        cursor_pos = text_length;
    }
    
    // Atualizar posição
    gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, cursor_pos);
}

// Função de renderização da caixa de texto
static void textbox_render(gui_element_t element, SDL_Renderer* renderer) {
    if (!element || !renderer) {
        return;
    }

    // Obter retângulo da caixa de texto
    gui_rect_t rect;
    gui_element_get_rect(element, &rect);
    
    // Converter para SDL_Rect
    SDL_Rect sdl_rect = {
        rect.x, rect.y, rect.width, rect.height
    };
    
    // Obter cores
    gui_color_t bg_color = DEFAULT_BG_COLOR;
    gui_color_t border_color = DEFAULT_BORDER_COLOR;
    gui_color_t text_color = DEFAULT_TEXT_COLOR;
    
    gui_element_get_property_color(element, GUI_PROP_TEXTBOX_BG_COLOR, &bg_color);
    gui_element_get_property_color(element, GUI_PROP_TEXTBOX_BORDER_COLOR, &border_color);
    gui_element_get_property_color(element, GUI_PROP_TEXTBOX_TEXT_COLOR, &text_color);
    
    // Obter largura da borda
    int border_width = DEFAULT_BORDER_WIDTH;
    gui_element_get_property_int(element, GUI_PROP_TEXTBOX_BORDER_WIDTH, &border_width);
    
    // Verificar se está com foco
    bool focused = DEFAULT_FOCUSED;
    gui_element_get_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, &focused);
    
    // Renderizar fundo
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, &sdl_rect);
    
    // Renderizar borda
    if (border_width > 0) {
        SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
        
        // Se estiver com foco, desenhar borda mais grossa
        if (focused) {
            for (int i = 0; i < border_width + 1; i++) {
                SDL_Rect border_rect = {
                    sdl_rect.x - i, sdl_rect.y - i,
                    sdl_rect.w + 2 * i, sdl_rect.h + 2 * i
                };
                SDL_RenderDrawRect(renderer, &border_rect);
            }
        } else {
            for (int i = 0; i < border_width; i++) {
                SDL_Rect border_rect = {
                    sdl_rect.x - i, sdl_rect.y - i,
                    sdl_rect.w + 2 * i, sdl_rect.h + 2 * i
                };
                SDL_RenderDrawRect(renderer, &border_rect);
            }
        }
    }
    
    // Obter texto
    const char* text = gui_element_get_text(element);
    if (!text || strlen(text) == 0) {
        return;
    }
    
    // Calcular retângulo interno para o texto (com margem)
    SDL_Rect text_rect = {
        sdl_rect.x + 5, sdl_rect.y + 2,
        sdl_rect.w - 10, sdl_rect.h - 4
    };
    
    // Renderizar texto
    gui_render_text(renderer, text, &text_rect, &text_color, 0, 1); // Alinhado à esquerda e centralizado verticalmente
    
    // Renderizar cursor se estiver com foco
    if (focused) {
        int cursor_pos = DEFAULT_CURSOR_POS;
        gui_element_get_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, &cursor_pos);
        
        // Calcular posição do cursor
        int cursor_x = text_rect.x;
        if (cursor_pos > 0 && text) {
            char temp[GUI_MAX_TEXT] = {0};
            strncpy(temp, text, cursor_pos);
            temp[cursor_pos] = '\0';
            
            // Obter largura do texto até o cursor
            int text_width = 0;
            gui_get_text_size(temp, &text_width, NULL);
            cursor_x += text_width;
        }
        
        // Desenhar cursor (linha vertical)
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, text_color.a);
        SDL_RenderDrawLine(renderer, cursor_x, text_rect.y + 2, cursor_x, text_rect.y + text_rect.h - 2);
    }
}

// Função de processamento de eventos da caixa de texto
static bool textbox_process_event(gui_element_t element, const gui_event_t* event) {
    if (!element || !event) {
        return false;
    }
    
    // Verificar se está em modo somente leitura para eventos de teclado
    bool read_only = DEFAULT_READ_ONLY;
    gui_element_get_property_bool(element, GUI_PROP_TEXTBOX_READ_ONLY, &read_only);
    
    // Obter estado de foco
    bool focused = DEFAULT_FOCUSED;
    gui_element_get_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, &focused);
    
    // Processar eventos de mouse
    if (event->type == GUI_EVENT_MOUSE_BUTTON_DOWN) {
        // Verificar se o clique foi dentro da caixa de texto
        gui_rect_t rect;
        gui_element_get_rect(element, &rect);
        
        if (event->mouse.x >= rect.x && event->mouse.x < rect.x + rect.width &&
            event->mouse.y >= rect.y && event->mouse.y < rect.y + rect.height) {
            
            // Dar foco à caixa de texto
            if (!focused) {
                gui_element_set_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, true);
                return true;
            }
            
            // Calcular nova posição do cursor baseada no clique
            // (Simplificado - em uma implementação real, seria necessário calcular com base na largura de cada caractere)
            const char* text = gui_element_get_text(element);
            size_t text_length = text ? strlen(text) : 0;
            
            int relative_x = event->mouse.x - rect.x - 5; // 5 é a margem
            if (relative_x <= 0) {
                gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, 0);
            } else {
                // Estimativa simples - em uma implementação real, seria mais preciso
                int estimated_pos = (relative_x * text_length) / (rect.width - 10);
                if (estimated_pos > text_length) {
                    estimated_pos = text_length;
                }
                gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, estimated_pos);
            }
            
            return true;
        } else if (focused) {
            // Clique fora da caixa de texto - remover foco
            gui_element_set_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, false);
            
            // Chamar callback de perda de foco
            textbox_callbacks_t* callbacks = NULL;
            if (gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_FOCUS_LOST, (void**)&callbacks) && 
                callbacks && callbacks->on_focus_lost) {
                callbacks->on_focus_lost(gui_element_get_id(element), callbacks->user_data);
            }
        }
    }
    
    // Processar eventos de teclado apenas se estiver com foco
    if (focused) {
        if (event->type == GUI_EVENT_KEY_DOWN) {
            switch (event->key.keycode) {
                case SDLK_BACKSPACE:
                    if (!read_only) {
                        process_backspace(element);
                        return true;
                    }
                    break;
                    
                case SDLK_DELETE:
                    if (!read_only) {
                        process_delete(element);
                        return true;
                    }
                    break;
                    
                case SDLK_LEFT:
                    move_cursor(element, -1);
                    return true;
                    
                case SDLK_RIGHT:
                    move_cursor(element, 1);
                    return true;
                    
                case SDLK_HOME:
                    gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, 0);
                    return true;
                    
                case SDLK_END: {
                    const char* text = gui_element_get_text(element);
                    size_t text_length = text ? strlen(text) : 0;
                    gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, text_length);
                    return true;
                }
                
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    // Remover foco ao pressionar Enter
                    gui_element_set_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, false);
                    
                    // Chamar callback de perda de foco
                    textbox_callbacks_t* callbacks = NULL;
                    if (gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_FOCUS_LOST, (void**)&callbacks) && 
                        callbacks && callbacks->on_focus_lost) {
                        callbacks->on_focus_lost(gui_element_get_id(element), callbacks->user_data);
                    }
                    return true;
            }
        } else if (event->type == GUI_EVENT_TEXT_INPUT && !read_only) {
            process_text_input(element, event->text.text);
            return true;
        }
    }
    
    return false;
}

// Função de atualização da caixa de texto
static void textbox_update(gui_element_t element) {
    // Não há atualizações lógicas específicas para a caixa de texto
}

// Função de destruição da caixa de texto
static void textbox_destroy(gui_element_t element) {
    if (!element) {
        return;
    }
    
    // Liberar memória dos callbacks
    textbox_callbacks_t* callbacks = NULL;
    if (gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_TEXT_CHANGED, (void**)&callbacks) && callbacks) {
        free(callbacks);
    }
}

// Implementação das funções públicas
gui_element_id_t gui_textbox_create(const gui_rect_t* rect, const char* text) {
    if (!rect) {
        GUI_LOG_ERROR("Invalid rectangle for textbox");
        return GUI_INVALID_ID;
    }
    
    // Criar elemento base
    gui_element_t element = gui_element_create(GUI_ELEMENT_TEXTBOX, rect, text ? text : "");
    if (!element) {
        GUI_LOG_ERROR("Failed to create base element for textbox");
        return GUI_INVALID_ID;
    }
    
    // Configurar funções do elemento
    gui_element_set_render_func(element, textbox_render);
    gui_element_set_process_event_func(element, textbox_process_event);
    gui_element_set_update_func(element, textbox_update);
    gui_element_set_destroy_func(element, textbox_destroy);
    
    // Definir propriedades padrão
    gui_element_set_property_color(element, GUI_PROP_TEXTBOX_TEXT_COLOR, &DEFAULT_TEXT_COLOR);
    gui_element_set_property_color(element, GUI_PROP_TEXTBOX_BG_COLOR, &DEFAULT_BG_COLOR);
    gui_element_set_property_color(element, GUI_PROP_TEXTBOX_BORDER_COLOR, &DEFAULT_BORDER_COLOR);
    gui_element_set_property_int(element, GUI_PROP_TEXTBOX_BORDER_WIDTH, DEFAULT_BORDER_WIDTH);
    gui_element_set_property_size_t(element, GUI_PROP_TEXTBOX_MAX_LENGTH, DEFAULT_MAX_LENGTH);
    gui_element_set_property_bool(element, GUI_PROP_TEXTBOX_READ_ONLY, DEFAULT_READ_ONLY);
    gui_element_set_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, DEFAULT_FOCUSED);
    gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, DEFAULT_CURSOR_POS);
    
    // Alocar estrutura para callbacks
    textbox_callbacks_t* callbacks = (textbox_callbacks_t*)malloc(sizeof(textbox_callbacks_t));
    if (callbacks) {
        memset(callbacks, 0, sizeof(textbox_callbacks_t));
        gui_element_set_property_ptr(element, GUI_PROP_TEXTBOX_ON_TEXT_CHANGED, callbacks);
        gui_element_set_property_ptr(element, GUI_PROP_TEXTBOX_ON_FOCUS_LOST, callbacks);
    }
    
    return gui_element_get_id(element);
}

bool gui_textbox_set_text(gui_element_id_t textbox_id, const char* text) {
    if (textbox_id == GUI_INVALID_ID || !text) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    // Verificar tamanho máximo
    size_t max_length = DEFAULT_MAX_LENGTH;
    gui_element_get_property_size_t(element, GUI_PROP_TEXTBOX_MAX_LENGTH, &max_length);
    
    if (strlen(text) > max_length) {
        return false;
    }
    
    // Atualizar texto
    bool result = gui_element_set_text(element, text);
    
    // Ajustar posição do cursor se necessário
    int cursor_pos = DEFAULT_CURSOR_POS;
    gui_element_get_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, &cursor_pos);
    
    if (cursor_pos > strlen(text)) {
        gui_element_set_property_int(element, GUI_PROP_TEXTBOX_CURSOR_POS, strlen(text));
    }
    
    // Chamar callback de texto alterado
    textbox_callbacks_t* callbacks = NULL;
    if (result && gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_TEXT_CHANGED, (void**)&callbacks) && 
        callbacks && callbacks->on_text_changed) {
        callbacks->on_text_changed(textbox_id, text, callbacks->user_data);
    }
    
    return result;
}

bool gui_textbox_get_text(gui_element_id_t textbox_id, char* buffer, size_t buffer_size) {
    if (textbox_id == GUI_INVALID_ID || !buffer || buffer_size == 0) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    const char* text = gui_element_get_text(element);
    if (!text) {
        buffer[0] = '\0';
        return true;
    }
    
    strncpy(buffer, text, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    
    return true;
}

bool gui_textbox_set_text_color(gui_element_id_t textbox_id, const gui_color_t* color) {
    if (textbox_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    return gui_element_set_property_color(element, GUI_PROP_TEXTBOX_TEXT_COLOR, color);
}

bool gui_textbox_set_background_color(gui_element_id_t textbox_id, const gui_color_t* color) {
    if (textbox_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    return gui_element_set_property_color(element, GUI_PROP_TEXTBOX_BG_COLOR, color);
}

bool gui_textbox_set_border_color(gui_element_id_t textbox_id, const gui_color_t* color) {
    if (textbox_id == GUI_INVALID_ID || !color) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    return gui_element_set_property_color(element, GUI_PROP_TEXTBOX_BORDER_COLOR, color);
}

bool gui_textbox_set_border_width(gui_element_id_t textbox_id, int width) {
    if (textbox_id == GUI_INVALID_ID || width < 0) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    return gui_element_set_property_int(element, GUI_PROP_TEXTBOX_BORDER_WIDTH, width);
}

bool gui_textbox_set_max_length(gui_element_id_t textbox_id, size_t max_length) {
    if (textbox_id == GUI_INVALID_ID || max_length == 0) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    return gui_element_set_property_size_t(element, GUI_PROP_TEXTBOX_MAX_LENGTH, max_length);
}

bool gui_textbox_set_read_only(gui_element_id_t textbox_id, bool read_only) {
    if (textbox_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    return gui_element_set_property_bool(element, GUI_PROP_TEXTBOX_READ_ONLY, read_only);
}

bool gui_textbox_set_on_text_changed(gui_element_id_t textbox_id, void (*callback)(gui_element_id_t, const char*, void*), void* user_data) {
    if (textbox_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    textbox_callbacks_t* callbacks = NULL;
    if (!gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_TEXT_CHANGED, (void**)&callbacks) || !callbacks) {
        return false;
    }
    
    callbacks->on_text_changed = callback;
    callbacks->user_data = user_data;
    
    return true;
}

bool gui_textbox_set_on_focus_lost(gui_element_id_t textbox_id, void (*callback)(gui_element_id_t, void*), void* user_data) {
    if (textbox_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    textbox_callbacks_t* callbacks = NULL;
    if (!gui_element_get_property_ptr(element, GUI_PROP_TEXTBOX_ON_FOCUS_LOST, (void**)&callbacks) || !callbacks) {
        return false;
    }
    
    callbacks->on_focus_lost = callback;
    callbacks->user_data = user_data;
    
    return true;
}

bool gui_textbox_set_focused(gui_element_id_t textbox_id, bool focused) {
    if (textbox_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    return gui_element_set_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, focused);
}

bool gui_textbox_is_focused(gui_element_id_t textbox_id) {
    if (textbox_id == GUI_INVALID_ID) {
        return false;
    }
    
    gui_element_t element = gui_element_get_by_id(textbox_id);
    if (!element || gui_element_get_type(element) != GUI_ELEMENT_TEXTBOX) {
        return false;
    }
    
    bool focused = DEFAULT_FOCUSED;
    gui_element_get_property_bool(element, GUI_PROP_TEXTBOX_FOCUSED, &focused);
    
    return focused;
}
