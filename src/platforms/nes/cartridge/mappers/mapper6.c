/**
 * @file mapper6.c
 * @brief Implementação do Mapper 6 (FFE F4xxx) para o Nintendo Entertainment System
 *
 * Este mapper também é conhecido como Front Fareast (FFE) F4xxx.
 * Foi usado principalmente em jogos lançados pela FFE na Ásia.
 */

#include "mapper6.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Define categoria de log
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_MAPPER

// Estrutura de contexto para o Mapper 6
typedef struct
{
    nes_cartridge_t *cart; // Ponteiro para o cartucho

    // Registradores do Mapper 6
    uint8_t prg_bank;        // Banco de PRG-ROM ($8000-$BFFF)
    uint8_t chr_bank;        // Banco de CHR-ROM/RAM
    uint8_t prg_ram_protect; // Proteção de PRG-RAM
    uint8_t mirror_mode;     // Modo de espelhamento

    // Informações do cartucho
    uint8_t *prg_rom;     // Ponteiro para a PRG-ROM
    int prg_rom_size;     // Tamanho da PRG-ROM
    uint8_t *chr_rom;     // Ponteiro para a CHR-ROM
    int chr_rom_size;     // Tamanho da CHR-ROM
    uint8_t *chr_ram;     // Ponteiro para a CHR-RAM (se houver)
    int chr_ram_size;     // Tamanho da CHR-RAM
    uint8_t uses_chr_ram; // Flag indicando se usa CHR-RAM
} mapper6_context_t;

// Protótipos de funções
static uint8_t mapper6_cpu_read(void *ctx, uint16_t addr);
static void mapper6_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper6_chr_read(void *ctx, uint16_t addr);
static void mapper6_chr_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper6_reset(void *ctx);
static void mapper6_shutdown(void *ctx);
static void mapper6_update_banks(mapper6_context_t *ctx);

/**
 * @brief Inicializa o Mapper 6
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_6_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        EMU_LOG_ERROR("nes_mapper_6_init: cartridge é NULL");
        return NULL;
    }

    // Aloca a estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        EMU_LOG_ERROR("nes_mapper_6_init: falha ao alocar memória para o mapper");
        return NULL;
    }

    // Inicializa a estrutura do mapper
    memset(mapper, 0, sizeof(nes_mapper_t));
    mapper->mapper_number = 6;

    // Aloca o contexto do mapper
    mapper6_context_t *ctx = (mapper6_context_t *)malloc(sizeof(mapper6_context_t));
    if (!ctx)
    {
        EMU_LOG_ERROR("nes_mapper_6_init: falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }

    // Inicializa o contexto
    memset(ctx, 0, sizeof(mapper6_context_t));
    ctx->cart = cartridge;
    ctx->prg_rom = cartridge->prg_rom;
    ctx->prg_rom_size = cartridge->prg_rom_size;
    ctx->chr_rom = cartridge->chr_rom;
    ctx->chr_rom_size = cartridge->chr_rom_size;
    ctx->chr_ram = cartridge->chr_ram;
    ctx->chr_ram_size = cartridge->chr_ram_size;
    ctx->uses_chr_ram = (cartridge->chr_rom == NULL || cartridge->chr_rom_size == 0);
    ctx->mirror_mode = cartridge->mirror_mode;

    // Configuração inicial dos bancos
    ctx->prg_bank = 0;
    ctx->chr_bank = 0;
    ctx->prg_ram_protect = 0;

    // Configura as funções do mapper
    mapper->context = ctx;
    mapper->cpu_read = mapper6_cpu_read;
    mapper->cpu_write = mapper6_cpu_write;
    mapper->chr_read = mapper6_chr_read;
    mapper->chr_write = mapper6_chr_write;
    mapper->reset = mapper6_reset;
    mapper->shutdown = mapper6_shutdown;

    EMU_LOG_INFO("Mapper 6 (FFE F4xxx) inicializado: PRG-ROM=%dKB, CHR-%s=%dKB",
                 cartridge->prg_rom_size / 1024,
                 ctx->uses_chr_ram ? "RAM" : "ROM",
                 (ctx->uses_chr_ram ? cartridge->chr_ram_size : cartridge->chr_rom_size) / 1024);

    return mapper;
}

/**
 * @brief Atualiza os bancos de memória com base nos registradores
 *
 * @param ctx Contexto do mapper
 */
static void mapper6_update_banks(mapper6_context_t *ctx)
{
    // No Mapper 6, apenas atualizamos o mirror mode no cartucho
    ctx->cart->mirror_mode = ctx->mirror_mode;
}

/**
 * @brief Reset do mapper
 *
 * @param ctx Contexto do mapper
 */
static void mapper6_reset(void *ctx)
{
    mapper6_context_t *mapper_ctx = (mapper6_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper6_reset: contexto é NULL");
        return;
    }

    // Reseta os registradores
    mapper_ctx->prg_bank = 0;
    mapper_ctx->chr_bank = 0;
    mapper_ctx->prg_ram_protect = 0;
    mapper_ctx->mirror_mode = mapper_ctx->cart->mirror_mode;

    // Atualiza os bancos
    mapper6_update_banks(mapper_ctx);

    EMU_LOG_INFO("Mapper 6 resetado");
}

/**
 * @brief Desliga o mapper e libera recursos
 *
 * @param ctx Contexto do mapper
 */
static void mapper6_shutdown(void *ctx)
{
    if (!ctx)
    {
        EMU_LOG_ERROR("mapper6_shutdown: contexto é NULL");
        return;
    }

    EMU_LOG_INFO("Mapper 6 desligado");
    free(ctx);
}

/**
 * @brief Lê um byte da memória do CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido
 * @return uint8_t Valor lido
 */
static uint8_t mapper6_cpu_read(void *ctx, uint16_t addr)
{
    mapper6_context_t *mapper_ctx = (mapper6_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper6_cpu_read: contexto é NULL");
        return 0;
    }

    // $6000-$7FFF: PRG-RAM
    if (addr >= 0x6000 && addr < 0x8000)
    {
        // Verifica se há proteção de escrita na PRG-RAM
        if ((mapper_ctx->prg_ram_protect & 0x80) == 0)
        {
            return 0xFF; // RAM desabilitada, retorna pull-up
        }

        uint32_t ram_addr = addr - 0x6000;
        if (ram_addr < mapper_ctx->cart->prg_ram_size)
        {
            return mapper_ctx->cart->prg_ram[ram_addr];
        }
        return 0xFF;
    }

    // $8000-$BFFF: Banco selecionável de PRG-ROM
    if (addr >= 0x8000 && addr < 0xC000)
    {
        uint32_t bank_offset = mapper_ctx->prg_bank * 0x4000; // 16KB
        uint32_t addr_offset = addr - 0x8000;
        uint32_t rom_addr = (bank_offset + addr_offset) % mapper_ctx->prg_rom_size;
        return mapper_ctx->prg_rom[rom_addr];
    }

    // $C000-$FFFF: Último banco fixo de PRG-ROM
    if (addr >= 0xC000)
    {
        uint32_t last_bank = (mapper_ctx->prg_rom_size / 0x4000) - 1; // Último banco de 16KB
        uint32_t bank_offset = last_bank * 0x4000;
        uint32_t addr_offset = addr - 0xC000;
        uint32_t rom_addr = (bank_offset + addr_offset) % mapper_ctx->prg_rom_size;
        return mapper_ctx->prg_rom[rom_addr];
    }

    return 0;
}

/**
 * @brief Escreve um byte na memória do CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser escrito
 * @param val Valor a ser escrito
 */
static void mapper6_cpu_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper6_context_t *mapper_ctx = (mapper6_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper6_cpu_write: contexto é NULL");
        return;
    }

    // $6000-$7FFF: PRG-RAM
    if (addr >= 0x6000 && addr < 0x8000)
    {
        // Verifica se há proteção de escrita na PRG-RAM
        if ((mapper_ctx->prg_ram_protect & 0xC0) != 0x80)
        {
            // RAM protegida contra escrita ou desabilitada
            return;
        }

        uint32_t ram_addr = addr - 0x6000;
        if (ram_addr < mapper_ctx->cart->prg_ram_size)
        {
            mapper_ctx->cart->prg_ram[ram_addr] = val;
            mapper_ctx->cart->sram_dirty = 1;
        }
        return;
    }

    // $8000-$FFFF: Registradores do mapper
    if (addr >= 0x8000)
    {
        // $8000-$FFFF (espelhados): Seleção de banco e configuração
        switch (addr & 0xF000)
        {
        case 0x8000: // $8000-$8FFF: Seleciona banco PRG
            mapper_ctx->prg_bank = val & 0x0F;
            break;

        case 0x9000: // $9000-$9FFF: Controle de espelhamento
            if (val & 0x01)
            {
                mapper_ctx->mirror_mode = NES_MIRROR_HORIZONTAL;
            }
            else
            {
                mapper_ctx->mirror_mode = NES_MIRROR_VERTICAL;
            }
            break;

        case 0xA000: // $A000-$AFFF: Proteção de PRG-RAM
            mapper_ctx->prg_ram_protect = val;
            break;

        case 0xB000: // $B000-$BFFF: Seleciona banco CHR
            mapper_ctx->chr_bank = val & 0x03;
            break;
        }

        // Atualiza os bancos após qualquer escrita
        mapper6_update_banks(mapper_ctx);
    }
}

/**
 * @brief Lê um byte da memória do PPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido
 * @return uint8_t Valor lido
 */
static uint8_t mapper6_chr_read(void *ctx, uint16_t addr)
{
    mapper6_context_t *mapper_ctx = (mapper6_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper6_chr_read: contexto é NULL");
        return 0;
    }

    // Endereço fora do range válido
    if (addr >= 0x2000)
    {
        return 0;
    }

    // Se usar CHR-RAM
    if (mapper_ctx->uses_chr_ram)
    {
        return mapper_ctx->chr_ram[addr % mapper_ctx->chr_ram_size];
    }

    // Se usar CHR-ROM
    uint32_t bank_offset = mapper_ctx->chr_bank * 0x2000; // 8KB
    uint32_t addr_offset = addr;
    uint32_t chr_addr = (bank_offset + addr_offset) % mapper_ctx->chr_rom_size;
    return mapper_ctx->chr_rom[chr_addr];
}

/**
 * @brief Escreve um byte na memória do PPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser escrito
 * @param val Valor a ser escrito
 */
static void mapper6_chr_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper6_context_t *mapper_ctx = (mapper6_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper6_chr_write: contexto é NULL");
        return;
    }

    // Endereço fora do range válido
    if (addr >= 0x2000)
    {
        return;
    }

    // Escrita só é permitida em CHR-RAM
    if (mapper_ctx->uses_chr_ram)
    {
        mapper_ctx->chr_ram[addr % mapper_ctx->chr_ram_size] = val;
    }
}
