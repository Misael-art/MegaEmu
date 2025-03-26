#ifndef CARTRIDGE_INTERFACE_H
#define CARTRIDGE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Códigos de erro genéricos para cartuchos
 */
#define CARTRIDGE_ERROR_NONE 0             /**< Sem erro */
#define CARTRIDGE_ERROR_INVALID_ROM -10    /**< ROM inválida */
#define CARTRIDGE_ERROR_INVALID_MAPPER -11 /**< Mapper inválido */
#define CARTRIDGE_ERROR_NO_BATTERY -12     /**< Sem bateria para save */
#define CARTRIDGE_ERROR_SAVE_FAILED -13    /**< Falha ao salvar */

/**
 * @brief Flags de cartuchos
 */
#define CARTRIDGE_FLAG_BATTERY 0x01      /**< Cartucho com bateria */
#define CARTRIDGE_FLAG_TRAINER 0x02      /**< Cartucho com trainer */
#define CARTRIDGE_FLAG_FOUR_SCREEN 0x04  /**< Modo four-screen */
#define CARTRIDGE_FLAG_VS_UNISYSTEM 0x08 /**< VS Unisystem */

/**
 * @brief Informações do cabeçalho do cartucho
 */
typedef struct {
  uint32_t prg_rom_size;  /**< Tamanho da ROM de programa */
  uint32_t chr_rom_size;  /**< Tamanho da ROM de caracteres */
  uint32_t prg_ram_size;  /**< Tamanho da RAM de programa */
  uint32_t chr_ram_size;  /**< Tamanho da RAM de caracteres */
  uint16_t mapper_number; /**< Número do mapper */
  uint8_t flags;          /**< Flags do cartucho */
  char system_type[32];   /**< Tipo de sistema (NES, SNES, etc) */
  char game_title[32];    /**< Título do jogo */
  uint32_t crc32;         /**< CRC32 da ROM */
} cartridge_header_t;

/**
 * @brief Configuração do sistema de cartuchos
 */
typedef struct {
  const char *rom_file; /**< Caminho do arquivo ROM */
  const char *save_dir; /**< Diretório para saves */
  bool enable_patches;  /**< Habilitar patches de ROM */
  int32_t log_level;    /**< Nível de log */
} cartridge_config_t;

/**
 * @brief Estado do sistema de cartuchos
 */
typedef struct {
  bool has_save;         /**< Possui save */
  bool is_modified;      /**< Save foi modificado */
  uint32_t mapper_state; /**< Estado do mapper */
  uint8_t flags;         /**< Flags de estado */
} cartridge_state_t;

/**
 * @brief Interface genérica de cartuchos
 */
typedef struct {
  void *context; /**< Contexto específico do cartucho */

  // Funções de controle
  int32_t (*init)(void *ctx, const cartridge_config_t *config);
  void (*shutdown)(void *ctx);
  void (*reset)(void *ctx);

  // Funções de acesso à ROM/RAM
  uint8_t (*read_prg)(void *ctx, uint32_t address);
  void (*write_prg)(void *ctx, uint32_t address, uint8_t value);
  uint8_t (*read_chr)(void *ctx, uint32_t address);
  void (*write_chr)(void *ctx, uint32_t address, uint8_t value);

  // Funções de mapper
  void (*set_prg_bank)(void *ctx, uint32_t bank, uint32_t value);
  void (*set_chr_bank)(void *ctx, uint32_t bank, uint32_t value);
  uint32_t (*get_prg_bank)(void *ctx, uint32_t bank);
  uint32_t (*get_chr_bank)(void *ctx, uint32_t bank);

  // Funções de save
  bool (*save_ram)(void *ctx, const char *filename);
  bool (*load_ram)(void *ctx, const char *filename);
  bool (*has_battery)(void *ctx);
  bool (*is_ram_modified)(void *ctx);

  // Funções de informação
  void (*get_header)(void *ctx, cartridge_header_t *header);
  const char *(*get_mapper_name)(void *ctx);
  uint32_t (*get_mapper_id)(void *ctx);

  // Funções de estado
  void (*get_state)(void *ctx, cartridge_state_t *state);
  void (*set_state)(void *ctx, const cartridge_state_t *state);

  // Funções de debug
  int32_t (*dump_state)(void *ctx, char *buffer, int32_t buffer_size);
  void (*set_breakpoint)(void *ctx, uint32_t address, bool enabled);
  bool (*check_breakpoint)(void *ctx, uint32_t address);
} cartridge_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* CARTRIDGE_INTERFACE_H */
