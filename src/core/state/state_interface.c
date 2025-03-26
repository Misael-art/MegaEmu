#include "state_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Estrutura interna do contexto de save state
 */
typedef struct {
  state_config_t config;
  state_header_t current_header;
  state_metadata_t current_metadata;
  uint8_t *buffer;
  uint32_t buffer_size;
  uint32_t buffer_pos;
  bool is_saving;
} state_context_t;

// Funções auxiliares internas
static uint32_t calculate_checksum(const uint8_t *data, uint32_t size) {
  uint32_t checksum = 0;
  for (uint32_t i = 0; i < size; i++) {
    checksum = ((checksum << 5) + checksum) + data[i];
  }
  return checksum;
}

static void init_header(state_header_t *header, const char *system,
                        const char *rom_name) {
  memset(header, 0, sizeof(state_header_t));
  memcpy(header->magic, "SAVE", 4);
  header->version_major = STATE_VERSION_MAJOR;
  header->version_minor = STATE_VERSION_MINOR;
  strncpy(header->system, system, sizeof(header->system) - 1);
  strncpy(header->rom_name, rom_name, sizeof(header->rom_name) - 1);
  header->timestamp = (uint32_t)time(NULL);
}

// Implementação das funções da interface
static int32_t state_init(void *ctx, const state_config_t *config) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !config) {
    return STATE_ERROR_INVALID_FILE;
  }

  memcpy(&context->config, config, sizeof(state_config_t));
  context->buffer_size = 1024 * 1024; // 1MB inicial
  context->buffer = (uint8_t *)malloc(context->buffer_size);
  if (!context->buffer) {
    return STATE_ERROR_IO;
  }

  context->buffer_pos = 0;
  context->is_saving = false;
  return STATE_ERROR_NONE;
}

static void state_shutdown(void *ctx) {
  state_context_t *context = (state_context_t *)ctx;
  if (context) {
    free(context->buffer);
    context->buffer = NULL;
    context->buffer_size = 0;
  }
}

static bool state_save(void *ctx, const char *filename) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !filename) {
    return false;
  }

  FILE *file = fopen(filename, "wb");
  if (!file) {
    return false;
  }

  // Atualiza o checksum antes de salvar
  context->current_header.size = context->buffer_pos;
  context->current_header.checksum =
      calculate_checksum(context->buffer, context->buffer_pos);

  // Escreve o cabeçalho
  if (fwrite(&context->current_header, sizeof(state_header_t), 1, file) != 1) {
    fclose(file);
    return false;
  }

  // Escreve os metadados se necessário
  if (context->current_header.flags & STATE_FLAG_METADATA) {
    if (fwrite(&context->current_metadata, sizeof(state_metadata_t), 1, file) !=
        1) {
      fclose(file);
      return false;
    }
  }

  // Escreve o buffer de dados
  if (fwrite(context->buffer, 1, context->buffer_pos, file) !=
      context->buffer_pos) {
    fclose(file);
    return false;
  }

  fclose(file);
  return true;
}

static bool state_load(void *ctx, const char *filename) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !filename) {
    return false;
  }

  FILE *file = fopen(filename, "rb");
  if (!file) {
    return false;
  }

  // Lê o cabeçalho
  state_header_t header;
  if (fread(&header, sizeof(state_header_t), 1, file) != 1) {
    fclose(file);
    return false;
  }

  // Verifica o cabeçalho
  if (memcmp(header.magic, "SAVE", 4) != 0 ||
      header.version_major != STATE_VERSION_MAJOR) {
    fclose(file);
    return false;
  }

  // Lê os metadados se presentes
  if (header.flags & STATE_FLAG_METADATA) {
    if (fread(&context->current_metadata, sizeof(state_metadata_t), 1, file) !=
        1) {
      fclose(file);
      return false;
    }
  }

  // Redimensiona o buffer se necessário
  if (header.size > context->buffer_size) {
    uint8_t *new_buffer = (uint8_t *)realloc(context->buffer, header.size);
    if (!new_buffer) {
      fclose(file);
      return false;
    }
    context->buffer = new_buffer;
    context->buffer_size = header.size;
  }

  // Lê os dados
  if (fread(context->buffer, 1, header.size, file) != header.size) {
    fclose(file);
    return false;
  }

  // Verifica o checksum
  uint32_t checksum = calculate_checksum(context->buffer, header.size);
  if (checksum != header.checksum) {
    fclose(file);
    return false;
  }

  context->current_header = header;
  context->buffer_pos = header.size;
  fclose(file);
  return true;
}

static bool state_quick_save(void *ctx, int32_t slot) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || slot < 0 || slot >= 10) {
    return false;
  }

  char filename[512];
  snprintf(filename, sizeof(filename), "%s/quick%d.sav",
           context->config.save_dir, slot);
  return state_save(ctx, filename);
}

static bool state_quick_load(void *ctx, int32_t slot) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || slot < 0 || slot >= 10) {
    return false;
  }

  char filename[512];
  snprintf(filename, sizeof(filename), "%s/quick%d.sav",
           context->config.save_dir, slot);
  return state_load(ctx, filename);
}

// Funções de componentes
static bool state_save_component(void *ctx, const void *state, uint32_t size,
                                 uint32_t type_id) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !state || size == 0) {
    return false;
  }

  // Verifica se precisa redimensionar o buffer
  uint32_t required_size =
      context->buffer_pos + size + 8; // 8 bytes para tipo e tamanho
  if (required_size > context->buffer_size) {
    uint8_t *new_buffer =
        (uint8_t *)realloc(context->buffer, required_size * 2);
    if (!new_buffer) {
      return false;
    }
    context->buffer = new_buffer;
    context->buffer_size = required_size * 2;
  }

  // Escreve o tipo e tamanho
  memcpy(context->buffer + context->buffer_pos, &type_id, 4);
  context->buffer_pos += 4;
  memcpy(context->buffer + context->buffer_pos, &size, 4);
  context->buffer_pos += 4;

  // Escreve os dados
  memcpy(context->buffer + context->buffer_pos, state, size);
  context->buffer_pos += size;

  return true;
}

static bool state_load_component(void *ctx, void *state, uint32_t size,
                                 uint32_t type_id) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !state || size == 0 ||
      context->buffer_pos >= context->buffer_size) {
    return false;
  }

  uint32_t stored_type, stored_size;

  // Lê o tipo e tamanho
  memcpy(&stored_type, context->buffer + context->buffer_pos, 4);
  context->buffer_pos += 4;
  memcpy(&stored_size, context->buffer + context->buffer_pos, 4);
  context->buffer_pos += 4;

  // Verifica o tipo e tamanho
  if (stored_type != type_id || stored_size != size) {
    return false;
  }

  // Lê os dados
  memcpy(state, context->buffer + context->buffer_pos, size);
  context->buffer_pos += size;

  return true;
}

#define DEFINE_COMPONENT_SAVE(name, id)                                        \
  static bool state_save_##name##_state(void *ctx, const void *state,          \
                                        uint32_t size) {                       \
    return state_save_component(ctx, state, size, id);                         \
  }

#define DEFINE_COMPONENT_LOAD(name, id)                                        \
  static bool state_load_##name##_state(void *ctx, void *state,                \
                                        uint32_t size) {                       \
    return state_load_component(ctx, state, size, id);                         \
  }

// Define funções para cada componente
DEFINE_COMPONENT_SAVE(cpu, 1)
DEFINE_COMPONENT_SAVE(ppu, 2)
DEFINE_COMPONENT_SAVE(apu, 3)
DEFINE_COMPONENT_SAVE(memory, 4)
DEFINE_COMPONENT_SAVE(cart, 5)

DEFINE_COMPONENT_LOAD(cpu, 1)
DEFINE_COMPONENT_LOAD(ppu, 2)
DEFINE_COMPONENT_LOAD(apu, 3)
DEFINE_COMPONENT_LOAD(memory, 4)
DEFINE_COMPONENT_LOAD(cart, 5)

// Funções de metadados
static bool state_set_metadata(void *ctx, const state_metadata_t *metadata) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !metadata) {
    return false;
  }

  memcpy(&context->current_metadata, metadata, sizeof(state_metadata_t));
  context->current_header.flags |= STATE_FLAG_METADATA;
  return true;
}

static bool state_get_metadata(void *ctx, state_metadata_t *metadata) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !metadata ||
      !(context->current_header.flags & STATE_FLAG_METADATA)) {
    return false;
  }

  memcpy(metadata, &context->current_metadata, sizeof(state_metadata_t));
  return true;
}

// Funções de informação
static bool state_get_header(void *ctx, state_header_t *header) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !header) {
    return false;
  }

  memcpy(header, &context->current_header, sizeof(state_header_t));
  return true;
}

static bool state_verify(void *ctx, const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    return false;
  }

  state_header_t header;
  bool valid = (fread(&header, sizeof(state_header_t), 1, file) == 1) &&
               (memcmp(header.magic, "SAVE", 4) == 0) &&
               (header.version_major == STATE_VERSION_MAJOR);

  fclose(file);
  return valid;
}

static uint32_t state_get_size(void *ctx) {
  state_context_t *context = (state_context_t *)ctx;
  return context ? context->buffer_pos : 0;
}

static uint32_t state_get_num_slots(void *ctx) {
  return 10; // Número fixo de slots rápidos
}

// Funções de debug
static int32_t state_dump(void *ctx, char *buffer, int32_t buffer_size) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context || !buffer || buffer_size <= 0) {
    return -1;
  }

  int32_t written = snprintf(
      buffer, buffer_size,
      "State Dump:\n"
      "Header:\n"
      "  Magic: %.4s\n"
      "  Version: %d.%d\n"
      "  Flags: 0x%08X\n"
      "  Size: %d bytes\n"
      "  Checksum: 0x%08X\n"
      "  System: %s\n"
      "  ROM: %s\n"
      "  Timestamp: %u\n",
      context->current_header.magic, context->current_header.version_major,
      context->current_header.version_minor, context->current_header.flags,
      context->current_header.size, context->current_header.checksum,
      context->current_header.system, context->current_header.rom_name,
      context->current_header.timestamp);

  return written;
}

static bool state_compare(void *ctx, const char *filename1,
                          const char *filename2) {
  state_context_t *context = (state_context_t *)ctx;
  if (!context) {
    return false;
  }

  // Carrega o primeiro arquivo
  if (!state_load(ctx, filename1)) {
    return false;
  }

  state_header_t header1 = context->current_header;
  uint8_t *data1 = malloc(header1.size);
  if (!data1) {
    return false;
  }
  memcpy(data1, context->buffer, header1.size);

  // Carrega o segundo arquivo
  if (!state_load(ctx, filename2)) {
    free(data1);
    return false;
  }

  // Compara os dados
  bool equal = (header1.size == context->current_header.size) &&
               (memcmp(data1, context->buffer, header1.size) == 0);

  free(data1);
  return equal;
}

// Cria a interface
state_interface_t *create_state_interface(void) {
  state_interface_t *interface =
      (state_interface_t *)malloc(sizeof(state_interface_t));
  if (!interface) {
    return NULL;
  }

  // Inicializa o contexto
  state_context_t *context = (state_context_t *)malloc(sizeof(state_context_t));
  if (!context) {
    free(interface);
    return NULL;
  }

  memset(context, 0, sizeof(state_context_t));
  interface->context = context;

  // Configura as funções
  interface->init = state_init;
  interface->shutdown = state_shutdown;
  interface->save_state = state_save;
  interface->load_state = state_load;
  interface->quick_save = state_quick_save;
  interface->quick_load = state_quick_load;

  interface->save_cpu_state = state_save_cpu_state;
  interface->save_ppu_state = state_save_ppu_state;
  interface->save_apu_state = state_save_apu_state;
  interface->save_memory_state = state_save_memory_state;
  interface->save_cart_state = state_save_cart_state;

  interface->load_cpu_state = state_load_cpu_state;
  interface->load_ppu_state = state_load_ppu_state;
  interface->load_apu_state = state_load_apu_state;
  interface->load_memory_state = state_load_memory_state;
  interface->load_cart_state = state_load_cart_state;

  interface->set_metadata = state_set_metadata;
  interface->get_metadata = state_get_metadata;
  interface->get_header = state_get_header;
  interface->verify_state = state_verify;
  interface->get_state_size = state_get_size;
  interface->get_num_slots = state_get_num_slots;
  interface->dump_state = state_dump;
  interface->compare_states = state_compare;

  return interface;
}

void destroy_state_interface(state_interface_t *interface) {
  if (interface) {
    if (interface->context) {
      state_shutdown(interface->context);
      free(interface->context);
    }
    free(interface);
  }
}
