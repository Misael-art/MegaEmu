/**
 * @file mapper255.c
 * @brief Implementação do Mapper 255 (110-in-1)
 */

#include <stdlib.h>
#include <string.h>
#include "mapper255.h"
#include "../../log.h"

// Estrutura de contexto do Mapper 255
typedef struct
{
    nes_mapper_t mapper;        // Interface do mapper
    nes_cartridge_t *cartridge; // Ponteiro para o cartucho

    uint8_t prg_bank;    // Banco atual de PRG
    uint8_t chr_bank;    // Banco atual de CHR
    uint8_t mirror_mode; // Modo de espelhamento
    uint8_t protect;     // Estado da proteção
} mapper255_context_t;

static uint8_t mapper255_cpu_read(nes_mapper_t *mapper, uint16_t address)
{
    mapper255_context_t *ctx = (mapper255_context_t *)mapper;

    // Leitura de PRG-RAM
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        if (ctx->cartridge->prg_ram)
        {
            return ctx->cartridge->prg_ram[address - 0x6000];
        }
        return 0;
    }

    // Leitura de PRG-ROM
    if (address >= 0x8000)
    {
        uint32_t bank = ctx->prg_bank;
        uint32_t offset = address & 0x7FFF;
        return ctx->cartridge->prg_rom[bank * 0x8000 + offset];
    }

    return 0;
}

static void mapper255_cpu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value)
{
    mapper255_context_t *ctx = (mapper255_context_t *)mapper;

    // Escrita em PRG-RAM
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        if (ctx->cartridge->prg_ram && !(ctx->protect & M255_PROTECT_ON))
        {
            ctx->cartridge->prg_ram[address - 0x6000] = value;
        }
        return;
    }

    // Escrita em registradores
    if (address >= 0x8000)
    {
        switch (address & 0x8001)
        {
        case M255_REG_BANK:
            // O registrador de banco é dividido em:
            // Bits 0-5: Seleção de banco PRG
            // Bits 6-7: Seleção de banco CHR
            ctx->prg_bank = value & 0x3F;
            ctx->chr_bank = (value >> 6) & 0x03;
            break;

        case M255_REG_PROTECT:
            // Bit 7: Proteção de PRG-RAM
            // Bit 0: Modo de espelhamento
            ctx->protect = value & M255_PROTECT_ON;
            ctx->mirror_mode = value & M255_MIRROR_VERT;
            break;
        }
    }
}

static uint8_t mapper255_ppu_read(nes_mapper_t *mapper, uint16_t address)
{
    mapper255_context_t *ctx = (mapper255_context_t *)mapper;

    if (address < 0x2000)
    {
        if (ctx->cartridge->chr_rom)
        {
            uint32_t bank = ctx->chr_bank;
            uint32_t offset = address & 0x1FFF;
            return ctx->cartridge->chr_rom[bank * 0x2000 + offset];
        }
        else if (ctx->cartridge->chr_ram)
        {
            return ctx->cartridge->chr_ram[address];
        }
    }

    return 0;
}

static void mapper255_ppu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value)
{
    mapper255_context_t *ctx = (mapper255_context_t *)mapper;

    if (address < 0x2000 && ctx->cartridge->chr_ram)
    {
        ctx->cartridge->chr_ram[address] = value;
    }
}

static void mapper255_reset(nes_mapper_t *mapper)
{
    mapper255_context_t *ctx = (mapper255_context_t *)mapper;

    // Reset dos registradores
    ctx->prg_bank = 0;
    ctx->chr_bank = 0;
    ctx->mirror_mode = 0;
    ctx->protect = M255_PROTECT_ON; // Proteção ativada por padrão
}

nes_mapper_t *nes_mapper_255_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        log_error("Mapper 255: Cartucho inválido");
        return NULL;
    }

    mapper255_context_t *ctx = (mapper255_context_t *)malloc(sizeof(mapper255_context_t));
    if (!ctx)
    {
        log_error("Mapper 255: Falha na alocação de memória");
        return NULL;
    }

    memset(ctx, 0, sizeof(mapper255_context_t));
    ctx->cartridge = cartridge;

    // Configurar interface do mapper
    ctx->mapper.cpu_read = mapper255_cpu_read;
    ctx->mapper.cpu_write = mapper255_cpu_write;
    ctx->mapper.ppu_read = mapper255_ppu_read;
    ctx->mapper.ppu_write = mapper255_ppu_write;
    ctx->mapper.reset = mapper255_reset;
    ctx->mapper.clock = NULL;       // Não usa clock
    ctx->mapper.irq_pending = NULL; // Não usa IRQ

    // Inicialização inicial
    mapper255_reset((nes_mapper_t *)ctx);

    log_info("Mapper 255 inicializado com sucesso");
    return (nes_mapper_t *)ctx;
}
