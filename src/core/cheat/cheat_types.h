/**
 * @file cheat_types.h
 * @brief Definições de tipos e estruturas para o sistema de cheats
 */

#ifndef MEGA_EMU_CHEAT_TYPES_H
#define MEGA_EMU_CHEAT_TYPES_H

#include "../global_defines.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tipos de cheats suportados
 */
typedef enum {
  CHEAT_TYPE_RAW,        ///< Cheat direto (modificação direta de memória)
  CHEAT_TYPE_GAME_GENIE, ///< Códigos Game Genie
  CHEAT_TYPE_PRO_ACTION_REPLAY, ///< Códigos Pro Action Replay
  CHEAT_TYPE_GAMESHARK, ///< Códigos GameShark (para plataformas onde aplicável)
  CHEAT_TYPE_CONDITIONAL ///< Cheats condicionais
} mega_emu_cheat_type_t;

/**
 * @brief Plataformas suportadas para cheats
 */
typedef enum {
  CHEAT_PLATFORM_MEGADRIVE,    ///< Mega Drive/Genesis
  CHEAT_PLATFORM_MASTERSYSTEM, ///< Master System
  CHEAT_PLATFORM_GAMEGEAR,     ///< Game Gear
  CHEAT_PLATFORM_NES,          ///< NES
  CHEAT_PLATFORM_SNES,         ///< SNES
  CHEAT_PLATFORM_GAMEBOY,      ///< Game Boy
  CHEAT_PLATFORM_GENERIC       ///< Genérico (para todos os sistemas)
} mega_emu_cheat_platform_t;

/**
 * @brief Comparadores para cheats condicionais
 */
typedef enum {
  CHEAT_CMP_EQUAL,         ///< Igual (==)
  CHEAT_CMP_NOT_EQUAL,     ///< Diferente (!=)
  CHEAT_CMP_GREATER,       ///< Maior (>)
  CHEAT_CMP_LESS,          ///< Menor (<)
  CHEAT_CMP_GREATER_EQUAL, ///< Maior ou igual (>=)
  CHEAT_CMP_LESS_EQUAL     ///< Menor ou igual (<=)
} mega_emu_cheat_comparator_t;

/**
 * @brief Tamanhos de dados para operações de cheat
 */
typedef enum {
  CHEAT_SIZE_8BIT,  ///< 8 bits (1 byte)
  CHEAT_SIZE_16BIT, ///< 16 bits (2 bytes)
  CHEAT_SIZE_32BIT, ///< 32 bits (4 bytes)
  CHEAT_SIZE_24BIT  ///< 24 bits (3 bytes, para casos específicos)
} mega_emu_cheat_size_t;

/**
 * @brief Estrutura para um cheat genérico
 */
typedef struct {
  char name[128];                     ///< Nome do cheat
  char description[256];              ///< Descrição
  bool enabled;                       ///< Se o cheat está ativo
  mega_emu_cheat_type_t type;         ///< Tipo do cheat
  mega_emu_cheat_platform_t platform; ///< Plataforma alvo
  char code[32]; ///< Código original (Game Genie, PAR, etc.)

  // Informações de endereçamento
  uint32_t address;           ///< Endereço na memória
  uint32_t value;             ///< Valor a ser escrito
  mega_emu_cheat_size_t size; ///< Tamanho do valor (8/16/24/32 bits)

  // Para cheats condicionais
  bool is_conditional;                    ///< Se é um cheat condicional
  uint32_t compare_value;                 ///< Valor para comparação
  mega_emu_cheat_comparator_t comparator; ///< Operador de comparação

  // Para cheats que precisam de vários endereços/valores
  uint32_t alt_address; ///< Endereço alternativo
  uint32_t alt_value;   ///< Valor alternativo
} mega_emu_cheat_t;

/**
 * @brief Lista de cheats para um jogo específico
 */
typedef struct {
  char game_name[128];      ///< Nome do jogo
  char game_hash[65];       ///< Hash MD5/SHA1 do ROM (para identificação)
  mega_emu_cheat_t *cheats; ///< Array de cheats
  uint32_t cheat_count;     ///< Número de cheats na lista
  uint32_t capacity;        ///< Capacidade do array (para alocação)
} mega_emu_cheat_list_t;

/**
 * @brief Resultado da busca de cheats
 */
typedef struct {
  uint32_t address;           ///< Endereço encontrado
  mega_emu_cheat_size_t size; ///< Tamanho do valor
  uint32_t current_value;     ///< Valor atual
  uint32_t previous_value;    ///< Valor anterior (para comparação)
} mega_emu_cheat_search_result_t;

/**
 * @brief Contexto de busca para o Cheat Finder
 */
typedef struct {
  mega_emu_cheat_search_result_t *results; ///< Resultados da busca
  uint32_t result_count;                   ///< Número de resultados
  uint32_t capacity;                       ///< Capacidade do array

  // Filtros atuais
  mega_emu_cheat_comparator_t comparator; ///< Comparador atual
  uint32_t compare_value;                 ///< Valor para comparação
  bool use_previous_value;    ///< Se deve comparar com o valor anterior
  mega_emu_cheat_size_t size; ///< Tamanho do valor para busca
} mega_emu_cheat_finder_t;

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_CHEAT_TYPES_H */
