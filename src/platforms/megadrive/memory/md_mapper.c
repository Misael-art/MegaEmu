/**
 * @file md_mapper.c
 * @brief Implementação do sistema de mappers para cartuchos do Mega Drive/Genesis
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-22
 */

#include "md_mapper.h"
#include <string.h>
#include <stdio.h>
#include "../../../utils/log_utils.h"
#include "../../../utils/file_utils.h"

// Define o tamanho padrão de SRAM
#define DEFAULT_SRAM_SIZE (64 * 1024) // 64KB

// Forward declarations das funções de mapper específicas
static uint8_t mapper_none_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_none_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t mapper_sega_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_sega_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t mapper_ssf2_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_ssf2_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t mapper_ssrpg_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_ssrpg_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t mapper_eeprom_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_eeprom_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t mapper_codemasters_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_codemasters_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t mapper_pier_solar_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_pier_solar_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t mapper_ea_read_rom(md_mapper_t *mapper, uint32_t address);
static void mapper_ea_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value);

// Funções padrão para acesso à SRAM
static uint8_t default_read_sram(md_mapper_t *mapper, uint32_t address);
static void default_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value);

// Funções específicas de SRAM para diferentes mappers
static uint8_t ssrpg_read_sram(md_mapper_t *mapper, uint32_t address);
static void ssrpg_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value);
static uint8_t eeprom_read_sram(md_mapper_t *mapper, uint32_t address);
static void eeprom_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value);

/**
 * @brief Inicializa um mapper do Mega Drive
 */
bool md_mapper_init(md_mapper_t *mapper, md_mapper_type_t type, uint8_t *rom_data, uint32_t rom_size)
{
    if (!mapper || !rom_data || rom_size == 0)
    {
        LOG_ERROR("Parâmetros inválidos para inicialização do mapper");
        return false;
    }

    // Limpar estrutura do mapper
    memset(mapper, 0, sizeof(md_mapper_t));

    // Configurar dados básicos
    mapper->type = type;
    mapper->rom_data = rom_data;
    mapper->rom_size = rom_size;

    // Configuração específica para cada tipo de mapper
    switch (type)
    {
    case MD_MAPPER_NONE:
        mapper->num_banks = 1;
        mapper->bank_size = rom_size;
        mapper->read_rom = mapper_none_read_rom;
        mapper->write_rom = mapper_none_write_rom;
        mapper->read_sram = default_read_sram;
        mapper->write_sram = default_write_sram;
        break;

    case MD_MAPPER_SEGA:
        mapper->num_banks = 1;
        mapper->bank_size = rom_size;
        mapper->read_rom = mapper_sega_read_rom;
        mapper->write_rom = mapper_sega_write_rom;
        mapper->read_sram = default_read_sram;
        mapper->write_sram = default_write_sram;

        // Alocar SRAM para jogos Sega que a utilizam
        mapper->sram_size = DEFAULT_SRAM_SIZE;
        mapper->sram_data = (uint8_t *)calloc(1, mapper->sram_size);
        if (!mapper->sram_data)
        {
            LOG_ERROR("Falha ao alocar memória para SRAM");
            return false;
        }

        // Endereços padrão da SRAM no espaço de memória
        mapper->sram_start = 0x200000;
        mapper->sram_end = 0x20FFFF;
        break;

    case MD_MAPPER_SSF2:
        // Super Street Fighter 2 usa bancos de 512KB
        mapper->num_banks = rom_size / (512 * 1024);
        mapper->bank_size = 512 * 1024;
        mapper->read_rom = mapper_ssf2_read_rom;
        mapper->write_rom = mapper_ssf2_write_rom;
        mapper->read_sram = default_read_sram;
        mapper->write_sram = default_write_sram;

        // Inicializar configuração de bancos
        for (uint32_t i = 0; i < 8; i++)
        {
            mapper->current_bank[i] = i % mapper->num_banks;
        }
        break;

    case MD_MAPPER_SSRPG:
        // Jogos RPG da Sega como Phantasy Star ou Shining Force
        mapper->num_banks = 1;
        mapper->bank_size = rom_size;
        mapper->read_rom = mapper_ssrpg_read_rom;
        mapper->write_rom = mapper_ssrpg_write_rom;
        mapper->read_sram = ssrpg_read_sram;
        mapper->write_sram = ssrpg_write_sram;

        // Alocar SRAM para jogos RPG
        mapper->sram_size = DEFAULT_SRAM_SIZE;
        mapper->sram_data = (uint8_t *)calloc(1, mapper->sram_size);
        if (!mapper->sram_data)
        {
            LOG_ERROR("Falha ao alocar memória para SRAM");
            return false;
        }

        // Endereços específicos para jogos RPG
        mapper->sram_start = 0x200000;
        mapper->sram_end = 0x20FFFF;
        break;

    case MD_MAPPER_EEPROM:
        // Jogos com EEPROM integrada
        mapper->num_banks = 1;
        mapper->bank_size = rom_size;
        mapper->read_rom = mapper_eeprom_read_rom;
        mapper->write_rom = mapper_eeprom_write_rom;
        mapper->read_sram = eeprom_read_sram;
        mapper->write_sram = eeprom_write_sram;

        // Alocar memória para EEPROM (normalmente pequena, 8KB)
        mapper->eeprom_size = 8 * 1024;
        mapper->eeprom_data = (uint8_t *)calloc(1, mapper->eeprom_size);
        if (!mapper->eeprom_data)
        {
            LOG_ERROR("Falha ao alocar memória para EEPROM");
            return false;
        }
        break;

    case MD_MAPPER_CODEMASTERS:
        // Mapper específico da Codemasters
        mapper->num_banks = rom_size / (16 * 1024);
        mapper->bank_size = 16 * 1024;
        mapper->read_rom = mapper_codemasters_read_rom;
        mapper->write_rom = mapper_codemasters_write_rom;
        mapper->read_sram = default_read_sram;
        mapper->write_sram = default_write_sram;

        // Inicializar configuração de bancos
        for (uint32_t i = 0; i < 8; i++)
        {
            mapper->current_bank[i] = i % mapper->num_banks;
        }
        break;

    case MD_MAPPER_PIER_SOLAR:
        // Jogos homebrew com chips especiais (como Pier Solar)
        mapper->num_banks = rom_size / (512 * 1024);
        mapper->bank_size = 512 * 1024;
        mapper->read_rom = mapper_pier_solar_read_rom;
        mapper->write_rom = mapper_pier_solar_write_rom;
        mapper->read_sram = default_read_sram;
        mapper->write_sram = default_write_sram;

        // Alocar memória para SRAM (Pier Solar usa 1MB)
        mapper->sram_size = 1024 * 1024;
        mapper->sram_data = (uint8_t *)calloc(1, mapper->sram_size);
        if (!mapper->sram_data)
        {
            LOG_ERROR("Falha ao alocar memória para SRAM");
            return false;
        }

        // Endereços específicos para Pier Solar
        mapper->sram_start = 0x200000;
        mapper->sram_end = 0x2FFFFF;
        break;

    case MD_MAPPER_EA:
        // Electronic Arts mapper
        mapper->num_banks = rom_size / (16 * 1024);
        mapper->bank_size = 16 * 1024;
        mapper->read_rom = mapper_ea_read_rom;
        mapper->write_rom = mapper_ea_write_rom;
        mapper->read_sram = default_read_sram;
        mapper->write_sram = default_write_sram;

        // Inicializar configuração de bancos
        for (uint32_t i = 0; i < 8; i++)
        {
            mapper->current_bank[i] = i % mapper->num_banks;
        }
        break;

    default:
        LOG_ERROR("Tipo de mapper desconhecido: %d", type);
        return false;
    }

    LOG_INFO("Mapper inicializado: tipo=%d, num_banks=%u, bank_size=%u bytes",
             type, mapper->num_banks, mapper->bank_size);

    return true;
}

/**
 * @brief Reseta um mapper do Mega Drive
 */
void md_mapper_reset(md_mapper_t *mapper)
{
    if (!mapper)
    {
        return;
    }

    // Resetar configuração de bancos para cada tipo de mapper
    switch (mapper->type)
    {
    case MD_MAPPER_NONE:
    case MD_MAPPER_SEGA:
    case MD_MAPPER_SSRPG:
    case MD_MAPPER_EEPROM:
        // Esses mappers não têm configuração de banco específica para resetar
        break;

    case MD_MAPPER_SSF2:
    case MD_MAPPER_CODEMASTERS:
    case MD_MAPPER_PIER_SOLAR:
    case MD_MAPPER_EA:
        // Resetar configuração de bancos
        for (uint32_t i = 0; i < 8; i++)
        {
            mapper->current_bank[i] = i % mapper->num_banks;
        }
        break;
    }

    // Resetar estado da EEPROM, se aplicável
    if (mapper->type == MD_MAPPER_EEPROM)
    {
        mapper->eeprom_state = 0;
        mapper->eeprom_address = 0;
    }
}

/**
 * @brief Libera os recursos de um mapper do Mega Drive
 */
void md_mapper_shutdown(md_mapper_t *mapper)
{
    if (!mapper)
    {
        return;
    }

    // Liberar SRAM, se alocada
    if (mapper->sram_data)
    {
        free(mapper->sram_data);
        mapper->sram_data = NULL;
    }

    // Liberar EEPROM, se alocada
    if (mapper->eeprom_data)
    {
        free(mapper->eeprom_data);
        mapper->eeprom_data = NULL;
    }

    // Não liberamos rom_data, pois é gerenciado externamente
}

/**
 * @brief Lê um byte da ROM mapeada
 */
uint8_t md_mapper_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->read_rom)
    {
        return 0xFF;
    }

    return mapper->read_rom(mapper, address);
}

/**
 * @brief Escreve um byte na ROM mapeada
 */
void md_mapper_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper || !mapper->write_rom)
    {
        return;
    }

    mapper->write_rom(mapper, address, value);
}

/**
 * @brief Lê um byte da SRAM mapeada
 */
uint8_t md_mapper_read_sram(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->read_sram)
    {
        return 0xFF;
    }

    return mapper->read_sram(mapper, address);
}

/**
 * @brief Escreve um byte na SRAM mapeada
 */
void md_mapper_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper || !mapper->write_sram)
    {
        return;
    }

    mapper->write_sram(mapper, address, value);
}

/**
 * @brief Ativa/desativa a SRAM
 */
void md_mapper_set_sram_enabled(md_mapper_t *mapper, bool enabled)
{
    if (!mapper)
    {
        return;
    }

    mapper->sram_enabled = enabled;
}

/**
 * @brief Salva o conteúdo da SRAM em um arquivo
 */
bool md_mapper_save_sram(md_mapper_t *mapper, const char *filename)
{
    if (!mapper || !filename || !mapper->sram_data || mapper->sram_size == 0)
    {
        return false;
    }

    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        LOG_ERROR("Falha ao abrir arquivo para escrita: %s", filename);
        return false;
    }

    size_t written = fwrite(mapper->sram_data, 1, mapper->sram_size, file);
    fclose(file);

    if (written != mapper->sram_size)
    {
        LOG_ERROR("Falha ao escrever SRAM no arquivo: %s", filename);
        return false;
    }

    LOG_INFO("SRAM salva com sucesso: %s", filename);
    return true;
}

/**
 * @brief Carrega o conteúdo da SRAM de um arquivo
 */
bool md_mapper_load_sram(md_mapper_t *mapper, const char *filename)
{
    if (!mapper || !filename || !mapper->sram_data || mapper->sram_size == 0)
    {
        return false;
    }

    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        LOG_ERROR("Falha ao abrir arquivo para leitura: %s", filename);
        return false;
    }

    size_t read = fread(mapper->sram_data, 1, mapper->sram_size, file);
    fclose(file);

    if (read != mapper->sram_size)
    {
        LOG_WARNING("Tamanho do arquivo SRAM diferente do esperado: %zu != %u", read, mapper->sram_size);
        // Não consideramos isso um erro fatal
    }

    LOG_INFO("SRAM carregada com sucesso: %s", filename);
    return true;
}

/**
 * @brief Detecta o tipo de mapper com base nos dados da ROM
 */
md_mapper_type_t md_mapper_detect_type(const uint8_t *rom_data, uint32_t rom_size)
{
    if (!rom_data || rom_size < 0x200)
    {
        return MD_MAPPER_NONE;
    }

    // Verificar assinaturas específicas no cabeçalho da ROM

    // Super Street Fighter 2 (verificar nome no cabeçalho)
    if (strncmp((const char *)&rom_data[0x120], "SUPER STREET FIGHTER2", 21) == 0)
    {
        return MD_MAPPER_SSF2;
    }

    // Jogos RPG da Sega (verificar SRAM no cabeçalho)
    // Endereços 0x1B0-0x1B3 contêm informações sobre SRAM
    if (rom_data[0x1B0] == 'R' && rom_data[0x1B1] == 'A')
    {
        return MD_MAPPER_SSRPG;
    }

    // Jogos com EEPROM (verificar I/O support no cabeçalho)
    if (strncmp((const char *)&rom_data[0x190], "MCD:J", 5) == 0 ||
        strncmp((const char *)&rom_data[0x190], "MCD:E", 5) == 0)
    {
        return MD_MAPPER_EEPROM;
    }

    // Jogos da Codemasters (verificar publisher no cabeçalho)
    if (strncmp((const char *)&rom_data[0x110], "(C)CODEMASTERS", 14) == 0)
    {
        return MD_MAPPER_CODEMASTERS;
    }

    // Pier Solar (verificar nome no cabeçalho)
    if (strncmp((const char *)&rom_data[0x120], "PIER SOLAR", 10) == 0)
    {
        return MD_MAPPER_PIER_SOLAR;
    }

    // Electronic Arts (verificar publisher no cabeçalho)
    if (strncmp((const char *)&rom_data[0x110], "EAI", 3) == 0 ||
        strncmp((const char *)&rom_data[0x110], "ELECTRONIC ARTS", 15) == 0)
    {
        return MD_MAPPER_EA;
    }

    // Se não corresponder a nenhum tipo específico, usar o mapper Sega padrão
    return MD_MAPPER_SEGA;
}

/**
 * @brief Implementação de leitura para o mapper NONE
 */
static uint8_t mapper_none_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // Sem mapeamento, acesso direto à ROM
    if (address < mapper->rom_size)
    {
        return mapper->rom_data[address];
    }

    // Endereço fora da ROM
    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper NONE
 */
static void mapper_none_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    // ROMs são somente leitura, ignore escritas para o mapper NONE
    (void)mapper;
    (void)address;
    (void)value;
}

/**
 * @brief Implementação de leitura para o mapper SEGA
 */
static uint8_t mapper_sega_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // Verificar se o endereço está na área de SRAM
    if (mapper->sram_enabled &&
        address >= mapper->sram_start &&
        address <= mapper->sram_end)
    {
        return mapper->read_sram(mapper, address - mapper->sram_start);
    }

    // Acesso direto à ROM se estiver dentro do tamanho
    if (address < mapper->rom_size)
    {
        return mapper->rom_data[address];
    }

    // "Mirroring" de ROM para endereços fora do tamanho original
    if (address < 0x400000)
    { // Espaço máximo de cartridge de 4MB
        return mapper->rom_data[address % mapper->rom_size];
    }

    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper SEGA
 */
static void mapper_sega_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper)
    {
        return;
    }

    // Verificar se o endereço está na área de SRAM
    if (mapper->sram_enabled &&
        address >= mapper->sram_start &&
        address <= mapper->sram_end)
    {
        mapper->write_sram(mapper, address - mapper->sram_start, value);
        return;
    }

    // Verificar escrita para registradores de controle de SRAM
    if (address == 0xA13000 || address == 0xA13001)
    {
        // Escrita para o registrador de controle de SRAM
        // Bit 0: 0 = SRAM desabilitada, 1 = SRAM habilitada
        mapper->sram_enabled = (value & 0x01) != 0;
        return;
    }
}

/**
 * @brief Implementação de leitura para o mapper SSF2 (Super Street Fighter 2)
 */
static uint8_t mapper_ssf2_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // SSF2 tem 8 bancos de 512KB
    uint32_t bank_index = (address >> 19) & 0x07; // Obtém o índice do banco (0-7)
    uint32_t bank_offset = address & 0x7FFFF;     // Offset dentro do banco (0-512KB)
    uint32_t rom_address = (mapper->current_bank[bank_index] * mapper->bank_size) + bank_offset;

    if (rom_address < mapper->rom_size)
    {
        return mapper->rom_data[rom_address];
    }

    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper SSF2
 */
static void mapper_ssf2_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper)
    {
        return;
    }

    // Registradores de bancos no SSF2 estão em 0xA130xx
    if ((address & 0xFFFF00) == 0xA13000)
    {
        uint32_t reg = address & 0xFF;
        if (reg <= 0x0F)
        {
            // Registradores para mapear bancos
            uint32_t bank_index = reg & 0x07;
            mapper->current_bank[bank_index] = value % mapper->num_banks;
        }
    }
}

/**
 * @brief Implementação de leitura para o mapper SSRPG (Sega RPG)
 */
static uint8_t mapper_ssrpg_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // Verificar se o endereço está na área de SRAM
    if (mapper->sram_enabled &&
        address >= mapper->sram_start &&
        address <= mapper->sram_end)
    {
        return mapper->read_sram(mapper, address - mapper->sram_start);
    }

    // Acesso normal à ROM
    if (address < mapper->rom_size)
    {
        return mapper->rom_data[address];
    }

    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper SSRPG
 */
static void mapper_ssrpg_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper)
    {
        return;
    }

    // Verificar se o endereço está na área de SRAM
    if (mapper->sram_enabled &&
        address >= mapper->sram_start &&
        address <= mapper->sram_end)
    {
        mapper->write_sram(mapper, address - mapper->sram_start, value);
        return;
    }

    // Registrador de controle de SRAM em 0xA130F1
    if (address == 0xA130F1)
    {
        // Bit 0: 0 = SRAM desabilitada, 1 = SRAM habilitada
        mapper->sram_enabled = (value & 0x01) != 0;
        return;
    }
}

/**
 * @brief Leitura de SRAM específica para jogos RPG da Sega
 */
static uint8_t ssrpg_read_sram(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->sram_data || !mapper->sram_enabled)
    {
        return 0xFF;
    }

    // RPGs da Sega têm SRAM em bancos menores
    if (address < mapper->sram_size)
    {
        return mapper->sram_data[address];
    }

    return 0xFF;
}

/**
 * @brief Escrita de SRAM específica para jogos RPG da Sega
 */
static void ssrpg_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper || !mapper->sram_data || !mapper->sram_enabled)
    {
        return;
    }

    if (address < mapper->sram_size)
    {
        mapper->sram_data[address] = value;
    }
}

/**
 * @brief Implementação de leitura para o mapper EEPROM
 */
static uint8_t mapper_eeprom_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // Verificar se o endereço é para a EEPROM (normalmente em 0x200000)
    if (address >= 0x200000 && address < 0x201000)
    {
        return mapper->read_sram(mapper, address - 0x200000);
    }

    // Acesso normal à ROM
    if (address < mapper->rom_size)
    {
        return mapper->rom_data[address];
    }

    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper EEPROM
 */
static void mapper_eeprom_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper)
    {
        return;
    }

    // Verificar se o endereço é para a EEPROM (normalmente em 0x200000)
    if (address >= 0x200000 && address < 0x201000)
    {
        mapper->write_sram(mapper, address - 0x200000, value);
        return;
    }
}

/**
 * @brief Leitura de EEPROM (implementação simplificada)
 */
static uint8_t eeprom_read_sram(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->eeprom_data)
    {
        return 0xFF;
    }

    // Implementação simplificada - na realidade, EEPROM tem protocolo específico
    if (address < mapper->eeprom_size)
    {
        return mapper->eeprom_data[address];
    }

    return 0xFF;
}

/**
 * @brief Escrita de EEPROM (implementação simplificada)
 */
static void eeprom_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper || !mapper->eeprom_data)
    {
        return;
    }

    // Implementação simplificada - na realidade, EEPROM tem protocolo específico
    if (address < mapper->eeprom_size)
    {
        mapper->eeprom_data[address] = value;
    }
}

/**
 * @brief Implementação de leitura para o mapper Codemasters
 */
static uint8_t mapper_codemasters_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // Mapper da Codemasters tem bancos de 16KB
    uint32_t bank_index = (address >> 14) & 0x07; // Obtém o índice do banco (0-7)
    uint32_t bank_offset = address & 0x3FFF;      // Offset dentro do banco (0-16KB)
    uint32_t rom_address = (mapper->current_bank[bank_index] * mapper->bank_size) + bank_offset;

    if (rom_address < mapper->rom_size)
    {
        return mapper->rom_data[rom_address];
    }

    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper Codemasters
 */
static void mapper_codemasters_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper)
    {
        return;
    }

    // Registradores de bancos da Codemasters estão em 0x00xxxx
    if ((address & 0xFE0000) == 0x000000)
    {
        // Registradores 0x0000, 0x4000, 0x8000, 0xC000 controlam os bancos
        uint32_t reg = address & 0xC000;
        uint32_t bank_index = reg >> 14;
        mapper->current_bank[bank_index] = value % mapper->num_banks;
    }
}

/**
 * @brief Implementação de leitura para o mapper Pier Solar
 */
static uint8_t mapper_pier_solar_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // Verificar se o endereço está na área de SRAM (Pier Solar tem SRAM grande)
    if (mapper->sram_enabled &&
        address >= mapper->sram_start &&
        address <= mapper->sram_end)
    {
        return mapper->read_sram(mapper, address - mapper->sram_start);
    }

    // Pier Solar tem bancos de 512KB
    uint32_t bank_index = (address >> 19) & 0x07; // Obtém o índice do banco (0-7)
    uint32_t bank_offset = address & 0x7FFFF;     // Offset dentro do banco (0-512KB)
    uint32_t rom_address = (mapper->current_bank[bank_index] * mapper->bank_size) + bank_offset;

    if (rom_address < mapper->rom_size)
    {
        return mapper->rom_data[rom_address];
    }

    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper Pier Solar
 */
static void mapper_pier_solar_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper)
    {
        return;
    }

    // Verificar se o endereço está na área de SRAM
    if (mapper->sram_enabled &&
        address >= mapper->sram_start &&
        address <= mapper->sram_end)
    {
        mapper->write_sram(mapper, address - mapper->sram_start, value);
        return;
    }

    // Registradores de bancos do Pier Solar estão em 0xA130xx
    if ((address & 0xFFFF00) == 0xA13000)
    {
        uint32_t reg = address & 0xFF;
        if (reg <= 0x0F)
        {
            // Registradores para mapear bancos
            uint32_t bank_index = reg & 0x07;
            mapper->current_bank[bank_index] = value % mapper->num_banks;
        }
        else if (reg == 0x10)
        {
            // Controle de SRAM
            mapper->sram_enabled = (value & 0x01) != 0;
        }
    }
}

/**
 * @brief Implementação de leitura para o mapper EA
 */
static uint8_t mapper_ea_read_rom(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->rom_data)
    {
        return 0xFF;
    }

    // Mapper da EA tem bancos de 16KB
    uint32_t bank_index = (address >> 14) & 0x07; // Obtém o índice do banco (0-7)
    uint32_t bank_offset = address & 0x3FFF;      // Offset dentro do banco (0-16KB)
    uint32_t rom_address = (mapper->current_bank[bank_index] * mapper->bank_size) + bank_offset;

    if (rom_address < mapper->rom_size)
    {
        return mapper->rom_data[rom_address];
    }

    return 0xFF;
}

/**
 * @brief Implementação de escrita para o mapper EA
 */
static void mapper_ea_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper)
    {
        return;
    }

    // Registradores de bancos da EA estão em 0xA130xx
    if ((address & 0xFFFF00) == 0xA13000)
    {
        uint32_t reg = address & 0xFF;
        if (reg >= 0x00 && reg <= 0x07)
        {
            // Registradores para mapear bancos
            uint32_t bank_index = reg & 0x07;
            mapper->current_bank[bank_index] = value % mapper->num_banks;
        }
    }
}

/**
 * @brief Leitura de SRAM padrão
 */
static uint8_t default_read_sram(md_mapper_t *mapper, uint32_t address)
{
    if (!mapper || !mapper->sram_data || !mapper->sram_enabled)
    {
        return 0xFF;
    }

    if (address < mapper->sram_size)
    {
        return mapper->sram_data[address];
    }

    return 0xFF;
}

/**
 * @brief Escrita de SRAM padrão
 */
static void default_write_sram(md_mapper_t *mapper, uint32_t address, uint8_t value)
{
    if (!mapper || !mapper->sram_data || !mapper->sram_enabled)
    {
        return;
    }

    if (address < mapper->sram_size)
    {
        mapper->sram_data[address] = value;
    }
}

/**
 * @brief Registra o mapper no sistema de save state
 */
int32_t md_mapper_register_save_state(save_state_t *state)
{
    if (!state)
    {
        LOG_ERROR("Estado inválido para registro de mapper");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Obter o mapper global
    extern md_mapper_t g_md_mapper;

    // Registrar campos básicos
    save_state_register_field(state, "md_mapper_type", &g_md_mapper.type, sizeof(md_mapper_type_t));
    save_state_register_field(state, "md_mapper_num_banks", &g_md_mapper.num_banks, sizeof(uint32_t));
    save_state_register_field(state, "md_mapper_bank_size", &g_md_mapper.bank_size, sizeof(uint32_t));
    save_state_register_field(state, "md_mapper_current_banks", g_md_mapper.current_bank, sizeof(uint32_t) * 8);
    save_state_register_field(state, "md_mapper_sram_enabled", &g_md_mapper.sram_enabled, sizeof(bool));
    save_state_register_field(state, "md_mapper_sram_start", &g_md_mapper.sram_start, sizeof(uint32_t));
    save_state_register_field(state, "md_mapper_sram_end", &g_md_mapper.sram_end, sizeof(uint32_t));

    // Registrar conteúdo da SRAM se existir
    if (g_md_mapper.sram_data && g_md_mapper.sram_size > 0)
    {
        save_state_register_field(state, "md_mapper_sram_size", &g_md_mapper.sram_size, sizeof(uint32_t));
        save_state_register_field(state, "md_mapper_sram_data", g_md_mapper.sram_data, g_md_mapper.sram_size);
    }

    // Registrar dados de EEPROM se existirem
    if (g_md_mapper.eeprom_data && g_md_mapper.eeprom_size > 0)
    {
        save_state_register_field(state, "md_mapper_eeprom_size", &g_md_mapper.eeprom_size, sizeof(uint32_t));
        save_state_register_field(state, "md_mapper_eeprom_data", g_md_mapper.eeprom_data, g_md_mapper.eeprom_size);
        save_state_register_field(state, "md_mapper_eeprom_state", &g_md_mapper.eeprom_state, sizeof(uint8_t));
        save_state_register_field(state, "md_mapper_eeprom_address", &g_md_mapper.eeprom_address, sizeof(uint16_t));
    }

    LOG_INFO("Mapper registrado no sistema de save state");
    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Restaura o estado do mapper a partir de um save state
 */
int32_t md_mapper_restore_save_state(save_state_t *state)
{
    if (!state)
    {
        LOG_ERROR("Estado inválido para restauração de mapper");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Obter o mapper global
    extern md_mapper_t g_md_mapper;

    // Restaurar campos básicos
    md_mapper_type_t type;
    if (save_state_read_field(state, "md_mapper_type", &type, sizeof(md_mapper_type_t)) != SAVE_STATE_ERROR_NONE)
    {
        LOG_ERROR("Falha ao ler tipo de mapper do save state");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Preservar ponteiros importantes
    uint8_t *rom_data = g_md_mapper.rom_data;
    uint32_t rom_size = g_md_mapper.rom_size;

    // Se o tipo de mapper mudou, reinicializar
    if (type != g_md_mapper.type)
    {
        LOG_INFO("Tipo de mapper alterado de %d para %d, reinicializando", g_md_mapper.type, type);
        md_mapper_shutdown(&g_md_mapper);
        if (!md_mapper_init(&g_md_mapper, type, rom_data, rom_size))
        {
            LOG_ERROR("Falha ao reinicializar mapper");
            return SAVE_STATE_ERROR_INVALID;
        }
    }

    // Restaurar campos
    save_state_read_field(state, "md_mapper_num_banks", &g_md_mapper.num_banks, sizeof(uint32_t));
    save_state_read_field(state, "md_mapper_bank_size", &g_md_mapper.bank_size, sizeof(uint32_t));
    save_state_read_field(state, "md_mapper_current_banks", g_md_mapper.current_bank, sizeof(uint32_t) * 8);
    save_state_read_field(state, "md_mapper_sram_enabled", &g_md_mapper.sram_enabled, sizeof(bool));
    save_state_read_field(state, "md_mapper_sram_start", &g_md_mapper.sram_start, sizeof(uint32_t));
    save_state_read_field(state, "md_mapper_sram_end", &g_md_mapper.sram_end, sizeof(uint32_t));

    // Restaurar conteúdo da SRAM
    uint32_t sram_size;
    if (save_state_read_field(state, "md_mapper_sram_size", &sram_size, sizeof(uint32_t)) == SAVE_STATE_ERROR_NONE)
    {
        // Se o tamanho da SRAM mudou, realocar
        if (sram_size != g_md_mapper.sram_size)
        {
            LOG_INFO("Tamanho da SRAM alterado de %u para %u bytes", g_md_mapper.sram_size, sram_size);

            if (g_md_mapper.sram_data)
            {
                free(g_md_mapper.sram_data);
            }

            g_md_mapper.sram_data = (uint8_t *)malloc(sram_size);
            if (!g_md_mapper.sram_data)
            {
                LOG_ERROR("Falha ao alocar memória para SRAM");
                return SAVE_STATE_ERROR_MEMORY;
            }

            g_md_mapper.sram_size = sram_size;
        }

        // Restaurar dados da SRAM
        if (g_md_mapper.sram_data && g_md_mapper.sram_size > 0)
        {
            save_state_read_field(state, "md_mapper_sram_data", g_md_mapper.sram_data, g_md_mapper.sram_size);
        }
    }

    // Restaurar dados de EEPROM
    uint32_t eeprom_size;
    if (save_state_read_field(state, "md_mapper_eeprom_size", &eeprom_size, sizeof(uint32_t)) == SAVE_STATE_ERROR_NONE)
    {
        // Se o tamanho da EEPROM mudou, realocar
        if (eeprom_size != g_md_mapper.eeprom_size)
        {
            LOG_INFO("Tamanho da EEPROM alterado de %u para %u bytes", g_md_mapper.eeprom_size, eeprom_size);

            if (g_md_mapper.eeprom_data)
            {
                free(g_md_mapper.eeprom_data);
            }

            g_md_mapper.eeprom_data = (uint8_t *)malloc(eeprom_size);
            if (!g_md_mapper.eeprom_data)
            {
                LOG_ERROR("Falha ao alocar memória para EEPROM");
                return SAVE_STATE_ERROR_MEMORY;
            }

            g_md_mapper.eeprom_size = eeprom_size;
        }

        // Restaurar dados da EEPROM
        if (g_md_mapper.eeprom_data && g_md_mapper.eeprom_size > 0)
        {
            save_state_read_field(state, "md_mapper_eeprom_data", g_md_mapper.eeprom_data, g_md_mapper.eeprom_size);
            save_state_read_field(state, "md_mapper_eeprom_state", &g_md_mapper.eeprom_state, sizeof(uint8_t));
            save_state_read_field(state, "md_mapper_eeprom_address", &g_md_mapper.eeprom_address, sizeof(uint16_t));
        }
    }

    LOG_INFO("Estado do mapper restaurado com sucesso");
    return SAVE_STATE_ERROR_NONE;
}
