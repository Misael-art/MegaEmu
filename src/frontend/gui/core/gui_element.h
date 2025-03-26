/**
 * @file gui_element.h
 * @brief Interface para manipulação de elementos da GUI do Mega_Emu
 */
#ifndef EMU_GUI_ELEMENT_H
#define EMU_GUI_ELEMENT_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "gui_common.h"
#include "gui_types.h"

/**
 * @brief Inicializa o sistema de elementos da GUI
 * 
 * @return true se a inicialização foi bem-sucedida, false caso contrário
 */
bool gui_element_init(void);

/**
 * @brief Finaliza o sistema de elementos da GUI e libera recursos
 */
void gui_element_shutdown(void);

/**
 * @brief Cria um novo elemento da GUI
 * 
 * @param type Tipo do elemento a ser criado
 * @return gui_element_id_t ID do elemento criado ou GUI_INVALID_ID em caso de erro
 */
gui_element_id_t gui_element_create(gui_element_type_t type);

/**
 * @brief Destrói um elemento da GUI
 * 
 * @param id ID do elemento a ser destruído
 */
void gui_element_destroy(gui_element_id_t id);

/**
 * @brief Obtém um ponteiro para um elemento da GUI
 * 
 * @param id ID do elemento
 * @return gui_element_s* Ponteiro para o elemento ou NULL se não encontrado
 */
gui_element_s* gui_element_get(gui_element_id_t id);

/**
 * @brief Adiciona um elemento filho a um elemento pai
 * 
 * @param parent_id ID do elemento pai
 * @param child_id ID do elemento filho
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_add_child(gui_element_id_t parent_id, gui_element_id_t child_id);

/**
 * @brief Remove um elemento filho de um elemento pai
 * 
 * @param parent_id ID do elemento pai
 * @param child_id ID do elemento filho
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_remove_child(gui_element_id_t parent_id, gui_element_id_t child_id);

/**
 * @brief Define o texto de um elemento
 * 
 * @param id ID do elemento
 * @param text Texto a ser definido
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_set_text(gui_element_id_t id, const char* text);

/**
 * @brief Define a posição de um elemento
 * 
 * @param id ID do elemento
 * @param x Coordenada X
 * @param y Coordenada Y
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_set_position(gui_element_id_t id, int32_t x, int32_t y);

/**
 * @brief Define o tamanho de um elemento
 * 
 * @param id ID do elemento
 * @param width Largura
 * @param height Altura
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_set_size(gui_element_id_t id, int32_t width, int32_t height);

/**
 * @brief Define a visibilidade de um elemento
 * 
 * @param id ID do elemento
 * @param visible true para visível, false para invisível
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_set_visible(gui_element_id_t id, bool visible);

/**
 * @brief Define se um elemento está habilitado
 * 
 * @param id ID do elemento
 * @param enabled true para habilitado, false para desabilitado
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_set_enabled(gui_element_id_t id, bool enabled);

/**
 * @brief Define uma propriedade de um elemento
 * 
 * @param id ID do elemento
 * @param name Nome da propriedade
 * @param type Tipo da propriedade
 * @param value Valor da propriedade
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool gui_element_set_property(gui_element_id_t id, const char* name, gui_property_type_t type, const void* value);

/**
 * @brief Obtém uma propriedade de um elemento
 * 
 * @param id ID do elemento
 * @param name Nome da propriedade
 * @param type Tipo da propriedade (saída)
 * @param value Valor da propriedade (saída)
 * @return true se a propriedade foi encontrada, false caso contrário
 */
bool gui_element_get_property(gui_element_id_t id, const char* name, gui_property_type_t* type, void* value);

/**
 * @brief Processa um evento para todos os elementos
 * 
 * @param event Evento a ser processado
 * @return true se o evento foi processado, false caso contrário
 */
bool gui_element_process_event(const gui_event_t* event);

/**
 * @brief Atualiza todos os elementos
 */
void gui_element_update(void);

/**
 * @brief Renderiza todos os elementos
 * 
 * @param renderer Renderizador SDL
 */
void gui_element_render(SDL_Renderer* renderer);

#endif /* EMU_GUI_ELEMENT_H */
