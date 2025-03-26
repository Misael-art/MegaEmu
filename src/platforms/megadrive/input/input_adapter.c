/**
 * @file input_adapter.c
 * @brief Implementação do adaptador de input para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "input_adapter.h"
#include <stdlib.h>
#include <string.h>

// Funções estáticas do adaptador
static int adapter_init(void *ctx, const emu_input_config_t *config) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context || !config)
    return -1;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));

  // Configura polling
  context->polling_enabled = true;
  context->poll_counter = 0;

  // Inicializa controles
  for (int i = 0; i < MD_MAX_CONTROLLERS; i++) {
    context->pads[i].type = MD_PAD_TYPE_3BTN; // Assume 3 botões por padrão
    context->pads[i].buttons = 0;
    context->pads[i].buttons_prev = 0;
    context->pads[i].counter = 0;
    context->pads[i].connected = true; // Assume conectado por padrão
  }

  return 0;
}

static void adapter_reset(void *ctx) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context)
    return;

  // Reseta contadores
  context->poll_counter = 0;

  // Reseta estado dos controles
  for (int i = 0; i < MD_MAX_CONTROLLERS; i++) {
    context->pads[i].buttons = 0;
    context->pads[i].buttons_prev = 0;
    context->pads[i].counter = 0;
  }
}

static void adapter_shutdown(void *ctx) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context)
    return;

  // Limpa todo o contexto
  memset(context, 0, sizeof(*context));
}

static void adapter_poll(void *ctx) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context || !context->polling_enabled)
    return;

  // Atualiza contador de polling
  context->poll_counter++;

  // Atualiza estado anterior dos botões
  for (int i = 0; i < MD_MAX_CONTROLLERS; i++) {
    context->pads[i].buttons_prev = context->pads[i].buttons;
  }

  // Atualiza contadores dos controles de 6 botões
  for (int i = 0; i < MD_MAX_CONTROLLERS; i++) {
    if (context->pads[i].type == MD_PAD_TYPE_6BTN) {
      context->pads[i].counter = (context->pads[i].counter + 1) % 5;
    }
  }
}

static void adapter_process_events(void *ctx) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context)
    return;

  // Processa eventos de controle
  // Neste caso, não há nada a fazer pois os eventos são processados em tempo
  // real
}

static bool adapter_is_button_pressed(void *ctx, uint8_t button) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context)
    return false;

  // Verifica apenas o primeiro controle por compatibilidade
  return (context->pads[0].buttons & (1 << button)) != 0;
}

static bool adapter_is_button_released(void *ctx, uint8_t button) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context)
    return false;

  // Verifica apenas o primeiro controle por compatibilidade
  return ((context->pads[0].buttons_prev & (1 << button)) != 0) &&
         ((context->pads[0].buttons & (1 << button)) == 0);
}

static bool adapter_is_button_held(void *ctx, uint8_t button) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context)
    return false;

  // Verifica apenas o primeiro controle por compatibilidade
  return ((context->pads[0].buttons_prev & (1 << button)) != 0) &&
         ((context->pads[0].buttons & (1 << button)) != 0);
}

static void adapter_get_state(void *ctx, emu_input_state_t *state) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context || !state)
    return;

  // Retorna estado do primeiro controle por compatibilidade
  state->buttons = context->pads[0].buttons;
  state->flags = EMU_INPUT_FLAG_CONFIGURED;
  if (context->pads[0].connected)
    state->flags |= EMU_INPUT_FLAG_CONNECTED;
  if (context->polling_enabled)
    state->flags |= EMU_INPUT_FLAG_ACTIVE;
  state->context = context;
}

static void adapter_set_state(void *ctx, const emu_input_state_t *state) {
  megadrive_input_context_t *context = (megadrive_input_context_t *)ctx;
  if (!context || !state)
    return;

  // Atualiza estado do primeiro controle por compatibilidade
  context->pads[0].buttons = state->buttons;
  context->pads[0].connected = (state->flags & EMU_INPUT_FLAG_CONNECTED) != 0;
  context->polling_enabled = (state->flags & EMU_INPUT_FLAG_ACTIVE) != 0;
}

// Funções públicas
emu_input_interface_t *megadrive_input_adapter_create(void) {
  emu_input_interface_t *interface = calloc(1, sizeof(emu_input_interface_t));
  megadrive_input_context_t *context =
      calloc(1, sizeof(megadrive_input_context_t));

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
  interface->poll = adapter_poll;
  interface->process_events = adapter_process_events;
  interface->is_button_pressed = adapter_is_button_pressed;
  interface->is_button_released = adapter_is_button_released;
  interface->is_button_held = adapter_is_button_held;
  interface->get_state = adapter_get_state;
  interface->set_state = adapter_set_state;

  return interface;
}

void megadrive_input_adapter_destroy(emu_input_interface_t *input) {
  if (!input)
    return;

  if (input->context) {
    adapter_shutdown(input->context);
    free(input->context);
  }

  free(input);
}

megadrive_input_context_t *
megadrive_input_get_context(emu_input_interface_t *input) {
  if (!input || !input->context)
    return NULL;
  return (megadrive_input_context_t *)input->context;
}

int megadrive_input_set_context(emu_input_interface_t *input,
                                const megadrive_input_context_t *context) {
  if (!input || !input->context || !context)
    return -1;

  memcpy(input->context, context, sizeof(megadrive_input_context_t));
  return 0;
}

void megadrive_input_set_pad_type(emu_input_interface_t *input, uint8_t port,
                                  md_pad_type_t type) {
  megadrive_input_context_t *context = megadrive_input_get_context(input);
  if (!context || port >= MD_MAX_CONTROLLERS)
    return;

  context->pads[port].type = type;
  context->pads[port].counter = 0;
}

void megadrive_input_set_button(emu_input_interface_t *input, uint8_t port,
                                uint8_t button, bool pressed) {
  megadrive_input_context_t *context = megadrive_input_get_context(input);
  if (!context || port >= MD_MAX_CONTROLLERS)
    return;

  // Verifica se o botão é válido para o tipo de controle
  if (context->pads[port].type == MD_PAD_TYPE_3BTN &&
      button >= MD_BUTTON_COUNT_3BTN)
    return;
  if (context->pads[port].type == MD_PAD_TYPE_6BTN &&
      button >= MD_BUTTON_COUNT_6BTN)
    return;

  // Atualiza estado do botão
  if (pressed) {
    context->pads[port].buttons |= (1 << button);
  } else {
    context->pads[port].buttons &= ~(1 << button);
  }
}

bool megadrive_input_get_button(emu_input_interface_t *input, uint8_t port,
                                uint8_t button) {
  megadrive_input_context_t *context = megadrive_input_get_context(input);
  if (!context || port >= MD_MAX_CONTROLLERS)
    return false;

  // Verifica se o botão é válido para o tipo de controle
  if (context->pads[port].type == MD_PAD_TYPE_3BTN &&
      button >= MD_BUTTON_COUNT_3BTN)
    return false;
  if (context->pads[port].type == MD_PAD_TYPE_6BTN &&
      button >= MD_BUTTON_COUNT_6BTN)
    return false;

  return (context->pads[port].buttons & (1 << button)) != 0;
}

uint16_t megadrive_input_get_pad_state(emu_input_interface_t *input,
                                       uint8_t port) {
  megadrive_input_context_t *context = megadrive_input_get_context(input);
  if (!context || port >= MD_MAX_CONTROLLERS)
    return 0;

  return context->pads[port].buttons;
}
