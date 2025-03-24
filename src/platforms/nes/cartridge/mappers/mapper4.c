/**
 * @file mapper4.c
 * @brief Implementação do Mapper 4 (MMC3) para o Nintendo Entertainment System
 *
 * O Mapper 4 (MMC3) é usado em jogos populares como Super Mario Bros. 3, Mega Man 3-6,
 * e possui as seguintes características:
 * - PRG-ROM: Até 512KB (bancos de 8KB com mapeamento configurável)
 * - CHR-ROM/RAM: Até 256KB (bancos de 1KB e 2KB com mapeamento configurável)
 * - IRQ baseado em scanline da PPU
 * - Controle de espelhamento vertical/horizontal
 * - Suporte para PRG-RAM com bateria
 */

#include "platforms/nes/cartridge/nes_cartridge.h"
#include "utils/logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Definição da categoria de log para o mapper
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_NES_MAPPERS

// Macros de log específicas para o Mapper 4
#define MAPPER_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Tamanho de um banco PRG (8KB)
#define PRG_BANK_SIZE (8 * 1024)

// Tamanho de um banco CHR (1KB)
#define CHR_BANK_SIZE (1 * 1024)

// Estrutura de contexto para o Mapper 4
typedef struct
{
    nes_cartridge_t *cart; // Ponteiro para o cartucho

    // Registradores do MMC3
    uint8_t bank_select;     // Registrador de seleção de banco (0x8000-0x8001)
    uint8_t bank_data[8];    // Dados dos bancos (0x8001)
    uint8_t mirror_mode;     // Modo de mirroring (0xA000-0xA001)
    uint8_t prg_ram_protect; // Proteção da PRG-RAM (0xA001)
    uint8_t irq_latch;       // Valor de recarga do IRQ (0xC000-0xC001)
    uint8_t irq_counter;     // Contador de IRQ (interno)
    uint8_t irq_enable;      // Flag de habilitação do IRQ (0xE000-0xE001)
    uint8_t irq_pending;     // Flag indicando IRQ pendente
    uint8_t irq_reload;      // Flag para recarregar o contador IRQ

    // Mapeamento de bancos
    uint32_t prg_banks[4]; // Endereços dos bancos de PRG-ROM
    uint32_t chr_banks[8]; // Endereços dos bancos de CHR-ROM/RAM

    // Informações do cartucho
    uint8_t *prg_rom;     // Ponteiro para a PRG-ROM
    int prg_rom_size;     // Tamanho da PRG-ROM em bytes
    uint8_t *chr_rom;     // Ponteiro para a CHR-ROM
    int chr_rom_size;     // Tamanho da CHR-ROM em bytes
    uint8_t *chr_ram;     // Ponteiro para a CHR-RAM (se existir)
    int chr_ram_size;     // Tamanho da CHR-RAM em bytes
    uint8_t uses_chr_ram; // Flag indicando se usa CHR-RAM

    // Para o IRQ
    int a12_low_cycles;     // Ciclos com A12 em baixo (para detecção de borda de subida)
    uint8_t last_a12_state; // Último estado da linha A12
} mapper4_context_t;

// Protótipos de funções
static uint8_t mapper4_cpu_read(void *ctx, uint16_t addr);
static void mapper4_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper4_chr_read(void *ctx, uint16_t addr);
static void mapper4_chr_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper4_reset(void *ctx);
static void mapper4_shutdown(void *ctx);
static void mapper4_scanline(void *ctx);
static uint8_t mapper4_irq_state(void *ctx);
static void mapper4_irq_clear(void *ctx);
static void mapper4_update_banks(mapper4_context_t *ctx);

/**
 * @brief Inicializa o Mapper 4 (MMC3)
 *
 * @param cartridge Ponteiro para o cartucho
 * @return nes_mapper_t* Ponteiro para o mapper inicializado, ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_4_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): cartridge inválido");
        return NULL;
    }

    MAPPER_LOG_INFO("Inicializando Mapper 4 (MMC3)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)malloc(sizeof(nes_mapper_t));
    if (!mapper)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): falha ao alocar estrutura do mapper");
        return NULL;
    }

    // Inicializa a estrutura com zeros
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto do mapper
    mapper4_context_t *ctx = (mapper4_context_t *)malloc(sizeof(mapper4_context_t));
    if (!ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): falha ao alocar contexto do mapper");
        free(mapper);
        return NULL;
    }

    // Inicializa o contexto
    memset(ctx, 0, sizeof(mapper4_context_t));

    ctx->cart = cartridge;
    ctx->prg_rom = cartridge->prg_rom;
    ctx->prg_rom_size = cartridge->prg_rom_size;
    ctx->chr_rom = cartridge->chr_rom;
    ctx->chr_rom_size = cartridge->chr_rom_size;
    ctx->chr_ram = cartridge->chr_ram;
    ctx->chr_ram_size = cartridge->chr_ram_size;

    // Determinar se usa CHR-RAM
    ctx->uses_chr_ram = (ctx->chr_rom_size == 0 && ctx->chr_ram_size > 0) ? 1 : 0;

    if (ctx->prg_rom_size == 0)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): tamanho de PRG-ROM inválido: %d", ctx->prg_rom_size);
        free(ctx);
        free(mapper);
        return NULL;
    }

    if (ctx->chr_rom_size == 0 && !ctx->uses_chr_ram)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): sem CHR-ROM ou CHR-RAM");
        free(ctx);
        free(mapper);
        return NULL;
    }

    // Configura as funções do mapper
    mapper->mapper_number = 4;
    mapper->cpu_read = mapper4_cpu_read;
    mapper->cpu_write = mapper4_cpu_write;
    mapper->chr_read = mapper4_chr_read;
    mapper->chr_write = mapper4_chr_write;
    mapper->reset = mapper4_reset;
    mapper->shutdown = mapper4_shutdown;
    mapper->scanline = mapper4_scanline;
    mapper->irq_state = mapper4_irq_state;
    mapper->irq_clear = mapper4_irq_clear;
    mapper->context = ctx;

    // Reset inicial
    mapper4_reset(ctx);

    MAPPER_LOG_INFO("Mapper 4 (MMC3) inicializado com sucesso: PRG-ROM=%dKB, CHR-%s=%dKB",
                    ctx->prg_rom_size / 1024,
                    ctx->uses_chr_ram ? "RAM" : "ROM",
                    (ctx->uses_chr_ram ? ctx->chr_ram_size : ctx->chr_rom_size) / 1024);

    return mapper;
}

/**
 * @brief Atualiza o mapeamento de bancos após mudança nos registradores
 *
 * @param ctx Contexto do mapper
 */
static void mapper4_update_banks(mapper4_context_t *ctx)
{
    // Número total de bancos PRG
    const uint32_t prg_banks_count = ctx->prg_rom_size / PRG_BANK_SIZE;

    // Número total de bancos CHR
    const uint32_t chr_banks_count = ctx->uses_chr_ram ? (ctx->chr_ram_size / CHR_BANK_SIZE) : (ctx->chr_rom_size / CHR_BANK_SIZE);

    // Modo de mapeamento PRG (bit 6 do bank select)
    uint8_t prg_mode = (ctx->bank_select & 0x40) != 0;

    // Modo de mapeamento CHR (bit 7 do bank select)
    uint8_t chr_mode = (ctx->bank_select & 0x80) != 0;

    // Mapeamento de bancos PRG
    if (!prg_mode)
    {
        // Modo 0: Banco em $8000 é fixo no penúltimo, banco em $A000 é variável
        ctx->prg_banks[0] = (prg_banks_count - 2) * PRG_BANK_SIZE;
        ctx->prg_banks[1] = (ctx->bank_data[6] % prg_banks_count) * PRG_BANK_SIZE;
    }
    else
    {
        // Modo 1: Banco em $8000 é variável, banco em $A000 é fixo no penúltimo
        ctx->prg_banks[0] = (ctx->bank_data[6] % prg_banks_count) * PRG_BANK_SIZE;
        ctx->prg_banks[1] = (prg_banks_count - 2) * PRG_BANK_SIZE;
    }

    // Os dois últimos bancos são fixos:
    // $C000-$DFFF: Variável (registrador 7)
    ctx->prg_banks[2] = (ctx->bank_data[7] % prg_banks_count) * PRG_BANK_SIZE;
    // $E000-$FFFF: Fixo no último banco
    ctx->prg_banks[3] = (prg_banks_count - 1) * PRG_BANK_SIZE;

    // Mapeamento de bancos CHR
    if (!chr_mode)
    {
        // Modo 0: Dois bancos de 2KB em $0000, quatro bancos de 1KB em $1000
        // $0000-$07FF: Registrador 0 (2KB)
        ctx->chr_banks[0] = ((ctx->bank_data[0] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE;
        ctx->chr_banks[1] = ((ctx->bank_data[0] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE + CHR_BANK_SIZE;
        // $0800-$0FFF: Registrador 1 (2KB)
        ctx->chr_banks[2] = ((ctx->bank_data[1] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE;
        ctx->chr_banks[3] = ((ctx->bank_data[1] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE + CHR_BANK_SIZE;
        // $1000-$13FF: Registrador 2 (1KB)
        ctx->chr_banks[4] = (ctx->bank_data[2] % chr_banks_count) * CHR_BANK_SIZE;
        // $1400-$17FF: Registrador 3 (1KB)
        ctx->chr_banks[5] = (ctx->bank_data[3] % chr_banks_count) * CHR_BANK_SIZE;
        // $1800-$1BFF: Registrador 4 (1KB)
        ctx->chr_banks[6] = (ctx->bank_data[4] % chr_banks_count) * CHR_BANK_SIZE;
        // $1C00-$1FFF: Registrador 5 (1KB)
        ctx->chr_banks[7] = (ctx->bank_data[5] % chr_banks_count) * CHR_BANK_SIZE;
    }
    else
    {
        // Modo 1: Dois bancos de 2KB em $1000, quatro bancos de 1KB em $0000
        // $0000-$03FF: Registrador 2 (1KB)
        ctx->chr_banks[0] = (ctx->bank_data[2] % chr_banks_count) * CHR_BANK_SIZE;
        // $0400-$07FF: Registrador 3 (1KB)
        ctx->chr_banks[1] = (ctx->bank_data[3] % chr_banks_count) * CHR_BANK_SIZE;
        // $0800-$0BFF: Registrador 4 (1KB)
        ctx->chr_banks[2] = (ctx->bank_data[4] % chr_banks_count) * CHR_BANK_SIZE;
        // $0C00-$0FFF: Registrador 5 (1KB)
        ctx->chr_banks[3] = (ctx->bank_data[5] % chr_banks_count) * CHR_BANK_SIZE;
        // $1000-$17FF: Registrador 0 (2KB)
        ctx->chr_banks[4] = ((ctx->bank_data[0] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE;
        ctx->chr_banks[5] = ((ctx->bank_data[0] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE + CHR_BANK_SIZE;
        // $1800-$1FFF: Registrador 1 (2KB)
        ctx->chr_banks[6] = ((ctx->bank_data[1] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE;
        ctx->chr_banks[7] = ((ctx->bank_data[1] & 0xFE) % chr_banks_count) * CHR_BANK_SIZE + CHR_BANK_SIZE;
    }

    MAPPER_LOG_DEBUG("Mapper 4: PRG banks - $8000: %06X, $A000: %06X, $C000: %06X, $E000: %06X",
                     ctx->prg_banks[0], ctx->prg_banks[1], ctx->prg_banks[2], ctx->prg_banks[3]);
}

/**
 * @brief Reseta o estado do Mapper 4
 *
 * @param ctx Contexto do mapper
 */
static void mapper4_reset(void *ctx)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido no reset");
        return;
    }

    // Resetar registradores
    mapper_ctx->bank_select = 0;
    for (int i = 0; i < 8; i++)
    {
        mapper_ctx->bank_data[i] = 0;
    }
    mapper_ctx->mirror_mode = 0;
    mapper_ctx->prg_ram_protect = 0;
    mapper_ctx->irq_latch = 0;
    mapper_ctx->irq_counter = 0;
    mapper_ctx->irq_enable = 0;
    mapper_ctx->irq_pending = 0;
    mapper_ctx->irq_reload = 0;
    mapper_ctx->a12_low_cycles = 0;
    mapper_ctx->last_a12_state = 0;

    // Configuração inicial dos bancos
    mapper4_update_banks(mapper_ctx);

    MAPPER_LOG_INFO("Mapper 4 (MMC3): reset realizado");
}

/**
 * @brief Finaliza e libera recursos do Mapper 4
 *
 * @param ctx Contexto do mapper
 */
static void mapper4_shutdown(void *ctx)
{
    if (!ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido no shutdown");
        return;
    }

    MAPPER_LOG_DEBUG("Mapper 4 (MMC3): liberando recursos");
    free(ctx);
}

/**
 * @brief Lê um byte do espaço de endereços da CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para leitura
 * @return uint8_t Valor lido
 */
static uint8_t mapper4_cpu_read(void *ctx, uint16_t addr)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido na leitura da CPU");
        return 0;
    }

    // Verificar se o endereço está no intervalo correto
    if (addr < 0x6000)
    {
        MAPPER_LOG_WARN("Mapper 4 (MMC3): tentativa de leitura fora do intervalo: 0x%04X", addr);
        return 0;
    }

    // PRG-RAM em $6000-$7FFF
    if (addr >= 0x6000 && addr < 0x8000)
    {
        // Verificar se a PRG-RAM está habilitada (depende do bit 7 de prg_ram_protect)
        if ((mapper_ctx->prg_ram_protect & 0x80) == 0)
        {
            // PRG-RAM desabilitada
            return 0;
        }

        // Calcular endereço na PRG-RAM
        uint32_t ram_addr = addr - 0x6000;
        if (ram_addr >= mapper_ctx->cart->prg_ram_size)
        {
            MAPPER_LOG_WARN("Mapper 4 (MMC3): endereço de PRG-RAM fora dos limites: 0x%04X (máximo: 0x%06X)",
                            ram_addr, mapper_ctx->cart->prg_ram_size - 1);
            ram_addr %= mapper_ctx->cart->prg_ram_size;
        }

        return mapper_ctx->cart->prg_ram[ram_addr];
    }

    // Mapeamento de PRG-ROM em $8000-$FFFF
    uint32_t bank = (addr - 0x8000) / 0x2000;   // Qual banco de 8KB (0-3)
    uint32_t offset = (addr - 0x8000) % 0x2000; // Deslocamento dentro do banco
    uint32_t prg_addr = mapper_ctx->prg_banks[bank] + offset;

    // Verificar limites
    if (prg_addr >= mapper_ctx->prg_rom_size)
    {
        MAPPER_LOG_WARN("Mapper 4 (MMC3): endereço de PRG-ROM fora dos limites: 0x%06X (máximo: 0x%06X)",
                        prg_addr, mapper_ctx->prg_rom_size - 1);
        prg_addr %= mapper_ctx->prg_rom_size;
    }

    return mapper_ctx->prg_rom[prg_addr];
}

/**
 * @brief Escreve um byte no espaço de endereços da CPU
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para escrita
 * @param val Valor a ser escrito
 */
static void mapper4_cpu_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido na escrita da CPU");
        return;
    }

    // Verificar se o endereço está no intervalo correto
    if (addr < 0x6000)
    {
        MAPPER_LOG_WARN("Mapper 4 (MMC3): tentativa de escrita fora do intervalo: 0x%04X", addr);
        return;
    }

    // PRG-RAM em $6000-$7FFF
    if (addr >= 0x6000 && addr < 0x8000)
    {
        // Verificar se a PRG-RAM está habilitada e protegida contra escrita
        if ((mapper_ctx->prg_ram_protect & 0x80) == 0 || (mapper_ctx->prg_ram_protect & 0x40) == 0)
        {
            // PRG-RAM desabilitada ou protegida contra escrita
            return;
        }

        // Calcular endereço na PRG-RAM
        uint32_t ram_addr = addr - 0x6000;
        if (ram_addr >= mapper_ctx->cart->prg_ram_size)
        {
            MAPPER_LOG_WARN("Mapper 4 (MMC3): endereço de PRG-RAM fora dos limites: 0x%04X (máximo: 0x%06X)",
                            ram_addr, mapper_ctx->cart->prg_ram_size - 1);
            ram_addr %= mapper_ctx->cart->prg_ram_size;
        }

        mapper_ctx->cart->prg_ram[ram_addr] = val;
        // Se houver bateria, marca a PRG-RAM como suja
        if (mapper_ctx->cart->has_battery)
        {
            mapper_ctx->cart->sram_dirty = 1;
        }
        return;
    }

    // Registradores do MMC3 em $8000-$FFFF
    // Cada par de endereços tem uma função específica
    switch (addr & 0xE001)
    {
    case 0x8000: // Bank select ($8000-$9FFE, even)
        mapper_ctx->bank_select = val;
        mapper4_update_banks(mapper_ctx);
        break;

    case 0x8001: // Bank data ($8001-$9FFF, odd)
    {
        uint8_t reg = mapper_ctx->bank_select & 0x07;
        mapper_ctx->bank_data[reg] = val;
        mapper4_update_banks(mapper_ctx);
    }
    break;

    case 0xA000: // Mirroring ($A000-$BFFE, even)
        mapper_ctx->mirror_mode = val & 0x01;
        if (mapper_ctx->cart->mirror_mode != NES_MIRROR_FOUR_SCREEN)
        {
            mapper_ctx->cart->mirror_mode = (val & 0x01) ? NES_MIRROR_HORIZONTAL : NES_MIRROR_VERTICAL;
        }
        break;

    case 0xA001: // PRG RAM protect ($A001-$BFFF, odd)
        mapper_ctx->prg_ram_protect = val;
        break;

    case 0xC000: // IRQ latch ($C000-$DFFE, even)
        mapper_ctx->irq_latch = val;
        break;

    case 0xC001: // IRQ reload ($C001-$DFFF, odd)
        // Reset o contador e marca para ser recarregado na próxima borda de descida de A12
        mapper_ctx->irq_counter = 0;
        mapper_ctx->irq_reload = 1;
        break;

    case 0xE000: // IRQ disable ($E000-$FFFE, even)
        mapper_ctx->irq_enable = 0;
        mapper_ctx->irq_pending = 0;
        break;

    case 0xE001: // IRQ enable ($E001-$FFFF, odd)
        mapper_ctx->irq_enable = 1;
        break;
    }
}

/**
 * @brief Lê um byte do espaço de endereços da PPU (padrões/tiles)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para leitura
 * @return uint8_t Valor lido
 */
static uint8_t mapper4_chr_read(void *ctx, uint16_t addr)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido na leitura da PPU");
        return 0;
    }

    // Verificar se o endereço está no intervalo correto
    if (addr >= 0x2000)
    {
        MAPPER_LOG_WARN("Mapper 4 (MMC3): acesso de leitura a endereço PPU não mapeado: $%04X", addr);
        return 0;
    }

    // Detecção de borda de subida da linha A12 (bit 12 do endereço)
    uint8_t a12_state = (addr & 0x1000) ? 1 : 0;
    if (a12_state == 1 && mapper_ctx->last_a12_state == 0)
    {
        // Borda de subida detectada em A12
        if (mapper_ctx->irq_reload)
        {
            mapper_ctx->irq_counter = mapper_ctx->irq_latch;
            mapper_ctx->irq_reload = 0;
        }
        else if (mapper_ctx->irq_counter == 0)
        {
            mapper_ctx->irq_counter = mapper_ctx->irq_latch;
        }
        else
        {
            mapper_ctx->irq_counter--;
            if (mapper_ctx->irq_counter == 0 && mapper_ctx->irq_enable)
            {
                mapper_ctx->irq_pending = 1;
            }
        }
    }
    mapper_ctx->last_a12_state = a12_state;

    // Determinar qual banco de CHR acessar com base no endereço
    uint32_t bank_index;
    if (addr < 0x0400)
        bank_index = 0;
    else if (addr < 0x0800)
        bank_index = 1;
    else if (addr < 0x0C00)
        bank_index = 2;
    else if (addr < 0x1000)
        bank_index = 3;
    else if (addr < 0x1400)
        bank_index = 4;
    else if (addr < 0x1800)
        bank_index = 5;
    else if (addr < 0x1C00)
        bank_index = 6;
    else
        bank_index = 7;

    uint32_t offset = addr & 0x03FF; // Offset dentro do banco de 1KB
    uint32_t chr_addr = mapper_ctx->chr_banks[bank_index] + offset;

    // Se estiver usando CHR-RAM
    if (mapper_ctx->uses_chr_ram)
    {
        if (chr_addr >= mapper_ctx->chr_ram_size)
        {
            MAPPER_LOG_WARN("Mapper 4 (MMC3): acesso a endereço CHR-RAM inválido: $%04X (efetivo: $%06X)", addr, chr_addr);
            chr_addr %= mapper_ctx->chr_ram_size;
        }
        return mapper_ctx->chr_ram[chr_addr];
    }
    // Se estiver usando CHR-ROM
    else
    {
        if (chr_addr >= mapper_ctx->chr_rom_size)
        {
            MAPPER_LOG_WARN("Mapper 4 (MMC3): acesso a endereço CHR-ROM inválido: $%04X (efetivo: $%06X)", addr, chr_addr);
            chr_addr %= mapper_ctx->chr_rom_size;
        }
        return mapper_ctx->chr_rom[chr_addr];
    }
}

/**
 * @brief Escreve um byte no espaço de endereços da PPU (padrões/tiles)
 *
 * @param ctx Contexto do mapper
 * @param addr Endereço para escrita
 * @param val Valor a ser escrito
 */
static void mapper4_chr_write(void *ctx, uint16_t addr, uint8_t val)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido na escrita da PPU");
        return;
    }

    // Verificar se o endereço está no intervalo correto
    if (addr >= 0x2000)
    {
        MAPPER_LOG_WARN("Mapper 4 (MMC3): acesso de escrita a endereço PPU não mapeado: $%04X = $%02X", addr, val);
        return;
    }

    // Detecção de borda de subida da linha A12 (bit 12 do endereço)
    uint8_t a12_state = (addr & 0x1000) ? 1 : 0;
    if (a12_state == 1 && mapper_ctx->last_a12_state == 0)
    {
        // Borda de subida detectada em A12
        if (mapper_ctx->irq_reload)
        {
            mapper_ctx->irq_counter = mapper_ctx->irq_latch;
            mapper_ctx->irq_reload = 0;
        }
        else if (mapper_ctx->irq_counter == 0)
        {
            mapper_ctx->irq_counter = mapper_ctx->irq_latch;
        }
        else
        {
            mapper_ctx->irq_counter--;
            if (mapper_ctx->irq_counter == 0 && mapper_ctx->irq_enable)
            {
                mapper_ctx->irq_pending = 1;
            }
        }
    }
    mapper_ctx->last_a12_state = a12_state;

    // Apenas escreve na CHR-RAM, se estiver usando
    if (mapper_ctx->uses_chr_ram)
    {
        // Determinar qual banco de CHR acessar com base no endereço
        uint32_t bank_index;
        if (addr < 0x0400)
            bank_index = 0;
        else if (addr < 0x0800)
            bank_index = 1;
        else if (addr < 0x0C00)
            bank_index = 2;
        else if (addr < 0x1000)
            bank_index = 3;
        else if (addr < 0x1400)
            bank_index = 4;
        else if (addr < 0x1800)
            bank_index = 5;
        else if (addr < 0x1C00)
            bank_index = 6;
        else
            bank_index = 7;

        uint32_t offset = addr & 0x03FF; // Offset dentro do banco de 1KB
        uint32_t chr_addr = mapper_ctx->chr_banks[bank_index] + offset;

        if (chr_addr >= mapper_ctx->chr_ram_size)
        {
            MAPPER_LOG_WARN("Mapper 4 (MMC3): acesso de escrita a endereço CHR-RAM inválido: $%04X = $%02X (efetivo: $%06X)", addr, val, chr_addr);
            chr_addr %= mapper_ctx->chr_ram_size;
        }

        mapper_ctx->chr_ram[chr_addr] = val;
    }
    else
    {
        // Tentativa de escrita em CHR-ROM, que é apenas leitura
        MAPPER_LOG_WARN("Mapper 4 (MMC3): tentativa de escrita em CHR-ROM: $%04X = $%02X", addr, val);
    }
}

/**
 * @brief Notifica o mapper sobre uma nova scanline para IRQ
 *
 * @param ctx Contexto do mapper
 */
static void mapper4_scanline(void *ctx)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido no scanline");
        return;
    }

    // O MMC3 não usa esta função para IRQ, pois o IRQ é baseado em acessos a CHR-ROM
    // A função foi mantida apenas por compatibilidade com a interface de mappers
}

/**
 * @brief Retorna o estado do sinal de IRQ
 *
 * @param ctx Contexto do mapper
 * @return uint8_t 1 se IRQ está ativo, 0 caso contrário
 */
static uint8_t mapper4_irq_state(void *ctx)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido no irq_state");
        return 0;
    }

    return mapper_ctx->irq_pending;
}

/**
 * @brief Limpa o sinal de IRQ
 *
 * @param ctx Contexto do mapper
 */
static void mapper4_irq_clear(void *ctx)
{
    mapper4_context_t *mapper_ctx = (mapper4_context_t *)ctx;
    if (!mapper_ctx)
    {
        MAPPER_LOG_ERROR("Mapper 4 (MMC3): contexto inválido no irq_clear");
        return;
    }

    mapper_ctx->irq_pending = 0;
}
