/** * @file nes_cartridge.h * @brief Definições para o subsistema de cartuchos do NES */
#ifndef NES_CARTRIDGE_H
#define NES_CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../core/core_types.h"
#include "../../../utils/error_handling.h"
#include "../../../utils/enhanced_log.h"
#include "../ppu/nes_ppu.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** * @brief Constantes para o cabeçalho iNES */
#define NES_INES_MAGIC "NES\x1A"    /**< Assinatura do formato iNES */
#define NES_INES_HEADER_SIZE 16     /**< Tamanho do cabeçalho iNES */
#define NES_PRG_ROM_BANK_SIZE 16384 /**< Tamanho de um banco de PRG-ROM (16KB) */
#define NES_CHR_ROM_BANK_SIZE 8192  /**< Tamanho de um banco de CHR-ROM (8KB) */

/** * @brief Flags para tipo de espelhamento em cartuchos */
#define NES_MIRROR_HORIZONTAL_FLAG 0 /**< Flag para espelhamento horizontal */
#define NES_MIRROR_VERTICAL_FLAG 1   /**< Flag para espelhamento vertical */
#define NES_MIRROR_FOUR_SCREEN 8     /**< Flag para modo de quatro telas */

    /**     * @brief Informações sobre a ROM do NES     */
    typedef struct
    {
        int32_t mapper_number; /**< Número do mapper */
        int32_t prg_rom_size;  /**< Tamanho da PRG-ROM em bytes */
        int32_t chr_rom_size;  /**< Tamanho da CHR-ROM em bytes */
        int32_t prg_ram_size;  /**< Tamanho da PRG-RAM em bytes */
        int32_t has_battery;   /**< 1 se tem bateria (SRAM persistente), 0 caso contrário */
        int32_t mapper_type;   /**< Tipo do mapper (enum nes_mapper_type_t) */
        int32_t mirroring;     /**< Tipo de espelhamento */
        uint8_t *prg_rom;      /**< Ponteiro para dados da ROM de programa */
        uint8_t *chr_rom;      /**< Ponteiro para dados da ROM de caracteres */
    } nes_rom_info_t;

    /**     * @brief Callbacks para mappers     */
    typedef uint8_t (*nes_mapper_read_func_t)(void *ctx, uint16_t addr);
    typedef void (*nes_mapper_write_func_t)(void *ctx, uint16_t addr, uint8_t val);
    typedef uint8_t (*nes_mapper_chr_read_func_t)(void *ctx, uint16_t addr);
    typedef void (*nes_mapper_chr_write_func_t)(void *ctx, uint16_t addr, uint8_t val);
    typedef void (*nes_mapper_scanline_func_t)(void *ctx);
    typedef void (*nes_mapper_reset_func_t)(void *ctx);
    typedef void (*nes_mapper_shutdown_func_t)(void *ctx);

    /**     * @brief Estrutura de um mapper     */
    typedef struct
    {
        int32_t mapper_number;                 /**< Número do mapper */
        nes_mapper_read_func_t cpu_read;       /**< Função de leitura do espaço da CPU */
        nes_mapper_write_func_t cpu_write;     /**< Função de escrita do espaço da CPU */
        nes_mapper_chr_read_func_t chr_read;   /**< Função de leitura da CHR-ROM/RAM */
        nes_mapper_chr_write_func_t chr_write; /**< Função de escrita da CHR-ROM/RAM */
        nes_mapper_scanline_func_t scanline;   /**< Função chamada a cada scanline */
        nes_mapper_reset_func_t reset;         /**< Função chamada ao resetar o sistema */
        nes_mapper_shutdown_func_t shutdown;   /**< Função chamada ao desligar o sistema */
        void *context;                         /**< Contexto para callbacks */
    } nes_mapper_t;

    /**     * @brief Estrutura do cartucho do NES     */
    typedef struct
    {
        // Dados da ROM
        uint8_t *prg_rom; /**< Dados da PRG-ROM */
        uint8_t *chr_rom; /**< Dados da CHR-ROM */
        uint8_t *prg_ram; /**< Dados da PRG-RAM (incluindo SRAM) */
        uint8_t *chr_ram; /**< Dados da CHR-RAM (se o cartucho não tiver CHR-ROM) */
        // Tamanhos
        int32_t prg_rom_size; /**< Tamanho da PRG-ROM em bytes */
        int32_t chr_rom_size; /**< Tamanho da CHR-ROM em bytes */
        int32_t prg_ram_size; /**< Tamanho da PRG-RAM em bytes */
        int32_t chr_ram_size; /**< Tamanho da CHR-RAM em bytes */
        // Informações da ROM
        int32_t mapper_number; /**< Número do mapper */
        int32_t has_battery;   /**< 1 se tem bateria (SRAM persistente), 0 caso contrário */
        int32_t mirroring;     /**< Tipo de espelhamento */
        // Campos adicionais
        int32_t mapper_type; /**< Tipo do mapper (enum nes_mapper_type_t) */
        int32_t mirror_mode; /**< Modo de espelhamento (enum nes_mirror_mode_t) */
        void *mapper_data;   /**< Dados específicos do mapper */
        int32_t sram_dirty;  /**< Flag indicando se a SRAM foi modificada */
        // Mapper
        nes_mapper_t *mapper; /**< Ponteiro para o mapper */
        // Caminho do arquivo ROM
        char *rom_path; /**< Caminho para o arquivo ROM */
    } nes_cartridge_t;

    /**     * @brief Estrutura base para contexto de mappers     */
    typedef struct
    {
        nes_cartridge_t *cart;    /**< Ponteiro para o cartucho */
        void *cpu;                /**< Ponteiro para a CPU (para gerar IRQs) */
        int32_t current_scanline; /**< Scanline atual para uso na função scanline */
    } nes_mapper_base_ctx_t;

    /**     * @brief Inicializa um novo cartucho NES     *     * @return nes_cartridge_t* Ponteiro para o cartucho inicializado, NULL em caso de erro     */
    nes_cartridge_t *nes_cartridge_init(void);

    /**     * @brief Finaliza e libera recursos de um cartucho     *     * @param cart Ponteiro para o cartucho     */
    void nes_cartridge_shutdown(nes_cartridge_t *cart);

    /**     * @brief Carrega uma ROM no formato iNES     *     * @param cart Ponteiro para o cartucho     * @param rom_path Caminho para o arquivo ROM     * @return int32_t 0 em caso de sucesso, código de erro em caso de falha     */
    int32_t nes_cartridge_load(nes_cartridge_t *cart, const char *rom_path);

    /**     * @brief Obtém informações da ROM carregada     *     * @param cart Ponteiro para o cartucho     * @param info Ponteiro para a estrutura onde armazenar as informações     * @return int32_t 0 em caso de sucesso, código de erro em caso de falha     */
    int32_t nes_cartridge_get_info(nes_cartridge_t *cart, nes_rom_info_t *info);

    /**     * @brief Reseta o estado do cartucho     *     * @param cart Ponteiro para o cartucho     */
    void nes_cartridge_reset(nes_cartridge_t *cart);

    /**     * @brief Lê um uint8_t do espaço de endereços da CPU     *     * @param cart Ponteiro para o cartucho     * @param address Endereço a ler     * @return uint8_t Valor lido     */
    uint8_t nes_cartridge_cpu_read(nes_cartridge_t *cart, uint16_t address);

    /**     * @brief Escreve um uint8_t no espaço de endereços da CPU     *     * @param cart Ponteiro para o cartucho     * @param address Endereço a escrever     * @param value Valor a escrever     */
    void nes_cartridge_cpu_write(nes_cartridge_t *cart, uint16_t address, uint8_t value);

    /**     * @brief Lê um uint8_t do espaço de endereços da PPU     *     * @param cart Ponteiro para o cartucho     * @param address Endereço a ler     * @return uint8_t Valor lido     */
    uint8_t nes_cartridge_chr_read(nes_cartridge_t *cart, uint16_t address);

    /**     * @brief Escreve um uint8_t no espaço de endereços da PPU     *     * @param cart Ponteiro para o cartucho     * @param address Endereço a escrever     * @param value Valor a escrever     */
    void nes_cartridge_chr_write(nes_cartridge_t *cart, uint16_t address, uint8_t value);

    /**     * @brief Notifica o cartucho sobre um novo scanline     *     * @param cart Ponteiro para o cartucho     */
    void nes_cartridge_scanline(nes_cartridge_t *cart);

    /**     * @brief Salva o estado da SRAM para arquivo     *     * @param cart Ponteiro para o cartucho     * @param save_path Caminho para o arquivo de save (ou NULL para usar o padrão)     * @return int32_t 0 em caso de sucesso, código de erro em caso de falha     */
    int32_t nes_cartridge_save_sram(nes_cartridge_t *cart, const char *save_path);

    /**     * @brief Carrega o estado da SRAM de um arquivo     *     * @param cart Ponteiro para o cartucho     * @param save_path Caminho para o arquivo de save (ou NULL para usar o padrão)     * @return int32_t 0 em caso de sucesso, código de erro em caso de falha     */
    int32_t nes_cartridge_load_sram(nes_cartridge_t *cart, const char *save_path);

    /**     * @brief Cria um mapper para o cartucho baseado no número do mapper     *     * @param cart Ponteiro para o cartucho     * @return int32_t 0 em caso de sucesso, código de erro em caso de falha     */
    int32_t nes_cartridge_create_mapper(nes_cartridge_t *cart);

    /**     * @brief Retorna o modo de espelhamento do cartucho     *     * @param cartridge Ponteiro para o cartucho     * @return nes_mirror_mode_t Modo de espelhamento correspondente     */
    nes_mirror_mode_t nes_cartridge_get_mirror_mode(nes_cartridge_t *cartridge);

    /**
     * @brief Tipo de função para inicialização de mappers
     */
    typedef int32_t (*nes_mapper_init_func_t)(nes_cartridge_t *cart);

    /**
     * @brief Registra todos os mappers suportados no sistema
     *
     * Esta função deve ser chamada durante a inicialização do subsistema de cartuchos.
     * Ela registra todos os mappers suportados para que possam ser instanciados conforme necessário.
     */
    void nes_cartridge_register_mappers(void);

    /**
     * @brief Verifica se um mapper específico é suportado
     *
     * @param mapper_number Número do mapper a verificar
     * @return bool true se o mapper é suportado, false caso contrário
     */
    bool nes_cartridge_is_mapper_supported(int32_t mapper_number);

    /**
     * @brief Obtém o nome de um mapper específico
     *
     * @param mapper_number Número do mapper
     * @return const char* Nome do mapper ou "Unknown" se não for reconhecido
     */
    const char *nes_cartridge_get_mapper_name(int32_t mapper_number);

    /**
     * @brief Inicializa o sistema de mappers
     *
     * @return int32_t 0 em caso de sucesso, código de erro em caso de falha
     */
    int32_t nes_cartridge_mappers_init(void);

    /**
     * @brief Finaliza o sistema de mappers
     */
    void nes_cartridge_mappers_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* NES_CARTRIDGE_H */
