/**
 * @file sms_cartridge.c
 * @brief Implementação do sistema de cartucho do Master System
 */

#include "sms_cartridge.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../../../utils/crc32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o cartucho do Master System
#define EMU_LOG_CAT_CARTRIDGE EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para o cartucho do Master System
#define SMS_CART_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)

/**
 * @brief Tipos de mapper suportados
 */
typedef enum {
    SMS_MAPPER_NONE = 0,      // ROM simples sem mapper
    SMS_MAPPER_SEGA,          // Mapper padrão da Sega
    SMS_MAPPER_CODEMASTERS,   // Mapper da Codemasters
    SMS_MAPPER_KOREAN         // Mapper coreano
} sms_mapper_type_t;

/**
 * @brief Estrutura interna do cartucho
 */
struct sms_cartridge_t {
    uint8_t *rom_data;            // Dados da ROM
    uint32_t rom_size;            // Tamanho da ROM em bytes
    uint32_t rom_mask;            // Máscara para acesso à ROM
    uint8_t *ram_data;            // Dados da RAM (se presente)
    uint32_t ram_size;            // Tamanho da RAM em bytes
    uint8_t has_battery;          // Flag indicando se tem bateria para salvar
    sms_mapper_type_t mapper_type;// Tipo de mapper
    uint8_t mapper_regs[4];       // Registradores do mapper
    sms_rom_info_t rom_info;      // Informações sobre a ROM
};

/**
 * @brief Detecta o tipo de mapper com base no cabeçalho e tamanho da ROM
 * 
 * @param cartridge Ponteiro para a instância
 */
static void sms_cartridge_detect_mapper(sms_cartridge_t *cartridge) {
    if (!cartridge || !cartridge->rom_data) {
        return;
    }
    
    // Por padrão, assume mapper Sega
    cartridge->mapper_type = SMS_MAPPER_SEGA;
    
    // Verifica assinaturas conhecidas para mappers específicos
    
    // Codemasters: verifica assinatura no cabeçalho
    if (cartridge->rom_size >= 0x8000) {
        // Verifica padrões conhecidos de ROMs da Codemasters
        if (cartridge->rom_data[0x7FF0] == 0x55 && cartridge->rom_data[0x7FF1] == 0xAA) {
            cartridge->mapper_type = SMS_MAPPER_CODEMASTERS;
            SMS_CART_LOG_INFO("Mapper Codemasters detectado");
            return;
        }
    }
    
    // Korean: verifica padrões conhecidos
    if (cartridge->rom_size >= 0x8000) {
        // Alguns jogos coreanos têm padrões específicos
        // Esta é uma simplificação, na prática seria necessário verificar títulos específicos
        if (strstr(cartridge->rom_info.title, "Sangokushi") || 
            strstr(cartridge->rom_info.title, "Jang Gun")) {
            cartridge->mapper_type = SMS_MAPPER_KOREAN;
            SMS_CART_LOG_INFO("Mapper coreano detectado");
            return;
        }
    }
    
    // Se a ROM for muito pequena, não precisa de mapper
    if (cartridge->rom_size <= 0x8000) {
        cartridge->mapper_type = SMS_MAPPER_NONE;
        SMS_CART_LOG_INFO("ROM pequena, sem mapper");
        return;
    }
    
    SMS_CART_LOG_INFO("Usando mapper padrão Sega");
}

/**
 * @brief Extrai informações do cabeçalho da ROM
 * 
 * @param cartridge Ponteiro para a instância
 */
static void sms_cartridge_parse_header(sms_cartridge_t *cartridge) {
    if (!cartridge || !cartridge->rom_data || cartridge->rom_size < 0x8000) {
        return;
    }
    
    // O cabeçalho do Master System está localizado em 0x7FF0-0x7FFF
    uint8_t *header = &cartridge->rom_data[0x7FF0];
    
    // Verifica se o cabeçalho é válido (deve começar com "TMR SEGA")
    if (header[0] == 'T' && header[1] == 'M' && header[2] == 'R' && 
        header[3] == ' ' && header[4] == 'S' && header[5] == 'E' && 
        header[6] == 'G' && header[7] == 'A') {
        
        // Extrai o título (até 32 caracteres)
        memset(cartridge->rom_info.title, 0, sizeof(cartridge->rom_info.title));
        strncpy(cartridge->rom_info.title, (const char*)&cartridge->rom_data[0x7FF0 + 16], 32);
        
        // Garante que o título termine com nulo
        cartridge->rom_info.title[32] = '\0';
        
        // Extrai informações de região
        cartridge->rom_info.region = (header[15] & 0xF0) >> 4;
        
        // Verifica se tem bateria
        cartridge->has_battery = (header[15] & 0x08) ? 1 : 0;
        cartridge->rom_info.has_battery = cartridge->has_battery;
        
        SMS_CART_LOG_INFO("Cabeçalho válido encontrado: %s", cartridge->rom_info.title);
    } else {
        // Cabeçalho inválido, usa nome genérico
        strcpy(cartridge->rom_info.title, "Unknown SMS Game");
        cartridge->rom_info.region = 3; // Desconhecido
        cartridge->has_battery = 0;
        cartridge->rom_info.has_battery = 0;
        
        SMS_CART_LOG_WARN("Cabeçalho inválido, usando nome genérico");
    }
    
    // Define o tipo de mapper com base no cabeçalho
    sms_cartridge_detect_mapper(cartridge);
    cartridge->rom_info.mapper_type = cartridge->mapper_type;
}

/**
 * @brief Cria uma nova instância do cartucho
 * 
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_cartridge_t* sms_cartridge_create(void) {
    sms_cartridge_t *cartridge = (sms_cartridge_t*)malloc(sizeof(sms_cartridge_t));
    if (!cartridge) {
        SMS_CART_LOG_ERROR("Falha ao alocar memória para o cartucho");
        return NULL;
    }
    
    // Inicializa a estrutura
    memset(cartridge, 0, sizeof(sms_cartridge_t));
    
    SMS_CART_LOG_INFO("Cartucho criado com sucesso");
    
    return cartridge;
}

/**
 * @brief Destrói uma instância do cartucho e libera recursos
 * 
 * @param cartridge Ponteiro para a instância
 */
void sms_cartridge_destroy(sms_cartridge_t *cartridge) {
    if (!cartridge) {
        return;
    }
    
    // Libera a ROM se estiver carregada
    if (cartridge->rom_data) {
        free(cartridge->rom_data);
        cartridge->rom_data = NULL;
    }
    
    // Libera a RAM se estiver alocada
    if (cartridge->ram_data) {
        free(cartridge->ram_data);
        cartridge->ram_data = NULL;
    }
    
    // Libera a estrutura principal
    free(cartridge);
    
    SMS_CART_LOG_INFO("Cartucho destruído");
}

/**
 * @brief Reseta o cartucho para o estado inicial
 * 
 * @param cartridge Ponteiro para a instância
 */
void sms_cartridge_reset(sms_cartridge_t *cartridge) {
    if (!cartridge) {
        return;
    }
    
    // Reseta os registradores do mapper
    memset(cartridge->mapper_regs, 0, sizeof(cartridge->mapper_regs));
    
    SMS_CART_LOG_INFO("Cartucho resetado");
}

/**
 * @brief Carrega uma ROM no cartucho
 * 
 * @param cartridge Ponteiro para a instância
 * @param rom_path Caminho para o arquivo ROM
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_load_rom(sms_cartridge_t *cartridge, const char *rom_path) {
    if (!cartridge || !rom_path) {
        return -1;
    }
    
    // Libera a ROM anterior se existir
    if (cartridge->rom_data) {
        free(cartridge->rom_data);
        cartridge->rom_data = NULL;
        cartridge->rom_size = 0;
    }
    
    // Abre o arquivo da ROM
    FILE *file = fopen(rom_path, "rb");
    if (!file) {
        SMS_CART_LOG_ERROR("Falha ao abrir arquivo ROM: %s", rom_path);
        return -1;
    }
    
    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Verifica se o tamanho é válido
    if (size <= 0 || size > SMS_MAX_ROM_SIZE) {
        SMS_CART_LOG_ERROR("Tamanho da ROM inválido: %ld bytes (máximo: %d bytes)", size, SMS_MAX_ROM_SIZE);
        fclose(file);
        return -1;
    }
    
    // Aloca memória para a ROM
    cartridge->rom_data = (uint8_t*)malloc(size);
    if (!cartridge->rom_data) {
        SMS_CART_LOG_ERROR("Falha ao alocar memória para a ROM");
        fclose(file);
        return -1;
    }
    
    // Lê o arquivo da ROM
    size_t read = fread(cartridge->rom_data, 1, size, file);
    fclose(file);
    
    if (read != size) {
        SMS_CART_LOG_ERROR("Falha ao ler arquivo ROM: %s", rom_path);
        free(cartridge->rom_data);
        cartridge->rom_data = NULL;
        return -1;
    }
    
    cartridge->rom_size = size;
    
    // Calcula a máscara da ROM (potência de 2 mais próxima)
    uint32_t mask = 1;
    while (mask < size) {
        mask <<= 1;
    }
    cartridge->rom_mask = mask - 1;
    
    // Extrai informações do cabeçalho
    sms_cartridge_parse_header(cartridge);
    
    // Calcula o checksum CRC32
    cartridge->rom_info.checksum = crc32_calculate(cartridge->rom_data, cartridge->rom_size);
    cartridge->rom_info.size = cartridge->rom_size;
    cartridge->rom_info.rom_data = cartridge->rom_data;
    
    // Aloca RAM para o cartucho se necessário
    if (cartridge->has_battery) {
        cartridge->ram_size = SMS_MAX_RAM_SIZE;
        cartridge->ram_data = (uint8_t*)malloc(cartridge->ram_size);
        if (!cartridge->ram_data) {
            SMS_CART_LOG_ERROR("Falha ao alocar memória para a RAM do cartucho");
            // Não é um erro fatal, podemos continuar sem a RAM
            cartridge->ram_size = 0;
        } else {
            // Inicializa a RAM com zeros
            memset(cartridge->ram_data, 0, cartridge->ram_size);
        }
    }
    
    SMS_CART_LOG_INFO("ROM carregada com sucesso: %s (%d bytes, CRC32: %08X)", 
                     cartridge->rom_info.title, cartridge->rom_size, cartridge->rom_info.checksum);
    
    return 0;
}

/**
 * @brief Obtém informações sobre a ROM carregada
 * 
 * @param cartridge Ponteiro para a instância
 * @param info Ponteiro para a estrutura que receberá as informações
 */
void sms_cartridge_get_info(sms_cartridge_t *cartridge, sms_rom_info_t *info) {
    if (!cartridge || !info) {
        return;
    }
    
    // Copia as informações da ROM
    *info = cartridge->rom_info;
}

/**
 * @brief Lê um byte da ROM/RAM do cartucho
 * 
 * @param cartridge Ponteiro para a instância
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint8_t sms_cartridge_read(sms_cartridge_t *cartridge, uint16_t address) {
    if (!cartridge || !cartridge->rom_data) {
        return 0xFF;
    }
    
    // Mapeamento de memória baseado no tipo de mapper
    switch (cartridge->mapper_type) {
        case SMS_MAPPER_NONE:
            // Sem mapper, acesso direto à ROM
            if (address < cartridge->rom_size) {
                return cartridge->rom_data[address];
            }
            break;
            
        case SMS_MAPPER_SEGA:
            // Mapper padrão da Sega
            // Cada página de 16KB é mapeada de acordo com os registradores do mapper
            if (address < cartridge->rom_size) {
                return cartridge->rom_data[address & cartridge->rom_mask];
            }
            break;
            
        case SMS_MAPPER_CODEMASTERS:
            // Mapper da Codemasters
            // Implementação simplificada
            if (address < cartridge->rom_size) {
                return cartridge->rom_data[address & cartridge->rom_mask];
            }
            break;
            
        case SMS_MAPPER_KOREAN:
            // Mapper coreano
            // Implementação simplificada
            if (address < cartridge->rom_size) {
                return cartridge->rom_data[address & cartridge->rom_mask];
            }
            break;
    }
    
    // Acesso à RAM do cartucho (se presente)
    if (cartridge->ram_data && address >= 0x8000 && address < 0xC000) {
        uint32_t ram_addr = address - 0x8000;
        if (ram_addr < cartridge->ram_size) {
            return cartridge->ram_data[ram_addr];
        }
    }
    
    // Endereço não mapeado
    return 0xFF;
}

/**
 * @brief Escreve um byte na RAM do cartucho
 * 
 * @param cartridge Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_cartridge_write(sms_cartridge_t *cartridge, uint16_t address, uint8_t value) {
    if (!cartridge) {
        return;
    }
    
    // Escrita nos registradores do mapper
    switch (cartridge->mapper_type) {
        case SMS_MAPPER_NONE:
            // Sem mapper, não há registradores para escrever
            break;
            
        case SMS_MAPPER_SEGA:
            // Mapper padrão da Sega
            // Registradores em 0xFFFC-0xFFFF
            if (address >= 0xFFFC && address <= 0xFFFF) {
                uint8_t reg = address - 0xFFFC;
                cartridge->mapper_regs[reg] = value;
                SMS_CART_LOG_TRACE("Mapper Sega: registrador %d = %02X", reg, value);
            }
            break;
            
        case SMS_MAPPER_CODEMASTERS:
            // Mapper da Codemasters
            // Registradores em 0x0000, 0x4000, 0x8000
            if (address == 0x0000 || address == 0x4000 || address == 0x8000) {
                uint8_t reg = address >> 14;
                cartridge->mapper_regs[reg] = value;
                SMS_CART_LOG_TRACE("Mapper Codemasters: registrador %d = %02X", reg, value);
            }
            break;
            
        case SMS_MAPPER_KOREAN:
            // Mapper coreano
            // Implementação simplificada
            if (address == 0xA000) {
                cartridge->mapper_regs[0] = value;
                SMS_CART_LOG_TRACE("Mapper coreano: registrador 0 = %02X", value);
            }
            break;
    }
    
    // Escrita na RAM do cartucho (se presente)
    if (cartridge->ram_data && address >= 0x8000 && address < 0xC000) {
        uint32_t ram_addr = address - 0x8000;
        if (ram_addr < cartridge->ram_size) {
            cartridge->ram_data[ram_addr] = value;
        }
    }
}

/**
 * @brief Salva o conteúdo da RAM do cartucho em arquivo
 * 
 * @param cartridge Ponteiro para a instância
 * @param save_path Caminho para o arquivo de save
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_save_ram(sms_cartridge_t *cartridge, const char *save_path) {
    if (!cartridge || !cartridge->ram_data || !cartridge->has_battery || !save_path) {
        return -1;
    }
    
    // Abre o arquivo para escrita
    FILE *file = fopen(save_path, "wb");
    if (!file) {
        SMS_CART_LOG_ERROR("Falha ao abrir arquivo para salvar RAM: %s", save_path);
        return -1;
    }
    
    // Escreve o conteúdo da RAM
    size_t written = fwrite(cartridge->ram_data, 1, cartridge->ram_size, file);
    fclose(file);
    
    if (written != cartridge->ram_size) {
        SMS_CART_LOG_ERROR("Falha ao escrever arquivo de RAM: %s", save_path);
        return -1;
    }
    
    SMS_CART_LOG_INFO("RAM do cartucho salva com sucesso: %s", save_path);
    
    return 0;
}

/**
 * @brief Carrega o conteúdo da RAM do cartucho de um arquivo
 * 
 * @param cartridge Ponteiro para a instância
 * @param save_path Caminho para o arquivo de save
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_load_ram(sms_cartridge_t *cartridge, const char *save_path) {
    if (!cartridge || !cartridge->ram_data || !cartridge->has_battery || !save_path) {
        return -1;
    }
    
    // Abre o arquivo para leitura
    FILE *file = fopen(save_path, "rb");
    if (!file) {
        SMS_CART_LOG_WARN("Arquivo de RAM não encontrado: %s", save_path);
        return -1;
    }
    
    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Verifica se o tamanho é válido
    if (size != cartridge->ram_size) {
        SMS_CART_LOG_WARN("Tamanho do arquivo de RAM inválido: %ld bytes (esperado: %d bytes)", 
                         size, cartridge->ram_size);
        fclose(file);
        return -1;
    }
    
    // Lê o conteúdo da RAM
    size_t read = fread(cartridge->ram_data, 1, cartridge->ram_size, file);
    fclose(file);
    
    if (read != cartridge->ram_size) {
        SMS_CART_LOG_ERROR("Falha ao ler arquivo de RAM: %s", save_path);
        return -1;
    }
    
    SMS_CART_LOG_INFO("RAM do cartucho carregada com sucesso: %s", save_path);
    
    return 0;
}

/**
 * @brief Registra o cartucho no sistema de save state
 * 
 * @param cartridge Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_cartridge_register_save_state(sms_cartridge_t *cartridge, save_state_t *state) {
    if (!cartridge || !state) {
        return -1;
    }
    
    // Registra os registradores do mapper
    save_state_register_field(state, "sms_mapper_regs", cartridge->mapper_regs, sizeof(cartridge->mapper_regs));
    
    // Registra a RAM do cartucho (se presente)
    if (cartridge->ram_data && cartridge->ram_size > 0) {
        save_state_register_field(state, "sms_cart_ram", cartridge->ram_data, cartridge->ram_size);
    }
    
    SMS_CART_LOG_DEBUG("Cartucho registrado no sistema de save state");
    
    return 0;
}
