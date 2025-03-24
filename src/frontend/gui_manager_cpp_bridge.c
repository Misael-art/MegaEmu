/**
 * @file gui_manager_cpp_bridge.c
 * @brief Implementação da ponte C++ para o gerenciador de GUI
 */

#include "gui_manager_cpp_bridge.h"
#include "gui_manager.h"
#include "gui/gui_types.h"
#include "gui/gui_common.h"
#include "utils/enhanced_log.h"
#include <stdlib.h>
#include <string.h>

// Instância global do gerenciador de GUI
static gui_manager_t g_gui_manager;
static bool g_save_state_visible = false;

// Callback para quando o save state é completado
static on_save_state_callback_t g_save_state_callback = NULL;

/**
 * @brief Cria uma instância do gerenciador de GUI
 *
 * @param renderer Renderizador SDL
 * @return Ponteiro para o gerenciador de GUI criado
 */
gui_manager_t *gui_manager_create(SDL_Renderer *renderer)
{
    gui_manager_t *manager = (gui_manager_t *)malloc(sizeof(gui_manager_t));
    if (!manager)
    {
        EMU_LOG_ERROR("Falha ao alocar memória para o gerenciador de GUI");
        return NULL;
    }
    memset(manager, 0, sizeof(gui_manager_t));
    manager->renderer = renderer;
    manager->running = true;
    manager->focused_element = GUI_INVALID_ID;
    manager->main_screen = GUI_INVALID_ID;
    manager->emu_frame = GUI_INVALID_ID;
    return manager;
}

/**
 * @brief Destrói uma instância do gerenciador de GUI
 *
 * @param manager Gerenciador de GUI a ser destruído
 */
void gui_manager_destroy(gui_manager_t *manager)
{
    if (!manager)
    {
        return;
    }
    gui_manager_shutdown(manager);
    free(manager);
}

/**
 * @brief Processa um evento para o gerenciador de GUI
 *
 * @param manager Gerenciador de GUI
 * @param event Evento a ser processado
 */
void gui_manager_process_event(gui_manager_t *manager, const gui_event_s *event)
{
    if (!manager || !event)
    {
        return;
    }
    gui_manager_process_callbacks(manager, event);
}

/**
 * @brief Cria um botão gerenciado pelo gerenciador de GUI
 *
 * @param manager Gerenciador de GUI
 * @param parent_id ID do elemento pai
 * @param text Texto do botão
 * @param x Posição X
 * @param y Posição Y
 * @param width Largura
 * @param height Altura
 * @param callback Função de callback
 * @param user_data Dados do usuário
 * @return ID do botão criado
 */
gui_element_id_t gui_manager_create_button(
    gui_manager_t *manager,
    gui_element_id_t parent_id,
    const char *text,
    int x, int y, int width, int height,
    void (*callback)(gui_element_s *, void *),
    void *user_data)
{
    if (!manager)
    {
        return GUI_INVALID_ID;
    }
    gui_element_id_t element_id;
    gui_rect_t rect = {x, y, width, height};
    gui_error_e result = gui_manager_add_element(manager, GUI_ELEMENT_BUTTON, &rect, &element_id);
    if (result != GUI_ERROR_NONE)
    {
        EMU_LOG_ERROR("Falha ao criar botão: %d", result);
        return GUI_INVALID_ID;
    }
    if (text)
    {
        gui_manager_set_element_text(manager, element_id, text);
    }
    if (parent_id != GUI_INVALID_ID)
    {
        gui_manager_add_child(manager, parent_id, element_id);
    }
    // Registrar callback (simplificado - na implementação real, seria necessário
    // adaptar o callback para o formato esperado pelo sistema de eventos)
    return element_id;
}

/**
 * @brief Define a visibilidade de um elemento
 *
 * @param manager Gerenciador de GUI
 * @param element_id ID do elemento
 * @param visible Visibilidade
 */
void gui_manager_set_element_visible(
    gui_manager_t *manager,
    gui_element_id_t element_id,
    bool visible)
{
    if (!manager || element_id == GUI_INVALID_ID)
    {
        return;
    }
    gui_element_s *element = gui_manager_get_element(manager, element_id);
    if (element)
    {
        element->visible = visible;
    }
}

/**
 * @brief Traz um elemento para a frente da hierarquia
 *
 * @param manager Gerenciador de GUI
 * @param element_id ID do elemento
 */
void gui_manager_bring_to_front(
    gui_manager_t *manager,
    gui_element_id_t element_id)
{
    // Implementação simplificada - na implementação real, seria necessário
    // reorganizar a ordem de renderização dos elementos
    if (!manager || element_id == GUI_INVALID_ID)
    {
        return;
    }
    // Apenas marca o elemento como focado
    manager->focused_element = element_id;
}

// Implementação das funções de ponte
bool gui_manager_create_window(const char *title, int width, int height)
{
    gui_error_e result = gui_manager_init(&g_gui_manager, title, width, height);
    return (result == GUI_ERROR_NONE);
}

void gui_manager_destroy_window()
{
    gui_manager_shutdown(&g_gui_manager);
}

void gui_manager_render()
{
    gui_manager_render(&g_gui_manager);
}

void gui_manager_update()
{
    gui_manager_update(&g_gui_manager);
}

bool gui_manager_process_events()
{
    return gui_manager_process_events(&g_gui_manager);
}

bool gui_save_state_get_state()
{
    return g_save_state_visible;
}

void gui_save_state_set_state(bool state)
{
    g_save_state_visible = state;
}

void render_save_state()
{
    if (g_save_state_visible)
    {
        EMU_LOG_INFO(EMU_LOG_CAT_GUI, "Renderizando interface de save state");
        // Implementação simplificada - apenas registra a chamada
    }
}

/**
 * @brief Callback para quando o save state é completado
 *
 * @param success Indica se a operação foi bem-sucedida
 */
void on_save_state_complete(bool success)
{
    EMU_LOG_INFO(EMU_LOG_CAT_GUI, "Save state %s", success ? "concluído com sucesso" : "falhou");
    if (g_save_state_callback)
    {
        g_save_state_callback(success);
    }
    // Ocultar a interface após a conclusão
    g_save_state_visible = false;
}
