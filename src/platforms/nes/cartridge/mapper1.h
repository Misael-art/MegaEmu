#ifndef MAPPER1_H
#define MAPPER1_H

#include <stdint.h>
#include <stddef.h>

// Estrutura opaca para o Mapper 1
typedef struct Mapper1 Mapper1;

// Funções de criação e destruição
Mapper1 *mapper1_create(uint8_t *rom_data, size_t prg_size, size_t chr_size);
void mapper1_destroy(Mapper1 *mapper);

// Funções de acesso
size_t mapper1_get_prg_size(const Mapper1 *mapper);
size_t mapper1_get_chr_size(const Mapper1 *mapper);
uint8_t mapper1_get_control(const Mapper1 *mapper);

// Funções de leitura/escrita
uint8_t mapper1_read_prg(const Mapper1 *mapper, uint16_t address);
void mapper1_write_prg(Mapper1 *mapper, uint16_t address, uint8_t value);
uint8_t mapper1_read_chr(const Mapper1 *mapper, uint16_t address);
void mapper1_write_chr(Mapper1 *mapper, uint16_t address, uint8_t value);

#endif // MAPPER1_H
