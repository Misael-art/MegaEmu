/**
 * @file mapper26.c
 * @brief Implementação do Mapper 26 (VRC6a) para NES
 */

#include "platforms/nes/cartridge/mappers/mapper26.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 26
#define MAPPER26_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER26_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER26_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER26_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER26_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Registradores do VRC6a (com pinagem A0/A1 trocada)
#define VRC6A_PRG_SEL_8K 0xB002
#define VRC6A_PRG_SEL_16K 0x8000
#define VRC6A_CHR_SEL_1K_0 0xD000
#define VRC6A_CHR_SEL_1K_1 0xD002
#define VRC6A_CHR_SEL_1K_2 0xD001
#define VRC6A_CHR_SEL_1K_3 0xD003
#define VRC6A_CHR_SEL_1K_4 0xE000
#define VRC6A_CHR_SEL_1K_5 0xE002
#define VRC6A_CHR_SEL_1K_6 0xE001
#define VRC6A_CHR_SEL_1K_7 0xE003
#define VRC6A_IRQ_LATCH 0xF000
#define VRC6A_IRQ_CONTROL 0xF002
#define VRC6A_IRQ_ACK 0xF001
#define VRC6A_AUDIO_CONTROL 0x9003
#define VRC6A_PULSE1_CTRL 0x9000
#define VRC6A_PULSE1_FREQ_L 0x9002
#define VRC6A_PULSE1_FREQ_H 0x9001
#define VRC6A_PULSE2_CTRL 0xA000
#define VRC6A_PULSE2_FREQ_L 0xA002
#define VRC6A_PULSE2_FREQ_H 0xA001
#define VRC6A_SAW_CTRL 0xB000
#define VRC6A_SAW_FREQ_L 0xB002
#define VRC6A_SAW_FREQ_H 0xB001

// Estrutura do contexto do mapper
typedef struct
{
    nes_cartridge_t *cart; // Ponteiro para o cartucho
    void *cpu;             // Ponteiro para a CPU (para IRQs)

    // Bank switching
    uint8_t prg_bank_8k;  // Banco de 8KB em $6000-$7FFF
    uint8_t prg_bank_16k; // Banco de 16KB em $8000-$BFFF
    uint8_t chr_banks[8]; // Bancos de 1KB para CHR

    // IRQ
    uint8_t irq_latch;   // Valor de recarga do contador
    uint8_t irq_counter; // Contador atual
    uint8_t irq_enable;  // Flag de habilitação
    uint8_t irq_mode;    // Modo de contagem
    uint8_t irq_pending; // Flag de IRQ pendente

    // Som
    uint8_t audio_control;  // Controle de áudio global
    uint8_t pulse1_regs[3]; // Registradores do canal de pulso 1
    uint8_t pulse2_regs[3]; // Registradores do canal de pulso 2
    uint8_t saw_regs[3];    // Registradores do canal de dente de serra
} mapper26_context_t;

// Protótipos de funções
static uint8_t mapper26_cpu_read(void *context, uint16_t address);
static void mapper26_cpu_write(void *context, uint16_t address, uint8_t value);
static uint8_t mapper26_ppu_read(void *context, uint16_t address);
static void mapper26_ppu_write(void *context, uint16_t address, uint8_t value);
static void mapper26_scanline(void *context);
static void mapper26_reset(void *context);
static void mapper26_shutdown(void *context);

/**
 * @brief Inicializa o Mapper 26 (VRC6a)
 */
nes_mapper_t *nes_mapper_26_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER26_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER26_LOG_INFO("Inicializando Mapper 26 (VRC6a)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER26_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto específico do mapper
    mapper26_context_t *context = (mapper26_context_t *)malloc(sizeof(mapper26_context_t));
    if (!context)
    {
        MAPPER26_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(context, 0, sizeof(mapper26_context_t));

    // Configura o contexto
    context->cart = cartridge;

    // Configura o mapper
    mapper->number = 26;
    mapper->name = "VRC6a";
    mapper->context = context;
    mapper->cpu_read = mapper26_cpu_read;
    mapper->cpu_write = mapper26_cpu_write;
    mapper->ppu_read = mapper26_ppu_read;
    mapper->ppu_write = mapper26_ppu_write;
    mapper->scanline = mapper26_scanline;
    mapper->reset = mapper26_reset;
    mapper->shutdown = mapper26_shutdown;

    MAPPER26_LOG_INFO("Mapper 26 (VRC6a) inicializado com sucesso");
    return mapper;
}

/**
 * @brief Lê um byte da memória (acesso da CPU)
 */
static uint8_t mapper26_cpu_read(void *context, uint16_t address)
{
    mapper26_context_t *ctx = (mapper26_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address >= 0x6000 && address <= 0x7FFF)
    {
        // PRG-RAM
        if (cart->prg_ram)
        {
            return cart->prg_ram[address - 0x6000];
        }
    }
    else if (address >= 0x8000 && address <= 0xBFFF)
    {
        // PRG-ROM banco selecionável de 16KB
        uint32_t offset = (ctx->prg_bank_16k * 0x4000) + (address - 0x8000);
        if (offset < cart->prg_rom_size)
        {
            return cart->prg_rom[offset];
        }
    }
    else if (address >= 0xC000 && address <= 0xFFFF)
    {
        // PRG-ROM último banco fixo de 16KB
        uint32_t offset = (cart->prg_rom_size - 0x4000) + (address - 0xC000);
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
static void mapper26_cpu_write(void *context, uint16_t address, uint8_t value)
{
    mapper26_context_t *ctx = (mapper26_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address >= 0x6000 && address <= 0x7FFF)
    {
        // PRG-RAM
        if (cart->prg_ram)
        {
            cart->prg_ram[address - 0x6000] = value;
            if (cart->has_battery)
            {
                cart->sram_dirty = 1;
            }
        }
    }
    else if (address >= 0x8000)
    {
        // Registradores do mapper (com pinagem A0/A1 trocada)
        switch (address & 0xF003)
        {
        case VRC6A_PRG_SEL_16K:
            ctx->prg_bank_16k = value;
            MAPPER26_LOG_DEBUG("Banco PRG 16K selecionado: %d", value);
            break;

        case VRC6A_PRG_SEL_8K:
            ctx->prg_bank_8k = value;
            MAPPER26_LOG_DEBUG("Banco PRG 8K selecionado: %d", value);
            break;

        case VRC6A_CHR_SEL_1K_0:
        case VRC6A_CHR_SEL_1K_1:
        case VRC6A_CHR_SEL_1K_2:
        case VRC6A_CHR_SEL_1K_3:
            ctx->chr_banks[address - VRC6A_CHR_SEL_1K_0] = value;
            MAPPER26_LOG_DEBUG("Banco CHR %d selecionado: %d",
                               address - VRC6A_CHR_SEL_1K_0, value);
            break;

        case VRC6A_CHR_SEL_1K_4:
        case VRC6A_CHR_SEL_1K_5:
        case VRC6A_CHR_SEL_1K_6:
        case VRC6A_CHR_SEL_1K_7:
            ctx->chr_banks[4 + (address - VRC6A_CHR_SEL_1K_4)] = value;
            MAPPER26_LOG_DEBUG("Banco CHR %d selecionado: %d",
                               4 + (address - VRC6A_CHR_SEL_1K_4), value);
            break;

        case VRC6A_IRQ_LATCH:
            ctx->irq_latch = value;
            MAPPER26_LOG_DEBUG("IRQ latch definido: %d", value);
            break;

        case VRC6A_IRQ_CONTROL:
            ctx->irq_enable = value & 0x03;
            ctx->irq_mode = (value >> 2) & 0x03;
            if (ctx->irq_enable & 0x02)
            {
                ctx->irq_counter = ctx->irq_latch;
            }
            ctx->irq_pending = 0;
            MAPPER26_LOG_DEBUG("IRQ control: enable=%d, mode=%d",
                               ctx->irq_enable, ctx->irq_mode);
            break;

        case VRC6A_IRQ_ACK:
            ctx->irq_pending = 0;
            if (ctx->irq_enable & 0x01)
            {
                ctx->irq_enable |= 0x02;
            }
            MAPPER26_LOG_DEBUG("IRQ reconhecido");
            break;

        case VRC6A_AUDIO_CONTROL:
            ctx->audio_control = value;
            MAPPER26_LOG_DEBUG("Audio control: %02X", value);
            break;

        case VRC6A_PULSE1_CTRL:
        case VRC6A_PULSE1_FREQ_L:
        case VRC6A_PULSE1_FREQ_H:
            ctx->pulse1_regs[address - VRC6A_PULSE1_CTRL] = value;
            break;

        case VRC6A_PULSE2_CTRL:
        case VRC6A_PULSE2_FREQ_L:
        case VRC6A_PULSE2_FREQ_H:
            ctx->pulse2_regs[address - VRC6A_PULSE2_CTRL] = value;
            break;

        case VRC6A_SAW_CTRL:
        case VRC6A_SAW_FREQ_L:
        case VRC6A_SAW_FREQ_H:
            ctx->saw_regs[address - VRC6A_SAW_CTRL] = value;
            break;
        }
    }
}

/**
 * @brief Lê um byte da CHR ROM/RAM
 */
static uint8_t mapper26_ppu_read(void *context, uint16_t address)
{
    mapper26_context_t *ctx = (mapper26_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF)
    {
        uint8_t bank = ctx->chr_banks[address >> 10];
        uint32_t offset = (bank * 0x400) + (address & 0x3FF);

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
static void mapper26_ppu_write(void *context, uint16_t address, uint8_t value)
{
    mapper26_context_t *ctx = (mapper26_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF && cart->chr_ram)
    {
        uint8_t bank = ctx->chr_banks[address >> 10];
        uint32_t offset = (bank * 0x400) + (address & 0x3FF);

        if (offset < cart->chr_ram_size)
        {
            cart->chr_ram[offset] = value;
        }
    }
}

/**
 * @brief Processa um scanline
 */
static void mapper26_scanline(void *context)
{
    mapper26_context_t *ctx = (mapper26_context_t *)context;

    if (ctx->irq_enable & 0x02)
    {
        if (ctx->irq_counter == 0xFF)
        {
            ctx->irq_counter = ctx->irq_latch;
            ctx->irq_pending = 1;
            MAPPER26_LOG_DEBUG("IRQ triggered");

            if (ctx->cart->cpu)
            {
                nes_cpu_trigger_irq(ctx->cart->cpu);
            }
        }
        else
        {
            ctx->irq_counter++;
        }
    }
}

/**
 * @brief Reseta o mapper para seu estado inicial
 */
static void mapper26_reset(void *context)
{
    mapper26_context_t *ctx = (mapper26_context_t *)context;

    MAPPER26_LOG_INFO("Resetando Mapper 26 (VRC6a)");

    // Reseta registradores
    ctx->prg_bank_8k = 0;
    ctx->prg_bank_16k = 0;
    memset(ctx->chr_banks, 0, sizeof(ctx->chr_banks));

    // Reseta IRQ
    ctx->irq_latch = 0;
    ctx->irq_counter = 0;
    ctx->irq_enable = 0;
    ctx->irq_mode = 0;
    ctx->irq_pending = 0;

    // Reseta som
    ctx->audio_control = 0;
    memset(ctx->pulse1_regs, 0, sizeof(ctx->pulse1_regs));
    memset(ctx->pulse2_regs, 0, sizeof(ctx->pulse2_regs));
    memset(ctx->saw_regs, 0, sizeof(ctx->saw_regs));
}

/**
 * @brief Desliga o mapper e libera recursos
 */
static void mapper26_shutdown(void *context)
{
    if (context)
    {
        MAPPER26_LOG_INFO("Desligando Mapper 26 (VRC6a)");
        free(context);
    }
}
