/**
 * @file cheat.c
 * @brief Implementação do sistema de cheats
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cheat.h"
#include "../logging/log.h"
#include "../utils/file_utils.h"

#define INITIAL_CAPACITY 16
#define INITIAL_FINDER_CAPACITY 1024

// Estrutura de contexto global do sistema de cheats
typedef struct {
    void* memory_interface;
    mega_emu_cheat_platform_t platform;
    mega_emu_cheat_list_t active_list;
    bool initialized;
} cheat_context_t;

// Contexto global
static cheat_context_t g_cheat_ctx = {0};

// Funções de memória
static uint8_t read_memory_8bit(uint32_t address);
static uint16_t read_memory_16bit(uint32_t address);
static uint32_t read_memory_32bit(uint32_t address);
static void write_memory_8bit(uint32_t address, uint8_t value);
static void write_memory_16bit(uint32_t address, uint16_t value);
static void write_memory_32bit(uint32_t address, uint32_t value);

// Inicialização e finalização
bool mega_emu_cheat_init(void* memory, mega_emu_cheat_platform_t platform) {
    // Já inicializado?
    if (g_cheat_ctx.initialized) {
        LOG_WARNING("Sistema de cheats já inicializado.");
        return false;
    }

    // Verifica parâmetros
    if (!memory) {
        LOG_ERROR("Interface de memória inválida.");
        return false;
    }

    // Inicializa contexto
    memset(&g_cheat_ctx, 0, sizeof(cheat_context_t));
    g_cheat_ctx.memory_interface = memory;
    g_cheat_ctx.platform = platform;

    // Inicializa lista de cheats
    g_cheat_ctx.active_list.cheats = (mega_emu_cheat_t*)malloc(INITIAL_CAPACITY * sizeof(mega_emu_cheat_t));
    if (!g_cheat_ctx.active_list.cheats) {
        LOG_ERROR("Falha ao alocar memória para lista de cheats.");
        return false;
    }

    g_cheat_ctx.active_list.capacity = INITIAL_CAPACITY;
    g_cheat_ctx.active_list.cheat_count = 0;

    g_cheat_ctx.initialized = true;
    LOG_INFO("Sistema de cheats inicializado para plataforma %d", platform);

    return true;
}

void mega_emu_cheat_shutdown(void) {
    if (!g_cheat_ctx.initialized) {
        return;
    }

    // Libera recursos
    if (g_cheat_ctx.active_list.cheats) {
        free(g_cheat_ctx.active_list.cheats);
        g_cheat_ctx.active_list.cheats = NULL;
    }

    g_cheat_ctx.initialized = false;
    LOG_INFO("Sistema de cheats finalizado.");
}

// Gerenciamento de cheats
int mega_emu_cheat_add(const mega_emu_cheat_t* cheat) {
    if (!g_cheat_ctx.initialized || !cheat) {
        return -1;
    }

    // Verifica se precisa expandir o array
    if (g_cheat_ctx.active_list.cheat_count >= g_cheat_ctx.active_list.capacity) {
        uint32_t new_capacity = g_cheat_ctx.active_list.capacity * 2;
        mega_emu_cheat_t* new_cheats = (mega_emu_cheat_t*)realloc(
            g_cheat_ctx.active_list.cheats,
            new_capacity * sizeof(mega_emu_cheat_t)
        );

        if (!new_cheats) {
            LOG_ERROR("Falha ao expandir lista de cheats.");
            return -1;
        }

        g_cheat_ctx.active_list.cheats = new_cheats;
        g_cheat_ctx.active_list.capacity = new_capacity;
    }

    // Adiciona o novo cheat
    int index = g_cheat_ctx.active_list.cheat_count;
    memcpy(&g_cheat_ctx.active_list.cheats[index], cheat, sizeof(mega_emu_cheat_t));
    g_cheat_ctx.active_list.cheat_count++;

    LOG_INFO("Cheat adicionado: %s (índice %d)", cheat->name, index);
    return index;
}

bool mega_emu_cheat_remove(uint32_t index) {
    if (!g_cheat_ctx.initialized || index >= g_cheat_ctx.active_list.cheat_count) {
        return false;
    }

    // Remove deslocando todos os elementos
    for (uint32_t i = index; i < g_cheat_ctx.active_list.cheat_count - 1; i++) {
        memcpy(&g_cheat_ctx.active_list.cheats[i],
               &g_cheat_ctx.active_list.cheats[i + 1],
               sizeof(mega_emu_cheat_t));
    }

    g_cheat_ctx.active_list.cheat_count--;
    LOG_INFO("Cheat removido: índice %d", index);

    return true;
}

bool mega_emu_cheat_enable(uint32_t index, bool enabled) {
    if (!g_cheat_ctx.initialized || index >= g_cheat_ctx.active_list.cheat_count) {
        return false;
    }

    g_cheat_ctx.active_list.cheats[index].enabled = enabled;
    LOG_INFO("Cheat %d %s", index, enabled ? "habilitado" : "desabilitado");

    return true;
}

// Aplicação de cheats
uint32_t mega_emu_cheat_apply_all(void) {
    if (!g_cheat_ctx.initialized) {
        return 0;
    }

    uint32_t applied_count = 0;

    for (uint32_t i = 0; i < g_cheat_ctx.active_list.cheat_count; i++) {
        mega_emu_cheat_t* cheat = &g_cheat_ctx.active_list.cheats[i];

        if (!cheat->enabled) {
            continue;
        }

        // Para cheats condicionais, verifica a condição
        if (cheat->is_conditional) {
            uint32_t current_value;

            // Lê o valor atual baseado no tamanho
            switch (cheat->size) {
                case CHEAT_SIZE_8BIT:
                    current_value = read_memory_8bit(cheat->address);
                    break;
                case CHEAT_SIZE_16BIT:
                    current_value = read_memory_16bit(cheat->address);
                    break;
                case CHEAT_SIZE_32BIT:
                    current_value = read_memory_32bit(cheat->address);
                    break;
                case CHEAT_SIZE_24BIT:
                    // 24 bits são tratados como 32 bits com máscara
                    current_value = read_memory_32bit(cheat->address) & 0xFFFFFF;
                    break;
                default:
                    continue;
            }

            // Verifica a condição
            bool condition_met = false;

            switch (cheat->comparator) {
                case CHEAT_CMP_EQUAL:
                    condition_met = (current_value == cheat->compare_value);
                    break;
                case CHEAT_CMP_NOT_EQUAL:
                    condition_met = (current_value != cheat->compare_value);
                    break;
                case CHEAT_CMP_GREATER:
                    condition_met = (current_value > cheat->compare_value);
                    break;
                case CHEAT_CMP_LESS:
                    condition_met = (current_value < cheat->compare_value);
                    break;
                case CHEAT_CMP_GREATER_EQUAL:
                    condition_met = (current_value >= cheat->compare_value);
                    break;
                case CHEAT_CMP_LESS_EQUAL:
                    condition_met = (current_value <= cheat->compare_value);
                    break;
                default:
                    condition_met = false;
            }

            // Se a condição não foi atendida, pula este cheat
            if (!condition_met) {
                continue;
            }
        }

        // Aplica o cheat baseado no tipo
        switch (cheat->type) {
            case CHEAT_TYPE_RAW:
            case CHEAT_TYPE_GAME_GENIE:
            case CHEAT_TYPE_PRO_ACTION_REPLAY:
            case CHEAT_TYPE_GAMESHARK:
                // Todos estes são aplicados da mesma forma
                switch (cheat->size) {
                    case CHEAT_SIZE_8BIT:
                        write_memory_8bit(cheat->address, (uint8_t)cheat->value);
                        break;
                    case CHEAT_SIZE_16BIT:
                        write_memory_16bit(cheat->address, (uint16_t)cheat->value);
                        break;
                    case CHEAT_SIZE_32BIT:
                        write_memory_32bit(cheat->address, cheat->value);
                        break;
                    case CHEAT_SIZE_24BIT: {
                        // 24 bits: preserva byte mais significativo
                        uint32_t current = read_memory_32bit(cheat->address);
                        uint32_t new_value = (current & 0xFF000000) | (cheat->value & 0xFFFFFF);
                        write_memory_32bit(cheat->address, new_value);
                        break;
                    }
                }
                break;

            case CHEAT_TYPE_CONDITIONAL:
                // Já tratamos a condição acima, só aplica o valor
                switch (cheat->size) {
                    case CHEAT_SIZE_8BIT:
                        write_memory_8bit(cheat->address, (uint8_t)cheat->value);
                        break;
                    case CHEAT_SIZE_16BIT:
                        write_memory_16bit(cheat->address, (uint16_t)cheat->value);
                        break;
                    case CHEAT_SIZE_32BIT:
                        write_memory_32bit(cheat->address, cheat->value);
                        break;
                    case CHEAT_SIZE_24BIT: {
                        uint32_t current = read_memory_32bit(cheat->address);
                        uint32_t new_value = (current & 0xFF000000) | (cheat->value & 0xFFFFFF);
                        write_memory_32bit(cheat->address, new_value);
                        break;
                    }
                }
                break;
        }

        applied_count++;
    }

    return applied_count;
}

// Funções auxiliares de acesso à memória
static uint8_t read_memory_8bit(uint32_t address) {
    // Implementação depende da interface de memória de cada plataforma
    // Esta é uma implementação simplificada e genérica
    uint8_t value = 0;

    // TO DO: Usar a interface de memória adequada conforme a plataforma
    // Esta implementação deverá ser expandida para acessar a memória através da interface

    return value;
}

static uint16_t read_memory_16bit(uint32_t address) {
    uint16_t value = read_memory_8bit(address);
    value |= ((uint16_t)read_memory_8bit(address + 1)) << 8;
    return value;
}

static uint32_t read_memory_32bit(uint32_t address) {
    uint32_t value = read_memory_16bit(address);
    value |= ((uint32_t)read_memory_16bit(address + 2)) << 16;
    return value;
}

static void write_memory_8bit(uint32_t address, uint8_t value) {
    // Implementação depende da interface de memória de cada plataforma
    // Esta é uma implementação simplificada e genérica

    // TO DO: Usar a interface de memória adequada conforme a plataforma
    // Esta implementação deverá ser expandida para acessar a memória através da interface
}

static void write_memory_16bit(uint32_t address, uint16_t value) {
    write_memory_8bit(address, (uint8_t)(value & 0xFF));
    write_memory_8bit(address + 1, (uint8_t)((value >> 8) & 0xFF));
}

static void write_memory_32bit(uint32_t address, uint32_t value) {
    write_memory_16bit(address, (uint16_t)(value & 0xFFFF));
    write_memory_16bit(address + 2, (uint16_t)((value >> 16) & 0xFFFF));
}

// Funções de decodificação de códigos
bool mega_emu_cheat_decode_game_genie(const char* code, mega_emu_cheat_platform_t platform, mega_emu_cheat_t* result) {
    if (!code || !result) {
        return false;
    }

    // Limpa o resultado
    memset(result, 0, sizeof(mega_emu_cheat_t));

    // Copia o código original
    strncpy(result->code, code, sizeof(result->code) - 1);
    result->platform = platform;
    result->type = CHEAT_TYPE_GAME_GENIE;

    // A decodificação varia por plataforma
    switch (platform) {
        case CHEAT_PLATFORM_NES:
            // Implementação para NES
            // Formato: AAAA-BBBB ou AAAA-BBBB-CCCC
            // Onde A é endereço, B é valor e C é comparador
            break;

        case CHEAT_PLATFORM_SNES:
            // Implementação para SNES
            break;

        case CHEAT_PLATFORM_MEGADRIVE:
            // Implementação para Mega Drive
            break;

        case CHEAT_PLATFORM_GAMEBOY:
            // Implementação para Game Boy
            break;

        default:
            LOG_ERROR("Plataforma não suportada para códigos Game Genie: %d", platform);
            return false;
    }

    return true;
}

bool mega_emu_cheat_decode_pro_action_replay(const char* code, mega_emu_cheat_platform_t platform, mega_emu_cheat_t* result) {
    if (!code || !result) {
        return false;
    }

    // Limpa o resultado
    memset(result, 0, sizeof(mega_emu_cheat_t));

    // Copia o código original
    strncpy(result->code, code, sizeof(result->code) - 1);
    result->platform = platform;
    result->type = CHEAT_TYPE_PRO_ACTION_REPLAY;

    // A decodificação varia por plataforma
    switch (platform) {
        case CHEAT_PLATFORM_MEGADRIVE:
            // Implementação para Mega Drive
            // Formato: AAAAAAAA:VVVV
            // Onde A é endereço e V é valor
            break;

        case CHEAT_PLATFORM_MASTERSYSTEM:
            // Implementação para Master System
            break;

        case CHEAT_PLATFORM_GAMEGEAR:
            // Implementação para Game Gear
            break;

        default:
            LOG_ERROR("Plataforma não suportada para códigos Pro Action Replay: %d", platform);
            return false;
    }

    return true;
}

// Cheat Finder
mega_emu_cheat_finder_t* mega_emu_cheat_finder_create(void) {
    if (!g_cheat_ctx.initialized) {
        return NULL;
    }

    mega_emu_cheat_finder_t* finder = (mega_emu_cheat_finder_t*)malloc(sizeof(mega_emu_cheat_finder_t));
    if (!finder) {
        LOG_ERROR("Falha ao alocar memória para Cheat Finder.");
        return NULL;
    }

    // Inicializa a estrutura
    memset(finder, 0, sizeof(mega_emu_cheat_finder_t));

    finder->results = (mega_emu_cheat_search_result_t*)malloc(
        INITIAL_FINDER_CAPACITY * sizeof(mega_emu_cheat_search_result_t)
    );

    if (!finder->results) {
        LOG_ERROR("Falha ao alocar memória para resultados do Cheat Finder.");
        free(finder);
        return NULL;
    }

    finder->capacity = INITIAL_FINDER_CAPACITY;
    finder->result_count = 0;
    finder->size = CHEAT_SIZE_8BIT;

    return finder;
}

void mega_emu_cheat_finder_destroy(mega_emu_cheat_finder_t* finder) {
    if (!finder) {
        return;
    }

    if (finder->results) {
        free(finder->results);
    }

    free(finder);
}

bool mega_emu_cheat_finder_init_search(mega_emu_cheat_finder_t* finder, mega_emu_cheat_size_t size) {
    if (!g_cheat_ctx.initialized || !finder) {
        return false;
    }

    // Limpa resultados anteriores
    finder->result_count = 0;
    finder->size = size;

    // Escaneia toda a memória acessível
    // [Esta parte depende da implementação específica de cada plataforma]

    return true;
}

uint32_t mega_emu_cheat_finder_search(mega_emu_cheat_finder_t* finder,
                                   mega_emu_cheat_comparator_t comparator,
                                   uint32_t value,
                                   bool use_previous_value) {
    if (!g_cheat_ctx.initialized || !finder) {
        return 0;
    }

    finder->comparator = comparator;
    finder->compare_value = value;
    finder->use_previous_value = use_previous_value;

    // Refina a busca
    // [Esta parte depende da implementação específica de cada plataforma]

    return finder->result_count;
}

bool mega_emu_cheat_finder_get_result(const mega_emu_cheat_finder_t* finder,
                                    uint32_t index,
                                    mega_emu_cheat_search_result_t* result) {
    if (!g_cheat_ctx.initialized || !finder || !result || index >= finder->result_count) {
        return false;
    }

    memcpy(result, &finder->results[index], sizeof(mega_emu_cheat_search_result_t));
    return true;
}

mega_emu_cheat_t* mega_emu_cheat_finder_create_cheat(const mega_emu_cheat_finder_t* finder,
                                                 uint32_t index,
                                                 const char* name,
                                                 const char* description,
                                                 uint32_t* value) {
    if (!g_cheat_ctx.initialized || !finder || index >= finder->result_count) {
        return NULL;
    }

    mega_emu_cheat_t* cheat = (mega_emu_cheat_t*)malloc(sizeof(mega_emu_cheat_t));
    if (!cheat) {
        LOG_ERROR("Falha ao alocar memória para novo cheat.");
        return NULL;
    }

    // Inicializa o cheat
    memset(cheat, 0, sizeof(mega_emu_cheat_t));

    // Copia as informações básicas
    if (name) {
        strncpy(cheat->name, name, sizeof(cheat->name) - 1);
    } else {
        snprintf(cheat->name, sizeof(cheat->name), "Cheat %08X", finder->results[index].address);
    }

    if (description) {
        strncpy(cheat->description, description, sizeof(cheat->description) - 1);
    } else {
        snprintf(cheat->description, sizeof(cheat->description), "Endereço %08X", finder->results[index].address);
    }

    cheat->platform = g_cheat_ctx.platform;
    cheat->type = CHEAT_TYPE_RAW;
    cheat->address = finder->results[index].address;
    cheat->size = finder->results[index].size;

    // Define o valor
    if (value) {
        cheat->value = *value;
    } else {
        cheat->value = finder->results[index].current_value;
    }

    return cheat;
}

// Funcionalidades pendentes
bool mega_emu_cheat_load_from_file(const char* filename) {
    // TO DO: Implementar carregamento de arquivo
    return false;
}

bool mega_emu_cheat_save_to_file(const char* filename) {
    // TO DO: Implementar salvamento de arquivo
    return false;
}
