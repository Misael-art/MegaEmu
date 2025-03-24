/**
 * @file common_types.h
 * @brief Definições de tipos comuns para o projeto
 */

#ifndef EMU_COMMON_TYPES_H
#define EMU_COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Macro para marcação de funções como inline
 */
#define EMU_INLINE inline

/**
 * @brief Macro para marcação de funções como não utilizadas
 */
#ifdef _MSC_VER
#define EMU_UNUSED
#else
#define EMU_UNUSED __attribute__((unused))
#endif

/**
 * @brief Macro para marcação de retorno de função como não utilizado
 */
#ifdef _MSC_VER
#define EMU_UNUSED_RESULT
#else
#define EMU_UNUSED_RESULT __attribute__((warn_unused_result))
#endif

/**
 * @brief Macro para marcação de parâmetros não utilizados
 */
#define EMU_UNUSED_PARAM(x) (void)(x)

    /**
     * @brief Definições de error codes
     */
    typedef enum emu_error_enum
    {
        EMU_SUCCESS = 0,
        EMU_ERROR_NONE = EMU_SUCCESS,
        EMU_ERROR_GENERIC = -1,
        EMU_ERROR_INVALID_PARAMETER = -2,
        EMU_ERROR_OUT_OF_MEMORY = -3,
        EMU_ERROR_FILE_NOT_FOUND = -4,
        EMU_ERROR_NOT_SUPPORTED = -5,
        EMU_ERROR_NOT_INITIALIZED = -6,
        EMU_ERROR_ALREADY_INITIALIZED = -7,
        EMU_ERROR_INVALID_ADDRESS = -8,
        EMU_ERROR_PERMISSION = -9,
        EMU_ERROR_INVALID_STATE = -10,
        EMU_ERROR_UNKNOWN = -11,
        EMU_ERROR_NULL_POINTER = -12,
        EMU_ERROR_OUT_OF_BOUNDS = -13
    } emu_error_t;

#ifdef __cplusplus
}
#endif

#endif /* EMU_COMMON_TYPES_H */
