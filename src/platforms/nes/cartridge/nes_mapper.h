/**
 * @file nes_mapper.h
 * @brief Interface C para os mappers do NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef NES_MAPPER_H
#define NES_MAPPER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Estrutura opaca para o mapper NES.
 *
 * Esta estrutura serve como interface entre o código C e a implementação C++.
 * Os detalhes da implementação permanecem escondidos aqui.
 */
typedef struct nes_mapper_s nes_mapper_t;

/**
 * @brief Códigos de mapper conhecidos do NES
 */
typedef enum {
    NES_MAPPER_NROM     = 0,   // Mapper 0: NROM (No Mapper)
    NES_MAPPER_MMC1     = 1,   // Mapper 1: MMC1 (SLROM, SOROM, etc.)
    NES_MAPPER_UNROM    = 2,   // Mapper 2: UNROM
    NES_MAPPER_CNROM    = 3,   // Mapper 3: CNROM
    NES_MAPPER_MMC3     = 4,   // Mapper 4: MMC3
    NES_MAPPER_MMC5     = 5,   // Mapper 5: MMC5
    NES_MAPPER_AOROM    = 7,   // Mapper 7: AOROM
    NES_MAPPER_MMC2     = 9,   // Mapper 9: MMC2 (PNROM)
    NES_MAPPER_MMC4     = 10,  // Mapper 10: MMC4 (FJROM)
    NES_MAPPER_COLOR_DREAMS = 11, // Mapper 11: Color Dreams
    // Adicionar outros mappers conforme necessário
} nes_mapper_type_t;

/**
 * @brief Cria um novo mapper do tipo especificado
 *
 * @param mapper_id ID do mapper (corresponde à enum nes_mapper_type_t)
 * @param rom_data Ponteiro para os dados da ROM
 * @param prg_size Tamanho da PRG-ROM em bytes
 * @param chr_size Tamanho da CHR-ROM em bytes
 * @return nes_mapper_t* Ponteiro para o mapper criado, ou NULL em caso de erro
 */
nes_mapper_t* nes_mapper_create(int mapper_id, const uint8_t* rom_data, size_t prg_size, size_t chr_size);

/**
 * @brief Destrói um mapper previamente criado
 *
 * @param mapper Ponteiro para o mapper a ser destruído
 */
void nes_mapper_destroy(nes_mapper_t* mapper);

/**
 * @brief Reseta o estado do mapper
 *
 * @param mapper Ponteiro para o mapper
 */
void nes_mapper_reset(nes_mapper_t* mapper);

/**
 * @brief Lê um byte da CPU
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de memória (0x8000-0xFFFF)
 * @return uint8_t Byte lido
 */
uint8_t nes_mapper_cpu_read(nes_mapper_t* mapper, uint16_t address);

/**
 * @brief Escreve um byte pela CPU
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de memória (0x8000-0xFFFF)
 * @param data Byte a ser escrito
 */
void nes_mapper_cpu_write(nes_mapper_t* mapper, uint16_t address, uint8_t data);

/**
 * @brief Lê um byte da PPU (CHR)
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de memória (0x0000-0x1FFF)
 * @return uint8_t Byte lido
 */
uint8_t nes_mapper_ppu_read(nes_mapper_t* mapper, uint16_t address);

/**
 * @brief Escreve um byte pela PPU (CHR)
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de memória (0x0000-0x1FFF)
 * @param data Byte a ser escrito
 */
void nes_mapper_ppu_write(nes_mapper_t* mapper, uint16_t address, uint8_t data);

/**
 * @brief Notifica o mapper sobre uma nova scanline para IRQ
 *
 * @param mapper Ponteiro para o mapper
 */
void nes_mapper_scanline(nes_mapper_t* mapper);

/**
 * @brief Verifica o estado do sinal de IRQ
 *
 * @param mapper Ponteiro para o mapper
 * @return bool true se IRQ está ativo, false caso contrário
 */
bool nes_mapper_irq_state(nes_mapper_t* mapper);

/**
 * @brief Limpa o sinal de IRQ
 *
 * @param mapper Ponteiro para o mapper
 */
void nes_mapper_irq_clear(nes_mapper_t* mapper);

/**
 * @brief Salva o estado do mapper
 *
 * @param mapper Ponteiro para o mapper
 * @param state Ponteiro para o estado
 * @param state_size Tamanho do buffer de estado
 * @return bool true se bem-sucedido, false caso contrário
 */
bool nes_mapper_save_state(nes_mapper_t* mapper, void* state, size_t state_size);

/**
 * @brief Carrega o estado do mapper
 *
 * @param mapper Ponteiro para o mapper
 * @param state Ponteiro para o estado
 * @param state_size Tamanho do buffer de estado
 * @return bool true se bem-sucedido, false caso contrário
 */
bool nes_mapper_load_state(nes_mapper_t* mapper, const void* state, size_t state_size);

/**
 * @brief Obtém o tipo de espelhamento de nametable do mapper
 *
 * @param mapper Ponteiro para o mapper
 * @return int Tipo de espelhamento (0=horizontal, 1=vertical, 2=single screen, 3=four screen)
 */
int nes_mapper_get_mirroring(nes_mapper_t* mapper);

#ifdef __cplusplus
}
#endif

#endif // NES_MAPPER_H
