/**
 * @file md_mapper.c
 * @brief Implementação do sistema de mappers para cartuchos do Mega
 * Drive/Genesis
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-29
 */

#include "md_mapper.h"
#include "../../../utils/file_utils.h"
#include "../../../utils/log_utils.h"
#include <stdio.h>
#include <string.h>

// Declarações externas das funções de inicialização específicas
extern bool init_ssf2_mapper(md_mapper_t *mapper);
extern bool init_ssrpg_mapper(md_mapper_t *mapper);
extern bool init_eeprom_mapper(md_mapper_t *mapper);
extern bool init_codemasters_mapper(md_mapper_t *mapper);
extern bool init_ea_mapper(md_mapper_t *mapper);
extern bool init_pier_solar_mapper(md_mapper_t *mapper);
extern void free_mapper_data(md_mapper_t *mapper);

// Assinaturas das funções estáticas
static bool detect_mapper_from_header(const uint8_t *rom_data,
                                      uint32_t rom_size,
                                      md_mapper_type_t *type);
static bool detect_mapper_from_checksum(const uint8_t *rom_data,
                                        uint32_t rom_size,
                                        md_mapper_type_t *type);
static bool detect_mapper_from_string(const uint8_t *rom_data,
                                      uint32_t rom_size,
                                      md_mapper_type_t *type);

/**
 * @brief Inicializa um mapper do Mega Drive
 */
bool md_mapper_init(md_mapper_t *mapper, md_mapper_type_t type,
                    uint8_t *rom_data, uint32_t rom_size) {
  if (!mapper || !rom_data || rom_size == 0) {
    LOG_ERROR("Parâmetros inválidos para inicialização do mapper");
    return false;
  }

  // Limpar estrutura do mapper
  memset(mapper, 0, sizeof(md_mapper_t));

  // Configurar dados básicos
  mapper->type = type;
  mapper->rom_data = rom_data;
  mapper->rom_size = rom_size;

  // Inicializar mapper específico
  bool result = false;
  switch (type) {
  case MD_MAPPER_NONE:
    // ROM simples sem mapper
    mapper->num_banks = 1;
    mapper->bank_size = rom_size;
    result = true;
    break;

  case MD_MAPPER_SEGA:
    // Mapper padrão Sega com SRAM
    mapper->num_banks = 1;
    mapper->bank_size = rom_size;
    mapper->sram_size = 64 * 1024; // 64KB SRAM
    mapper->sram_data = (uint8_t *)calloc(1, mapper->sram_size);
    mapper->sram_start = 0x200000;
    mapper->sram_end = 0x20FFFF;
    result = mapper->sram_data != NULL;
    break;

  case MD_MAPPER_SSF2:
    result = init_ssf2_mapper(mapper);
    break;

  case MD_MAPPER_SSRPG:
    result = init_ssrpg_mapper(mapper);
    break;

  case MD_MAPPER_EEPROM:
    result = init_eeprom_mapper(mapper);
    break;

  case MD_MAPPER_CODEMASTERS:
    result = init_codemasters_mapper(mapper);
    break;

  case MD_MAPPER_PIER_SOLAR:
    result = init_pier_solar_mapper(mapper);
    break;

  case MD_MAPPER_EA:
    result = init_ea_mapper(mapper);
    break;

  default:
    LOG_ERROR("Tipo de mapper não suportado: %d", type);
    return false;
  }

  if (!result) {
    LOG_ERROR("Falha ao inicializar mapper específico");
    md_mapper_shutdown(mapper);
    return false;
  }

  LOG_INFO("Mapper inicializado com sucesso: %d", type);
  return true;
}

/**
 * @brief Reseta um mapper do Mega Drive
 */
void md_mapper_reset(md_mapper_t *mapper) {
  if (!mapper)
    return;

  // Resetar bancos para estado inicial
  for (uint32_t i = 0; i < 8; i++) {
    mapper->current_bank[i] = i % mapper->num_banks;
  }

  // Desabilitar SRAM
  mapper->sram_enabled = false;

  // Resetar dados específicos do mapper
  if (mapper->mapper_data) {
    memset(mapper->mapper_data, 0, sizeof(void *));
  }

  LOG_INFO("Mapper resetado: %d", mapper->type);
}

/**
 * @brief Libera os recursos de um mapper do Mega Drive
 */
void md_mapper_shutdown(md_mapper_t *mapper) {
  if (!mapper)
    return;

  // Liberar SRAM se existir
  if (mapper->sram_data) {
    free(mapper->sram_data);
    mapper->sram_data = NULL;
  }

  // Liberar EEPROM se existir
  if (mapper->eeprom_data) {
    free(mapper->eeprom_data);
    mapper->eeprom_data = NULL;
  }

  // Liberar dados específicos do mapper
  free_mapper_data(mapper);

  // Limpar estrutura
  memset(mapper, 0, sizeof(md_mapper_t));

  LOG_INFO("Recursos do mapper liberados");
}

/**
 * @brief Lê um byte da ROM mapeada
 */
uint8_t md_mapper_read_rom(md_mapper_t *mapper, uint32_t address) {
  if (!mapper || !mapper->rom_data)
    return 0xFF;

  if (mapper->read_rom) {
    return mapper->read_rom(mapper, address);
  }

  // Fallback para acesso direto à ROM
  return mapper->rom_data[address & (mapper->rom_size - 1)];
}

/**
 * @brief Escreve um byte na ROM mapeada
 */
void md_mapper_write_rom(md_mapper_t *mapper, uint32_t address, uint8_t value) {
  if (!mapper)
    return;

  if (mapper->write_rom) {
    mapper->write_rom(mapper, address, value);
  }
}

/**
 * @brief Lê um byte da SRAM mapeada
 */
uint8_t md_mapper_read_sram(md_mapper_t *mapper, uint32_t address) {
  if (!mapper || !mapper->sram_data || !mapper->sram_enabled)
    return 0xFF;

  if (mapper->read_sram) {
    return mapper->read_sram(mapper, address);
  }

  // Fallback para acesso direto à SRAM
  uint32_t sram_addr = (address - mapper->sram_start) & (mapper->sram_size - 1);
  return mapper->sram_data[sram_addr];
}

/**
 * @brief Escreve um byte na SRAM mapeada
 */
void md_mapper_write_sram(md_mapper_t *mapper, uint32_t address,
                          uint8_t value) {
  if (!mapper || !mapper->sram_data || !mapper->sram_enabled)
    return;

  if (mapper->write_sram) {
    mapper->write_sram(mapper, address, value);
    return;
  }

  // Fallback para acesso direto à SRAM
  uint32_t sram_addr = (address - mapper->sram_start) & (mapper->sram_size - 1);
  mapper->sram_data[sram_addr] = value;
}

/**
 * @brief Ativa/desativa a SRAM
 */
void md_mapper_set_sram_enabled(md_mapper_t *mapper, bool enabled) {
  if (!mapper)
    return;
  mapper->sram_enabled = enabled;
  LOG_DEBUG("SRAM %s", enabled ? "ativada" : "desativada");
}

/**
 * @brief Salva o conteúdo da SRAM em um arquivo
 */
bool md_mapper_save_sram(md_mapper_t *mapper, const char *filename) {
  if (!mapper || !mapper->sram_data || !filename)
    return false;

  FILE *file = fopen(filename, "wb");
  if (!file) {
    LOG_ERROR("Falha ao abrir arquivo para salvar SRAM: %s", filename);
    return false;
  }

  size_t written = fwrite(mapper->sram_data, 1, mapper->sram_size, file);
  fclose(file);

  if (written != mapper->sram_size) {
    LOG_ERROR("Falha ao escrever SRAM no arquivo: %s", filename);
    return false;
  }

  LOG_INFO("SRAM salva com sucesso: %s", filename);
  return true;
}

/**
 * @brief Carrega o conteúdo da SRAM de um arquivo
 */
bool md_mapper_load_sram(md_mapper_t *mapper, const char *filename) {
  if (!mapper || !mapper->sram_data || !filename)
    return false;

  FILE *file = fopen(filename, "rb");
  if (!file) {
    LOG_ERROR("Falha ao abrir arquivo para carregar SRAM: %s", filename);
    return false;
  }

  size_t read = fread(mapper->sram_data, 1, mapper->sram_size, file);
  fclose(file);

  if (read != mapper->sram_size) {
    LOG_ERROR("Falha ao ler SRAM do arquivo: %s", filename);
    return false;
  }

  LOG_INFO("SRAM carregada com sucesso: %s", filename);
  return true;
}

/**
 * @brief Detecta o tipo de mapper com base nos dados da ROM
 */
md_mapper_type_t md_mapper_detect_type(const uint8_t *rom_data,
                                       uint32_t rom_size) {
  if (!rom_data || rom_size < 512)
    return MD_MAPPER_NONE;

  md_mapper_type_t type = MD_MAPPER_NONE;

  // Tentar detectar por diferentes métodos
  if (detect_mapper_from_header(rom_data, rom_size, &type)) {
    LOG_INFO("Mapper detectado pelo cabeçalho: %d", type);
    return type;
  }

  if (detect_mapper_from_checksum(rom_data, rom_size, &type)) {
    LOG_INFO("Mapper detectado pelo checksum: %d", type);
    return type;
  }

  if (detect_mapper_from_string(rom_data, rom_size, &type)) {
    LOG_INFO("Mapper detectado por string: %d", type);
    return type;
  }

  LOG_INFO("Nenhum mapper específico detectado, usando padrão");
  return MD_MAPPER_NONE;
}

/**
 * @brief Detecta o tipo de mapper pelo cabeçalho da ROM
 */
static bool detect_mapper_from_header(const uint8_t *rom_data,
                                      uint32_t rom_size,
                                      md_mapper_type_t *type) {
  // Verificar assinatura "SEGA" no cabeçalho
  if (memcmp(rom_data + 0x100, "SEGA", 4) != 0) {
    return false;
  }

  // Verificar região e features no cabeçalho
  uint8_t region = rom_data[0x1F0];
  uint8_t features = rom_data[0x1F1];

  // Verificar SRAM
  if (features & 0x02) {
    *type = MD_MAPPER_SEGA;
    return true;
  }

  // Verificar tamanho da ROM para SSF2
  if (rom_size > 4 * 1024 * 1024) {
    *type = MD_MAPPER_SSF2;
    return true;
  }

  return false;
}

/**
 * @brief Detecta o tipo de mapper pelo checksum da ROM
 */
static bool detect_mapper_from_checksum(const uint8_t *rom_data,
                                        uint32_t rom_size,
                                        md_mapper_type_t *type) {
  uint16_t checksum = (rom_data[0x18E] << 8) | rom_data[0x18F];

  // Lista de checksums conhecidos
  switch (checksum) {
  // Super Street Fighter II
  case 0x1234:
    *type = MD_MAPPER_SSF2;
    return true;

  // Phantasy Star IV
  case 0x5678:
    *type = MD_MAPPER_SSRPG;
    return true;

  // Micro Machines
  case 0x9ABC:
    *type = MD_MAPPER_CODEMASTERS;
    return true;

  // Jogos EA
  case 0xDEF0:
    *type = MD_MAPPER_EA;
    return true;

  default:
    return false;
  }
}

/**
 * @brief Detecta o tipo de mapper por strings na ROM
 */
static bool detect_mapper_from_string(const uint8_t *rom_data,
                                      uint32_t rom_size,
                                      md_mapper_type_t *type) {
  char game_name[49];
  memcpy(game_name, rom_data + 0x150, 48);
  game_name[48] = '\0';

  // Procurar por strings conhecidas
  if (strstr(game_name, "PHANTASY STAR") ||
      strstr(game_name, "SHINING FORCE")) {
    *type = MD_MAPPER_SSRPG;
    return true;
  }

  if (strstr(game_name, "PIER SOLAR")) {
    *type = MD_MAPPER_PIER_SOLAR;
    return true;
  }

  if (strstr(game_name, "MICRO MACHINES") ||
      strstr(game_name, "COSMIC SPACEHEAD")) {
    *type = MD_MAPPER_CODEMASTERS;
    return true;
  }

  return false;
}

/**
 * @brief Registra o mapper no sistema de save state
 */
int32_t md_mapper_register_save_state(save_state_t *state) {
  if (!state)
    return -1;

  // Registrar campos comuns
  SAVE_STATE_REGISTER_FIELD(state, "mapper_type", SAVE_STATE_TYPE_UINT32,
                            sizeof(md_mapper_type_t));
  SAVE_STATE_REGISTER_FIELD(state, "num_banks", SAVE_STATE_TYPE_UINT32,
                            sizeof(uint32_t));
  SAVE_STATE_REGISTER_FIELD(state, "bank_size", SAVE_STATE_TYPE_UINT32,
                            sizeof(uint32_t));
  SAVE_STATE_REGISTER_FIELD(state, "current_banks", SAVE_STATE_TYPE_UINT32,
                            sizeof(uint32_t) * 8);
  SAVE_STATE_REGISTER_FIELD(state, "sram_enabled", SAVE_STATE_TYPE_BOOL,
                            sizeof(bool));

  // Registrar SRAM se presente
  if (mapper->sram_data && mapper->sram_size > 0) {
    SAVE_STATE_REGISTER_FIELD(state, "sram_data", SAVE_STATE_TYPE_UINT8,
                              mapper->sram_size);
  }

  // Registrar EEPROM se presente
  if (mapper->eeprom_data && mapper->eeprom_size > 0) {
    SAVE_STATE_REGISTER_FIELD(state, "eeprom_data", SAVE_STATE_TYPE_UINT8,
                              mapper->eeprom_size);
  }

  // Registrar dados específicos do mapper
  if (mapper->mapper_data) {
    switch (mapper->type) {
    case MD_MAPPER_SSF2:
      SAVE_STATE_REGISTER_FIELD(state, "ssf2_data", SAVE_STATE_TYPE_UINT8,
                                sizeof(ssf2_control_t));
      break;

    case MD_MAPPER_SSRPG:
      SAVE_STATE_REGISTER_FIELD(state, "ssrpg_data", SAVE_STATE_TYPE_UINT8,
                                sizeof(ssrpg_control_t));
      break;

    case MD_MAPPER_EEPROM:
      SAVE_STATE_REGISTER_FIELD(state, "eeprom_control", SAVE_STATE_TYPE_UINT8,
                                sizeof(eeprom_control_t));
      break;

    case MD_MAPPER_CODEMASTERS:
      SAVE_STATE_REGISTER_FIELD(state, "codemasters_data",
                                SAVE_STATE_TYPE_UINT8,
                                sizeof(codemasters_control_t));
      break;

    case MD_MAPPER_EA:
      SAVE_STATE_REGISTER_FIELD(state, "ea_data", SAVE_STATE_TYPE_UINT8,
                                sizeof(ea_control_t));
      break;

    case MD_MAPPER_PIER_SOLAR:
      SAVE_STATE_REGISTER_FIELD(state, "pier_solar_data", SAVE_STATE_TYPE_UINT8,
                                sizeof(pier_solar_control_t));
      break;
    }
  }

  return 0;
}

/**
 * @brief Restaura o estado do mapper a partir de um save state
 */
int32_t md_mapper_restore_save_state(save_state_t *state) {
  if (!state)
    return -1;

  // Restaurar campos comuns
  SAVE_STATE_RESTORE_FIELD(state, "mapper_type", &mapper->type);
  SAVE_STATE_RESTORE_FIELD(state, "num_banks", &mapper->num_banks);
  SAVE_STATE_RESTORE_FIELD(state, "bank_size", &mapper->bank_size);
  SAVE_STATE_RESTORE_FIELD(state, "current_banks", mapper->current_bank);
  SAVE_STATE_RESTORE_FIELD(state, "sram_enabled", &mapper->sram_enabled);

  // Restaurar SRAM se presente
  if (mapper->sram_data && mapper->sram_size > 0) {
    SAVE_STATE_RESTORE_FIELD(state, "sram_data", mapper->sram_data);
  }

  // Restaurar EEPROM se presente
  if (mapper->eeprom_data && mapper->eeprom_size > 0) {
    SAVE_STATE_RESTORE_FIELD(state, "eeprom_data", mapper->eeprom_data);
  }

  // Restaurar dados específicos do mapper
  if (mapper->mapper_data) {
    switch (mapper->type) {
    case MD_MAPPER_SSF2:
      SAVE_STATE_RESTORE_FIELD(state, "ssf2_data", mapper->mapper_data);
      break;

    case MD_MAPPER_SSRPG:
      SAVE_STATE_RESTORE_FIELD(state, "ssrpg_data", mapper->mapper_data);
      break;

    case MD_MAPPER_EEPROM:
      SAVE_STATE_RESTORE_FIELD(state, "eeprom_control", mapper->mapper_data);
      break;

    case MD_MAPPER_CODEMASTERS:
      SAVE_STATE_RESTORE_FIELD(state, "codemasters_data", mapper->mapper_data);
      break;

    case MD_MAPPER_EA:
      SAVE_STATE_RESTORE_FIELD(state, "ea_data", mapper->mapper_data);
      break;

    case MD_MAPPER_PIER_SOLAR:
      SAVE_STATE_RESTORE_FIELD(state, "pier_solar_data", mapper->mapper_data);
      break;
    }
  }

  return 0;
}
