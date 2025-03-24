#include "mapper0.h"
#include <stdlib.h>
#include <string.h>

struct Mapper0
{
    uint8_t *prg_rom;
    uint8_t *chr_rom;
    size_t prg_size;
    size_t chr_size;
};

Mapper0 *mapper0_create(uint8_t *rom_data, size_t prg_size, size_t chr_size)
{
    if (!rom_data || !prg_size)
        return NULL;

    Mapper0 *mapper = (Mapper0 *)malloc(sizeof(Mapper0));
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

    return mapper;
}

void mapper0_destroy(Mapper0 *mapper)
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

size_t mapper0_get_prg_size(const Mapper0 *mapper)
{
    return mapper ? mapper->prg_size : 0;
}

size_t mapper0_get_chr_size(const Mapper0 *mapper)
{
    return mapper ? mapper->chr_size : 0;
}

uint8_t mapper0_read_prg(const Mapper0 *mapper, uint16_t address)
{
    if (!mapper || !mapper->prg_rom)
        return 0;

    // Endereços válidos: 0x8000-0xFFFF
    if (address < 0x8000)
        return 0;

    // Espelhar se PRG-ROM for 16KB
    uint32_t offset = (address - 0x8000) % mapper->prg_size;
    return mapper->prg_rom[offset];
}

void mapper0_write_prg(Mapper0 *mapper, uint16_t address, uint8_t value)
{
    // NROM não suporta escrita em PRG-ROM
    (void)mapper;
    (void)address;
    (void)value;
}

uint8_t mapper0_read_chr(const Mapper0 *mapper, uint16_t address)
{
    if (!mapper || !mapper->chr_rom || address >= mapper->chr_size)
        return 0;
    return mapper->chr_rom[address];
}

void mapper0_write_chr(Mapper0 *mapper, uint16_t address, uint8_t value)
{
    // NROM não suporta escrita em CHR-ROM
    (void)mapper;
    (void)address;
    (void)value;
}
