/**
 * @file mapper90.c
 * @brief Implementação do Mapper 90 (JY Company)
 */

#include <stdlib.h>
#include <string.h>
#include "mapper90.h"
#include "../../log.h"

// Estrutura de contexto do Mapper 90
typedef struct
{
    nes_mapper_t mapper;        // Interface do mapper
    nes_cartridge_t *cartridge; // Ponteiro para o cartucho

    // Registradores
    uint8_t prg_mode;    // Modo de banco PRG
    uint8_t chr_mode;    // Modo de banco CHR
    uint8_t mirror_mode; // Modo de espelhamento
    uint8_t irq_latch;   // Valor do latch do IRQ
    uint8_t irq_counter; // Contador do IRQ
    uint8_t irq_enabled; // Flag de IRQ habilitado
    uint8_t irq_mode;    // Modo do IRQ (CPU/PPU)
    uint8_t mult_a;      // Multiplicador A
    uint8_t mult_b;      // Multiplicador B
    uint8_t protect;     // Registrador de proteção

    // Bancos
    uint8_t prg_bank[4]; // Bancos de PRG (8KB cada)
    uint8_t chr_bank[8]; // Bancos de CHR (1KB cada)

    // Estado do IRQ
    uint16_t irq_cycles; // Ciclos para próximo IRQ
    uint8_t irq_pending; // Flag de IRQ pendente
} mapper90_context_t;

// Funções de acesso à memória
static uint8_t mapper90_cpu_read(nes_mapper_t *mapper, uint16_t address)
{
    mapper90_context_t *ctx = (mapper90_context_t *)mapper;

    // Leitura de registradores
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        switch (address & 0x600F)
        {
        case M90_REG_MULT_A:
            return ctx->mult_a;
        case M90_REG_MULT_B:
            return ctx->mult_b;
        case M90_REG_MULT_A + 1:
            return (uint8_t)((ctx->mult_a * ctx->mult_b) >> 8);
        case M90_REG_MULT_B + 1:
            return (uint8_t)(ctx->mult_a * ctx->mult_b);
        }
        return ctx->cartridge->prg_ram[address - 0x6000];
    }

    // Leitura de PRG-ROM
    if (address >= 0x8000)
    {
        uint32_t bank = ctx->prg_bank[(address - 0x8000) / 0x2000];
        uint32_t offset = (address - 0x8000) % 0x2000;
        return ctx->cartridge->prg_rom[bank * 0x2000 + offset];
    }

    return 0;
}

static void mapper90_cpu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value)
{
    mapper90_context_t *ctx = (mapper90_context_t *)mapper;

    // Escrita em registradores
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        switch (address & 0x600F)
        {
        case M90_REG_PRG_MODE:
            ctx->prg_mode = value;
            break;
        case M90_REG_CHR_MODE:
            ctx->chr_mode = value;
            break;
        case M90_REG_MIRROR:
            ctx->mirror_mode = value & 0x03;
            break;
        case M90_REG_IRQ_LATCH:
            ctx->irq_latch = value;
            break;
        case M90_REG_IRQ_ENABLE:
            ctx->irq_enabled = value & M90_IRQ_ENABLE;
            if (ctx->irq_enabled)
            {
                ctx->irq_counter = ctx->irq_latch;
            }
            break;
        case M90_REG_IRQ_MODE:
            ctx->irq_mode = value & 0x01;
            break;
        case M90_REG_MULT_A:
            ctx->mult_a = value;
            break;
        case M90_REG_MULT_B:
            ctx->mult_b = value;
            break;
        case M90_REG_PROTECT:
            ctx->protect = value;
            break;
        default:
            if (address >= 0x6000 && address <= 0x7FFF)
            {
                ctx->cartridge->prg_ram[address - 0x6000] = value;
            }
        }
        return;
    }

    // Escrita em bancos PRG/CHR
    if (address >= 0x8000)
    {
        uint8_t bank = (address >> 13) & 0x03;
        if (address & 0x1000)
        {
            ctx->chr_bank[((address >> 11) & 0x03) | ((bank & 0x01) << 2)] = value;
        }
        else
        {
            ctx->prg_bank[bank] = value;
        }
    }
}

static uint8_t mapper90_ppu_read(nes_mapper_t *mapper, uint16_t address)
{
    mapper90_context_t *ctx = (mapper90_context_t *)mapper;

    if (address < 0x2000)
    {
        uint32_t bank = ctx->chr_bank[address / 0x400];
        uint32_t offset = address % 0x400;
        return ctx->cartridge->chr_rom[bank * 0x400 + offset];
    }

    return 0;
}

static void mapper90_ppu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value)
{
    mapper90_context_t *ctx = (mapper90_context_t *)mapper;

    if (address < 0x2000 && ctx->cartridge->chr_ram != NULL)
    {
        uint32_t bank = ctx->chr_bank[address / 0x400];
        uint32_t offset = address % 0x400;
        ctx->cartridge->chr_ram[bank * 0x400 + offset] = value;
    }
}

static void mapper90_reset(nes_mapper_t *mapper)
{
    mapper90_context_t *ctx = (mapper90_context_t *)mapper;

    // Reset dos registradores
    ctx->prg_mode = 0;
    ctx->chr_mode = 0;
    ctx->mirror_mode = 0;
    ctx->irq_latch = 0;
    ctx->irq_counter = 0;
    ctx->irq_enabled = 0;
    ctx->irq_mode = 0;
    ctx->mult_a = 0;
    ctx->mult_b = 0;
    ctx->protect = 0;

    // Reset dos bancos
    memset(ctx->prg_bank, 0, sizeof(ctx->prg_bank));
    memset(ctx->chr_bank, 0, sizeof(ctx->chr_bank));

    // Configuração inicial dos bancos PRG
    ctx->prg_bank[3] = (ctx->cartridge->prg_rom_size / 0x2000) - 1;
}

static void mapper90_clock(nes_mapper_t *mapper)
{
    mapper90_context_t *ctx = (mapper90_context_t *)mapper;

    if (ctx->irq_enabled)
    {
        if (ctx->irq_mode == M90_IRQ_MODE_CPU)
        {
            if (--ctx->irq_cycles == 0)
            {
                ctx->irq_pending = 1;
                ctx->irq_cycles = ctx->irq_counter;
            }
        }
    }
}

static uint8_t mapper90_irq_pending(nes_mapper_t *mapper)
{
    mapper90_context_t *ctx = (mapper90_context_t *)mapper;
    uint8_t pending = ctx->irq_pending;
    ctx->irq_pending = 0;
    return pending;
}

nes_mapper_t *nes_mapper_90_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        log_error("Mapper 90: Cartucho inválido");
        return NULL;
    }

    mapper90_context_t *ctx = (mapper90_context_t *)malloc(sizeof(mapper90_context_t));
    if (!ctx)
    {
        log_error("Mapper 90: Falha na alocação de memória");
        return NULL;
    }

    memset(ctx, 0, sizeof(mapper90_context_t));
    ctx->cartridge = cartridge;

    // Configurar interface do mapper
    ctx->mapper.cpu_read = mapper90_cpu_read;
    ctx->mapper.cpu_write = mapper90_cpu_write;
    ctx->mapper.ppu_read = mapper90_ppu_read;
    ctx->mapper.ppu_write = mapper90_ppu_write;
    ctx->mapper.reset = mapper90_reset;
    ctx->mapper.clock = mapper90_clock;
    ctx->mapper.irq_pending = mapper90_irq_pending;

    // Inicialização inicial
    mapper90_reset((nes_mapper_t *)ctx);

    log_info("Mapper 90 inicializado com sucesso");
    return (nes_mapper_t *)ctx;
}
