/**
 * @file sms_cartridge.c
 * @brief Implementação do sistema de cartucho do Master System
 */

#include "sms_cartridge.h"
#include "../../../utils/crc32.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o cartucho do Master System
#define EMU_LOG_CAT_CARTRIDGE EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para o cartucho do Master System
#define SMS_CART_LOG_ERROR(...)                                                \
  EMU_LOG_ERROR(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_DEBUG(...)                                                \
  EMU_LOG_DEBUG(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)
#define SMS_CART_LOG_TRACE(...)                                                \
  EMU_LOG_TRACE(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)

/**
 * @brief Tipos de mapper suportados
 */
typedef enum {
  SMS_MAPPER_NONE = 0,    // ROM simples sem mapper
  SMS_MAPPER_SEGA,        // Mapper padrão da Sega
  SMS_MAPPER_CODEMASTERS, // Mapper da Codemasters
  SMS_MAPPER_KOREAN,      // Mapper coreano
  SMS_MAPPER_MSX,         // Mapper compatível com MSX
  SMS_MAPPER_NEMESIS,     // Mapper usado em Nemesis (8KB páginas)
  SMS_MAPPER_JANGGUN,     // Mapper especial para The Jang Gun
  SMS_MAPPER_MULTI_GAME   // Mapper para cartuchos multi-jogos
} sms_mapper_type_t;

/**
 * @brief Estrutura interna do cartucho
 */
struct sms_cartridge_t {
  uint8_t *rom_data;             // Dados da ROM
  uint32_t rom_size;             // Tamanho da ROM em bytes
  uint32_t rom_mask;             // Máscara para acesso à ROM
  uint8_t *ram_data;             // Dados da RAM (se presente)
  uint32_t ram_size;             // Tamanho da RAM em bytes
  uint8_t has_battery;           // Flag indicando se tem bateria para salvar
  sms_mapper_type_t mapper_type; // Tipo de mapper
  uint8_t mapper_regs[4];        // Registradores do mapper
  sms_rom_info_t rom_info;       // Informações sobre a ROM

  // Flags e recursos específicos de mappers
  uint8_t sram_chip_enabled;       // Flag de ativação de chip SRAM (1=ativado)
  uint8_t sram_chip_write_enabled; // Flag de escrita em SRAM (1=permitido)
  uint32_t sram_mask;              // Máscara para acesso à SRAM
  uint8_t ram_control_reg;         // Registrador de controle de RAM
  uint16_t multi_game_menu_page;   // Página do menu para multi-jogos
};

/**
 * @brief Lista de ROMs conhecidas para detectar mappers específicos
 */
static const struct {
  const char *partial_name;      // Parte do nome para corresponder
  uint32_t crc32;                // CRC32 da ROM (0 = ignorar)
  sms_mapper_type_t mapper_type; // Tipo de mapper a usar
  uint32_t ram_size;             // Tamanho da RAM (0 = padrão)
  uint8_t has_battery;           // Flag de bateria
} known_roms[] = {
    // Jogos da Codemasters
    {"Cosmic Spacehd", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Cosmic Spacehead", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Dinobasher", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Excellent Dizzy", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Fantastic Dizzy", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Micro Machines", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"S.S. Tennis", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Sega Chess", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Somari", 0, SMS_MAPPER_CODEMASTERS, 0, 0},
    {"Super Shinobi", 0, SMS_MAPPER_CODEMASTERS, 0, 0},

    // Jogos com mapper coreano
    {"Dodgeball King", 0, SMS_MAPPER_KOREAN, 0, 0},
    {"Jang Gun", 0, SMS_MAPPER_JANGGUN, 0, 0},
    {"Sangokushi", 0, SMS_MAPPER_KOREAN, 0, 0},
    {"Super Boy 3", 0, SMS_MAPPER_KOREAN, 0, 0},

    // Jogos com mapper MSX
    {"Bomber Raid", 0, SMS_MAPPER_MSX, 0, 0},
    {"Penguin Land", 0, SMS_MAPPER_MSX, 0, 0},

    // Jogos com mapper Nemesis
    {"Nemesis", 0, SMS_MAPPER_NEMESIS, 0, 0},

    // Cartuchos multi-jogos
    {"Mega Games", 0, SMS_MAPPER_MULTI_GAME, 0, 0},
    {"Multi Games", 0, SMS_MAPPER_MULTI_GAME, 0, 0},

    // Jogos com SRAM específica
    {"Phantasy Star", 0, SMS_MAPPER_SEGA, 8 * 1024, 1},
    {"Ys", 0, SMS_MAPPER_SEGA, 8 * 1024, 1},
    {"Golvellius", 0, SMS_MAPPER_SEGA, 8 * 1024, 1},

    // Fim da lista
    {NULL, 0, SMS_MAPPER_NONE, 0, 0}};

/**
 * @brief Detecta o tipo de mapper com base no cabeçalho, tamanho e conteúdo da
 * ROM
 *
 * @param cartridge Ponteiro para a instância
 */
static void sms_cartridge_detect_mapper(sms_cartridge_t *cartridge) {
  if (!cartridge || !cartridge->rom_data) {
    return;
  }

  // Por padrão, assume mapper Sega
  cartridge->mapper_type = SMS_MAPPER_SEGA;
  cartridge->ram_size = 0;

  // Primeiramente, procura na lista de ROMs conhecidas
  for (int i = 0; known_roms[i].partial_name != NULL; i++) {
    bool match = false;

    // Verificação pelo nome
    if (known_roms[i].partial_name && cartridge->rom_info.title[0] &&
        strstr(cartridge->rom_info.title, known_roms[i].partial_name)) {
      match = true;
    }

    // Verificação pelo CRC32 (se fornecido)
    if (known_roms[i].crc32 != 0 &&
        cartridge->rom_info.checksum == known_roms[i].crc32) {
      match = true;
    }

    // Se houve correspondência, define o tipo de mapper
    if (match) {
      cartridge->mapper_type = known_roms[i].mapper_type;

      // Configura RAM e bateria, se especificados
      if (known_roms[i].ram_size > 0) {
        cartridge->ram_size = known_roms[i].ram_size;
      }

      if (known_roms[i].has_battery) {
        cartridge->has_battery = 1;
        cartridge->rom_info.has_battery = 1;
      }

      SMS_CART_LOG_INFO(
          "ROM conhecida detectada: %s, mapper=%d, ram=%d, bateria=%d",
          cartridge->rom_info.title, cartridge->mapper_type,
          cartridge->ram_size, cartridge->has_battery);
      return;
    }
  }

  // Verificação baseada em assinaturas
  if (cartridge->rom_size >= 0x8000) {
    // Codemasters: verifica assinatura no cabeçalho
    if (cartridge->rom_data[0x7FF0] == 0x55 &&
        cartridge->rom_data[0x7FF1] == 0xAA) {
      cartridge->mapper_type = SMS_MAPPER_CODEMASTERS;
      SMS_CART_LOG_INFO("Mapper Codemasters detectado pela assinatura");
      return;
    }

    // Korean: verificação simplificada baseada em patterns conhecidos
    if (cartridge->rom_size > 0x10000 &&
        (cartridge->rom_data[0x7FDF] == 0xA0 ||
         cartridge->rom_data[0x7FFC] == 0xA0)) {
      cartridge->mapper_type = SMS_MAPPER_KOREAN;
      SMS_CART_LOG_INFO("Mapper coreano detectado pela assinatura");
      return;
    }
  }

  // Se a ROM for muito pequena, não precisa de mapper
  if (cartridge->rom_size <= 0x8000) {
    cartridge->mapper_type = SMS_MAPPER_NONE;
    SMS_CART_LOG_INFO("ROM pequena (%d KB), sem mapper",
                      cartridge->rom_size / 1024);
    return;
  }

  // Se nenhum mapper específico foi detectado, usa o padrão Sega
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
    strncpy(cartridge->rom_info.title,
            (const char *)&cartridge->rom_data[0x7FF0 + 16], 32);

    // Garante que o título termine com nulo
    cartridge->rom_info.title[32] = '\0';

    // Extrai informações de região
    cartridge->rom_info.region = (header[15] & 0xF0) >> 4;

    // Verifica se tem bateria
    cartridge->has_battery = (header[15] & 0x08) ? 1 : 0;
    cartridge->rom_info.has_battery = cartridge->has_battery;

    SMS_CART_LOG_INFO("Cabeçalho válido encontrado: %s",
                      cartridge->rom_info.title);
  } else {
    // Cabeçalho inválido, usa nome genérico
    strcpy(cartridge->rom_info.title, "Unknown SMS Game");
    cartridge->rom_info.region = 3; // Desconhecido
    cartridge->has_battery = 0;
    cartridge->rom_info.has_battery = 0;

    SMS_CART_LOG_WARN("Cabeçalho inválido, usando nome genérico");
  }

  // Calcula checksum da ROM para identificação
  if (cartridge->rom_data && cartridge->rom_size > 0) {
    cartridge->rom_info.checksum =
        crc32(0, cartridge->rom_data, cartridge->rom_size);
    SMS_CART_LOG_INFO("Checksum da ROM: 0x%08X", cartridge->rom_info.checksum);
  }

  // Define o tipo de mapper com base no cabeçalho
  sms_cartridge_detect_mapper(cartridge);
  cartridge->rom_info.mapper_type = cartridge->mapper_type;
}

/**
 * @brief Aloca e inicializa a SRAM do cartucho
 *
 * @param cartridge Ponteiro para a instância
 * @return 0 se sucesso, -1 caso contrário
 */
static int sms_cartridge_init_sram(sms_cartridge_t *cartridge) {
  if (!cartridge) {
    return -1;
  }

  // Determina o tamanho da SRAM (se já não estiver definido)
  if (cartridge->ram_size == 0) {
    // Tamanho padrão para cartuchos com bateria
    if (cartridge->has_battery) {
      cartridge->ram_size = 8 * 1024; // 8KB padrão
    } else {
      // Alguns mappers tem RAM mesmo sem bateria
      switch (cartridge->mapper_type) {
      case SMS_MAPPER_CODEMASTERS:
        cartridge->ram_size = 8 * 1024; // 8KB comum
        break;
      case SMS_MAPPER_KOREAN:
        cartridge->ram_size = 8 * 1024; // 8KB comum
        break;
      case SMS_MAPPER_MSX:
        cartridge->ram_size = 8 * 1024; // 8KB comum
        break;
      default:
        cartridge->ram_size = 0; // Sem RAM
        break;
      }
    }
  }

  // Se for necessário ter RAM, aloca memória
  if (cartridge->ram_size > 0) {
    // Libera memória anterior, se houver
    if (cartridge->ram_data) {
      free(cartridge->ram_data);
      cartridge->ram_data = NULL;
    }

    // Aloca nova memória
    cartridge->ram_data = (uint8_t *)malloc(cartridge->ram_size);
    if (!cartridge->ram_data) {
      SMS_CART_LOG_ERROR("Falha ao alocar memória para SRAM (%d bytes)",
                         cartridge->ram_size);
      cartridge->ram_size = 0;
      return -1;
    }

    // Inicializa a memória
    memset(cartridge->ram_data, 0xFF, cartridge->ram_size);

    // Calcula máscara para acesso à RAM
    cartridge->sram_mask = cartridge->ram_size - 1;

    SMS_CART_LOG_INFO("SRAM inicializada: %d KB", cartridge->ram_size / 1024);
  }

  return 0;
}

/**
 * @brief Cria uma nova instância do cartucho
 *
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_cartridge_t *sms_cartridge_create(void) {
  sms_cartridge_t *cartridge =
      (sms_cartridge_t *)malloc(sizeof(sms_cartridge_t));
  if (!cartridge) {
    SMS_CART_LOG_ERROR("Falha ao alocar memória para o cartucho");
    return NULL;
  }

  // Inicializa a estrutura
  memset(cartridge, 0, sizeof(sms_cartridge_t));

  // Configura valores padrão
  cartridge->mapper_type = SMS_MAPPER_NONE;
  cartridge->sram_chip_enabled = 0;
  cartridge->sram_chip_write_enabled = 0;
  cartridge->ram_control_reg = 0;

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

  // Reseta registradores do mapper
  memset(cartridge->mapper_regs, 0, sizeof(cartridge->mapper_regs));

  // Reinicia os flags de controle de SRAM
  cartridge->sram_chip_enabled = 0;
  cartridge->sram_chip_write_enabled = 0;
  cartridge->ram_control_reg = 0;

  // Registradores específicos por tipo de mapper
  switch (cartridge->mapper_type) {
  case SMS_MAPPER_NONE:
    // Sem ação especial
    break;

  case SMS_MAPPER_SEGA:
    // Mapper padrão Sega - inicializa com última página
    if (cartridge->rom_size > 0xC000) {
      uint32_t last_page = (cartridge->rom_size - 0x4000) / 0x4000;
      cartridge->mapper_regs[0] = 0;         // Primeira página em 0x0000
      cartridge->mapper_regs[1] = 1;         // Segunda página em 0x4000
      cartridge->mapper_regs[2] = last_page; // Última página em 0x8000
    }
    break;

  case SMS_MAPPER_CODEMASTERS:
    // Mapper Codemasters - inicializa com primeiras páginas
    cartridge->mapper_regs[0] = 0; // Página 0 em 0x0000
    cartridge->mapper_regs[1] = 1; // Página 1 em 0x4000
    cartridge->mapper_regs[2] = 2; // Página 2 em 0x8000
    break;

  case SMS_MAPPER_KOREAN:
  case SMS_MAPPER_MSX:
  case SMS_MAPPER_NEMESIS:
  case SMS_MAPPER_JANGGUN:
    // Mappers especiais - inicializa com primeiras páginas
    cartridge->mapper_regs[0] = 0;
    cartridge->mapper_regs[1] = 1;
    cartridge->mapper_regs[2] = 2;
    cartridge->mapper_regs[3] = 0;
    break;

  case SMS_MAPPER_MULTI_GAME:
    // Mapper de multi-jogos - volta para o menu
    cartridge->mapper_regs[0] = 0;
    cartridge->mapper_regs[1] = 1;
    cartridge->mapper_regs[2] = cartridge->multi_game_menu_page;
    break;
  }

  SMS_CART_LOG_INFO("Cartucho resetado, mapper=%d", cartridge->mapper_type);
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

  // Abre o arquivo da ROM
  FILE *rom_file = fopen(rom_path, "rb");
  if (!rom_file) {
    SMS_CART_LOG_ERROR("Falha ao abrir arquivo ROM: %s", rom_path);
    return -1;
  }

  // Obtém o tamanho do arquivo
  fseek(rom_file, 0, SEEK_END);
  long file_size = ftell(rom_file);
  fseek(rom_file, 0, SEEK_SET);

  // Verifica se o tamanho é válido
  if (file_size <= 0 || file_size > SMS_MAX_ROM_SIZE) {
    SMS_CART_LOG_ERROR("Tamanho de ROM inválido: %ld bytes", file_size);
    fclose(rom_file);
    return -1;
  }

  // Aloca memória para a ROM
  uint8_t *rom_buffer = (uint8_t *)malloc(file_size);
  if (!rom_buffer) {
    SMS_CART_LOG_ERROR("Falha ao alocar memória para ROM (%ld bytes)",
                       file_size);
    fclose(rom_file);
    return -1;
  }

  // Lê os dados da ROM
  size_t bytes_read = fread(rom_buffer, 1, file_size, rom_file);
  fclose(rom_file);

  if (bytes_read != file_size) {
    SMS_CART_LOG_ERROR("Falha ao ler arquivo ROM: %s", rom_path);
    free(rom_buffer);
    return -1;
  }

  // Libera a ROM anterior, se houver
  if (cartridge->rom_data) {
    free(cartridge->rom_data);
  }

  // Configura a nova ROM
  cartridge->rom_data = rom_buffer;
  cartridge->rom_size = (uint32_t)file_size;

  // Calcula a máscara para acesso à ROM
  cartridge->rom_mask = 1;
  while (cartridge->rom_mask < cartridge->rom_size) {
    cartridge->rom_mask <<= 1;
  }
  cartridge->rom_mask--;

  // Extrai informações do cabeçalho
  sms_cartridge_parse_header(cartridge);

  // Inicializa a SRAM
  sms_cartridge_init_sram(cartridge);

  // Configura o ponteiro de ROM na estrutura de informações
  cartridge->rom_info.rom_data = cartridge->rom_data;
  cartridge->rom_info.size = cartridge->rom_size;

  // Reseta o cartucho para o estado inicial
  sms_cartridge_reset(cartridge);

  SMS_CART_LOG_INFO("ROM carregada com sucesso: %s (%d KB, mapper=%d)",
                    cartridge->rom_info.title, cartridge->rom_size / 1024,
                    cartridge->mapper_type);

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

  // Calcula endereço físico de ROM baseado no mapper
  uint32_t rom_addr = 0;
  bool use_sram = false;

  switch (cartridge->mapper_type) {
  case SMS_MAPPER_NONE:
    // Sem mapper, acesso direto à ROM
    rom_addr = address;
    break;

  case SMS_MAPPER_SEGA:
    // Mapper padrão da Sega
    if (address < 0x4000) {
      // Página 0 (0x0000-0x3FFF)
      rom_addr = (cartridge->mapper_regs[0] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0x8000) {
      // Página 1 (0x4000-0x7FFF)
      rom_addr = (cartridge->mapper_regs[1] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0xC000) {
      // Página 2 (0x8000-0xBFFF)
      rom_addr = (cartridge->mapper_regs[2] * 0x4000) + (address & 0x3FFF);

      // Verifica se o acesso é para a SRAM
      if (cartridge->ram_data && cartridge->ram_size > 0 &&
          cartridge->sram_chip_enabled && address >= 0x8000 &&
          address < 0xC000) {
        use_sram = true;
      }
    }
    break;

  case SMS_MAPPER_CODEMASTERS:
    // Mapper da Codemasters
    if (address < 0x4000) {
      // Página 0 (0x0000-0x3FFF)
      rom_addr = (cartridge->mapper_regs[0] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0x8000) {
      // Página 1 (0x4000-0x7FFF)
      rom_addr = (cartridge->mapper_regs[1] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0xC000) {
      // Página 2 (0x8000-0xBFFF)
      rom_addr = (cartridge->mapper_regs[2] * 0x4000) + (address & 0x3FFF);

      // Verifica se o acesso é para a SRAM
      if (cartridge->ram_data && cartridge->ram_size > 0 &&
          cartridge->sram_chip_enabled && address >= 0xA000 &&
          address < 0xC000) {
        // A SRAM no Codemasters está mapeada em 0xA000-0xBFFF
        use_sram = true;
      }
    }
    break;

  case SMS_MAPPER_KOREAN:
    // Mapper coreano
    if (address < 0x4000) {
      // Página 0 (0x0000-0x3FFF) - fixa
      rom_addr = address;
    } else if (address < 0x8000) {
      // Página 1 (0x4000-0x7FFF)
      rom_addr = (cartridge->mapper_regs[0] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0xC000) {
      // Página 2 (0x8000-0xBFFF)
      rom_addr = (cartridge->mapper_regs[1] * 0x4000) + (address & 0x3FFF);

      // Verifica se o acesso é para a SRAM
      if (cartridge->ram_data && cartridge->ram_size > 0 &&
          cartridge->sram_chip_enabled && address >= 0xA000 &&
          address < 0xBFFF) {
        // A SRAM está mapeada em 0xA000-0xBFFF
        use_sram = true;
      }
    }
    break;

  case SMS_MAPPER_MSX:
    // Mapper MSX
    if (address < 0x4000) {
      // Página 0 (0x0000-0x3FFF) - fixa
      rom_addr = address;
    } else if (address < 0x8000) {
      // Página 1 (0x4000-0x7FFF)
      rom_addr = (cartridge->mapper_regs[0] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0xC000) {
      // Página 2 (0x8000-0xBFFF)
      rom_addr = (cartridge->mapper_regs[1] * 0x4000) + (address & 0x3FFF);

      // Verifica se o acesso é para a SRAM
      if (cartridge->ram_data && cartridge->ram_size > 0 &&
          cartridge->sram_chip_enabled && address >= 0x8000 &&
          address < 0xC000) {
        use_sram = true;
      }
    }
    break;

  case SMS_MAPPER_NEMESIS:
    // Mapper Nemesis - usa páginas de 8KB
    if (address < 0x2000) {
      // Página 0 (0x0000-0x1FFF) - fixa
      rom_addr = address;
    } else if (address < 0x4000) {
      // Página 1 (0x2000-0x3FFF)
      rom_addr = (cartridge->mapper_regs[0] * 0x2000) + (address & 0x1FFF);
    } else if (address < 0x6000) {
      // Página 2 (0x4000-0x5FFF)
      rom_addr = (cartridge->mapper_regs[1] * 0x2000) + (address & 0x1FFF);
    } else if (address < 0x8000) {
      // Página 3 (0x6000-0x7FFF)
      rom_addr = (cartridge->mapper_regs[2] * 0x2000) + (address & 0x1FFF);
    } else if (address < 0xA000) {
      // Página 4 (0x8000-0x9FFF)
      rom_addr = (cartridge->mapper_regs[3] * 0x2000) + (address & 0x1FFF);
    } else if (address < 0xC000) {
      // Página 5 (0xA000-0xBFFF)
      rom_addr =
          ((cartridge->rom_size / 0x2000) - 1) * 0x2000 + (address & 0x1FFF);
    }
    break;

  case SMS_MAPPER_JANGGUN:
    // Mapper especial para The Jang Gun
    if (address < 0x4000) {
      // Página 0 (0x0000-0x3FFF) - controlada por reg3
      rom_addr =
          ((cartridge->mapper_regs[3] & 0x0F) * 0x4000) + (address & 0x3FFF);
    } else if (address < 0x8000) {
      // Página 1 (0x4000-0x7FFF) - controlada por reg0 e reg1
      if (cartridge->mapper_regs[3] & 0x10) {
        // Modo A: 0x4000-0x5FFF e 0x6000-0x7FFF separados
        if (address < 0x6000) {
          rom_addr = (cartridge->mapper_regs[0] * 0x2000) + (address & 0x1FFF);
        } else {
          rom_addr = (cartridge->mapper_regs[1] * 0x2000) + (address & 0x1FFF);
        }
      } else {
        // Modo B: 0x4000-0x7FFF unificado
        rom_addr = (cartridge->mapper_regs[0] * 0x4000) + (address & 0x3FFF);
      }
    } else if (address < 0xC000) {
      // Página 2 (0x8000-0xBFFF) - controlada por reg2
      rom_addr = (cartridge->mapper_regs[2] * 0x4000) + (address & 0x3FFF);
    }
    break;

  case SMS_MAPPER_MULTI_GAME:
    // Mapper para cartuchos multi-jogos
    if (address < 0x4000) {
      // Página 0 (0x0000-0x3FFF)
      rom_addr = (cartridge->mapper_regs[0] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0x8000) {
      // Página 1 (0x4000-0x7FFF)
      rom_addr = (cartridge->mapper_regs[1] * 0x4000) + (address & 0x3FFF);
    } else if (address < 0xC000) {
      // Página 2 (0x8000-0xBFFF)
      rom_addr = (cartridge->mapper_regs[2] * 0x4000) + (address & 0x3FFF);
    }
    break;
  }

  // Verifica se devemos acessar a SRAM
  if (use_sram && cartridge->ram_data) {
    uint32_t ram_addr = address & cartridge->sram_mask;
    return cartridge->ram_data[ram_addr];
  }

  // Verifica limites da ROM
  if (rom_addr < cartridge->rom_size) {
    return cartridge->rom_data[rom_addr];
  }

  // Fora dos limites da ROM
  return 0xFF;
}

/**
 * @brief Escreve um byte na RAM do cartucho
 *
 * @param cartridge Ponteiro para a instância
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void sms_cartridge_write(sms_cartridge_t *cartridge, uint16_t address,
                         uint8_t value) {
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

      // Controle de SRAM (para alguns jogos)
      if (reg == 3) {
        // O último registrador controla a ativação da SRAM em alguns jogos
        cartridge->sram_chip_enabled = (value & 0x08) ? 1 : 0;
        cartridge->sram_chip_write_enabled = (value & 0x04) ? 1 : 0;
        SMS_CART_LOG_TRACE("SRAM: enabled=%d, write=%d",
                           cartridge->sram_chip_enabled,
                           cartridge->sram_chip_write_enabled);
      }
    }

    // Alguns jogos controlam a SRAM em 0x8000
    if (address == 0x8000) {
      cartridge->ram_control_reg = value;
      cartridge->sram_chip_enabled = (value & 0x08) ? 1 : 0;
      cartridge->sram_chip_write_enabled = (value & 0x04) ? 1 : 0;
      SMS_CART_LOG_TRACE("SRAM Control: enabled=%d, write=%d",
                         cartridge->sram_chip_enabled,
                         cartridge->sram_chip_write_enabled);
    }

    // Acesso à SRAM (se habilitado)
    if (cartridge->ram_data && cartridge->ram_size > 0 &&
        cartridge->sram_chip_enabled && cartridge->sram_chip_write_enabled &&
        address >= 0x8000 && address < 0xC000) {

      uint32_t ram_addr = address & cartridge->sram_mask;
      cartridge->ram_data[ram_addr] = value;
      SMS_CART_LOG_TRACE("SRAM Write: addr=%04X, value=%02X", address, value);
    }
    break;

  case SMS_MAPPER_CODEMASTERS:
    // Mapper da Codemasters
    // Registradores em 0x0000, 0x4000, 0x8000
    if (address == 0x0000 || address == 0x4000 || address == 0x8000) {
      uint8_t reg = address >> 14;
      cartridge->mapper_regs[reg] = value;
      SMS_CART_LOG_TRACE("Mapper Codemasters: registrador %d = %02X", reg,
                         value);

      // Codemasters usa bit 7 para controlar a RAM em alguns jogos
      if (address == 0x8000) {
        cartridge->sram_chip_enabled = (value & 0x80) ? 0 : 1;
        cartridge->sram_chip_write_enabled = cartridge->sram_chip_enabled;
      }
    }

    // Acesso à SRAM (se habilitado)
    if (cartridge->ram_data && cartridge->ram_size > 0 &&
        cartridge->sram_chip_enabled && cartridge->sram_chip_write_enabled &&
        address >= 0xA000 && address < 0xC000) {

      uint32_t ram_addr = (address - 0xA000) & cartridge->sram_mask;
      cartridge->ram_data[ram_addr] = value;
      SMS_CART_LOG_TRACE("SRAM Write (Codemasters): addr=%04X, value=%02X",
                         address, value);
    }
    break;

  case SMS_MAPPER_KOREAN:
    // Mapper coreano
    // Registradores em 0xA000
    if (address == 0xA000) {
      cartridge->mapper_regs[0] = value;
      SMS_CART_LOG_TRACE("Mapper coreano: registrador 0 = %02X", value);
    }

    // Controle secundário em 0x8000 (alguns jogos)
    if (address == 0x8000) {
      cartridge->mapper_regs[1] = value;
      SMS_CART_LOG_TRACE("Mapper coreano: registrador 1 = %02X", value);
    }

    // Acesso à SRAM (se habilitado)
    if (cartridge->ram_data && cartridge->ram_size > 0 && address >= 0xA000 &&
        address < 0xC000) {

      uint32_t ram_addr = (address - 0xA000) & cartridge->sram_mask;
      cartridge->ram_data[ram_addr] = value;
      SMS_CART_LOG_TRACE("SRAM Write (Korean): addr=%04X, value=%02X", address,
                         value);
    }
    break;

  case SMS_MAPPER_MSX:
    // Mapper MSX
    // Registradores em 0x4000-0x4001
    if (address == 0x4000) {
      cartridge->mapper_regs[0] = value;
      SMS_CART_LOG_TRACE("Mapper MSX: registrador 0 = %02X", value);
    } else if (address == 0x4001) {
      cartridge->mapper_regs[1] = value;
      SMS_CART_LOG_TRACE("Mapper MSX: registrador 1 = %02X", value);
    }

    // Acesso à SRAM (se habilitado)
    if (cartridge->ram_data && cartridge->ram_size > 0 && address >= 0x8000 &&
        address < 0xC000) {

      uint32_t ram_addr = (address - 0x8000) & cartridge->sram_mask;
      cartridge->ram_data[ram_addr] = value;
      SMS_CART_LOG_TRACE("SRAM Write (MSX): addr=%04X, value=%02X", address,
                         value);
    }
    break;

  case SMS_MAPPER_NEMESIS:
    // Mapper Nemesis
    // Registradores em 0x2000-0x2003 (páginas de 8KB)
    if (address >= 0x2000 && address <= 0x2003) {
      uint8_t reg = address - 0x2000;
      cartridge->mapper_regs[reg] = value;
      SMS_CART_LOG_TRACE("Mapper Nemesis: registrador %d = %02X", reg, value);
    }
    break;

  case SMS_MAPPER_JANGGUN:
    // Mapper especial para The Jang Gun
    // Registradores em 0x4000, 0x6000, 0x8000, 0xA000
    if (address == 0x4000) {
      cartridge->mapper_regs[0] = value;
      SMS_CART_LOG_TRACE("Mapper Janggun: registrador 0 = %02X", value);
    } else if (address == 0x6000) {
      cartridge->mapper_regs[1] = value;
      SMS_CART_LOG_TRACE("Mapper Janggun: registrador 1 = %02X", value);
    } else if (address == 0x8000) {
      cartridge->mapper_regs[2] = value;
      SMS_CART_LOG_TRACE("Mapper Janggun: registrador 2 = %02X", value);
    } else if (address == 0xA000) {
      cartridge->mapper_regs[3] = value;
      SMS_CART_LOG_TRACE("Mapper Janggun: registrador 3 = %02X", value);
    }
    break;

  case SMS_MAPPER_MULTI_GAME:
    // Mapper para cartuchos multi-jogos
    // Registradores em 0x3FFE, 0x7FFF, 0xBFFF
    if (address == 0x3FFE) {
      cartridge->mapper_regs[0] = value;
      SMS_CART_LOG_TRACE("Mapper Multi-Game: registrador 0 = %02X", value);
    } else if (address == 0x7FFF) {
      cartridge->mapper_regs[1] = value;
      SMS_CART_LOG_TRACE("Mapper Multi-Game: registrador 1 = %02X", value);
    } else if (address == 0xBFFF) {
      cartridge->mapper_regs[2] = value;
      SMS_CART_LOG_TRACE("Mapper Multi-Game: registrador 2 = %02X", value);

      // Armazena a página do menu para uso após reset
      if (value == 0) {
        cartridge->multi_game_menu_page = 0;
      }
    }
    break;
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
  if (!cartridge || !cartridge->ram_data || !cartridge->has_battery ||
      !save_path) {
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
  if (!cartridge || !cartridge->ram_data || !cartridge->has_battery ||
      !save_path) {
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
    SMS_CART_LOG_WARN(
        "Tamanho do arquivo de RAM inválido: %ld bytes (esperado: %d bytes)",
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
int sms_cartridge_register_save_state(sms_cartridge_t *cartridge,
                                      save_state_t *state) {
  if (!cartridge || !state) {
    return -1;
  }

  // Registra os registradores do mapper
  save_state_register_field(state, "sms_cartridge_mapper_regs",
                            cartridge->mapper_regs,
                            sizeof(cartridge->mapper_regs));

  // Registra o tipo de mapper
  save_state_register_field(state, "sms_cartridge_mapper_type",
                            &cartridge->mapper_type,
                            sizeof(cartridge->mapper_type));

  // Registra as flags da SRAM
  save_state_register_field(state, "sms_cartridge_sram_chip_enabled",
                            &cartridge->sram_chip_enabled,
                            sizeof(cartridge->sram_chip_enabled));

  save_state_register_field(state, "sms_cartridge_sram_chip_write_enabled",
                            &cartridge->sram_chip_write_enabled,
                            sizeof(cartridge->sram_chip_write_enabled));

  save_state_register_field(state, "sms_cartridge_ram_control_reg",
                            &cartridge->ram_control_reg,
                            sizeof(cartridge->ram_control_reg));

  save_state_register_field(state, "sms_cartridge_multi_game_menu_page",
                            &cartridge->multi_game_menu_page,
                            sizeof(cartridge->multi_game_menu_page));

  // Registra a RAM do cartucho, se presente
  if (cartridge->ram_data && cartridge->ram_size > 0) {
    save_state_register_field(state, "sms_cartridge_ram", cartridge->ram_data,
                              cartridge->ram_size);
  }

  return 0;
}

/**
 * @brief Ativa ou desativa manualmente a SRAM do cartucho
 *
 * @param cartridge Ponteiro para a instância
 * @param enabled Estado da SRAM (1=ativado, 0=desativado)
 * @param write_enabled Estado da permissão de escrita (1=permitido, 0=somente
 * leitura)
 */
void sms_cartridge_set_sram_enabled(sms_cartridge_t *cartridge, uint8_t enabled,
                                    uint8_t write_enabled) {
  if (!cartridge || !cartridge->ram_data || cartridge->ram_size == 0) {
    return;
  }

  cartridge->sram_chip_enabled = enabled ? 1 : 0;
  cartridge->sram_chip_write_enabled = write_enabled ? 1 : 0;

  SMS_CART_LOG_INFO("SRAM configurada manualmente: enabled=%d, write=%d",
                    cartridge->sram_chip_enabled,
                    cartridge->sram_chip_write_enabled);
}

/**
 * @brief Verifica se a SRAM está habilitada
 *
 * @param cartridge Ponteiro para a instância
 * @return 1 se a SRAM está habilitada, 0 caso contrário
 */
uint8_t sms_cartridge_is_sram_enabled(sms_cartridge_t *cartridge) {
  if (!cartridge || !cartridge->ram_data || cartridge->ram_size == 0) {
    return 0;
  }

  return cartridge->sram_chip_enabled;
}

/**
 * @brief Obtém ponteiro para os dados da SRAM
 *
 * @param cartridge Ponteiro para a instância
 * @param size Ponteiro para receber o tamanho da SRAM (em bytes)
 * @return Ponteiro para os dados da SRAM ou NULL se não houver
 */
uint8_t *sms_cartridge_get_sram_data(sms_cartridge_t *cartridge,
                                     uint32_t *size) {
  if (!cartridge || !cartridge->ram_data || cartridge->ram_size == 0) {
    if (size) {
      *size = 0;
    }
    return NULL;
  }

  if (size) {
    *size = cartridge->ram_size;
  }

  return cartridge->ram_data;
}
