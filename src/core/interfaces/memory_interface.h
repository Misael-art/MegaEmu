/**
 * @file memory_interface.h
 * @brief Interface simplificada do sistema de memória
 */

#ifndef EMU_MEMORY_INTERFACE_H
#define EMU_MEMORY_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../utils/error_handling.h"
#include "../core_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Número máximo de regiões de memória */
#define EMU_MAX_MEMORY_REGIONS 16

/* Flags para regiões de memória */
typedef enum {
  EMU_MEMORY_READ = 0x01,    /**< Região pode ser lida */
  EMU_MEMORY_WRITE = 0x02,   /**< Região pode ser escrita */
  EMU_MEMORY_EXECUTE = 0x04, /**< Região pode ser executada */
  EMU_MEMORY_ROM = EMU_MEMORY_READ | EMU_MEMORY_EXECUTE,
  EMU_MEMORY_RAM = EMU_MEMORY_READ | EMU_MEMORY_WRITE | EMU_MEMORY_EXECUTE,
  EMU_MEMORY_OWNED = 0x20 /**< Memória é gerenciada pelo sistema */
} emu_memory_flags_t;

/* Forward declarations */
typedef struct memory_region_s memory_region_t;
typedef struct memory_callbacks_s memory_callbacks_t;
// Comentado para usar a definição de core_types.h
// typedef struct emu_memory_instance_s emu_memory_instance_t;
typedef emu_memory_instance_t *emu_memory_t;

/* Callbacks para operações de memória */
struct memory_callbacks_s {
  uint8_t (*read_8)(memory_region_t *region, uint32_t address);
  uint16_t (*read_16)(memory_region_t *region, uint32_t address);
  uint32_t (*read_32)(memory_region_t *region, uint32_t address);
  void (*write_8)(memory_region_t *region, uint32_t address, uint8_t value);
  void (*write_16)(memory_region_t *region, uint32_t address, uint16_t value);
  void (*write_32)(memory_region_t *region, uint32_t address, uint32_t value);
};

/* Estrutura de região de memória */
struct memory_region_s {
  uint32_t start;               /**< Endereço inicial */
  uint32_t size;                /**< Tamanho em bytes */
  uint8_t *data;                /**< Ponteiro para os dados */
  emu_memory_flags_t flags;     /**< Flags de acesso */
  memory_callbacks_t callbacks; /**< Callbacks de acesso */
  void *user_data;              /**< Dados do usuário */
};

/* Estrutura principal de memória */
struct emu_memory_instance {
  memory_region_t regions[EMU_MAX_MEMORY_REGIONS]; /**< Regiões de memória */
  int32_t num_regions;                             /**< Número de regiões */
  bool initialized;                                /**< Flag de inicialização */
  void *user_data;                                 /**< Dados do usuário */
};

/* Funções de interface */
emu_memory_t emu_memory_create(void);
void emu_memory_destroy(emu_memory_t memory);
bool emu_memory_init(emu_memory_t memory);
void emu_memory_shutdown(emu_memory_t memory);
bool emu_memory_reset(emu_memory_t memory);

/* Funções de gerenciamento de regiões */
bool emu_memory_add_region(emu_memory_t memory, uint32_t start, uint32_t size,
                           uint8_t *data, emu_memory_flags_t flags,
                           memory_callbacks_t *callbacks);
bool emu_memory_remove_region(emu_memory_t memory, uint32_t start);

/* Funções de acesso à memória */
uint8_t emu_memory_read_8(emu_memory_t memory, uint32_t address);
uint16_t emu_memory_read_16(emu_memory_t memory, uint32_t address);
uint32_t emu_memory_read_32(emu_memory_t memory, uint32_t address);
void emu_memory_write_8(emu_memory_t memory, uint32_t address, uint8_t value);
void emu_memory_write_16(emu_memory_t memory, uint32_t address, uint16_t value);
void emu_memory_write_32(emu_memory_t memory, uint32_t address, uint32_t value);

/* Funções de utilitário */
bool emu_memory_dump(emu_memory_t memory, uint32_t start_address, uint32_t size,
                     uint8_t *buffer);
bool emu_memory_load(emu_memory_t memory, uint32_t start_address, uint32_t size,
                     const uint8_t *buffer);

/* Funções de dados do usuário */
void emu_memory_set_user_data(emu_memory_t memory, void *user_data);
void *emu_memory_get_user_data(emu_memory_t memory);

#ifdef __cplusplus
}
#endif

#endif /* EMU_MEMORY_INTERFACE_H */
