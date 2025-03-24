/**
 * @file mapper20.c
 * @brief Implementação do Mapper 20 (FDS - Famicom Disk System) para NES
 */

#include "platforms/nes/cartridge/mappers/mapper20.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 20
#define MAPPER20_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER20_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER20_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER20_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER20_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Registradores do FDS
#define FDS_DISK_REG 0x4020
#define FDS_STATUS_REG 0x4030
#define FDS_CONTROL_REG 0x4024
#define FDS_DATA_REG 0x4031
#define FDS_TIMER_LOW 0x4032
#define FDS_TIMER_HIGH 0x4033
#define FDS_WAVE_DATA 0x4040
#define FDS_WAVE_CONTROL 0x4080
#define FDS_ENVELOPE_SPEED 0x4082
#define FDS_VOLUME_GAIN 0x4083
#define FDS_SWEEP_SPEED 0x4084
#define FDS_SWEEP_GAIN 0x4085
#define FDS_MOD_DATA 0x4086
#define FDS_MOD_CONTROL 0x4087
#define FDS_MASTER_IO 0x4089
#define FDS_READ_ENABLE 0x4090
#define FDS_WRITE_ENABLE 0x4091

// Estrutura do contexto do mapper
typedef struct
{
    nes_cartridge_t *cart; // Ponteiro para o cartucho
    void *cpu;             // Ponteiro para a CPU (para IRQs)

    // Estado do disco
    uint8_t disk_inserted;    // Flag indicando disco inserido
    uint8_t disk_side;        // Lado atual do disco (0 ou 1)
    uint8_t disk_ready;       // Flag indicando disco pronto
    uint8_t disk_irq_enable;  // Flag de habilitação de IRQ do disco
    uint8_t disk_irq_pending; // Flag de IRQ pendente

    // Registradores de controle
    uint8_t control_reg; // Registrador de controle
    uint8_t status_reg;  // Registrador de status
    uint8_t data_reg;    // Registrador de dados

    // Timer
    uint16_t timer_counter;    // Contador do timer
    uint16_t timer_reload;     // Valor de recarga do timer
    uint8_t timer_irq_enable;  // Flag de habilitação de IRQ do timer
    uint8_t timer_irq_pending; // Flag de IRQ pendente do timer

    // Som
    uint8_t wave_ram[64];  // RAM de forma de onda
    uint8_t wave_pos;      // Posição atual na RAM de forma de onda
    uint8_t mod_table[32]; // Tabela de modulação
    uint8_t mod_pos;       // Posição atual na tabela de modulação

    // RAM de expansão
    uint8_t *disk_ram; // 32KB de RAM para dados do disco
} mapper20_context_t;

// Protótipos de funções
static uint8_t mapper20_cpu_read(void *context, uint16_t address);
static void mapper20_cpu_write(void *context, uint16_t address, uint8_t value);
static uint8_t mapper20_ppu_read(void *context, uint16_t address);
static void mapper20_ppu_write(void *context, uint16_t address, uint8_t value);
static void mapper20_reset(void *context);
static void mapper20_shutdown(void *context);

/**
 * @brief Inicializa o Mapper 20 (FDS)
 */
nes_mapper_t *nes_mapper_20_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER20_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER20_LOG_INFO("Inicializando Mapper 20 (FDS)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER20_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto específico do mapper
    mapper20_context_t *context = (mapper20_context_t *)malloc(sizeof(mapper20_context_t));
    if (!context)
    {
        MAPPER20_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(context, 0, sizeof(mapper20_context_t));

    // Aloca RAM do disco
    context->disk_ram = (uint8_t *)malloc(32 * 1024);
    if (!context->disk_ram)
    {
        MAPPER20_LOG_ERROR("Falha ao alocar memória para a RAM do disco");
        free(context);
        free(mapper);
        return NULL;
    }
    memset(context->disk_ram, 0, 32 * 1024);

    // Configura o contexto
    context->cart = cartridge;
    context->disk_ready = 1;
    context->status_reg = 0x80; // Disco pronto

    // Configura o mapper
    mapper->number = 20;
    mapper->name = "FDS";
    mapper->context = context;
    mapper->cpu_read = mapper20_cpu_read;
    mapper->cpu_write = mapper20_cpu_write;
    mapper->ppu_read = mapper20_ppu_read;
    mapper->ppu_write = mapper20_ppu_write;
    mapper->reset = mapper20_reset;
    mapper->shutdown = mapper20_shutdown;

    MAPPER20_LOG_INFO("Mapper 20 (FDS) inicializado com sucesso");
    return mapper;
}

/**
 * @brief Lê um byte da memória (acesso da CPU)
 */
static uint8_t mapper20_cpu_read(void *context, uint16_t address)
{
    mapper20_context_t *ctx = (mapper20_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address >= 0x4020 && address <= 0x409F)
    {
        // Registradores do FDS
        switch (address)
        {
        case FDS_STATUS_REG:
        {
            uint8_t status = ctx->status_reg;
            ctx->disk_irq_pending = 0;
            ctx->timer_irq_pending = 0;
            return status;
        }
        case FDS_DATA_REG:
            return ctx->data_reg;
        case FDS_TIMER_LOW:
            return ctx->timer_counter & 0xFF;
        case FDS_TIMER_HIGH:
            return (ctx->timer_counter >> 8) & 0xFF;
        case FDS_WAVE_DATA:
        {
            uint8_t data = ctx->wave_ram[ctx->wave_pos];
            ctx->wave_pos = (ctx->wave_pos + 1) & 0x3F;
            return data;
        }
        }
    }
    else if (address >= 0x6000 && address <= 0xDFFF)
    {
        // RAM do disco
        return ctx->disk_ram[address - 0x6000];
    }
    else if (address >= 0xE000 && address <= 0xFFFF)
    {
        // BIOS do FDS
        if (cart->prg_rom)
        {
            return cart->prg_rom[address - 0xE000];
        }
    }

    return 0;
}

/**
 * @brief Escreve um byte na memória (acesso da CPU)
 */
static void mapper20_cpu_write(void *context, uint16_t address, uint8_t value)
{
    mapper20_context_t *ctx = (mapper20_context_t *)context;

    if (address >= 0x4020 && address <= 0x409F)
    {
        // Registradores do FDS
        switch (address)
        {
        case FDS_CONTROL_REG:
            ctx->control_reg = value;
            if (value & 0x80)
            {
                ctx->disk_irq_enable = 1;
            }
            break;
        case FDS_DATA_REG:
            ctx->data_reg = value;
            break;
        case FDS_TIMER_LOW:
            ctx->timer_reload = (ctx->timer_reload & 0xFF00) | value;
            break;
        case FDS_TIMER_HIGH:
            ctx->timer_reload = (ctx->timer_reload & 0x00FF) | (value << 8);
            ctx->timer_counter = ctx->timer_reload;
            ctx->timer_irq_enable = value & 0x80;
            ctx->timer_irq_pending = 0;
            break;
        case FDS_WAVE_DATA:
            ctx->wave_ram[ctx->wave_pos] = value;
            ctx->wave_pos = (ctx->wave_pos + 1) & 0x3F;
            break;
        case FDS_MASTER_IO:
            if (value & 0x80)
            {
                ctx->disk_ready = 1;
                ctx->status_reg |= 0x80;
            }
            break;
        }
    }
    else if (address >= 0x6000 && address <= 0xDFFF)
    {
        // RAM do disco
        ctx->disk_ram[address - 0x6000] = value;
    }
}

/**
 * @brief Lê um byte da CHR RAM
 */
static uint8_t mapper20_ppu_read(void *context, uint16_t address)
{
    mapper20_context_t *ctx = (mapper20_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF && cart->chr_ram)
    {
        return cart->chr_ram[address];
    }

    return 0;
}

/**
 * @brief Escreve um byte na CHR RAM
 */
static void mapper20_ppu_write(void *context, uint16_t address, uint8_t value)
{
    mapper20_context_t *ctx = (mapper20_context_t *)context;
    nes_cartridge_t *cart = ctx->cart;

    if (address <= 0x1FFF && cart->chr_ram)
    {
        cart->chr_ram[address] = value;
    }
}

/**
 * @brief Reseta o mapper para seu estado inicial
 */
static void mapper20_reset(void *context)
{
    mapper20_context_t *ctx = (mapper20_context_t *)context;

    MAPPER20_LOG_INFO("Resetando Mapper 20 (FDS)");

    // Reseta registradores
    ctx->control_reg = 0;
    ctx->status_reg = 0x80;
    ctx->data_reg = 0;
    ctx->timer_counter = 0;
    ctx->timer_reload = 0;
    ctx->timer_irq_enable = 0;
    ctx->timer_irq_pending = 0;
    ctx->disk_irq_enable = 0;
    ctx->disk_irq_pending = 0;
    ctx->disk_ready = 1;
    ctx->wave_pos = 0;
    ctx->mod_pos = 0;

    // Limpa RAMs
    memset(ctx->wave_ram, 0, sizeof(ctx->wave_ram));
    memset(ctx->mod_table, 0, sizeof(ctx->mod_table));
    memset(ctx->disk_ram, 0, 32 * 1024);
}

/**
 * @brief Desliga o mapper e libera recursos
 */
static void mapper20_shutdown(void *context)
{
    if (context)
    {
        mapper20_context_t *ctx = (mapper20_context_t *)context;
        MAPPER20_LOG_INFO("Desligando Mapper 20 (FDS)");

        if (ctx->disk_ram)
        {
            free(ctx->disk_ram);
        }
        free(ctx);
    }
}
