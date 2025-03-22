/**
 * @file gui_label.h
 * @brief Interface para o widget de label (rótulo) da GUI
 */
#ifndef EMU_GUI_LABEL_H
#define EMU_GUI_LABEL_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "../core/gui_element.h"
#include "../core/gui_types.h"

/**
 * @brief Cria um label (rótulo)
 * 
 * @param rect Retângulo do label
 * @param text Texto do label
 * @return gui_element_id_t ID do label criado ou GUI_INVALID_ID em caso de erro
 */
gui_element_id_t gui_label_create(const gui_rect_t* rect, const char* text);

/**
 * @brief Define a cor do texto do label
 * 
 * @param label_id ID do label
 * @param color Cor do texto
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_label_set_text_color(gui_element_id_t label_id, const gui_color_t* color);

/**
 * @brief Define a cor de fundo do label
 * 
 * @param label_id ID do label
 * @param color Cor de fundo
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_label_set_background_color(gui_element_id_t label_id, const gui_color_t* color);

/**
 * @brief Define o alinhamento horizontal do texto do label
 * 
 * @param label_id ID do label
 * @param alignment Alinhamento (0 = esquerda, 1 = centro, 2 = direita)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_label_set_h_alignment(gui_element_id_t label_id, int alignment);

/**
 * @brief Define o alinhamento vertical do texto do label
 * 
 * @param label_id ID do label
 * @param alignment Alinhamento (0 = topo, 1 = centro, 2 = base)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_label_set_v_alignment(gui_element_id_t label_id, int alignment);

/**
 * @brief Define se o label tem fundo transparente
 * 
 * @param label_id ID do label
 * @param transparent true para fundo transparente, false para fundo colorido
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_label_set_transparent(gui_element_id_t label_id, bool transparent);

#endif /* EMU_GUI_LABEL_H */
