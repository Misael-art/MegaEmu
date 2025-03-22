/**
 * @file gui_button.h
 * @brief Interface para o widget de botão da GUI
 */
#ifndef EMU_GUI_BUTTON_H
#define EMU_GUI_BUTTON_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "../core/gui_element.h"
#include "../core/gui_types.h"

/**
 * @brief Cria um botão
 * 
 * @param rect Retângulo do botão
 * @param text Texto do botão
 * @return gui_element_id_t ID do botão criado ou GUI_INVALID_ID em caso de erro
 */
gui_element_id_t gui_button_create(const gui_rect_t* rect, const char* text);

/**
 * @brief Define o callback para o evento de clique do botão
 * 
 * @param button_id ID do botão
 * @param callback Função de callback
 * @param user_data Dados do usuário a serem passados para o callback
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_button_set_click_callback(gui_element_id_t button_id, void (*callback)(gui_element_id_t, void*), void* user_data);

/**
 * @brief Define a cor do texto do botão
 * 
 * @param button_id ID do botão
 * @param color Cor do texto
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_button_set_text_color(gui_element_id_t button_id, const gui_color_t* color);

/**
 * @brief Define a cor de fundo do botão
 * 
 * @param button_id ID do botão
 * @param color Cor de fundo
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_button_set_background_color(gui_element_id_t button_id, const gui_color_t* color);

/**
 * @brief Define a cor de fundo do botão quando hover
 * 
 * @param button_id ID do botão
 * @param color Cor de fundo quando hover
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_button_set_hover_color(gui_element_id_t button_id, const gui_color_t* color);

/**
 * @brief Define a cor de fundo do botão quando pressionado
 * 
 * @param button_id ID do botão
 * @param color Cor de fundo quando pressionado
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_button_set_pressed_color(gui_element_id_t button_id, const gui_color_t* color);

#endif /* EMU_GUI_BUTTON_H */
