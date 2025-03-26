/**
 * @file mapper_nemesis.c
 * @brief Implementação do mapeador Nemesis (usado em jogos da série Nemesis/Gradius)
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

// Configurações do mapeador Nemesis
#define NEMESIS_PAGE_SIZE 0x2000    // 8KB por página
#define NEMESIS_MAX_PAGES 64        // Máximo de 64 páginas (512KB)
#define NEMESIS_RAM_SIZE 0x4000     // 16KB de RAM

// Estrutura específica do mapeador Nemesis
typedef struct {
    uint8_t control_reg;            // Registrador de controle
    bool ram_enabled;               // RAM habilitada
    uint8_t ram_page;              // Página de RAM atual
    uint8_t bank_regs[8];          // Registradores de banco
} nemesis_mapper_t;

// Protótipos das funções de operação
static void nemesis_reset(mapper_base_t *mapper);
static void nemesis_shutdown(mapper_base_t *mapper);
static uint8_t nemesis_read(mapper_base_t *mapper, uint16_t addr);
static void nemesis_write(mapper_base_t *mapper, uint16_t addr, uint8_t value);
static void nemesis_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value);
static uint8_t nemesis_get_current_page(mapper_base_t *mapper, uint8_t slot);
static bool nemesis_has_ram(mapper_base_t *mapper);
static uint8_t *nemesis_get_ram(mapper_base_t *mapper);
static size_t nemesis_get_ram_size(mapper_base_t *mapper);
static int nemesis_save_state(mapper_base_t *mapper, save_state_t *state);
static int nemesis_load_state(mapper_base_t *mapper, save_state_t *state);

// Interface de operações do mapeador Nemesis
static const mapper_ops_t nemesis_ops = {
    .reset = nemesis_reset,
    .shutdown = nemesis_shutdown,
    .read = nemesis_read,
    .write = nemesis_write,
    .page_select = nemesis_page_select,
    .get_current_page = nemesis_get_current_page,
    .has_ram = nemesis_has_ram,
    .get_ram = nemesis_get_ram,
    .get_ram_size = nemesis_get_ram_size,
    .save_state = nemesis_save_state,
    .load_state = nemesis_load_state,
    .notify_address = NULL,
    .notify_time = NULL
};

/**
 * @brief Cria uma nova instância do mapeador Nemesis
 */
mapper_base_t *mapper_nemesis_create(void) {
    mapper_base_t *mapper = (mapper_base_t *)malloc(sizeof(mapper_base_t));
    if (!mapper) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para mapeador Nemesis");
        return NULL;
    }

    // Aloca estrutura específica
    nemesis_mapper_t *nemesis = (nemesis_mapper_t *)malloc(sizeof(nemesis_mapper_t));
    if (!nemesis) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para dados específicos do mapeador Nemesis");
        free(mapper);
        return NULL;
    }

    // Inicializa estrutura base
    mapper->ops = &nemesis_ops;
    mapper->type = MAPPER_NEMESIS;
    mapper->rom_data = NULL;
    mapper->rom_size = 0;
    mapper->ram_data = NULL;
    mapper->ram_size = NEMESIS_RAM_SIZE;
    mapper->extra_data = nemesis;

    // Reseta o mapeador
    nemesis_reset(mapper);

    MAPPER_LOG_INFO("Mapeador Nemesis criado");
    return mapper;
}

/**
 * @brief Reseta o mapeador Nemesis
 */
static void nemesis_reset(mapper_base_t *mapper) {
    if (!mapper || !mapper->extra_data) return;

    nemesis_mapper_t *nemesis = (nemesis_mapper_t *)mapper->extra_data;

    // Reseta estado
    nemesis->control_reg = 0;
    nemesis->ram_enabled = false;
    nemesis->ram_page = 0;
    memset(nemesis->bank_regs, 0, sizeof(nemesis->bank_regs));

    // Reseta páginas
    memset(mapper->current_page, 0, sizeof(mapper->current_page));

    // Configura páginas iniciais
    if (mapper->rom_data) {
        for (int i = 0; i < 8; i++) {
            mapper->pages[i] = mapper->rom_data + (i * NEMESIS_PAGE_SIZE);
        }
    }

    MAPPER_LOG_INFO("Mapeador Nemesis resetado");
}

/**
 * @brief Finaliza o mapeador Nemesis
 */
static void nemesis_shutdown(mapper_base_t *mapper) {
    if (!mapper) return;

    free(mapper->extra_data);
    free(mapper);

    MAPPER_LOG_INFO("Mapeador Nemesis finalizado");
}

/**
 * @brief Lê um byte da memória mapeada
 */
static uint8_t nemesis_read(mapper_base_t *mapper, uint16_t addr) {
    if (!mapper || !mapper->rom_data) return 0xFF;

    nemesis_mapper_t *nemesis = (nemesis_mapper_t *)mapper->extra_data;

    // RAM (0xC000-0xFFFF)
    if (addr >= 0xC000 && nemesis->ram_enabled && mapper->ram_data) {
        uint32_t ram_addr = (nemesis->ram_page << 13) | (addr & 0x1FFF);
        if (ram_addr < mapper->ram_size) {
            return mapper->ram_data[ram_addr];
        }
        return 0xFF;
    }

    // ROM
    uint8_t page = (addr >> 13) & 0x07;  // Divide por 8KB
    if (mapper->pages[page]) {
        return mapper->pages[page][addr & 0x1FFF];
    }

    return 0xFF;
}

/**
 * @brief Escreve um byte na memória mapeada
 */
static void nemesis_write(mapper_base_t *mapper, uint16_t addr, uint8_t value) {
    if (!mapper) return;

    nemesis_mapper_t *nemesis = (nemesis_mapper_t *)mapper->extra_data;

    // Registradores de controle (0x8000-0x9FFF)
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        uint8_t reg = (addr >> 11) & 0x07;
        nemesis->bank_regs[reg] = value;
        nemesis_page_select(mapper, reg, value);
        return;
    }

    // RAM (0xC000-0xFFFF)
    if (addr >= 0xC000 && nemesis->ram_enabled && mapper->ram_data) {
        uint32_t ram_addr = (nemesis->ram_page << 13) | (addr & 0x1FFF);
        if (ram_addr < mapper->ram_size) {
            mapper->ram_data[ram_addr] = value;
        }
    }
}

/**
 * @brief Seleciona uma página de ROM
 */
static void nemesis_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value) {
    if (!mapper || !mapper->rom_data) return;

    // Calcula número máximo de páginas
    uint8_t max_pages = (mapper->rom_size + NEMESIS_PAGE_SIZE - 1) / NEMESIS_PAGE_SIZE;
    if (max_pages == 0) return;

    // Ajusta valor da página
    value %= max_pages;

    // Atualiza página atual
    mapper->current_page[slot] = value;
    mapper->pages[slot] = mapper->rom_data + (value * NEMESIS_PAGE_SIZE);

    MAPPER_LOG_TRACE("Página %d selecionada para slot %d", value, slot);
}

/**
 * @brief Obtém a página atual de um slot
 */
static uint8_t nemesis_get_current_page(mapper_base_t *mapper, uint8_t slot) {
    if (!mapper || slot >= 8) return 0;
    return mapper->current_page[slot];
}

/**
 * @brief Verifica se o mapeador tem RAM
 */
static bool nemesis_has_ram(mapper_base_t *mapper) {
    return mapper && mapper->ram_data && mapper->ram_size > 0;
}

/**
 * @brief Obtém ponteiro para a RAM
 */
static uint8_t *nemesis_get_ram(mapper_base_t *mapper) {
    return mapper ? mapper->ram_data : NULL;
}

/**
 * @brief Obtém tamanho da RAM
 */
static size_t nemesis_get_ram_size(mapper_base_t *mapper) {
    return mapper ? mapper->ram_size : 0;
}

/**
 * @brief Salva o estado do mapeador
 */
static int nemesis_save_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    nemesis_mapper_t *nemesis = (nemesis_mapper_t *)mapper->extra_data;

    // Salva estado específico do mapeador Nemesis
    save_state_register_field(state, "nemesis_control_reg",
                            &nemesis->control_reg, sizeof(nemesis->control_reg));
    save_state_register_field(state, "nemesis_ram_enabled",
                            &nemesis->ram_enabled, sizeof(nemesis->ram_enabled));
    save_state_register_field(state, "nemesis_ram_page",
                            &nemesis->ram_page, sizeof(nemesis->ram_page));
    save_state_register_field(state, "nemesis_bank_regs",
                            nemesis->bank_regs, sizeof(nemesis->bank_regs));

    // Salva páginas atuais
    save_state_register_field(state, "nemesis_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    return 0;
}

/**
 * @brief Carrega o estado do mapeador
 */
static int nemesis_load_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    nemesis_mapper_t *nemesis = (nemesis_mapper_t *)mapper->extra_data;

    // Carrega estado específico do mapeador Nemesis
    save_state_register_field(state, "nemesis_control_reg",
                            &nemesis->control_reg, sizeof(nemesis->control_reg));
    save_state_register_field(state, "nemesis_ram_enabled",
                            &nemesis->ram_enabled, sizeof(nemesis->ram_enabled));
    save_state_register_field(state, "nemesis_ram_page",
                            &nemesis->ram_page, sizeof(nemesis->ram_page));
    save_state_register_field(state, "nemesis_bank_regs",
                            nemesis->bank_regs, sizeof(nemesis->bank_regs));

    // Carrega páginas atuais
    save_state_register_field(state, "nemesis_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    // Reconfigura ponteiros de página
    for (int i = 0; i < 8; i++) {
        if (mapper->rom_data && mapper->current_page[i] < NEMESIS_MAX_PAGES) {
            mapper->pages[i] = mapper->rom_data + (mapper->current_page[i] * NEMESIS_PAGE_SIZE);
        } else {
            mapper->pages[i] = NULL;
        }
    }

    return 0;
}
