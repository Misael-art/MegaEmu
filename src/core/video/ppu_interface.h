/**
 * @file ppu_interface.h
 * @brief Interface genérica para PPUs
 */
#ifndef PPU_INTERFACE_H
#define PPU_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Códigos de erro genéricos para PPUs
 */
#define PPU_ERROR_NONE 0            /**< Sem erro */
#define PPU_ERROR_INVALID_REG -10   /**< Registro inválido */
#define PPU_ERROR_INVALID_ADDR -11  /**< Endereço inválido */
#define PPU_ERROR_INVALID_STATE -12 /**< Estado inválido */

/**
 * @brief Flags de estado genéricas da PPU
 */
#define PPU_FLAG_VBLANK 0x01      /**< PPU em VBlank */
#define PPU_FLAG_SPRITE0_HIT 0x02 /**< Colisão com Sprite 0 */
#define PPU_FLAG_RENDERING 0x04   /**< PPU renderizando */
#define PPU_FLAG_NMI_ENABLED 0x08 /**< NMI habilitado */

/**
 * @brief Funções de callback para acesso à memória
 */
typedef uint8_t (*ppu_read_func_t)(void *context, uint32_t address);
typedef void (*ppu_write_func_t)(void *context, uint32_t address,
                                 uint8_t value);

/**
 * @brief Configuração genérica de PPU
 */
typedef struct {
  ppu_read_func_t read_mem;   /**< Função para leitura de memória */
  ppu_write_func_t write_mem; /**< Função para escrita de memória */
  void *context;              /**< Contexto para callbacks de memória */
  int32_t log_level;          /**< Nível de log para a PPU */
  uint32_t screen_width;      /**< Largura da tela */
  uint32_t screen_height;     /**< Altura da tela */
} ppu_config_t;

/**
 * @brief Estado genérico da PPU
 */
typedef struct {
  uint32_t scanline; /**< Scanline atual */
  uint32_t cycle;    /**< Ciclo atual na scanline */
  uint32_t frame;    /**< Frame atual */
  uint8_t flags;     /**< Flags de estado */
} ppu_state_t;

/**
 * @brief Interface genérica de PPU
 */
typedef struct {
  void *context; /**< Contexto específico da PPU */

  // Funções de controle
  int32_t (*init)(void *ctx, const ppu_config_t *config);
  void (*shutdown)(void *ctx);
  void (*reset)(void *ctx);
  int32_t (*execute)(void *ctx, int32_t cycles);

  // Funções de estado
  void (*get_state)(void *ctx, ppu_state_t *state);
  void (*set_state)(void *ctx, const ppu_state_t *state);

  // Funções de registro
  uint8_t (*read_register)(void *ctx, uint32_t reg);
  void (*write_register)(void *ctx, uint32_t reg, uint8_t value);

  // Funções de memória de vídeo
  uint8_t (*read_vram)(void *ctx, uint32_t addr);
  void (*write_vram)(void *ctx, uint32_t addr, uint8_t value);

  // Funções de paleta
  uint8_t (*read_palette)(void *ctx, uint32_t addr);
  void (*write_palette)(void *ctx, uint32_t addr, uint8_t value);

  // Funções de sprite
  uint8_t (*read_oam)(void *ctx, uint32_t addr);
  void (*write_oam)(void *ctx, uint32_t addr, uint8_t value);
  void (*dma_write)(void *ctx, const uint8_t *data);

  // Funções de frame
  void (*end_frame)(void *ctx);
  const uint32_t *(*get_frame_buffer)(void *ctx);

  // Funções de debug
  int32_t (*dump_state)(void *ctx, char *buffer, int32_t buffer_size);
} ppu_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* PPU_INTERFACE_H */
