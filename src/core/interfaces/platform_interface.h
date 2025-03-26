/**
 * @file platform_interface.h
 * @brief Interface genérica para implementações de plataforma
 *
 * Este arquivo define a interface que todas as implementações
 * de plataforma devem implementar para serem compatíveis com
 * o core do emulador Mega_Emu.
 */

#ifndef EMU_PLATFORM_INTERFACE_H
#define EMU_PLATFORM_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>
#include "memory_interface.h"
#include "video_interface.h"
#include "audio_interface.h"
#include "state_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct emu_platform_s emu_platform_t;
typedef struct emu_platform_info_s emu_platform_info_t;

// Informações da plataforma
struct emu_platform_info_s {
  char name[32];              /**< Nome da plataforma */
  char id[8];                /**< Identificador único */
  uint32_t cpu_clock;        /**< Clock da CPU em Hz */
  uint32_t vdp_clock;        /**< Clock do VDP em Hz */
  uint32_t sound_clock;      /**< Clock do sistema de som em Hz */
  uint16_t screen_width;     /**< Largura da tela */
  uint16_t screen_height;    /**< Altura da tela */
  bool has_secondary_cpu;    /**< Possui CPU secundária */
  bool has_color;           /**< Suporta cores */
  uint8_t max_sprites;      /**< Número máximo de sprites */
  uint8_t max_colors;       /**< Número máximo de cores */
};

// Estrutura principal da plataforma
struct emu_platform_s {
  emu_platform_info_t info;   /**< Informações da plataforma */
  void *platform_data;        /**< Dados específicos da plataforma */
  emu_memory_t memory;        /**< Sistema de memória */
  emu_video_t video;         /**< Sistema de vídeo */
  emu_audio_t audio;         /**< Sistema de áudio */
  bool initialized;           /**< Flag de inicialização */

  // Funções obrigatórias
  bool (*init)(emu_platform_t *platform, void *ctx);
  void (*shutdown)(emu_platform_t *platform);
  bool (*reset)(emu_platform_t *platform);
  bool (*load_rom)(emu_platform_t *platform, const char *filename);
  bool (*run_frame)(emu_platform_t *platform);
  uint32_t (*run_cycles)(emu_platform_t *platform, uint32_t cycles);

  // Funções de estado
  void (*save_state)(emu_platform_t *platform, emu_state_t *state);
  bool (*load_state)(emu_platform_t *platform, const emu_state_t *state);
};

// Funções de interface pública
emu_platform_t *emu_platform_create(void);
void emu_platform_destroy(emu_platform_t *platform);
bool emu_platform_init(emu_platform_t *platform, void *ctx);
void emu_platform_shutdown(emu_platform_t *platform);
bool emu_platform_reset(emu_platform_t *platform);
bool emu_platform_load_rom(emu_platform_t *platform, const char *filename);
bool emu_platform_run_frame(emu_platform_t *platform);
uint32_t emu_platform_run_cycles(emu_platform_t *platform, uint32_t cycles);
void emu_platform_save_state(emu_platform_t *platform, emu_state_t *state);
bool emu_platform_load_state(emu_platform_t *platform, const emu_state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* EMU_PLATFORM_INTERFACE_H */
