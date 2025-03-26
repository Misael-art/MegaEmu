/**
 * @file rom_db_utils.c
 * @brief Implementação de funções de utilitário para o banco de dados de ROMs
 */

#include "rom_db.h"
#include "../global_defines.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sqlite3.h>

// Referência externa para o contexto do banco de dados
extern sqlite3 *g_rom_db_handle;

/**
 * @brief Função de utilidade para converter string em enum de plataforma
 *
 * @param platform_str Nome da plataforma
 * @return mega_emu_rom_db_platform_t Enum correspondente
 */
mega_emu_rom_db_platform_t
mega_emu_rom_db_string_to_platform(const char *platform_str) {
    if (platform_str == NULL || *platform_str == '\0') {
        return ROM_DB_PLATFORM_UNKNOWN;
    }

    // Converter para minúsculas para comparação
    char platform_lower[32];
    size_t len = strlen(platform_str);
    if (len > sizeof(platform_lower) - 1) {
        len = sizeof(platform_lower) - 1;
    }

    for (size_t i = 0; i < len; i++) {
        platform_lower[i] = (char)tolower((unsigned char)platform_str[i]);
    }
    platform_lower[len] = '\0';

    // Comparar com nomes conhecidos
    if (strstr(platform_lower, "megadrive") || strstr(platform_lower, "genesis")) {
        return ROM_DB_PLATFORM_MEGADRIVE;
    } else if (strstr(platform_lower, "mastersystem") || strstr(platform_lower, "sms")) {
        return ROM_DB_PLATFORM_MASTERSYSTEM;
    } else if (strstr(platform_lower, "gamegear") || strstr(platform_lower, "gg")) {
        return ROM_DB_PLATFORM_GAMEGEAR;
    } else if (strstr(platform_lower, "nes") || strstr(platform_lower, "famicom")) {
        return ROM_DB_PLATFORM_NES;
    } else if (strstr(platform_lower, "snes") || strstr(platform_lower, "superfamicom")) {
        return ROM_DB_PLATFORM_SNES;
    } else if (strstr(platform_lower, "gameboy") && strstr(platform_lower, "color")) {
        return ROM_DB_PLATFORM_GAMEBOY_COLOR;
    } else if (strstr(platform_lower, "gameboy") || strstr(platform_lower, "gb")) {
        return ROM_DB_PLATFORM_GAMEBOY;
    }

    return ROM_DB_PLATFORM_UNKNOWN;
}

/**
 * @brief Função de utilidade para converter enum de plataforma em string
 *
 * @param platform Enum da plataforma
 * @return const char* Nome da plataforma
 */
const char *
mega_emu_rom_db_platform_to_string(mega_emu_rom_db_platform_t platform) {
    switch (platform) {
        case ROM_DB_PLATFORM_MEGADRIVE:
            return "Mega Drive";
        case ROM_DB_PLATFORM_MASTERSYSTEM:
            return "Master System";
        case ROM_DB_PLATFORM_GAMEGEAR:
            return "Game Gear";
        case ROM_DB_PLATFORM_NES:
            return "NES";
        case ROM_DB_PLATFORM_SNES:
            return "SNES";
        case ROM_DB_PLATFORM_GAMEBOY:
            return "Game Boy";
        case ROM_DB_PLATFORM_GAMEBOY_COLOR:
            return "Game Boy Color";
        case ROM_DB_PLATFORM_UNKNOWN:
        default:
            return "Desconhecido";
    }
}

/**
 * @brief Função de utilidade para converter string em enum de região
 *
 * @param region_str Nome da região
 * @return mega_emu_rom_db_region_t Enum correspondente
 */
mega_emu_rom_db_region_t
mega_emu_rom_db_string_to_region(const char *region_str) {
    if (region_str == NULL || *region_str == '\0') {
        return ROM_DB_REGION_UNKNOWN;
    }

    // Converter para minúsculas para comparação
    char region_lower[32];
    size_t len = strlen(region_str);
    if (len > sizeof(region_lower) - 1) {
        len = sizeof(region_lower) - 1;
    }

    for (size_t i = 0; i < len; i++) {
        region_lower[i] = (char)tolower((unsigned char)region_str[i]);
    }
    region_lower[len] = '\0';

    // Comparar com nomes conhecidos
    if (strstr(region_lower, "japan") || strstr(region_lower, "jpn") || strstr(region_lower, "j")) {
        return ROM_DB_REGION_JAPAN;
    } else if (strstr(region_lower, "usa") || strstr(region_lower, "us") || strstr(region_lower, "u")) {
        return ROM_DB_REGION_USA;
    } else if (strstr(region_lower, "europe") || strstr(region_lower, "eur") || strstr(region_lower, "e")) {
        return ROM_DB_REGION_EUROPE;
    } else if (strstr(region_lower, "brazil") || strstr(region_lower, "bra") || strstr(region_lower, "br")) {
        return ROM_DB_REGION_BRAZIL;
    } else if (strstr(region_lower, "korea") || strstr(region_lower, "kor") || strstr(region_lower, "k")) {
        return ROM_DB_REGION_KOREA;
    } else if (strstr(region_lower, "china") || strstr(region_lower, "chn") || strstr(region_lower, "c")) {
        return ROM_DB_REGION_CHINA;
    } else if (strstr(region_lower, "world") || strstr(region_lower, "wld") || strstr(region_lower, "w")) {
        return ROM_DB_REGION_WORLD;
    }

    return ROM_DB_REGION_OTHER;
}

/**
 * @brief Função de utilidade para converter enum de região em string
 *
 * @param region Enum da região
 * @return const char* Nome da região
 */
const char *mega_emu_rom_db_region_to_string(mega_emu_rom_db_region_t region) {
    switch (region) {
        case ROM_DB_REGION_JAPAN:
            return "Japão";
        case ROM_DB_REGION_USA:
            return "EUA";
        case ROM_DB_REGION_EUROPE:
            return "Europa";
        case ROM_DB_REGION_BRAZIL:
            return "Brasil";
        case ROM_DB_REGION_KOREA:
            return "Coréia";
        case ROM_DB_REGION_CHINA:
            return "China";
        case ROM_DB_REGION_WORLD:
            return "Mundial";
        case ROM_DB_REGION_OTHER:
            return "Outra";
        case ROM_DB_REGION_UNKNOWN:
        default:
            return "Desconhecida";
    }
}

/**
 * @brief Função de utilidade para converter string em enum de gênero
 *
 * @param genre_str Nome do gênero
 * @return mega_emu_rom_db_genre_t Enum correspondente
 */
mega_emu_rom_db_genre_t mega_emu_rom_db_string_to_genre(const char *genre_str) {
    if (genre_str == NULL || *genre_str == '\0') {
        return ROM_DB_GENRE_UNKNOWN;
    }

    // Converter para minúsculas para comparação
    char genre_lower[32];
    size_t len = strlen(genre_str);
    if (len > sizeof(genre_lower) - 1) {
        len = sizeof(genre_lower) - 1;
    }

    for (size_t i = 0; i < len; i++) {
        genre_lower[i] = (char)tolower((unsigned char)genre_str[i]);
    }
    genre_lower[len] = '\0';

    // Comparar com nomes conhecidos
    if (strstr(genre_lower, "action")) {
        return ROM_DB_GENRE_ACTION;
    } else if (strstr(genre_lower, "adventure")) {
        return ROM_DB_GENRE_ADVENTURE;
    } else if (strstr(genre_lower, "arcade")) {
        return ROM_DB_GENRE_ARCADE;
    } else if (strstr(genre_lower, "board")) {
        return ROM_DB_GENRE_BOARD_GAME;
    } else if (strstr(genre_lower, "fight")) {
        return ROM_DB_GENRE_FIGHTING;
    } else if (strstr(genre_lower, "platform")) {
        return ROM_DB_GENRE_PLATFORMER;
    } else if (strstr(genre_lower, "puzzle")) {
        return ROM_DB_GENRE_PUZZLE;
    } else if (strstr(genre_lower, "rac")) {
        return ROM_DB_GENRE_RACING;
    } else if (strstr(genre_lower, "rpg") || strstr(genre_lower, "role")) {
        return ROM_DB_GENRE_RPG;
    } else if (strstr(genre_lower, "shoot")) {
        return ROM_DB_GENRE_SHOOTER;
    } else if (strstr(genre_lower, "sim")) {
        return ROM_DB_GENRE_SIMULATION;
    } else if (strstr(genre_lower, "sport")) {
        return ROM_DB_GENRE_SPORTS;
    } else if (strstr(genre_lower, "strat")) {
        return ROM_DB_GENRE_STRATEGY;
    } else if (strstr(genre_lower, "edu")) {
        return ROM_DB_GENRE_EDUCATIONAL;
    }

    return ROM_DB_GENRE_OTHER;
}

/**
 * @brief Função de utilidade para converter enum de gênero em string
 *
 * @param genre Enum do gênero
 * @return const char* Nome do gênero
 */
const char *mega_emu_rom_db_genre_to_string(mega_emu_rom_db_genre_t genre) {
    switch (genre) {
        case ROM_DB_GENRE_ACTION:
            return "Ação";
        case ROM_DB_GENRE_ADVENTURE:
            return "Aventura";
        case ROM_DB_GENRE_ARCADE:
            return "Arcade";
        case ROM_DB_GENRE_BOARD_GAME:
            return "Jogo de Tabuleiro";
        case ROM_DB_GENRE_FIGHTING:
            return "Luta";
        case ROM_DB_GENRE_PLATFORMER:
            return "Plataforma";
        case ROM_DB_GENRE_PUZZLE:
            return "Quebra-cabeça";
        case ROM_DB_GENRE_RACING:
            return "Corrida";
        case ROM_DB_GENRE_RPG:
            return "RPG";
        case ROM_DB_GENRE_SHOOTER:
            return "Tiro";
        case ROM_DB_GENRE_SIMULATION:
            return "Simulação";
        case ROM_DB_GENRE_SPORTS:
            return "Esportes";
        case ROM_DB_GENRE_STRATEGY:
            return "Estratégia";
        case ROM_DB_GENRE_EDUCATIONAL:
            return "Educativo";
        case ROM_DB_GENRE_OTHER:
            return "Outro";
        case ROM_DB_GENRE_UNKNOWN:
        default:
            return "Desconhecido";
    }
}

/**
 * @brief Obtém estatísticas do banco de dados
 *
 * @param platform_count Vetor para armazenar o número de ROMs por plataforma
 * @param region_count Vetor para armazenar o número de ROMs por região
 * @param total_entries Ponteiro para variável que receberá o número total de
 * entradas
 * @return true Se as estatísticas foram obtidas com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_get_stats(uint32_t platform_count[ROM_DB_PLATFORM_COUNT],
                            uint32_t region_count[ROM_DB_REGION_COUNT],
                            uint32_t *total_entries) {
    if (!g_rom_db_handle) {
        return false;
    }

    // Inicializar arrays
    if (platform_count) {
        memset(platform_count, 0, sizeof(uint32_t) * ROM_DB_PLATFORM_COUNT);
    }

    if (region_count) {
        memset(region_count, 0, sizeof(uint32_t) * ROM_DB_REGION_COUNT);
    }

    // Obter total de entradas
    if (total_entries) {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(g_rom_db_handle, "SELECT COUNT(*) FROM rom_entries", -1, &stmt, NULL);

        if (rc == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                *total_entries = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        } else {
            return false;
        }
    }

    // Obter contagem por plataforma
    if (platform_count) {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(g_rom_db_handle,
                                 "SELECT platform, COUNT(*) FROM rom_entries GROUP BY platform",
                                 -1, &stmt, NULL);

        if (rc == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int platform = sqlite3_column_int(stmt, 0);
                int count = sqlite3_column_int(stmt, 1);

                if (platform >= 0 && platform < ROM_DB_PLATFORM_COUNT) {
                    platform_count[platform] = count;
                }
            }
            sqlite3_finalize(stmt);
        } else {
            return false;
        }
    }

    // Obter contagem por região
    if (region_count) {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(g_rom_db_handle,
                                 "SELECT region, COUNT(*) FROM rom_entries GROUP BY region",
                                 -1, &stmt, NULL);

        if (rc == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int region = sqlite3_column_int(stmt, 0);
                int count = sqlite3_column_int(stmt, 1);

                if (region >= 0 && region < ROM_DB_REGION_COUNT) {
                    region_count[region] = count;
                }
            }
            sqlite3_finalize(stmt);
        } else {
            return false;
        }
    }

    return true;
}

/**
 * @brief Compacta o banco de dados (remove espaços não utilizados)
 *
 * @return true Se a compactação foi realizada com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_compact(void) {
    if (!g_rom_db_handle) {
        return false;
    }

    char *err_msg = NULL;
    int rc = sqlite3_exec(g_rom_db_handle, "VACUUM", NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        if (err_msg) {
            sqlite3_free(err_msg);
        }
        return false;
    }

    return true;
}
