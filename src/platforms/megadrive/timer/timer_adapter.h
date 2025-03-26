/**
 * @file timer_adapter.h
 * @brief Adaptador de timer para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef MEGADRIVE_TIMER_ADAPTER_H
#define MEGADRIVE_TIMER_ADAPTER_H

#include "core/interfaces/timer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Constantes do timer
 */
#define MD_MASTER_CLOCK 53693175 // Clock principal (NTSC)
#define MD_Z80_CLOCK 3579545     // Clock do Z80
#define MD_TIMER_COUNT 4         // Número de timers

/**
 * @brief Tipos de timer do Mega Drive
 */
typedef enum {
  MD_TIMER_HBLANK = 0, // Timer de interrupção horizontal
  MD_TIMER_VBLANK,     // Timer de interrupção vertical
  MD_TIMER_Z80,        // Timer do Z80
  MD_TIMER_YM2612      // Timer do YM2612
} md_timer_type_t;

/**
 * @brief Estado de um timer
 */
typedef struct {
  md_timer_type_t type;     // Tipo do timer
  uint32_t period;          // Período em ciclos
  uint32_t counter;         // Contador atual
  uint32_t reload;          // Valor de recarga
  uint32_t prescaler;       // Divisor de frequência
  bool enabled;             // Timer habilitado
  bool expired;             // Timer expirou
  void (*callback)(void *); // Callback ao expirar
  void *user_data;          // Dados do usuário
} md_timer_state_t;

/**
 * @brief Contexto específico do adaptador de timer
 */
typedef struct {
  md_timer_state_t timers[MD_TIMER_COUNT]; // Estado dos timers
  uint32_t master_clock;                   // Clock principal
  uint32_t cycles;                         // Ciclos executados
  bool enabled;                            // Sistema habilitado
  void *user_data;                         // Dados específicos da implementação
} megadrive_timer_context_t;

/**
 * @brief Cria uma nova instância do adaptador de timer
 * @return Ponteiro para a interface de timer ou NULL em caso de erro
 */
emu_timer_interface_t *megadrive_timer_adapter_create(void);

/**
 * @brief Destrói uma instância do adaptador de timer
 * @param timer Ponteiro para a interface de timer
 */
void megadrive_timer_adapter_destroy(emu_timer_interface_t *timer);

/**
 * @brief Obtém o contexto específico do adaptador de timer
 * @param timer Ponteiro para a interface de timer
 * @return Ponteiro para o contexto ou NULL em caso de erro
 */
megadrive_timer_context_t *
megadrive_timer_get_context(emu_timer_interface_t *timer);

/**
 * @brief Define o contexto específico do adaptador de timer
 * @param timer Ponteiro para a interface de timer
 * @param context Ponteiro para o novo contexto
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int megadrive_timer_set_context(emu_timer_interface_t *timer,
                                const megadrive_timer_context_t *context);

/**
 * @brief Configura um timer específico
 * @param timer Ponteiro para a interface de timer
 * @param type Tipo do timer
 * @param period Período em ciclos
 * @param callback Callback ao expirar
 * @param user_data Dados do usuário
 */
void megadrive_timer_configure(emu_timer_interface_t *timer,
                               md_timer_type_t type, uint32_t period,
                               void (*callback)(void *), void *user_data);

/**
 * @brief Habilita ou desabilita um timer
 * @param timer Ponteiro para a interface de timer
 * @param type Tipo do timer
 * @param enabled Estado do timer
 */
void megadrive_timer_enable(emu_timer_interface_t *timer, md_timer_type_t type,
                            bool enabled);

/**
 * @brief Define o prescaler de um timer
 * @param timer Ponteiro para a interface de timer
 * @param type Tipo do timer
 * @param prescaler Valor do prescaler
 */
void megadrive_timer_set_prescaler(emu_timer_interface_t *timer,
                                   md_timer_type_t type, uint32_t prescaler);

/**
 * @brief Obtém o estado de um timer
 * @param timer Ponteiro para a interface de timer
 * @param type Tipo do timer
 * @param state Ponteiro para receber o estado
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int megadrive_timer_get_state(emu_timer_interface_t *timer,
                              md_timer_type_t type, md_timer_state_t *state);

#ifdef __cplusplus
}
#endif

#endif // MEGADRIVE_TIMER_ADAPTER_H
