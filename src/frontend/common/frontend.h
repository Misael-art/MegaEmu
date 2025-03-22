/** 
 * @file frontend.h 
 * @brief Interface principal para o frontend do emulador 
 */
#ifndef EMU_FRONTEND_H
#define EMU_FRONTEND_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../gui/core/gui_manager.h"
#include "frontend_config.h"

/**
 * @brief Handle opaco para o frontend do emulador
 */
typedef struct emu_frontend *emu_frontend_t;

/**
 * @brief Inicializa o frontend do emulador
 *
 * @param title Título da janela
 * @param width Largura da janela
 * @param height Altura da janela
 * @return emu_frontend_t Handle para o frontend inicializado, ou NULL em caso de erro
 */
emu_frontend_t emu_frontend_init(const char* title, int32_t width, int32_t height);

/**
 * @brief Finaliza o frontend do emulador e libera recursos
 *
 * @param frontend Handle para o frontend a ser finalizado
 */
void emu_frontend_shutdown(emu_frontend_t frontend);

/**
 * @brief Executa o loop principal do frontend
 *
 * @param frontend Handle para o frontend
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t emu_frontend_run(emu_frontend_t frontend);

/**
 * @brief Renderiza um frame no frontend
 *
 * @param frontend Handle para o frontend
 * @param framebuffer Buffer contendo os pixels do frame a ser renderizado
 * @param width Largura do framebuffer
 * @param height Altura do framebuffer
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t emu_frontend_render_frame(emu_frontend_t frontend, const uint32_t *framebuffer, int32_t width, int32_t height);

/**
 * @brief Atualiza a janela do frontend
 *
 * @param frontend Handle para o frontend
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t emu_frontend_update_window(emu_frontend_t frontend);

/**
 * @brief Processa eventos do frontend
 *
 * @param frontend Handle para o frontend
 * @return bool true se o frontend deve continuar executando, false caso contrário
 */
bool emu_frontend_process_events(emu_frontend_t frontend);

/**
 * @brief Obtém o gerenciador de GUI do frontend
 *
 * @param frontend Handle para o frontend
 * @return gui_manager_t Gerenciador de GUI ou NULL em caso de erro
 */
gui_manager_t emu_frontend_get_gui_manager(emu_frontend_t frontend);

/**
 * @brief Cria um elemento de GUI no frontend
 *
 * @param frontend Handle para o frontend
 * @param type Tipo do elemento
 * @param rect Retângulo do elemento
 * @param text Texto do elemento (pode ser NULL)
 * @return gui_element_id_t ID do elemento criado ou GUI_INVALID_ID em caso de erro
 */
gui_element_id_t emu_frontend_create_element(emu_frontend_t frontend, gui_element_type_t type, const gui_rect_t* rect, const char* text);

/**
 * @brief Remove um elemento de GUI do frontend
 *
 * @param frontend Handle para o frontend
 * @param element_id ID do elemento a ser removido
 */
void emu_frontend_remove_element(emu_frontend_t frontend, gui_element_id_t element_id);

/**
 * @brief Define a cor de fundo do frontend
 *
 * @param frontend Handle para o frontend
 * @param r Componente vermelho (0-255)
 * @param g Componente verde (0-255)
 * @param b Componente azul (0-255)
 * @param a Componente alfa (0-255)
 */
void emu_frontend_set_background_color(emu_frontend_t frontend, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

#ifdef __cplusplus
}
#endif

#endif /* EMU_FRONTEND_H */