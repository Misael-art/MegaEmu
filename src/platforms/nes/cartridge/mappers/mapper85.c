/**
 * @file mapper85.c
 * @brief Implementação do Mapper 85 (VRC7)
 *
 * O VRC7 é um mapper complexo que adiciona um chip de som FM ao NES.
 * Características principais:
 * - Suporta até 512KB de PRG-ROM
 * - Suporta até 256KB de CHR-ROM
 * - 8KB de PRG-RAM com bateria opcional
 * - IRQ baseado em scanline
 * - Chip de som FM YM2413 (OPLL)
 */

#include "platforms/nes/cartridge/mappers/mapper85.h"
#include "platforms/nes/cartridge/nes_cartridge.h"
#include <stdlib.h>
#include <string.h>

// Definições de logging
#define EMU_LOG_CAT_NES_MAPPERS 0
#define EMU_LOG_ERROR(cat, fmt, ...) ((void)0)
#define EMU_LOG_WARN(cat, fmt, ...) ((void)0)
#define EMU_LOG_INFO(cat, fmt, ...) ((void)0)
#define EMU_LOG_DEBUG(cat, fmt, ...) ((void)0)
#define EMU_LOG_TRACE(cat, fmt, ...) ((void)0)

// Estrutura de contexto do mapper
typedef struct
{
    nes_cartridge_t *cart;

    // Registradores de banco
    uint8_t prg_bank[3]; // 8KB cada
    uint8_t chr_bank[8]; // 1KB cada

    // Registradores de IRQ
    uint8_t irq_latch;
    uint8_t irq_counter;
    uint8_t irq_control;
    bool irq_enabled;
    bool irq_pending;

    // Registradores de som
    uint8_t sound_reg_addr;
    uint8_t sound_reg_data[0x40];

    // Estado do mirroring
    uint8_t mirror_mode;
} mapper85_context_t;

// Funções auxiliares
static uint32_t get_prg_bank_addr(mapper85_context_t *ctx, uint8_t bank)
{
    return (bank & 0x3F) * 0x2000;
}

static uint32_t get_chr_bank_addr(mapper85_context_t *ctx, uint8_t bank)
{
    return (bank & 0xFF) * 0x400;
}

// Funções de acesso à memória
static uint8_t cpu_read(void *context, uint16_t addr)
{
    mapper85_context_t *ctx = (mapper85_context_t *)context;

    // PRG-RAM (6000-7FFF)
    if (addr >= 0x6000 && addr <= 0x7FFF)
    {
        return ctx->cart->prg_ram[addr - 0x6000];
    }

    // PRG-ROM (8000-FFFF)
    if (addr >= 0x8000)
    {
        uint32_t bank_addr;

        if (addr <= 0x9FFF)
        {
            bank_addr = get_prg_bank_addr(ctx, ctx->prg_bank[0]);
        }
        else if (addr <= 0xBFFF)
        {
            bank_addr = get_prg_bank_addr(ctx, ctx->prg_bank[1]);
        }
        else if (addr <= 0xDFFF)
        {
            bank_addr = get_prg_bank_addr(ctx, ctx->prg_bank[2]);
        }
        else
        {
            // Último banco fixo
            bank_addr = ctx->cart->prg_rom_size - 0x2000;
        }

        return ctx->cart->prg_rom[bank_addr + (addr & 0x1FFF)];
    }

    return 0;
}

static void cpu_write(void *context, uint16_t addr, uint8_t value)
{
    mapper85_context_t *ctx = (mapper85_context_t *)context;

    // PRG-RAM (6000-7FFF)
    if (addr >= 0x6000 && addr <= 0x7FFF)
    {
        ctx->cart->prg_ram[addr - 0x6000] = value;
        return;
    }

    // Registradores (8000-FFFF)
    if (addr >= 0x8000)
    {
        switch (addr & 0xF010)
        {
        case 0x8000: // PRG banco 0
            ctx->prg_bank[0] = value;
            break;

        case 0x8010: // PRG banco 1
            ctx->prg_bank[1] = value;
            break;

        case 0x9000: // PRG banco 2
            ctx->prg_bank[2] = value;
            break;

        case 0x9010: // Sound register address
            ctx->sound_reg_addr = value & 0x3F;
            break;

        case 0x9030: // Sound register data
            ctx->sound_reg_data[ctx->sound_reg_addr] = value;
            break;

        case 0xA000: // CHR banco 0
        case 0xA010: // CHR banco 1
        case 0xB000: // CHR banco 2
        case 0xB010: // CHR banco 3
        case 0xC000: // CHR banco 4
        case 0xC010: // CHR banco 5
        case 0xD000: // CHR banco 6
        case 0xD010: // CHR banco 7
            ctx->chr_bank[(addr - 0xA000) >> 4] = value;
            break;

        case 0xE000: // IRQ latch
            ctx->irq_latch = value;
            break;

        case 0xE010: // IRQ control
            ctx->irq_enabled = (value & 0x02) != 0;
            ctx->irq_control = value;
            if (ctx->irq_enabled)
            {
                ctx->irq_counter = ctx->irq_latch;
            }
            ctx->irq_pending = false;
            break;

        case 0xF000: // IRQ acknowledge
            ctx->irq_pending = false;
            break;
        }
    }
}

static uint8_t ppu_read(void *context, uint16_t addr)
{
    mapper85_context_t *ctx = (mapper85_context_t *)context;

    if (addr < 0x2000)
    {
        uint8_t bank = ctx->chr_bank[addr >> 10];
        uint32_t chr_addr = get_chr_bank_addr(ctx, bank);
        return ctx->cart->chr_rom[chr_addr + (addr & 0x3FF)];
    }

    return 0;
}

static void ppu_write(void *context, uint16_t addr, uint8_t value)
{
    // VRC7 não tem CHR-RAM, então escritas são ignoradas
    (void)context;
    (void)addr;
    (void)value;
}

static void scanline(void *context)
{
    mapper85_context_t *ctx = (mapper85_context_t *)context;

    if (!ctx->irq_enabled)
    {
        return;
    }

    if (ctx->irq_counter == 0xFF)
    {
        ctx->irq_counter = ctx->irq_latch;
        ctx->irq_pending = true;
    }
    else
    {
        ctx->irq_counter++;
    }
}

static void reset(void *context)
{
    mapper85_context_t *ctx = (mapper85_context_t *)context;

    // Reseta os bancos PRG
    ctx->prg_bank[0] = 0;
    ctx->prg_bank[1] = 1;
    ctx->prg_bank[2] = 2;

    // Reseta os bancos CHR
    for (int i = 0; i < 8; i++)
    {
        ctx->chr_bank[i] = i;
    }

    // Reseta o IRQ
    ctx->irq_latch = 0;
    ctx->irq_counter = 0;
    ctx->irq_control = 0;
    ctx->irq_enabled = false;
    ctx->irq_pending = false;

    // Reseta os registradores de som
    ctx->sound_reg_addr = 0;
    memset(ctx->sound_reg_data, 0, sizeof(ctx->sound_reg_data));
}

static void shutdown(void *context)
{
    mapper85_context_t *ctx = (mapper85_context_t *)context;
    free(ctx);
}

nes_mapper_t *nes_mapper_85_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        EMU_LOG_ERROR(EMU_LOG_CAT_NES_MAPPERS, "Cartridge inválido para Mapper 85");
        return NULL;
    }

    // Aloca e inicializa o contexto
    mapper85_context_t *ctx = (mapper85_context_t *)malloc(sizeof(mapper85_context_t));
    if (!ctx)
    {
        EMU_LOG_ERROR(EMU_LOG_CAT_NES_MAPPERS, "Falha ao alocar contexto para Mapper 85");
        return NULL;
    }

    memset(ctx, 0, sizeof(mapper85_context_t));
    ctx->cart = cartridge;

    // Aloca e inicializa o mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        EMU_LOG_ERROR(EMU_LOG_CAT_NES_MAPPERS, "Falha ao alocar Mapper 85");
        free(ctx);
        return NULL;
    }

    mapper->number = 85;
    mapper->name = "VRC7";
    mapper->context = ctx;
    mapper->cpu_read = cpu_read;
    mapper->cpu_write = cpu_write;
    mapper->ppu_read = ppu_read;
    mapper->ppu_write = ppu_write;
    mapper->scanline = scanline;
    mapper->reset = reset;
    mapper->shutdown = shutdown;

    // Inicializa o estado
    reset(ctx);

    EMU_LOG_INFO(EMU_LOG_CAT_NES_MAPPERS, "Mapper 85 (VRC7) inicializado com sucesso");
    return mapper;
}
