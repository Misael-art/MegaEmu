#ifndef EMU_ERROR_HANDLING_H
#define EMU_ERROR_HANDLING_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include "common_types.h"

// Definição do número total de códigos de erro
#define EMU_ERROR_COUNT 14

    // Estrutura para armazenar informações de erro
    struct emu_error_info
    {
        emu_error_t code;
        const char *message;
        const char *file;
        int32_t line;
    };

    typedef struct emu_error_info emu_error_info;

    // Funções de gerenciamento de erros
    void emu_error_init(void);
    void emu_error_shutdown(void);
    void emu_error_set(emu_error_t code, const char *message, const char *file, int32_t line);
    emu_error_t emu_error_get_code(void);
    const char *emu_error_get_message(void);
    const char *emu_error_get_file(void);
    int32_t emu_error_get_line(void);
    void emu_error_clear(void);
    bool emu_error_has_error(void);
    const char *emu_error_code_to_string(emu_error_t code);

// Macros para verificação de erros
#define EMU_CHECK_ERROR(condition, code, message)             \
    do                                                        \
    {                                                         \
        if (!(condition))                                     \
        {                                                     \
            emu_error_set(code, message, __FILE__, __LINE__); \
            return false;                                     \
        }                                                     \
    } while (0)

#define EMU_CHECK_NULL(ptr, message) \
    EMU_CHECK_ERROR((ptr) != NULL, EMU_ERROR_NULL_POINTER, message)

#define EMU_CHECK_BOUNDS(value, max, message) \
    EMU_CHECK_ERROR((value) < (max), EMU_ERROR_OUT_OF_BOUNDS, message)

#define EMU_THROW(code, message)                          \
    do                                                    \
    {                                                     \
        emu_error_set(code, message, __FILE__, __LINE__); \
        return false;                                     \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // EMU_ERROR_HANDLING_H
