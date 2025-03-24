/**
 * @file mapper75.c
 * @brief Implementação do Mapper 75 (VRC1)
 */

#include "platforms/nes/cartridge/mappers/mapper75.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 75
#define MAPPER75_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER75_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER75_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER75_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER75_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Registradores do VRC1
#define VRC1_PRG_SEL_8K 0x8000
#define VRC1_PRG_SEL_16K 0xA000
#define VRC1_CHR_SEL_4K_LOW 0x9000
#define VRC1_CHR_SEL_4K_HIGH 0x9002
#define VRC1_MIRROR_CTRL 0x9001

// Estrutura do contexto do mapper
typedef struct
{
    nes_cartridge_t *cart;  // Ponteiro para o cartucho
    uint8_t prg_bank_8k;    // Banco de 8KB em $8000-$9FFF
    uint8_t prg_bank_16k;   // Banco de 16KB em $A000-$DFFF
    uint8_t chr_bank_4k[2]; // Bancos de 4KB para CHR
    uint8_t mirror_mode;    // Modo de espelhamento
} mapper75_context_t;

// Protótipos de funções
static uint8_t mapper75_cpu_read(void *ctx, uint16_t addr);
static void mapper75_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper75_ppu_read(void *ctx, uint16_t addr);
static void mapper75_ppu_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper75_reset(void *ctx);
static void mapper75_shutdown(void *ctx);

/**
 * @brief Inicializa o Mapper 75 (VRC1)
 */
nes_mapper_t *nes_mapper_75_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER75_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER75_LOG_INFO("Inicializando Mapper 75 (VRC1)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER75_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto do mapper
    mapper75_context_t *ctx = (mapper75_context_t *)malloc(sizeof(mapper75_context_t));
    if (!ctx)
    {
        MAPPER75_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(ctx, 0, sizeof(mapper75_context_t));

    // Inicializa o contexto
    ctx->cart = cartridge;
    ctx->prg_bank_8k = 0;
    ctx->prg_bank_16k = 0;
    ctx->chr_bank_4k[0] = 0;
    ctx->chr_bank_4k[1] = 1;
    ctx->mirror_mode = 0;

    // Configura o mapper
    mapper->number = 75;
    mapper->name = "VRC1";
    mapper->context = ctx;
    mapper->cpu_read = mapper75_cpu_read;
    mapper->cpu_write = mapper75_cpu_write;
    mapper->ppu_read = mapper75_ppu_read;
    mapper->ppu_write = mapper75_ppu_write;
    mapper->reset = mapper75_reset;
    mapper->shutdown = mapper75_shutdown;

    MAPPER75_LOG_INFO("Mapper 75 (VRC1) inicializado com sucesso");
    return mapper;
}

/**
 * @brief Lê um byte da memória (CPU)
 */
static uint8_t mapper75_cpu_read(void *ctx, uint16_t addr)
{
    mapper75_context_t *m75 = (mapper75_context_t *)ctx;

    if (addr >= 0x8000 && addr <= 0x9FFF)
    {
        // Banco selecionável de 8KB em $8000-$9FFF
        uint32_t offset = ((uint32_t)m75->prg_bank_8k * 0x2000) + (addr & 0x1FFF);
        return m75->cart->prg_rom[offset % m75->cart->prg_rom_size];
    }
    else if (addr >= 0xA000 && addr <= 0xDFFF)
    {
        // Banco selecionável de 16KB em $A000-$DFFF
        uint32_t offset = ((uint32_t)m75->prg_bank_16k * 0x4000) + (addr & 0x3FFF);
        return m75->cart->prg_rom[offset % m75->cart->prg_rom_size];
    }
    else if (addr >= 0xE000)
    {
        // Último banco fixo em $E000-$FFFF
        uint32_t offset = (m75->cart->prg_rom_size - 0x2000) + (addr & 0x1FFF);
        return m75->cart->prg_rom[offset % m75->cart->prg_rom_size];
    }

    return 0;
}

/**
 * @brief Escreve um byte na memória (CPU)
 */
static void mapper75_cpu_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper75_context_t *m75 = (mapper75_context_t *)ctx;

    switch (addr & 0xF003)
    {
    case VRC1_PRG_SEL_8K:
        m75->prg_bank_8k = val & 0x0F;
        MAPPER75_LOG_DEBUG("Banco PRG 8K selecionado: %d", m75->prg_bank_8k);
        break;

    case VRC1_PRG_SEL_16K:
        m75->prg_bank_16k = val & 0x0F;
        MAPPER75_LOG_DEBUG("Banco PRG 16K selecionado: %d", m75->prg_bank_16k);
        break;

    case VRC1_CHR_SEL_4K_LOW:
        m75->chr_bank_4k[0] = val & 0x0F;
        MAPPER75_LOG_DEBUG("Banco CHR 4K baixo selecionado: %d", m75->chr_bank_4k[0]);
        break;

    case VRC1_CHR_SEL_4K_HIGH:
        m75->chr_bank_4k[1] = val & 0x0F;
        MAPPER75_LOG_DEBUG("Banco CHR 4K alto selecionado: %d", m75->chr_bank_4k[1]);
        break;

    case VRC1_MIRROR_CTRL:
        m75->mirror_mode = val & 0x01;
        MAPPER75_LOG_DEBUG("Modo de espelhamento: %s",
                           m75->mirror_mode ? "Vertical" : "Horizontal");
        break;
    }
}

/**
 * @brief Lê um byte da memória de padrões (PPU)
 */
static uint8_t mapper75_ppu_read(void *ctx, uint16_t addr)
{
    mapper75_context_t *m75 = (mapper75_context_t *)ctx;

    if (addr < 0x1000)
    {
        // Primeiro banco de 4KB
        uint32_t offset = ((uint32_t)m75->chr_bank_4k[0] * 0x1000) + (addr & 0xFFF);
        return m75->cart->chr_rom[offset % m75->cart->chr_rom_size];
    }
    else if (addr < 0x2000)
    {
        // Segundo banco de 4KB
        uint32_t offset = ((uint32_t)m75->chr_bank_4k[1] * 0x1000) + (addr & 0xFFF);
        return m75->cart->chr_rom[offset % m75->cart->chr_rom_size];
    }

    return 0;
}

/**
 * @brief Escreve um byte na memória de padrões (PPU)
 */
static void mapper75_ppu_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper75_context_t *m75 = (mapper75_context_t *)ctx;

    // VRC1 não tem CHR-RAM, então ignora escritas
    MAPPER75_LOG_WARN("Tentativa de escrita em CHR-ROM: $%04X = $%02X", addr, val);
}

/**
 * @brief Reseta o mapper
 */
static void mapper75_reset(void *ctx)
{
    mapper75_context_t *m75 = (mapper75_context_t *)ctx;

    m75->prg_bank_8k = 0;
    m75->prg_bank_16k = 0;
    m75->chr_bank_4k[0] = 0;
    m75->chr_bank_4k[1] = 1;
    m75->mirror_mode = 0;

    MAPPER75_LOG_INFO("Mapper 75 (VRC1) resetado");
}

/**
 * @brief Finaliza o mapper
 */
static void mapper75_shutdown(void *ctx)
{
    if (ctx)
    {
        MAPPER75_LOG_INFO("Desligando Mapper 75 (VRC1)");
        free(ctx);
    }
}
