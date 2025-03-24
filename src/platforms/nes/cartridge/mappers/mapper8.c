/**
 * @file mapper8.c
 * @brief Implementação do Mapper 8 (FFE F3xxx) para NES
 *
 * O Mapper 8 (FFE F3xxx) é um mapper relativamente simples usado em jogos
 * da Capcom. Características principais:
 * - PRG-ROM: Bancos de 16KB ou 32KB
 * - CHR-ROM: Bancos de 8KB
 * - Espelhamento controlado pelo cartucho (horizontal ou vertical)
 * - Usado principalmente em jogos da Capcom
 */

#include "platforms/nes/cartridge/nes_cartridge.h"
#include "common/logging.h"
#include <stdlib.h>
#include <string.h>

#define MAPPER8_LOG_INFO(...) EMU_LOG_INFO("[MAPPER8]", __VA_ARGS__)
#define MAPPER8_LOG_ERROR(...) EMU_LOG_ERROR("[MAPPER8]", __VA_ARGS__)
#define MAPPER8_LOG_DEBUG(...) EMU_LOG_DEBUG("[MAPPER8]", __VA_ARGS__)
#define MAPPER8_LOG_WARN(...) EMU_LOG_WARN("[MAPPER8]", __VA_ARGS__)

// Contexto específico do Mapper 8
typedef struct
{
    uint8_t *prg_rom;
    uint32_t prg_rom_size;
    uint8_t *chr_rom;
    uint32_t chr_rom_size;
    uint8_t *prg_ram;
    uint32_t prg_ram_size;
    uint8_t *chr_ram;
    uint32_t chr_ram_size;

    uint8_t prg_bank; // Banco PRG atual
    uint8_t chr_bank; // Banco CHR atual
    uint8_t mirror_mode;

    // Número total de bancos PRG e CHR
    uint8_t prg_banks;
    uint8_t chr_banks;
} mapper8_context_t;

// Protótipos de funções
static uint8_t mapper8_cpu_read(void *context, uint16_t address);
static void mapper8_cpu_write(void *context, uint16_t address, uint8_t value);
static uint8_t mapper8_ppu_read(void *context, uint16_t address);
static void mapper8_ppu_write(void *context, uint16_t address, uint8_t value);
static void mapper8_reset(void *context);
static void mapper8_shutdown(void *context);

/**
 * @brief Inicializa o Mapper 8
 * @param cartridge Ponteiro para a estrutura do cartucho
 * @return Ponteiro para a estrutura do mapper ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_8_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER8_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER8_LOG_INFO("Inicializando Mapper 8 (FFE F3xxx)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER8_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto específico do mapper
    mapper8_context_t *context = (mapper8_context_t *)malloc(sizeof(mapper8_context_t));
    if (!context)
    {
        MAPPER8_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(context, 0, sizeof(mapper8_context_t));

    // Configura o contexto
    context->prg_rom = cartridge->prg_rom;
    context->prg_rom_size = cartridge->prg_rom_size;
    context->chr_rom = cartridge->chr_rom;
    context->chr_rom_size = cartridge->chr_rom_size;
    context->prg_ram = cartridge->prg_ram;
    context->prg_ram_size = cartridge->prg_ram_size;
    context->chr_ram = cartridge->chr_ram;
    context->chr_ram_size = cartridge->chr_ram_size;
    context->mirror_mode = cartridge->mirror_mode;

    // Calcula o número de bancos
    context->prg_banks = context->prg_rom_size / 16384;
    context->chr_banks = (context->chr_rom_size > 0) ? (context->chr_rom_size / 8192) : 0;

    MAPPER8_LOG_DEBUG("PRG-ROM: %d KB (%d bancos de 16KB)", context->prg_rom_size / 1024, context->prg_banks);

    if (context->chr_rom_size > 0)
    {
        MAPPER8_LOG_DEBUG("CHR-ROM: %d KB (%d bancos de 8KB)", context->chr_rom_size / 1024, context->chr_banks);
    }
    else if (context->chr_ram_size > 0)
    {
        MAPPER8_LOG_DEBUG("CHR-RAM: %d KB", context->chr_ram_size / 1024);
    }

    // Inicializa os bancos
    context->prg_bank = 0;
    context->chr_bank = 0;

    // Configura o mapper
    mapper->number = 8;
    mapper->name = "FFE F3xxx";
    mapper->context = context;
    mapper->cpu_read = mapper8_cpu_read;
    mapper->cpu_write = mapper8_cpu_write;
    mapper->ppu_read = mapper8_ppu_read;
    mapper->ppu_write = mapper8_ppu_write;
    mapper->reset = mapper8_reset;
    mapper->shutdown = mapper8_shutdown;

    MAPPER8_LOG_INFO("Mapper 8 inicializado com sucesso");
    return mapper;
}

/**
 * @brief Manipula leituras da CPU
 * @param context Contexto do mapper
 * @param address Endereço a ser lido (0x0000-0xFFFF)
 * @return Byte lido do endereço
 */
static uint8_t mapper8_cpu_read(void *context, uint16_t address)
{
    mapper8_context_t *ctx = (mapper8_context_t *)context;

    // PRG-RAM ($6000-$7FFF)
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        if (ctx->prg_ram && ctx->prg_ram_size > 0)
        {
            return ctx->prg_ram[address - 0x6000];
        }
        return 0xFF;
    }

    // PRG-ROM ($8000-$FFFF)
    if (address >= 0x8000 && address <= 0xFFFF)
    {
        // No Mapper 8, o banco é selecionado para toda a PRG-ROM
        uint32_t prg_addr = ((uint32_t)ctx->prg_bank * 32768) + (address - 0x8000);

        // Garante que não ultrapassamos o tamanho da PRG-ROM
        prg_addr %= ctx->prg_rom_size;

        return ctx->prg_rom[prg_addr];
    }

    return 0;
}

/**
 * @brief Manipula escritas da CPU
 * @param context Contexto do mapper
 * @param address Endereço a ser escrito (0x0000-0xFFFF)
 * @param value Valor a ser escrito
 */
static void mapper8_cpu_write(void *context, uint16_t address, uint8_t value)
{
    mapper8_context_t *ctx = (mapper8_context_t *)context;

    // PRG-RAM ($6000-$7FFF)
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        if (ctx->prg_ram && ctx->prg_ram_size > 0)
        {
            ctx->prg_ram[address - 0x6000] = value;
        }
        return;
    }

    // Registros de controle ($8000-$FFFF)
    if (address >= 0x8000 && address <= 0xFFFF)
    {
        // Seleciona o banco PRG ($8000-$AFFF)
        if (address >= 0x8000 && address <= 0xAFFF)
        {
            ctx->prg_bank = value & 0x0F; // Apenas os 4 bits inferiores são usados
            MAPPER8_LOG_DEBUG("Banco PRG selecionado: %d", ctx->prg_bank);
        }
        // Seleciona o banco CHR ($B000-$DFFF)
        else if (address >= 0xB000 && address <= 0xDFFF)
        {
            ctx->chr_bank = value & 0x0F; // Apenas os 4 bits inferiores são usados
            MAPPER8_LOG_DEBUG("Banco CHR selecionado: %d", ctx->chr_bank);
        }
    }
}

/**
 * @brief Manipula leituras da PPU
 * @param context Contexto do mapper
 * @param address Endereço a ser lido (0x0000-0x1FFF)
 * @return Byte lido do endereço
 */
static uint8_t mapper8_ppu_read(void *context, uint16_t address)
{
    mapper8_context_t *ctx = (mapper8_context_t *)context;

    // Pattern Tables ($0000-$1FFF)
    if (address <= 0x1FFF)
    {
        if (ctx->chr_rom && ctx->chr_rom_size > 0)
        {
            // Aplica o banco selecionado
            uint32_t chr_addr = ((uint32_t)ctx->chr_bank * 8192) + address;

            // Garante que não ultrapassamos o tamanho da CHR-ROM
            chr_addr %= ctx->chr_rom_size;

            return ctx->chr_rom[chr_addr];
        }
        else if (ctx->chr_ram && ctx->chr_ram_size > 0)
        {
            return ctx->chr_ram[address];
        }
    }

    return 0;
}

/**
 * @brief Manipula escritas da PPU
 * @param context Contexto do mapper
 * @param address Endereço a ser escrito (0x0000-0x1FFF)
 * @param value Valor a ser escrito
 */
static void mapper8_ppu_write(void *context, uint16_t address, uint8_t value)
{
    mapper8_context_t *ctx = (mapper8_context_t *)context;

    // Pattern Tables ($0000-$1FFF) - Escreve apenas se houver CHR-RAM
    if (address <= 0x1FFF)
    {
        if (ctx->chr_ram && ctx->chr_ram_size > 0)
        {
            ctx->chr_ram[address] = value;
        }
    }
}

/**
 * @brief Reseta o mapper para seu estado inicial
 * @param context Contexto do mapper
 */
static void mapper8_reset(void *context)
{
    mapper8_context_t *ctx = (mapper8_context_t *)context;

    // Reinicia o estado do mapper
    ctx->prg_bank = 0;
    ctx->chr_bank = 0;

    MAPPER8_LOG_INFO("Mapper 8 resetado");
}

/**
 * @brief Desliga o mapper e libera recursos
 * @param context Contexto do mapper
 */
static void mapper8_shutdown(void *context)
{
    if (context)
    {
        MAPPER8_LOG_INFO("Desligando Mapper 8");
        free(context);
    }
}
