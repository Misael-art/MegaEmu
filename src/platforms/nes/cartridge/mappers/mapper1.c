/**
 * @file mapper1.c
 * @brief Implementação do mapper MMC1 (Mapper 1) para o NES
 *
 * O MMC1 é um dos mappers mais utilizados para jogos de NES.
 * Características:
 * - Suporta até 512KB de PRG ROM com banco de 16KB
 * - Suporta até 256KB de CHR ROM/RAM com bancos de 4KB ou 8KB
 * - Possui registradores de controle acessíveis por escrita serial
 * - Suporta diferentes modos de espelhamento de VRAM
 *
 * Jogos populares: Zelda, Metroid, Final Fantasy, Mega Man 2, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../nes_cartridge.h"
#include "platforms/nes/nes_types.h"
#include "utils/logger/logger.h"

// Categoria de log para o mapper
#define MAPPER1_LOG_CAT "NES_MAPPER1"

// Registradores do MMC1
#define MMC1_REG_CONTROL 0
#define MMC1_REG_CHR_BANK0 1
#define MMC1_REG_CHR_BANK1 2
#define MMC1_REG_PRG_BANK 3

// Bits do registrador de controle
#define MMC1_CTRL_MIRROR_MASK 0x03
#define MMC1_CTRL_MIRROR_ONE_LOW 0x00
#define MMC1_CTRL_MIRROR_ONE_HIGH 0x01
#define MMC1_CTRL_MIRROR_VERTICAL 0x02
#define MMC1_CTRL_MIRROR_HORIZONTAL 0x03
#define MMC1_CTRL_PRG_BANK_MODE 0x0C
#define MMC1_CTRL_CHR_BANK_MODE 0x10

/**
 * @brief Estrutura de contexto para o mapper MMC1
 */
typedef struct
{
    nes_cartridge_t *cartridge; // Ponteiro para o cartridge

    uint8_t registers[4];   // Registradores internos do MMC1
    uint8_t shift_register; // Registrador de deslocamento para escrita serial
    uint8_t shift_count;    // Contador de bits para escrita serial

    uint8_t prg_rom_banks; // Número de bancos de 16KB de PRG ROM
    uint8_t chr_rom_banks; // Número de bancos de 8KB de CHR ROM

    uint8_t *prg_rom; // Ponteiro para os dados de PRG ROM
    uint8_t *chr_rom; // Ponteiro para os dados de CHR ROM/RAM
    bool chr_is_ram;  // Flag indicando se CHR é RAM

    // Modo de espelhamento da VRAM
    nes_mirror_mode_t mirror_mode;
} mapper1_context_t;

// Protótipos de funções
static uint8_t mapper1_cpu_read(nes_mapper_t *mapper, uint16_t address);
static void mapper1_cpu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value);
static uint8_t mapper1_ppu_read(nes_mapper_t *mapper, uint16_t address);
static void mapper1_ppu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value);
static void mapper1_reset(nes_mapper_t *mapper);
static void mapper1_shutdown(nes_mapper_t *mapper);
static nes_mirror_mode_t mapper1_get_mirror_mode(nes_mapper_t *mapper);

/**
 * @brief Inicializa o mapper MMC1
 *
 * @param cartridge Ponteiro para a estrutura do cartridge
 * @return nes_mapper_t* Ponteiro para o mapper ou NULL em caso de erro
 */
nes_mapper_t *nes_mapper_1_init(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        LOG_ERROR(MAPPER1_LOG_CAT, "Cartridge nulo");
        return NULL;
    }

    LOG_INFO(MAPPER1_LOG_CAT, "Inicializando mapper MMC1 (1)");

    // Aloca estrutura do mapper
    nes_mapper_t *mapper = (nes_mapper_t *)calloc(1, sizeof(nes_mapper_t));
    if (!mapper)
    {
        LOG_ERROR(MAPPER1_LOG_CAT, "Falha ao alocar memória para mapper");
        return NULL;
    }

    // Aloca estrutura de contexto
    mapper1_context_t *context = (mapper1_context_t *)calloc(1, sizeof(mapper1_context_t));
    if (!context)
    {
        LOG_ERROR(MAPPER1_LOG_CAT, "Falha ao alocar memória para contexto do mapper");
        free(mapper);
        return NULL;
    }

    // Inicializa o contexto
    context->cartridge = cartridge;
    context->prg_rom = cartridge->prg_rom;
    context->prg_rom_banks = cartridge->prg_rom_size / NES_PRG_ROM_BANK_SIZE;

    if (cartridge->chr_rom_size > 0)
    {
        context->chr_rom = cartridge->chr_rom;
        context->chr_rom_banks = cartridge->chr_rom_size / NES_CHR_ROM_BANK_SIZE;
        context->chr_is_ram = false;
    }
    else
    {
        // Usar CHR RAM se não há CHR ROM
        context->chr_rom = (uint8_t *)calloc(NES_CHR_RAM_SIZE, 1);
        context->chr_rom_banks = 1;
        context->chr_is_ram = true;

        if (!context->chr_rom)
        {
            LOG_ERROR(MAPPER1_LOG_CAT, "Falha ao alocar memória para CHR RAM");
            free(context);
            free(mapper);
            return NULL;
        }
    }

    // Inicialização dos registradores (valores padrão)
    context->registers[MMC1_REG_CONTROL] = 0x0C; // PRG ROM bank mode: fix first bank
    context->registers[MMC1_REG_CHR_BANK0] = 0;
    context->registers[MMC1_REG_CHR_BANK1] = 0;
    context->registers[MMC1_REG_PRG_BANK] = 0;

    context->shift_register = 0x10; // bit 4 set (reset flag)
    context->shift_count = 0;

    // O modo de espelhamento é inicializado baseado no controle
    uint8_t mirror_bits = context->registers[MMC1_REG_CONTROL] & MMC1_CTRL_MIRROR_MASK;
    switch (mirror_bits)
    {
    case MMC1_CTRL_MIRROR_HORIZONTAL:
        context->mirror_mode = NES_MIRROR_HORIZONTAL;
        break;
    case MMC1_CTRL_MIRROR_VERTICAL:
        context->mirror_mode = NES_MIRROR_VERTICAL;
        break;
    case MMC1_CTRL_MIRROR_ONE_LOW:
        context->mirror_mode = NES_MIRROR_ONE_SCREEN_LOWER;
        break;
    case MMC1_CTRL_MIRROR_ONE_HIGH:
        context->mirror_mode = NES_MIRROR_ONE_SCREEN_UPPER;
        break;
    default:
        context->mirror_mode = cartridge->mirror_mode;
        break;
    }

    // Configura o mapper
    mapper->number = 1;
    mapper->name = "MMC1";
    mapper->context = context;

    // Configura os ponteiros de função
    mapper->cpu_read = mapper1_cpu_read;
    mapper->cpu_write = mapper1_cpu_write;
    mapper->ppu_read = mapper1_ppu_read;
    mapper->ppu_write = mapper1_ppu_write;
    mapper->reset = mapper1_reset;
    mapper->shutdown = mapper1_shutdown;
    mapper->get_mirror_mode = mapper1_get_mirror_mode;

    LOG_INFO(MAPPER1_LOG_CAT, "Mapper MMC1 inicializado com %d bancos de PRG ROM e %d bancos de CHR %s",
             context->prg_rom_banks, context->chr_rom_banks, context->chr_is_ram ? "RAM" : "ROM");

    return mapper;
}

/**
 * @brief Finaliza o mapper MMC1 e libera memória
 *
 * @param mapper Ponteiro para o mapper
 */
static void mapper1_shutdown(nes_mapper_t *mapper)
{
    if (!mapper || !mapper->context)
    {
        return;
    }

    mapper1_context_t *context = (mapper1_context_t *)mapper->context;

    // Libera CHR RAM se foi alocada
    if (context->chr_is_ram && context->chr_rom)
    {
        free(context->chr_rom);
    }

    // Libera o contexto e o mapper
    free(context);
    free(mapper);

    LOG_INFO(MAPPER1_LOG_CAT, "Mapper MMC1 finalizado");
}

/**
 * @brief Reseta o mapper MMC1 para o estado inicial
 *
 * @param mapper Ponteiro para o mapper
 */
static void mapper1_reset(nes_mapper_t *mapper)
{
    if (!mapper || !mapper->context)
    {
        return;
    }

    mapper1_context_t *context = (mapper1_context_t *)mapper->context;

    // Reinicia os registradores para os valores padrão
    context->registers[MMC1_REG_CONTROL] = 0x0C;
    context->registers[MMC1_REG_CHR_BANK0] = 0;
    context->registers[MMC1_REG_CHR_BANK1] = 0;
    context->registers[MMC1_REG_PRG_BANK] = 0;

    context->shift_register = 0x10; // bit 4 set (reset flag)
    context->shift_count = 0;

    // Reinicia o modo de espelhamento
    uint8_t mirror_bits = context->registers[MMC1_REG_CONTROL] & MMC1_CTRL_MIRROR_MASK;
    switch (mirror_bits)
    {
    case MMC1_CTRL_MIRROR_HORIZONTAL:
        context->mirror_mode = NES_MIRROR_HORIZONTAL;
        break;
    case MMC1_CTRL_MIRROR_VERTICAL:
        context->mirror_mode = NES_MIRROR_VERTICAL;
        break;
    case MMC1_CTRL_MIRROR_ONE_LOW:
        context->mirror_mode = NES_MIRROR_ONE_SCREEN_LOWER;
        break;
    case MMC1_CTRL_MIRROR_ONE_HIGH:
        context->mirror_mode = NES_MIRROR_ONE_SCREEN_UPPER;
        break;
    default:
        context->mirror_mode = context->cartridge->mirror_mode;
        break;
    }

    LOG_INFO(MAPPER1_LOG_CAT, "Mapper MMC1 resetado");
}

/**
 * @brief Escreve em um registrador do MMC1
 *
 * @param context Ponteiro para o contexto do mapper
 * @param address Endereço de escrita ($8000-$FFFF)
 * @param value Valor a ser escrito
 */
static void mapper1_write_register(mapper1_context_t *context, uint16_t address, uint8_t value)
{
    // Ao escrever com bit 7 definido, reseta o registrador de deslocamento
    if (value & 0x80)
    {
        context->shift_register = 0x10; // bit 4 set (reset flag)
        context->shift_count = 0;
        context->registers[MMC1_REG_CONTROL] |= 0x0C; // Modo fixo para primeira e última banco
        return;
    }

    // Escreve o próximo bit no registrador de deslocamento
    if (value & 0x01)
    {
        context->shift_register |= (1 << context->shift_count);
    }
    context->shift_count++;

    // Se completamos 5 bits, carregamos o registrador correspondente
    if (context->shift_count == 5)
    {
        // Seleciona o registrador baseado no endereço
        uint8_t reg_index;
        if (address <= 0x9FFF)
        {
            reg_index = MMC1_REG_CONTROL;
        }
        else if (address <= 0xBFFF)
        {
            reg_index = MMC1_REG_CHR_BANK0;
        }
        else if (address <= 0xDFFF)
        {
            reg_index = MMC1_REG_CHR_BANK1;
        }
        else
        {
            reg_index = MMC1_REG_PRG_BANK;
        }

        // Atualiza o registrador selecionado
        context->registers[reg_index] = context->shift_register & 0x1F;

        // Se atualizou o registrador de controle, atualiza o modo de espelhamento
        if (reg_index == MMC1_REG_CONTROL)
        {
            uint8_t mirror_bits = context->registers[MMC1_REG_CONTROL] & MMC1_CTRL_MIRROR_MASK;
            switch (mirror_bits)
            {
            case MMC1_CTRL_MIRROR_HORIZONTAL:
                context->mirror_mode = NES_MIRROR_HORIZONTAL;
                break;
            case MMC1_CTRL_MIRROR_VERTICAL:
                context->mirror_mode = NES_MIRROR_VERTICAL;
                break;
            case MMC1_CTRL_MIRROR_ONE_LOW:
                context->mirror_mode = NES_MIRROR_ONE_SCREEN_LOWER;
                break;
            case MMC1_CTRL_MIRROR_ONE_HIGH:
                context->mirror_mode = NES_MIRROR_ONE_SCREEN_UPPER;
                break;
            }
        }

        // Reseta o registrador de deslocamento
        context->shift_register = 0;
        context->shift_count = 0;
    }
}

/**
 * @brief Lê do espaço de endereçamento da CPU (0x8000-0xFFFF)
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de leitura (0x8000-0xFFFF)
 * @return uint8_t Valor lido
 */
static uint8_t mapper1_cpu_read(nes_mapper_t *mapper, uint16_t address)
{
    if (!mapper || !mapper->context)
    {
        return 0;
    }

    mapper1_context_t *context = (mapper1_context_t *)mapper->context;

    if (address < 0x8000)
    {
        LOG_ERROR(MAPPER1_LOG_CAT, "Tentativa de leitura inválida: $%04X", address);
        return 0;
    }

    uint8_t prg_mode = (context->registers[MMC1_REG_CONTROL] & MMC1_CTRL_PRG_BANK_MODE) >> 2;
    uint8_t prg_bank = context->registers[MMC1_REG_PRG_BANK] & 0x0F;
    uint8_t bank;
    uint32_t offset;

    if (address >= 0x8000 && address <= 0xBFFF)
    {
        // Região do primeiro banco de 16KB (0x8000-0xBFFF)
        switch (prg_mode)
        {
        case 0:                   // Modo 32KB
        case 1:                   // Modo 32KB
            bank = prg_bank & ~1; // Ignora bit 0, bancos concatenados de 32KB
            offset = (bank * NES_PRG_ROM_BANK_SIZE) + (address - 0x8000);
            break;
        case 2: // Fixo em 0
            offset = (address - 0x8000);
            break;
        case 3: // Banco selecionado
            offset = (prg_bank * NES_PRG_ROM_BANK_SIZE) + (address - 0x8000);
            break;
        default:
            offset = (address - 0x8000);
            break;
        }
    }
    else
    {
        // Região do segundo banco de 16KB (0xC000-0xFFFF)
        switch (prg_mode)
        {
        case 0:                         // Modo 32KB
        case 1:                         // Modo 32KB
            bank = (prg_bank & ~1) + 1; // Segunda metade do par de 32KB
            offset = (bank * NES_PRG_ROM_BANK_SIZE) + (address - 0xC000);
            break;
        case 2: // Banco selecionado
            offset = (prg_bank * NES_PRG_ROM_BANK_SIZE) + (address - 0xC000);
            break;
        case 3: // Fixo no último banco
            bank = context->prg_rom_banks - 1;
            offset = (bank * NES_PRG_ROM_BANK_SIZE) + (address - 0xC000);
            break;
        default:
            bank = context->prg_rom_banks - 1;
            offset = (bank * NES_PRG_ROM_BANK_SIZE) + (address - 0xC000);
            break;
        }
    }

    // Assegura que o offset está dentro do limite do PRG ROM
    if (offset >= context->cartridge->prg_rom_size)
    {
        offset %= context->cartridge->prg_rom_size;
    }

    return context->prg_rom[offset];
}

/**
 * @brief Escreve no espaço de endereçamento da CPU (0x8000-0xFFFF)
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de escrita (0x8000-0xFFFF)
 * @param value Valor a ser escrito
 */
static void mapper1_cpu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value)
{
    if (!mapper || !mapper->context)
    {
        return;
    }

    mapper1_context_t *context = (mapper1_context_t *)mapper->context;

    if (address < 0x8000)
    {
        LOG_ERROR(MAPPER1_LOG_CAT, "Tentativa de escrita inválida: $%04X = $%02X", address, value);
        return;
    }

    // Qualquer escrita no espaço 0x8000-0xFFFF é direcionada para os registradores
    mapper1_write_register(context, address, value);
}

/**
 * @brief Lê do espaço de endereçamento da PPU (0x0000-0x1FFF)
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de leitura (0x0000-0x1FFF)
 * @return uint8_t Valor lido
 */
static uint8_t mapper1_ppu_read(nes_mapper_t *mapper, uint16_t address)
{
    if (!mapper || !mapper->context)
    {
        return 0;
    }

    mapper1_context_t *context = (mapper1_context_t *)mapper->context;

    if (address >= 0x2000)
    {
        LOG_ERROR(MAPPER1_LOG_CAT, "Tentativa de leitura PPU inválida: $%04X", address);
        return 0;
    }

    uint8_t chr_mode = (context->registers[MMC1_REG_CONTROL] & MMC1_CTRL_CHR_BANK_MODE) >> 4;
    uint32_t offset;

    if (chr_mode == 0)
    {
        // Modo 8KB (ignorando bit 0 do seletor de banco)
        uint8_t bank = context->registers[MMC1_REG_CHR_BANK0] & 0x1E;
        offset = (bank * 4096) + address;
    }
    else
    {
        // Modo 4KB
        if (address < 0x1000)
        {
            // Primeira metade
            uint8_t bank = context->registers[MMC1_REG_CHR_BANK0];
            offset = (bank * 4096) + address;
        }
        else
        {
            // Segunda metade
            uint8_t bank = context->registers[MMC1_REG_CHR_BANK1];
            offset = (bank * 4096) + (address - 0x1000);
        }
    }

    // Assegura que o offset está dentro do limite
    if (!context->chr_is_ram && offset >= context->cartridge->chr_rom_size)
    {
        offset %= context->cartridge->chr_rom_size;
    }
    else if (context->chr_is_ram && offset >= NES_CHR_RAM_SIZE)
    {
        offset %= NES_CHR_RAM_SIZE;
    }

    return context->chr_rom[offset];
}

/**
 * @brief Escreve no espaço de endereçamento da PPU (0x0000-0x1FFF)
 *
 * @param mapper Ponteiro para o mapper
 * @param address Endereço de escrita (0x0000-0x1FFF)
 * @param value Valor a ser escrito
 */
static void mapper1_ppu_write(nes_mapper_t *mapper, uint16_t address, uint8_t value)
{
    if (!mapper || !mapper->context)
    {
        return;
    }

    mapper1_context_t *context = (mapper1_context_t *)mapper->context;

    if (address >= 0x2000)
    {
        LOG_ERROR(MAPPER1_LOG_CAT, "Tentativa de escrita PPU inválida: $%04X = $%02X", address, value);
        return;
    }

    // Só escreve se for CHR RAM
    if (context->chr_is_ram)
    {
        uint8_t chr_mode = (context->registers[MMC1_REG_CONTROL] & MMC1_CTRL_CHR_BANK_MODE) >> 4;
        uint32_t offset;

        if (chr_mode == 0)
        {
            // Modo 8KB (ignorando bit 0 do seletor de banco)
            uint8_t bank = context->registers[MMC1_REG_CHR_BANK0] & 0x1E;
            offset = (bank * 4096) + address;
        }
        else
        {
            // Modo 4KB
            if (address < 0x1000)
            {
                // Primeira metade
                uint8_t bank = context->registers[MMC1_REG_CHR_BANK0];
                offset = (bank * 4096) + address;
            }
            else
            {
                // Segunda metade
                uint8_t bank = context->registers[MMC1_REG_CHR_BANK1];
                offset = (bank * 4096) + (address - 0x1000);
            }
        }

        // Assegura que o offset está dentro do limite
        if (offset < NES_CHR_RAM_SIZE)
        {
            context->chr_rom[offset] = value;
        }
    }
    else
    {
        LOG_WARN(MAPPER1_LOG_CAT, "Tentativa de escrita em CHR ROM: $%04X = $%02X", address, value);
    }
}

/**
 * @brief Obtém o modo de espelhamento
 *
 * @param mapper Ponteiro para o mapper
 * @return nes_mirror_mode_t Modo de espelhamento
 */
static nes_mirror_mode_t mapper1_get_mirror_mode(nes_mapper_t *mapper)
{
    if (!mapper || !mapper->context)
    {
        return NES_MIRROR_HORIZONTAL;
    }

    mapper1_context_t *context = (mapper1_context_t *)mapper->context;
    return context->mirror_mode;
}
