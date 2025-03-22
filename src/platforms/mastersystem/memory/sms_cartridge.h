/**
 * @file sms_cartridge.h
 * @brief Definições para o sistema de cartucho do Master System
 */

#ifndef SMS_CARTRIDGE_H
#define SMS_CARTRIDGE_H

#include <stdint.h>
#include "../../../core/save_state.h"

/**
 * @brief Tamanhos máximos de ROM e RAM
 */
#define SMS_MAX_ROM_SIZE (512 * 1024)  // 512 KB
#define SMS_MAX_RAM_SIZE (32 * 1024)   // 32 KB

/**
 * @brief Informações sobre a ROM carregada
 */
typedef struct {
    char title[33];           // Título do jogo (terminado em nulo)
    uint32_t size;            // Tamanho da ROM em bytes
    uint32_t checksum;        // Checksum CRC32 da ROM
    uint8_t region;           // Região (0=Japão, 1=EUA/Europa)
    uint8_t has_battery;      // Indica se tem bateria para salvar
    uint8_t mapper_type;      // Tipo de mapper usado
    uint8_t *rom_data;        // Ponteiro para os dados da ROM
} sms_rom_info_t;

/**
 * @brief Opaque handle para o cartucho
 */
typedef struct sms_cartridge_t sms_cartridge_t;

/**
 * @brief Cria uma nova instância do cartucho
 * 
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_cartridge_t* sms_cartridge_create(void);

/**
 * @brief Destrói uma instância do cartucho e libera recursos
 * 
 * @param cartridge Ponteiro para a instância
 */
void sms_cartridge_destroy(sms_cartridge_t *cartridge);

/**
 * @brief Reseta o cartucho para o estado inicial
 * 
 * @param cartridge Ponteiro para a instância
 */
void sms_cartridge_reset(sms_cartridge_t *cartridge);

/**
 * @brief Carrega uma ROM no cartucho
 * 
 * @param cartridge Ponteiro para a instância
 * @param rom_path Caminho para o arquivo ROM
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_load_rom(sms_cartridge_t *cartridge, const char *rom_path);

/**
 * @brief Obtém informações sobre a ROM carregada
 * 
 * @param cartridge Ponteiro para a instância
 * @param info Ponteiro para a estrutura que receberá as informações
 */
void sms_cartridge_get_info(sms_cartridge_t *cartridge, sms_rom_info_t *info);

/**
 * @brief Lê um byte da ROM/RAM do cartucho
 * 
 * @param cartridge Ponteiro para a instância
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint8_t sms_cartridge_read(sms_cartridge_t *cartridge, uint16_t address);

/**
 * @brief Escreve um byte na RAM do cartucho
 * 
 * @param cartridge Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_cartridge_write(sms_cartridge_t *cartridge, uint16_t address, uint8_t value);

/**
 * @brief Salva o conteúdo da RAM do cartucho em arquivo
 * 
 * @param cartridge Ponteiro para a instância
 * @param save_path Caminho para o arquivo de save
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_save_ram(sms_cartridge_t *cartridge, const char *save_path);

/**
 * @brief Carrega o conteúdo da RAM do cartucho de um arquivo
 * 
 * @param cartridge Ponteiro para a instância
 * @param save_path Caminho para o arquivo de save
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_load_ram(sms_cartridge_t *cartridge, const char *save_path);

/**
 * @brief Registra o cartucho no sistema de save state
 * 
 * @param cartridge Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_register_save_state(sms_cartridge_t *cartridge, save_state_t *state);

#endif /* SMS_CARTRIDGE_H */
