/**
 * @file timer_adapter.c
 * @brief Implementação do adaptador de timer para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "timer_adapter.h"
#include <stdlib.h>
#include <string.h>

// Funções estáticas do adaptador
static int adapter_init(void *ctx, const emu_timer_config_t *config) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context || !config)
    return -1;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));

  // Configura clock principal
  context->master_clock = MD_MASTER_CLOCK;
  context->cycles = 0;
  context->enabled = true;

  // Inicializa timers
  for (int i = 0; i < MD_TIMER_COUNT; i++) {
    context->timers[i].type = (md_timer_type_t)i;
    context->timers[i].period = 0;
    context->timers[i].counter = 0;
    context->timers[i].reload = 0;
    context->timers[i].prescaler = 1;
    context->timers[i].enabled = false;
    context->timers[i].expired = false;
    context->timers[i].callback = NULL;
    context->timers[i].user_data = NULL;
  }

  return 0;
}

static void adapter_reset(void *ctx) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Reseta contadores
  context->cycles = 0;

  // Reseta timers
  for (int i = 0; i < MD_TIMER_COUNT; i++) {
    context->timers[i].counter = context->timers[i].reload;
    context->timers[i].expired = false;
  }
}

static void adapter_shutdown(void *ctx) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));
}

static void adapter_start(void *ctx) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;
  context->enabled = true;
}

static void adapter_stop(void *ctx) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;
  context->enabled = false;
}

static void adapter_pause(void *ctx) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;
  context->enabled = false;
}

static void adapter_resume(void *ctx) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;
  context->enabled = true;
}

static void adapter_update(void *ctx, uint32_t cycles) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context || !context->enabled)
    return;

  // Atualiza contador global
  context->cycles += cycles;

  // Atualiza cada timer
  for (int i = 0; i < MD_TIMER_COUNT; i++) {
    md_timer_state_t *timer = &context->timers[i];
    if (!timer->enabled || timer->period == 0)
      continue;

    // Atualiza contador
    uint32_t prev_counter = timer->counter;
    timer->counter += cycles / timer->prescaler;

    // Verifica expiração
    if (timer->counter >= timer->period) {
      timer->expired = true;
      timer->counter = timer->reload;

      // Chama callback se definido
      if (timer->callback) {
        timer->callback(timer->user_data);
      }
    }
    // Verifica overflow
    else if (timer->counter < prev_counter) {
      timer->counter = timer->period - 1;
    }
  }
}

static void adapter_set_period(void *ctx, uint32_t period) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Define período para o timer HBLANK por padrão
  context->timers[MD_TIMER_HBLANK].period = period;
  context->timers[MD_TIMER_HBLANK].reload = 0;
}

static void adapter_set_prescaler(void *ctx, uint32_t prescaler) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Define prescaler para o timer HBLANK por padrão
  context->timers[MD_TIMER_HBLANK].prescaler = prescaler;
}

static void adapter_set_compare(void *ctx, uint32_t compare) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Define valor de comparação para o timer HBLANK por padrão
  context->timers[MD_TIMER_HBLANK].period = compare;
}

static void adapter_set_reload(void *ctx, uint32_t reload) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Define valor de recarga para o timer HBLANK por padrão
  context->timers[MD_TIMER_HBLANK].reload = reload;
}

static void adapter_set_mode(void *ctx, emu_timer_mode_t mode) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Configura modo para o timer HBLANK por padrão
  switch (mode) {
  case EMU_TIMER_MODE_ONESHOT:
    context->timers[MD_TIMER_HBLANK].reload = 0;
    break;
  case EMU_TIMER_MODE_PERIODIC:
    context->timers[MD_TIMER_HBLANK].reload =
        context->timers[MD_TIMER_HBLANK].period;
    break;
  default:
    break;
  }
}

static void adapter_set_callback(void *ctx, void (*callback)(void *),
                                 void *user_data) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context)
    return;

  // Define callback para o timer HBLANK por padrão
  context->timers[MD_TIMER_HBLANK].callback = callback;
  context->timers[MD_TIMER_HBLANK].user_data = user_data;
}

static void adapter_get_state(void *ctx, emu_timer_state_t *state) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context || !state)
    return;

  // Retorna estado do timer HBLANK por padrão
  state->counter = context->timers[MD_TIMER_HBLANK].counter;
  state->compare = context->timers[MD_TIMER_HBLANK].period;
  state->reload = context->timers[MD_TIMER_HBLANK].reload;
  state->flags = 0;
  if (context->enabled)
    state->flags |= EMU_TIMER_FLAG_RUNNING;
  if (context->timers[MD_TIMER_HBLANK].expired)
    state->flags |= EMU_TIMER_FLAG_EXPIRED;
  state->context = context;
}

static void adapter_set_state(void *ctx, const emu_timer_state_t *state) {
  megadrive_timer_context_t *context = (megadrive_timer_context_t *)ctx;
  if (!context || !state)
    return;

  // Atualiza estado do timer HBLANK por padrão
  context->timers[MD_TIMER_HBLANK].counter = state->counter;
  context->timers[MD_TIMER_HBLANK].period = state->compare;
  context->timers[MD_TIMER_HBLANK].reload = state->reload;
  context->enabled = (state->flags & EMU_TIMER_FLAG_RUNNING) != 0;
  context->timers[MD_TIMER_HBLANK].expired =
      (state->flags & EMU_TIMER_FLAG_EXPIRED) != 0;
}

// Funções públicas
emu_timer_interface_t *megadrive_timer_adapter_create(void) {
  emu_timer_interface_t *interface = calloc(1, sizeof(emu_timer_interface_t));
  megadrive_timer_context_t *context =
      calloc(1, sizeof(megadrive_timer_context_t));

  if (!interface || !context) {
    free(interface);
    free(context);
    return NULL;
  }

  // Configura a interface
  interface->context = context;
  interface->init = adapter_init;
  interface->reset = adapter_reset;
  interface->shutdown = adapter_shutdown;
  interface->start = adapter_start;
  interface->stop = adapter_stop;
  interface->pause = adapter_pause;
  interface->resume = adapter_resume;
  interface->update = adapter_update;
  interface->set_period = adapter_set_period;
  interface->set_prescaler = adapter_set_prescaler;
  interface->set_compare = adapter_set_compare;
  interface->set_reload = adapter_set_reload;
  interface->set_mode = adapter_set_mode;
  interface->set_callback = adapter_set_callback;
  interface->get_state = adapter_get_state;
  interface->set_state = adapter_set_state;

  return interface;
}

void megadrive_timer_adapter_destroy(emu_timer_interface_t *timer) {
  if (!timer)
    return;

  if (timer->context) {
    adapter_shutdown(timer->context);
    free(timer->context);
  }

  free(timer);
}

megadrive_timer_context_t *
megadrive_timer_get_context(emu_timer_interface_t *timer) {
  if (!timer || !timer->context)
    return NULL;
  return (megadrive_timer_context_t *)timer->context;
}

int megadrive_timer_set_context(emu_timer_interface_t *timer,
                                const megadrive_timer_context_t *context) {
  if (!timer || !timer->context || !context)
    return -1;

  memcpy(timer->context, context, sizeof(megadrive_timer_context_t));
  return 0;
}

void megadrive_timer_configure(emu_timer_interface_t *timer,
                               md_timer_type_t type, uint32_t period,
                               void (*callback)(void *), void *user_data) {
  megadrive_timer_context_t *context = megadrive_timer_get_context(timer);
  if (!context || type >= MD_TIMER_COUNT)
    return;

  md_timer_state_t *t = &context->timers[type];
  t->period = period;
  t->reload = period; // Por padrão, recarrega com o mesmo período
  t->counter = 0;
  t->callback = callback;
  t->user_data = user_data;
  t->enabled = true;
  t->expired = false;
}

void megadrive_timer_enable(emu_timer_interface_t *timer, md_timer_type_t type,
                            bool enabled) {
  megadrive_timer_context_t *context = megadrive_timer_get_context(timer);
  if (!context || type >= MD_TIMER_COUNT)
    return;

  context->timers[type].enabled = enabled;
  if (enabled) {
    context->timers[type].counter = context->timers[type].reload;
    context->timers[type].expired = false;
  }
}

void megadrive_timer_set_prescaler(emu_timer_interface_t *timer,
                                   md_timer_type_t type, uint32_t prescaler) {
  megadrive_timer_context_t *context = megadrive_timer_get_context(timer);
  if (!context || type >= MD_TIMER_COUNT)
    return;

  context->timers[type].prescaler = prescaler;
}

int megadrive_timer_get_state(emu_timer_interface_t *timer,
                              md_timer_type_t type, md_timer_state_t *state) {
  megadrive_timer_context_t *context = megadrive_timer_get_context(timer);
  if (!context || !state || type >= MD_TIMER_COUNT)
    return -1;

  memcpy(state, &context->timers[type], sizeof(md_timer_state_t));
  return 0;
}
