/**
 * @file mapper3.c
 * @brief Implementação do Mapper 3 (CNROM) para o Nintendo Entertainment System
 *
 * O Mapper 3 (CNROM) é um mapper simples usado em jogos como Adventure Island,
 * Arkanoid, Bump'n'Jump, e outros títulos da Konami e Nintendo.
 *
 * Características:
 * - PRG-ROM: 16KB ou 32KB (fixos, sem troca de bancos)
 * - CHR-ROM: Até 32KB (4 bancos de 8KB)
 * - Chaveamento simples de bancos de CHR-ROM
 * - Não possui PRG-RAM com bateria
 * - Suporta espelhamento vertical e horizontal (determinado pelo cabeçalho do arquivo)
 *
 * O CNROM não possui chaveamento de PRG-ROM; toda a PRG-ROM é mapeada de forma fixa.
 * Qualquer escrita no espaço de endereçamento $8000-$FFFF seleciona um dos quatro
 * bancos de 8KB de CHR-ROM.
 */

#include "platforms/nes/cartridge/nes_cartridge.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 3
#define MAPPER_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Estrutura de contexto para o Mapper 3
typedef struct
{
    nes_cartridge_t *cart;  // Ponteiro para o cartucho
    uint8_t chr_bank;       // Banco atual de CHR-ROM (0-3)
    uint8_t chr_bank_count; // Número total de bancos de 8KB
    uint8_t *prg_rom;       // Ponteiro para a PRG-ROM
    int prg_rom_size;       // Tamanho da PRG-ROM em bytes
    uint8_t *chr_rom;       // Ponteiro para a CHR-ROM
    int chr_rom_size;       // Tamanho da CHR-ROM em bytes
    uint8_t *chr_ram;       // Ponteiro para a CHR-RAM (se existir)
    int chr_ram_size;       // Tamanho da CHR-RAM em bytes
} mapper3_context_t;

// Protótipos de funções
static uint8_t mapper3_cpu_read(void *ctx, uint16_t addr);
static void mapper3_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper3_chr_read(void *ctx, uint16_t addr);
static void mapper3_chr_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper3_reset(void *ctx);
static void mapper3_shutdown(void *ctx);

/**
 * @brief Inicializa o Mapper 3 (CNROM)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_3_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): cartridge inválido");
        return NULL;
    }

    MAPPER_LOG_INFO("Inicializando Mapper 3 (CNROM)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): falha ao alocar estrutura do mapper");
        return NULL;
    }

    // Inicializa a estrutura com zeros
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto do mapper
    mapper3_context_t *ctx = (mapper3_context_t *)malloc(sizeof(mapper3_context_t));
    if (!ctx)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): falha ao alocar contexto do mapper");
        free(mapper);
        return NULL;
    }

    // Inicializa o contexto
    ctx->cart = cartridge;
    ctx->prg_rom = cartridge->prg_rom;
    ctx->prg_rom_size = cartridge->prg_rom_size;
    ctx->chr_rom = cartridge->chr_rom;
    ctx->chr_rom_size = cartridge->chr_rom_size;
    ctx->chr_ram = cartridge->chr_ram;
    ctx->chr_ram_size = cartridge->chr_ram_size;

    // Inicializa o banco de CHR
    ctx->chr_bank = 0;
    ctx->chr_bank_count = cartridge->chr_rom_size / 8192; // 8KB por banco

    MAPPER_LOG_DEBUG("Mapper 3 (CNROM): PRG-ROM: %d KB, CHR-ROM: %d KB (%d bancos de 8KB)",
                     ctx->prg_rom_size / 1024, ctx->chr_rom_size / 1024, ctx->chr_bank_count);

    // Configura as funções do mapper
    mapper->mapper_number = 3;
    mapper->cpu_read = mapper3_cpu_read;
    mapper->cpu_write = mapper3_cpu_write;
    mapper->chr_read = mapper3_chr_read;
    mapper->chr_write = mapper3_chr_write;
    mapper->reset = mapper3_reset;
    mapper->shutdown = mapper3_shutdown;
    mapper->context = ctx;

    MAPPER_LOG_INFO("Mapper 3 (CNROM) inicializado com sucesso");
    return mapper;
}

/**
 * @brief Reseta o estado do Mapper 3
 *
 * @param ctx Contexto do mapper
 */
static void mapper3_reset(void *ctx)
{
    mapper3_context_t *mapper_ctx = (mapper3_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): contexto inválido no reset");
        return;
    }

    // Reseta para o primeiro banco de CHR-ROM
    mapper_ctx->chr_bank = 0;

    MAPPER_LOG_DEBUG("Mapper 3 (CNROM): reset realizado, banco de CHR-ROM: %d", mapper_ctx->chr_bank);
}

/**
 * @brief Finaliza e libera recursos do Mapper 3
 *
 * @param ctx Contexto do mapper
 */
static void mapper3_shutdown(void *ctx)
{
    if (!ctx)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): contexto inválido no shutdown");
        return;
    }

    MAPPER_LOG_DEBUG("Mapper 3 (CNROM): liberando recursos");
    free(ctx);
}

/**
 * @brief Lê um byte do espaço de endereços da CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para leitura
 * @return uint8_t Valor lido
 */
static uint8_t mapper3_cpu_read(void *ctx, uint16_t addr)
{
    mapper3_context_t *mapper_ctx = (mapper3_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): contexto inválido na leitura da CPU");
        return 0;
    }

    // Endereços $8000-$FFFF: PRG-ROM
    if (addr >= 0x8000)
    {
        // Para ROMs de 16KB, espelha em $8000-$FFFF
        if (mapper_ctx->prg_rom_size == 16384)
        {
            uint32_t effective_addr = (addr - 0x8000) % 16384;
            return mapper_ctx->prg_rom[effective_addr];
        }
        // Para ROMs de 32KB, mapeia diretamente
        else
        {
            uint32_t effective_addr = (addr - 0x8000);
            if (effective_addr < mapper_ctx->prg_rom_size)
            {
                return mapper_ctx->prg_rom[effective_addr];
            }
            else
            {
                MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso a endereço PRG-ROM inválido: $%04X (efetivo: $%06X)", addr, effective_addr);
                return 0;
            }
        }
    }
    // Endereços $6000-$7FFF: PRG-RAM (geralmente não há)
    else if (addr >= 0x6000 && addr <= 0x7FFF)
    {
        // Alguns cartuchos CNROM têm PRG-RAM, outros não
        if (mapper_ctx->cart->prg_ram && mapper_ctx->cart->prg_ram_size > 0)
        {
            uint16_t ram_addr = addr - 0x6000;
            if (ram_addr < mapper_ctx->cart->prg_ram_size)
            {
                return mapper_ctx->cart->prg_ram[ram_addr];
            }
        }

        MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de leitura a PRG-RAM inexistente: $%04X", addr);
        return 0;
    }

    // Qualquer outro endereço
    MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de leitura a endereço não mapeado: $%04X", addr);
    return 0;
}

/**
 * @brief Escreve um byte no espaço de endereços da CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para escrita
 * @param val Valor a ser escrito
 */
static void mapper3_cpu_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper3_context_t *mapper_ctx = (mapper3_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): contexto inválido na escrita da CPU");
        return;
    }

    // Endereços $8000-$FFFF: seleção de banco de CHR
    if (addr >= 0x8000)
    {
        // No CNROM, qualquer escrita para $8000-$FFFF seleciona o banco de CHR
        uint8_t bank = val & 0x03; // Apenas os 2 bits inferiores são usados (4 bancos máximo)

        // Verificar se o banco selecionado é válido
        if (bank >= mapper_ctx->chr_bank_count)
        {
            MAPPER_LOG_WARN("Mapper 3 (CNROM): tentativa de selecionar banco inválido %d (máximo: %d)",
                            bank, mapper_ctx->chr_bank_count - 1);

            // Ajustar para um banco válido
            bank %= mapper_ctx->chr_bank_count;
        }

        mapper_ctx->chr_bank = bank;
        MAPPER_LOG_DEBUG("Mapper 3 (CNROM): banco CHR selecionado: %d", mapper_ctx->chr_bank);
    }
    // Endereços $6000-$7FFF: PRG-RAM (geralmente não há)
    else if (addr >= 0x6000 && addr <= 0x7FFF)
    {
        // Alguns cartuchos CNROM têm PRG-RAM, outros não
        if (mapper_ctx->cart->prg_ram && mapper_ctx->cart->prg_ram_size > 0)
        {
            uint16_t ram_addr = addr - 0x6000;
            if (ram_addr < mapper_ctx->cart->prg_ram_size)
            {
                mapper_ctx->cart->prg_ram[ram_addr] = val;
                return;
            }
        }

        MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de escrita a PRG-RAM inexistente: $%04X = $%02X", addr, val);
    }
    else
    {
        // Qualquer outro endereço
        MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de escrita a endereço não mapeado: $%04X = $%02X", addr, val);
    }
}

/**
 * @brief Lê um byte do espaço de endereços da PPU (padrões/tiles)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para leitura
 * @return uint8_t Valor lido
 */
static uint8_t mapper3_chr_read(void *ctx, uint16_t addr)
{
    mapper3_context_t *mapper_ctx = (mapper3_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): contexto inválido na leitura da PPU");
        return 0;
    }

    // Endereços $0000-$1FFF: CHR-ROM/RAM
    if (addr <= 0x1FFF)
    {
        // Se temos CHR-ROM, usar ela
        if (mapper_ctx->chr_rom && mapper_ctx->chr_rom_size > 0)
        {
            uint32_t effective_addr = (mapper_ctx->chr_bank * 8192) + addr;
            if (effective_addr < mapper_ctx->chr_rom_size)
            {
                return mapper_ctx->chr_rom[effective_addr];
            }
            else
            {
                MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso a endereço CHR-ROM inválido: $%04X (efetivo: $%06X)", addr, effective_addr);
                return 0;
            }
        }
        // Se não temos CHR-ROM mas temos CHR-RAM, usar ela
        else if (mapper_ctx->chr_ram && mapper_ctx->chr_ram_size > 0)
        {
            if (addr < mapper_ctx->chr_ram_size)
            {
                return mapper_ctx->chr_ram[addr];
            }
            else
            {
                MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso a endereço CHR-RAM inválido: $%04X", addr);
                return 0;
            }
        }
        // Nem CHR-ROM nem CHR-RAM
        else
        {
            MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de leitura a CHR inexistente: $%04X", addr);
            return 0;
        }
    }

    // Qualquer outro endereço
    MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de leitura a endereço PPU não mapeado: $%04X", addr);
    return 0;
}

/**
 * @brief Escreve um byte no espaço de endereços da PPU (padrões/tiles)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para escrita
 * @param val Valor a ser escrito
 */
static void mapper3_chr_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper3_context_t *mapper_ctx = (mapper3_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 3 (CNROM): contexto inválido na escrita da PPU");
        return;
    }

    // Endereços $0000-$1FFF: CHR-RAM (se existir)
    if (addr <= 0x1FFF)
    {
        // Se temos CHR-ROM, ignorar a escrita
        if (mapper_ctx->chr_rom && mapper_ctx->chr_rom_size > 0)
        {
            MAPPER_LOG_WARN("Mapper 3 (CNROM): tentativa de escrita em CHR-ROM: $%04X = $%02X", addr, val);
            return;
        }
        // Se temos CHR-RAM, permitir a escrita
        else if (mapper_ctx->chr_ram && mapper_ctx->chr_ram_size > 0)
        {
            if (addr < mapper_ctx->chr_ram_size)
            {
                mapper_ctx->chr_ram[addr] = val;
                return;
            }
            else
            {
                MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de escrita a endereço CHR-RAM inválido: $%04X = $%02X", addr, val);
                return;
            }
        }
        // Nem CHR-ROM nem CHR-RAM
        else
        {
            MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de escrita a CHR inexistente: $%04X = $%02X", addr, val);
            return;
        }
    }

    // Qualquer outro endereço
    MAPPER_LOG_WARN("Mapper 3 (CNROM): acesso de escrita a endereço PPU não mapeado: $%04X = $%02X", addr, val);
}
