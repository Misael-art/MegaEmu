/**
 * @file mapper7.c
 * @brief Implementação do Mapper 7 (AxROM) para o Nintendo Entertainment System
 *
 * O Mapper 7 (AxROM) caracteriza-se por:
 * - Bancos de 32KB mapeados em $8000-$FFFF
 * - Espelhamento de nametable single-screen selecionável
 * - Não utiliza CHR-ROM, apenas CHR-RAM (padrão de 8KB)
 * - Utilizado em jogos como Battletoads, Marble Madness, Wizards & Warriors
 */

#include "mapper7.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Define categoria de log
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_MAPPER

// Estrutura de contexto para o Mapper 7
typedef struct
{
    nes_cartridge_t *cart; // Ponteiro para o cartucho

    // Registradores do Mapper 7
    uint8_t prg_bank;         // Banco PRG selecionado (32KB)
    uint8_t nametable_select; // Seleção de nametable (0 ou 1)

    // Informações do cartucho
    uint8_t *prg_rom; // Ponteiro para a PRG-ROM
    int prg_rom_size; // Tamanho da PRG-ROM
    uint8_t *chr_ram; // Ponteiro para a CHR-RAM
    int chr_ram_size; // Tamanho da CHR-RAM
} mapper7_context_t;

// Protótipos de funções
static uint8_t mapper7_cpu_read(void *ctx, uint16_t addr);
static void mapper7_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper7_chr_read(void *ctx, uint16_t addr);
static void mapper7_chr_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper7_reset(void *ctx);
static void mapper7_shutdown(void *ctx);

/**
 * @brief Inicializa o Mapper 7
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_7_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        EMU_LOG_ERROR("nes_mapper_7_init: cartridge é NULL");
        return NULL;
    }

    // Aloca a estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        EMU_LOG_ERROR("nes_mapper_7_init: falha ao alocar memória para o mapper");
        return NULL;
    }

    // Inicializa a estrutura do mapper
    memset(mapper, 0, sizeof(nes_mapper_t));
    mapper->mapper_number = 7;

    // Aloca o contexto do mapper
    mapper7_context_t *ctx = (mapper7_context_t *)malloc(sizeof(mapper7_context_t));
    if (!ctx)
    {
        EMU_LOG_ERROR("nes_mapper_7_init: falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }

    // Inicializa o contexto
    memset(ctx, 0, sizeof(mapper7_context_t));
    ctx->cart = cartridge;
    ctx->prg_rom = cartridge->prg_rom;
    ctx->prg_rom_size = cartridge->prg_rom_size;
    ctx->chr_ram = cartridge->chr_ram;
    ctx->chr_ram_size = cartridge->chr_ram_size;

    // Verifica se há CHR-RAM, importante para o Mapper 7
    if (!ctx->chr_ram || ctx->chr_ram_size == 0)
    {
        EMU_LOG_WARN("nes_mapper_7_init: CHR-RAM não encontrada ou tamanho zero");
        if (cartridge->chr_rom)
        {
            EMU_LOG_WARN("nes_mapper_7_init: CHR-ROM detectada, mas Mapper 7 usa apenas CHR-RAM");
        }
    }

    // Configuração inicial (banco 0, nametable 0)
    ctx->prg_bank = 0;
    ctx->nametable_select = 0;

    // Atualiza o modo de espelhamento (single-screen com nametable 0)
    cartridge->mirror_mode = NES_MIRROR_SINGLE_SCREEN_NT0;

    // Configura as funções do mapper
    mapper->context = ctx;
    mapper->cpu_read = mapper7_cpu_read;
    mapper->cpu_write = mapper7_cpu_write;
    mapper->chr_read = mapper7_chr_read;
    mapper->chr_write = mapper7_chr_write;
    mapper->reset = mapper7_reset;
    mapper->shutdown = mapper7_shutdown;

    EMU_LOG_INFO("Mapper 7 (AxROM) inicializado: PRG-ROM=%dKB, CHR-RAM=%dKB",
                 cartridge->prg_rom_size / 1024,
                 cartridge->chr_ram_size / 1024);

    return mapper;
}

/**
 * @brief Reset do mapper
 *
 * @param ctx Contexto do mapper
 */
static void mapper7_reset(void *ctx)
{
    mapper7_context_t *mapper_ctx = (mapper7_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper7_reset: contexto é NULL");
        return;
    }

    // Reseta os registradores para valores padrão
    mapper_ctx->prg_bank = 0;
    mapper_ctx->nametable_select = 0;

    // Atualiza o modo de espelhamento
    mapper_ctx->cart->mirror_mode = NES_MIRROR_SINGLE_SCREEN_NT0;

    EMU_LOG_INFO("Mapper 7 resetado");
}

/**
 * @brief Desliga o mapper e libera recursos
 *
 * @param ctx Contexto do mapper
 */
static void mapper7_shutdown(void *ctx)
{
    if (!ctx)
    {
        EMU_LOG_ERROR("mapper7_shutdown: contexto é NULL");
        return;
    }

    EMU_LOG_INFO("Mapper 7 desligado");
    free(ctx);
}

/**
 * @brief Lê um byte da memória do CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido
 * @return uint8_t Valor lido
 */
static uint8_t mapper7_cpu_read(void *ctx, uint16_t addr)
{
    mapper7_context_t *mapper_ctx = (mapper7_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper7_cpu_read: contexto é NULL");
        return 0;
    }

    // $6000-$7FFF: PRG-RAM
    if (addr >= 0x6000 && addr < 0x8000)
    {
        uint32_t ram_addr = addr - 0x6000;
        if (ram_addr < mapper_ctx->cart->prg_ram_size)
        {
            return mapper_ctx->cart->prg_ram[ram_addr];
        }
        return 0xFF; // Pull-up se não houver RAM
    }

    // $8000-$FFFF: Banco de 32KB selecionado
    if (addr >= 0x8000)
    {
        uint32_t bank_offset = mapper_ctx->prg_bank * 0x8000; // 32KB
        uint32_t addr_offset = addr - 0x8000;
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
static void mapper7_cpu_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper7_context_t *mapper_ctx = (mapper7_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper7_cpu_write: contexto é NULL");
        return;
    }

    // $6000-$7FFF: PRG-RAM
    if (addr >= 0x6000 && addr < 0x8000)
    {
        uint32_t ram_addr = addr - 0x6000;
        if (ram_addr < mapper_ctx->cart->prg_ram_size)
        {
            mapper_ctx->cart->prg_ram[ram_addr] = val;
            mapper_ctx->cart->sram_dirty = 1;
        }
        return;
    }

    // $8000-$FFFF: Escrita para selecionar banco e nametable
    if (addr >= 0x8000)
    {
        // Os registradores são mapeados em todo o range $8000-$FFFF
        // bit 0-2: Seleção de banco PRG (0-7)
        // bit 4: Seleção de nametable (0=primeira tela, 1=segunda tela)
        mapper_ctx->prg_bank = val & 0x07;                // 3 bits de seleção de banco
        mapper_ctx->nametable_select = (val >> 4) & 0x01; // Bit 4

        // Atualiza o modo de espelhamento com base no nametable selecionado
        if (mapper_ctx->nametable_select == 0)
        {
            mapper_ctx->cart->mirror_mode = NES_MIRROR_SINGLE_SCREEN_NT0;
        }
        else
        {
            mapper_ctx->cart->mirror_mode = NES_MIRROR_SINGLE_SCREEN_NT1;
        }

        EMU_LOG_DEBUG("Mapper 7: selecionado banco PRG %d e nametable %d",
                      mapper_ctx->prg_bank, mapper_ctx->nametable_select);
    }
}

/**
 * @brief Lê um byte da memória do PPU (CHR-RAM)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser lido
 * @return uint8_t Valor lido
 */
static uint8_t mapper7_chr_read(void *ctx, uint16_t addr)
{
    mapper7_context_t *mapper_ctx = (mapper7_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper7_chr_read: contexto é NULL");
        return 0;
    }

    // Endereço fora do range válido
    if (addr >= 0x2000)
    {
        return 0;
    }

    // Mapper 7 usa CHR-RAM
    if (mapper_ctx->chr_ram && mapper_ctx->chr_ram_size > 0)
    {
        return mapper_ctx->chr_ram[addr % mapper_ctx->chr_ram_size];
    }

    EMU_LOG_WARN("mapper7_chr_read: CHR-RAM não disponível");
    return 0;
}

/**
 * @brief Escreve um byte na memória do PPU (CHR-RAM)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço a ser escrito
 * @param val Valor a ser escrito
 */
static void mapper7_chr_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper7_context_t *mapper_ctx = (mapper7_context_t *)ctx;
    if (!mapper_ctx)
    {
        EMU_LOG_ERROR("mapper7_chr_write: contexto é NULL");
        return;
    }

    // Endereço fora do range válido
    if (addr >= 0x2000)
    {
        return;
    }

    // Mapper 7 permite escrita na CHR-RAM
    if (mapper_ctx->chr_ram && mapper_ctx->chr_ram_size > 0)
    {
        mapper_ctx->chr_ram[addr % mapper_ctx->chr_ram_size] = val;
    }
    else
    {
        EMU_LOG_WARN("mapper7_chr_write: CHR-RAM não disponível");
    }
}
