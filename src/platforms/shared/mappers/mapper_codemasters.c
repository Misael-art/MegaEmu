/**
 * @file mapper_codemasters.c
 * @brief Implementação do mapeador Codemasters
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

// Configurações do mapeador Codemasters
#define CODEMASTERS_PAGE_SIZE 0x4000  // 16KB por página
#define CODEMASTERS_MAX_PAGES 32      // Máximo de 32 páginas (512KB)

// Estrutura específica do mapeador Codemasters
typedef struct {
    uint8_t control_reg;              // Registrador de controle
    bool ram_enabled;                 // RAM habilitada
} codemasters_mapper_t;

// Protótipos das funções de operação
static void codemasters_reset(mapper_base_t *mapper);
static void codemasters_shutdown(mapper_base_t *mapper);
static uint8_t codemasters_read(mapper_base_t *mapper, uint16_t addr);
static void codemasters_write(mapper_base_t *mapper, uint16_t addr, uint8_t value);
static void codemasters_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value);
static uint8_t codemasters_get_current_page(mapper_base_t *mapper, uint8_t slot);
static bool codemasters_has_ram(mapper_base_t *mapper);
static uint8_t *codemasters_get_ram(mapper_base_t *mapper);
static size_t codemasters_get_ram_size(mapper_base_t *mapper);
static int codemasters_save_state(mapper_base_t *mapper, save_state_t *state);
static int codemasters_load_state(mapper_base_t *mapper, save_state_t *state);

// Interface de operações do mapeador Codemasters
static const mapper_ops_t codemasters_ops = {
    .reset = codemasters_reset,
    .shutdown = codemasters_shutdown,
    .read = codemasters_read,
    .write = codemasters_write,
    .page_select = codemasters_page_select,
    .get_current_page = codemasters_get_current_page,
    .has_ram = codemasters_has_ram,
    .get_ram = codemasters_get_ram,
    .get_ram_size = codemasters_get_ram_size,
    .save_state = codemasters_save_state,
    .load_state = codemasters_load_state,
    .notify_address = NULL,
    .notify_time = NULL
};

/**
 * @brief Cria uma nova instância do mapeador Codemasters
 */
mapper_base_t *mapper_codemasters_create(void) {
    mapper_base_t *mapper = (mapper_base_t *)malloc(sizeof(mapper_base_t));
    if (!mapper) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para mapeador Codemasters");
        return NULL;
    }

    // Aloca estrutura específica
    codemasters_mapper_t *codemasters = (codemasters_mapper_t *)malloc(sizeof(codemasters_mapper_t));
    if (!codemasters) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para dados específicos do mapeador Codemasters");
        free(mapper);
        return NULL;
    }

    // Inicializa estrutura base
    mapper->ops = &codemasters_ops;
    mapper->type = MAPPER_CODEMASTERS;
    mapper->rom_data = NULL;
    mapper->rom_size = 0;
    mapper->ram_data = NULL;
    mapper->ram_size = 0;
    mapper->extra_data = codemasters;

    // Reseta o mapeador
    codemasters_reset(mapper);

    MAPPER_LOG_INFO("Mapeador Codemasters criado");
    return mapper;
}

/**
 * @brief Reseta o mapeador Codemasters
 */
static void codemasters_reset(mapper_base_t *mapper) {
    if (!mapper || !mapper->extra_data) return;

    codemasters_mapper_t *codemasters = (codemasters_mapper_t *)mapper->extra_data;

    // Reseta estado
    codemasters->control_reg = 0;
    codemasters->ram_enabled = false;

    // Reseta páginas
    memset(mapper->current_page, 0, sizeof(mapper->current_page));

    // Configura páginas iniciais
    if (mapper->rom_data) {
        mapper->pages[0] = mapper->rom_data;
        mapper->pages[1] = mapper->rom_data + CODEMASTERS_PAGE_SIZE;
        mapper->pages[2] = mapper->rom_data + (2 * CODEMASTERS_PAGE_SIZE);
    }

    MAPPER_LOG_INFO("Mapeador Codemasters resetado");
}

/**
 * @brief Finaliza o mapeador Codemasters
 */
static void codemasters_shutdown(mapper_base_t *mapper) {
    if (!mapper) return;

    free(mapper->extra_data);
    free(mapper);

    MAPPER_LOG_INFO("Mapeador Codemasters finalizado");
}

/**
 * @brief Lê um byte da memória mapeada
 */
static uint8_t codemasters_read(mapper_base_t *mapper, uint16_t addr) {
    if (!mapper || !mapper->rom_data) return 0xFF;

    codemasters_mapper_t *codemasters = (codemasters_mapper_t *)mapper->extra_data;

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 && codemasters->ram_enabled && mapper->ram_data) {
        return mapper->ram_data[addr & 0x3FFF];
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
static void codemasters_write(mapper_base_t *mapper, uint16_t addr, uint8_t value) {
    if (!mapper) return;

    codemasters_mapper_t *codemasters = (codemasters_mapper_t *)mapper->extra_data;

    // Registradores de controle (0x0000, 0x4000, 0x8000)
    if ((addr & 0x3FFF) == 0) {
        uint8_t slot = addr >> 14;
        codemasters_page_select(mapper, slot, value);
        return;
    }

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 && codemasters->ram_enabled && mapper->ram_data) {
        mapper->ram_data[addr & 0x3FFF] = value;
    }
}

/**
 * @brief Seleciona uma página de ROM
 */
static void codemasters_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value) {
    if (!mapper || !mapper->rom_data) return;

    // Calcula número máximo de páginas
    uint8_t max_pages = (mapper->rom_size + CODEMASTERS_PAGE_SIZE - 1) / CODEMASTERS_PAGE_SIZE;
    if (max_pages == 0) return;

    // Ajusta valor da página
    value %= max_pages;

    // Atualiza página atual
    mapper->current_page[slot] = value;
    mapper->pages[slot] = mapper->rom_data + (value * CODEMASTERS_PAGE_SIZE);

    MAPPER_LOG_TRACE("Página %d selecionada para slot %d", value, slot);
}

/**
 * @brief Obtém a página atual de um slot
 */
static uint8_t codemasters_get_current_page(mapper_base_t *mapper, uint8_t slot) {
    if (!mapper || slot >= 8) return 0;
    return mapper->current_page[slot];
}

/**
 * @brief Verifica se o mapeador tem RAM
 */
static bool codemasters_has_ram(mapper_base_t *mapper) {
    return mapper && mapper->ram_data && mapper->ram_size > 0;
}

/**
 * @brief Obtém ponteiro para a RAM
 */
static uint8_t *codemasters_get_ram(mapper_base_t *mapper) {
    return mapper ? mapper->ram_data : NULL;
}

/**
 * @brief Obtém tamanho da RAM
 */
static size_t codemasters_get_ram_size(mapper_base_t *mapper) {
    return mapper ? mapper->ram_size : 0;
}

/**
 * @brief Salva o estado do mapeador
 */
static int codemasters_save_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    codemasters_mapper_t *codemasters = (codemasters_mapper_t *)mapper->extra_data;

    // Salva estado específico do mapeador Codemasters
    save_state_register_field(state, "codemasters_control_reg",
                            &codemasters->control_reg, sizeof(codemasters->control_reg));
    save_state_register_field(state, "codemasters_ram_enabled",
                            &codemasters->ram_enabled, sizeof(codemasters->ram_enabled));

    // Salva páginas atuais
    save_state_register_field(state, "codemasters_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    return 0;
}

/**
 * @brief Carrega o estado do mapeador
 */
static int codemasters_load_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    codemasters_mapper_t *codemasters = (codemasters_mapper_t *)mapper->extra_data;

    // Carrega estado específico do mapeador Codemasters
    save_state_register_field(state, "codemasters_control_reg",
                            &codemasters->control_reg, sizeof(codemasters->control_reg));
    save_state_register_field(state, "codemasters_ram_enabled",
                            &codemasters->ram_enabled, sizeof(codemasters->ram_enabled));

    // Carrega páginas atuais
    save_state_register_field(state, "codemasters_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    // Reconfigura ponteiros de página
    for (int i = 0; i < 8; i++) {
        if (mapper->rom_data && mapper->current_page[i] < CODEMASTERS_MAX_PAGES) {
            mapper->pages[i] = mapper->rom_data + (mapper->current_page[i] * CODEMASTERS_PAGE_SIZE);
        } else {
            mapper->pages[i] = NULL;
        }
    }

    return 0;
}
