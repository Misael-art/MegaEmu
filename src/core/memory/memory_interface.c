/**
 * @file memory_interface.c
 * @brief Implementação da interface do sistema de memória
 */
#include "memory_interface.h"
#include "../../utils/log_utils.h"
#include "../../utils/validation_utils.h"
#include <stdlib.h>
#include <string.h>

/* Funções auxiliares internas */
static memory_region_t *find_region(emu_memory_t memory, uint32_t address);
static bool validate_address_alignment(uint32_t address, uint8_t size);

/**
 * @brief Cria uma nova instância do sistema de memória
 */
emu_memory_t emu_memory_create(void) {
  emu_memory_t memory = (emu_memory_t)calloc(1, sizeof(emu_memory_instance_t));
  if (!memory) {
    LOG_ERROR("Falha ao alocar memória para instância do sistema de memória");
    return NULL;
  }
  return memory;
}

/**
 * @brief Inicializa o sistema de memória
 */
bool emu_memory_init(emu_memory_t memory) {
  CHECK_NULL_RETURN_FALSE(memory, "Ponteiro de memória nulo");
  memory->num_regions = 0;
  memory->initialized = true;
  return true;
}

/**
 * @brief Adiciona uma nova região de memória
 */
bool emu_memory_add_region(emu_memory_t memory, uint32_t start, uint32_t size,
                           uint8_t *data, emu_memory_flags_t flags,
                           memory_callbacks_t *callbacks) {
  CHECK_NULL_RETURN_FALSE(memory, "Ponteiro de memória nulo");
  VALIDATE_PARAM_RETURN_FALSE(size > 0, "Tamanho de região inválido");
  VALIDATE_PARAM_RETURN_FALSE(memory->num_regions < EMU_MAX_MEMORY_REGIONS,
                              "Número máximo de regiões atingido");

  // Verificar sobreposição com regiões existentes
  for (int i = 0; i < memory->num_regions; i++) {
    memory_region_t *region = &memory->regions[i];
    if ((start >= region->start && start < region->start + region->size) ||
        (start + size > region->start &&
         start + size <= region->start + region->size)) {
      LOG_ERROR("Sobreposição detectada com região existente");
      return false;
    }
  }

  // Adicionar nova região
  memory_region_t *new_region = &memory->regions[memory->num_regions];
  new_region->start = start;
  new_region->size = size;
  new_region->data = data;
  new_region->flags = flags;

  // Copiar callbacks se fornecidos
  if (callbacks) {
    memcpy(&new_region->callbacks, callbacks, sizeof(memory_callbacks_t));
  } else {
    memset(&new_region->callbacks, 0, sizeof(memory_callbacks_t));
  }

  memory->num_regions++;
  return true;
}

/**
 * @brief Remove uma região de memória
 */
bool emu_memory_remove_region(emu_memory_t memory, uint32_t start) {
  CHECK_NULL_RETURN_FALSE(memory, "Ponteiro de memória nulo");

  for (int i = 0; i < memory->num_regions; i++) {
    if (memory->regions[i].start == start) {
      // Se a região possui flag OWNED, liberar memória
      if (memory->regions[i].flags & EMU_MEMORY_OWNED) {
        free(memory->regions[i].data);
      }

      // Mover regiões restantes
      if (i < memory->num_regions - 1) {
        memmove(&memory->regions[i], &memory->regions[i + 1],
                (memory->num_regions - i - 1) * sizeof(memory_region_t));
      }

      memory->num_regions--;
      return true;
    }
  }

  return false;
}

/**
 * @brief Lê um byte da memória
 */
uint8_t emu_memory_read_8(emu_memory_t memory, uint32_t address) {
  CHECK_NULL_RETURN_ZERO(memory, "Ponteiro de memória nulo");

  memory_region_t *region = find_region(memory, address);
  if (!region) {
    LOG_WARNING("Tentativa de leitura em endereço não mapeado: 0x%08X",
                address);
    return 0xFF;
  }

  if (!(region->flags & EMU_MEMORY_READ)) {
    LOG_WARNING("Tentativa de leitura em região somente escrita: 0x%08X",
                address);
    return 0xFF;
  }

  uint32_t offset = address - region->start;
  if (region->callbacks.read_8) {
    return region->callbacks.read_8(region, address);
  }

  return region->data[offset];
}

/**
 * @brief Lê uma word da memória
 */
uint16_t emu_memory_read_16(emu_memory_t memory, uint32_t address) {
  CHECK_NULL_RETURN_ZERO(memory, "Ponteiro de memória nulo");

  if (!validate_address_alignment(address, 2)) {
    LOG_WARNING("Acesso desalinhado de 16 bits: 0x%08X", address);
    return 0xFFFF;
  }

  memory_region_t *region = find_region(memory, address);
  if (!region || !(region->flags & EMU_MEMORY_READ)) {
    return 0xFFFF;
  }

  uint32_t offset = address - region->start;
  if (region->callbacks.read_16) {
    return region->callbacks.read_16(region, address);
  }

  return (region->data[offset] << 8) | region->data[offset + 1];
}

/**
 * @brief Lê uma long word da memória
 */
uint32_t emu_memory_read_32(emu_memory_t memory, uint32_t address) {
  CHECK_NULL_RETURN_ZERO(memory, "Ponteiro de memória nulo");

  if (!validate_address_alignment(address, 4)) {
    LOG_WARNING("Acesso desalinhado de 32 bits: 0x%08X", address);
    return 0xFFFFFFFF;
  }

  memory_region_t *region = find_region(memory, address);
  if (!region || !(region->flags & EMU_MEMORY_READ)) {
    return 0xFFFFFFFF;
  }

  uint32_t offset = address - region->start;
  if (region->callbacks.read_32) {
    return region->callbacks.read_32(region, address);
  }

  return (region->data[offset] << 24) | (region->data[offset + 1] << 16) |
         (region->data[offset + 2] << 8) | region->data[offset + 3];
}

/**
 * @brief Escreve um byte na memória
 */
void emu_memory_write_8(emu_memory_t memory, uint32_t address, uint8_t value) {
  CHECK_NULL_RETURN_VOID(memory, "Ponteiro de memória nulo");

  memory_region_t *region = find_region(memory, address);
  if (!region) {
    LOG_WARNING("Tentativa de escrita em endereço não mapeado: 0x%08X",
                address);
    return;
  }

  if (!(region->flags & EMU_MEMORY_WRITE)) {
    LOG_WARNING("Tentativa de escrita em região somente leitura: 0x%08X",
                address);
    return;
  }

  uint32_t offset = address - region->start;
  if (region->callbacks.write_8) {
    region->callbacks.write_8(region, address, value);
    return;
  }

  region->data[offset] = value;
}

/**
 * @brief Escreve uma word na memória
 */
void emu_memory_write_16(emu_memory_t memory, uint32_t address,
                         uint16_t value) {
  CHECK_NULL_RETURN_VOID(memory, "Ponteiro de memória nulo");

  if (!validate_address_alignment(address, 2)) {
    LOG_WARNING("Acesso desalinhado de 16 bits: 0x%08X", address);
    return;
  }

  memory_region_t *region = find_region(memory, address);
  if (!region || !(region->flags & EMU_MEMORY_WRITE)) {
    return;
  }

  uint32_t offset = address - region->start;
  if (region->callbacks.write_16) {
    region->callbacks.write_16(region, address, value);
    return;
  }

  region->data[offset] = (value >> 8) & 0xFF;
  region->data[offset + 1] = value & 0xFF;
}

/**
 * @brief Escreve uma long word na memória
 */
void emu_memory_write_32(emu_memory_t memory, uint32_t address,
                         uint32_t value) {
  CHECK_NULL_RETURN_VOID(memory, "Ponteiro de memória nulo");

  if (!validate_address_alignment(address, 4)) {
    LOG_WARNING("Acesso desalinhado de 32 bits: 0x%08X", address);
    return;
  }

  memory_region_t *region = find_region(memory, address);
  if (!region || !(region->flags & EMU_MEMORY_WRITE)) {
    return;
  }

  uint32_t offset = address - region->start;
  if (region->callbacks.write_32) {
    region->callbacks.write_32(region, address, value);
    return;
  }

  region->data[offset] = (value >> 24) & 0xFF;
  region->data[offset + 1] = (value >> 16) & 0xFF;
  region->data[offset + 2] = (value >> 8) & 0xFF;
  region->data[offset + 3] = value & 0xFF;
}

/**
 * @brief Reseta o sistema de memória
 */
bool emu_memory_reset(emu_memory_t memory) {
  CHECK_NULL_RETURN_FALSE(memory, "Ponteiro de memória nulo");

  // Limpar todas as regiões que não são ROM
  for (int i = 0; i < memory->num_regions; i++) {
    memory_region_t *region = &memory->regions[i];
    if ((region->flags & EMU_MEMORY_WRITE) && region->data) {
      memset(region->data, 0, region->size);
    }
  }

  return true;
}

/**
 * @brief Desliga o sistema de memória
 */
void emu_memory_shutdown(emu_memory_t memory) {
  if (!memory)
    return;

  // Liberar memória de regiões com flag OWNED
  for (int i = 0; i < memory->num_regions; i++) {
    if (memory->regions[i].flags & EMU_MEMORY_OWNED) {
      free(memory->regions[i].data);
    }
  }

  memory->num_regions = 0;
  memory->initialized = false;
}

/**
 * @brief Destrói uma instância do sistema de memória
 */
void emu_memory_destroy(emu_memory_t memory) {
  if (!memory)
    return;

  emu_memory_shutdown(memory);
  free(memory);
}

/**
 * @brief Faz dump de uma região de memória
 */
bool emu_memory_dump(emu_memory_t memory, uint32_t start_address, uint32_t size,
                     uint8_t *buffer) {
  CHECK_NULL_RETURN_FALSE(memory, "Ponteiro de memória nulo");
  CHECK_NULL_RETURN_FALSE(buffer, "Buffer de destino nulo");
  VALIDATE_PARAM_RETURN_FALSE(size > 0, "Tamanho inválido para dump");

  memory_region_t *region = find_region(memory, start_address);
  if (!region || !(region->flags & EMU_MEMORY_READ)) {
    return false;
  }

  uint32_t offset = start_address - region->start;
  if (offset + size > region->size) {
    size = region->size - offset;
  }

  memcpy(buffer, region->data + offset, size);
  return true;
}

/**
 * @brief Carrega dados em uma região de memória
 */
bool emu_memory_load(emu_memory_t memory, uint32_t start_address, uint32_t size,
                     const uint8_t *buffer) {
  CHECK_NULL_RETURN_FALSE(memory, "Ponteiro de memória nulo");
  CHECK_NULL_RETURN_FALSE(buffer, "Buffer de origem nulo");
  VALIDATE_PARAM_RETURN_FALSE(size > 0, "Tamanho inválido para load");

  memory_region_t *region = find_region(memory, start_address);
  if (!region || !(region->flags & EMU_MEMORY_WRITE)) {
    return false;
  }

  uint32_t offset = start_address - region->start;
  if (offset + size > region->size) {
    size = region->size - offset;
  }

  memcpy(region->data + offset, buffer, size);
  return true;
}

/* Funções auxiliares internas */

/**
 * @brief Encontra a região de memória que contém um endereço
 */
static memory_region_t *find_region(emu_memory_t memory, uint32_t address) {
  for (int i = 0; i < memory->num_regions; i++) {
    memory_region_t *region = &memory->regions[i];
    if (address >= region->start && address < region->start + region->size) {
      return region;
    }
  }
  return NULL;
}

/**
 * @brief Valida o alinhamento de um endereço
 */
static bool validate_address_alignment(uint32_t address, uint8_t size) {
  return (address % size) == 0;
}
