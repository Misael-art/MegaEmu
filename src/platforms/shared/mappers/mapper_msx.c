/**
 * @file mapper_msx.c
 * @brief Implementação do mapeador MSX (usado em jogos convertidos do MSX)
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

// Configurações do mapeador MSX
#define MSX_PAGE_SIZE 0x4000       // 16KB por página
#define MSX_MAX_PAGES 32           // Máximo de 32 páginas (512KB)
#define MSX_RAM_SIZE 0x8000        // 32KB de RAM

// Estrutura específica do mapeador MSX
typedef struct {
    uint8_t control_reg;           // Registrador de controle
    bool ram_enabled;              // RAM habilitada
    uint8_t ram_page;             // Página de RAM atual
    uint8_t bank_regs[4];         // Registradores de banco
} msx_mapper_t;

// Protótipos das funções de operação
static void msx_reset(mapper_base_t *mapper);
static void msx_shutdown(mapper_base_t *mapper);
static uint8_t msx_read(mapper_base_t *mapper, uint16_t addr);
static void msx_write(mapper_base_t *mapper, uint16_t addr, uint8_t value);
static void msx_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value);
static uint8_t msx_get_current_page(mapper_base_t *mapper, uint8_t slot);
static bool msx_has_ram(mapper_base_t *mapper);
static uint8_t *msx_get_ram(mapper_base_t *mapper);
static size_t msx_get_ram_size(mapper_base_t *mapper);
static int msx_save_state(mapper_base_t *mapper, save_state_t *state);
static int msx_load_state(mapper_base_t *mapper, save_state_t *state);

// Interface de operações do mapeador MSX
static const mapper_ops_t msx_ops = {
    .reset = msx_reset,
    .shutdown = msx_shutdown,
    .read = msx_read,
    .write = msx_write,
    .page_select = msx_page_select,
    .get_current_page = msx_get_current_page,
    .has_ram = msx_has_ram,
    .get_ram = msx_get_ram,
    .get_ram_size = msx_get_ram_size,
    .save_state = msx_save_state,
    .load_state = msx_load_state,
    .notify_address = NULL,
    .notify_time = NULL
};

/**
 * @brief Cria uma nova instância do mapeador MSX
 */
mapper_base_t *mapper_msx_create(void) {
    mapper_base_t *mapper = (mapper_base_t *)malloc(sizeof(mapper_base_t));
    if (!mapper) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para mapeador MSX");
        return NULL;
    }

    // Aloca estrutura específica
    msx_mapper_t *msx = (msx_mapper_t *)malloc(sizeof(msx_mapper_t));
    if (!msx) {
        MAPPER_LOG_ERROR("Falha ao alocar memória para dados específicos do mapeador MSX");
        free(mapper);
        return NULL;
    }

    // Inicializa estrutura base
    mapper->ops = &msx_ops;
    mapper->type = MAPPER_MSX;
    mapper->rom_data = NULL;
    mapper->rom_size = 0;
    mapper->ram_data = NULL;
    mapper->ram_size = MSX_RAM_SIZE;
    mapper->extra_data = msx;

    // Reseta o mapeador
    msx_reset(mapper);

    MAPPER_LOG_INFO("Mapeador MSX criado");
    return mapper;
}

/**
 * @brief Reseta o mapeador MSX
 */
static void msx_reset(mapper_base_t *mapper) {
    if (!mapper || !mapper->extra_data) return;

    msx_mapper_t *msx = (msx_mapper_t *)mapper->extra_data;

    // Reseta estado
    msx->control_reg = 0;
    msx->ram_enabled = false;
    msx->ram_page = 0;
    memset(msx->bank_regs, 0, sizeof(msx->bank_regs));

    // Reseta páginas
    memset(mapper->current_page, 0, sizeof(mapper->current_page));

    // Configura páginas iniciais
    if (mapper->rom_data) {
        mapper->pages[0] = mapper->rom_data;
        mapper->pages[1] = mapper->rom_data + MSX_PAGE_SIZE;
        mapper->pages[2] = mapper->rom_data + (2 * MSX_PAGE_SIZE);
        mapper->pages[3] = mapper->rom_data + (3 * MSX_PAGE_SIZE);
    }

    MAPPER_LOG_INFO("Mapeador MSX resetado");
}

/**
 * @brief Finaliza o mapeador MSX
 */
static void msx_shutdown(mapper_base_t *mapper) {
    if (!mapper) return;

    free(mapper->extra_data);
    free(mapper);

    MAPPER_LOG_INFO("Mapeador MSX finalizado");
}

/**
 * @brief Lê um byte da memória mapeada
 */
static uint8_t msx_read(mapper_base_t *mapper, uint16_t addr) {
    if (!mapper || !mapper->rom_data) return 0xFF;

    msx_mapper_t *msx = (msx_mapper_t *)mapper->extra_data;

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 && msx->ram_enabled && mapper->ram_data) {
        uint32_t ram_addr = (msx->ram_page << 14) | (addr & 0x3FFF);
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
static void msx_write(mapper_base_t *mapper, uint16_t addr, uint8_t value) {
    if (!mapper) return;

    msx_mapper_t *msx = (msx_mapper_t *)mapper->extra_data;

    // Registradores de controle (0x4000-0x4003)
    if (addr >= 0x4000 && addr <= 0x4003) {
        uint8_t reg = addr & 0x03;
        msx->bank_regs[reg] = value;
        msx_page_select(mapper, reg, value);
        return;
    }

    // RAM (0x8000-0xBFFF)
    if (addr >= 0x8000 && addr < 0xC000 && msx->ram_enabled && mapper->ram_data) {
        uint32_t ram_addr = (msx->ram_page << 14) | (addr & 0x3FFF);
        if (ram_addr < mapper->ram_size) {
            mapper->ram_data[ram_addr] = value;
        }
    }
}

/**
 * @brief Seleciona uma página de ROM
 */
static void msx_page_select(mapper_base_t *mapper, uint8_t slot, uint8_t value) {
    if (!mapper || !mapper->rom_data) return;

    // Calcula número máximo de páginas
    uint8_t max_pages = (mapper->rom_size + MSX_PAGE_SIZE - 1) / MSX_PAGE_SIZE;
    if (max_pages == 0) return;

    // Ajusta valor da página
    value %= max_pages;

    // Atualiza página atual
    mapper->current_page[slot] = value;
    mapper->pages[slot] = mapper->rom_data + (value * MSX_PAGE_SIZE);

    MAPPER_LOG_TRACE("Página %d selecionada para slot %d", value, slot);
}

/**
 * @brief Obtém a página atual de um slot
 */
static uint8_t msx_get_current_page(mapper_base_t *mapper, uint8_t slot) {
    if (!mapper || slot >= 8) return 0;
    return mapper->current_page[slot];
}

/**
 * @brief Verifica se o mapeador tem RAM
 */
static bool msx_has_ram(mapper_base_t *mapper) {
    return mapper && mapper->ram_data && mapper->ram_size > 0;
}

/**
 * @brief Obtém ponteiro para a RAM
 */
static uint8_t *msx_get_ram(mapper_base_t *mapper) {
    return mapper ? mapper->ram_data : NULL;
}

/**
 * @brief Obtém tamanho da RAM
 */
static size_t msx_get_ram_size(mapper_base_t *mapper) {
    return mapper ? mapper->ram_size : 0;
}

/**
 * @brief Salva o estado do mapeador
 */
static int msx_save_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    msx_mapper_t *msx = (msx_mapper_t *)mapper->extra_data;

    // Salva estado específico do mapeador MSX
    save_state_register_field(state, "msx_control_reg",
                            &msx->control_reg, sizeof(msx->control_reg));
    save_state_register_field(state, "msx_ram_enabled",
                            &msx->ram_enabled, sizeof(msx->ram_enabled));
    save_state_register_field(state, "msx_ram_page",
                            &msx->ram_page, sizeof(msx->ram_page));
    save_state_register_field(state, "msx_bank_regs",
                            msx->bank_regs, sizeof(msx->bank_regs));

    // Salva páginas atuais
    save_state_register_field(state, "msx_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    return 0;
}

/**
 * @brief Carrega o estado do mapeador
 */
static int msx_load_state(mapper_base_t *mapper, save_state_t *state) {
    if (!mapper || !mapper->extra_data || !state) return -1;

    msx_mapper_t *msx = (msx_mapper_t *)mapper->extra_data;

    // Carrega estado específico do mapeador MSX
    save_state_register_field(state, "msx_control_reg",
                            &msx->control_reg, sizeof(msx->control_reg));
    save_state_register_field(state, "msx_ram_enabled",
                            &msx->ram_enabled, sizeof(msx->ram_enabled));
    save_state_register_field(state, "msx_ram_page",
                            &msx->ram_page, sizeof(msx->ram_page));
    save_state_register_field(state, "msx_bank_regs",
                            msx->bank_regs, sizeof(msx->bank_regs));

    // Carrega páginas atuais
    save_state_register_field(state, "msx_current_pages",
                            mapper->current_page, sizeof(mapper->current_page));

    // Reconfigura ponteiros de página
    for (int i = 0; i < 8; i++) {
        if (mapper->rom_data && mapper->current_page[i] < MSX_MAX_PAGES) {
            mapper->pages[i] = mapper->rom_data + (mapper->current_page[i] * MSX_PAGE_SIZE);
        } else {
            mapper->pages[i] = NULL;
        }
    }

    return 0;
}
