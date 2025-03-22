#include <stdint.h>/** * @file nes_window.h * @brief Interface para a janela principal do emulador NES */#ifndef NES_WINDOW_H#define NES_WINDOW_H#ifdef __cplusplusextern "C"{#endif    /**     * @brief Inicializa e executa o emulador NES com a ROM especificada     *     * @param rom_path Caminho para o arquivo ROM     * @return int32_t 0 em caso de sucesso, -1 em caso de erro     */    int32_t run_nes_emulator(const char *rom_path);#ifdef __cplusplus}#endif#endif // NES_WINDOW_H