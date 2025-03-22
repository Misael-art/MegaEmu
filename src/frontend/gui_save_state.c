/** * @file gui_save_state.c * @brief Implementação da interface gráfica para gerenciamento de save states * @author Mega_Emu Team * @version 1.0.0 * @date 2024-07-25 */#include "frontend/gui_save_state.h"#include "frontend/gui_manager.h"#include "gui/gui_common.h"#include "utils/save_state.h"#include "utils/file_utils.h"#include <stdio.h>#include <stdlib.h>#include <string.h>#include <time.h>#include <SDL2/SDL.h>#ifdef USE_SDL2_TTF#include <SDL2/SDL_ttf.h>#endif#define MAX_SLOTS MAX_SAVE_SLOTS#ifdef _WIN32#include <windows.h>#define MAX_PATH 260#else#define MAX_PATH 4096#endif// Variáveis globais para o módulostatic gui_save_state_t g_save_state;static bool g_initialized = false;// Estruturas auxiliarestypedef struct gui_save_state_context_s{    bool visible;    bool is_loading;    int selected_slot;    void (*callback)(bool success, bool is_load, int slot_index, void *user_data);    void *user_data;} gui_save_state_context_t;static gui_save_state_context_t g_context;// Funções estáticas auxiliaresstatic void init_colors(void);static void clear_slot(int slot);static void clear_all_slots(void);static void update_slot_info(int slot);static void render_slot(int slot, SDL_Renderer *renderer);static bool handle_mouse_click(int x, int y);static void handle_key_press(SDL_Keycode key);static char *get_slot_path(int slot);static void format_timestamp(time_t timestamp, char *buffer, size_t size);static char *format_date_time(time_t timestamp);gui_error_t gui_save_state_init(gui_manager_t *manager, const char *platform_name){    if (!manager || !platform_name)        return GUI_ERROR_INVALID_PARAMETER;    // Inicializa o estado de save global    g_save_state.selected_slot = -1;    g_save_state.visible = false;    g_save_state.enabled = true;    // Inicializa todos os slots como vazios    for (int i = 0; i < MAX_SAVE_SLOTS; i++)    {        g_save_state.slots[i].in_use = false;        g_save_state.slots[i].description[0] = '\0';        g_save_state.slots[i].filename[0] = '\0';        g_save_state.slots[i].timestamp = 0;        g_save_state.slots[i].size = 0;        g_save_state.slots[i].has_screenshot = false;    }    // Inicializa callbacks como NULL    g_save_state.on_save = NULL;    g_save_state.on_load = NULL;    g_save_state.on_delete = NULL;    g_save_state.user_data = NULL;    // Inicializa o contexto da GUI    g_context.visible = false;    g_context.is_loading = false;    g_context.selected_slot = -1;    g_context.callback = NULL;    g_context.user_data = NULL;    // Inicializa as propriedades base do elemento    g_save_state.base.id = 0; // Será definido pelo gerenciador    g_save_state.base.type = GUI_ELEMENT_CUSTOM;    gui_element_set_bounds(&g_save_state.base, 100, 100, 400, 300);    g_save_state.base.state.visible = false;    g_save_state.base.state.enabled = true;    g_save_state.base.num_children = 0;    g_save_state.base.property_count = 0;    // Define as propriedades padrão    gui_element_set_property_string(&g_save_state.base, GUI_PROP_SAVE_DIR, "saves");    gui_element_set_property_int(&g_save_state.base, GUI_PROP_PLATFORM_ID, 0);    gui_element_set_property_string(&g_save_state.base, GUI_PROP_GAME_TITLE, platform_name);    // Inicializa as cores    init_colors();    g_initialized = true;    return GUI_ERROR_SUCCESS;}void gui_save_state_shutdown(void){    if (!g_initialized)        return;    // Limpa os slots    for (int i = 0; i < MAX_SAVE_SLOTS; i++)    {        g_save_state.slots[i].in_use = false;    }    g_initialized = false;}gui_error_t gui_save_state_save(int slot_index){    if (slot_index < 0 || slot_index >= MAX_SAVE_SLOTS)        return GUI_ERROR_INVALID_PARAMETER;    save_slot_t *slot_data = &g_save_state.slots[slot_index];    slot_data->in_use = true;    slot_data->timestamp = time(NULL);    // Gera descrição padrão se não houver    snprintf(slot_data->description, sizeof(slot_data->description),             "Save %d - %s", slot_index + 1, format_date_time(slot_data->timestamp));    // Gera nome de arquivo baseado no slot e plataforma    snprintf(slot_data->filename, sizeof(slot_data->filename),             "slot_%d.sav", slot_index);    // Chama o callback de save se registrado    if (g_save_state.on_save)    {        g_save_state.on_save(slot_index, g_save_state.user_data);    }    return GUI_ERROR_SUCCESS;}gui_error_t gui_save_state_load(int slot_index){    if (slot_index < 0 || slot_index >= MAX_SAVE_SLOTS)        return GUI_ERROR_INVALID_PARAMETER;    if (!g_save_state.slots[slot_index].in_use)        return GUI_ERROR_NOT_FOUND;    // Chama o callback de load se registrado    if (g_save_state.on_load)    {        g_save_state.on_load(slot_index, g_save_state.user_data);    }    return GUI_ERROR_SUCCESS;}gui_error_t gui_save_state_delete(int slot_index){    if (slot_index < 0 || slot_index >= MAX_SAVE_SLOTS)        return GUI_ERROR_INVALID_PARAMETER;    save_slot_t *slot_data = &g_save_state.slots[slot_index];    if (!slot_data->in_use)        return GUI_ERROR_NOT_FOUND;    // Limpa os dados do slot    slot_data->in_use = false;    slot_data->description[0] = '\0';    slot_data->timestamp = 0;    slot_data->size = 0;    // Remove o arquivo físico se existir    char *filepath = get_slot_path(slot_index);    if (filepath)    {        remove(filepath);        free(filepath);    }    // Chama o callback de delete se registrado    if (g_save_state.on_delete)    {        g_save_state.on_delete(slot_index, g_save_state.user_data);    }    return GUI_ERROR_SUCCESS;}void gui_save_state_show(bool for_loading,                         void (*callback)(bool success, bool is_load, int slot_index, void *user_data),                         void *user_data){    g_save_state.visible = true;    g_save_state.base.state.visible = true;    g_context.is_loading = for_loading;    g_context.callback = callback;    g_context.user_data = user_data;    // Atualiza informações de todos os slots    for (int i = 0; i < MAX_SAVE_SLOTS; i++)    {        update_slot_info(i);    }}void gui_save_state_hide(void){    g_save_state.visible = false;    g_save_state.base.state.visible = false;}bool gui_save_state_is_visible(void){    return g_save_state.visible;}void gui_save_state_clear_slot(gui_save_state_t *state, int32_t slot){    if (!state || slot < 0 || slot >= MAX_SAVE_SLOTS)        return;    clear_slot(slot);}void gui_save_state_clear_all(gui_save_state_t *state){    if (!state)        return;    clear_all_slots();}void gui_save_state_set_position(gui_save_state_t *state, int32_t x, int32_t y){    if (!state)        return;    // Atualiza a posição da viewport    gui_element_set_position(&state->base, x, y);}void gui_save_state_set_size(gui_save_state_t *state, int32_t width, int32_t height){    if (!state)        return;    // Atualiza o tamanho da viewport    gui_element_set_size(&state->base, width, height);}void gui_save_state_set_save_dir(gui_save_state_t *state, const char *dir){    if (!state || !dir)        return;    // Armazena o diretório de save    gui_element_set_property_string(&state->base, GUI_PROP_SAVE_DIR, dir);}void gui_save_state_set_platform(gui_save_state_t *state, uint32_t platform_id, const char *game_title){    if (!state || !game_title)        return;    // Armazena o ID da plataforma e título do jogo    gui_element_set_property_int(&state->base, GUI_PROP_PLATFORM_ID, platform_id);    gui_element_set_property_string(&state->base, GUI_PROP_GAME_TITLE, game_title);}void gui_save_state_set_callbacks(gui_save_state_t *state, save_state_callback_t on_save, save_state_callback_t on_load){    if (!state)        return;    // Configura os callbacks    state->on_save = on_save;    state->on_load = on_load;}bool gui_save_state_handle_event(gui_save_state_t *state, const gui_event_t *event){    if (!state || !state->visible || !event)        return false;    // Implementação de tratamento de eventos específicos para o save state    if (event->type == GUI_EVENT_MOUSE_BUTTON) {        if (event->data.mouse_button.state == GUI_BUTTON_PRESSED) {            return handle_mouse_click(event->data.mouse_button.point.x,                                     event->data.mouse_button.point.y);        }    }    else if (event->type == GUI_EVENT_KEY_DOWN) {        handle_key_press(event->data.key_code);        return true;    }    return false;}void gui_save_state_render(gui_save_state_t *state){    if (!state || !state->visible)        return;    // Esta função seria implementada para renderizar o save state    // usando o renderer associado ao gerenciador de GUI}// Implementação das funções auxiliares estáticasstatic void init_colors(void){    // Define as cores usadas para os elementos da interface usando o novo sistema de propriedades    gui_element_set_property_color(&g_save_state.base, GUI_PROP_BACKGROUND_COLOR,                                  gui_color_create(32, 32, 32, 255));    gui_element_set_property_color(&g_save_state.base, GUI_PROP_BORDER_COLOR,                                  gui_color_create(64, 64, 64, 255));    gui_element_set_property_color(&g_save_state.base, GUI_PROP_FOREGROUND_COLOR,                                  gui_color_create(48, 48, 48, 255));    gui_element_set_property_color(&g_save_state.base, GUI_PROP_SELECTED_COLOR,                                  gui_color_create(64, 96, 128, 255));}static void clear_slot(int slot){    if (slot < 0 || slot >= MAX_SLOTS)        return;    save_slot_t *s = &g_save_state.slots[slot];    memset(s, 0, sizeof(save_slot_t));    s->timestamp = 0;    s->in_use = false;    s->has_screenshot = false;}static void clear_all_slots(void){    for (int i = 0; i < MAX_SLOTS; i++)    {        clear_slot(i);    }}static void update_slot_info(int slot){    if (slot < 0 || slot >= MAX_SLOTS)        return;    char *path = get_slot_path(slot);    if (!path)        return;    save_slot_t *s = &g_save_state.slots[slot];    // Verifica se o arquivo existe    if (file_exists(path))    {        s->in_use = true;        s->timestamp = get_file_modification_time(path);        s->has_screenshot = false; // TODO: Implementar detecção de screenshot    }    else    {        clear_slot(slot);    }    free(path);}static void render_slot(int slot, SDL_Renderer *renderer){    if (slot < 0 || slot >= MAX_SLOTS || !renderer)        return;    // Placeholder para a implementação da renderização de slot    // Esta função seria chamada pela função gui_save_state_render}static bool handle_mouse_click(int x, int y){    // Verifica se o clique foi dentro da área do save state    gui_rect_t bounds = gui_element_get_bounds(&g_save_state.base);    if (!gui_point_in_rect(&bounds, x, y))        return false;    // Calcula o slot clicado com base nas coordenadas    int slot_height = 50; // Altura de cada slot    int slot_y = bounds.y + GUI_DEFAULT_MARGIN;    for (int i = 0; i < MAX_SLOTS; i++) {        gui_rect_t slot_rect = gui_rect_create(            bounds.x + GUI_DEFAULT_MARGIN,            slot_y,            bounds.width - (GUI_DEFAULT_MARGIN * 2),            slot_height - GUI_DEFAULT_SPACING        );        if (gui_point_in_rect(&slot_rect, x, y)) {            if (g_save_state.selected_slot == i) {                // Clicou no mesmo slot - executa a ação                if (g_context.is_loading) {                    if (g_save_state.slots[i].in_use) {                        gui_save_state_load(i);                        gui_save_state_hide();                        if (g_context.callback) {                            g_context.callback(true, true, i, g_context.user_data);                        }                    }                } else {                    gui_save_state_save(i);                    gui_save_state_hide();                    if (g_context.callback) {                        g_context.callback(true, false, i, g_context.user_data);                    }                }            } else {                // Seleciona o novo slot                g_save_state.selected_slot = i;            }            return true;        }        slot_y += slot_height;    }    return false;}static void handle_key_press(SDL_Keycode key){    switch (key) {        case SDLK_ESCAPE:            gui_save_state_hide();            if (g_context.callback) {                g_context.callback(false, g_context.is_loading, -1, g_context.user_data);            }            break;        case SDLK_UP:            if (g_save_state.selected_slot > 0) {                g_save_state.selected_slot--;            }            break;        case SDLK_DOWN:            if (g_save_state.selected_slot < MAX_SLOTS - 1) {                g_save_state.selected_slot++;            }            break;        case SDLK_RETURN:            if (g_save_state.selected_slot >= 0) {                if (g_context.is_loading) {                    if (g_save_state.slots[g_save_state.selected_slot].in_use) {                        gui_save_state_load(g_save_state.selected_slot);                        gui_save_state_hide();                        if (g_context.callback) {                            g_context.callback(true, true, g_save_state.selected_slot, g_context.user_data);                        }                    }                } else {                    gui_save_state_save(g_save_state.selected_slot);                    gui_save_state_hide();                    if (g_context.callback) {                        g_context.callback(true, false, g_save_state.selected_slot, g_context.user_data);                    }                }            }            break;        case SDLK_DELETE:            if (g_save_state.selected_slot >= 0 &&                g_save_state.slots[g_save_state.selected_slot].in_use) {                gui_save_state_delete(g_save_state.selected_slot);            }            break;        default:            break;    }}static char *get_slot_path(int slot){    if (slot < 0 || slot >= MAX_SLOTS)        return NULL;    // Aloca espaço para o caminho completo    char *path = malloc(MAX_PATH);    if (!path)        return NULL;    // Obtém o diretório de saves e título do jogo    char save_dir[GUI_MAX_PROPERTY_VALUE] = {0};    char game_title[GUI_MAX_PROPERTY_VALUE] = {0};    gui_element_get_property_string(&g_save_state.base, GUI_PROP_SAVE_DIR,                                   save_dir, GUI_MAX_PROPERTY_VALUE);    gui_element_get_property_string(&g_save_state.base, GUI_PROP_GAME_TITLE,                                   game_title, GUI_MAX_PROPERTY_VALUE);    if (!save_dir[0] || !game_title[0])    {        free(path);        return NULL;    }    // Formata o nome do arquivo    snprintf(path, MAX_PATH, "%s/%s_slot%d.sav",             save_dir,             game_title,             slot);    return path;}static void format_timestamp(time_t timestamp, char *buffer, size_t size){    if (!buffer || size == 0)        return;    struct tm *timeinfo = localtime(&timestamp);    if (timeinfo)    {        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);    }    else    {        strncpy(buffer, "Data desconhecida", size - 1);        buffer[size - 1] = '\0';    }}// Utilitário para formatar data/hora para exibiçãostatic char *format_date_time(time_t timestamp){    static char buffer[64];    struct tm *timeinfo = localtime(&timestamp);    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", timeinfo);    return buffer;}