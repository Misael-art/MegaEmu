/**
 * @file config_validator.h
 * @brief Sistema de validação e esquemas para configurações
 */

#ifndef CONFIG_VALIDATOR_H
#define CONFIG_VALIDATOR_H

#include "config_interface.h"

/**
 * @brief Tipos de validação para configurações
 */
typedef enum
{
    EMU_CONFIG_VALIDATE_NONE = 0, /**< Sem validação */
    EMU_CONFIG_VALIDATE_RANGE,    /**< Validação de intervalo numérico */
    EMU_CONFIG_VALIDATE_ENUM,     /**< Validação de valor enum/conjunto */
    EMU_CONFIG_VALIDATE_CALLBACK, /**< Validação via callback customizado */
    EMU_CONFIG_VALIDATE_PATTERN   /**< Validação via padrão/regex */
} emu_config_validate_type_t;

/**
 * @brief Parâmetros para validação
 */
typedef struct
{
    union
    {
        // Validação de intervalo para valores inteiros
        struct
        {
            int64_t min; /**< Valor mínimo */
            int64_t max; /**< Valor máximo */
        } int_range;

        // Validação de intervalo para valores float
        struct
        {
            double min; /**< Valor mínimo */
            double max; /**< Valor máximo */
        } float_range;

        // Validação de enumeração
        struct
        {
            const char **values; /**< Array de valores válidos */
            uint32_t count;      /**< Número de valores válidos */
        } enum_values;

        // Validação via callback
        struct
        {
            bool (*callback)(const emu_config_value_t *value, void *userdata);
            void *userdata;
        } callback;

        // Validação de padrão (string)
        struct
        {
            const char *pattern; /**< Padrão regex */
        } regex;
    };
} emu_config_validate_params_t;

/**
 * @brief Item de esquema para validação de configuração
 */
typedef struct
{
    const char *key;                            /**< Chave de configuração */
    emu_config_type_t type;                     /**< Tipo de dado esperado */
    emu_config_validate_type_t validation_type; /**< Tipo de validação */
    emu_config_validate_params_t params;        /**< Parâmetros de validação */
    emu_config_value_t default_value;           /**< Valor padrão */
    const char *description;                    /**< Descrição da configuração */
    bool required;                              /**< Se é uma configuração obrigatória */
} emu_config_schema_item_t;

/**
 * @brief Esquema de configuração
 */
typedef struct
{
    emu_config_schema_item_t *items; /**< Array de itens do esquema */
    uint32_t count;                  /**< Número de itens */
    const char *name;                /**< Nome do esquema */
    uint32_t version;                /**< Versão do esquema */
} emu_config_schema_t;

/**
 * @brief Cria um novo esquema de configuração
 * @param name Nome do esquema
 * @param version Versão do esquema
 * @return Esquema de configuração ou NULL em caso de erro
 */
emu_config_schema_t *emu_config_schema_create(const char *name, uint32_t version);

/**
 * @brief Destrói um esquema de configuração
 * @param schema Esquema a ser destruído
 */
void emu_config_schema_destroy(emu_config_schema_t *schema);

/**
 * @brief Adiciona um item ao esquema de configuração
 * @param schema Esquema de configuração
 * @param item Item a ser adicionado
 * @return true se adicionado com sucesso, false caso contrário
 */
bool emu_config_schema_add_item(emu_config_schema_t *schema, const emu_config_schema_item_t *item);

/**
 * @brief Valida um valor de configuração contra um item de esquema
 * @param value Valor de configuração a ser validado
 * @param schema_item Item de esquema para validação
 * @return true se o valor é válido, false caso contrário
 */
bool emu_config_validate(const emu_config_value_t *value, const emu_config_schema_item_t *schema_item);

/**
 * @brief Valida todas as configurações contra um esquema
 * @param schema Esquema de configuração
 * @return true se todas as configurações são válidas, false caso contrário
 */
bool emu_config_validate_all(const emu_config_schema_t *schema);

/**
 * @brief Aplica valores padrão para configurações não definidas
 * @param schema Esquema de configuração
 * @return true se os valores foram aplicados com sucesso, false caso contrário
 */
bool emu_config_apply_defaults(const emu_config_schema_t *schema);

/**
 * @brief Normaliza configurações para os valores válidos mais próximos
 * @param schema Esquema de configuração
 * @return true se as configurações foram normalizadas com sucesso, false caso contrário
 */
bool emu_config_normalize(const emu_config_schema_t *schema);

/**
 * @brief Obtém a mensagem de erro da última validação
 * @return Mensagem de erro ou NULL se não houver erro
 */
const char *emu_config_get_validation_error(void);

/**
 * @brief Verifica se uma configuração requerida existe
 * @param schema Esquema de configuração
 * @param errors_out Array para armazenar chaves de configurações ausentes
 * @param max_errors Tamanho máximo do array
 * @return Número de configurações requeridas ausentes
 */
uint32_t emu_config_check_required(const emu_config_schema_t *schema,
                                   char **errors_out, uint32_t max_errors);

/**
 * @brief Estrutura para representar um perfil de configuração
 */
typedef struct
{
    char name[64];               /**< Nome do perfil */
    char description[256];       /**< Descrição do perfil */
    emu_config_schema_t *schema; /**< Esquema associado */
    uint64_t created_time;       /**< Timestamp de criação */
    uint64_t modified_time;      /**< Timestamp da última modificação */
} emu_config_profile_t;

/**
 * @brief Cria um novo perfil de configuração
 * @param name Nome do perfil
 * @param description Descrição do perfil
 * @param schema Esquema associado (opcional)
 * @return Perfil criado ou NULL em caso de erro
 */
emu_config_profile_t *emu_config_profile_create(const char *name,
                                                const char *description,
                                                emu_config_schema_t *schema);

/**
 * @brief Destrói um perfil de configuração
 * @param profile Perfil a ser destruído
 */
void emu_config_profile_destroy(emu_config_profile_t *profile);

/**
 * @brief Salva o estado atual da configuração como um perfil
 * @param profile Perfil para armazenar a configuração
 * @return true se salvo com sucesso, false caso contrário
 */
bool emu_config_profile_save_current(emu_config_profile_t *profile);

/**
 * @brief Carrega um perfil de configuração
 * @param profile Perfil a ser carregado
 * @return true se carregado com sucesso, false caso contrário
 */
bool emu_config_profile_load(emu_config_profile_t *profile);

/**
 * @brief Exporta um perfil para um arquivo
 * @param profile Perfil a ser exportado
 * @param filename Nome do arquivo
 * @return true se exportado com sucesso, false caso contrário
 */
bool emu_config_profile_export(const emu_config_profile_t *profile, const char *filename);

/**
 * @brief Importa um perfil de um arquivo
 * @param filename Nome do arquivo
 * @param schema Esquema para validação (opcional)
 * @return Perfil importado ou NULL em caso de erro
 */
emu_config_profile_t *emu_config_profile_import(const char *filename,
                                                emu_config_schema_t *schema);

/**
 * @brief Enumera os perfis disponíveis
 * @param profiles Buffer para armazenar os nomes dos perfis
 * @param max_count Capacidade máxima do buffer
 * @return Número de perfis encontrados
 */
uint32_t emu_config_profile_enumerate(char **profiles, uint32_t max_count);

/**
 * @brief Obtém informações sobre um perfil salvo
 * @param name Nome do perfil
 * @param profile Estrutura para receber as informações
 * @return true se as informações foram obtidas com sucesso, false caso contrário
 */
bool emu_config_profile_get_info(const char *name, emu_config_profile_t *profile);

/**
 * @brief Exclui um perfil
 * @param name Nome do perfil
 * @return true se excluído com sucesso, false caso contrário
 */
bool emu_config_profile_delete(const char *name);

// Esquemas predefinidos para plataformas suportadas
extern emu_config_schema_t *EMU_CONFIG_SCHEMA_CORE;
extern emu_config_schema_t *EMU_CONFIG_SCHEMA_VIDEO;
extern emu_config_schema_t *EMU_CONFIG_SCHEMA_AUDIO;
extern emu_config_schema_t *EMU_CONFIG_SCHEMA_INPUT;
extern emu_config_schema_t *EMU_CONFIG_SCHEMA_NES;
extern emu_config_schema_t *EMU_CONFIG_SCHEMA_MEGA_DRIVE;
extern emu_config_schema_t *EMU_CONFIG_SCHEMA_MASTER_SYSTEM;

#endif // CONFIG_VALIDATOR_H
