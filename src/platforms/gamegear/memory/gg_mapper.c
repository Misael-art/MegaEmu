/**
 * @file gg_mapper.c
 * @brief Implementação do sistema de mapeamento de memória do Game Gear
 */

#include "gg_mapper.h"
#include "../cartridge/gg_cartridge.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG_MAPPER EMU_LOG_CAT_MEMORY

// Macros de log
#define GG_MAPPER_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG_MAPPER, __VA_ARGS__)
#define GG_MAPPER_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG_MAPPER, __VA_ARGS__)
#define GG_MAPPER_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG_MAPPER, __VA_ARGS__)
#define GG_MAPPER_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG_MAPPER, __VA_ARGS__)
#define GG_MAPPER_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG_MAPPER, __VA_ARGS__)

// Tamanho da página de ROM (16KB)
#define GG_MAPPER_PAGE_SIZE 0x4000

// Número máximo de páginas de ROM
#define GG_MAPPER_MAX_PAGES 32

/**
 * @brief Estrutura do sistema de mapeamento do Game Gear
 */
struct gg_mapper_t {
    gg_cartridge_t *cart;           // Ponteiro para o cartucho
    gg_mapper_type_t type;          // Tipo do mapeador
    uint8_t *rom_pages[GG_MAPPER_MAX_PAGES]; // Ponteiros para páginas de ROM
    uint8_t current_page;           // Página atual
    uint8_t num_pages;              // Número total de páginas
    bool ram_enabled;               // Flag indicando se a RAM está habilitada
};

/**
 * @brief Detecta o tipo de mapeador baseado no cartucho
 * @param cart Ponteiro para o cartucho
 * @return Tipo do mapeador detectado
 */
static gg_mapper_type_t detect_mapper_type(gg_cartridge_t *cart) {
    // TODO: Implementar detecção baseada em assinaturas conhecidas
    // Por enquanto, assume mapeador padrão Sega
    return GG_MAPPER_SEGA;
}

/**
 * @brief Inicializa as páginas de ROM
 * @param mapper Ponteiro para o mapeador
 * @return true se sucesso, false caso contrário
 */
static bool init_rom_pages(gg_mapper_t *mapper) {
    const uint8_t *rom_data = gg_cartridge_get_rom_data(mapper->cart);
    size_t rom_size = gg_cartridge_get_rom_size(mapper->cart);

    if (!rom_data || rom_size == 0) {
        GG_MAPPER_LOG_ERROR("Dados da ROM inválidos");
        return false;
    }

    // Calcula número de páginas
    mapper->num_pages = (rom_size + GG_MAPPER_PAGE_SIZE - 1) / GG_MAPPER_PAGE_SIZE;
    if (mapper->num_pages > GG_MAPPER_MAX_PAGES) {
        GG_MAPPER_LOG_ERROR("ROM excede o número máximo de páginas");
        return false;
    }

    // Inicializa ponteiros para páginas
    for (int i = 0; i < mapper->num_pages; i++) {
        mapper->rom_pages[i] = (uint8_t *)rom_data + (i * GG_MAPPER_PAGE_SIZE);
    }

    // Preenche páginas restantes com a última página
    for (int i = mapper->num_pages; i < GG_MAPPER_MAX_PAGES; i++) {
        mapper->rom_pages[i] = mapper->rom_pages[mapper->num_pages - 1];
    }

    return true;
}

/**
 * @brief Cria uma nova instância do sistema de mapeamento
 */
gg_mapper_t *gg_mapper_create(gg_cartridge_t *cart) {
    if (!cart) {
        GG_MAPPER_LOG_ERROR("Cartucho inválido");
        return NULL;
    }

    gg_mapper_t *mapper = (gg_mapper_t *)malloc(sizeof(gg_mapper_t));
    if (!mapper) {
        GG_MAPPER_LOG_ERROR("Falha ao alocar memória para mapeador");
        return NULL;
    }

    // Inicializa estrutura
    mapper->cart = cart;
    mapper->type = detect_mapper_type(cart);
    mapper->current_page = 0;
    mapper->ram_enabled = false;

    // Inicializa páginas de ROM
    if (!init_rom_pages(mapper)) {
        free(mapper);
        return NULL;
    }

    GG_MAPPER_LOG_INFO("Sistema de mapeamento do Game Gear criado (tipo: %d)", mapper->type);
    return mapper;
}

/**
 * @brief Destrói uma instância do sistema de mapeamento
 */
void gg_mapper_destroy(gg_mapper_t *mapper) {
    if (!mapper) return;
    free(mapper);
    GG_MAPPER_LOG_INFO("Sistema de mapeamento do Game Gear destruído");
}

/**
 * @brief Reseta o sistema de mapeamento
 */
void gg_mapper_reset(gg_mapper_t *mapper) {
    if (!mapper) return;

    mapper->current_page = 0;
    mapper->ram_enabled = false;

    GG_MAPPER_LOG_INFO("Sistema de mapeamento do Game Gear resetado");
}

/**
 * @brief Lê um byte da memória mapeada
 */
uint8_t gg_mapper_read(gg_mapper_t *mapper, uint16_t addr) {
    if (!mapper) return 0xFF;

    // Páginas fixas (0x0000-0x3FFF)
    if (addr < 0x4000) {
        return mapper->rom_pages[0][addr];
    }

    // Páginas mapeadas (0x4000-0x7FFF)
    if (addr < 0x8000) {
        return mapper->rom_pages[mapper->current_page][addr - 0x4000];
    }

    // SRAM (0x8000-0xBFFF)
    if (addr < 0xC000 && mapper->ram_enabled && gg_cartridge_has_sram(mapper->cart)) {
        const uint8_t *sram = gg_cartridge_get_sram_data(mapper->cart);
        return sram ? sram[addr - 0x8000] : 0xFF;
    }

    return 0xFF;
}

/**
 * @brief Escreve um byte na memória mapeada
 */
void gg_mapper_write(gg_mapper_t *mapper, uint16_t addr, uint8_t value) {
    if (!mapper) return;

    switch (mapper->type) {
        case GG_MAPPER_SEGA:
            // Controle de página (0xFFFC-0xFFFF)
            if (addr >= 0xFFFC) {
                switch (addr & 0x0003) {
                    case 0: // Controle de RAM
                        mapper->ram_enabled = (value & 0x08) != 0;
                        break;

                    case 1: // Seleção de página baixa
                    case 2: // Seleção de página média
                    case 3: // Seleção de página alta
                        mapper->current_page = value % mapper->num_pages;
                        break;
                }
            }
            // SRAM (0x8000-0xBFFF)
            else if (addr >= 0x8000 && addr < 0xC000 &&
                     mapper->ram_enabled && gg_cartridge_has_sram(mapper->cart)) {
                uint8_t *sram = (uint8_t *)gg_cartridge_get_sram_data(mapper->cart);
                if (sram) {
                    sram[addr - 0x8000] = value;
                }
            }
            break;

        case GG_MAPPER_CODEMASTERS:
            // Controle de página (0x0000, 0x4000, 0x8000)
            if ((addr & 0xC000) == 0x0000) {
                mapper->current_page = value % mapper->num_pages;
            }
            break;

        default:
            break;
    }
}

/**
 * @brief Obtém o tipo de mapeador em uso
 */
gg_mapper_type_t gg_mapper_get_type(const gg_mapper_t *mapper) {
    return mapper ? mapper->type : GG_MAPPER_NONE;
}

/**
 * @brief Registra campos do mapeador no sistema de save state
 */
int gg_mapper_register_save_state(gg_mapper_t *mapper, save_state_t *state) {
    if (!mapper || !state) return -1;

    // Registra estado do mapeador
    save_state_register_field(state, "gg_mapper_current_page",
                            &mapper->current_page,
                            sizeof(mapper->current_page));

    save_state_register_field(state, "gg_mapper_ram_enabled",
                            &mapper->ram_enabled,
                            sizeof(mapper->ram_enabled));

    return 0;
}
