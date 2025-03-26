/**
 * @file mapper_korean.c
 * @brief Implementação do mapeador Korean (usado em jogos coreanos)
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

// Configurações do mapeador Korean
#define KOREAN_PAGE_SIZE 0x4000      // 16KB por página
#define KOREAN_MAX_PAGES 32          // Máximo de 32 páginas (512KB)

// Estrutura específica do mapeador Korean
typedef struct {
    uint8_t control_reg;             // Registrador de controle
    bool ram_enabled;                // RAM habilitada
    uint8_t bank_select_mask;        // Máscara de seleção de banco
} korean_mapper_t;

// Protótipos das funções de operação
static void korean_reset(mapper_base_t *mapper);
static void korean_shutdown(mapper_base_t *mapper);
static uint8_t korean_read(mapper_base_t *mapper, uint16_t addr);
static void korean_write(mapper_base_t *mapper, uint16_t addr, uint8_t value);
static void korean_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value);
static uint8_t korean_get_current_page(mapper_base_t *mapper, uint8_t slot);
static bool korean_has_ram(mapper_base_t *mapper);
static uint8_t *korean_get_ram(mapper_base_t *mapper);
static size_t korean_get_ram_size(mapper_base_t *mapper);
static int korean_save_state(mapper_base_t *mapper, save_state_t *state);
static int korean_load_state(mapper_base_t *mapper, save_state_t *state);

// Interface de operações do mapeador Korean
static const mapper_ops_t korean_ops = {
    .reset = korean_reset,
    .shutdown = korean_shutdown,
    .read = korean_read,
    .write = korean_write,
    .page_select = korean_page_select,
    .get_current_page = korean_get_current_page,
    .has_ram = korean_has_ram,
    .get_ram = korean_get_ram,
    .get_ram_size = korean_get_ram_size,
    .save_state = korean_save_state,
    .load_state = korean_load_state,
    .notify_address = NULL,
    .notify_time = NULL
};

/**
 * @brief Cria uma nova instância do mapeador Korean
 */
mapper_base_t *mapper_korean_create(void) {
    mapper_base_t *mapper = (mapper_base_t *)malloc(sizeof(mapper_base_t));
    if (!mapper) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para mapeador Korean");
        return NULL;
    }

    // Aloca estrutura específica
    korean_mapper_t *korean = (korean_mapper_t *)malloc(sizeof(korean_mapper_t));
    if (!korean) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para dados específicos do mapeador Korean");
        free(mapper);
        return NULL;
    }

    // Inicializa estrutura base
    mapper->ops = &korean_ops;
    mapper->type = MAPPER_KOREAN;
    mapper->rom_data = NULL;
    mapper->rom_size = 0;
    mapper->ram_data = NULL;
    mapper->ram_size = 0;
    mapper->extra_data = korean;

    // Reseta o mapeador
    korean_reset(mapper);

    MAPPER_LOG_INFO("Mapeador Korean criado");
    return mapper;
}

/**
 * @brief Reseta o mapeador Korean
 */
static void korean_reset(mapper_base_t *mapper) {
    if (!mapper || !mapper->extra_data) return;

    korean_mapper_t *korean = (korean_mapper_t *)mapper->extra_data;

    // Reseta estado
    korean->control_reg = 0;
    korean->ram_enabled = false;
    korean->bank_select_mask = 0x1F;  // Máscara padrão para 32 bancos

    // Reseta páginas
    memset(mapper->current_page, 0, sizeof(mapper->current_page));

    // Configura páginas iniciais
    if (mapper->rom_data) {
        mapper->pages[0] = mapper->rom_data;
        mapper->pages[1] = mapper->rom_data + KOREAN_PAGE_SIZE;
        mapper->pages[2] = mapper->rom_data + (2 * KOREAN_PAGE_SIZE);
    }

    MAPPER_LOG_INFO("Mapeador Korean resetado");
}

/**
 * @brief Finaliza o mapeador Korean
 */
static void korean_shutdown(mapper_base_t *mapper) {
    if (!mapper) return;

    free(mapper->extra_data);
    free(mapper);

    MAPPER_LOG_INFO("Mapeador Korean finalizado");
}

/**
 * @brief Lê um byte da memória mapeada
 */
static uint8_t korean_read(mapper_base_t *mapper, uint16_t addr) {
    if (!mapper || !mapper->rom_data) return 0xFF;

    korean_mapper_t *korean = (korean_mapper_t *)mapper->extra_data;

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 && korean->ram_enabled && mapper->ram_data) {
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
static void korean_write(mapper_base_t *mapper, uint16_t addr, uint8_t value) {
    if (!mapper) return;

    korean_mapper_t *korean = (korean_mapper_t *)mapper->extra_data;

    // Registradores de controle (0xA000)
    if (addr == 0xA000) {
        korean->control_reg = value;
        korean_page_select(mapper, 2, value & korean->bank_select_mask);
        return;
    }

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 && korean->ram_enabled && mapper->ram_data) {
        mapper->ram_data[addr & 0x3FFF] = value;
    }
}

/**
 * @brief Seleciona uma página de ROM
 */
static void korean_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value) {
    if (!mapper || !mapper->rom_data) return;

    korean_mapper_t *korean = (korean_mapper_t *)mapper->extra_data;

    // Calcula número máximo de páginas
    uint8_t max_pages = (mapper->rom_size + KOREAN_PAGE_SIZE - 1) / KOREAN_PAGE_SIZE;
    if (max_pages == 0) return;

    // Ajusta valor da página usando a máscara
    value &= korean->bank_select_mask;
    value %= max_pages;

    // Atualiza página atual
    mapper->current_page[slot] = value;
    mapper->pages[slot] = mapper->rom_data + (value * KOREAN_PAGE_SIZE);

    MAPPER_LOG_TRACE("Página %d selecionada para slot %d", value, slot);
}

/**
 * @brief Obtém a página atual de um slot
 */
static uint8_t korean_get_current_page(mapper_base_t *mapper, uint8_t slot) {
    if (!mapper || slot >= 8) return 0;
    return mapper->current_page[slot];
}

/**
 * @brief Verifica se o mapeador tem RAM
 */
static bool korean_has_ram(mapper_base_t *mapper) {
    return mapper && mapper->ram_data && mapper->ram_size > 0;
}

/**
 * @brief Obtém ponteiro para a RAM
 */
static uint8_t *korean_get_ram(mapper_base_t *mapper) {
    return mapper ? mapper->ram_data : NULL;
}

/**
 * @brief Obtém tamanho da RAM
 */
static size_t korean_get_ram_size(mapper_base_t *mapper) {
    return mapper ? mapper->ram_size : 0;
}

/**
 * @brief Salva o estado do mapeador
 */
static int korean_save_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    korean_mapper_t *korean = (korean_mapper_t *)mapper->extra_data;

    // Salva estado específico do mapeador Korean
    save_state_register_field(state, "korean_control_reg",
                            &korean->control_reg, sizeof(korean->control_reg));
    save_state_register_field(state, "korean_ram_enabled",
                            &korean->ram_enabled, sizeof(korean->ram_enabled));
    save_state_register_field(state, "korean_bank_select_mask",
                            &korean->bank_select_mask, sizeof(korean->bank_select_mask));

    // Salva páginas atuais
    save_state_register_field(state, "korean_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    return 0;
}

/**
 * @brief Carrega o estado do mapeador
 */
static int korean_load_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    korean_mapper_t *korean = (korean_mapper_t *)mapper->extra_data;

    // Carrega estado específico do mapeador Korean
    save_state_register_field(state, "korean_control_reg",
                            &korean->control_reg, sizeof(korean->control_reg));
    save_state_register_field(state, "korean_ram_enabled",
                            &korean->ram_enabled, sizeof(korean->ram_enabled));
    save_state_register_field(state, "korean_bank_select_mask",
                            &korean->bank_select_mask, sizeof(korean->bank_select_mask));

    // Carrega páginas atuais
    save_state_register_field(state, "korean_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    // Reconfigura ponteiros de página
    for (int i = 0; i < 8; i++) {
        if (mapper->rom_data && mapper->current_page[i] < KOREAN_MAX_PAGES) {
            mapper->pages[i] = mapper->rom_data + (mapper->current_page[i] * KOREAN_PAGE_SIZE);
        } else {
            mapper->pages[i] = NULL;
        }
    }

    return 0;
}
