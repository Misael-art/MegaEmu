/**
 * @file core_types.h
 * @brief Definições de tipos básicos compartilhados para o emulador Mega_Emu
 *
 * Este arquivo contém as definições centralizadas de tipos, estruturas e
 * códigos de erro usados em todo o projeto. Todos os componentes do emulador
 * devem incluir este arquivo para garantir consistência nas definições.
 *
 * @note Este é um arquivo de cabeçalho CENTRALIZADO. Todas as alterações devem
 * ser cuidadosamente consideradas, pois afetam todo o projeto.
 */

#ifndef _CORE_TYPES_H_
#define _CORE_TYPES_H_

#include "../utils/common_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup BasicTypes Tipos Básicos
 * @brief Definições de tipos básicos para facilitar a portabilidade
 * @{
 */
typedef uint8_t u8;   /**< Inteiro sem sinal de 8 bits */
typedef uint16_t u16; /**< Inteiro sem sinal de 16 bits */
typedef uint32_t u32; /**< Inteiro sem sinal de 32 bits */
typedef uint64_t u64; /**< Inteiro sem sinal de 64 bits */
typedef int8_t s8;    /**< Inteiro com sinal de 8 bits */
typedef int16_t s16;  /**< Inteiro com sinal de 16 bits */
typedef int32_t s32;  /**< Inteiro com sinal de 32 bits */
typedef int64_t s64;  /**< Inteiro com sinal de 64 bits */
/** @} */

/**
 * @brief Tipo de resultado para operações do emulador
 */
typedef enum result_t {
  RESULT_SUCCESS = EMU_SUCCESS,
  RESULT_ERROR_INIT = EMU_ERROR_GENERIC,
  RESULT_ERROR_MEMORY = EMU_ERROR_OUT_OF_MEMORY,
  RESULT_ERROR_INVALID = EMU_ERROR_INVALID_PARAMETER,
  RESULT_ERROR_NOT_FOUND = EMU_ERROR_FILE_NOT_FOUND,
  RESULT_ERROR_NOT_IMPL = EMU_ERROR_NOT_SUPPORTED
} result_t;

/**
 * @defgroup CoreStructures Estruturas Principais
 * @brief Forward declarations das estruturas principais do emulador
 * @{
 */
struct emu_platform_instance;
struct emu_core_instance;
struct emu_platform_info_instance;
struct emu_cpu_instance;
struct emu_memory_instance;
struct emu_video_instance;
struct emu_frontend_instance;
struct emu_audio_instance;
struct emu_input_instance;
struct emu_state_s;

typedef struct emu_platform_instance
    emu_platform_instance_t; /**< Abstração de plataforma emulada */
typedef struct emu_core_instance emu_core_instance_t; /**< Núcleo do emulador */
typedef struct emu_platform_info_instance
    emu_platform_info_instance_t; /**< Informações da plataforma */
typedef struct emu_cpu_instance emu_cpu_instance_t; /**< CPU emulada */
typedef struct emu_memory_instance
    emu_memory_instance_t; /**< Sistema de memória */
typedef struct emu_video_instance emu_video_instance_t; /**< Sistema de vídeo */
typedef struct emu_frontend_instance
    emu_frontend_instance_t; /**< Interface com o usuário */
typedef struct emu_audio_instance emu_audio_instance_t; /**< Sistema de áudio */
typedef struct emu_input_instance
    emu_input_instance_t;               /**< Sistema de entrada */
typedef struct emu_state_s emu_state_t; /**< Sistema de estado (save state) */
/** @} */

/**
 * @defgroup PlatformCallbacks Callbacks de Plataforma
 * @brief Tipos para funções de callback da plataforma
 * @{
 */

/**
 * @brief Função de inicialização de plataforma
 * @param platform Ponteiro para a estrutura da plataforma
 * @return Código de erro (EMU_ERROR_NONE se sucesso)
 */
typedef int32_t (*emu_platform_init_func)(emu_platform_instance_t *platform);

/**
 * @brief Função de finalização de plataforma
 * @param platform Ponteiro para a estrutura da plataforma
 */
typedef void (*emu_platform_shutdown_func)(emu_platform_instance_t *platform);

/**
 * @brief Função de reset de plataforma
 * @param platform Ponteiro para a estrutura da plataforma
 */
typedef void (*emu_platform_reset_func)(emu_platform_instance_t *platform);

/**
 * @brief Função para carregar ROM na plataforma
 * @param platform Ponteiro para a estrutura da plataforma
 * @param data Dados da ROM
 * @param size Tamanho dos dados da ROM
 * @return Código de erro (EMU_ERROR_NONE se sucesso)
 */
typedef int32_t (*emu_platform_load_rom_func)(emu_platform_instance_t *platform,
                                              const uint8_t *data, size_t size);

/**
 * @brief Função para executar um frame completo
 * @param platform Ponteiro para a estrutura da plataforma
 * @return Código de erro (EMU_ERROR_NONE se sucesso)
 */
typedef int32_t (*emu_platform_run_frame_func)(
    emu_platform_instance_t *platform);

/**
 * @brief Função para executar um número específico de ciclos
 * @param platform Ponteiro para a estrutura da plataforma
 * @param cycles Número de ciclos a executar
 * @return Código de erro (EMU_ERROR_NONE se sucesso)
 */
typedef int32_t (*emu_platform_run_cycles_func)(
    emu_platform_instance_t *platform, int32_t cycles);
/** @} */

/**
 * @defgroup Macros Macros Utilitárias
 * @brief Macros úteis para o desenvolvimento
 * @{
 */

/**
 * @brief Marca um parâmetro como não utilizado para evitar warnings
 */
#define UNUSED(x) (void)(x)

/**
 * @brief Calcula o número de elementos em um array estático
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _CORE_TYPES_H_ */
