/**
 * @file memory_interface.h
 * @brief Interface simplificada do sistema de memória
 */
#ifndef MEMORY_INTERFACE_H
#define MEMORY_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Códigos de erro genéricos para memória
 */
#define MEMORY_ERROR_NONE 0           /**< Sem erro */
#define MEMORY_ERROR_INVALID_ADDR -10 /**< Endereço inválido */
#define MEMORY_ERROR_READ_ONLY                                                 \
  -11 /**< Tentativa de escrita em memória somente leitura */
#define MEMORY_ERROR_NO_MAPPER -12 /**< Mapper não configurado */

/**
 * @brief Flags de memória
 */
#define MEMORY_FLAG_READ_ONLY 0x01 /**< Memória somente leitura */
#define MEMORY_FLAG_MIRRORED 0x02  /**< Memória espelhada */
#define MEMORY_FLAG_BANKED 0x04    /**< Memória bankeada */
#define MEMORY_FLAG_MAPPED 0x08    /**< Memória mapeada */

/**
 * @brief Funções de callback para mapeamento de memória
 */
typedef uint8_t (*memory_read_func_t)(void *context, uint32_t address);
typedef void (*memory_write_func_t)(void *context, uint32_t address,
                                    uint8_t value);

/**
 * @brief Configuração de região de memória
 */
typedef struct {
  uint32_t start_address;    /**< Endereço inicial */
  uint32_t size;             /**< Tamanho em bytes */
  uint8_t flags;             /**< Flags da região */
  memory_read_func_t read;   /**< Função de leitura */
  memory_write_func_t write; /**< Função de escrita */
  void *context;             /**< Contexto para callbacks */
} memory_region_config_t;

/**
 * @brief Configuração do sistema de memória
 */
typedef struct {
  memory_region_config_t *regions; /**< Array de configurações de regiões */
  uint32_t num_regions;            /**< Número de regiões */
  int32_t log_level;               /**< Nível de log */
} memory_config_t;

/**
 * @brief Estado do sistema de memória
 */
typedef struct {
  uint32_t current_bank; /**< Banco atual (para memória bankeada) */
  uint32_t num_banks;    /**< Número total de bancos */
  uint8_t flags;         /**< Flags de estado */
} memory_state_t;

/**
 * @brief Interface genérica de memória
 */
typedef struct {
  void *context; /**< Contexto específico da memória */

  // Funções de controle
  int32_t (*init)(void *ctx, const memory_config_t *config);
  void (*shutdown)(void *ctx);
  void (*reset)(void *ctx);

  // Funções de acesso
  uint8_t (*read)(void *ctx, uint32_t address);
  void (*write)(void *ctx, uint32_t address, uint8_t value);
  uint16_t (*read16)(void *ctx, uint32_t address);
  void (*write16)(void *ctx, uint32_t address, uint16_t value);
  uint32_t (*read32)(void *ctx, uint32_t address);
  void (*write32)(void *ctx, uint32_t address, uint32_t value);

  // Funções de DMA
  void (*dma_read)(void *ctx, uint32_t address, uint8_t *buffer, uint32_t size);
  void (*dma_write)(void *ctx, uint32_t address, const uint8_t *buffer,
                    uint32_t size);

  // Funções de banco de memória
  void (*set_bank)(void *ctx, uint32_t bank);
  uint32_t (*get_bank)(void *ctx);

  // Funções de estado
  void (*get_state)(void *ctx, memory_state_t *state);
  void (*set_state)(void *ctx, const memory_state_t *state);

  // Funções de debug
  int32_t (*dump_state)(void *ctx, char *buffer, int32_t buffer_size);
  void (*set_breakpoint)(void *ctx, uint32_t address, bool enabled);
  bool (*check_breakpoint)(void *ctx, uint32_t address);
} memory_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_INTERFACE_H */
