#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Tipos básicos para endereçamento
typedef uint16_t address_t;
typedef uint8_t byte_t;
typedef uint16_t word_t;

// Tipos para bancos de memória
typedef struct
{
    byte_t *data;
    size_t size;
} MemoryBank;

// Tipos para cartuchos
typedef struct
{
    MemoryBank prg_rom;
    MemoryBank chr_rom;
    uint8_t mapper_number;
} CartridgeInfo;

// Tipos para registradores
typedef union
{
    struct
    {
        uint8_t low;
        uint8_t high;
    };
    uint16_t value;
} Register16;

// Tipos para flags
typedef struct
{
    uint8_t carry : 1;
    uint8_t zero : 1;
    uint8_t interrupt : 1;
    uint8_t decimal : 1;
    uint8_t break_cmd : 1;
    uint8_t unused : 1;
    uint8_t overflow : 1;
    uint8_t negative : 1;
} StatusFlags;

#endif // PLATFORM_TYPES_H
