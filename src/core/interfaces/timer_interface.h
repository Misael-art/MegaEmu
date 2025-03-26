/**
 * @file timer_interface.h
 * @brief Interface padrão para sistemas de timer no Mega_Emu
 * @version 2.0
 *
 * Esta interface DEVE ser implementada por todos os adaptadores de timer.
 * Parte da Fase 1 do plano de migração.
 */

#ifndef EMU_TIMER_INTERFACE_H
#define EMU_TIMER_INTERFACE_H

#include "core_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tipos de timer suportados
 */
typedef enum {
  EMU_TIMER_TYPE_NONE = 0,
  EMU_TIMER_TYPE_CYCLE,    // Baseado em ciclos de CPU
  EMU_TIMER_TYPE_SCANLINE, // Baseado em linhas de varredura
  EMU_TIMER_TYPE_FRAME,    // Baseado em quadros
  EMU_TIMER_TYPE_REAL      // Baseado em tempo real
} emu_timer_type_t;

/**
 * @brief Modos de operação do timer
 */
typedef enum {
  EMU_TIMER_MODE_ONESHOT = 0, // Dispara uma vez e para
  EMU_TIMER_MODE_PERIODIC,    // Dispara periodicamente
  EMU_TIMER_MODE_PWM,         // Modulação por largura de pulso
  EMU_TIMER_MODE_WATCHDOG     // Modo watchdog
} emu_timer_mode_t;

/**
 * @brief Flags de status do timer
 */
typedef enum {
  EMU_TIMER_FLAG_NONE = 0x00,
  EMU_TIMER_FLAG_RUNNING = 0x01,
  EMU_TIMER_FLAG_EXPIRED = 0x02,
  EMU_TIMER_FLAG_OVERFLOW = 0x04,
  EMU_TIMER_FLAG_UNDERFLOW = 0x08,
  EMU_TIMER_FLAG_ERROR = 0x10
} emu_timer_flags_t;

/**
 * @brief Configuração do timer
 */
typedef struct {
  emu_timer_type_t type;    // Tipo do timer
  emu_timer_mode_t mode;    // Modo de operação
  uint32_t period;          // Período do timer
  uint32_t prescaler;       // Divisor de frequência
  bool auto_reload;         // Recarrega automaticamente
  void (*callback)(void *); // Callback ao expirar
  void *user_data;          // Dados específicos da implementação
} emu_timer_config_t;

/**
 * @brief Estado do timer
 */
typedef struct {
  uint32_t counter;        // Contador atual
  uint32_t compare;        // Valor de comparação
  uint32_t reload;         // Valor de recarga
  emu_timer_flags_t flags; // Flags de status
  void *context;           // Contexto específico
} emu_timer_state_t;

/**
 * @brief Interface padrão para sistemas de timer
 */
typedef struct {
  void *context; // Contexto da implementação

  // Funções de ciclo de vida
  int (*init)(void *ctx, const emu_timer_config_t *config);
  void (*reset)(void *ctx);
  void (*shutdown)(void *ctx);

  // Funções de controle
  void (*start)(void *ctx);
  void (*stop)(void *ctx);
  void (*pause)(void *ctx);
  void (*resume)(void *ctx);

  // Funções de configuração
  void (*set_period)(void *ctx, uint32_t period);
  void (*set_prescaler)(void *ctx, uint32_t prescaler);
  void (*set_compare)(void *ctx, uint32_t compare);
  void (*set_reload)(void *ctx, uint32_t reload);
  void (*set_mode)(void *ctx, emu_timer_mode_t mode);
  void (*set_callback)(void *ctx, void (*callback)(void *), void *user_data);

  // Funções de contagem
  void (*update)(void *ctx, uint32_t cycles);
  uint32_t (*get_counter)(void *ctx);
  uint32_t (*get_elapsed)(void *ctx);
  uint32_t (*get_remaining)(void *ctx);

  // Funções de estado
  void (*get_state)(void *ctx, emu_timer_state_t *state);
  void (*set_state)(void *ctx, const emu_timer_state_t *state);
  emu_timer_flags_t (*get_flags)(void *ctx);

  // Funções de debug
  void (*dump_state)(void *ctx, void *buffer, uint32_t size);
  void (*get_stats)(void *ctx, void *stats, uint32_t size);
  const char *(*get_mode_name)(void *ctx);
} emu_timer_interface_t;

/**
 * @brief Cria uma nova instância da interface de timer
 * @param type Tipo de timer
 * @return Ponteiro para a interface ou NULL em caso de erro
 */
emu_timer_interface_t *emu_timer_create(emu_timer_type_t type);

/**
 * @brief Destrói uma instância da interface de timer
 * @param timer Ponteiro para a interface
 */
void emu_timer_destroy(emu_timer_interface_t *timer);

#ifdef __cplusplus
}
#endif

#endif // EMU_TIMER_INTERFACE_H
