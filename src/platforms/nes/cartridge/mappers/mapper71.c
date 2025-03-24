/**
 * @file mapper71.c
 * @brief Implementação do Mapper 71 (Camerica)
 *
 * O Mapper 71 é um mapper simples usado pela Camerica/Codemasters.
 * Características:
 * - Suporta até 256KB de PRG-ROM
 * - Não tem CHR-ROM, usa 8KB de CHR-RAM
 * - Não tem PRG-RAM
 * - Bancos de 16KB em $8000-$BFFF
 * - Banco fixo em $C000-$FFFF (último banco)
 */

#include "platforms/nes/cartridge/mappers/mapper71.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 71
#define MAPPER71_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER71_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER71_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER71_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER71_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Estrutura de contexto do mapper
typedef struct {
    nes_cartridge_t* cart;    // Ponteiro para o cartucho
    uint8_t prg_bank;         // Banco atual de PRG-ROM
} mapper71_context_t;

// Funções de acesso à memória
static uint8_t mapper71_cpu_read(void* ctx, uint16_t addr);
static void mapper71_cpu_write(void* ctx, uint16_t addr, uint8_t val);
static uint8_t mapper71_ppu_read(void* ctx, uint16_t addr);
static void mapper71_ppu_write(void* ctx, uint16_t addr, uint8_t val);
static void mapper71_reset(void* ctx);
static void mapper71_shutdown(void* ctx);

/**
 * @brief Inicializa o Mapper 71
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t* nes_mapper_71_init(nes_cartridge_t* cartridge) {
    if (!cartridge) {
        MAPPER71_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER71_LOG_INFO("Inicializando Mapper 71 (Camerica)");

    // Aloca estrutura do mapper
    nes_mapper_t* mapper = (nes_mapper_t*)malloc(sizeof(nes_mapper_t));
    if (!mapper) {
        MAPPER71_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto do mapper
    mapper71_context_t* ctx = (mapper71_context_t*)malloc(sizeof(mapper71_context_t));
    if (!ctx) {
        MAPPER71_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(ctx, 0, sizeof(mapper71_context_t));

    // Inicializa o contexto
    ctx->cart = cartridge;
    ctx->prg_bank = 0;

    // Configura o mapper
    mapper->number = 71;
    mapper->name = "Camerica";
    mapper->context = ctx;
    mapper->cpu_read = mapper71_cpu_read;
    mapper->cpu_write = mapper71_cpu_write;
    mapper->ppu_read = mapper71_ppu_read;
    mapper->ppu_write = mapper71_ppu_write;
    mapper->reset = mapper71_reset;
    mapper->shutdown = mapper71_shutdown;

    MAPPER71_LOG_INFO("Mapper 71 (Camerica) inicializado com sucesso");
    return mapper;
}

/**
 * @brief Lê um byte da memória (CPU)
 */
static uint8_t mapper71_cpu_read(void* ctx, uint16_t addr) {
    mapper71_context_t* m71 = (mapper71_context_t*)ctx;

    if (addr >= 0x8000 && addr <= 0xBFFF) {
        // Banco selecionável em $8000-$BFFF
        uint32_t offset = ((uint32_t)m71->prg_bank * 0x4000) + (addr & 0x3FFF);
        return m71->cart->prg_rom[offset % m71->cart->prg_rom_size];
    }
    else if (addr >= 0xC000) {
        // Banco fixo em $C000-$FFFF (último banco)
        uint32_t last_bank = (m71->cart->prg_rom_size - 0x4000);
        uint32_t offset = last_bank + (addr & 0x3FFF);
        return m71->cart->prg_rom[offset % m71->cart->prg_rom_size];
    }

    return 0;
}

/**
 * @brief Escreve um byte na memória (CPU)
 */
static void mapper71_cpu_write(void* ctx, uint16_t addr, uint8_t val) {
    mapper71_context_t* m71 = (mapper71_context_t*)ctx;

    if (addr >= 0xC000 && addr <= 0xFFFF) {
        // Registrador de seleção de banco em $C000-$FFFF
        m71->prg_bank = val & 0x0F; // 16 bancos possíveis
        MAPPER71_LOG_DEBUG("Trocando banco PRG ROM para %d", m71->prg_bank);
    }
}

/**
 * @brief Lê um byte da memória de padrões (PPU)
 */
static uint8_t mapper71_ppu_read(void* ctx, uint16_t addr) {
    mapper71_context_t* m71 = (mapper71_context_t*)ctx;

    if (addr < 0x2000) {
        // Mapper 71 usa CHR-RAM
        if (m71->cart->chr_ram) {
            return m71->cart->chr_ram[addr];
        }
    }

    return 0;
}

/**
 * @brief Escreve um byte na memória de padrões (PPU)
 */
static void mapper71_ppu_write(void* ctx, uint16_t addr, uint8_t val) {
    mapper71_context_t* m71 = (mapper71_context_t*)ctx;

    if (addr < 0x2000) {
        // Mapper 71 usa CHR-RAM
        if (m71->cart->chr_ram) {
            m71->cart->chr_ram[addr] = val;
        }
    }
}

/**
 * @brief Reseta o mapper
 */
static void mapper71_reset(void* ctx) {
    mapper71_context_t* m71 = (mapper71_context_t*)ctx;
    m71->prg_bank = 0;
    MAPPER71_LOG_INFO("Resetando Mapper 71 (Camerica)");
}

/**
 * @brief Finaliza o mapper
 */
static void mapper71_shutdown(void* ctx) {
    if (ctx) {
        MAPPER71_LOG_INFO("Desligando Mapper 71 (Camerica)");
        free(ctx);
    }
}
