/**
 * @file md_mapper.h
 * @brief Sistema de mappers para cartuchos do Mega Drive/Genesis
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-22
 *
 * Este arquivo define a interface para o sistema de mappers do Mega Drive/Genesis.
 * Os mappers permitem que o console suporte diferentes configurações de memória
 * nos cartuchos, incluindo bancos de memória, SRAM, e chips especiais.
 */

#ifndef EMU_MD_MAPPER_H
#define EMU_MD_MAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../../utils/common_types.h"
#include "../../../core/save_state.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Tipos de mapper suportados para o Mega Drive
     */
    typedef enum
    {
        MD_MAPPER_NONE = 0,    // ROM padrão sem mapper
        MD_MAPPER_SEGA,        // Mapper Sega padrão
        MD_MAPPER_SSF2,        // Super Street Fighter 2 (bancos de 512KB)
        MD_MAPPER_SSRPG,       // Phantasy Star/Shining Force (SRAM especial)
        MD_MAPPER_EEPROM,      // Jogos com EEPROM integrada
        MD_MAPPER_CODEMASTERS, // Mapper específico da Codemasters
        MD_MAPPER_PIER_SOLAR,  // Jogos homebrew com chips especiais
        MD_MAPPER_EA           // Electronic Arts mapper
    } md_mapper_type_t;

    /**
     * @brief Estrutura para mapper do Mega Drive
     */
    typedef struct
    {
        md_mapper_type_t type;    // Tipo de mapper
        uint32_t num_banks;       // Número de bancos
        uint32_t bank_size;       // Tamanho de cada banco em bytes
        uint32_t current_bank[8]; // Índice dos bancos atuais (até 8 slots)

        // Dados da ROM e SRAM
        uint8_t *rom_data;   // Ponteiro para os dados da ROM
        uint32_t rom_size;   // Tamanho da ROM
        uint8_t *sram_data;  // Ponteiro para os dados da SRAM
        uint32_t sram_size;  // Tamanho da SRAM
        bool sram_enabled;   // SRAM habilitada
        uint32_t sram_start; // Endereço inicial da SRAM
        uint32_t sram_end;   // Endereço final da SRAM

        // Dados para EEPROM (se aplicável)
        uint8_t *eeprom_data;    // Ponteiro para os dados da EEPROM
        uint32_t eeprom_size;    // Tamanho da EEPROM
        uint8_t eeprom_state;    // Estado atual da EEPROM
        uint16_t eeprom_address; // Endereço atual da EEPROM

        // Funções específicas para cada tipo de mapper
        uint8_t (*read_rom)(struct md_mapper *mapper, uint32_t address);
        void (*write_rom)(struct md_mapper *mapper, uint32_t address, uint8_t value);
        uint8_t (*read_sram)(struct md_mapper *mapper, uint32_t address);
        void (*write_sram)(struct md_mapper *mapper, uint32_t address, uint8_t value);
        void (*reset)(struct md_mapper *mapper);
    } md_mapper_t;

    /**
     * @brief Inicializa um mapper do Mega Drive
     *
     * @param mapper Ponteiro para o mapper a ser inicializado
     * @param type Tipo do mapper
     * @param rom_data Ponteiro para os dados da ROM
     * @param rom_size Tamanho da ROM
     * @return true se o mapper foi inicializado com sucesso, false caso contrário
     */
    bool md_mapper_init(md_mapper_t *mapper, md_mapper_type_t type, uint8_t *rom_data, uint32_t rom_size);

    /**
     * @brief Reseta um mapper do Mega Drive
     *
     * @param mapper Ponteiro para o mapper a ser resetado
     */
    void md_mapper_reset(md_mapper_t *mapper);

    /**
     * @brief Libera os recursos de um mapper do Mega Drive
     *
     * @param mapper Ponteiro para o mapper a ser liberado
     */
    void md_mapper_shutdown(md_mapper_t *mapper);

    /**
     * @brief Lê um byte da ROM mapeada
     *
     * @param mapper Ponteiro para o mapper
     * @param address Endereço a ser lido
     * @return Byte lido
     */
    uint8_t md_mapper_read_rom(md_mapper_t *mapper, uint32_t address);

    /**
     * @brief Escreve um byte na ROM mapeada (usado para controle de banco)
     *
     * @param mapper Ponteiro para o mapper
     * @param address Endereço a ser escrito
     * @param value Valor a ser escrito
     */
    void md_mapper_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);

    /**
     * @brief Lê um byte da SRAM mapeada
     *
     * @param mapper Ponteiro para o mapper
     * @param address Endereço a ser lido
     * @return Byte lido
     */
    uint8_t md_mapper_read_sram(md_mapper_t *mapper, uint32_t address);

    /**
     * @brief Escreve um byte na SRAM mapeada
     *
     * @param mapper Ponteiro para o mapper
     * @param address Endereço a ser escrito
     * @param value Valor a ser escrito
     */
    void md_mapper_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value);

    /**
     * @brief Ativa/desativa a SRAM
     *
     * @param mapper Ponteiro para o mapper
     * @param enabled Estado da SRAM (true = ativada, false = desativada)
     */
    void md_mapper_set_sram_enabled(md_mapper_t *mapper, bool enabled);

    /**
     * @brief Salva o conteúdo da SRAM em um arquivo
     *
     * @param mapper Ponteiro para o mapper
     * @param filename Nome do arquivo para salvar
     * @return true se o arquivo foi salvo com sucesso, false caso contrário
     */
    bool md_mapper_save_sram(md_mapper_t *mapper, const char *filename);

    /**
     * @brief Carrega o conteúdo da SRAM de um arquivo
     *
     * @param mapper Ponteiro para o mapper
     * @param filename Nome do arquivo para carregar
     * @return true se o arquivo foi carregado com sucesso, false caso contrário
     */
    bool md_mapper_load_sram(md_mapper_t *mapper, const char *filename);

    /**
     * @brief Detecta o tipo de mapper com base nos dados da ROM
     *
     * @param rom_data Ponteiro para os dados da ROM
     * @param rom_size Tamanho da ROM
     * @return Tipo de mapper detectado
     */
    md_mapper_type_t md_mapper_detect_type(const uint8_t *rom_data, uint32_t rom_size);

    /**
     * @brief Registra o mapper no sistema de save state
     *
     * @param state Ponteiro para o save state
     * @return Código de erro (0 para sucesso)
     */
    int32_t md_mapper_register_save_state(save_state_t *state);

    /**
     * @brief Restaura o estado do mapper a partir de um save state
     *
     * @param state Ponteiro para o save state
     * @return Código de erro (0 para sucesso)
     */
    int32_t md_mapper_restore_save_state(save_state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* EMU_MD_MAPPER_H */
