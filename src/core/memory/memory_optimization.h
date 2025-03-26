/**
 * @file memory_optimization.h
 * @brief Perfis de otimização de memória específicos por plataforma
 */

#ifndef MEMORY_OPTIMIZATION_H
#define MEMORY_OPTIMIZATION_H

#include "memory_interface.h"
#include "memory_cache.h"

/**
 * @brief Tipos de perfis de otimização
 */
typedef enum
{
    EMU_MEMORY_PROFILE_GENERIC = 0,   /**< Perfil genérico sem otimizações específicas */
    EMU_MEMORY_PROFILE_NES,           /**< Perfil otimizado para NES */
    EMU_MEMORY_PROFILE_MEGA_DRIVE,    /**< Perfil otimizado para Mega Drive */
    EMU_MEMORY_PROFILE_MASTER_SYSTEM, /**< Perfil otimizado para Master System */
    EMU_MEMORY_PROFILE_SNES,          /**< Perfil otimizado para SNES */
    EMU_MEMORY_PROFILE_CUSTOM         /**< Perfil personalizado */
} emu_memory_profile_type_t;

/**
 * @brief Configuração de prefetching
 */
typedef struct
{
    bool enabled;           /**< Se o prefetching está ativado */
    uint32_t window_size;   /**< Tamanho da janela de prefetch em bytes */
    uint32_t trigger_count; /**< Número de acessos sequenciais para acionar prefetch */
} emu_memory_prefetch_config_t;

/**
 * @brief Região de memória otimizada
 */
typedef struct
{
    uint32_t start;        /**< Endereço inicial */
    uint32_t size;         /**< Tamanho em bytes */
    bool cache_priority;   /**< Se a região tem prioridade no cache */
    bool prefetch_enabled; /**< Se o prefetching está habilitado para esta região */
} emu_memory_optimized_region_t;

/**
 * @brief Perfil de otimização de memória
 */
typedef struct
{
    emu_memory_profile_type_t type;               /**< Tipo do perfil */
    char name[32];                                /**< Nome do perfil */
    uint32_t cache_size;                          /**< Tamanho do cache em entradas */
    emu_memory_prefetch_config_t prefetch_config; /**< Configuração de prefetching */
    emu_memory_optimized_region_t *regions;       /**< Regiões otimizadas */
    uint32_t num_regions;                         /**< Número de regiões */
} emu_memory_profile_t;

/**
 * @brief Aplica um perfil de otimização a uma instância de memória
 * @param memory Instância de memória
 * @param profile Perfil de otimização
 * @return true se o perfil foi aplicado com sucesso, false caso contrário
 */
bool emu_memory_apply_profile(emu_memory_t memory, const emu_memory_profile_t *profile);

/**
 * @brief Obtém um perfil de otimização predefinido
 * @param type Tipo do perfil
 * @return Perfil de otimização ou NULL se o tipo não for reconhecido
 */
emu_memory_profile_t *emu_memory_get_profile(emu_memory_profile_type_t type);

/**
 * @brief Cria um perfil de otimização personalizado
 * @param name Nome do perfil
 * @param cache_size Tamanho do cache em entradas
 * @return Novo perfil ou NULL em caso de erro
 */
emu_memory_profile_t *emu_memory_create_profile(const char *name, uint32_t cache_size);

/**
 * @brief Destrói um perfil de otimização
 * @param profile Perfil a ser destruído
 */
void emu_memory_destroy_profile(emu_memory_profile_t *profile);

/**
 * @brief Adiciona uma região otimizada a um perfil
 * @param profile Perfil de otimização
 * @param start Endereço inicial da região
 * @param size Tamanho da região em bytes
 * @param cache_priority Se a região tem prioridade no cache
 * @param prefetch_enabled Se o prefetching está habilitado para esta região
 * @return true se a região foi adicionada com sucesso, false caso contrário
 */
bool emu_memory_profile_add_region(emu_memory_profile_t *profile, uint32_t start, uint32_t size,
                                   bool cache_priority, bool prefetch_enabled);

/**
 * @brief Configura o prefetching para um perfil
 * @param profile Perfil de otimização
 * @param enabled Se o prefetching está ativado
 * @param window_size Tamanho da janela de prefetch em bytes
 * @param trigger_count Número de acessos sequenciais para acionar prefetch
 * @return true se a configuração foi aplicada com sucesso, false caso contrário
 */
bool emu_memory_profile_configure_prefetch(emu_memory_profile_t *profile, bool enabled,
                                           uint32_t window_size, uint32_t trigger_count);

/**
 * @brief Perfil otimizado para o NES
 *
 * Otimizações específicas:
 * - Cache prioritário para RAM (0x0000-0x07FF)
 * - Prefetching para ROM (0x8000-0xFFFF)
 * - Otimização para acesso a tabelas de pattern
 */
extern const emu_memory_profile_t EMU_MEMORY_PROFILE_NES_DEFAULT;

/**
 * @brief Perfil otimizado para o Mega Drive
 *
 * Otimizações específicas:
 * - Cache prioritário para RAM de trabalho
 * - Prefetching para ROM
 * - Otimização para acesso a VRAM e campos de sprite
 */
extern const emu_memory_profile_t EMU_MEMORY_PROFILE_MEGA_DRIVE_DEFAULT;

/**
 * @brief Perfil otimizado para o Master System
 *
 * Otimizações específicas:
 * - Cache prioritário para RAM (0xC000-0xDFFF)
 * - Prefetching para ROM (0x0000-0xBFFF)
 * - Otimização para acesso a VRAM
 */
extern const emu_memory_profile_t EMU_MEMORY_PROFILE_MASTER_SYSTEM_DEFAULT;

/**
 * @brief Perfil otimizado para o SNES
 *
 * Otimizações específicas:
 * - Cache prioritário para RAM de trabalho
 * - Prefetching para ROM
 * - Otimização para acesso a VRAM e OAM
 */
extern const emu_memory_profile_t EMU_MEMORY_PROFILE_SNES_DEFAULT;

#endif // MEMORY_OPTIMIZATION_H
