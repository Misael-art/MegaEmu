/**
 * @file gui_textbox.h
 * @brief Interface para o widget de caixa de texto da GUI
 */
#ifndef EMU_GUI_TEXTBOX_H
#define EMU_GUI_TEXTBOX_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "../core/gui_element.h"
#include "../core/gui_types.h"

/**
 * @brief Cria uma caixa de texto
 * 
 * @param rect Retângulo da caixa de texto
 * @param text Texto inicial da caixa de texto
 * @return gui_element_id_t ID da caixa de texto criada ou GUI_INVALID_ID em caso de erro
 */
gui_element_id_t gui_textbox_create(const gui_rect_t* rect, const char* text);

/**
 * @brief Define o texto da caixa de texto
 * 
 * @param textbox_id ID da caixa de texto
 * @param text Novo texto
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_text(gui_element_id_t textbox_id, const char* text);

/**
 * @brief Obtém o texto atual da caixa de texto
 * 
 * @param textbox_id ID da caixa de texto
 * @param buffer Buffer para armazenar o texto
 * @param buffer_size Tamanho do buffer
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_get_text(gui_element_id_t textbox_id, char* buffer, size_t buffer_size);

/**
 * @brief Define a cor do texto da caixa de texto
 * 
 * @param textbox_id ID da caixa de texto
 * @param color Cor do texto
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_text_color(gui_element_id_t textbox_id, const gui_color_t* color);

/**
 * @brief Define a cor de fundo da caixa de texto
 * 
 * @param textbox_id ID da caixa de texto
 * @param color Cor de fundo
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_background_color(gui_element_id_t textbox_id, const gui_color_t* color);

/**
 * @brief Define a cor da borda da caixa de texto
 * 
 * @param textbox_id ID da caixa de texto
 * @param color Cor da borda
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_border_color(gui_element_id_t textbox_id, const gui_color_t* color);

/**
 * @brief Define a largura da borda da caixa de texto
 * 
 * @param textbox_id ID da caixa de texto
 * @param width Largura da borda em pixels
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_border_width(gui_element_id_t textbox_id, int width);

/**
 * @brief Define o tamanho máximo de texto permitido
 * 
 * @param textbox_id ID da caixa de texto
 * @param max_length Tamanho máximo de texto
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_max_length(gui_element_id_t textbox_id, size_t max_length);

/**
 * @brief Define se a caixa de texto está em modo somente leitura
 * 
 * @param textbox_id ID da caixa de texto
 * @param read_only true para somente leitura, false para permitir edição
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_read_only(gui_element_id_t textbox_id, bool read_only);

/**
 * @brief Define um callback para ser chamado quando o texto for alterado
 * 
 * @param textbox_id ID da caixa de texto
 * @param callback Função de callback
 * @param user_data Dados de usuário a serem passados para o callback
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_on_text_changed(gui_element_id_t textbox_id, void (*callback)(gui_element_id_t, const char*, void*), void* user_data);

/**
 * @brief Define um callback para ser chamado quando a caixa de texto perder o foco
 * 
 * @param textbox_id ID da caixa de texto
 * @param callback Função de callback
 * @param user_data Dados de usuário a serem passados para o callback
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_on_focus_lost(gui_element_id_t textbox_id, void (*callback)(gui_element_id_t, void*), void* user_data);

/**
 * @brief Define se a caixa de texto está com foco
 * 
 * @param textbox_id ID da caixa de texto
 * @param focused true para dar foco, false para remover
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_textbox_set_focused(gui_element_id_t textbox_id, bool focused);

/**
 * @brief Verifica se a caixa de texto está com foco
 * 
 * @param textbox_id ID da caixa de texto
 * @return true se a caixa de texto está com foco, false caso contrário
 */
bool gui_textbox_is_focused(gui_element_id_t textbox_id);

#endif /* EMU_GUI_TEXTBOX_H */
