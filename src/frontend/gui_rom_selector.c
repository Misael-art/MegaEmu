/** * @file gui_rom_selector.c * @brief Implementação do seletor de ROMs para o Mega_Emu */#include <SDL2/SDL.h>#ifdef USE_SDL2_TTF#include <SDL2/SDL_ttf.h>#endif#ifdef _WIN32#include <windows.h>#else#include <dirent.h>#include <sys/stat.h>#endif#include <string.h>#include <stdio.h>#include <stdlib.h>#include <ctype.h>#include <stdarg.h>#include "frontend/gui_rom_selector.h"#include "utils/error_handling.h"#include "utils/enhanced_log.h"#include "frontend/console_types.h"#include "gui_types.h"#include "gui_manager.h"#include "utils/file_utils.h"#ifndef EMU_UNUSED#define EMU_UNUSED(x) (void)(x)#endif#define MAX_FILES 500#define FONT_SIZE 16// Definições#define ROM_SELECTOR_MAX_CONSOLES CONSOLE_COUNT#define MAX_SLOTS 10// Definição das cores#define COLOR_BACKGROUND_R 32#define COLOR_BACKGROUND_G 32#define COLOR_BACKGROUND_B 32#define COLOR_BACKGROUND_A 255#define COLOR_TEXT_R 255#define COLOR_TEXT_G 255#define COLOR_TEXT_B 255#define COLOR_TEXT_A 255#define COLOR_SELECTED_R 64#define COLOR_SELECTED_G 128#define COLOR_SELECTED_B 255#define COLOR_SELECTED_A 255#define COLOR_CONSOLE_TAB_R 48#define COLOR_CONSOLE_TAB_G 48#define COLOR_CONSOLE_TAB_B 48#define COLOR_CONSOLE_TAB_A 255#define COLOR_CONSOLE_TAB_SELECTED_R 64#define COLOR_CONSOLE_TAB_SELECTED_G 64#define COLOR_CONSOLE_TAB_SELECTED_B 64#define COLOR_CONSOLE_TAB_SELECTED_A 255#define COLOR_SELECTED_TAB_R 64#define COLOR_SELECTED_TAB_G 64#define COLOR_SELECTED_TAB_B 64#define COLOR_SELECTED_TAB_A 255// Estrutura estendida para o seletor de ROMs com campos adicionaistypedef struct gui_rom_selector_extended_s{    gui_rom_selector_t base;    SDL_Rect viewport;    int scroll_position;    int font_size;    int num_roms;    rom_info_t roms[ROM_SELECTOR_MAX_ROMS];#ifdef USE_SDL2_TTF    TTF_Font *font;#endif} gui_rom_selector_extended_t;// Instância global do seletor de ROMsstatic gui_rom_selector_extended_t g_rom_selector;// Função para converter Color para SDL_Colorstatic SDL_Color to_sdl_color(gui_color_t color){    SDL_Color sdl_color;    sdl_color.r = color.r;    sdl_color.g = color.g;    sdl_color.b = color.b;    sdl_color.a = color.a;    return sdl_color;}// Função auxiliar para renderizar textostatic void render_text(SDL_Renderer *renderer,#ifdef USE_SDL2_TTF                        TTF_Font *font,#endif                        const char *text, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a){#ifdef USE_SDL2_TTF    if (font == NULL || text == NULL)        return;    SDL_Color color = {r, g, b, a};    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);    if (surface == NULL)    {        EMU_LOG_ERROR(EMU_LOG_CAT_GUI, "Falha ao renderizar texto: %s", TTF_GetError());        return;    }    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);    if (texture == NULL)    {        EMU_LOG_ERROR(EMU_LOG_CAT_GUI, "Falha ao criar textura para texto: %s", SDL_GetError());        SDL_FreeSurface(surface);        return;    }    SDL_Rect dest = {x, y, surface->w, surface->h};    SDL_RenderCopy(renderer, texture, NULL, &dest);    SDL_FreeSurface(surface);    SDL_DestroyTexture(texture);#else    // Modo de fallback - renderiza um retângulo colorido no lugar do texto    EMU_UNUSED(text);    SDL_Rect text_rect = {x, y, (int)strlen(text) * 8, 16}; // Estimativa simples    SDL_SetRenderDrawColor(renderer, r, g, b, a);    SDL_RenderFillRect(renderer, &text_rect);#endif}// Inicializa o seletor de ROMsgui_error_t gui_rom_selector_init(gui_rom_selector_t *selector){    if (!selector)        return GUI_ERROR_INVALID_PARAMETER;    // Inicializar campos base    memset(selector, 0, sizeof(gui_rom_selector_t));    selector->visible = false;    selector->enabled = true;    selector->bounds = (gui_rect_t){50, 50, 700, 500};    selector->background_color = (gui_color_t){40, 42, 54, 255};    selector->text_color = (gui_color_t){255, 255, 255, 255};    selector->selection_color = (gui_color_t){80, 80, 180, 255};    selector->border_color = (gui_color_t){120, 122, 134, 255};    selector->selected_index = 0;    selector->current_console = CONSOLE_NES;    // Se for nossa instância global, inicializar campos estendidos    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        g_rom_selector.viewport = (SDL_Rect){            selector->bounds.x,            selector->bounds.y,            selector->bounds.width,            selector->bounds.height};        g_rom_selector.scroll_position = 0;        g_rom_selector.font_size = 20;        g_rom_selector.num_roms = 0;#ifdef USE_SDL2_TTF        // Inicializar fonte        g_rom_selector.font = NULL;        if (TTF_WasInit())        {            g_rom_selector.font = TTF_OpenFont("resources/fonts/default.ttf", g_rom_selector.font_size);            if (!g_rom_selector.font)            {                EMU_LOG_WARNING(EMU_LOG_CAT_GUI, "Não foi possível carregar fonte: %s", TTF_GetError());            }        }#endif    }    // Inicializar informações de consoles    for (int i = 0; i < ROM_SELECTOR_MAX_CONSOLES; i++)    {        console_type_t console_type = i + 1; // Começando de CONSOLE_GENESIS (1)        if (console_type < CONSOLE_MAX)        {            gui_rom_selector_init_console(selector, console_type);        }    }    return GUI_ERROR_SUCCESS;}// Inicializa as informações de um console específicogui_error_t gui_rom_selector_init_console(gui_rom_selector_t *selector, console_type_t console_type){    if (!selector || console_type <= CONSOLE_UNKNOWN || console_type >= CONSOLE_MAX)        return GUI_ERROR_INVALID_PARAMETER;    console_selector_info_t *console_info = NULL;    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        // Configurar informações do console        const char *name = gui_rom_selector_get_console_name(console_type);        if (name)        {            if (console_type - 1 < ROM_SELECTOR_MAX_CONSOLES)            {                console_info = &selector->console_info[console_type - 1];                strncpy(console_info->name, name, sizeof(console_info->name) - 1);                console_info->name[sizeof(console_info->name) - 1] = '\0';                // Configurar extensões por tipo de console                console_info->num_extensions = 0;                switch (console_type)                {                case CONSOLE_GENESIS:                    strncpy(console_info->extensions[0], ".md", 8);                    strncpy(console_info->extensions[1], ".bin", 8);                    strncpy(console_info->extensions[2], ".gen", 8);                    console_info->num_extensions = 3;                    strncpy(console_info->rom_dir, "roms/genesis", 256);                    break;                case CONSOLE_NES:                    strncpy(console_info->extensions[0], ".nes", 8);                    console_info->num_extensions = 1;                    strncpy(console_info->rom_dir, "roms/nes", 256);                    break;                case CONSOLE_SNES:                    strncpy(console_info->extensions[0], ".sfc", 8);                    strncpy(console_info->extensions[1], ".smc", 8);                    console_info->num_extensions = 2;                    strncpy(console_info->rom_dir, "roms/snes", 256);                    break;                case CONSOLE_SMS:                case CONSOLE_MASTERSYSTEM:                    strncpy(console_info->extensions[0], ".sms", 8);                    console_info->num_extensions = 1;                    strncpy(console_info->rom_dir, "roms/sms", 256);                    break;                default:                    break;                }            }        }    }    return GUI_ERROR_SUCCESS;}// Renderiza o seletor de ROMsvoid gui_rom_selector_render(gui_rom_selector_t *selector, SDL_Renderer *renderer){    if (!selector || !renderer || !selector->visible)    {        return;    }    gui_rom_selector_extended_t *ext_selector = NULL;    SDL_Rect viewport = {0};    // Verifica se é nossa instância global    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        ext_selector = &g_rom_selector;        viewport = ext_selector->viewport;    }    else    {        // Cria uma viewport a partir dos limites do seletor        viewport.x = selector->bounds.x;        viewport.y = selector->bounds.y;        viewport.w = selector->bounds.width;        viewport.h = selector->bounds.height;    }    // Desenha o fundo    SDL_SetRenderDrawColor(renderer,                           selector->background_color.r,                           selector->background_color.g,                           selector->background_color.b,                           selector->background_color.a);    SDL_RenderFillRect(renderer, &viewport);    // Verificação de segurança    if (!ext_selector)    {        EMU_LOG_ERROR(EMU_LOG_CAT_GUI, "Seletor de ROMs estendido não encontrado");        return;    }    // Desenhar lista de ROMs    int y = viewport.y - ext_selector->scroll_position;    for (int i = 0; i < ext_selector->num_roms; i++)    {        if (y + ext_selector->font_size > viewport.y + viewport.h)            break;        if (y >= viewport.y)        {            SDL_Color color;            if (i == selector->selected_index)            {                // Usa a cor de seleção                color.r = selector->selection_color.r;                color.g = selector->selection_color.g;                color.b = selector->selection_color.b;                color.a = selector->selection_color.a;                SDL_Rect highlight = {                    viewport.x,                    y,                    viewport.w,                    ext_selector->font_size};                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);                SDL_RenderFillRect(renderer, &highlight);                // Texto em branco para melhor contraste                color.r = color.g = color.b = 255;                color.a = 255;            }            else            {                // Usa a cor do texto normal                color.r = selector->text_color.r;                color.g = selector->text_color.g;                color.b = selector->text_color.b;                color.a = selector->text_color.a;            }            render_text(renderer,#ifdef USE_SDL2_TTF                        ext_selector->font,#endif                        ext_selector->roms[i].name,                        viewport.x + 5,                        y,                        color.r, color.g, color.b, color.a);        }        y += ext_selector->font_size;    }}// Atualiza o seletor de ROMsvoid gui_rom_selector_update(gui_rom_selector_t *selector, SDL_Renderer *renderer){    if (!selector || !selector->visible || !renderer)        return;    // Verificar se o renderer é válido    SDL_Rect viewport;    // Obter viewport do seletor    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        viewport = g_rom_selector.viewport;    }    else    {        viewport.x = selector->bounds.x;        viewport.y = selector->bounds.y;        viewport.w = selector->bounds.width;        viewport.h = selector->bounds.height;    }    // Desenhar fundo    SDL_SetRenderDrawColor(renderer,                           selector->background_color.r,                           selector->background_color.g,                           selector->background_color.b,                           selector->background_color.a);    SDL_RenderFillRect(renderer, &viewport);    // Desenhar tabs dos consoles    int tab_width = viewport.w / CONSOLE_COUNT;    for (int i = 0; i < CONSOLE_COUNT; i++)    {        SDL_Rect tab = {            viewport.x + (i * tab_width),            viewport.y,            tab_width,            30};        if (i == selector->current_console)        {            SDL_SetRenderDrawColor(renderer,                                   COLOR_CONSOLE_TAB_SELECTED_R,                                   COLOR_CONSOLE_TAB_SELECTED_G,                                   COLOR_CONSOLE_TAB_SELECTED_B,                                   COLOR_CONSOLE_TAB_SELECTED_A);        }        else        {            SDL_SetRenderDrawColor(renderer,                                   COLOR_CONSOLE_TAB_R,                                   COLOR_CONSOLE_TAB_G,                                   COLOR_CONSOLE_TAB_B,                                   COLOR_CONSOLE_TAB_A);        }        SDL_RenderFillRect(renderer, &tab);        // Renderizar nome do console        const char *console_name = gui_rom_selector_get_console_name(i);        SDL_Color textColor = {COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, COLOR_TEXT_A};        render_text(renderer,#ifdef USE_SDL2_TTF                    selector->font,#endif                    console_name,                    tab.x + 5, tab.y + 5,                    textColor.r, textColor.g, textColor.b, textColor.a);    }    // Desenhar lista de ROMs    console_selector_info_t *current_console = &selector->console_info[selector->current_console];    int y = viewport.y + 40;    int scroll_position = 0;    // Pegar scroll position do seletor estendido se disponível    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        scroll_position = g_rom_selector.scroll_position;    }    for (int i = 0; i < selector->num_roms; i++)    {        SDL_Rect item = {            viewport.x + 10,            y + (i * 25) - scroll_position,            viewport.w - 20,            20};        // Só renderiza se estiver visível        if (item.y + item.h < viewport.y ||            item.y > viewport.y + viewport.h)            continue;        if (i == selector->selected_index)        {            SDL_SetRenderDrawColor(renderer,                                   selector->selection_color.r,                                   selector->selection_color.g,                                   selector->selection_color.b,                                   selector->selection_color.a);            SDL_RenderFillRect(renderer, &item);        }        // Extrair apenas o nome do arquivo da ROM        const char *rom_name = NULL;        if (i < selector->num_roms && selector->roms && &selector->roms[i])        {            rom_name = selector->roms[i].name;        }        else        {            continue; // Pular se não tiver dados válidos        }        render_text(renderer,#ifdef USE_SDL2_TTF                    selector->font,#endif                    rom_name,                    item.x + 5, item.y + 2,                    COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, COLOR_TEXT_A);    }}// Processa eventos para o seletor de ROMsbool gui_rom_selector_handle_event(gui_rom_selector_t *selector, SDL_Event *event){    if (!selector || !selector->visible || !event)        return false;    // Verificar se é um seletor válido    SDL_Rect viewport;    int scroll_position = 0;    // Obter viewport do seletor    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        viewport = g_rom_selector.viewport;        scroll_position = g_rom_selector.scroll_position;    }    else    {        viewport.x = selector->bounds.x;        viewport.y = selector->bounds.y;        viewport.w = selector->bounds.width;        viewport.h = selector->bounds.height;    }    int x, y;    switch (event->type)    {    case SDL_MOUSEBUTTONDOWN:        if (event->button.button == SDL_BUTTON_LEFT)        {            x = event->button.x;            y = event->button.y;            // Verificar se o clique está na viewport            if (x < viewport.x || x >= viewport.x + viewport.w ||                y < viewport.y || y >= viewport.y + viewport.h)            {                return false;            }            // Verificar clique nas tabs            int tab_width = viewport.w / CONSOLE_COUNT;            if (y >= viewport.y && y < viewport.y + 30)            {                int tab_clicked = (x - viewport.x) / tab_width;                if (tab_clicked >= 0 && tab_clicked < CONSOLE_COUNT)                {                    gui_rom_selector_set_console(selector, tab_clicked);                    return true;                }            }            // Verificar clique na lista de ROMs            if (y >= viewport.y + 40)            {                int item_clicked = (y - (viewport.y + 40) + scroll_position) / 25;                if (item_clicked >= 0 && item_clicked < selector->num_roms)                {                    selector->selected_index = item_clicked;                    // Verificar se a ROM selecionada é válida                    if (selector->roms && item_clicked < selector->num_roms)                    {                        strncpy(selector->selected_rom_path, selector->roms[item_clicked].path,                                ROM_SELECTOR_MAX_PATH - 1);                        selector->selected_rom_path[ROM_SELECTOR_MAX_PATH - 1] = '\0';                        // Se houver callback registrado, chamar                        if (selector->on_rom_selected)                        {                            selector->on_rom_selected(selector->selected_rom_path, selector->current_console);                        }                    }                    return true;                }            }        }        break;    case SDL_MOUSEWHEEL:    {        // Atualizar o scroll        if (selector == (gui_rom_selector_t *)&g_rom_selector)        {            g_rom_selector.scroll_position -= event->wheel.y * 30;            if (g_rom_selector.scroll_position < 0)                g_rom_selector.scroll_position = 0;            // Limite máximo de scroll baseado no número de ROMs            int max_scroll = (selector->num_roms * 25) - (viewport.h - 40);            if (max_scroll < 0)                max_scroll = 0;            if (g_rom_selector.scroll_position > max_scroll)                g_rom_selector.scroll_position = max_scroll;        }        return true;    }    case SDL_KEYDOWN:    {        switch (event->key.keysym.sym)        {        case SDLK_UP:            if (selector->selected_index > 0)            {                selector->selected_index--;                // Ajustar scroll se necessário                if (selector == (gui_rom_selector_t *)&g_rom_selector)                {                    int item_y = viewport.y + 40 + (selector->selected_index * 25) - g_rom_selector.scroll_position;                    if (item_y < viewport.y + 40)                    {                        g_rom_selector.scroll_position = (selector->selected_index * 25);                    }                }            }            return true;        case SDLK_DOWN:            if (selector->selected_index < selector->num_roms - 1)            {                selector->selected_index++;                // Ajustar scroll se necessário                if (selector == (gui_rom_selector_t *)&g_rom_selector)                {                    int item_y = viewport.y + 40 + (selector->selected_index * 25) - g_rom_selector.scroll_position;                    if (item_y > viewport.y + viewport.h - 25)                    {                        g_rom_selector.scroll_position = (selector->selected_index * 25) - (viewport.h - 65);                        if (g_rom_selector.scroll_position < 0)                            g_rom_selector.scroll_position = 0;                    }                }            }            return true;        case SDLK_RETURN:            if (selector->selected_index >= 0 && selector->selected_index < selector->num_roms && selector->roms)            {                strncpy(selector->selected_rom_path, selector->roms[selector->selected_index].path,                        ROM_SELECTOR_MAX_PATH - 1);                selector->selected_rom_path[ROM_SELECTOR_MAX_PATH - 1] = '\0';                // Se houver callback registrado, chamar                if (selector->on_rom_selected)                {                    selector->on_rom_selected(selector->selected_rom_path, selector->current_console);                }                selector->visible = false;                return true;            }            return true;        case SDLK_ESCAPE:            selector->visible = false;            return true;        }    }    break;    }    return false;}// Define o console atualgui_error_t gui_rom_selector_set_console(gui_rom_selector_t *selector, int console_index){    if (!selector || console_index < 0 || console_index >= CONSOLE_COUNT)        return GUI_ERROR_INVALID_PARAMETER;    // Mudar para o console solicitado    selector->current_console = console_index;    selector->selected_index = 0;    // Resetar scroll na instância global    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        g_rom_selector.scroll_position = 0;    }    // Carregar ROMs do console selecionado    gui_rom_selector_scan_roms(selector);    return GUI_ERROR_SUCCESS;}// Obtém o nome de um console pelo seu tipoconst char *gui_rom_selector_get_console_name(console_type_t console_type){    switch (console_type)    {    case CONSOLE_GENESIS:        return "Genesis";    case CONSOLE_NES:        return "NES";    case CONSOLE_SNES:        return "SNES";    case CONSOLE_SMS:    case CONSOLE_MASTERSYSTEM:        return "Master System";    default:        return "Desconhecido";    }}// Mostra o seletor de ROMsvoid gui_rom_selector_show(gui_rom_selector_t *selector){    if (!selector)        return;    selector->visible = true;    gui_rom_selector_scan_roms(selector);}// Esconde o seletor de ROMsvoid gui_rom_selector_hide(gui_rom_selector_t *selector){    if (!selector)        return;    selector->visible = false;}// Escaneia por ROMs para o console atualgui_error_t gui_rom_selector_scan_roms(gui_rom_selector_t *selector){    if (!selector)        return GUI_ERROR_INVALID_PARAMETER;    // Verificar se é instância global    gui_rom_selector_extended_t *ext_selector = NULL;    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        ext_selector = &g_rom_selector;        ext_selector->num_roms = 0; // Reset contador    }    // Verificar se o console é válido    if (selector->current_console < 0 || selector->current_console >= CONSOLE_COUNT)    {        EMU_LOG_ERROR(EMU_LOG_CAT_GUI, "Console inválido: %d", selector->current_console);        return GUI_ERROR_INVALID_PARAMETER;    }    // Obter informações do console atual    console_selector_info_t *console_info = &selector->console_info[selector->current_console];    // Simular carregamento de ROMs (substituir por código real)    char rom_path[ROM_SELECTOR_MAX_PATH];    selector->num_roms = 0;    // Pasta raiz de ROMs    const char *root_dir = console_info->rom_dir;    // Lista arquivos no diretório de ROMs    // TODO: Implementar listagem real de ROMs    for (int i = 0; i < 10; i++)    {        // Criar alguns dados de exemplo        snprintf(rom_path, ROM_SELECTOR_MAX_PATH, "%s/rom%d.%s",                 root_dir, i, console_info->extensions[0] + 1); // +1 para pular o '.'        // Adicionar ROM à lista        if (selector->num_roms < ROM_SELECTOR_MAX_ROMS)        {            rom_info_t *rom = &selector->roms[selector->num_roms];            strncpy(rom->path, rom_path, ROM_SELECTOR_MAX_PATH - 1);            rom->path[ROM_SELECTOR_MAX_PATH - 1] = '\0';            // Extrair nome da ROM do caminho            const char *filename = strrchr(rom_path, '/');            if (!filename)                filename = strrchr(rom_path, '\\');            if (filename)            {                filename++; // Pular o separador            }            else            {                filename = rom_path; // Usar caminho completo se não encontrar separador            }            strncpy(rom->name, filename, ROM_SELECTOR_MAX_NAME - 1);            rom->name[ROM_SELECTOR_MAX_NAME - 1] = '\0';            // Outros dados da ROM            rom->size = 1024 * (100 + i);                                 // Tamanho fictício            rom->favorite = (i % 3 == 0);                                 // Alguns favoritos de exemplo            rom->last_played = (i % 5 == 0) ? time(NULL) - i * 86400 : 0; // Datas fictícias            rom->recently_played = (i % 5 == 0);            // Incrementar contador            selector->num_roms++;            // Atualizar contador no seletor estendido se for instância global            if (ext_selector)            {                ext_selector->num_roms = selector->num_roms;                ext_selector->roms[i] = *rom; // Copiar dados da ROM            }        }    }    // Resetar índice selecionado    selector->selected_index = 0;    return GUI_ERROR_SUCCESS;}// Retorna a ROM selecionada atualmenteconst rom_info_t *gui_rom_selector_get_selected_rom(const gui_rom_selector_t *selector){    if (!selector || selector->selected_index < 0 || selector->selected_index >= selector->num_roms)        return NULL;    return &selector->roms[selector->selected_index];}// Define a viewport para o seletorvoid gui_rom_selector_set_viewport(gui_rom_selector_t *selector, gui_rect_t viewport){    if (!selector)        return;    // Atualizar viewport    selector->bounds = viewport;    // Se for instância global, atualizar campos adicionais    if (selector == (gui_rom_selector_t *)&g_rom_selector)    {        g_rom_selector.viewport.x = viewport.x;        g_rom_selector.viewport.y = viewport.y;        g_rom_selector.viewport.w = viewport.width;        g_rom_selector.viewport.h = viewport.height;    }}// Define o callback para seleção de ROMvoid gui_rom_selector_set_callback(gui_rom_selector_t *selector, void (*callback)(const char *rom_path)){    if (!selector || !callback)        return;    // Adaptador para o callback (ignorando o console_type)    selector->on_rom_selected = (void (*)(const char *, console_type_t))callback;}// Verifica se o seletor está visívelbool gui_rom_selector_is_visible(const gui_rom_selector_t *selector){    return selector ? selector->visible : false;}