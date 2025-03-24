/**
 * @file mapper5.c
 * @brief Implementação do Mapper 5 (MMC5) para NES
 *
 * O Mapper 5 (MMC5) é um dos mappers mais complexos do NES, usado em jogos como
 * Castlevania III. Este mapper oferece recursos avançados como:
 * - Bancos de PRG-ROM flexíveis (8KB/16KB/32KB)
 * - Bancos de CHR-ROM flexíveis (1KB/2KB/4KB/8KB)
 * - ExRAM de 1KB para nametables e atributos
 * - Multiplicador de hardware 8x8
 * - IRQ baseado em scanline
 * - Modos de espelhamento avançados
 * - Split screen e efeitos de tela
 */

#include "platforms/nes/cartridge/mappers/mapper5.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 5
#define MAPPER5_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER5_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER5_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER5_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER5_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Registradores do MMC5
#define MMC5_PRG_MODE 0x5100
#define MMC5_CHR_MODE 0x5101
#define MMC5_PRG_RAM_PROTECT 0x5102
#define MMC5_EXRAM_MODE 0x5104
#define MMC5_NAMETABLE_MODE 0x5105
#define MMC5_FILL_TILE 0x5106
#define MMC5_FILL_ATTR 0x5107
#define MMC5_PRG_BANK_0 0x5113
#define MMC5_PRG_BANK_1 0x5114
#define MMC5_PRG_BANK_2 0x5115
#define MMC5_PRG_BANK_3 0x5116
#define MMC5_PRG_BANK_4 0x5117
#define MMC5_CHR_BANK_0 0x5120
#define MMC5_CHR_BANK_1 0x5121
#define MMC5_CHR_BANK_2 0x5122
#define MMC5_CHR_BANK_3 0x5123
#define MMC5_CHR_BANK_4 0x5124
#define MMC5_CHR_BANK_5 0x5125
#define MMC5_CHR_BANK_6 0x5126
#define MMC5_CHR_BANK_7 0x5127
#define MMC5_CHR_BANK_8 0x5128
#define MMC5_CHR_BANK_9 0x5129
#define MMC5_CHR_BANK_10 0x512A
#define MMC5_CHR_BANK_11 0x512B
#define MMC5_IRQ_SCANLINE 0x5203
#define MMC5_IRQ_STATUS 0x5204
#define MMC5_MULT_A 0x5205
#define MMC5_MULT_B 0x5206

// Estrutura do mapper MMC5
typedef struct
{
    nes_cartridge_t *cart; // Ponteiro para o cartucho
    void *cpu;             // Ponteiro para a CPU (para IRQs)

    // Registradores de controle
    uint8_t prg_mode;       // Modo de PRG-ROM (0-3)
    uint8_t chr_mode;       // Modo de CHR-ROM (0-3)
    uint8_t exram_mode;     // Modo de ExRAM (0-3)
    uint8_t nametable_mode; // Modo de mapeamento de nametable

    // Bank switching
    uint8_t prg_banks[5];  // Registradores de PRG bank
    uint8_t chr_banks[12]; // Registradores de CHR bank

    // ExRAM
    uint8_t exram[1024]; // Expansion RAM de 1KB

    // IRQ
    uint8_t irq_scanline; // Valor de scanline para IRQ
    uint8_t irq_enable;   // Flag de habilitação de IRQ
    uint8_t irq_status;   // Status de IRQ

    // Multiplicador de hardware
    uint8_t mult_a;       // Multiplicador A
    uint8_t mult_b;       // Multiplicador B
    uint16_t mult_result; // Resultado da multiplicação

    // Estado do PPU
    uint16_t last_ppu_addr; // Último endereço acessado pelo PPU
    int in_frame;           // Flag indicando rendering ativo
} mapper5_context_t;

// Protótipos de funções
static uint8_t mapper5_cpu_read(void *context, uint16_t address);
static void mapper5_cpu_write(void *context, uint16_t address, uint8_t value);
static uint8_t mapper5_ppu_read(void *context, uint16_t address);
static void mapper5_ppu_write(void *context, uint16_t address, uint8_t value);
static void mapper5_scanline(void *context);
static void mapper5_reset(void *context);
static void mapper5_shutdown(void *context);

/**
 * @brief Inicializa o Mapper 5 (MMC5)
 * @param cartridge Ponteiro para o cartucho
 * @return Ponteiro para o mapper inicializado ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_5_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER5_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER5_LOG_INFO("Inicializando Mapper 5 (MMC5)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER5_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto específico do mapper
    mapper5_context_t *context = (mapper5_context_t *)malloc(sizeof(mapper5_context_t));
    if (!context)
    {
        MAPPER5_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(context, 0, sizeof(mapper5_context_t));

    // Configura o contexto
    context->cart = cartridge;
    context->prg_mode = 3; // Modo 8KB por padrão
    context->chr_mode = 3; // Modo 1KB por padrão

    // Configura o mapper
    mapper->number = 5;
    mapper->name = "MMC5";
    mapper->context = context;
    mapper->cpu_read = mapper5_cpu_read;
    mapper->cpu_write = mapper5_cpu_write;
    mapper->ppu_read = mapper5_ppu_read;
    mapper->ppu_write = mapper5_ppu_write;
    mapper->scanline = mapper5_scanline;
    mapper->reset = mapper5_reset;
    mapper->shutdown = mapper5_shutdown;

    MAPPER5_LOG_INFO("Mapper 5 (MMC5) inicializado com sucesso");
    return mapper;
}

/**
 * @brief Lê um byte da memória (acesso da CPU)
 */
static uint8_t mapper5_cpu_read(void *context, uint16_t address)
{
    mapper5_context_t *ctx = (mapper5_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address >= 0x5000 && address <= 0x5FFF)
    {
        // Registradores do MMC5
        switch (address)
        {
        case MMC5_IRQ_STATUS:
            return ctx->irq_status;
        case MMC5_MULT_A:
            return ctx->mult_result & 0xFF;
        case MMC5_MULT_B:
            return ctx->mult_result >> 8;
        default:
            return 0;
        }
    }
    else if (address >= 0x6000 && address <= 0x7FFF)
    {
        // PRG RAM
        if (cart->prg_ram)
        {
            return cart->prg_ram[address - 0x6000];
        }
    }
    else if (address >= 0x8000 && address <= 0xFFFF)
    {
        // PRG ROM
        uint32_t bank = 0;
        uint32_t offset = 0;

        switch (ctx->prg_mode)
        {
        case 0: // 32KB
            bank = ctx->prg_banks[4] >> 2;
            offset = (address - 0x8000) + (bank * 0x8000);
            break;
        case 1: // 16KB + 16KB
            if (address < 0xC000)
            {
                bank = ctx->prg_banks[2] >> 1;
                offset = (address - 0x8000) + (bank * 0x4000);
            }
            else
            {
                bank = ctx->prg_banks[4] >> 1;
                offset = (address - 0xC000) + (bank * 0x4000);
            }
            break;
        case 2: // 16KB + 8KB + 8KB
            if (address < 0xC000)
            {
                bank = ctx->prg_banks[2] >> 1;
                offset = (address - 0x8000) + (bank * 0x4000);
            }
            else if (address < 0xE000)
            {
                bank = ctx->prg_banks[3];
                offset = (address - 0xC000) + (bank * 0x2000);
            }
            else
            {
                bank = ctx->prg_banks[4];
                offset = (address - 0xE000) + (bank * 0x2000);
            }
            break;
        case 3: // 8KB + 8KB + 8KB + 8KB
            if (address < 0xA000)
            {
                bank = ctx->prg_banks[0];
                offset = (address - 0x8000) + (bank * 0x2000);
            }
            else if (address < 0xC000)
            {
                bank = ctx->prg_banks[1];
                offset = (address - 0xA000) + (bank * 0x2000);
            }
            else if (address < 0xE000)
            {
                bank = ctx->prg_banks[2];
                offset = (address - 0xC000) + (bank * 0x2000);
            }
            else
            {
                bank = ctx->prg_banks[3];
                offset = (address - 0xE000) + (bank * 0x2000);
            }
            break;
        }

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
static void mapper5_cpu_write(void *context, uint16_t address, uint8_t value)
{
    mapper5_context_t *ctx = (mapper5_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address >= 0x5000 && address <= 0x5FFF)
    {
        // Registradores do MMC5
        switch (address)
        {
        case MMC5_PRG_MODE:
            ctx->prg_mode = value & 0x03;
            MAPPER5_LOG_DEBUG("Modo PRG alterado para %d", ctx->prg_mode);
            break;
        case MMC5_CHR_MODE:
            ctx->chr_mode = value & 0x03;
            MAPPER5_LOG_DEBUG("Modo CHR alterado para %d", ctx->chr_mode);
            break;
        case MMC5_EXRAM_MODE:
            ctx->exram_mode = value & 0x03;
            MAPPER5_LOG_DEBUG("Modo ExRAM alterado para %d", ctx->exram_mode);
            break;
        case MMC5_NAMETABLE_MODE:
            ctx->nametable_mode = value;
            MAPPER5_LOG_DEBUG("Modo de nametable alterado para %02X", value);
            break;
        case MMC5_IRQ_SCANLINE:
            ctx->irq_scanline = value;
            MAPPER5_LOG_DEBUG("Scanline IRQ configurado para %d", value);
            break;
        case MMC5_IRQ_STATUS:
            ctx->irq_enable = value & 0x80;
            ctx->irq_status &= ~0x80;
            MAPPER5_LOG_DEBUG("Status IRQ atualizado: enable=%d", ctx->irq_enable);
            break;
        case MMC5_MULT_A:
            ctx->mult_a = value;
            ctx->mult_result = ctx->mult_a * ctx->mult_b;
            MAPPER5_LOG_TRACE("Multiplicador A definido: %d", value);
            break;
        case MMC5_MULT_B:
            ctx->mult_b = value;
            ctx->mult_result = ctx->mult_a * ctx->mult_b;
            MAPPER5_LOG_TRACE("Multiplicador B definido: %d", value);
            break;
        // PRG banking
        case MMC5_PRG_BANK_0:
        case MMC5_PRG_BANK_1:
        case MMC5_PRG_BANK_2:
        case MMC5_PRG_BANK_3:
        case MMC5_PRG_BANK_4:
            ctx->prg_banks[address - MMC5_PRG_BANK_0] = value;
            MAPPER5_LOG_DEBUG("Banco PRG %d selecionado: %d",
                              address - MMC5_PRG_BANK_0, value);
            break;
        // CHR banking
        case MMC5_CHR_BANK_0:
        case MMC5_CHR_BANK_1:
        case MMC5_CHR_BANK_2:
        case MMC5_CHR_BANK_3:
        case MMC5_CHR_BANK_4:
        case MMC5_CHR_BANK_5:
        case MMC5_CHR_BANK_6:
        case MMC5_CHR_BANK_7:
        case MMC5_CHR_BANK_8:
        case MMC5_CHR_BANK_9:
        case MMC5_CHR_BANK_10:
        case MMC5_CHR_BANK_11:
            ctx->chr_banks[address - MMC5_CHR_BANK_0] = value;
            MAPPER5_LOG_DEBUG("Banco CHR %d selecionado: %d",
                              address - MMC5_CHR_BANK_0, value);
            break;
        }
    }
    else if (address >= 0x6000 && address <= 0x7FFF)
    {
        // PRG RAM
        if (cart->prg_ram)
        {
            cart->prg_ram[address - 0x6000] = value;
            if (cart->has_battery)
            {
                cart->sram_dirty = 1;
            }
        }
    }
}

/**
 * @brief Lê um byte da CHR ROM/RAM
 */
static uint8_t mapper5_ppu_read(void *context, uint16_t address)
{
    mapper5_context_t *ctx = (mapper5_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF)
    {
        uint32_t bank = 0;
        uint32_t offset = 0;

        switch (ctx->chr_mode)
        {
        case 0: // 8KB
            bank = ctx->chr_banks[7];
            offset = address + (bank * 0x2000);
            break;
        case 1: // 4KB + 4KB
            if (address < 0x1000)
            {
                bank = ctx->chr_banks[3];
                offset = address + (bank * 0x1000);
            }
            else
            {
                bank = ctx->chr_banks[7];
                offset = (address - 0x1000) + (bank * 0x1000);
            }
            break;
        case 2: // 2KB + 2KB + 2KB + 2KB
            bank = ctx->chr_banks[address >> 11];
            offset = (address & 0x7FF) + (bank * 0x800);
            break;
        case 3: // 1KB x 8
            bank = ctx->chr_banks[address >> 10];
            offset = (address & 0x3FF) + (bank * 0x400);
            break;
        }

        if (cart->chr_rom && offset < cart->chr_rom_size)
        {
            return cart->chr_rom[offset];
        }
        else if (cart->chr_ram && offset < cart->chr_ram_size)
        {
            return cart->chr_ram[offset];
        }
    }

    return 0;
}

/**
 * @brief Escreve um byte na CHR RAM
 */
static void mapper5_ppu_write(void *context, uint16_t address, uint8_t value)
{
    mapper5_context_t *ctx = (mapper5_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF && cart->chr_ram)
    {
        uint32_t bank = 0;
        uint32_t offset = 0;

        switch (ctx->chr_mode)
        {
        case 0: // 8KB
            bank = ctx->chr_banks[7];
            offset = address + (bank * 0x2000);
            break;
        case 1: // 4KB + 4KB
            if (address < 0x1000)
            {
                bank = ctx->chr_banks[3];
                offset = address + (bank * 0x1000);
            }
            else
            {
                bank = ctx->chr_banks[7];
                offset = (address - 0x1000) + (bank * 0x1000);
            }
            break;
        case 2: // 2KB + 2KB + 2KB + 2KB
            bank = ctx->chr_banks[address >> 11];
            offset = (address & 0x7FF) + (bank * 0x800);
            break;
        case 3: // 1KB x 8
            bank = ctx->chr_banks[address >> 10];
            offset = (address & 0x3FF) + (bank * 0x400);
            break;
        }

        if (offset < cart->chr_ram_size)
        {
            cart->chr_ram[offset] = value;
        }
    }
}

/**
 * @brief Processa um scanline
 */
static void mapper5_scanline(void *context)
{
    mapper5_context_t *ctx = (mapper5_context_t *)context;

    if (ctx->in_frame)
    {
        if (ctx->cart->ppu && ctx->cart->ppu->current_scanline == ctx->irq_scanline)
        {
            ctx->irq_status |= 0x80;
            if (ctx->irq_enable)
            {
                MAPPER5_LOG_DEBUG("IRQ gerado na scanline %d", ctx->irq_scanline);
                if (ctx->cart->cpu)
                {
                    nes_cpu_trigger_irq(ctx->cart->cpu);
                }
            }
        }
    }

    // Atualiza o estado in_frame
    if (ctx->cart->ppu)
    {
        if (ctx->cart->ppu->current_scanline == 0)
        {
            ctx->in_frame = 1;
        }
        else if (ctx->cart->ppu->current_scanline == 241)
        {
            ctx->in_frame = 0;
        }
    }
}

/**
 * @brief Reseta o mapper para seu estado inicial
 */
static void mapper5_reset(void *context)
{
    mapper5_context_t *ctx = (mapper5_context_t *)context;

    MAPPER5_LOG_INFO("Resetando Mapper 5 (MMC5)");

    // Reseta registradores
    ctx->prg_mode = 3;
    ctx->chr_mode = 3;
    ctx->exram_mode = 0;
    ctx->nametable_mode = 0;
    ctx->irq_scanline = 0;
    ctx->irq_enable = 0;
    ctx->irq_status = 0;
    ctx->mult_a = 0;
    ctx->mult_b = 0;
    ctx->mult_result = 0;
    ctx->in_frame = 0;

    // Limpa bancos
    memset(ctx->prg_banks, 0, sizeof(ctx->prg_banks));
    memset(ctx->chr_banks, 0, sizeof(ctx->chr_banks));

    // Limpa ExRAM
    memset(ctx->exram, 0, sizeof(ctx->exram));
}

/**
 * @brief Desliga o mapper e libera recursos
 */
static void mapper5_shutdown(void *context)
{
    if (context)
    {
        MAPPER5_LOG_INFO("Desligando Mapper 5 (MMC5)");
        free(context);
    }
}
