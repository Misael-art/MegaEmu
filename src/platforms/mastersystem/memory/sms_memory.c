/**
 * @file sms_memory.c
 * @brief Implementação do sistema de memória do Master System
 */

#include "sms_memory.h"
#include "sms_cartridge.h"
#include "../video/sms_vdp.h"
#include "../audio/sms_psg.h"
#include "../io/sms_io.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para a memória do Master System
#define EMU_LOG_CAT_MEMORY EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para a memória do Master System
#define SMS_MEM_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MEMORY, __VA_ARGS__)
#define SMS_MEM_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MEMORY, __VA_ARGS__)
#define SMS_MEM_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MEMORY, __VA_ARGS__)
#define SMS_MEM_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MEMORY, __VA_ARGS__)
#define SMS_MEM_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MEMORY, __VA_ARGS__)

/**
 * @brief Tamanhos de memória do Master System
 */
#define SMS_RAM_SIZE 0x2000          // 8KB RAM
#define SMS_SYSTEM_RAM_START 0xC000  // Início da RAM do sistema
#define SMS_SYSTEM_RAM_END 0xDFFF    // Fim da RAM do sistema
#define SMS_MIRROR_RAM_START 0xE000  // Início da RAM espelhada
#define SMS_MIRROR_RAM_END 0xFFFF    // Fim da RAM espelhada
#define SMS_BIOS_SIZE 0x2000         // 8KB BIOS ROM (opcional)

/**
 * @brief Estrutura interna do sistema de memória
 */
struct sms_memory_t {
    uint8_t ram[SMS_RAM_SIZE];           // RAM do sistema
    uint8_t *bios;                       // BIOS ROM (opcional)
    uint8_t has_bios;                    // Flag indicando se o BIOS está carregado
    uint8_t mapper_control;              // Registrador de controle do mapper
    uint8_t mapper_slots[4];             // Slots de paginação do mapper
    
    // Ponteiros para outros componentes
    sms_cartridge_t *cartridge;          // Cartucho
    sms_vdp_t *vdp;                      // VDP
    sms_psg_t *psg;                      // PSG
    sms_input_t *input;                  // Sistema de entrada
};

/**
 * @brief Cria uma nova instância do sistema de memória
 * 
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_memory_t* sms_memory_create(void) {
    sms_memory_t *memory = (sms_memory_t*)malloc(sizeof(sms_memory_t));
    if (!memory) {
        SMS_MEM_LOG_ERROR("Falha ao alocar memória para o sistema de memória");
        return NULL;
    }
    
    // Inicializa a estrutura
    memset(memory, 0, sizeof(sms_memory_t));
    
    // Inicializa a RAM com valores aleatórios (comportamento real do hardware)
    for (int i = 0; i < SMS_RAM_SIZE; i++) {
        memory->ram[i] = rand() & 0xFF;
    }
    
    // Inicializa os slots do mapper
    memory->mapper_slots[0] = 0;
    memory->mapper_slots[1] = 1;
    memory->mapper_slots[2] = 2;
    memory->mapper_slots[3] = 3;
    
    SMS_MEM_LOG_INFO("Sistema de memória criado com sucesso");
    
    return memory;
}

/**
 * @brief Destrói uma instância do sistema de memória e libera recursos
 * 
 * @param memory Ponteiro para a instância
 */
void sms_memory_destroy(sms_memory_t *memory) {
    if (!memory) {
        return;
    }
    
    // Libera o BIOS se estiver carregado
    if (memory->bios) {
        free(memory->bios);
        memory->bios = NULL;
    }
    
    // Libera a estrutura principal
    free(memory);
    
    SMS_MEM_LOG_INFO("Sistema de memória destruído");
}

/**
 * @brief Reseta o sistema de memória para o estado inicial
 * 
 * @param memory Ponteiro para a instância
 */
void sms_memory_reset(sms_memory_t *memory) {
    if (!memory) {
        return;
    }
    
    // Inicializa a RAM com valores aleatórios (comportamento real do hardware)
    for (int i = 0; i < SMS_RAM_SIZE; i++) {
        memory->ram[i] = rand() & 0xFF;
    }
    
    // Reseta os slots do mapper
    memory->mapper_slots[0] = 0;
    memory->mapper_slots[1] = 1;
    memory->mapper_slots[2] = 2;
    memory->mapper_slots[3] = 3;
    memory->mapper_control = 0;
    
    SMS_MEM_LOG_INFO("Sistema de memória resetado");
}

/**
 * @brief Conecta o sistema de memória ao VDP
 * 
 * @param memory Ponteiro para a instância
 * @param vdp Ponteiro para o VDP
 */
void sms_memory_connect_vdp(sms_memory_t *memory, void *vdp) {
    if (!memory) {
        return;
    }
    
    memory->vdp = (sms_vdp_t*)vdp;
    SMS_MEM_LOG_DEBUG("VDP conectado ao sistema de memória");
}

/**
 * @brief Conecta o sistema de memória ao PSG
 * 
 * @param memory Ponteiro para a instância
 * @param psg Ponteiro para o PSG
 */
void sms_memory_connect_psg(sms_memory_t *memory, void *psg) {
    if (!memory) {
        return;
    }
    
    memory->psg = (sms_psg_t*)psg;
    SMS_MEM_LOG_DEBUG("PSG conectado ao sistema de memória");
}

/**
 * @brief Conecta o sistema de memória ao sistema de entrada
 * 
 * @param memory Ponteiro para a instância
 * @param input Ponteiro para o sistema de entrada
 */
void sms_memory_connect_input(sms_memory_t *memory, void *input) {
    if (!memory) {
        return;
    }
    
    memory->input = (sms_input_t*)input;
    SMS_MEM_LOG_DEBUG("Sistema de entrada conectado ao sistema de memória");
}

/**
 * @brief Conecta o sistema de memória ao cartucho
 * 
 * @param memory Ponteiro para a instância
 * @param cartridge Ponteiro para o cartucho
 */
void sms_memory_connect_cartridge(sms_memory_t *memory, void *cartridge) {
    if (!memory) {
        return;
    }
    
    memory->cartridge = (sms_cartridge_t*)cartridge;
    SMS_MEM_LOG_DEBUG("Cartucho conectado ao sistema de memória");
}

/**
 * @brief Carrega um BIOS no sistema de memória
 * 
 * @param memory Ponteiro para a instância
 * @param bios_path Caminho para o arquivo do BIOS
 * @return 0 em caso de sucesso, código de erro caso contrário
 */
int sms_memory_load_bios(sms_memory_t *memory, const char *bios_path) {
    if (!memory || !bios_path) {
        return -1;
    }
    
    // Libera o BIOS anterior se existir
    if (memory->bios) {
        free(memory->bios);
        memory->bios = NULL;
        memory->has_bios = 0;
    }
    
    // Abre o arquivo do BIOS
    FILE *file = fopen(bios_path, "rb");
    if (!file) {
        SMS_MEM_LOG_ERROR("Falha ao abrir arquivo do BIOS: %s", bios_path);
        return -1;
    }
    
    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Verifica se o tamanho é válido
    if (size != SMS_BIOS_SIZE) {
        SMS_MEM_LOG_ERROR("Tamanho do BIOS inválido: %ld bytes (esperado: %d bytes)", size, SMS_BIOS_SIZE);
        fclose(file);
        return -1;
    }
    
    // Aloca memória para o BIOS
    memory->bios = (uint8_t*)malloc(SMS_BIOS_SIZE);
    if (!memory->bios) {
        SMS_MEM_LOG_ERROR("Falha ao alocar memória para o BIOS");
        fclose(file);
        return -1;
    }
    
    // Lê o arquivo do BIOS
    size_t read = fread(memory->bios, 1, SMS_BIOS_SIZE, file);
    fclose(file);
    
    if (read != SMS_BIOS_SIZE) {
        SMS_MEM_LOG_ERROR("Falha ao ler arquivo do BIOS: %s", bios_path);
        free(memory->bios);
        memory->bios = NULL;
        return -1;
    }
    
    memory->has_bios = 1;
    SMS_MEM_LOG_INFO("BIOS carregado com sucesso: %s", bios_path);
    
    return 0;
}

/**
 * @brief Lê um byte da memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint8_t sms_memory_read(sms_memory_t *memory, uint16_t address) {
    if (!memory) {
        return 0xFF;
    }
    
    // Leitura da RAM do sistema (0xC000-0xDFFF)
    if (address >= SMS_SYSTEM_RAM_START && address <= SMS_SYSTEM_RAM_END) {
        return memory->ram[address - SMS_SYSTEM_RAM_START];
    }
    
    // Leitura da RAM espelhada (0xE000-0xFFFF)
    if (address >= SMS_MIRROR_RAM_START && address <= SMS_MIRROR_RAM_END) {
        return memory->ram[address - SMS_MIRROR_RAM_START];
    }
    
    // Leitura das portas de I/O (0x00-0x3F)
    if ((address & 0xFF) <= 0x3F) {
        // Portas de memória mapeadas
        switch (address & 0xFF) {
            case 0x00: // Porta de controle do joypad 1
            case 0x01: // Porta de controle do joypad 2
                return sms_input_read_port(memory->input, address & 0xFF);
                
            case 0x7E: // Porta de controle do VDP
            case 0x7F: // Porta de dados do VDP
                return sms_vdp_read(memory->vdp, address & 0xFF);
                
            case 0x06: // Porta do PSG
                return sms_psg_read(memory->psg);
                
            default:
                return 0xFF;
        }
    }
    
    // Leitura do BIOS (0x0000-0x1FFF) se disponível e ativado
    if (address < SMS_BIOS_SIZE && memory->has_bios && (memory->mapper_control & 0x08) == 0) {
        return memory->bios[address];
    }
    
    // Leitura do cartucho (ROM)
    if (memory->cartridge) {
        // Mapeamento de memória baseado nos slots
        if (address < 0x4000) {
            // Slot 0 (0x0000-0x3FFF)
            return sms_cartridge_read(memory->cartridge, (memory->mapper_slots[0] * 0x4000) + (address & 0x3FFF));
        } else if (address < 0x8000) {
            // Slot 1 (0x4000-0x7FFF)
            return sms_cartridge_read(memory->cartridge, (memory->mapper_slots[1] * 0x4000) + (address & 0x3FFF));
        } else if (address < 0xC000) {
            // Slot 2 (0x8000-0xBFFF)
            return sms_cartridge_read(memory->cartridge, (memory->mapper_slots[2] * 0x4000) + (address & 0x3FFF));
        }
    }
    
    // Endereço não mapeado
    SMS_MEM_LOG_TRACE("Leitura de endereço não mapeado: 0x%04X", address);
    return 0xFF;
}

/**
 * @brief Escreve um byte na memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_memory_write(sms_memory_t *memory, uint16_t address, uint8_t value) {
    if (!memory) {
        return;
    }
    
    // Escrita na RAM do sistema (0xC000-0xDFFF)
    if (address >= SMS_SYSTEM_RAM_START && address <= SMS_SYSTEM_RAM_END) {
        memory->ram[address - SMS_SYSTEM_RAM_START] = value;
        return;
    }
    
    // Escrita na RAM espelhada (0xE000-0xFFFF)
    if (address >= SMS_MIRROR_RAM_START && address <= SMS_MIRROR_RAM_END) {
        memory->ram[address - SMS_MIRROR_RAM_START] = value;
        return;
    }
    
    // Escrita nas portas de I/O (0x00-0x3F)
    if ((address & 0xFF) <= 0x3F) {
        // Portas de memória mapeadas
        switch (address & 0xFF) {
            case 0x00: // Porta de controle do joypad 1
            case 0x01: // Porta de controle do joypad 2
                sms_input_write_port(memory->input, address & 0xFF, value);
                return;
                
            case 0x7E: // Porta de controle do VDP
            case 0x7F: // Porta de dados do VDP
                sms_vdp_write(memory->vdp, address & 0xFF, value);
                return;
                
            case 0x06: // Porta do PSG
                sms_psg_write(memory->psg, value);
                return;
                
            default:
                return;
        }
    }
    
    // Escrita no controle do mapper (0xFFFC-0xFFFF)
    if (address >= 0xFFFC && address <= 0xFFFF) {
        int slot = address - 0xFFFC;
        memory->mapper_slots[slot] = value;
        SMS_MEM_LOG_TRACE("Mapper slot %d definido para página %d", slot, value);
        return;
    }
    
    // Escrita no cartucho (RAM)
    if (memory->cartridge && address >= 0x8000 && address < 0xC000) {
        sms_cartridge_write(memory->cartridge, address, value);
        return;
    }
    
    // Endereço não mapeado para escrita
    SMS_MEM_LOG_TRACE("Escrita em endereço não mapeado: 0x%04X = 0x%02X", address, value);
}

/**
 * @brief Lê uma palavra (16 bits) da memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser lido
 * @return Valor lido
 */
uint16_t sms_memory_read_word(sms_memory_t *memory, uint16_t address) {
    uint8_t low = sms_memory_read(memory, address);
    uint8_t high = sms_memory_read(memory, address + 1);
    return (high << 8) | low;
}

/**
 * @brief Escreve uma palavra (16 bits) na memória no endereço especificado
 * 
 * @param memory Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_memory_write_word(sms_memory_t *memory, uint16_t address, uint16_t value) {
    sms_memory_write(memory, address, value & 0xFF);
    sms_memory_write(memory, address + 1, (value >> 8) & 0xFF);
}

/**
 * @brief Registra o sistema de memória no sistema de save state
 * 
 * @param memory Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_memory_register_save_state(sms_memory_t *memory, save_state_t *state) {
    if (!memory || !state) {
        return -1;
    }
    
    // Registra a RAM do sistema
    save_state_register_field(state, "sms_ram", memory->ram, SMS_RAM_SIZE);
    
    // Registra os slots do mapper
    save_state_register_field(state, "sms_mapper_slots", memory->mapper_slots, 4);
    
    // Registra o controle do mapper
    save_state_register_field(state, "sms_mapper_control", &memory->mapper_control, 1);
    
    SMS_MEM_LOG_DEBUG("Sistema de memória registrado no sistema de save state");
    
    return 0;
}
