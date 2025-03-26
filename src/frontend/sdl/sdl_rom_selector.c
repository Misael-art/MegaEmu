/**
 * @file sdl_rom_selector.c
 * @brief Implementação do seletor de ROMs para o frontend SDL
 */
#include "sdl_rom_selector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "utils/enhanced_log.h"
#include "utils/error_handling.h"
#include "utils/file_utils.h"
#include "utils/time_utils.h"

// Diretórios padrão para armazenar metadados e thumbnails
#define METADATA_DIR "data/metadata"
#define THUMBNAILS_DIR "data/thumbnails"

// Macros de log para o seletor de ROMs
#define SELECTOR_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define SELECTOR_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define SELECTOR_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define SELECTOR_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)

// Utilitários privados do seletor de ROMs
static bool load_system_icons(sdl_rom_selector_t *selector);
static bool extract_rom_name(const char *path, char *name, size_t max_len);
static bool is_valid_extension(const char *filename, const char extensions[][16], uint8_t count);
static void update_filtered_items(sdl_rom_selector_t *selector);
static bool load_thumbnail(sdl_rom_selector_t *selector, sdl_rom_item_t *item);
static void sort_items(sdl_rom_selector_t *selector);

/**
 * @brief Inicializa o seletor de ROMs
 *
 * @param selector Ponteiro para o estado do seletor
 * @param renderer Renderizador SDL para uso
 * @return true Se inicializado com sucesso
 * @return false Se falhou
 */
bool sdl_rom_selector_init(sdl_rom_selector_t *selector, sdl_game_renderer_t *renderer) {
    if (!selector || !renderer) {
        SELECTOR_LOG_ERROR("Parâmetros inválidos para inicialização do seletor de ROMs");
        return false;
    }

    // Inicializa estrutura com zeros
    memset(selector, 0, sizeof(sdl_rom_selector_t));

    // Configura estado inicial
    selector->renderer = renderer;
    selector->visible = false;
    selector->selected_index = -1;
    selector->scroll_position = 0;

    // Configura filtro padrão
    selector->filter.view = SDL_ROM_VIEW_ALL;
    selector->filter.system = CONSOLE_UNKNOWN;
    selector->filter.show_all_systems = true;

    // Carrega fontes
    selector->title_font = TTF_OpenFont("assets/fonts/roboto_bold.ttf", 20);
    selector->item_font = TTF_OpenFont("assets/fonts/roboto_regular.ttf", 16);

    if (!selector->title_font || !selector->item_font) {
        SELECTOR_LOG_ERROR("Falha ao carregar fontes: %s", TTF_GetError());
        sdl_rom_selector_shutdown(selector);
        return false;
    }

    // Cria textura placeholder para thumbnails
    SDL_Surface *placeholder = SDL_CreateRGBSurface(0, SDL_ROM_THUMBNAIL_SIZE, SDL_ROM_THUMBNAIL_SIZE,
                                                    32, 0, 0, 0, 0);
    if (placeholder) {
        SDL_FillRect(placeholder, NULL, SDL_MapRGB(placeholder->format, 100, 100, 100));
        selector->thumbnail_placeholder = SDL_CreateTextureFromSurface(
            renderer->renderer, placeholder);
        SDL_FreeSurface(placeholder);
    }

    // Carrega ícones de sistema
    if (!load_system_icons(selector)) {
        SELECTOR_LOG_WARN("Falha ao carregar alguns ícones de sistema");
    }

    // Define diretórios de ROM padrão e extensões
    // NES
    strcpy(selector->rom_dirs[CONSOLE_NES], "roms/nes");
    strcpy(selector->extensions[CONSOLE_NES][0], "nes");
    strcpy(selector->extensions[CONSOLE_NES][1], "zip");
    selector->extension_count[CONSOLE_NES] = 2;

    // Mega Drive
    strcpy(selector->rom_dirs[CONSOLE_MEGA_DRIVE], "roms/megadrive");
    strcpy(selector->extensions[CONSOLE_MEGA_DRIVE][0], "md");
    strcpy(selector->extensions[CONSOLE_MEGA_DRIVE][1], "bin");
    strcpy(selector->extensions[CONSOLE_MEGA_DRIVE][2], "gen");
    strcpy(selector->extensions[CONSOLE_MEGA_DRIVE][3], "zip");
    selector->extension_count[CONSOLE_MEGA_DRIVE] = 4;

    // Master System
    strcpy(selector->rom_dirs[CONSOLE_MASTER_SYSTEM], "roms/sms");
    strcpy(selector->extensions[CONSOLE_MASTER_SYSTEM][0], "sms");
    strcpy(selector->extensions[CONSOLE_MASTER_SYSTEM][1], "zip");
    selector->extension_count[CONSOLE_MASTER_SYSTEM] = 2;

    // Game Gear
    strcpy(selector->rom_dirs[CONSOLE_GAME_GEAR], "roms/gamegear");
    strcpy(selector->extensions[CONSOLE_GAME_GEAR][0], "gg");
    strcpy(selector->extensions[CONSOLE_GAME_GEAR][1], "zip");
    selector->extension_count[CONSOLE_GAME_GEAR] = 2;

    // Carrega metadados existentes
    if (!sdl_rom_selector_load_metadata(selector)) {
        SELECTOR_LOG_INFO("Nenhum metadado anterior encontrado, iniciando vazio");
    }

    // Escaneia diretórios de ROMs
    if (!sdl_rom_selector_scan_directories(selector)) {
        SELECTOR_LOG_WARN("Falha ao escanear diretórios de ROMs");
    }

    SELECTOR_LOG_INFO("Seletor de ROMs inicializado com %d itens", selector->item_count);
    return true;
}

/**
 * @brief Finaliza o seletor de ROMs e libera recursos
 *
 * @param selector Ponteiro para o estado do seletor
 */
void sdl_rom_selector_shutdown(sdl_rom_selector_t *selector) {
    if (!selector) {
        return;
    }

    // Salva metadados
    sdl_rom_selector_save_metadata(selector);

    // Libera texturas de thumbnails
    for (uint32_t i = 0; i < selector->item_count; i++) {
        if (selector->items[i].thumbnail &&
            selector->items[i].thumbnail != selector->thumbnail_placeholder) {
            SDL_DestroyTexture(selector->items[i].thumbnail);
            selector->items[i].thumbnail = NULL;
        }
    }

    // Libera ícones de sistema
    for (int i = 0; i < CONSOLE_COUNT; i++) {
        if (selector->system_icons[i]) {
            SDL_DestroyTexture(selector->system_icons[i]);
            selector->system_icons[i] = NULL;
        }
    }

    // Libera texturas de UI
    if (selector->background_texture) {
        SDL_DestroyTexture(selector->background_texture);
        selector->background_texture = NULL;
    }

    if (selector->highlight_texture) {
        SDL_DestroyTexture(selector->highlight_texture);
        selector->highlight_texture = NULL;
    }

    if (selector->thumbnail_placeholder) {
        SDL_DestroyTexture(selector->thumbnail_placeholder);
        selector->thumbnail_placeholder = NULL;
    }

    // Libera fontes
    if (selector->title_font) {
        TTF_CloseFont(selector->title_font);
        selector->title_font = NULL;
    }

    if (selector->item_font) {
        TTF_CloseFont(selector->item_font);
        selector->item_font = NULL;
    }

    // Limpa estado
    memset(selector, 0, sizeof(sdl_rom_selector_t));
}

/**
 * @brief Escaneia diretórios de ROMs em busca de novos arquivos
 *
 * @param selector Ponteiro para o estado do seletor
 * @return true Se pelo menos um diretório foi escaneado com sucesso
 * @return false Se falhou completamente
 */
bool sdl_rom_selector_scan_directories(sdl_rom_selector_t *selector) {
    if (!selector) {
        return false;
    }

    bool any_success = false;

    // Para cada sistema suportado
    for (int system = 0; system < CONSOLE_COUNT; system++) {
        // Ignora sistemas indefinidos ou sem diretórios
        if (system == CONSOLE_UNKNOWN || strlen(selector->rom_dirs[system]) == 0) {
            continue;
        }

        SELECTOR_LOG_INFO("Escaneando ROMs para sistema %d em %s",
                          system, selector->rom_dirs[system]);

        DIR *dir = opendir(selector->rom_dirs[system]);
        if (!dir) {
            SELECTOR_LOG_WARN("Não foi possível abrir diretório para sistema %d", system);
            continue;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            // Ignora entradas . e ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Verifica se é um arquivo e tem extensão válida
            if (entry->d_type == DT_REG &&
                is_valid_extension(entry->d_name,
                                  selector->extensions[system],
                                  selector->extension_count[system])) {

                // Constrói caminho completo
                char full_path[SDL_ROM_MAX_PATH];
                snprintf(full_path, SDL_ROM_MAX_PATH, "%s/%s",
                         selector->rom_dirs[system], entry->d_name);

                // Adiciona ROM se não existir
                sdl_rom_selector_add_rom(selector, full_path, (console_type_t)system);
            }
        }

        closedir(dir);
        any_success = true;
    }

    // Ordena e filtra após escaneamento
    if (any_success) {
        sort_items(selector);
        update_filtered_items(selector);
    }

    return any_success;
}

/**
 * @brief Adiciona uma nova ROM ao seletor
 *
 * @param selector Ponteiro para o estado do seletor
 * @param path Caminho para o arquivo da ROM
 * @param system Sistema a que pertence a ROM
 * @return true Se adicionado com sucesso
 * @return false Se falhou ou já existe
 */
bool sdl_rom_selector_add_rom(sdl_rom_selector_t *selector, const char *path, console_type_t system) {
    if (!selector || !path || selector->item_count >= SDL_ROM_MAX_ITEMS) {
        return false;
    }

    // Verifica se já existe pelo caminho
    for (uint32_t i = 0; i < selector->item_count; i++) {
        if (strcmp(selector->items[i].path, path) == 0) {
            // Atualiza apenas o sistema se necessário
            if (selector->items[i].system == CONSOLE_UNKNOWN && system != CONSOLE_UNKNOWN) {
                selector->items[i].system = system;
                return true;
            }
            return false; // Já existe
        }
    }

    // Obtém estatísticas do arquivo
    struct stat st;
    if (stat(path, &st) != 0) {
        SELECTOR_LOG_WARN("Falha ao obter estatísticas do arquivo: %s", path);
        return false;
    }

    // Inicializa novo item
    sdl_rom_item_t *item = &selector->items[selector->item_count];
    strncpy(item->path, path, SDL_ROM_MAX_PATH - 1);
    item->path[SDL_ROM_MAX_PATH - 1] = '\0';

    // Extrai nome do arquivo
    if (!extract_rom_name(path, item->name, SDL_ROM_MAX_NAME)) {
        SELECTOR_LOG_WARN("Falha ao extrair nome da ROM: %s", path);
        return false;
    }

    // Configura valores padrão
    item->system = system;
    item->size = st.st_size;
    item->last_played = 0;
    item->added_date = time(NULL);
    item->favorite = false;
    item->play_count = 0;
    item->thumbnail = NULL;

    // Usa nome como display_name inicialmente
    strncpy(item->display_name, item->name, SDL_ROM_MAX_NAME - 1);
    item->display_name[SDL_ROM_MAX_NAME - 1] = '\0';

    // Tenta carregar thumbnail se disponível
    snprintf(item->thumbnail_path, SDL_ROM_MAX_PATH,
             "%s/%s.png", THUMBNAILS_DIR, item->name);

    load_thumbnail(selector, item);

    // Incrementa contador
    selector->item_count++;

    return true;
}

/**
 * @brief Remove uma ROM do seletor
 *
 * @param selector Ponteiro para o estado do seletor
 * @param path Caminho da ROM a remover
 * @return true Se removido com sucesso
 * @return false Se não encontrado ou falhou
 */
bool sdl_rom_selector_remove_rom(sdl_rom_selector_t *selector, const char *path) {
    if (!selector || !path) {
        return false;
    }

    // Procura a ROM pelo caminho
    int index = -1;
    for (uint32_t i = 0; i < selector->item_count; i++) {
        if (strcmp(selector->items[i].path, path) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return false; // Não encontrado
    }

    // Libera thumbnail se existir
    if (selector->items[index].thumbnail &&
        selector->items[index].thumbnail != selector->thumbnail_placeholder) {
        SDL_DestroyTexture(selector->items[index].thumbnail);
        selector->items[index].thumbnail = NULL;
    }

    // Remove movendo as ROMs restantes
    if (index < selector->item_count - 1) {
        memmove(&selector->items[index], &selector->items[index + 1],
                (selector->item_count - index - 1) * sizeof(sdl_rom_item_t));
    }

    selector->item_count--;

    // Atualiza índice selecionado e filtro
    if (selector->selected_index >= selector->item_count) {
        selector->selected_index = selector->item_count - 1;
    }

    update_filtered_items(selector);
    return true;
}

/**
 * @brief Limpa todas as ROMs do seletor
 *
 * @param selector Ponteiro para o estado do seletor
 */
void sdl_rom_selector_clear_roms(sdl_rom_selector_t *selector) {
    if (!selector) {
        return;
    }

    // Libera thumbnails
    for (uint32_t i = 0; i < selector->item_count; i++) {
        if (selector->items[i].thumbnail &&
            selector->items[i].thumbnail != selector->thumbnail_placeholder) {
            SDL_DestroyTexture(selector->items[i].thumbnail);
            selector->items[i].thumbnail = NULL;
        }
    }

    // Reseta contadores e estado
    selector->item_count = 0;
    selector->filtered_count = 0;
    selector->selected_index = -1;
    selector->scroll_position = 0;
}

/**
 * @brief Define a visibilidade do seletor de ROMs
 *
 * @param selector Ponteiro para o estado do seletor
 * @param visible true para mostrar, false para esconder
 */
void sdl_rom_selector_show(sdl_rom_selector_t *selector) {
    if (selector) {
        selector->visible = true;

        // Carrega thumbnails que podem não estar carregados
        sdl_rom_selector_load_thumbnails(selector);
    }
}

/**
 * @brief Esconde o seletor de ROMs
 *
 * @param selector Ponteiro para o estado do seletor
 */
void sdl_rom_selector_hide(sdl_rom_selector_t *selector) {
    if (selector) {
        selector->visible = false;
    }
}

/**
 * @brief Verifica se o seletor está visível
 *
 * @param selector Ponteiro para o estado do seletor
 * @return true Se visível
 * @return false Se não visível ou inválido
 */
bool sdl_rom_selector_is_visible(const sdl_rom_selector_t *selector) {
    return selector ? selector->visible : false;
}

/**
 * @brief Define a área de visualização do seletor
 *
 * @param selector Ponteiro para o estado do seletor
 * @param viewport Retângulo da área de visualização
 */
void sdl_rom_selector_set_viewport(sdl_rom_selector_t *selector, SDL_Rect viewport) {
    if (selector) {
        selector->viewport = viewport;
    }
}

/**
 * @brief Carrega os ícones dos sistemas suportados
 *
 * @param selector Ponteiro para o estado do seletor
 * @return true Se pelo menos alguns ícones foram carregados
 * @return false Se falhou completamente
 */
static bool load_system_icons(sdl_rom_selector_t *selector) {
    if (!selector || !selector->renderer) {
        return false;
    }

    // Estrutura de mapeamento sistema -> arquivo de ícone
    struct {
        console_type_t system;
        const char *filename;
    } icon_map[] = {
        {CONSOLE_NES, "assets/icons/nes.png"},
        {CONSOLE_MEGA_DRIVE, "assets/icons/megadrive.png"},
        {CONSOLE_MASTER_SYSTEM, "assets/icons/mastersystem.png"},
        {CONSOLE_GAME_GEAR, "assets/icons/gamegear.png"},
        {CONSOLE_UNKNOWN, "assets/icons/generic.png"}
    };

    // Inicializa todos os ponteiros de ícones como NULL
    memset(selector->system_icons, 0, sizeof(selector->system_icons));

    bool any_loaded = false;

    // Carrega cada ícone
    for (size_t i = 0; i < sizeof(icon_map) / sizeof(icon_map[0]); i++) {
        SDL_Surface *surface = SDL_LoadBMP(icon_map[i].filename);
        if (!surface) {
            // Tenta PNG se BMP falhar
            // Presumindo uso de SDL_image
            surface = IMG_Load(icon_map[i].filename);
        }

        if (surface) {
            selector->system_icons[icon_map[i].system] =
                SDL_CreateTextureFromSurface(selector->renderer->renderer, surface);
            SDL_FreeSurface(surface);

            if (selector->system_icons[icon_map[i].system]) {
                any_loaded = true;
            }
        } else {
            SELECTOR_LOG_WARN("Falha ao carregar ícone para sistema %d: %s",
                             icon_map[i].system, icon_map[i].filename);
        }
    }

    return any_loaded;
}
