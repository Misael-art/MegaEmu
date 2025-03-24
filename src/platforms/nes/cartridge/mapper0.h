#ifndef MAPPER0_H
#define MAPPER0_H

#include <stdint.h>
#include <stddef.h>

// Estrutura opaca para o Mapper 0
typedef struct Mapper0 Mapper0;

// Funções de criação e destruição
Mapper0* mapper0_create(uint8_t* rom_data, size_t prg_size, size_t chr_size);
void mapper0_destroy(Mapper0* mapper);

// Funções de acesso
size_t mapper0_get_prg_size(const Mapper0* mapper);
size_t mapper0_get_chr_size(const Mapper0* mapper);

// Funções de leitura/escrita
uint8_t mapper0_read_prg(const Mapper0* mapper, uint16_t address);
void mapper0_write_prg(Mapper0* mapper, uint16_t address, uint8_t value);
uint8_t mapper0_read_chr(const Mapper0* mapper, uint16_t address);
void mapper0_write_chr(Mapper0* mapper, uint16_t address, uint8_t value);

#endif // MAPPER0_H
