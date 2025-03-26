/**
 * @file gg_cartridge.c
 * @brief Implementação do sistema de cartucho do Game Gear
 */

#include "gg_cartridge.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG_CART EMU_LOG_CAT_CARTRIDGE

// Macros de log
#define GG_CART_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG_CART, __VA_ARGS__)
#define GG_CART_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG_CART, __VA_ARGS__)
#define GG_CART_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG_CART, __VA_ARGS__)
#define GG_CART_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG_CART, __VA_ARGS__)
#define GG_CART_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG_CART, __VA_ARGS__)

// Tamanho máximo da ROM (512KB)
#define GG_CARTRIDGE_MAX_ROM_SIZE (512 * 1024)

// Tamanho da SRAM (8KB)
#define GG_CARTRIDGE_SRAM_SIZE (8 * 1024)

// String mágica para identificar cartuchos Game Gear
#define GG_CARTRIDGE_MAGIC "TMR SEGA"

/**
 * @brief Estrutura do sistema de cartucho do Game Gear
 */
struct gg_cartridge_t {
    uint8_t *rom_data;              // Dados da ROM
    size_t rom_size;                // Tamanho da ROM
    gg_cartridge_header_t header;   // Cabeçalho do cartucho
    uint8_t *sram_data;            // Dados da SRAM
    bool has_sram;                 // Flag indicando suporte a SRAM
};

/**
 * @brief Verifica se um cartucho é válido
 * @param data Ponteiro para os dados do cartucho
 * @param size Tamanho dos dados
 * @return true se o cartucho é válido, false caso contrário
 */
static bool validate_cartridge(const uint8_t *data, size_t size) {
    if (!data || size < GG_CARTRIDGE_HEADER_SIZE) {
        GG_CART_LOG_ERROR("Dados do cartucho inválidos");
        return false;
    }

    // Verifica string mágica
    if (memcmp(data, GG_CARTRIDGE_MAGIC, strlen(GG_CARTRIDGE_MAGIC)) != 0) {
        GG_CART_LOG_ERROR("String mágica do cartucho inválida");
        return false;
    }

    // Verifica tamanho máximo
    if (size > GG_CARTRIDGE_MAX_ROM_SIZE) {
        GG_CART_LOG_ERROR("Tamanho da ROM excede o máximo suportado");
        return false;
    }

    return true;
}

/**
 * @brief Detecta se o cartucho tem suporte a SRAM
 * @param data Ponteiro para os dados do cartucho
 * @param size Tamanho dos dados
 * @return true se tem suporte a SRAM, false caso contrário
 */
static bool detect_sram(const uint8_t *data, size_t size) {
    // TODO: Implementar detecção de SRAM baseada em assinaturas conhecidas
    // Por enquanto, assume que não há SRAM
    return false;
}

/**
 * @brief Cria uma nova instância do sistema de cartucho
 */
gg_cartridge_t *gg_cartridge_create(void) {
    gg_cartridge_t *cart = (gg_cartridge_t *)malloc(sizeof(gg_cartridge_t));
    if (!cart) {
        GG_CART_LOG_ERROR("Falha ao alocar memória para cartucho");
        return NULL;
    }

    // Inicializa estrutura
    memset(cart, 0, sizeof(gg_cartridge_t));

    GG_CART_LOG_INFO("Sistema de cartucho do Game Gear criado");
    return cart;
}

/**
 * @brief Destrói uma instância do sistema de cartucho
 */
void gg_cartridge_destroy(gg_cartridge_t *cart) {
    if (!cart) return;

    // Libera memória da ROM
    if (cart->rom_data) {
        free(cart->rom_data);
    }

    // Libera memória da SRAM
    if (cart->sram_data) {
        free(cart->sram_data);
    }

    free(cart);
    GG_CART_LOG_INFO("Sistema de cartucho do Game Gear destruído");
}

/**
 * @brief Carrega uma ROM no cartucho
 */
bool gg_cartridge_load_rom(gg_cartridge_t *cart, const uint8_t *data, size_t size) {
    if (!cart || !data || size == 0) {
        GG_CART_LOG_ERROR("Parâmetros inválidos para carregar ROM");
        return false;
    }

    // Valida cartucho
    if (!validate_cartridge(data, size)) {
        return false;
    }

    // Libera ROM anterior se existir
    if (cart->rom_data) {
        free(cart->rom_data);
        cart->rom_data = NULL;
        cart->rom_size = 0;
    }

    // Aloca memória para a nova ROM
    cart->rom_data = (uint8_t *)malloc(size);
    if (!cart->rom_data) {
        GG_CART_LOG_ERROR("Falha ao alocar memória para ROM");
        return false;
    }

    // Copia dados da ROM
    memcpy(cart->rom_data, data, size);
    cart->rom_size = size;

    // Copia cabeçalho
    memcpy(&cart->header, data, GG_CARTRIDGE_HEADER_SIZE);

    // Detecta suporte a SRAM
    cart->has_sram = detect_sram(data, size);

    // Aloca SRAM se necessário
    if (cart->has_sram) {
        cart->sram_data = (uint8_t *)malloc(GG_CARTRIDGE_SRAM_SIZE);
        if (!cart->sram_data) {
            GG_CART_LOG_ERROR("Falha ao alocar memória para SRAM");
            free(cart->rom_data);
            cart->rom_data = NULL;
            cart->rom_size = 0;
            cart->has_sram = false;
            return false;
        }
        memset(cart->sram_data, 0xFF, GG_CARTRIDGE_SRAM_SIZE);
    }

    GG_CART_LOG_INFO("ROM carregada: %zu bytes", size);
    return true;
}

/**
 * @brief Obtém o cabeçalho do cartucho
 */
const gg_cartridge_header_t *gg_cartridge_get_header(const gg_cartridge_t *cart) {
    if (!cart || !cart->rom_data) return NULL;
    return &cart->header;
}

/**
 * @brief Obtém o tamanho da ROM em bytes
 */
size_t gg_cartridge_get_rom_size(const gg_cartridge_t *cart) {
    return cart ? cart->rom_size : 0;
}

/**
 * @brief Obtém um ponteiro para os dados da ROM
 */
const uint8_t *gg_cartridge_get_rom_data(const gg_cartridge_t *cart) {
    return cart ? cart->rom_data : NULL;
}

/**
 * @brief Verifica se o cartucho tem uma ROM carregada
 */
bool gg_cartridge_has_rom(const gg_cartridge_t *cart) {
    return cart && cart->rom_data && cart->rom_size > 0;
}

/**
 * @brief Verifica se o cartucho tem suporte a SRAM
 */
bool gg_cartridge_has_sram(const gg_cartridge_t *cart) {
    return cart ? cart->has_sram : false;
}

/**
 * @brief Obtém o tamanho da SRAM em bytes
 */
size_t gg_cartridge_get_sram_size(const gg_cartridge_t *cart) {
    return (cart && cart->has_sram) ? GG_CARTRIDGE_SRAM_SIZE : 0;
}

/**
 * @brief Carrega dados na SRAM do cartucho
 */
bool gg_cartridge_load_sram(gg_cartridge_t *cart, const uint8_t *data, size_t size) {
    if (!cart || !cart->has_sram || !data || size != GG_CARTRIDGE_SRAM_SIZE) {
        GG_CART_LOG_ERROR("Parâmetros inválidos para carregar SRAM");
        return false;
    }

    memcpy(cart->sram_data, data, GG_CARTRIDGE_SRAM_SIZE);
    GG_CART_LOG_INFO("SRAM carregada: %zu bytes", size);
    return true;
}

/**
 * @brief Obtém um ponteiro para os dados da SRAM
 */
const uint8_t *gg_cartridge_get_sram_data(const gg_cartridge_t *cart) {
    return (cart && cart->has_sram) ? cart->sram_data : NULL;
}

/**
 * @brief Registra campos do cartucho no sistema de save state
 */
int gg_cartridge_register_save_state(gg_cartridge_t *cart, save_state_t *state) {
    if (!cart || !state) return -1;

    // Registra dados da SRAM se presente
    if (cart->has_sram) {
        save_state_register_field(state, "gg_cart_sram",
                                cart->sram_data,
                                GG_CARTRIDGE_SRAM_SIZE);
    }

    return 0;
}
