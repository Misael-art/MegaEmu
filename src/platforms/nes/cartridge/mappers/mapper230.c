/**
 * @file mapper230.c
 * @brief Implementação do Mapper 230 (22-in-1)
 */

#include <stdlib.h>
#include <string.h>
#include "mapper230.h"
#include "../../log.h"

// Estrutura de contexto do Mapper 230
typedef struct {
    nes_mapper_t mapper;           // Interface do mapper
    nes_cartridge_t* cartridge;    // Ponteiro para o cartucho

    uint8_t prg_bank;             // Banco atual de PRG
    uint8_t chr_bank;             // Banco atual de CHR
    uint8_t mirror_mode;          // Modo de espelhamento
} mapper230_context_t;

static uint8_t mapper230_cpu_read(nes_mapper_t* mapper, uint16_t address) {
    mapper230_context_t* ctx = (mapper230_context_t*)mapper;

    if (address >= 0x8000) {
        uint32_t bank = ctx->prg_bank;
        uint32_t offset = address & 0x7FFF;
        return ctx->cartridge->prg_rom[bank * 0x8000 + offset];
    }

    return 0;
}

static void mapper230_cpu_write(nes_mapper_t* mapper, uint16_t address, uint8_t value) {
    mapper230_context_t* ctx = (mapper230_context_t*)mapper;

    if (address >= 0x8000) {
        // O registrador de banco é dividido em:
        // Bits 0-3: Seleção de banco PRG
        // Bits 4-5: Seleção de banco CHR
        // Bit 6: Modo de espelhamento (0 = vertical, 1 = horizontal)
        ctx->prg_bank = value & 0x0F;
        ctx->chr_bank = (value >> 4) & 0x03;
        ctx->mirror_mode = (value >> 6) & 0x01;
    }
}

static uint8_t mapper230_ppu_read(nes_mapper_t* mapper, uint16_t address) {
    mapper230_context_t* ctx = (mapper230_context_t*)mapper;

    if (address < 0x2000) {
        uint32_t bank = ctx->chr_bank;
        uint32_t offset = address & 0x1FFF;
        return ctx->cartridge->chr_rom[bank * 0x2000 + offset];
    }

    return 0;
}

static void mapper230_ppu_write(nes_mapper_t* mapper, uint16_t address, uint8_t value) {
    // O Mapper 230 não suporta escrita em CHR
    (void)mapper;
    (void)address;
    (void)value;
}

static void mapper230_reset(nes_mapper_t* mapper) {
    mapper230_context_t* ctx = (mapper230_context_t*)mapper;

    // Reset dos registradores
    ctx->prg_bank = 0;
    ctx->chr_bank = 0;
    ctx->mirror_mode = 0;
}

nes_mapper_t* nes_mapper_230_init(nes_cartridge_t* cartridge) {
    if (!cartridge) {
        log_error("Mapper 230: Cartucho inválido");
        return NULL;
    }

    mapper230_context_t* ctx = (mapper230_context_t*)malloc(sizeof(mapper230_context_t));
    if (!ctx) {
        log_error("Mapper 230: Falha na alocação de memória");
        return NULL;
    }

    memset(ctx, 0, sizeof(mapper230_context_t));
    ctx->cartridge = cartridge;

    // Configurar interface do mapper
    ctx->mapper.cpu_read = mapper230_cpu_read;
    ctx->mapper.cpu_write = mapper230_cpu_write;
    ctx->mapper.ppu_read = mapper230_ppu_read;
    ctx->mapper.ppu_write = mapper230_ppu_write;
    ctx->mapper.reset = mapper230_reset;
    ctx->mapper.clock = NULL;          // Não usa clock
    ctx->mapper.irq_pending = NULL;    // Não usa IRQ

    // Inicialização inicial
    mapper230_reset((nes_mapper_t*)ctx);

    log_info("Mapper 230 inicializado com sucesso");
    return (nes_mapper_t*)ctx;
}
