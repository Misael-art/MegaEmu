/**
 * @file video_interface.h
 * @brief Interface padrão para sistemas de vídeo no Mega_Emu
 * @version 2.0
 *
 * Esta interface DEVE ser implementada por todos os adaptadores de vídeo.
 * Parte da Fase 1 do plano de migração.
 */

#ifndef EMU_VIDEO_INTERFACE_H
#define EMU_VIDEO_INTERFACE_H

#include "core_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Modos de vídeo suportados
 */
typedef enum {
  EMU_VIDEO_MODE_DISABLED = 0,
  EMU_VIDEO_MODE_TEXT,
  EMU_VIDEO_MODE_BITMAP,
  EMU_VIDEO_MODE_TILE,
  EMU_VIDEO_MODE_SPRITE,
  EMU_VIDEO_MODE_MIXED
} emu_video_mode_t;

/**
 * @brief Flags de status do vídeo
 */
typedef enum {
  EMU_VIDEO_FLAG_NONE = 0x00,
  EMU_VIDEO_FLAG_VBLANK = 0x01,
  EMU_VIDEO_FLAG_HBLANK = 0x02,
  EMU_VIDEO_FLAG_SPRITE_OVF = 0x04,
  EMU_VIDEO_FLAG_COLLISION = 0x08,
  EMU_VIDEO_FLAG_INT_PENDING = 0x10
} emu_video_flags_t;

/**
 * @brief Configuração do sistema de vídeo
 */
typedef struct {
  uint16_t width;        // Largura da tela
  uint16_t height;       // Altura da tela
  uint8_t bpp;           // Bits por pixel
  emu_video_mode_t mode; // Modo de vídeo
  bool double_buffering; // Usar double buffering
  bool interlaced;       // Modo entrelaçado
  void *user_data;       // Dados específicos da implementação
} emu_video_config_t;

/**
 * @brief Estado do sistema de vídeo
 */
typedef struct {
  uint16_t line;           // Linha atual
  uint16_t cycle;          // Ciclo atual na linha
  emu_video_flags_t flags; // Flags de status
  void *context;           // Contexto específico
} emu_video_state_t;

/**
 * @brief Interface padrão para sistemas de vídeo
 */
typedef struct {
  void *context; // Contexto da implementação

  // Funções de ciclo de vida
  int (*init)(void *ctx, const emu_video_config_t *config);
  void (*reset)(void *ctx);
  void (*shutdown)(void *ctx);

  // Funções de renderização
  void (*begin_frame)(void *ctx);
  void (*end_frame)(void *ctx);
  void (*render_line)(void *ctx, int line);
  void (*update)(void *ctx, int cycles);

  // Funções de acesso
  void (*write_register)(void *ctx, uint16_t reg, uint8_t val);
  uint8_t (*read_register)(void *ctx, uint16_t reg);
  void (*write_vram)(void *ctx, uint32_t addr, uint8_t val);
  uint8_t (*read_vram)(void *ctx, uint32_t addr);

  // Funções de estado
  void (*get_state)(void *ctx, emu_video_state_t *state);
  void (*set_state)(void *ctx, const emu_video_state_t *state);

  // Funções de debug
  void (*dump_vram)(void *ctx, uint8_t *buffer, uint32_t size);
  void (*dump_palette)(void *ctx, uint32_t *buffer, uint32_t size);
  void (*dump_sprites)(void *ctx, void *buffer, uint32_t size);
  const char *(*get_mode_name)(void *ctx);
} emu_video_interface_t;

/**
 * @brief Cria uma nova instância da interface de vídeo
 * @param type Tipo de sistema de vídeo
 * @return Ponteiro para a interface ou NULL em caso de erro
 */
emu_video_interface_t *emu_video_create(int type);

/**
 * @brief Destrói uma instância da interface de vídeo
 * @param video Ponteiro para a interface
 */
void emu_video_destroy(emu_video_interface_t *video);

#ifdef __cplusplus
}
#endif

#endif // EMU_VIDEO_INTERFACE_H
