#ifndef MENU_SOUNDS_H
#define MENU_SOUNDS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Estrutura para efeitos sonoros do menu
    typedef struct
    {
        SDL_AudioDeviceID device;
        uint8_t *hover_sound;
        uint8_t *select_sound;
        uint8_t *back_sound;
        int hover_length;
        int select_length;
        int back_length;
    } menu_sounds_t;

    // Funções para gerenciar sons do menu
    bool menu_sounds_init(void);
    void menu_sounds_play_hover(void);
    void menu_sounds_play_select(void);
    void menu_sounds_play_back(void);
    void menu_sounds_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // MENU_SOUNDS_H
