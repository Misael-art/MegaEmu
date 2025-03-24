/**
 * @file mapper0.c
 * @brief Implementação do Mapper 0 (NROM) para o NES
 *
 * O Mapper 0 é o mais simples dos mappers do NES, usado em jogos como
 * Super Mario Bros., Donkey Kong, e outros jogos de primeira geração.
 *
 * Características:
 * - Até 32KB de PRG-ROM
 * - Até 8KB de CHR-ROM ou CHR-RAM
 * - Sem bancos de memória
 * - Sem registradores
 */

#include "../nes_cartridge.h"
#include "../../../../utils/enhanced_log.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER NES_LOG_CAT_MAPPER

// Estrutura específica para o contexto do Mapper 0
typedef struct
{
    nes_cartridge_t *cart;
} mapper0_context_t;

// Protótipos de funções do mapper
static uint8_t mapper0_cpu_read(void *ctx, uint16_t addr);
static void mapper0_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper0_chr_read(void *ctx, uint16_t addr);
static void mapper0_chr_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper0_reset(void *ctx);
static void mapper0_shutdown(void *ctx);

/**
 * @brief Inicializa o Mapper 0 (NROM)
 *
 * @param cart Ponteiro para o cartucho NES
 * @return int32_t 0 em caso de sucesso, código de erro em caso de falha
 */
int32_t nes_mapper_0_init(nes_cartridge_t *cart)
{
    if (!cart)
    {
        LOG_ERROR(EMU_LOG_CAT_MAPPER, "Mapper 0: Cart inválido");
        return NES_ERROR_INVALID_PARAMETER;
    }

    LOG_INFO(EMU_LOG_CAT_MAPPER, "Inicializando Mapper 0 (NROM)");

    // Aloca o contexto do mapper
    mapper0_context_t *ctx = (mapper0_context_t *)calloc(1, sizeof(mapper0_context_t));
    if (!ctx)
    {
        LOG_ERROR(EMU_LOG_CAT_MAPPER, "Mapper 0: Falha ao alocar contexto");
        return NES_ERROR_MEMORY_ALLOCATION;
    }

    // Configura o contexto
    ctx->cart = cart;

    // Configura as funções do mapper
    cart->mapper->context = ctx;
    cart->mapper->cpu_read = mapper0_cpu_read;
    cart->mapper->cpu_write = mapper0_cpu_write;
    cart->mapper->chr_read = mapper0_chr_read;
    cart->mapper->chr_write = mapper0_chr_write;
    cart->mapper->reset = mapper0_reset;
    cart->mapper->shutdown = mapper0_shutdown;
    cart->mapper->scanline = NULL; // O NROM não usa notificações de scanline

    // Debug
    LOG_INFO(EMU_LOG_CAT_MAPPER, "Mapper 0 (NROM) inicializado:");
    LOG_INFO(EMU_LOG_CAT_MAPPER, "  PRG-ROM: %d KB", cart->prg_rom_size / 1024);
    LOG_INFO(EMU_LOG_CAT_MAPPER, "  CHR-ROM: %d KB", cart->chr_rom_size / 1024);
    LOG_INFO(EMU_LOG_CAT_MAPPER, "  CHR-RAM: %d KB", cart->chr_ram_size / 1024);
    LOG_INFO(EMU_LOG_CAT_MAPPER, "  Modo de espelhamento: %d", cart->mirror_mode);

    return 0;
}

/**
 * @brief Finaliza o Mapper 0 e libera recursos
 *
 * @param ctx Contexto do mapper
 */
static void mapper0_shutdown(void *ctx)
{
    if (!ctx)
    {
        return;
    }

    LOG_INFO(EMU_LOG_CAT_MAPPER, "Finalizando Mapper 0 (NROM)");
    free(ctx);
}

/**
 * @brief Reseta o Mapper 0
 *
 * @param ctx Contexto do mapper
 */
static void mapper0_reset(void *ctx)
{
    if (!ctx)
    {
        return;
    }

    LOG_INFO(EMU_LOG_CAT_MAPPER, "Resetando Mapper 0 (NROM)");
    // O NROM não tem estado interno para resetar
}

/**
 * @brief Lê um byte do espaço de endereçamento da CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido (0x4020-0xFFFF)
 * @return uint8_t Valor lido
 */
static uint8_t mapper0_cpu_read(void *ctx, uint16_t addr)
{
    mapper0_context_t *mapper = (mapper0_context_t *)ctx;
    if (!mapper || !mapper->cart)
    {
        return 0;
    }

    nes_cartridge_t *cart = mapper->cart;

    // PRG-RAM ($6000-$7FFF)
    if (addr >= 0x6000 && addr < 0x8000)
    {
        if (cart->prg_ram && cart->prg_ram_size > 0)
        {
            uint16_t offset = (addr - 0x6000) % cart->prg_ram_size;
            return cart->prg_ram[offset];
        }
        return 0;
    }

    // PRG-ROM ($8000-$FFFF)
    if (addr >= 0x8000)
    {
        if (cart->prg_rom && cart->prg_rom_size > 0)
        {
            // Se a PRG-ROM for 16KB, espelha em $8000-$BFFF e $C000-$FFFF
            // Se a PRG-ROM for 32KB, mapeia diretamente
            if (cart->prg_rom_size == 16 * 1024)
            {
                return cart->prg_rom[(addr - 0x8000) % 0x4000];
            }
            else
            {
                return cart->prg_rom[(addr - 0x8000) % cart->prg_rom_size];
            }
        }
        return 0;
    }

    // Outros endereços não mapeados
    return 0;
}

/**
 * @brief Escreve um byte no espaço de endereçamento da CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser escrito
 * @param val Valor a ser escrito
 */
static void mapper0_cpu_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper0_context_t *mapper = (mapper0_context_t *)ctx;
    if (!mapper || !mapper->cart)
    {
        return;
    }

    nes_cartridge_t *cart = mapper->cart;

    // PRG-RAM ($6000-$7FFF)
    if (addr >= 0x6000 && addr < 0x8000)
    {
        if (cart->prg_ram && cart->prg_ram_size > 0)
        {
            uint16_t offset = (addr - 0x6000) % cart->prg_ram_size;
            cart->prg_ram[offset] = val;
            // Marca a RAM como modificada se tiver bateria
            if (cart->has_battery)
            {
                cart->sram_dirty = 1;
            }
        }
        return;
    }

    // Tentativa de escrever em PRG-ROM - normalmente ignorada
    if (addr >= 0x8000)
    {
        // Alguns jogos fazem isso como teste, então apenas logamos
        LOG_TRACE(EMU_LOG_CAT_MAPPER, "Mapper 0: Tentativa de escrita em PRG-ROM: $%04X = $%02X", addr, val);
        return;
    }
}

/**
 * @brief Lê um byte do espaço de padrões da PPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido (0x0000-0x1FFF)
 * @return uint8_t Valor lido
 */
static uint8_t mapper0_chr_read(void *ctx, uint16_t addr)
{
    mapper0_context_t *mapper = (mapper0_context_t *)ctx;
    if (!mapper || !mapper->cart)
    {
        return 0;
    }

    nes_cartridge_t *cart = mapper->cart;

    // Leitura direta da CHR-ROM ou CHR-RAM
    if (addr < 0x2000)
    {
        if (cart->chr_rom && cart->chr_rom_size > 0)
        {
            return cart->chr_rom[addr % cart->chr_rom_size];
        }
        else if (cart->chr_ram && cart->chr_ram_size > 0)
        {
            return cart->chr_ram[addr % cart->chr_ram_size];
        }
    }

    return 0;
}

/**
 * @brief Escreve um byte no espaço de padrões da PPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser escrito
 * @param val Valor a ser escrito
 */
static void mapper0_chr_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper0_context_t *mapper = (mapper0_context_t *)ctx;
    if (!mapper || !mapper->cart)
    {
        return;
    }

    nes_cartridge_t *cart = mapper->cart;

    // Apenas escreve se for CHR-RAM
    if (addr < 0x2000)
    {
        if (cart->chr_ram && cart->chr_ram_size > 0)
        {
            cart->chr_ram[addr % cart->chr_ram_size] = val;
        }
        else if (cart->chr_rom)
        {
            // Ignorar tentativas de escrita em CHR-ROM
            LOG_TRACE(EMU_LOG_CAT_MAPPER, "Mapper 0: Tentativa de escrita em CHR-ROM: $%04X = $%02X", addr, val);
        }
    }
}
