/**
 * @file sdl_hotkeys.c
 * @brief Implementação do sistema de teclas de atalho para o frontend SDL
 */
#include "sdl_hotkeys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tabela de correspondência entre ações e nomes
static const struct {
    sdl_hotkey_action_t action;
    const char *name;
    const char *description;
} hotkey_action_names[] = {
    { SDL_HOTKEY_ACTION_NONE, "none", "Nenhuma ação" },
    { SDL_HOTKEY_ACTION_TOGGLE_FULLSCREEN, "toggle_fullscreen", "Alternar tela cheia" },
    { SDL_HOTKEY_ACTION_SAVE_STATE, "save_state", "Salvar estado" },
    { SDL_HOTKEY_ACTION_LOAD_STATE, "load_state", "Carregar estado" },
    { SDL_HOTKEY_ACTION_RESET, "reset", "Reiniciar jogo" },
    { SDL_HOTKEY_ACTION_QUIT, "quit", "Sair do emulador" },
    { SDL_HOTKEY_ACTION_PAUSE, "pause", "Pausar jogo" },
    { SDL_HOTKEY_ACTION_FAST_FORWARD, "fast_forward", "Avançar rápido" },
    { SDL_HOTKEY_ACTION_SLOW_MOTION, "slow_motion", "Câmera lenta" },
    { SDL_HOTKEY_ACTION_SCREENSHOT, "screenshot", "Capturar tela" },
    { SDL_HOTKEY_ACTION_RECORD_VIDEO, "record_video", "Gravar vídeo" },
    { SDL_HOTKEY_ACTION_REWIND, "rewind", "Retroceder" },
    { SDL_HOTKEY_ACTION_NEXT_SLOT, "next_slot", "Próximo slot de save" },
    { SDL_HOTKEY_ACTION_PREV_SLOT, "prev_slot", "Slot de save anterior" },
    { SDL_HOTKEY_ACTION_TOGGLE_SCANLINES, "toggle_scanlines", "Alternar scanlines" },
    { SDL_HOTKEY_ACTION_TOGGLE_CRT, "toggle_crt", "Alternar efeito CRT" },
    { SDL_HOTKEY_ACTION_MUTE, "mute", "Silenciar áudio" },
    { SDL_HOTKEY_ACTION_VOLUME_UP, "volume_up", "Aumentar volume" },
    { SDL_HOTKEY_ACTION_VOLUME_DOWN, "volume_down", "Diminuir volume" },
    { SDL_HOTKEY_ACTION_TOGGLE_MENU, "toggle_menu", "Abrir/fechar menu" },
    { SDL_HOTKEY_ACTION_TOGGLE_DEBUG_INFO, "toggle_debug_info", "Alternar informações de debug" }
};

// Configuração padrão de hotkeys
static const sdl_hotkey_t default_hotkeys[] = {
    { SDLK_F11, KMOD_NONE, SDL_HOTKEY_ACTION_TOGGLE_FULLSCREEN, 0, true, "Alternar tela cheia" },
    { SDLK_F1, KMOD_NONE, SDL_HOTKEY_ACTION_SAVE_STATE, 0, true, "Salvar estado (slot atual)" },
    { SDLK_F3, KMOD_NONE, SDL_HOTKEY_ACTION_LOAD_STATE, 0, true, "Carregar estado (slot atual)" },
    { SDLK_F2, KMOD_NONE, SDL_HOTKEY_ACTION_NEXT_SLOT, 0, true, "Próximo slot de save" },
    { SDLK_F4, KMOD_SHIFT, SDL_HOTKEY_ACTION_PREV_SLOT, 0, true, "Slot de save anterior" },
    { SDLK_r, KMOD_CTRL, SDL_HOTKEY_ACTION_RESET, 0, true, "Reiniciar jogo" },
    { SDLK_ESCAPE, KMOD_NONE, SDL_HOTKEY_ACTION_TOGGLE_MENU, 0, true, "Abrir/fechar menu" },
    { SDLK_F5, KMOD_NONE, SDL_HOTKEY_ACTION_SCREENSHOT, 0, true, "Capturar tela" },
    { SDLK_SPACE, KMOD_NONE, SDL_HOTKEY_ACTION_PAUSE, 0, true, "Pausar/continuar jogo" },
    { SDLK_TAB, KMOD_NONE, SDL_HOTKEY_ACTION_FAST_FORWARD, 0, true, "Avançar rápido" },
    { SDLK_BACKQUOTE, KMOD_NONE, SDL_HOTKEY_ACTION_TOGGLE_DEBUG_INFO, 0, true, "Alternar informações de debug" },
    { SDLK_F10, KMOD_NONE, SDL_HOTKEY_ACTION_TOGGLE_SCANLINES, 0, true, "Alternar scanlines" },
    { SDLK_F9, KMOD_NONE, SDL_HOTKEY_ACTION_TOGGLE_CRT, 0, true, "Alternar efeito CRT" },
    { SDLK_m, KMOD_NONE, SDL_HOTKEY_ACTION_MUTE, 0, true, "Silenciar áudio" },
    { SDLK_PLUS, KMOD_NONE, SDL_HOTKEY_ACTION_VOLUME_UP, 0, true, "Aumentar volume" },
    { SDLK_MINUS, KMOD_NONE, SDL_HOTKEY_ACTION_VOLUME_DOWN, 0, true, "Diminuir volume" },
    { SDLK_F6, KMOD_NONE, SDL_HOTKEY_ACTION_RECORD_VIDEO, 0, true, "Iniciar/parar gravação de vídeo" },
    { SDLK_BACKSPACE, KMOD_NONE, SDL_HOTKEY_ACTION_REWIND, 0, true, "Retroceder" },
    { SDLK_q, KMOD_CTRL, SDL_HOTKEY_ACTION_QUIT, 0, true, "Sair do emulador" }
};

/**
 * @brief Inicializa o sistema de hotkeys
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @return true Se inicializado com sucesso
 * @return false Se falhou
 */
bool sdl_hotkeys_init(sdl_hotkeys_t *hotkeys) {
    if (!hotkeys) {
        return false;
    }

    // Limpar toda a estrutura
    memset(hotkeys, 0, sizeof(sdl_hotkeys_t));

    // Configurar hotkeys padrão
    sdl_hotkeys_reset_to_defaults(hotkeys);

    hotkeys->initialized = true;
    return true;
}

/**
 * @brief Finaliza o sistema de hotkeys
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 */
void sdl_hotkeys_shutdown(sdl_hotkeys_t *hotkeys) {
    if (!hotkeys) {
        return;
    }

    // Limpar estado
    memset(hotkeys, 0, sizeof(sdl_hotkeys_t));
}

/**
 * @brief Define uma hotkey para uma ação específica
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param action Ação a ser associada
 * @param key Tecla principal
 * @param modifiers Modificadores (Ctrl, Alt, Shift)
 * @param param Parâmetro opcional
 * @return true Se definido com sucesso
 * @return false Se falhou
 */
bool sdl_hotkeys_set(sdl_hotkeys_t *hotkeys, sdl_hotkey_action_t action,
                    SDL_Keycode key, SDL_Keymod modifiers, int param) {
    if (!hotkeys || !hotkeys->initialized || action <= SDL_HOTKEY_ACTION_NONE || action >= SDL_HOTKEY_ACTION_COUNT) {
        return false;
    }

    // Definir a hotkey
    hotkeys->hotkeys[action].key = key;
    hotkeys->hotkeys[action].modifiers = modifiers;
    hotkeys->hotkeys[action].action = action;
    hotkeys->hotkeys[action].param = param;
    hotkeys->hotkeys[action].enabled = true;

    // Copiar descrição se existir na tabela
    for (size_t i = 0; i < sizeof(hotkey_action_names) / sizeof(hotkey_action_names[0]); i++) {
        if (hotkey_action_names[i].action == action) {
            strncpy(hotkeys->hotkeys[action].description,
                    hotkey_action_names[i].description,
                    sizeof(hotkeys->hotkeys[action].description) - 1);
            break;
        }
    }

    return true;
}

/**
 * @brief Remove uma hotkey para uma ação específica
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param action Ação a ter a hotkey removida
 * @return true Se removido com sucesso
 * @return false Se falhou
 */
bool sdl_hotkeys_remove(sdl_hotkeys_t *hotkeys, sdl_hotkey_action_t action) {
    if (!hotkeys || !hotkeys->initialized || action <= SDL_HOTKEY_ACTION_NONE || action >= SDL_HOTKEY_ACTION_COUNT) {
        return false;
    }

    // Desabilitar a hotkey
    hotkeys->hotkeys[action].enabled = false;
    return true;
}

/**
 * @brief Limpa todas as hotkeys
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @return true Se limpo com sucesso
 * @return false Se falhou
 */
bool sdl_hotkeys_clear_all(sdl_hotkeys_t *hotkeys) {
    if (!hotkeys || !hotkeys->initialized) {
        return false;
    }

    // Limpar todas as hotkeys
    for (int i = 0; i < SDL_HOTKEY_ACTION_COUNT; i++) {
        hotkeys->hotkeys[i].enabled = false;
    }

    return true;
}

/**
 * @brief Obtém a hotkey para uma ação específica
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param action Ação a ter a hotkey obtida
 * @return const sdl_hotkey_t* Ponteiro para a hotkey, NULL se não encontrada
 */
const sdl_hotkey_t* sdl_hotkeys_get(const sdl_hotkeys_t *hotkeys, sdl_hotkey_action_t action) {
    if (!hotkeys || !hotkeys->initialized || action <= SDL_HOTKEY_ACTION_NONE || action >= SDL_HOTKEY_ACTION_COUNT) {
        return NULL;
    }

    // Retornar a hotkey se estiver habilitada
    if (hotkeys->hotkeys[action].enabled) {
        return &hotkeys->hotkeys[action];
    }

    return NULL;
}

/**
 * @brief Processa um evento SDL e executa a ação correspondente
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param event Ponteiro para o evento SDL
 * @return true Se um evento de hotkey foi processado
 * @return false Se não havia hotkey para este evento
 */
bool sdl_hotkeys_process_event(sdl_hotkeys_t *hotkeys, SDL_Event *event) {
    if (!hotkeys || !hotkeys->initialized || !event) {
        return false;
    }

    // Processar apenas eventos de tecla pressionada
    if (event->type != SDL_KEYDOWN) {
        return false;
    }

    // Obter a tecla e modificadores
    SDL_Keycode key = event->key.keysym.sym;
    SDL_Keymod modifiers = (SDL_Keymod)(event->key.keysym.mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT));

    // Procurar por uma hotkey correspondente
    for (int i = 1; i < SDL_HOTKEY_ACTION_COUNT; i++) {
        if (hotkeys->hotkeys[i].enabled &&
            hotkeys->hotkeys[i].key == key &&
            hotkeys->hotkeys[i].modifiers == modifiers) {

            // Encontrou hotkey - executar a ação correspondente
            sdl_hotkey_action_t action = (sdl_hotkey_action_t)i;
            int param = hotkeys->hotkeys[i].param;

            // Executar callback apropriado baseado na ação
            switch (action) {
                case SDL_HOTKEY_ACTION_TOGGLE_FULLSCREEN:
                    if (hotkeys->callbacks.toggle_fullscreen) {
                        hotkeys->callbacks.toggle_fullscreen(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_SAVE_STATE:
                    if (hotkeys->callbacks.save_state) {
                        hotkeys->callbacks.save_state(param, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_LOAD_STATE:
                    if (hotkeys->callbacks.load_state) {
                        hotkeys->callbacks.load_state(param, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_RESET:
                    if (hotkeys->callbacks.reset) {
                        hotkeys->callbacks.reset(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_QUIT:
                    if (hotkeys->callbacks.quit) {
                        hotkeys->callbacks.quit(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_PAUSE:
                    if (hotkeys->callbacks.pause) {
                        // Toggle state
                        static bool pause_state = false;
                        pause_state = !pause_state;
                        hotkeys->callbacks.pause(pause_state, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_FAST_FORWARD:
                    if (hotkeys->callbacks.fast_forward) {
                        // Tratar como toggle quando pressionado
                        static bool ff_state = false;
                        ff_state = !ff_state;
                        hotkeys->callbacks.fast_forward(ff_state, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_SLOW_MOTION:
                    if (hotkeys->callbacks.slow_motion) {
                        // Toggle state
                        static bool slow_state = false;
                        slow_state = !slow_state;
                        hotkeys->callbacks.slow_motion(slow_state, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_SCREENSHOT:
                    if (hotkeys->callbacks.screenshot) {
                        hotkeys->callbacks.screenshot(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_RECORD_VIDEO:
                    if (hotkeys->callbacks.record_video) {
                        // Toggle state
                        static bool recording = false;
                        recording = !recording;
                        hotkeys->callbacks.record_video(recording, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_REWIND:
                    if (hotkeys->callbacks.rewind) {
                        // Tratar como toggle quando pressionado
                        static bool rewind_state = false;
                        rewind_state = !rewind_state;
                        hotkeys->callbacks.rewind(rewind_state, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_NEXT_SLOT:
                    if (hotkeys->callbacks.next_slot) {
                        hotkeys->callbacks.next_slot(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_PREV_SLOT:
                    if (hotkeys->callbacks.prev_slot) {
                        hotkeys->callbacks.prev_slot(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_TOGGLE_SCANLINES:
                    if (hotkeys->callbacks.toggle_scanlines) {
                        hotkeys->callbacks.toggle_scanlines(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_TOGGLE_CRT:
                    if (hotkeys->callbacks.toggle_crt) {
                        hotkeys->callbacks.toggle_crt(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_MUTE:
                    if (hotkeys->callbacks.mute) {
                        // Toggle state
                        static bool mute_state = false;
                        mute_state = !mute_state;
                        hotkeys->callbacks.mute(mute_state, hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_VOLUME_UP:
                    if (hotkeys->callbacks.volume_up) {
                        hotkeys->callbacks.volume_up(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_VOLUME_DOWN:
                    if (hotkeys->callbacks.volume_down) {
                        hotkeys->callbacks.volume_down(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_TOGGLE_MENU:
                    if (hotkeys->callbacks.toggle_menu) {
                        hotkeys->callbacks.toggle_menu(hotkeys->userdata);
                    }
                    break;
                case SDL_HOTKEY_ACTION_TOGGLE_DEBUG_INFO:
                    if (hotkeys->callbacks.toggle_debug_info) {
                        hotkeys->callbacks.toggle_debug_info(hotkeys->userdata);
                    }
                    break;
                default:
                    break;
            }

            return true; // Hotkey processada
        }
    }

    return false; // Nenhuma hotkey encontrada
}

/**
 * @brief Verifica se um evento corresponde a uma ação de hotkey específica
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param event Ponteiro para o evento SDL
 * @param action Ação a verificar
 * @return true Se o evento corresponde à ação
 * @return false Se não corresponde
 */
bool sdl_hotkeys_is_action_key(const sdl_hotkeys_t *hotkeys, SDL_Event *event, sdl_hotkey_action_t action) {
    if (!hotkeys || !hotkeys->initialized || !event || action <= SDL_HOTKEY_ACTION_NONE || action >= SDL_HOTKEY_ACTION_COUNT) {
        return false;
    }

    // Verificar se é um evento de tecla
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) {
        return false;
    }

    // Obter a tecla e modificadores
    SDL_Keycode key = event->key.keysym.sym;
    SDL_Keymod modifiers = (SDL_Keymod)(event->key.keysym.mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT));

    // Verificar se corresponde à hotkey da ação solicitada
    return (hotkeys->hotkeys[action].enabled &&
            hotkeys->hotkeys[action].key == key &&
            hotkeys->hotkeys[action].modifiers == modifiers);
}

/**
 * @brief Salva a configuração de hotkeys em um arquivo
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param filepath Caminho do arquivo
 * @return true Se salvo com sucesso
 * @return false Se falhou
 */
bool sdl_hotkeys_save_config(const sdl_hotkeys_t *hotkeys, const char *filepath) {
    if (!hotkeys || !hotkeys->initialized || !filepath) {
        return false;
    }

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        return false;
    }

    // Salvar versão do formato
    uint32_t version = 1;
    fwrite(&version, sizeof(uint32_t), 1, file);

    // Salvar número de hotkeys
    fwrite(&hotkeys->count, sizeof(int), 1, file);

    // Salvar cada hotkey
    for (int i = 0; i < SDL_HOTKEY_ACTION_COUNT; i++) {
        if (hotkeys->hotkeys[i].enabled) {
            fwrite(&i, sizeof(int), 1, file);  // Índice da ação
            fwrite(&hotkeys->hotkeys[i], sizeof(sdl_hotkey_t), 1, file);
        }
    }

    fclose(file);
    return true;
}

/**
 * @brief Carrega a configuração de hotkeys de um arquivo
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param filepath Caminho do arquivo
 * @return true Se carregado com sucesso
 * @return false Se falhou
 */
bool sdl_hotkeys_load_config(sdl_hotkeys_t *hotkeys, const char *filepath) {
    if (!hotkeys || !hotkeys->initialized || !filepath) {
        return false;
    }

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return false;
    }

    // Verificar versão do formato
    uint32_t version;
    if (fread(&version, sizeof(uint32_t), 1, file) != 1 || version != 1) {
        fclose(file);
        return false;
    }

    // Limpar hotkeys existentes
    sdl_hotkeys_clear_all(hotkeys);

    // Ler número de hotkeys
    int count;
    if (fread(&count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return false;
    }

    // Ler cada hotkey
    for (int i = 0; i < count; i++) {
        int action_index;
        if (fread(&action_index, sizeof(int), 1, file) != 1) {
            break;
        }

        if (action_index > 0 && action_index < SDL_HOTKEY_ACTION_COUNT) {
            if (fread(&hotkeys->hotkeys[action_index], sizeof(sdl_hotkey_t), 1, file) != 1) {
                break;
            }
            hotkeys->hotkeys[action_index].enabled = true;
        }
    }

    hotkeys->count = count;
    fclose(file);
    return true;
}

/**
 * @brief Redefine as hotkeys para os valores padrão
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 */
void sdl_hotkeys_reset_to_defaults(sdl_hotkeys_t *hotkeys) {
    if (!hotkeys || !hotkeys->initialized) {
        return;
    }

    // Limpar todas as hotkeys
    sdl_hotkeys_clear_all(hotkeys);

    // Definir hotkeys padrão
    size_t num_defaults = sizeof(default_hotkeys) / sizeof(default_hotkeys[0]);
    for (size_t i = 0; i < num_defaults; i++) {
        const sdl_hotkey_t *def = &default_hotkeys[i];
        sdl_hotkeys_set(hotkeys, def->action, def->key, def->modifiers, def->param);
    }

    hotkeys->count = num_defaults;
}

/**
 * @brief Define todos os callbacks para o sistema de hotkeys
 *
 * @param hotkeys Ponteiro para a estrutura de hotkeys
 * @param callbacks... Ponteiros para funções de callback
 * @param userdata Dados do usuário para os callbacks
 */
void sdl_hotkeys_set_callbacks(
    sdl_hotkeys_t *hotkeys,
    void (*toggle_fullscreen)(void *userdata),
    void (*save_state)(int slot, void *userdata),
    void (*load_state)(int slot, void *userdata),
    void (*reset)(void *userdata),
    void (*quit)(void *userdata),
    void (*pause)(bool state, void *userdata),
    void (*fast_forward)(bool state, void *userdata),
    void (*slow_motion)(bool state, void *userdata),
    void (*screenshot)(void *userdata),
    void (*record_video)(bool state, void *userdata),
    void (*rewind)(bool state, void *userdata),
    void (*next_slot)(void *userdata),
    void (*prev_slot)(void *userdata),
    void (*toggle_scanlines)(void *userdata),
    void (*toggle_crt)(void *userdata),
    void (*mute)(bool state, void *userdata),
    void (*volume_up)(void *userdata),
    void (*volume_down)(void *userdata),
    void (*toggle_menu)(void *userdata),
    void (*toggle_debug_info)(void *userdata),
    void *userdata
) {
    if (!hotkeys || !hotkeys->initialized) {
        return;
    }

    // Definir callbacks
    hotkeys->callbacks.toggle_fullscreen = toggle_fullscreen;
    hotkeys->callbacks.save_state = save_state;
    hotkeys->callbacks.load_state = load_state;
    hotkeys->callbacks.reset = reset;
    hotkeys->callbacks.quit = quit;
    hotkeys->callbacks.pause = pause;
    hotkeys->callbacks.fast_forward = fast_forward;
    hotkeys->callbacks.slow_motion = slow_motion;
    hotkeys->callbacks.screenshot = screenshot;
    hotkeys->callbacks.record_video = record_video;
    hotkeys->callbacks.rewind = rewind;
    hotkeys->callbacks.next_slot = next_slot;
    hotkeys->callbacks.prev_slot = prev_slot;
    hotkeys->callbacks.toggle_scanlines = toggle_scanlines;
    hotkeys->callbacks.toggle_crt = toggle_crt;
    hotkeys->callbacks.mute = mute;
    hotkeys->callbacks.volume_up = volume_up;
    hotkeys->callbacks.volume_down = volume_down;
    hotkeys->callbacks.toggle_menu = toggle_menu;
    hotkeys->callbacks.toggle_debug_info = toggle_debug_info;

    // Definir dados do usuário
    hotkeys->userdata = userdata;
}

/**
 * @brief Obtém o nome de uma ação de hotkey
 *
 * @param action Ação a obter o nome
 * @return const char* Nome da ação, NULL se não encontrada
 */
const char* sdl_hotkeys_get_action_name(sdl_hotkey_action_t action) {
    if (action <= SDL_HOTKEY_ACTION_NONE || action >= SDL_HOTKEY_ACTION_COUNT) {
        return NULL;
    }

    // Procurar na tabela de nomes
    for (size_t i = 0; i < sizeof(hotkey_action_names) / sizeof(hotkey_action_names[0]); i++) {
        if (hotkey_action_names[i].action == action) {
            return hotkey_action_names[i].name;
        }
    }

    return NULL;
}

/**
 * @brief Obtém o nome formatado de uma tecla e seus modificadores
 *
 * @param key Tecla
 * @param modifiers Modificadores
 * @param output Buffer para o nome formatado
 * @param max_length Tamanho máximo do buffer
 * @return true Se obtido com sucesso
 * @return false Se falhou
 */
bool sdl_hotkeys_get_key_name(SDL_Keycode key, SDL_Keymod modifiers, char *output, size_t max_length) {
    if (!output || max_length == 0) {
        return false;
    }

    // Limpar buffer
    memset(output, 0, max_length);

    // Adicionar modificadores
    size_t offset = 0;
    if (modifiers & KMOD_CTRL) {
        strncpy(output + offset, "Ctrl+", max_length - offset);
        offset += 5;
    }
    if (modifiers & KMOD_SHIFT) {
        strncpy(output + offset, "Shift+", max_length - offset);
        offset += 6;
    }
    if (modifiers & KMOD_ALT) {
        strncpy(output + offset, "Alt+", max_length - offset);
        offset += 4;
    }

    // Obter nome da tecla
    const char *key_name = SDL_GetKeyName(key);
    if (key_name) {
        strncpy(output + offset, key_name, max_length - offset);
        return true;
    }

    return false;
}
