/**
 * @file frontend.h
 * @brief Interface principal para o frontend do emulador
 */
#ifndef EMU_FRONTEND_H
#define EMU_FRONTEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../gui/core/gui_manager.h"
#include "frontend_config.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Tipos de plataforma suportados pelo emulador
 */
typedef enum {
  EMU_PLATFORM_NONE = 0,
  EMU_PLATFORM_AUTO,
  EMU_PLATFORM_MEGA_DRIVE,
  EMU_PLATFORM_MASTER_SYSTEM,
  EMU_PLATFORM_GAME_GEAR,
  EMU_PLATFORM_NES,
  EMU_PLATFORM_SNES,
  EMU_PLATFORM_GAME_BOY,
  EMU_PLATFORM_GAME_BOY_COLOR,
  EMU_PLATFORM_GAME_BOY_ADVANCE,
  EMU_PLATFORM_ATARI_2600,
  EMU_PLATFORM_ATARI_7800,
  EMU_PLATFORM_COLECOVISION,
  EMU_PLATFORM_PC_ENGINE,
  EMU_PLATFORM_NEO_GEO,
  // Futuras plataformas...
} emu_platform_t;

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
 * @return emu_frontend_t Handle para o frontend inicializado, ou NULL em caso
 * de erro
 */
emu_frontend_t emu_frontend_init(const char *title, int32_t width,
                                 int32_t height);

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
int32_t emu_frontend_render_frame(emu_frontend_t frontend,
                                  const uint32_t *framebuffer, int32_t width,
                                  int32_t height);

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
 * @return bool true se o frontend deve continuar executando, false caso
 * contrário
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
 * @return gui_element_id_t ID do elemento criado ou GUI_INVALID_ID em caso de
 * erro
 */
gui_element_id_t emu_frontend_create_element(emu_frontend_t frontend,
                                             gui_element_type_t type,
                                             const gui_rect_t *rect,
                                             const char *text);

/**
 * @brief Remove um elemento de GUI do frontend
 *
 * @param frontend Handle para o frontend
 * @param element_id ID do elemento a ser removido
 */
void emu_frontend_remove_element(emu_frontend_t frontend,
                                 gui_element_id_t element_id);

/**
 * @brief Define a cor de fundo do frontend
 *
 * @param frontend Handle para o frontend
 * @param r Componente vermelho (0-255)
 * @param g Componente verde (0-255)
 * @param b Componente azul (0-255)
 * @param a Componente alfa (0-255)
 */
void emu_frontend_set_background_color(emu_frontend_t frontend, uint8_t r,
                                       uint8_t g, uint8_t b, uint8_t a);

/**
 * @brief Verifica se o emulador está rodando
 *
 * @return bool true se estiver rodando
 */
bool emu_frontend_is_running(void);

/**
 * @brief Verifica se o emulador está pausado
 *
 * @return bool true se estiver pausado
 */
bool emu_frontend_is_paused(void);

/**
 * @brief Pausa ou resume a emulação
 *
 * @param paused true para pausar, false para retomar
 */
void emu_frontend_set_paused(bool paused);

/**
 * @brief Carrega uma ROM no emulador e inicia a emulação
 *
 * @param rom_path Caminho para o arquivo da ROM
 * @param platform Plataforma a ser emulada (EMU_PLATFORM_AUTO para
 * autodetecção)
 * @return bool true se a ROM foi carregada com sucesso
 */
bool emu_frontend_load_rom(const char *rom_path, emu_platform_t platform);

/**
 * @brief Descarrega a ROM atual e para a emulação
 */
void emu_frontend_unload_rom(void);

/**
 * @brief Reinicia a emulação da ROM atual
 *
 * @return bool true se a ROM foi reiniciada com sucesso
 */
bool emu_frontend_reset_current_rom(void);

/**
 * @brief Define o diretório de ROMs
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_rom_directory(const char *directory);

/**
 * @brief Obtém o diretório de ROMs atual
 *
 * @return const char* Caminho para o diretório
 */
const char *emu_frontend_get_rom_directory(void);

/**
 * @brief Define o diretório de saves
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_save_directory(const char *directory);

/**
 * @brief Obtém o diretório de saves atual
 *
 * @return const char* Caminho para o diretório
 */
const char *emu_frontend_get_save_directory(void);

/**
 * @brief Define o diretório de screenshots
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_screenshots_directory(const char *directory);

/**
 * @brief Obtém o diretório de screenshots atual
 *
 * @return const char* Caminho para o diretório
 */
const char *emu_frontend_get_screenshots_directory(void);

/**
 * @brief Define o diretório de save states
 *
 * @param directory Caminho para o diretório
 */
void emu_frontend_set_states_directory(const char *directory);

/**
 * @brief Obtém o diretório de save states atual
 *
 * @return const char* Caminho para o diretório
 */
const char *emu_frontend_get_states_directory(void);

/**
 * @brief Obtém a plataforma atualmente carregada
 *
 * @return emu_platform_t Tipo de plataforma
 */
emu_platform_t emu_frontend_get_current_platform(void);

/**
 * @brief Obtém o caminho para a ROM atualmente carregada
 *
 * @return const char* Caminho da ROM
 */
const char *emu_frontend_get_current_rom_path(void);

/**
 * @brief Verifica se o frontend está inicializado
 *
 * @return bool true se estiver inicializado
 */
bool emu_frontend_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* EMU_FRONTEND_H */
