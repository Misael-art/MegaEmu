/** * @file console_types.h * @brief Definições de tipos para consoles de videogame suportados pelo emulador * @author Mega_Emu Team * @version 1.0.0 * @date 2024-07-25 */#ifndef CONSOLE_TYPES_H#define CONSOLE_TYPES_H#include <stdint.h>#include <stdbool.h>// Tipos de console suportadostypedef enum{    CONSOLE_UNKNOWN = 0,    CONSOLE_GENESIS,    CONSOLE_NES,    CONSOLE_SNES,    CONSOLE_SMS,    CONSOLE_MASTERSYSTEM,    CONSOLE_MAX,    CONSOLE_NONE} console_type_t;#define CONSOLE_COUNT 4// Estrutura para definir características do consoletypedef struct{    console_type_t type;    const char *name;    const char *short_name;    const char *description;    uint32_t display_width;    uint32_t display_height;    uint32_t max_save_slots;    bool supports_savestates;    bool supports_screenshots;    bool supports_cheats;} console_info_t;// Funções de callback para operações de estado de jogotypedef bool (*save_state_save_fn)(const char *path);typedef bool (*save_state_load_fn)(const char *path);// Definição de tamanho máximo para caminhos de arquivos#define MAX_PATH_LENGTH 256// Função auxiliar para criação de diretóriosbool create_directory(const char *path);// Funções auxiliares para verificação de arquivobool file_exists(const char *path);time_t get_file_modification_time(const char *path);bool delete_file(const char *path);#endif /* CONSOLE_TYPES_H */