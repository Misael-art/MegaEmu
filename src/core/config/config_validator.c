/**
 * @file config_validator.c
 * @brief Implementação do sistema de validação e esquemas para configurações
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include "config_validator.h"
#include "../../utils/error_handling.h"

// Buffer para mensagens de erro
static char g_error_buffer[512] = {0};

// Esquemas predefinidos
emu_config_schema_t *EMU_CONFIG_SCHEMA_CORE = NULL;
emu_config_schema_t *EMU_CONFIG_SCHEMA_VIDEO = NULL;
emu_config_schema_t *EMU_CONFIG_SCHEMA_AUDIO = NULL;
emu_config_schema_t *EMU_CONFIG_SCHEMA_INPUT = NULL;
emu_config_schema_t *EMU_CONFIG_SCHEMA_NES = NULL;
emu_config_schema_t *EMU_CONFIG_SCHEMA_MEGA_DRIVE = NULL;
emu_config_schema_t *EMU_CONFIG_SCHEMA_MASTER_SYSTEM = NULL;

// Caminho para o diretório de perfis
static char g_profiles_directory[512] = "profiles";

// Função para inicialização dos esquemas predefinidos (chamada internamente)
static void _initialize_predefined_schemas(void);

// Função para definir o caminho do diretório de perfis
void emu_config_set_profiles_directory(const char *path)
{
    if (path && strlen(path) < sizeof(g_profiles_directory) - 1)
    {
        strcpy(g_profiles_directory, path);
    }
}

// Implementação das funções do esquema de configuração

emu_config_schema_t *emu_config_schema_create(const char *name, uint32_t version)
{
    if (!name)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Nome de esquema inválido");
        return NULL;
    }

    emu_config_schema_t *schema = (emu_config_schema_t *)malloc(sizeof(emu_config_schema_t));
    if (!schema)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao alocar memória para esquema");
        return NULL;
    }

    schema->items = NULL;
    schema->count = 0;
    schema->name = strdup(name);
    schema->version = version;

    if (!schema->name)
    {
        free(schema);
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao alocar memória para nome de esquema");
        return NULL;
    }

    return schema;
}

void emu_config_schema_destroy(emu_config_schema_t *schema)
{
    if (!schema)
    {
        return;
    }

    // Liberar memória dos itens do esquema
    if (schema->items)
    {
        for (uint32_t i = 0; i < schema->count; i++)
        {
            // Liberar memória específica para cada tipo de validação
            switch (schema->items[i].validation_type)
            {
            case EMU_CONFIG_VALIDATE_ENUM:
                if (schema->items[i].params.enum_values.values)
                {
                    free(schema->items[i].params.enum_values.values);
                }
                break;

            case EMU_CONFIG_VALIDATE_PATTERN:
                if (schema->items[i].params.regex.pattern)
                {
                    free((void *)schema->items[i].params.regex.pattern);
                }
                break;

            default:
                break;
            }

            // Liberar memória da chave e descrição
            if (schema->items[i].key)
            {
                free((void *)schema->items[i].key);
            }

            if (schema->items[i].description)
            {
                free((void *)schema->items[i].description);
            }

            // Liberar memória de valor padrão, se for string
            if (schema->items[i].type == EMU_CONFIG_TYPE_STRING &&
                schema->items[i].default_value.string_value)
            {
                free(schema->items[i].default_value.string_value);
            }
        }

        free(schema->items);
    }

    // Liberar memória do nome
    if (schema->name)
    {
        free((void *)schema->name);
    }

    free(schema);
}

bool emu_config_schema_add_item(emu_config_schema_t *schema, const emu_config_schema_item_t *item)
{
    if (!schema || !item || !item->key)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Parâmetros inválidos para adicionar item ao esquema");
        return false;
    }

    // Verificar se a chave já existe
    for (uint32_t i = 0; i < schema->count; i++)
    {
        if (strcmp(schema->items[i].key, item->key) == 0)
        {
            snprintf(g_error_buffer, sizeof(g_error_buffer), "Chave duplicada no esquema: %s", item->key);
            return false;
        }
    }

    // Redimensionar o array de itens
    emu_config_schema_item_t *new_items = (emu_config_schema_item_t *)realloc(
        schema->items, (schema->count + 1) * sizeof(emu_config_schema_item_t));

    if (!new_items)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao alocar memória para item de esquema");
        return false;
    }

    schema->items = new_items;

    // Copiar o item para o esquema
    emu_config_schema_item_t *dest = &schema->items[schema->count];

    // Copiar valores básicos
    dest->key = strdup(item->key);
    dest->type = item->type;
    dest->validation_type = item->validation_type;
    dest->required = item->required;

    // Copiar descrição se existir
    if (item->description)
    {
        dest->description = strdup(item->description);
    }
    else
    {
        dest->description = NULL;
    }

    // Copiar parâmetros de validação específicos
    switch (item->validation_type)
    {
    case EMU_CONFIG_VALIDATE_RANGE:
        if (item->type == EMU_CONFIG_TYPE_INT)
        {
            dest->params.int_range = item->params.int_range;
        }
        else if (item->type == EMU_CONFIG_TYPE_FLOAT)
        {
            dest->params.float_range = item->params.float_range;
        }
        break;

    case EMU_CONFIG_VALIDATE_ENUM:
        if (item->params.enum_values.count > 0 && item->params.enum_values.values)
        {
            dest->params.enum_values.count = item->params.enum_values.count;
            dest->params.enum_values.values = (const char **)malloc(
                sizeof(const char *) * item->params.enum_values.count);

            if (dest->params.enum_values.values)
            {
                for (uint32_t i = 0; i < item->params.enum_values.count; i++)
                {
                    dest->params.enum_values.values[i] = strdup(item->params.enum_values.values[i]);
                }
            }
        }
        else
        {
            dest->validation_type = EMU_CONFIG_VALIDATE_NONE;
        }
        break;

    case EMU_CONFIG_VALIDATE_CALLBACK:
        dest->params.callback = item->params.callback;
        break;

    case EMU_CONFIG_VALIDATE_PATTERN:
        if (item->params.regex.pattern)
        {
            dest->params.regex.pattern = strdup(item->params.regex.pattern);
        }
        else
        {
            dest->validation_type = EMU_CONFIG_VALIDATE_NONE;
        }
        break;

    default:
        // Sem validação
        break;
    }

    // Copiar valor padrão
    dest->default_value.type = item->default_value.type;

    switch (item->default_value.type)
    {
    case EMU_CONFIG_TYPE_INT:
        dest->default_value.int_value = item->default_value.int_value;
        break;

    case EMU_CONFIG_TYPE_FLOAT:
        dest->default_value.float_value = item->default_value.float_value;
        break;

    case EMU_CONFIG_TYPE_BOOL:
        dest->default_value.bool_value = item->default_value.bool_value;
        break;

    case EMU_CONFIG_TYPE_STRING:
        if (item->default_value.string_value)
        {
            dest->default_value.string_value = strdup(item->default_value.string_value);
        }
        else
        {
            dest->default_value.string_value = NULL;
        }
        break;

    default:
        // Não inicializar outros tipos
        break;
    }

    // Incrementar contagem de itens
    schema->count++;

    return true;
}

// Validação de um valor contra um item de esquema
bool emu_config_validate(const emu_config_value_t *value, const emu_config_schema_item_t *schema_item)
{
    if (!value || !schema_item)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Parâmetros inválidos para validação");
        return false;
    }

    // Verificar tipo
    if (value->type != schema_item->type)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer),
                 "Tipo inválido para %s: esperado %d, obtido %d",
                 schema_item->key, schema_item->type, value->type);
        return false;
    }

    // Realizar validação específica
    switch (schema_item->validation_type)
    {
    case EMU_CONFIG_VALIDATE_RANGE:
        if (value->type == EMU_CONFIG_TYPE_INT)
        {
            if (value->int_value < schema_item->params.int_range.min ||
                value->int_value > schema_item->params.int_range.max)
            {
                snprintf(g_error_buffer, sizeof(g_error_buffer),
                         "Valor fora do intervalo para %s: %lld (intervalo: %lld a %lld)",
                         schema_item->key, value->int_value,
                         schema_item->params.int_range.min,
                         schema_item->params.int_range.max);
                return false;
            }
        }
        else if (value->type == EMU_CONFIG_TYPE_FLOAT)
        {
            if (value->float_value < schema_item->params.float_range.min ||
                value->float_value > schema_item->params.float_range.max)
            {
                snprintf(g_error_buffer, sizeof(g_error_buffer),
                         "Valor fora do intervalo para %s: %f (intervalo: %f a %f)",
                         schema_item->key, value->float_value,
                         schema_item->params.float_range.min,
                         schema_item->params.float_range.max);
                return false;
            }
        }
        break;

    case EMU_CONFIG_VALIDATE_ENUM:
        if (value->type == EMU_CONFIG_TYPE_STRING && value->string_value)
        {
            bool found = false;
            for (uint32_t i = 0; i < schema_item->params.enum_values.count; i++)
            {
                if (strcmp(value->string_value, schema_item->params.enum_values.values[i]) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                snprintf(g_error_buffer, sizeof(g_error_buffer),
                         "Valor não está na lista de valores permitidos para %s: %s",
                         schema_item->key, value->string_value);
                return false;
            }
        }
        break;

    case EMU_CONFIG_VALIDATE_CALLBACK:
        if (schema_item->params.callback.callback)
        {
            if (!schema_item->params.callback.callback(value, schema_item->params.callback.userdata))
            {
                snprintf(g_error_buffer, sizeof(g_error_buffer),
                         "Callback de validação falhou para %s",
                         schema_item->key);
                return false;
            }
        }
        break;

    case EMU_CONFIG_VALIDATE_PATTERN:
        if (value->type == EMU_CONFIG_TYPE_STRING && value->string_value)
        {
            regex_t regex;
            int reti = regcomp(&regex, schema_item->params.regex.pattern, REG_EXTENDED);
            if (reti)
            {
                snprintf(g_error_buffer, sizeof(g_error_buffer),
                         "Erro ao compilar regex para %s",
                         schema_item->key);
                return false;
            }

            reti = regexec(&regex, value->string_value, 0, NULL, 0);
            regfree(&regex);

            if (reti)
            {
                snprintf(g_error_buffer, sizeof(g_error_buffer),
                         "Valor não corresponde ao padrão para %s: %s",
                         schema_item->key, value->string_value);
                return false;
            }
        }
        break;

    default:
        // Sem validação
        break;
    }

    return true;
}

// Validação de todas as configurações contra um esquema
bool emu_config_validate_all(const emu_config_schema_t *schema)
{
    if (!schema)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Esquema inválido");
        return false;
    }

    const emu_config_interface_t *config = emu_config_get_interface();
    if (!config)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Interface de configuração indisponível");
        return false;
    }

    // Verificar cada item do esquema
    for (uint32_t i = 0; i < schema->count; i++)
    {
        const emu_config_schema_item_t *item = &schema->items[i];

        // Obter valor atual
        emu_config_value_t value;
        int32_t result = config->get_value(item->key, &value);

        // Se obrigatório e não existe
        if (result != 0 && item->required)
        {
            snprintf(g_error_buffer, sizeof(g_error_buffer),
                     "Configuração obrigatória ausente: %s",
                     item->key);
            return false;
        }

        // Se existe, validar
        if (result == 0)
        {
            if (!emu_config_validate(&value, item))
            {
                // Mensagem de erro já definida pela função emu_config_validate
                return false;
            }

            // Liberar memória se for string
            if (value.type == EMU_CONFIG_TYPE_STRING && value.string_value)
            {
                free(value.string_value);
            }
        }
    }

    return true;
}

// Aplicação de valores padrão
bool emu_config_apply_defaults(const emu_config_schema_t *schema)
{
    if (!schema)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Esquema inválido");
        return false;
    }

    const emu_config_interface_t *config = emu_config_get_interface();
    if (!config)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Interface de configuração indisponível");
        return false;
    }

    // Aplicar valor padrão para cada item
    for (uint32_t i = 0; i < schema->count; i++)
    {
        const emu_config_schema_item_t *item = &schema->items[i];

        // Verificar se a configuração já existe
        emu_config_value_t value;
        int32_t result = config->get_value(item->key, &value);

        // Se não existe, aplicar valor padrão
        if (result != 0)
        {
            // Definir valor padrão
            switch (item->type)
            {
            case EMU_CONFIG_TYPE_INT:
                config->set_int(item->key, item->default_value.int_value);
                break;

            case EMU_CONFIG_TYPE_FLOAT:
                config->set_float(item->key, item->default_value.float_value);
                break;

            case EMU_CONFIG_TYPE_BOOL:
                config->set_bool(item->key, item->default_value.bool_value);
                break;

            case EMU_CONFIG_TYPE_STRING:
                if (item->default_value.string_value)
                {
                    config->set_string(item->key, item->default_value.string_value);
                }
                break;

            default:
                // Não inicializar outros tipos
                break;
            }
        }
        else
        {
            // Liberar memória se for string
            if (value.type == EMU_CONFIG_TYPE_STRING && value.string_value)
            {
                free(value.string_value);
            }
        }
    }

    return true;
}

// Normalização de configurações
bool emu_config_normalize(const emu_config_schema_t *schema)
{
    if (!schema)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Esquema inválido");
        return false;
    }

    const emu_config_interface_t *config = emu_config_get_interface();
    if (!config)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Interface de configuração indisponível");
        return false;
    }

    // Normalizar cada item
    for (uint32_t i = 0; i < schema->count; i++)
    {
        const emu_config_schema_item_t *item = &schema->items[i];

        // Obter valor atual
        emu_config_value_t value;
        int32_t result = config->get_value(item->key, &value);

        // Se existe, normalizar
        if (result == 0)
        {
            bool updated = false;

            switch (item->validation_type)
            {
            case EMU_CONFIG_VALIDATE_RANGE:
                if (value.type == EMU_CONFIG_TYPE_INT)
                {
                    // Limitar ao intervalo
                    if (value.int_value < item->params.int_range.min)
                    {
                        value.int_value = item->params.int_range.min;
                        updated = true;
                    }
                    else if (value.int_value > item->params.int_range.max)
                    {
                        value.int_value = item->params.int_range.max;
                        updated = true;
                    }
                }
                else if (value.type == EMU_CONFIG_TYPE_FLOAT)
                {
                    // Limitar ao intervalo
                    if (value.float_value < item->params.float_range.min)
                    {
                        value.float_value = item->params.float_range.min;
                        updated = true;
                    }
                    else if (value.float_value > item->params.float_range.max)
                    {
                        value.float_value = item->params.float_range.max;
                        updated = true;
                    }
                }
                break;

            default:
                // Outros tipos de validação não possuem normalização direta
                break;
            }

            // Atualizar valor se foi normalizado
            if (updated)
            {
                config->set_value(item->key, &value);
            }

            // Liberar memória se for string
            if (value.type == EMU_CONFIG_TYPE_STRING && value.string_value)
            {
                free(value.string_value);
            }
        }
    }

    return true;
}

// Obtenção de erro de validação
const char *emu_config_get_validation_error(void)
{
    return g_error_buffer[0] ? g_error_buffer : NULL;
}

// Verificação de configurações obrigatórias
uint32_t emu_config_check_required(const emu_config_schema_t *schema,
                                   char **errors_out, uint32_t max_errors)
{
    if (!schema)
    {
        return 0;
    }

    const emu_config_interface_t *config = emu_config_get_interface();
    if (!config)
    {
        return 0;
    }

    uint32_t missing_count = 0;

    // Verificar cada item do esquema
    for (uint32_t i = 0; i < schema->count && missing_count < max_errors; i++)
    {
        const emu_config_schema_item_t *item = &schema->items[i];

        if (item->required)
        {
            // Verificar se a configuração existe
            emu_config_value_t value;
            int32_t result = config->get_value(item->key, &value);

            if (result != 0)
            {
                // Configuração obrigatória ausente
                if (errors_out)
                {
                    errors_out[missing_count] = strdup(item->key);
                }
                missing_count++;
            }
            else
            {
                // Liberar memória se for string
                if (value.type == EMU_CONFIG_TYPE_STRING && value.string_value)
                {
                    free(value.string_value);
                }
            }
        }
    }

    return missing_count;
}

// Implementação das funções de perfil

emu_config_profile_t *emu_config_profile_create(const char *name,
                                                const char *description,
                                                emu_config_schema_t *schema)
{
    if (!name)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Nome de perfil inválido");
        return NULL;
    }

    emu_config_profile_t *profile = (emu_config_profile_t *)malloc(sizeof(emu_config_profile_t));
    if (!profile)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao alocar memória para perfil");
        return NULL;
    }

    strncpy(profile->name, name, sizeof(profile->name) - 1);
    profile->name[sizeof(profile->name) - 1] = '\0';

    if (description)
    {
        strncpy(profile->description, description, sizeof(profile->description) - 1);
    }
    else
    {
        strcpy(profile->description, "");
    }

    profile->schema = schema;
    profile->created_time = time(NULL);
    profile->modified_time = profile->created_time;

    return profile;
}

void emu_config_profile_destroy(emu_config_profile_t *profile)
{
    if (!profile)
    {
        return;
    }

    // Não liberar o esquema, pois ele pode ser compartilhado

    free(profile);
}

// Salvar configuração atual como perfil
bool emu_config_profile_save_current(emu_config_profile_t *profile)
{
    if (!profile)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Perfil inválido");
        return false;
    }

    const emu_config_interface_t *config = emu_config_get_interface();
    if (!config)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Interface de configuração indisponível");
        return false;
    }

    // Criar caminho do arquivo
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.profile", g_profiles_directory, profile->name);

    // Atualizar timestamp
    profile->modified_time = time(NULL);

    // Salvar configuração para o arquivo
    return config->save_to_file(filepath) == 0;
}

// Carregar perfil
bool emu_config_profile_load(emu_config_profile_t *profile)
{
    if (!profile)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Perfil inválido");
        return false;
    }

    const emu_config_interface_t *config = emu_config_get_interface();
    if (!config)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Interface de configuração indisponível");
        return false;
    }

    // Criar caminho do arquivo
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.profile", g_profiles_directory, profile->name);

    // Carregar configuração do arquivo
    if (config->load_from_file(filepath) != 0)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao carregar perfil: %s", profile->name);
        return false;
    }

    // Validar configuração se tiver esquema
    if (profile->schema)
    {
        if (!emu_config_validate_all(profile->schema))
        {
            // Mensagem de erro já definida pela função emu_config_validate_all
            return false;
        }
    }

    return true;
}

// Exportar perfil para arquivo
bool emu_config_profile_export(const emu_config_profile_t *profile, const char *filename)
{
    if (!profile || !filename)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Parâmetros inválidos para exportação");
        return false;
    }

    // Criar arquivo
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao criar arquivo: %s", filename);
        return false;
    }

    // Escrever cabeçalho
    fprintf(file, "# Mega_Emu Configuration Profile\n");
    fprintf(file, "# Name: %s\n", profile->name);
    fprintf(file, "# Description: %s\n", profile->description);
    fprintf(file, "# Created: %llu\n", (unsigned long long)profile->created_time);
    fprintf(file, "# Modified: %llu\n", (unsigned long long)profile->modified_time);
    fprintf(file, "# Schema: %s (v%u)\n",
            profile->schema ? profile->schema->name : "None",
            profile->schema ? profile->schema->version : 0);
    fprintf(file, "\n");

    // Obter caminho do arquivo do perfil
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.profile", g_profiles_directory, profile->name);

    // Abrir arquivo do perfil
    FILE *profile_file = fopen(filepath, "r");
    if (!profile_file)
    {
        fclose(file);
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao abrir arquivo do perfil: %s", filepath);
        return false;
    }

    // Copiar conteúdo
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), profile_file))
    {
        fputs(buffer, file);
    }

    // Fechar arquivos
    fclose(profile_file);
    fclose(file);

    return true;
}

// Importar perfil de arquivo
emu_config_profile_t *emu_config_profile_import(const char *filename,
                                                emu_config_schema_t *schema)
{
    if (!filename)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Nome de arquivo inválido");
        return NULL;
    }

    // Abrir arquivo
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao abrir arquivo: %s", filename);
        return NULL;
    }

    // Extrair informações do cabeçalho
    char name[64] = {0};
    char description[256] = {0};
    uint64_t created_time = time(NULL);
    uint64_t modified_time = created_time;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (buffer[0] == '#')
        {
            // Linha de comentário
            if (strstr(buffer, "# Name:") == buffer)
            {
                sscanf(buffer, "# Name: %63[^\n]", name);
            }
            else if (strstr(buffer, "# Description:") == buffer)
            {
                sscanf(buffer, "# Description: %255[^\n]", description);
            }
            else if (strstr(buffer, "# Created:") == buffer)
            {
                sscanf(buffer, "# Created: %llu", (unsigned long long *)&created_time);
            }
            else if (strstr(buffer, "# Modified:") == buffer)
            {
                sscanf(buffer, "# Modified: %llu", (unsigned long long *)&modified_time);
            }
        }
        else if (buffer[0] != '\n')
        {
            // Primeira linha não-comentário, não vazia
            break;
        }
    }

    // Verificar se temos nome
    if (!name[0])
    {
        // Extrair nome do arquivo
        const char *last_slash = strrchr(filename, '/');
        if (!last_slash)
        {
            last_slash = strrchr(filename, '\\');
        }

        const char *basename = last_slash ? last_slash + 1 : filename;
        const char *dot = strrchr(basename, '.');

        if (dot)
        {
            size_t len = dot - basename;
            if (len > sizeof(name) - 1)
            {
                len = sizeof(name) - 1;
            }
            strncpy(name, basename, len);
            name[len] = '\0';
        }
        else
        {
            strncpy(name, basename, sizeof(name) - 1);
        }
    }

    // Criar perfil
    emu_config_profile_t *profile = emu_config_profile_create(name, description, schema);
    if (!profile)
    {
        fclose(file);
        // Mensagem de erro já definida pela função emu_config_profile_create
        return NULL;
    }

    // Definir timestamps
    profile->created_time = created_time;
    profile->modified_time = modified_time;

    // Voltar ao início do arquivo
    fseek(file, 0, SEEK_SET);

    // Criar arquivo de perfil
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.profile", g_profiles_directory, name);

    FILE *profile_file = fopen(filepath, "w");
    if (!profile_file)
    {
        fclose(file);
        emu_config_profile_destroy(profile);
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao criar arquivo de perfil: %s", filepath);
        return NULL;
    }

    // Copiar conteúdo, exceto linhas de comentário do cabeçalho
    bool in_header = true;
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (in_header)
        {
            if (buffer[0] == '#' || buffer[0] == '\n')
            {
                continue;
            }
            in_header = false;
        }

        fputs(buffer, profile_file);
    }

    // Fechar arquivos
    fclose(file);
    fclose(profile_file);

    return profile;
}

// Enumerar perfis disponíveis
uint32_t emu_config_profile_enumerate(char **profiles, uint32_t max_count)
{
    if (!profiles || max_count == 0)
    {
        return 0;
    }

    // Listar arquivos .profile no diretório
    // Na implementação real, usar funções do sistema operacional
    // Aqui, apenas um stub

    // TODO: Implementação real

    return 0;
}

// Obter informações de um perfil
bool emu_config_profile_get_info(const char *name, emu_config_profile_t *profile)
{
    if (!name || !profile)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Parâmetros inválidos");
        return false;
    }

    // Criar caminho do arquivo
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.profile", g_profiles_directory, name);

    // Abrir arquivo
    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Perfil não encontrado: %s", name);
        return false;
    }

    // Preencher informações básicas
    strncpy(profile->name, name, sizeof(profile->name) - 1);
    strcpy(profile->description, "");
    profile->schema = NULL;
    profile->created_time = 0;
    profile->modified_time = 0;

    // Extrair informações do cabeçalho
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (buffer[0] == '#')
        {
            // Linha de comentário
            if (strstr(buffer, "# Description:") == buffer)
            {
                sscanf(buffer, "# Description: %255[^\n]", profile->description);
            }
            else if (strstr(buffer, "# Created:") == buffer)
            {
                sscanf(buffer, "# Created: %llu", (unsigned long long *)&profile->created_time);
            }
            else if (strstr(buffer, "# Modified:") == buffer)
            {
                sscanf(buffer, "# Modified: %llu", (unsigned long long *)&profile->modified_time);
            }
        }
        else if (buffer[0] != '\n')
        {
            // Primeira linha não-comentário, não vazia
            break;
        }
    }

    fclose(file);
    return true;
}

// Excluir um perfil
bool emu_config_profile_delete(const char *name)
{
    if (!name)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Nome de perfil inválido");
        return false;
    }

    // Criar caminho do arquivo
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.profile", g_profiles_directory, name);

    // Excluir arquivo
    if (remove(filepath) != 0)
    {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Falha ao excluir perfil: %s", name);
        return false;
    }

    return true;
}

// Inicialização dos esquemas predefinidos
static void _initialize_predefined_schemas(void)
{
    // Esquema CORE
    if (!EMU_CONFIG_SCHEMA_CORE)
    {
        EMU_CONFIG_SCHEMA_CORE = emu_config_schema_create("Core", 1);
        if (EMU_CONFIG_SCHEMA_CORE)
        {
            // TODO: Adicionar itens ao esquema
        }
    }

    // Esquema VIDEO
    if (!EMU_CONFIG_SCHEMA_VIDEO)
    {
        EMU_CONFIG_SCHEMA_VIDEO = emu_config_schema_create("Video", 1);
        if (EMU_CONFIG_SCHEMA_VIDEO)
        {
            // TODO: Adicionar itens ao esquema
        }
    }

    // Esquema AUDIO
    if (!EMU_CONFIG_SCHEMA_AUDIO)
    {
        EMU_CONFIG_SCHEMA_AUDIO = emu_config_schema_create("Audio", 1);
        if (EMU_CONFIG_SCHEMA_AUDIO)
        {
            // TODO: Adicionar itens ao esquema
        }
    }

    // Esquema INPUT
    if (!EMU_CONFIG_SCHEMA_INPUT)
    {
        EMU_CONFIG_SCHEMA_INPUT = emu_config_schema_create("Input", 1);
        if (EMU_CONFIG_SCHEMA_INPUT)
        {
            // TODO: Adicionar itens ao esquema
        }
    }

    // Esquema NES
    if (!EMU_CONFIG_SCHEMA_NES)
    {
        EMU_CONFIG_SCHEMA_NES = emu_config_schema_create("NES", 1);
        if (EMU_CONFIG_SCHEMA_NES)
        {
            // TODO: Adicionar itens ao esquema
        }
    }

    // Esquema MEGA_DRIVE
    if (!EMU_CONFIG_SCHEMA_MEGA_DRIVE)
    {
        EMU_CONFIG_SCHEMA_MEGA_DRIVE = emu_config_schema_create("MegaDrive", 1);
        if (EMU_CONFIG_SCHEMA_MEGA_DRIVE)
        {
            // TODO: Adicionar itens ao esquema
        }
    }

    // Esquema MASTER_SYSTEM
    if (!EMU_CONFIG_SCHEMA_MASTER_SYSTEM)
    {
        EMU_CONFIG_SCHEMA_MASTER_SYSTEM = emu_config_schema_create("MasterSystem", 1);
        if (EMU_CONFIG_SCHEMA_MASTER_SYSTEM)
        {
            // TODO: Adicionar itens ao esquema
        }
    }
}

// Função de inicialização do módulo
void emu_config_validator_init(void)
{
    _initialize_predefined_schemas();
}

// Função de finalização do módulo
void emu_config_validator_shutdown(void)
{
    // Liberar esquemas predefinidos
    if (EMU_CONFIG_SCHEMA_CORE)
    {
        emu_config_schema_destroy(EMU_CONFIG_SCHEMA_CORE);
        EMU_CONFIG_SCHEMA_CORE = NULL;
    }

    if (EMU_CONFIG_SCHEMA_VIDEO)
    {
        emu_config_schema_destroy(EMU_CONFIG_SCHEMA_VIDEO);
        EMU_CONFIG_SCHEMA_VIDEO = NULL;
    }

    if (EMU_CONFIG_SCHEMA_AUDIO)
    {
        emu_config_schema_destroy(EMU_CONFIG_SCHEMA_AUDIO);
        EMU_CONFIG_SCHEMA_AUDIO = NULL;
    }

    if (EMU_CONFIG_SCHEMA_INPUT)
    {
        emu_config_schema_destroy(EMU_CONFIG_SCHEMA_INPUT);
        EMU_CONFIG_SCHEMA_INPUT = NULL;
    }

    if (EMU_CONFIG_SCHEMA_NES)
    {
        emu_config_schema_destroy(EMU_CONFIG_SCHEMA_NES);
        EMU_CONFIG_SCHEMA_NES = NULL;
    }

    if (EMU_CONFIG_SCHEMA_MEGA_DRIVE)
    {
        emu_config_schema_destroy(EMU_CONFIG_SCHEMA_MEGA_DRIVE);
        EMU_CONFIG_SCHEMA_MEGA_DRIVE = NULL;
    }

    if (EMU_CONFIG_SCHEMA_MASTER_SYSTEM)
    {
        emu_config_schema_destroy(EMU_CONFIG_SCHEMA_MASTER_SYSTEM);
        EMU_CONFIG_SCHEMA_MASTER_SYSTEM = NULL;
    }
}
