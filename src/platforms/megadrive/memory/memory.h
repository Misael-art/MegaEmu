/**
 * @file memory.h
 * @brief Interface do sistema de memória do Mega Drive
 */

#ifndef MD_MEMORY_H
#define MD_MEMORY_H

#include "../../../utils/common_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Estrutura que representa o estado do sistema de memória do Mega Drive
 */
typedef struct {
  uint8_t *rom;            /**< Memória ROM */
  uint32_t rom_size;       /**< Tamanho da ROM em bytes */
  uint8_t ram[0x10000];    /**< Memória RAM (64KB) */
  uint8_t vram[0x10000];   /**< Memória de vídeo (64KB) */
  uint8_t cram[0x80];      /**< Memória de cores (128 bytes) */
  uint8_t vsram[0x80];     /**< Memória de scroll vertical (128 bytes) */
  uint8_t z80_ram[0x2000]; /**< Memória RAM do Z80 (8KB) */
  bool cart_inserted;      /**< Indica se há cartucho inserido */
} md_memory_t;

/**
 * @brief Inicializa o sistema de memória
 * @param memory Ponteiro para a estrutura de memória
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_init(md_memory_t *memory);

/**
 * @brief Finaliza o sistema de memória e libera recursos
 * @param memory Ponteiro para a estrutura de memória
 */
void md_memory_shutdown(md_memory_t *memory);

/**
 * @brief Reseta o sistema de memória
 * @param memory Ponteiro para a estrutura de memória
 */
void md_memory_reset(md_memory_t *memory);

/**
 * @brief Carrega uma ROM no sistema
 * @param memory Ponteiro para a estrutura de memória
 * @param data Ponteiro para os dados da ROM
 * @param size Tamanho da ROM em bytes
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_load_rom(md_memory_t *memory, const uint8_t *data,
                               uint32_t size);

/**
 * @brief Remove a ROM do sistema
 * @param memory Ponteiro para a estrutura de memória
 */
void md_memory_unload_rom(md_memory_t *memory);

/**
 * @brief Lê um byte da memória
 * @param memory Ponteiro para a estrutura de memória
 * @param address Endereço a ser lido
 * @param value Ponteiro para armazenar o valor lido
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_read_u8(md_memory_t *memory, uint32_t address,
                              uint8_t *value);

/**
 * @brief Lê uma word (16 bits) da memória
 * @param memory Ponteiro para a estrutura de memória
 * @param address Endereço a ser lido
 * @param value Ponteiro para armazenar o valor lido
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_read_u16(md_memory_t *memory, uint32_t address,
                               uint16_t *value);

/**
 * @brief Lê uma long word (32 bits) da memória
 * @param memory Ponteiro para a estrutura de memória
 * @param address Endereço a ser lido
 * @param value Ponteiro para armazenar o valor lido
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_read_u32(md_memory_t *memory, uint32_t address,
                               uint32_t *value);

/**
 * @brief Escreve um byte na memória
 * @param memory Ponteiro para a estrutura de memória
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_write_u8(md_memory_t *memory, uint32_t address,
                               uint8_t value);

/**
 * @brief Escreve uma word (16 bits) na memória
 * @param memory Ponteiro para a estrutura de memória
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_write_u16(md_memory_t *memory, uint32_t address,
                                uint16_t value);

/**
 * @brief Escreve uma long word (32 bits) na memória
 * @param memory Ponteiro para a estrutura de memória
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 * @return EMU_ERROR_NONE em caso de sucesso, outro código de erro caso
 * contrário
 */
emu_error_t md_memory_write_u32(md_memory_t *memory, uint32_t address,
                                uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* MD_MEMORY_H */
