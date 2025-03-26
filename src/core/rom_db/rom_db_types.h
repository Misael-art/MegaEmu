/**
 * @file rom_db_types.h
 * @brief Definições de tipos para o banco de dados de ROMs
 */

#ifndef MEGA_EMU_ROM_DB_TYPES_H
#define MEGA_EMU_ROM_DB_TYPES_H

#include "../global_defines.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Plataformas suportadas pelo banco de dados de ROMs
 */
typedef enum {
  ROM_DB_PLATFORM_UNKNOWN = 0,
  ROM_DB_PLATFORM_MEGADRIVE,     // Mega Drive / Genesis
  ROM_DB_PLATFORM_MASTERSYSTEM,  // Master System
  ROM_DB_PLATFORM_GAMEGEAR,      // Game Gear
  ROM_DB_PLATFORM_NES,           // Nintendo Entertainment System
  ROM_DB_PLATFORM_SNES,          // Super Nintendo Entertainment System
  ROM_DB_PLATFORM_GAMEBOY,       // Game Boy
  ROM_DB_PLATFORM_GAMEBOY_COLOR, // Game Boy Color
  ROM_DB_PLATFORM_COUNT
} mega_emu_rom_db_platform_t;

/**
 * @brief Regiões de jogos
 */
typedef enum {
  ROM_DB_REGION_UNKNOWN = 0,
  ROM_DB_REGION_JAPAN,
  ROM_DB_REGION_USA,
  ROM_DB_REGION_EUROPE,
  ROM_DB_REGION_BRAZIL,
  ROM_DB_REGION_KOREA,
  ROM_DB_REGION_CHINA,
  ROM_DB_REGION_WORLD,
  ROM_DB_REGION_OTHER,
  ROM_DB_REGION_COUNT
} mega_emu_rom_db_region_t;

/**
 * @brief Status de compatibilidade da ROM
 */
typedef enum {
  ROM_DB_COMPAT_UNKNOWN = 0, // Compatibilidade desconhecida
  ROM_DB_COMPAT_PERFECT,     // Funciona perfeitamente
  ROM_DB_COMPAT_PLAYABLE,    // Jogável com pequenos problemas
  ROM_DB_COMPAT_PARTIAL,     // Parcialmente funcional
  ROM_DB_COMPAT_BROKEN,      // Quebrado, não funciona
  ROM_DB_COMPAT_COUNT
} mega_emu_rom_db_compat_t;

/**
 * @brief Tipos de mídias
 */
typedef enum {
  ROM_DB_MEDIA_UNKNOWN = 0,
  ROM_DB_MEDIA_CARTRIDGE, // Cartucho padrão
  ROM_DB_MEDIA_CD,        // CD (Mega CD, etc)
  ROM_DB_MEDIA_DISC,      // Disco (FDS, etc)
  ROM_DB_MEDIA_CUSTOM,    // Hardware customizado
  ROM_DB_MEDIA_COUNT
} mega_emu_rom_db_media_t;

/**
 * @brief Tipos de entrada para jogos
 */
typedef enum {
  ROM_DB_INPUT_UNKNOWN = 0,
  ROM_DB_INPUT_GAMEPAD,      // Controle padrão
  ROM_DB_INPUT_ZAPPER,       // Pistola de luz (NES)
  ROM_DB_INPUT_PADDLE,       // Paddle/Dial
  ROM_DB_INPUT_KEYBOARD,     // Teclado
  ROM_DB_INPUT_LIGHT_PHASER, // Light Phaser (Master System)
  ROM_DB_INPUT_MOUSE,        // Mouse
  ROM_DB_INPUT_MULTITAP,     // Multitap (4+ jogadores)
  ROM_DB_INPUT_CUSTOM,       // Hardware de entrada personalizado
  ROM_DB_INPUT_COUNT
} mega_emu_rom_db_input_t;

/**
 * @brief Gêneros de jogos
 */
typedef enum {
  ROM_DB_GENRE_UNKNOWN = 0,
  ROM_DB_GENRE_ACTION,
  ROM_DB_GENRE_ADVENTURE,
  ROM_DB_GENRE_ARCADE,
  ROM_DB_GENRE_BOARD_GAME,
  ROM_DB_GENRE_FIGHTING,
  ROM_DB_GENRE_PLATFORMER,
  ROM_DB_GENRE_PUZZLE,
  ROM_DB_GENRE_RACING,
  ROM_DB_GENRE_RPG,
  ROM_DB_GENRE_SHOOTER,
  ROM_DB_GENRE_SIMULATION,
  ROM_DB_GENRE_SPORTS,
  ROM_DB_GENRE_STRATEGY,
  ROM_DB_GENRE_EDUCATIONAL,
  ROM_DB_GENRE_OTHER,
  ROM_DB_GENRE_COUNT
} mega_emu_rom_db_genre_t;

/**
 * @brief Estrutura para armazenar hash de ROM
 */
typedef struct {
  uint8_t md5[16];  // Hash MD5
  uint8_t sha1[20]; // Hash SHA1
  uint8_t crc32[4]; // CRC32
} mega_emu_rom_db_hash_t;

/**
 * @brief Estrutura para armazenar informações de um jogo/ROM
 */
typedef struct {
  uint32_t id;           // ID único no banco de dados
  char title[128];       // Título do jogo
  char alt_title[128];   // Título alternativo
  char developer[64];    // Desenvolvedor
  char publisher[64];    // Publicador
  char release_date[12]; // Data de lançamento (YYYY-MM-DD)

  mega_emu_rom_db_platform_t platform;    // Plataforma
  mega_emu_rom_db_region_t region;        // Região principal
  mega_emu_rom_db_compat_t compatibility; // Status de compatibilidade
  mega_emu_rom_db_media_t media_type;     // Tipo de mídia
  mega_emu_rom_db_genre_t genre;          // Gênero principal
  mega_emu_rom_db_input_t input_type;     // Tipo de entrada principal

  char description[512];       // Descrição do jogo
  mega_emu_rom_db_hash_t hash; // Hash para identificação

  uint32_t size;   // Tamanho da ROM em bytes
  uint8_t players; // Número de jogadores

  // Campos opcionais
  char serial[32];    // Número serial/código do produto
  char version[16];   // Versão (1.0, Rev A, etc)
  char save_type[32]; // Tipo de save (SRAM, EEPROM, etc)
  bool has_battery;   // Se possui bateria para salvar

  // Flags para requisitos especiais de emulação
  uint32_t flags; // Flags para emulação especial

  // Campos para extensão futura
  char extra_data[256]; // Dados extras em formato JSON

  // Metadados do banco de dados
  uint32_t db_revision;  // Revisão do banco em que foi adicionado
  char added_date[12];   // Data em que foi adicionado (YYYY-MM-DD)
  char updated_date[12]; // Data da última atualização (YYYY-MM-DD)
} mega_emu_rom_db_entry_t;

/**
 * @brief Critérios para pesquisa no banco de dados
 */
typedef struct {
  char title[128];                     // Pesquisar por título (parcial)
  mega_emu_rom_db_platform_t platform; // Filtrar por plataforma
  mega_emu_rom_db_region_t region;     // Filtrar por região
  mega_emu_rom_db_genre_t genre;       // Filtrar por gênero
  mega_emu_rom_db_hash_t hash;         // Pesquisar por hash exato
  bool use_hash;                       // Flag para indicar se deve usar hash
  bool use_platform; // Flag para indicar se deve filtrar por plataforma
  bool use_region;   // Flag para indicar se deve filtrar por região
  bool use_genre;    // Flag para indicar se deve filtrar por gênero

  // Ordenação
  uint8_t sort_by;     // Campo para ordenação (0=título, 1=data, etc)
  bool sort_ascending; // Ordem ascendente ou descendente

  // Paginação
  uint32_t page;           // Página atual (começando em 0)
  uint32_t items_per_page; // Itens por página (0 = sem paginação)
} mega_emu_rom_db_search_t;

/**
 * @brief Resultado de pesquisa no banco de dados
 */
typedef struct {
  mega_emu_rom_db_entry_t *entries; // Vetor de entradas encontradas
  uint32_t count;                   // Número de entradas encontradas
  uint32_t total_matches;  // Total de correspondências (para paginação)
  bool success;            // Se a pesquisa foi bem-sucedida
  char error_message[256]; // Mensagem de erro, se houver
} mega_emu_rom_db_search_result_t;

/**
 * @brief Estrutura para callback de notificação de progresso
 */
typedef void (*mega_emu_rom_db_progress_callback_t)(uint32_t current,
                                                    uint32_t total,
                                                    void *user_data);

/**
 * @brief Estrutura para metadados do banco de dados
 */
typedef struct {
  uint32_t version;      // Versão do banco de dados
  uint32_t entry_count;  // Número total de entradas
  char build_date[12];   // Data de compilação (YYYY-MM-DD)
  char description[256]; // Descrição do banco de dados

  // Estatísticas
  uint32_t
      entries_by_platform[ROM_DB_PLATFORM_COUNT];  // Entradas por plataforma
  uint32_t entries_by_region[ROM_DB_REGION_COUNT]; // Entradas por região
} mega_emu_rom_db_metadata_t;

#endif // MEGA_EMU_ROM_DB_TYPES_H
