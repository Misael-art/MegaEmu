/**
 * @file mapper2.c
 * @brief Implementação do Mapper 2 (UxROM) para o NES
 *
 * O Mapper 2 (UxROM) é um dos mappers mais simples do NES, usado em jogos como Mega Man,
 * Castlevania e Metal Gear. Suas características principais são:
 * - Suporta até 256KB de PRG ROM
 * - Banco fixo de 16KB em $C000-$FFFF (último banco)
 * - Banco comutável de 16KB em $8000-$BFFF
 * - Não possui PRG RAM
 * - CHR ROM/RAM fixa de 8KB
 * - Espelhamento vertical ou horizontal fixo
 */

#include "platforms/nes/cartridge/nes_cartridge.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER2 "Mapper2"

// Macros de log específicas para o mapper
#define MAPPER2_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER2, __VA_ARGS__)
#define MAPPER2_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER2, __VA_ARGS__)
#define MAPPER2_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER2, __VA_ARGS__)
#define MAPPER2_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER2, __VA_ARGS__)
#define MAPPER2_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER2, __VA_ARGS__)

// Estrutura de contexto para o Mapper 2
typedef struct {
    nes_cartridge_t *cart;    // Ponteiro para o cartucho
    uint8_t prg_bank;         // Banco atual de PRG ROM (16KB)
    uint8_t num_prg_banks;    // Número total de bancos de 16KB
} mapper2_context_t;

// Protótipos das funções do mapper
static uint8_t mapper2_cpu_read(void *ctx, uint16_t addr);
static void mapper2_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper2_ppu_read(void *ctx, uint16_t addr);
static void mapper2_ppu_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper2_reset(void *ctx);
static void mapper2_shutdown(void *ctx);

/**
 * @brief Inicializa o Mapper 2
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para a estrutura do mapper ou NULL em caso de erro
 */
nes_mapper_t* nes_mapper_2_init(nes_cartridge_t *cartridge) {
    if (!cartridge) {
        MAPPER2_LOG_ERROR("nes_mapper_2_init: cartridge inválido");
        return NULL;
    }

    MAPPER2_LOG_INFO("Inicializando Mapper 2 (UxROM)");

    // Aloca a estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper) {
        MAPPER2_LOG_ERROR("nes_mapper_2_init: falha ao alocar mapper");
        return NULL;
    }

    // Aloca o contexto do mapper
    mapper2_context_t *ctx = (mapper2_context_t *)malloc(sizeof(mapper2_context_t));
    if (!ctx) {
        MAPPER2_LOG_ERROR("nes_mapper_2_init: falha ao alocar contexto");
        free(mapper);
        return NULL;
    }

    // Inicializa o contexto
    ctx->cart = cartridge;
    ctx->prg_bank = 0;
    ctx->num_prg_banks = cartridge->prg_rom_size / 16384; // 16KB por banco

    MAPPER2_LOG_DEBUG("Número de bancos PRG ROM: %d", ctx->num_prg_banks);

    // Configura as funções do mapper
    mapper->cpu_read = mapper2_cpu_read;
    mapper->cpu_write = mapper2_cpu_write;
    mapper->ppu_read = mapper2_ppu_read;
    mapper->ppu_write = mapper2_ppu_write;
    mapper->reset = mapper2_reset;
    mapper->shutdown = mapper2_shutdown;
    mapper->context = ctx;

    MAPPER2_LOG_INFO("Mapper 2 inicializado com sucesso");
    return mapper;
}

/**
 * @brief Lê um byte da memória mapeada pela CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido
 * @return uint8_t Valor lido
 */
static uint8_t mapper2_cpu_read(void *ctx, uint16_t addr) {
    mapper2_context_t *m2 = (mapper2_context_t *)ctx;

    if (addr >= 0x8000 && addr <= 0xBFFF) {
        // Banco comutável em $8000-$BFFF
        uint32_t prg_addr = (m2->prg_bank * 16384) + (addr - 0x8000);
        return m2->cart->prg_rom[prg_addr];
    }
    else if (addr >= 0xC000) {
        // Último banco fixo em $C000-$FFFF
        uint32_t prg_addr = ((m2->num_prg_banks - 1) * 16384) + (addr - 0xC000);
        return m2->cart->prg_rom[prg_addr];
    }

    MAPPER2_LOG_WARN("mapper2_cpu_read: endereço não mapeado: $%04X", addr);
    return 0;
}

/**
 * @brief Escreve um byte na memória mapeada pela CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser escrito
 * @param val Valor a ser escrito
 */
static void mapper2_cpu_write(void *ctx, uint16_t addr, uint8_t val) {
    mapper2_context_t *m2 = (mapper2_context_t *)ctx;

    if (addr >= 0x8000) {
        // Qualquer escrita em $8000-$FFFF seleciona o banco
        uint8_t new_bank = val & (m2->num_prg_banks - 1);
        if (new_bank != m2->prg_bank) {
            MAPPER2_LOG_DEBUG("Trocando banco PRG ROM para %d", new_bank);
            m2->prg_bank = new_bank;
        }
    }
}

/**
 * @brief Lê um byte da memória de padrões (VRAM)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido
 * @return uint8_t Valor lido
 */
static uint8_t mapper2_ppu_read(void *ctx, uint16_t addr) {
    mapper2_context_t *m2 = (mapper2_context_t *)ctx;

    if (addr < 0x2000) {
        if (m2->cart->chr_rom) {
            return m2->cart->chr_rom[addr];
        }
        else if (m2->cart->chr_ram) {
            return m2->cart->chr_ram[addr];
        }
    }

    MAPPER2_LOG_WARN("mapper2_ppu_read: endereço não mapeado: $%04X", addr);
    return 0;
}

/**
 * @brief Escreve um byte na memória de padrões (VRAM)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser escrito
 * @param val Valor a ser escrito
 */
static void mapper2_ppu_write(void *ctx, uint16_t addr, uint8_t val) {
    mapper2_context_t *m2 = (mapper2_context_t *)ctx;

    if (addr < 0x2000) {
        if (m2->cart->chr_ram) {
            m2->cart->chr_ram[addr] = val;
        }
        else {
            MAPPER2_LOG_WARN("mapper2_ppu_write: tentativa de escrita em CHR ROM: $%04X = $%02X", addr, val);
        }
    }
}

/**
 * @brief Reseta o mapper para seu estado inicial
 *
 * @param ctx Contexto do mapper
 */
static void mapper2_reset(void *ctx) {
    mapper2_context_t *m2 = (mapper2_context_t *)ctx;

    MAPPER2_LOG_INFO("Resetando Mapper 2");

    // No reset, seleciona o banco 0
    m2->prg_bank = 0;
}

/**
 * @brief Finaliza o mapper e libera recursos
 *
 * @param ctx Contexto do mapper
 */
static void mapper2_shutdown(void *ctx) {
    if (ctx) {
        MAPPER2_LOG_INFO("Finalizando Mapper 2");
        free(ctx);
    }
}
