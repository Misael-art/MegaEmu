/**
 * @file md_mapper_impl.c
 * @brief Implementação detalhada dos mappers específicos do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-29
 */

#include "md_mapper.h"
#include "../../../utils/log_utils.h"
#include "../../../utils/bit_utils.h"

// Constantes para mappers específicos
#define SSF2_BANK_SIZE          (512 * 1024)  // 512KB por banco
#define CODEMASTERS_BANK_SIZE   (16 * 1024)   // 16KB por banco
#define EA_BANK_SIZE           (16 * 1024)    // 16KB por banco
#define PIER_SOLAR_BANK_SIZE   (512 * 1024)   // 512KB por banco

// Registros de controle para EEPROM
#define EEPROM_COMMAND_READ    0x03
#define EEPROM_COMMAND_WRITE   0x02
#define EEPROM_COMMAND_WREN    0x06
#define EEPROM_COMMAND_WRDI    0x04
#define EEPROM_COMMAND_RDSR    0x05
#define EEPROM_COMMAND_WRSR    0x01

// Estados da EEPROM
typedef enum {
    EEPROM_STATE_IDLE,
    EEPROM_STATE_COMMAND,
    EEPROM_STATE_ADDRESS,
    EEPROM_STATE_DATA,
    EEPROM_STATE_WRITE_PENDING
} eeprom_state_t;

// Estrutura para controle da EEPROM
typedef struct {
    uint8_t command;
    uint16_t address;
    uint8_t data_buffer[256];
    uint8_t buffer_pos;
    uint8_t status_register;
    eeprom_state_t state;
    bool write_enabled;
} eeprom_control_t;

// Estrutura para controle do mapper SSF2
typedef struct {
    uint8_t bank_registers[8];
    uint32_t rom_mask;
} ssf2_control_t;

// Estrutura para controle do mapper SSRPG
typedef struct {
    uint8_t control_register;
    uint32_t sram_mask;
    bool sram_write_enabled;
} ssrpg_control_t;

// Estrutura para controle do mapper Codemasters
typedef struct {
    uint8_t bank_registers[8];
    uint32_t rom_mask;
    uint8_t control_register;
} codemasters_control_t;

// Estrutura para controle do mapper EA
typedef struct {
    uint8_t bank_registers[8];
    uint32_t rom_mask;
    uint8_t control_register;
} ea_control_t;

// Estrutura para controle do mapper Pier Solar
typedef struct {
    uint8_t bank_registers[8];
    uint32_t rom_mask;
    uint8_t control_register;
    uint8_t expansion_port;
    bool rtc_enabled;
} pier_solar_control_t;

// Funções auxiliares para manipulação de bancos
static inline uint32_t calculate_rom_address(uint32_t bank, uint32_t offset, uint32_t bank_size, uint32_t rom_mask) {
    return ((bank * bank_size) + offset) & rom_mask;
}

// Implementação do mapper SSF2
uint8_t ssf2_read_rom(md_mapper_t *mapper, uint32_t address) {
    ssf2_control_t *control = (ssf2_control_t *)mapper->mapper_data;
    uint32_t bank_index = (address >> 19) & 0x07;  // Divide por 512KB
    uint32_t offset = address & (SSF2_BANK_SIZE - 1);
    uint32_t rom_address = calculate_rom_address(
        control->bank_registers[bank_index],
        offset,
        SSF2_BANK_SIZE,
        control->rom_mask
    );
    return mapper->rom_data[rom_address];
}

void ssf2_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value) {
    ssf2_control_t *control = (ssf2_control_t *)mapper->mapper_data;
    if (address >= 0xA13000 && address <= 0xA130FF) {
        uint32_t bank_index = (address >> 1) & 0x07;
        control->bank_registers[bank_index] = value & (mapper->num_banks - 1);
        LOG_DEBUG("SSF2: Bank %d selecionado: %02X", bank_index, value);
    }
}

// Implementação do mapper SSRPG
uint8_t ssrpg_read_rom(md_mapper_t *mapper, uint32_t address) {
    ssrpg_control_t *control = (ssrpg_control_t *)mapper->mapper_data;

    // Verificar acesso à SRAM
    if (address >= mapper->sram_start && address <= mapper->sram_end && mapper->sram_enabled) {
        uint32_t sram_addr = (address - mapper->sram_start) & control->sram_mask;
        return mapper->sram_data[sram_addr];
    }

    return mapper->rom_data[address & (mapper->rom_size - 1)];
}

void ssrpg_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value) {
    ssrpg_control_t *control = (ssrpg_control_t *)mapper->mapper_data;

    // Registros de controle
    if (address == 0xA130F1) {
        control->control_register = value;
        control->sram_write_enabled = (value & 0x02) != 0;
        mapper->sram_enabled = (value & 0x01) != 0;
        LOG_DEBUG("SSRPG: Control Register: %02X", value);
    }
    // Escrita na SRAM
    else if (address >= mapper->sram_start && address <= mapper->sram_end &&
             mapper->sram_enabled && control->sram_write_enabled) {
        uint32_t sram_addr = (address - mapper->sram_start) & control->sram_mask;
        mapper->sram_data[sram_addr] = value;
    }
}

// Implementação do mapper EEPROM
uint8_t eeprom_read_rom(md_mapper_t *mapper, uint32_t address) {
    eeprom_control_t *control = (eeprom_control_t *)mapper->mapper_data;

    // Acesso à EEPROM
    if (address >= 0x200000 && address <= 0x201FFF) {
        switch (control->state) {
            case EEPROM_STATE_DATA:
                if (control->command == EEPROM_COMMAND_READ) {
                    uint8_t data = control->data_buffer[control->buffer_pos++];
                    if (control->buffer_pos >= 256) {
                        control->state = EEPROM_STATE_IDLE;
                    }
                    return data;
                }
                break;
            case EEPROM_STATE_IDLE:
                return control->status_register;
        }
        return 0xFF;
    }

    return mapper->rom_data[address & (mapper->rom_size - 1)];
}

void eeprom_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value) {
    eeprom_control_t *control = (eeprom_control_t *)mapper->mapper_data;

    if (address >= 0x200000 && address <= 0x201FFF) {
        switch (control->state) {
            case EEPROM_STATE_IDLE:
                control->command = value;
                control->state = EEPROM_STATE_COMMAND;
                control->buffer_pos = 0;

                switch (value) {
                    case EEPROM_COMMAND_WREN:
                        control->write_enabled = true;
                        control->state = EEPROM_STATE_IDLE;
                        break;
                    case EEPROM_COMMAND_WRDI:
                        control->write_enabled = false;
                        control->state = EEPROM_STATE_IDLE;
                        break;
                    case EEPROM_COMMAND_RDSR:
                        control->data_buffer[0] = control->status_register;
                        control->state = EEPROM_STATE_DATA;
                        break;
                }
                break;

            case EEPROM_STATE_COMMAND:
                if (control->command == EEPROM_COMMAND_READ ||
                    control->command == EEPROM_COMMAND_WRITE) {
                    control->address = (value << 8);
                    control->state = EEPROM_STATE_ADDRESS;
                }
                break;

            case EEPROM_STATE_ADDRESS:
                control->address |= value;
                if (control->command == EEPROM_COMMAND_READ) {
                    // Copiar dados da EEPROM para o buffer
                    memcpy(control->data_buffer,
                           &mapper->eeprom_data[control->address],
                           256);
                    control->state = EEPROM_STATE_DATA;
                } else {
                    control->state = EEPROM_STATE_DATA;
                }
                break;

            case EEPROM_STATE_DATA:
                if (control->command == EEPROM_COMMAND_WRITE && control->write_enabled) {
                    mapper->eeprom_data[control->address + control->buffer_pos] = value;
                    control->buffer_pos++;
                    if (control->buffer_pos >= 256) {
                        control->state = EEPROM_STATE_WRITE_PENDING;
                        // Simular tempo de escrita
                        control->status_register |= 0x01;
                    }
                }
                break;
        }
    }
}

// Implementação do mapper Codemasters
uint8_t codemasters_read_rom(md_mapper_t *mapper, uint32_t address) {
    codemasters_control_t *control = (codemasters_control_t *)mapper->mapper_data;
    uint32_t bank_index = (address >> 14) & 0x07;  // Divide por 16KB
    uint32_t offset = address & (CODEMASTERS_BANK_SIZE - 1);
    uint32_t rom_address = calculate_rom_address(
        control->bank_registers[bank_index],
        offset,
        CODEMASTERS_BANK_SIZE,
        control->rom_mask
    );
    return mapper->rom_data[rom_address];
}

void codemasters_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value) {
    codemasters_control_t *control = (codemasters_control_t *)mapper->mapper_data;

    // Registros de controle específicos da Codemasters
    if (address >= 0x8000 && address <= 0xBFFF) {
        uint32_t bank_index = (address >> 14) & 0x03;
        control->bank_registers[bank_index] = value & (mapper->num_banks - 1);
        LOG_DEBUG("Codemasters: Bank %d selecionado: %02X", bank_index, value);
    }
}

// Implementação do mapper EA
uint8_t ea_read_rom(md_mapper_t *mapper, uint32_t address) {
    ea_control_t *control = (ea_control_t *)mapper->mapper_data;
    uint32_t bank_index = (address >> 14) & 0x07;  // Divide por 16KB
    uint32_t offset = address & (EA_BANK_SIZE - 1);
    uint32_t rom_address = calculate_rom_address(
        control->bank_registers[bank_index],
        offset,
        EA_BANK_SIZE,
        control->rom_mask
    );
    return mapper->rom_data[rom_address];
}

void ea_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value) {
    ea_control_t *control = (ea_control_t *)mapper->mapper_data;

    // Registros de controle da EA
    if (address >= 0xA13000 && address <= 0xA13FFF) {
        uint32_t bank_index = (address & 0x0F) % 8;
        control->bank_registers[bank_index] = value & (mapper->num_banks - 1);
        LOG_DEBUG("EA: Bank %d selecionado: %02X", bank_index, value);
    }
}

// Implementação do mapper Pier Solar
uint8_t pier_solar_read_rom(md_mapper_t *mapper, uint32_t address) {
    pier_solar_control_t *control = (pier_solar_control_t *)mapper->mapper_data;

    // Verificar acesso à ROM
    if (address >= 0x000000 && address <= 0x3FFFFF) {
        uint32_t bank_index = (address >> 19) & 0x07;  // Divide por 512KB
        uint32_t offset = address & (PIER_SOLAR_BANK_SIZE - 1);
        uint32_t rom_address = calculate_rom_address(
            control->bank_registers[bank_index],
            offset,
            PIER_SOLAR_BANK_SIZE,
            control->rom_mask
        );
        return mapper->rom_data[rom_address];
    }

    // Verificar acesso à SRAM expandida
    if (address >= 0x200000 && address <= 0x2FFFFF && mapper->sram_enabled) {
        uint32_t sram_addr = address - 0x200000;
        return mapper->sram_data[sram_addr];
    }

    // Verificar acesso à porta de expansão
    if (address == 0xA130F1) {
        return control->expansion_port;
    }

    return 0xFF;
}

void pier_solar_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value) {
    pier_solar_control_t *control = (pier_solar_control_t *)mapper->mapper_data;

    // Registros de controle do Pier Solar
    if (address >= 0xA13000 && address <= 0xA130FF) {
        uint32_t reg = address & 0xFF;

        switch (reg) {
            case 0x00: // Controle de banco ROM
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
                control->bank_registers[reg] = value & (mapper->num_banks - 1);
                LOG_DEBUG("Pier Solar: Bank %d selecionado: %02X", reg, value);
                break;

            case 0xF0: // Controle geral
                control->control_register = value;
                mapper->sram_enabled = (value & 0x01) != 0;
                control->rtc_enabled = (value & 0x02) != 0;
                LOG_DEBUG("Pier Solar: Control Register: %02X", value);
                break;

            case 0xF1: // Porta de expansão
                control->expansion_port = value;
                LOG_DEBUG("Pier Solar: Expansion Port: %02X", value);
                break;
        }
    }

    // Escrita na SRAM expandida
    if (address >= 0x200000 && address <= 0x2FFFFF && mapper->sram_enabled) {
        uint32_t sram_addr = address - 0x200000;
        mapper->sram_data[sram_addr] = value;
    }
}

// Funções de inicialização específicas para cada mapper
bool init_ssf2_mapper(md_mapper_t *mapper) {
    ssf2_control_t *control = (ssf2_control_t *)malloc(sizeof(ssf2_control_t));
    if (!control) return false;

    memset(control, 0, sizeof(ssf2_control_t));
    control->rom_mask = mapper->rom_size - 1;
    mapper->mapper_data = control;

    mapper->read_rom = ssf2_read_rom;
    mapper->write_rom = ssf2_write_rom;
    return true;
}

bool init_ssrpg_mapper(md_mapper_t *mapper) {
    ssrpg_control_t *control = (ssrpg_control_t *)malloc(sizeof(ssrpg_control_t));
    if (!control) return false;

    memset(control, 0, sizeof(ssrpg_control_t));
    control->sram_mask = mapper->sram_size - 1;
    mapper->mapper_data = control;

    mapper->read_rom = ssrpg_read_rom;
    mapper->write_rom = ssrpg_write_rom;
    return true;
}

bool init_eeprom_mapper(md_mapper_t *mapper) {
    eeprom_control_t *control = (eeprom_control_t *)malloc(sizeof(eeprom_control_t));
    if (!control) return false;

    memset(control, 0, sizeof(eeprom_control_t));
    mapper->mapper_data = control;

    mapper->read_rom = eeprom_read_rom;
    mapper->write_rom = eeprom_write_rom;
    return true;
}

bool init_codemasters_mapper(md_mapper_t *mapper) {
    codemasters_control_t *control = (codemasters_control_t *)malloc(sizeof(codemasters_control_t));
    if (!control) return false;

    memset(control, 0, sizeof(codemasters_control_t));
    control->rom_mask = mapper->rom_size - 1;
    mapper->mapper_data = control;

    mapper->read_rom = codemasters_read_rom;
    mapper->write_rom = codemasters_write_rom;
    return true;
}

bool init_ea_mapper(md_mapper_t *mapper) {
    ea_control_t *control = (ea_control_t *)malloc(sizeof(ea_control_t));
    if (!control) return false;

    memset(control, 0, sizeof(ea_control_t));
    control->rom_mask = mapper->rom_size - 1;
    mapper->mapper_data = control;

    mapper->read_rom = ea_read_rom;
    mapper->write_rom = ea_write_rom;
    return true;
}

bool init_pier_solar_mapper(md_mapper_t *mapper) {
    pier_solar_control_t *control = (pier_solar_control_t *)malloc(sizeof(pier_solar_control_t));
    if (!control) return false;

    memset(control, 0, sizeof(pier_solar_control_t));
    control->rom_mask = mapper->rom_size - 1;
    mapper->mapper_data = control;

    mapper->read_rom = pier_solar_read_rom;
    mapper->write_rom = pier_solar_write_rom;
    return true;
}

// Função para liberar recursos específicos dos mappers
void free_mapper_data(md_mapper_t *mapper) {
    if (mapper->mapper_data) {
        free(mapper->mapper_data);
        mapper->mapper_data = NULL;
    }
}
