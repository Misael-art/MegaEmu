/**
 * @file audio_interface.h
 * @brief Interface padrão para sistemas de áudio no Mega_Emu
 * @version 2.0
 *
 * Esta interface DEVE ser implementada por todos os adaptadores de áudio.
 * Parte da Fase 1 do plano de migração.
 */

#ifndef EMU_AUDIO_INTERFACE_H
#define EMU_AUDIO_INTERFACE_H

#include "core_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Formatos de áudio suportados
 */
typedef enum {
  EMU_AUDIO_FORMAT_U8 = 0,
  EMU_AUDIO_FORMAT_S8,
  EMU_AUDIO_FORMAT_U16,
  EMU_AUDIO_FORMAT_S16,
  EMU_AUDIO_FORMAT_U32,
  EMU_AUDIO_FORMAT_S32,
  EMU_AUDIO_FORMAT_F32
} emu_audio_format_t;

/**
 * @brief Flags de status do áudio
 */
typedef enum {
  EMU_AUDIO_FLAG_NONE = 0x00,
  EMU_AUDIO_FLAG_PLAYING = 0x01,
  EMU_AUDIO_FLAG_PAUSED = 0x02,
  EMU_AUDIO_FLAG_BUFFERING = 0x04,
  EMU_AUDIO_FLAG_OVERFLOW = 0x08,
  EMU_AUDIO_FLAG_UNDERFLOW = 0x10
} emu_audio_flags_t;

/**
 * @brief Configuração do sistema de áudio
 */
typedef struct {
  uint32_t sample_rate;      // Taxa de amostragem em Hz
  uint8_t channels;          // Número de canais
  uint8_t bits_per_sample;   // Bits por amostra
  emu_audio_format_t format; // Formato do áudio
  uint32_t buffer_size;      // Tamanho do buffer em amostras
  void *user_data;           // Dados específicos da implementação
} emu_audio_config_t;

/**
 * @brief Estado do sistema de áudio
 */
typedef struct {
  uint32_t samples_played; // Total de amostras reproduzidas
  uint32_t buffer_level;   // Nível atual do buffer
  emu_audio_flags_t flags; // Flags de status
  void *context;           // Contexto específico
} emu_audio_state_t;

/**
 * @brief Interface padrão para sistemas de áudio
 */
typedef struct {
  void *context; // Contexto da implementação

  // Funções de ciclo de vida
  int (*init)(void *ctx, const emu_audio_config_t *config);
  void (*reset)(void *ctx);
  void (*shutdown)(void *ctx);

  // Funções de controle
  void (*start)(void *ctx);
  void (*stop)(void *ctx);
  void (*pause)(void *ctx);
  void (*resume)(void *ctx);

  // Funções de buffer
  int (*write_samples)(void *ctx, const void *buffer, uint32_t num_samples);
  int (*read_samples)(void *ctx, void *buffer, uint32_t num_samples);
  void (*clear_buffer)(void *ctx);

  // Funções de estado
  void (*get_state)(void *ctx, emu_audio_state_t *state);
  void (*set_state)(void *ctx, const emu_audio_state_t *state);
  uint32_t (*get_buffer_space)(void *ctx);
  uint32_t (*get_latency)(void *ctx);

  // Funções de configuração
  void (*set_volume)(void *ctx, float volume);
  void (*set_panning)(void *ctx, float left, float right);
  void (*set_sample_rate)(void *ctx, uint32_t sample_rate);

  // Funções de debug
  void (*dump_buffer)(void *ctx, void *buffer, uint32_t size);
  void (*get_stats)(void *ctx, void *stats, uint32_t size);
  const char *(*get_backend_name)(void *ctx);
} emu_audio_interface_t;

/**
 * @brief Cria uma nova instância da interface de áudio
 * @param type Tipo de sistema de áudio
 * @return Ponteiro para a interface ou NULL em caso de erro
 */
emu_audio_interface_t *emu_audio_create(int type);

/**
 * @brief Destrói uma instância da interface de áudio
 * @param audio Ponteiro para a interface
 */
void emu_audio_destroy(emu_audio_interface_t *audio);

#ifdef __cplusplus
}
#endif

#endif // EMU_AUDIO_INTERFACE_H
