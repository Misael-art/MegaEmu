/**
 * @file mapper_impl.h
 * @brief Interface para implementações dos mapeadores
 */

#ifndef MAPPER_IMPL_H
#define MAPPER_IMPL_H

#include "mapper_types.h"
#include "../../../core/save_state.h"
#include <stdint.h>
#include <stdbool.h>

// Estrutura base para mapeadores
typedef struct mapper_base_t mapper_base_t;

// Interface de operações do mapeador
typedef struct {
    // Operações básicas
    void (*reset)(mapper_base_t *mapper);
    void (*shutdown)(mapper_base_t *mapper);

    // Operações de memória
    uint8_t (*read)(mapper_base_t *mapper, uint16_t addr);
    void (*write)(mapper_base_t *mapper, uint16_t addr, uint8_t value);

    // Operações de paginação
    void (*page_select)(mapper_base_t *mapper, uint8_t page, uint8_t value);
    uint8_t (*get_current_page)(mapper_base_t *mapper, uint8_t slot);

    // Operações de RAM
    bool (*has_ram)(mapper_base_t *mapper);
    uint8_t *(*get_ram)(mapper_base_t *mapper);
    size_t (*get_ram_size)(mapper_base_t *mapper);

    // Operações de save state
    int (*save_state)(mapper_base_t *mapper, save_state_t *state);
    int (*load_state)(mapper_base_t *mapper, save_state_t *state);

    // Operações especiais
    void (*notify_address)(mapper_base_t *mapper, uint16_t addr);
    void (*notify_time)(mapper_base_t *mapper, uint64_t cycles);
} mapper_ops_t;

// Estrutura base do mapeador
struct mapper_base_t {
    const mapper_ops_t *ops;     // Operações do mapeador
    mapper_type_t type;          // Tipo do mapeador
    uint8_t *rom_data;           // Dados da ROM
    size_t rom_size;             // Tamanho da ROM
    uint8_t *ram_data;           // Dados da RAM (se suportado)
    size_t ram_size;             // Tamanho da RAM
    uint8_t *pages[8];           // Ponteiros para páginas mapeadas
    uint8_t current_page[8];     // Páginas atuais
    void *extra_data;            // Dados extras específicos do mapeador
};

// Funções de criação de mapeadores específicos
mapper_base_t *mapper_sega_create(void);
mapper_base_t *mapper_codemasters_create(void);
mapper_base_t *mapper_korean_create(void);
mapper_base_t *mapper_msx_create(void);
mapper_base_t *mapper_93c46_create(void);
mapper_base_t *mapper_4pak_create(void);
mapper_base_t *mapper_castle_create(void);

// Funções utilitárias
const mapper_info_t *mapper_get_info(mapper_type_t type);
mapper_type_t mapper_detect_type(const uint8_t *rom_data, size_t rom_size);
bool mapper_validate_rom(mapper_type_t type, const uint8_t *rom_data, size_t rom_size);

#endif // MAPPER_IMPL_H
