/**
 * @file mapper11.c
 * @brief Implementação do Mapper 11 (Color Dreams) para NES
 */

#include "platforms/nes/cartridge/mappers/mapper11.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 11
#define MAPPER11_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER11_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER11_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER11_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER11_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Estrutura do contexto do mapper
typedef struct
{
    nes_cartridge_t *cart; // Ponteiro para o cartucho
    uint8_t chr_bank;      // Banco atual de CHR-ROM
} mapper11_context_t;

// Protótipos de funções
static uint8_t mapper11_cpu_read(void *context, uint16_t address);
static void mapper11_cpu_write(void *context, uint16_t address, uint8_t value);
static uint8_t mapper11_ppu_read(void *context, uint16_t address);
static void mapper11_ppu_write(void *context, uint16_t address, uint8_t value);
static void mapper11_reset(void *context);
static void mapper11_shutdown(void *context);

/**
 * @brief Inicializa o Mapper 11 (Color Dreams)
 */
nes_mapper_t *nes_mapper_11_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER11_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER11_LOG_INFO("Inicializando Mapper 11 (Color Dreams)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER11_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto específico do mapper
    mapper11_context_t *context = (mapper11_context_t *)malloc(sizeof(mapper11_context_t));
    if (!context)
    {
        MAPPER11_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(context, 0, sizeof(mapper11_context_t));

    // Configura o contexto
    context->cart = cartridge;
    context->chr_bank = 0;

    // Configura o mapper
    mapper->number = 11;
    mapper->name = "Color Dreams";
    mapper->context = context;
    mapper->cpu_read = mapper11_cpu_read;
    mapper->cpu_write = mapper11_cpu_write;
    mapper->ppu_read = mapper11_ppu_read;
    mapper->ppu_write = mapper11_ppu_write;
    mapper->reset = mapper11_reset;
    mapper->shutdown = mapper11_shutdown;

    MAPPER11_LOG_INFO("Mapper 11 (Color Dreams) inicializado com sucesso");
    return mapper;
}

/**
 * @brief Lê um byte da memória (acesso da CPU)
 */
static uint8_t mapper11_cpu_read(void *context, uint16_t address)
{
    mapper11_context_t *ctx = (mapper11_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address >= 0x8000 && address <= 0xFFFF)
    {
        // PRG-ROM é fixo em um banco de 32KB
        uint32_t offset = address - 0x8000;
        if (offset < cart->prg_rom_size)
        {
            return cart->prg_rom[offset];
        }
    }

    return 0;
}

/**
 * @brief Escreve um byte na memória (acesso da CPU)
 */
static void mapper11_cpu_write(void *context, uint16_t address, uint8_t value)
{
    mapper11_context_t *ctx = (mapper11_context_t *)context;

    if (address >= 0x8000 && address <= 0xFFFF)
    {
        // Qualquer escrita em $8000-$FFFF seleciona o banco de CHR-ROM
        ctx->chr_bank = value & 0x0F;
        MAPPER11_LOG_DEBUG("Banco CHR alterado para %d", ctx->chr_bank);
    }
}

/**
 * @brief Lê um byte da CHR ROM/RAM
 */
static uint8_t mapper11_ppu_read(void *context, uint16_t address)
{
    mapper11_context_t *ctx = (mapper11_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF)
    {
        if (cart->chr_rom)
        {
            uint32_t offset = (ctx->chr_bank * 0x2000) + address;
            if (offset < cart->chr_rom_size)
            {
                return cart->chr_rom[offset];
            }
        }
        else if (cart->chr_ram)
        {
            return cart->chr_ram[address];
        }
    }

    return 0;
}

/**
 * @brief Escreve um byte na CHR RAM
 */
static void mapper11_ppu_write(void *context, uint16_t address, uint8_t value)
{
    mapper11_context_t *ctx = (mapper11_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF && cart->chr_ram)
    {
        cart->chr_ram[address] = value;
    }
}

/**
 * @brief Reseta o mapper para seu estado inicial
 */
static void mapper11_reset(void *context)
{
    mapper11_context_t *ctx = (mapper11_context_t *)context;

    MAPPER11_LOG_INFO("Resetando Mapper 11 (Color Dreams)");
    ctx->chr_bank = 0;
}

/**
 * @brief Desliga o mapper e libera recursos
 */
static void mapper11_shutdown(void *context)
{
    if (context)
    {
        MAPPER11_LOG_INFO("Desligando Mapper 11 (Color Dreams)");
        free(context);
    }
}
