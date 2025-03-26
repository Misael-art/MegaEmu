/**
 * @file mapper_sega.c
 * @brief Implementação do mapeador padrão Sega
 */

#include "mapper_impl.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_MAPPER EMU_LOG_CAT_MEMORY

// Macros de log
#define MAPPER_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MAPPER, __VA_ARGS__)
#define MAPPER_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MAPPER, __VA_ARGS__)

// Configurações do mapeador Sega
#define SEGA_PAGE_SIZE 0x4000      // 16KB por página
#define SEGA_RAM_SIZE 0x8000       // 32KB de RAM (máximo)
#define SEGA_MAX_PAGES 32          // Máximo de 32 páginas (512KB)

// Estrutura específica do mapeador Sega
typedef struct {
    bool ram_enabled;              // RAM habilitada
    bool ram_write_enabled;        // Escrita na RAM habilitada
    uint8_t ram_page;             // Página de RAM atual
    uint8_t control_reg;          // Registrador de controle
    uint8_t ram_control_reg;      // Registrador de controle da RAM
} sega_mapper_t;

// Protótipos das funções de operação
static void sega_reset(mapper_base_t *mapper);
static void sega_shutdown(mapper_base_t *mapper);
static uint8_t sega_read(mapper_base_t *mapper, uint16_t addr);
static void sega_write(mapper_base_t *mapper, uint16_t addr, uint8_t value);
static void sega_page_select(mapper_base_t *mapper, uint8_t page, uint8_t value);
static uint8_t sega_get_current_page(mapper_base_t *mapper, uint8_t slot);
static bool sega_has_ram(mapper_base_t *mapper);
static uint8_t *sega_get_ram(mapper_base_t *mapper);
static size_t sega_get_ram_size(mapper_base_t *mapper);
static int sega_save_state(mapper_base_t *mapper, save_state_t *state);
static int sega_load_state(mapper_base_t *mapper, save_state_t *state);

// Interface de operações do mapeador Sega
static const mapper_ops_t sega_ops = {
    .reset = sega_reset,
    .shutdown = sega_shutdown,
    .read = sega_read,
    .write = sega_write,
    .page_select = sega_page_select,
    .get_current_page = sega_get_current_page,
    .has_ram = sega_has_ram,
    .get_ram = sega_get_ram,
    .get_ram_size = sega_get_ram_size,
    .save_state = sega_save_state,
    .load_state = sega_load_state,
    .notify_address = NULL,
    .notify_time = NULL
};

/**
 * @brief Cria uma nova instância do mapeador Sega
 */
mapper_base_t *mapper_sega_create(void) {
    mapper_base_t *mapper = (mapper_base_t *)malloc(sizeof(mapper_base_t));
    if (!mapper) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para mapeador Sega");
        return NULL;
    }

    // Aloca estrutura específica
    sega_mapper_t *sega = (sega_mapper_t *)malloc(sizeof(sega_mapper_t));
    if (!sega) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para dados específicos do mapeador Sega");
        free(mapper);
        return NULL;
    }

    // Inicializa estrutura base
    mapper->ops = &sega_ops;
    mapper->type = MAPPER_SEGA;
    mapper->rom_data = NULL;
    mapper->rom_size = 0;
    mapper->ram_data = NULL;
    mapper->ram_size = 0;
    mapper->extra_data = sega;

    // Reseta o mapeador
    sega_reset(mapper);

    MAPPER_LOG_INFO("Mapeador Sega criado");
    return mapper;
}

/**
 * @brief Reseta o mapeador Sega
 */
static void sega_reset(mapper_base_t *mapper) {
    if (!mapper || !mapper->extra_data) return;

    sega_mapper_t *sega = (sega_mapper_t *)mapper->extra_data;

    // Reseta estado
    sega->ram_enabled = false;
    sega->ram_write_enabled = false;
    sega->ram_page = 0;
    sega->control_reg = 0;
    sega->ram_control_reg = 0;

    // Reseta páginas
    memset(mapper->current_page, 0, sizeof(mapper->current_page));

    // Configura páginas iniciais
    if (mapper->rom_data) {
        mapper->pages[0] = mapper->rom_data;
        mapper->pages[1] = mapper->rom_data + SEGA_PAGE_SIZE;
        mapper->pages[2] = mapper->rom_data + (2 * SEGA_PAGE_SIZE);
        mapper->pages[3] = mapper->rom_data + (3 * SEGA_PAGE_SIZE);
    }

    MAPPER_LOG_INFO("Mapeador Sega resetado");
}

/**
 * @brief Finaliza o mapeador Sega
 */
static void sega_shutdown(mapper_base_t *mapper) {
    if (!mapper) return;

    free(mapper->extra_data);
    free(mapper);

    MAPPER_LOG_INFO("Mapeador Sega finalizado");
}

/**
 * @brief Lê um byte da memória mapeada
 */
static uint8_t sega_read(mapper_base_t *mapper, uint16_t addr) {
    if (!mapper || !mapper->rom_data) return 0xFF;

    sega_mapper_t *sega = (sega_mapper_t *)mapper->extra_data;

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 && sega->ram_enabled && mapper->ram_data) {
        uint32_t ram_addr = (sega->ram_page << 14) | (addr & 0x3FFF);
        if (ram_addr < mapper->ram_size) {
            return mapper->ram_data[ram_addr];
        }
        return 0xFF;
    }

    // ROM
    uint8_t page = addr >> 14;  // Divide por 16KB
    if (mapper->pages[page]) {
        return mapper->pages[page][addr & 0x3FFF];
    }

    return 0xFF;
}

/**
 * @brief Escreve um byte na memória mapeada
 */
static void sega_write(mapper_base_t *mapper, uint16_t addr, uint8_t value) {
    if (!mapper) return;

    sega_mapper_t *sega = (sega_mapper_t *)mapper->extra_data;

    // Registradores de controle (0xFFFC-0xFFFF)
    if (addr >= 0xFFFC) {
        switch (addr & 0x0003) {
            case 0: // Controle de RAM
                sega->ram_control_reg = value;
                sega->ram_enabled = (value & 0x08) != 0;
                sega->ram_write_enabled = (value & 0x04) != 0;
                break;

            case 1: // Seleção de página baixa
            case 2: // Seleção de página média
            case 3: // Seleção de página alta
                sega_page_select(mapper, addr & 0x03, value);
                break;
        }
        return;
    }

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 &&
        sega->ram_enabled && sega->ram_write_enabled && mapper->ram_data) {
        uint32_t ram_addr = (sega->ram_page << 14) | (addr & 0x3FFF);
        if (ram_addr < mapper->ram_size) {
            mapper->ram_data[ram_addr] = value;
        }
    }
}

/**
 * @brief Seleciona uma página de ROM
 */
static void sega_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value) {
    if (!mapper || !mapper->rom_data) return;

    // Calcula número máximo de páginas
    uint8_t max_pages = (mapper->rom_size + SEGA_PAGE_SIZE - 1) / SEGA_PAGE_SIZE;
    if (max_pages == 0) return;

    // Ajusta valor da página
    value %= max_pages;

    // Atualiza página atual
    mapper->current_page[slot] = value;
    mapper->pages[slot] = mapper->rom_data + (value * SEGA_PAGE_SIZE);

    MAPPER_LOG_TRACE("Página %d selecionada para slot %d", value, slot);
}

/**
 * @brief Obtém a página atual de um slot
 */
static uint8_t sega_get_current_page(mapper_base_t *mapper, uint8_t slot) {
    if (!mapper || slot >= 8) return 0;
    return mapper->current_page[slot];
}

/**
 * @brief Verifica se o mapeador tem RAM
 */
static bool sega_has_ram(mapper_base_t *mapper) {
    return mapper && mapper->ram_data && mapper->ram_size > 0;
}

/**
 * @brief Obtém ponteiro para a RAM
 */
static uint8_t *sega_get_ram(mapper_base_t *mapper) {
    return mapper ? mapper->ram_data : NULL;
}

/**
 * @brief Obtém tamanho da RAM
 */
static size_t sega_get_ram_size(mapper_base_t *mapper) {
    return mapper ? mapper->ram_size : 0;
}

/**
 * @brief Salva o estado do mapeador
 */
static int sega_save_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    sega_mapper_t *sega = (sega_mapper_t *)mapper->extra_data;

    // Salva estado específico do mapeador Sega
    save_state_register_field(state, "sega_ram_enabled",
                            &sega->ram_enabled, sizeof(sega->ram_enabled));
    save_state_register_field(state, "sega_ram_write_enabled",
                            &sega->ram_write_enabled, sizeof(sega->ram_write_enabled));
    save_state_register_field(state, "sega_ram_page",
                            &sega->ram_page, sizeof(sega->ram_page));
    save_state_register_field(state, "sega_control_reg",
                            &sega->control_reg, sizeof(sega->control_reg));
    save_state_register_field(state, "sega_ram_control_reg",
                            &sega->ram_control_reg, sizeof(sega->ram_control_reg));

    // Salva páginas atuais
    save_state_register_field(state, "sega_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    return 0;
}

/**
 * @brief Carrega o estado do mapeador
 */
static int sega_load_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    sega_mapper_t *sega = (sega_mapper_t *)mapper->extra_data;

    // Carrega estado específico do mapeador Sega
    save_state_register_field(state, "sega_ram_enabled",
                            &sega->ram_enabled, sizeof(sega->ram_enabled));
    save_state_register_field(state, "sega_ram_write_enabled",
                            &sega->ram_write_enabled, sizeof(sega->ram_write_enabled));
    save_state_register_field(state, "sega_ram_page",
                            &sega->ram_page, sizeof(sega->ram_page));
    save_state_register_field(state, "sega_control_reg",
                            &sega->control_reg, sizeof(sega->control_reg));
    save_state_register_field(state, "sega_ram_control_reg",
                            &sega->ram_control_reg, sizeof(sega->ram_control_reg));

    // Carrega páginas atuais
    save_state_register_field(state, "sega_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    // Reconfigura ponteiros de página
    for (int i = 0; i < 8; i++) {
        if (mapper->rom_data && mapper->current_page[i] < SEGA_MAX_PAGES) {
            mapper->pages[i] = mapper->rom_data + (mapper->current_page[i] * SEGA_PAGE_SIZE);
        } else {
            mapper->pages[i] = NULL;
        }
    }

    return 0;
}
