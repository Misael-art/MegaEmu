/**
 * @file gui_manager.h
 * @brief Interface para o gerenciador de GUI do Mega_Emu
 */
#ifndef EMU_GUI_MANAGER_H
#define EMU_GUI_MANAGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "gui_element.h"
#include "gui_types.h"

/**
 * @brief Tipo opaco para o gerenciador de GUI
 */
typedef struct gui_manager_s* gui_manager_t;

/**
 * @brief Tipo para callback de evento
 */
typedef bool (*gui_event_callback_t)(gui_manager_t manager, const gui_event_t* event, void* user_data);

/**
 * @brief Inicializa o gerenciador de GUI
 * 
 * @return gui_manager_t Ponteiro para o gerenciador ou NULL em caso de erro
 */
gui_manager_t gui_manager_init(void);

/**
 * @brief Finaliza o gerenciador de GUI e libera recursos
 * 
 * @param manager Gerenciador de GUI
 */
void gui_manager_shutdown(gui_manager_t manager);

/**
 * @brief Adiciona um elemento ao gerenciador
 * 
 * @param manager Gerenciador de GUI
 * @param type Tipo do elemento
 * @param rect Retângulo do elemento
 * @param text Texto do elemento (pode ser NULL)
 * @return gui_element_id_t ID do elemento criado ou GUI_INVALID_ID em caso de erro
 */
gui_element_id_t gui_manager_add_element(gui_manager_t manager, gui_element_type_t type, const gui_rect_t* rect, const char* text);

/**
 * @brief Remove um elemento do gerenciador
 * 
 * @param manager Gerenciador de GUI
 * @param element_id ID do elemento a ser removido
 */
void gui_manager_remove_element(gui_manager_t manager, gui_element_id_t element_id);

/**
 * @brief Obtém um elemento do gerenciador
 * 
 * @param manager Gerenciador de GUI
 * @param element_id ID do elemento
 * @return gui_element_s* Ponteiro para o elemento ou NULL se não encontrado
 */
gui_element_s* gui_manager_get_element(gui_manager_t manager, gui_element_id_t element_id);

/**
 * @brief Registra um callback para um tipo de evento
 * 
 * @param manager Gerenciador de GUI
 * @param event_type Tipo de evento
 * @param callback Função de callback
 * @param user_data Dados do usuário a serem passados para o callback
 * @return uint32_t ID do callback ou 0 em caso de erro
 */
uint32_t gui_manager_register_callback(gui_manager_t manager, gui_event_type_t event_type, gui_event_callback_t callback, void* user_data);

/**
 * @brief Remove um callback registrado
 * 
 * @param manager Gerenciador de GUI
 * @param callback_id ID do callback a ser removido
 * @return true se o callback foi removido, false caso contrário
 */
bool gui_manager_unregister_callback(gui_manager_t manager, uint32_t callback_id);

/**
 * @brief Processa um evento
 * 
 * @param manager Gerenciador de GUI
 * @param event Evento a ser processado
 * @return true se o evento foi processado, false caso contrário
 */
bool gui_manager_process_event(gui_manager_t manager, const gui_event_t* event);

/**
 * @brief Atualiza todos os elementos do gerenciador
 * 
 * @param manager Gerenciador de GUI
 */
void gui_manager_update(gui_manager_t manager);

/**
 * @brief Renderiza todos os elementos do gerenciador
 * 
 * @param manager Gerenciador de GUI
 * @param renderer Renderizador SDL
 */
void gui_manager_render(gui_manager_t manager, SDL_Renderer* renderer);

/**
 * @brief Converte um evento SDL para um evento GUI
 * 
 * @param sdl_event Evento SDL
 * @param gui_event Evento GUI (saída)
 * @return true se o evento foi convertido, false caso contrário
 */
bool gui_manager_convert_sdl_event(const SDL_Event* sdl_event, gui_event_t* gui_event);

/**
 * @brief Encontra o elemento na posição especificada
 * 
 * @param manager Gerenciador de GUI
 * @param x Coordenada X
 * @param y Coordenada Y
 * @return gui_element_id_t ID do elemento encontrado ou GUI_INVALID_ID se nenhum elemento for encontrado
 */
gui_element_id_t gui_manager_find_element_at(gui_manager_t manager, int32_t x, int32_t y);

#endif /* EMU_GUI_MANAGER_H */
