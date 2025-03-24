#include "mapper1.h"
#include <stdlib.h>
#include <string.h>

// Registradores do MMC1
#define MMC1_CONTROL 0
#define MMC1_CHR_BANK_0 1
#define MMC1_CHR_BANK_1 2
#define MMC1_PRG_BANK 3

struct Mapper1
{
    uint8_t *prg_rom;
    uint8_t *chr_rom;
    size_t prg_size;
    size_t chr_size;

    // Registradores
    uint8_t registers[4];
    uint8_t shift_register;
    uint8_t shift_count;

    // Estado do banco
    uint8_t prg_bank;
    uint8_t chr_bank_0;
    uint8_t chr_bank_1;
};

Mapper1 *mapper1_create(uint8_t *rom_data, size_t prg_size, size_t chr_size)
{
    if (!rom_data || !prg_size)
        return NULL;

    Mapper1 *mapper = (Mapper1 *)malloc(sizeof(Mapper1));
    if (!mapper)
        return NULL;

    // Alocar e copiar PRG-ROM
    mapper->prg_rom = (uint8_t *)malloc(prg_size);
    if (!mapper->prg_rom)
    {
        free(mapper);
        return NULL;
    }
    memcpy(mapper->prg_rom, rom_data, prg_size);
    mapper->prg_size = prg_size;

    // Alocar e copiar CHR-ROM se presente
    if (chr_size > 0)
    {
        mapper->chr_rom = (uint8_t *)malloc(chr_size);
        if (!mapper->chr_rom)
        {
            free(mapper->prg_rom);
            free(mapper);
            return NULL;
        }
        memcpy(mapper->chr_rom, rom_data + prg_size, chr_size);
    }
    else
    {
        mapper->chr_rom = NULL;
    }
    mapper->chr_size = chr_size;

    // Inicializar registradores
    memset(mapper->registers, 0, sizeof(mapper->registers));
    mapper->shift_register = 0;
    mapper->shift_count = 0;

    // Estado inicial dos bancos
    mapper->prg_bank = 0;
    mapper->chr_bank_0 = 0;
    mapper->chr_bank_1 = 0;

    // Configuração inicial
    mapper->registers[MMC1_CONTROL] = 0x0C; // PRG ROM mode 3, CHR ROM mode 0

    return mapper;
}

void mapper1_destroy(Mapper1 *mapper)
{
    if (!mapper)
        return;

    free(mapper->prg_rom);
    if (mapper->chr_rom)
    {
        free(mapper->chr_rom);
    }
    free(mapper);
}

size_t mapper1_get_prg_size(const Mapper1 *mapper)
{
    return mapper ? mapper->prg_size : 0;
}

size_t mapper1_get_chr_size(const Mapper1 *mapper)
{
    return mapper ? mapper->chr_size : 0;
}

uint8_t mapper1_get_control(const Mapper1 *mapper)
{
    return mapper ? mapper->registers[MMC1_CONTROL] : 0;
}

static void update_banks(Mapper1 *mapper)
{
    uint8_t prg_mode = (mapper->registers[MMC1_CONTROL] >> 2) & 0x03;
    uint8_t chr_mode = (mapper->registers[MMC1_CONTROL] >> 4) & 0x01;

    // Atualizar banco PRG
    switch (prg_mode)
    {
    case 0: // Switch 32 KB at $8000, ignoring low bit of bank number
    case 1:
        mapper->prg_bank = mapper->registers[MMC1_PRG_BANK] & 0xFE;
        break;
    case 2: // Fix first bank at $8000 and switch 16 KB bank at $C000
        mapper->prg_bank = 0;
        break;
    case 3: // Fix last bank at $C000 and switch 16 KB bank at $8000
        mapper->prg_bank = mapper->registers[MMC1_PRG_BANK];
        break;
    }

    // Atualizar bancos CHR
    if (chr_mode == 0)
    {
        // Switch 8 KB at a time
        mapper->chr_bank_0 = mapper->registers[MMC1_CHR_BANK_0] & 0xFE;
        mapper->chr_bank_1 = mapper->chr_bank_0 + 1;
    }
    else
    {
        // Switch two separate 4 KB banks
        mapper->chr_bank_0 = mapper->registers[MMC1_CHR_BANK_0];
        mapper->chr_bank_1 = mapper->registers[MMC1_CHR_BANK_1];
    }
}

uint8_t mapper1_read_prg(const Mapper1 *mapper, uint16_t address)
{
    if (!mapper || !mapper->prg_rom || address < 0x8000)
        return 0;

    uint32_t bank_size = 0x4000; // 16KB
    uint32_t bank_mask = (mapper->prg_size / bank_size) - 1;
    uint32_t offset;

    if (address < 0xC000)
    {
        // $8000-$BFFF: First PRG bank
        offset = ((mapper->prg_bank & bank_mask) * bank_size) + (address - 0x8000);
    }
    else
    {
        // $C000-$FFFF: Last PRG bank
        uint32_t last_bank = mapper->prg_size - bank_size;
        offset = last_bank + (address - 0xC000);
    }

    return mapper->prg_rom[offset % mapper->prg_size];
}

void mapper1_write_prg(Mapper1 *mapper, uint16_t address, uint8_t value)
{
    if (!mapper || address < 0x8000)
        return;

    // Reset shift register se bit 7 está setado
    if (value & 0x80)
    {
        mapper->shift_register = 0;
        mapper->shift_count = 0;
        mapper->registers[MMC1_CONTROL] |= 0x0C;
        return;
    }

    // Carregar bit no shift register
    mapper->shift_register = ((mapper->shift_register >> 1) | ((value & 1) << 4));
    mapper->shift_count++;

    // Se 5 bits foram escritos, atualizar registrador apropriado
    if (mapper->shift_count == 5)
    {
        uint8_t reg = (address >> 13) & 0x03;
        mapper->registers[reg] = mapper->shift_register;
        mapper->shift_register = 0;
        mapper->shift_count = 0;

        update_banks(mapper);
    }
}

uint8_t mapper1_read_chr(const Mapper1 *mapper, uint16_t address)
{
    if (!mapper || !mapper->chr_rom || address >= 0x2000)
        return 0;

    uint32_t bank_size = 0x1000; // 4KB
    uint32_t bank_mask = (mapper->chr_size / bank_size) - 1;
    uint32_t offset;

    if (address < 0x1000)
    {
        // $0000-$0FFF: First CHR bank
        offset = ((mapper->chr_bank_0 & bank_mask) * bank_size) + (address - 0x0000);
    }
    else
    {
        // $1000-$1FFF: Second CHR bank
        offset = ((mapper->chr_bank_1 & bank_mask) * bank_size) + (address - 0x1000);
    }

    return mapper->chr_rom[offset % mapper->chr_size];
}

void mapper1_write_chr(Mapper1 *mapper, uint16_t address, uint8_t value)
{
    // MMC1 não suporta escrita em CHR-ROM
    (void)mapper;
    (void)address;
    (void)value;
}
