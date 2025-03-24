/**
 * @file mapper10.c
 * @brief Implementação do Mapper 10 (MMC4/FxROM) para NES
 *
 * O Mapper 10 (MMC4/FxROM) é uma variação do MMC2 (Mapper 9) usado principalmente
 * em jogos como Fire Emblem. Características principais:
 * - PRG-ROM: 128KB divididos em bancos de 16KB
 * - CHR-ROM: Bancos de 4KB com mecanismo de latch especial
 * - Suporte a PRG-RAM com bateria
 * - Controle de espelhamento selecionável
 * - Mecanismo de latch similar ao MMC2, mas com endereçamento diferente
 */

#include "platforms/nes/cartridge/nes_cartridge.h"
#include "common/logging.h"
#include <stdlib.h>
#include <string.h>

#define MAPPER10_LOG_INFO(...)  EMU_LOG_INFO("[MAPPER10]", __VA_ARGS__)
#define MAPPER10_LOG_ERROR(...) EMU_LOG_ERROR("[MAPPER10]", __VA_ARGS__)
#define MAPPER10_LOG_DEBUG(...) EMU_LOG_DEBUG("[MAPPER10]", __VA_ARGS__)
#define MAPPER10_LOG_WARN(...)  EMU_LOG_WARN("[MAPPER10]", __VA_ARGS__)

// Contexto específico do Mapper 10
typedef struct {
    uint8_t* prg_rom;
    uint32_t prg_rom_size;
    uint8_t* chr_rom;
    uint32_t chr_rom_size;
    uint8_t* prg_ram;
    uint32_t prg_ram_size;
    uint8_t* chr_ram;
    uint32_t chr_ram_size;

    // Registros de controle
    uint8_t prg_bank;           // Banco PRG (16KB) em $8000-$BFFF
    uint8_t chr_bank_0_fd;      // Banco CHR 0 para latch $FD
    uint8_t chr_bank_0_fe;      // Banco CHR 0 para latch $FE
    uint8_t chr_bank_1_fd;      // Banco CHR 1 para latch $FD
    uint8_t chr_bank_1_fe;      // Banco CHR 1 para latch $FE
    uint8_t latch_0;            // Latch para pattern table 0
    uint8_t latch_1;            // Latch para pattern table 1
    uint8_t mirror_mode;        // Modo de espelhamento

    // Número total de bancos
    uint8_t prg_banks;          // Total de bancos PRG de 16KB
    uint8_t chr_banks;          // Total de bancos CHR de 4KB

    nes_cartridge_t* cartridge; // Referência ao cartucho
} mapper10_context_t;

// Protótipos de funções
static uint8_t mapper10_cpu_read(void* context, uint16_t address);
static void mapper10_cpu_write(void* context, uint16_t address, uint8_t value);
static uint8_t mapper10_ppu_read(void* context, uint16_t address);
static void mapper10_ppu_write(void* context, uint16_t address, uint8_t value);
static void mapper10_reset(void* context);
static void mapper10_shutdown(void* context);

/**
 * @brief Inicializa o Mapper 10
 * @param cartridge Ponteiro para a estrutura do cartucho
 * @return Ponteiro para a estrutura do mapper ou NULL em caso de erro
 */
nes_mapper_t* nes_mapper_10_init(nes_cartridge_t* cartridge)
{
    if (!cartridge) {
        MAPPER10_LOG_ERROR("Tentativa de inicializar mapper com cartucho NULL");
        return NULL;
    }

    MAPPER10_LOG_INFO("Inicializando Mapper 10 (MMC4/FxROM)");

    // Aloca estrutura do mapper
    nes_mapper_t* mapper = (nes_mapper_t*)malloc(sizeof(nes_mapper_t));
    if (!mapper) {
        MAPPER10_LOG_ERROR("Falha ao alocar memória para o mapper");
        return NULL;
    }
    memset(mapper, 0, sizeof(nes_mapper_t));

    // Aloca contexto específico do mapper
    mapper10_context_t* context = (mapper10_context_t*)malloc(sizeof(mapper10_context_t));
    if (!context) {
        MAPPER10_LOG_ERROR("Falha ao alocar memória para o contexto do mapper");
        free(mapper);
        return NULL;
    }
    memset(context, 0, sizeof(mapper10_context_t));

    // Configura o contexto
    context->prg_rom = cartridge->prg_rom;
    context->prg_rom_size = cartridge->prg_rom_size;
    context->chr_rom = cartridge->chr_rom;
    context->chr_rom_size = cartridge->chr_rom_size;
    context->prg_ram = cartridge->prg_ram;
    context->prg_ram_size = cartridge->prg_ram_size;
    context->chr_ram = cartridge->chr_ram;
    context->chr_ram_size = cartridge->chr_ram_size;
    context->cartridge = cartridge;

    // Calcula o número de bancos
    context->prg_banks = context->prg_rom_size / 16384;
    context->chr_banks = (context->chr_rom_size > 0) ? (context->chr_rom_size / 4096) : 0;

    MAPPER10_LOG_DEBUG("PRG-ROM: %d KB (%d bancos de 16KB)", context->prg_rom_size / 1024, context->prg_banks);

    if (context->chr_rom_size > 0) {
        MAPPER10_LOG_DEBUG("CHR-ROM: %d KB (%d bancos de 4KB)", context->chr_rom_size / 1024, context->chr_banks);
    } else if (context->chr_ram_size > 0) {
        MAPPER10_LOG_DEBUG("CHR-RAM: %d KB", context->chr_ram_size / 1024);
    }

    // Inicializa os registros
    context->prg_bank = 0;
    context->chr_bank_0_fd = 0;
    context->chr_bank_0_fe = 0;
    context->chr_bank_1_fd = 0;
    context->chr_bank_1_fe = 0;
    context->latch_0 = 0xFE;    // Valor inicial para latch 0
    context->latch_1 = 0xFE;    // Valor inicial para latch 1
    context->mirror_mode = cartridge->mirror_mode;

    // Configura o mapper
    mapper->number = 10;
    mapper->name = "MMC4/FxROM";
    mapper->context = context;
    mapper->cpu_read = mapper10_cpu_read;
    mapper->cpu_write = mapper10_cpu_write;
    mapper->ppu_read = mapper10_ppu_read;
    mapper->ppu_write = mapper10_ppu_write;
    mapper->reset = mapper10_reset;
    mapper->shutdown = mapper10_shutdown;

    MAPPER10_LOG_INFO("Mapper 10 inicializado com sucesso");
    return mapper;
}

/**
 * @brief Manipula leituras da CPU
 * @param context Contexto do mapper
 * @param address Endereço a ser lido (0x0000-0xFFFF)
 * @return Byte lido do endereço
 */
static uint8_t mapper10_cpu_read(void* context, uint16_t address)
{
    mapper10_context_t* ctx = (mapper10_context_t*)context;

    // PRG-RAM ($6000-$7FFF)
    if (address >= 0x6000 && address <= 0x7FFF) {
        if (ctx->prg_ram && ctx->prg_ram_size > 0) {
            return ctx->prg_ram[address - 0x6000];
        }
        return 0xFF;
    }

    // PRG-ROM ($8000-$FFFF)
    if (address >= 0x8000 && address <= 0xFFFF) {
        uint32_t prg_addr;

        // $8000-$BFFF: Banco selecionável via $A000
        if (address >= 0x8000 && address <= 0xBFFF) {
            prg_addr = ((uint32_t)ctx->prg_bank * 16384) + (address - 0x8000);
        }
        // $C000-$FFFF: Fixo no último banco
        else {
            prg_addr = ((ctx->prg_banks - 1) * 16384) + (address - 0xC000);
        }

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
static void mapper10_cpu_write(void* context, uint16_t address, uint8_t value)
{
    mapper10_context_t* ctx = (mapper10_context_t*)context;

    // PRG-RAM ($6000-$7FFF)
    if (address >= 0x6000 && address <= 0x7FFF) {
        if (ctx->prg_ram && ctx->prg_ram_size > 0) {
            ctx->prg_ram[address - 0x6000] = value;
        }
        return;
    }

    // Registros de controle ($A000-$FFFF)
    if (address >= 0xA000 && address <= 0xFFFF) {
        // $A000-$AFFF: Seleciona o banco PRG
        if (address >= 0xA000 && address <= 0xAFFF) {
            ctx->prg_bank = value & 0x0F;
            MAPPER10_LOG_DEBUG("Banco PRG selecionado: %d", ctx->prg_bank);
        }
        // $B000-$BFFF: Seleciona o banco CHR 0 para latch $FD
        else if (address >= 0xB000 && address <= 0xBFFF) {
            ctx->chr_bank_0_fd = value & 0x1F;
            MAPPER10_LOG_DEBUG("Banco CHR 0 FD selecionado: %d", ctx->chr_bank_0_fd);
        }
        // $C000-$CFFF: Seleciona o banco CHR 0 para latch $FE
        else if (address >= 0xC000 && address <= 0xCFFF) {
            ctx->chr_bank_0_fe = value & 0x1F;
            MAPPER10_LOG_DEBUG("Banco CHR 0 FE selecionado: %d", ctx->chr_bank_0_fe);
        }
        // $D000-$DFFF: Seleciona o banco CHR 1 para latch $FD
        else if (address >= 0xD000 && address <= 0xDFFF) {
            ctx->chr_bank_1_fd = value & 0x1F;
            MAPPER10_LOG_DEBUG("Banco CHR 1 FD selecionado: %d", ctx->chr_bank_1_fd);
        }
        // $E000-$EFFF: Seleciona o banco CHR 1 para latch $FE
        else if (address >= 0xE000 && address <= 0xEFFF) {
            ctx->chr_bank_1_fe = value & 0x1F;
            MAPPER10_LOG_DEBUG("Banco CHR 1 FE selecionado: %d", ctx->chr_bank_1_fe);
        }
        // $F000-$FFFF: Controla o modo de espelhamento
        else if (address >= 0xF000 && address <= 0xFFFF) {
            ctx->mirror_mode = (value & 0x01) ? NES_MIRROR_VERTICAL : NES_MIRROR_HORIZONTAL;
            ctx->cartridge->mirror_mode = ctx->mirror_mode;
            MAPPER10_LOG_DEBUG("Modo de espelhamento: %s", ctx->mirror_mode ? "Vertical" : "Horizontal");
        }
    }
}

/**
 * @brief Manipula leituras da PPU
 * @param context Contexto do mapper
 * @param address Endereço a ser lido (0x0000-0x1FFF)
 * @return Byte lido do endereço
 */
static uint8_t mapper10_ppu_read(void* context, uint16_t address)
{
    mapper10_context_t* ctx = (mapper10_context_t*)context;

    // Pattern Tables ($0000-$1FFF)
    if (address <= 0x1FFF) {
        uint32_t chr_addr;

        // Verifica endereço especial para o tile $FD ou $FE na pattern table 0
        if (address >= 0x0FD8 && address <= 0x0FDF) {
            ctx->latch_0 = 0xFD;
            MAPPER10_LOG_DEBUG("Latch 0 ativado: $FD");
        } else if (address >= 0x0FE8 && address <= 0x0FEF) {
            ctx->latch_0 = 0xFE;
            MAPPER10_LOG_DEBUG("Latch 0 ativado: $FE");
        }

        // Verifica endereço especial para o tile $FD ou $FE na pattern table 1
        if (address >= 0x1FD8 && address <= 0x1FDF) {
            ctx->latch_1 = 0xFD;
            MAPPER10_LOG_DEBUG("Latch 1 ativado: $FD");
        } else if (address >= 0x1FE8 && address <= 0x1FEF) {
            ctx->latch_1 = 0xFE;
            MAPPER10_LOG_DEBUG("Latch 1 ativado: $FE");
        }

        // Seleciona o banco baseado no latch
        if (address < 0x1000) {
            // Pattern Table 0
            uint8_t bank = (ctx->latch_0 == 0xFD) ? ctx->chr_bank_0_fd : ctx->chr_bank_0_fe;
            chr_addr = (bank * 4096) + address;
        } else {
            // Pattern Table 1
            uint8_t bank = (ctx->latch_1 == 0xFD) ? ctx->chr_bank_1_fd : ctx->chr_bank_1_fe;
            chr_addr = (bank * 4096) + (address - 0x1000);
        }

        // Verifica se temos CHR-ROM
        if (ctx->chr_rom && ctx->chr_rom_size > 0) {
            // Garante que não ultrapassamos o tamanho da CHR-ROM
            chr_addr %= ctx->chr_rom_size;
            return ctx->chr_rom[chr_addr];
        } else if (ctx->chr_ram && ctx->chr_ram_size > 0) {
            // Para CHR-RAM, ignoramos o banking (normalmente 8KB apenas)
            return ctx->chr_ram[address % ctx->chr_ram_size];
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
static void mapper10_ppu_write(void* context, uint16_t address, uint8_t value)
{
    mapper10_context_t* ctx = (mapper10_context_t*)context;

    // Pattern Tables ($0000-$1FFF) - Escreve apenas se houver CHR-RAM
    if (address <= 0x1FFF) {
        if (ctx->chr_ram && ctx->chr_ram_size > 0) {
            ctx->chr_ram[address % ctx->chr_ram_size] = value;
        }
    }
}

/**
 * @brief Reseta o mapper para seu estado inicial
 * @param context Contexto do mapper
 */
static void mapper10_reset(void* context)
{
    mapper10_context_t* ctx = (mapper10_context_t*)context;

    // Reinicia o estado do mapper
    ctx->prg_bank = 0;
    ctx->chr_bank_0_fd = 0;
    ctx->chr_bank_0_fe = 0;
    ctx->chr_bank_1_fd = 0;
    ctx->chr_bank_1_fe = 0;
    ctx->latch_0 = 0xFE;
    ctx->latch_1 = 0xFE;
    ctx->mirror_mode = ctx->cartridge->mirror_mode;

    MAPPER10_LOG_INFO("Mapper 10 resetado");
}

/**
 * @brief Desliga o mapper e libera recursos
 * @param context Contexto do mapper
 */
static void mapper10_shutdown(void* context)
{
    if (context) {
        MAPPER10_LOG_INFO("Desligando Mapper 10");
        free(context);
    }
}
