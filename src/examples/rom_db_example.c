/**
 * @file rom_db_example.c
 * @brief Exemplo de uso do banco de dados de ROMs
 *
 * Este arquivo demonstra as operações básicas do banco de dados de ROMs,
 * incluindo inicialização, pesquisa, adição e atualização de entradas.
 */

#include "../core/rom_db/rom_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Callback para exibir progresso
void print_progress_callback(uint32_t current, uint32_t total, void *user_data) {
    const char *operation = (const char*)user_data;
    printf("\r%s: %u/%u (%.1f%%)", operation, current, total,
           (float)current * 100.0f / (float)total);
    fflush(stdout);

    if (current >= total) {
        printf("\n");
    }
}

// Mostrar informações de uma ROM
void print_rom_info(const mega_emu_rom_db_entry_t *entry) {
    char md5_str[33] = {0};
    char sha1_str[41] = {0};
    char crc32_str[9] = {0};

    mega_emu_rom_db_hash_to_string(&entry->hash, md5_str, sizeof(md5_str), 1);
    mega_emu_rom_db_hash_to_string(&entry->hash, sha1_str, sizeof(sha1_str), 2);
    mega_emu_rom_db_hash_to_string(&entry->hash, crc32_str, sizeof(crc32_str), 0);

    printf("==== Informações da ROM ====\n");
    printf("ID: %u\n", entry->id);
    printf("Título: %s\n", entry->title);
    if (entry->alt_title[0] != '\0') printf("Título Alternativo: %s\n", entry->alt_title);
    if (entry->developer[0] != '\0') printf("Desenvolvedor: %s\n", entry->developer);
    if (entry->publisher[0] != '\0') printf("Publicador: %s\n", entry->publisher);
    if (entry->release_date[0] != '\0') printf("Data de Lançamento: %s\n", entry->release_date);

    printf("Plataforma: %s\n", mega_emu_rom_db_platform_to_string(entry->platform));
    printf("Região: %s\n", mega_emu_rom_db_region_to_string(entry->region));
    printf("Gênero: %s\n", mega_emu_rom_db_genre_to_string(entry->genre));

    printf("Tamanho: %u bytes\n", entry->size);
    printf("MD5: %s\n", md5_str);
    printf("SHA1: %s\n", sha1_str);
    printf("CRC32: %s\n", crc32_str);

    if (entry->description[0] != '\0') {
        printf("\nDescrição:\n%s\n", entry->description);
    }

    printf("============================\n\n");
}

int main(int argc, char *argv[]) {
    // Inicializar o banco de dados
    printf("Inicializando banco de dados de ROMs...\n");
    if (!mega_emu_rom_db_init("romdb.sqlite")) {
        printf("Erro ao inicializar o banco de dados!\n");
        return 1;
    }

    // Exibir metadados do banco
    mega_emu_rom_db_metadata_t metadata;
    if (mega_emu_rom_db_get_metadata(&metadata)) {
        printf("Banco de dados versão %u\n", metadata.version);
        printf("Total de ROMs: %u\n", metadata.entry_count);
        printf("Data de compilação: %s\n", metadata.build_date);
        printf("Descrição: %s\n\n", metadata.description);

        // Mostrar estatísticas
        printf("Estatísticas por plataforma:\n");
        for (int i = 1; i < ROM_DB_PLATFORM_COUNT; i++) {
            if (metadata.entries_by_platform[i] > 0) {
                printf("  %s: %u ROMs\n",
                       mega_emu_rom_db_platform_to_string(i),
                       metadata.entries_by_platform[i]);
            }
        }

        printf("\nEstatísticas por região:\n");
        for (int i = 1; i < ROM_DB_REGION_COUNT; i++) {
            if (metadata.entries_by_region[i] > 0) {
                printf("  %s: %u ROMs\n",
                       mega_emu_rom_db_region_to_string(i),
                       metadata.entries_by_region[i]);
            }
        }
        printf("\n");
    }

    // Exemplo de cálculo de hash
    if (argc > 1) {
        printf("Calculando hash para o arquivo: %s\n", argv[1]);
        mega_emu_rom_db_hash_t hash;

        if (mega_emu_rom_db_calculate_hash(argv[1], &hash, print_progress_callback, "Calculando hash")) {
            char md5_str[33] = {0};
            char sha1_str[41] = {0};
            char crc32_str[9] = {0};

            mega_emu_rom_db_hash_to_string(&hash, md5_str, sizeof(md5_str), 1);
            mega_emu_rom_db_hash_to_string(&hash, sha1_str, sizeof(sha1_str), 2);
            mega_emu_rom_db_hash_to_string(&hash, crc32_str, sizeof(crc32_str), 0);

            printf("Hash calculado:\n");
            printf("  MD5: %s\n", md5_str);
            printf("  SHA1: %s\n", sha1_str);
            printf("  CRC32: %s\n\n", crc32_str);

            // Procurar ROM pelo hash
            mega_emu_rom_db_entry_t entry;
            if (mega_emu_rom_db_get_by_hash(&hash, &entry)) {
                printf("ROM encontrada no banco de dados!\n");
                print_rom_info(&entry);
            } else {
                printf("ROM não encontrada no banco de dados.\n");

                // Adicionar a ROM ao banco se não existir
                printf("Deseja adicionar esta ROM ao banco de dados? (S/N): ");
                char response;
                scanf(" %c", &response);

                if (response == 'S' || response == 's') {
                    // Preencher informações básicas
                    memset(&entry, 0, sizeof(mega_emu_rom_db_entry_t));
                    memcpy(&entry.hash, &hash, sizeof(mega_emu_rom_db_hash_t));

                    printf("Título: ");
                    scanf(" %127[^\n]", entry.title);

                    printf("Plataforma (1=MD, 2=SMS, 3=GG, 4=NES, 5=SNES, 6=GB, 7=GBC): ");
                    int platform;
                    scanf("%d", &platform);
                    entry.platform = (platform > 0 && platform < ROM_DB_PLATFORM_COUNT) ?
                                     platform : ROM_DB_PLATFORM_UNKNOWN;

                    printf("Região (1=JP, 2=US, 3=EU, 4=BR, 5=KR, 6=CN, 7=World): ");
                    int region;
                    scanf("%d", &region);
                    entry.region = (region > 0 && region < ROM_DB_REGION_COUNT) ?
                                   region : ROM_DB_REGION_UNKNOWN;

                    // Obter tamanho do arquivo
                    FILE *file = fopen(argv[1], "rb");
                    if (file) {
                        fseek(file, 0, SEEK_END);
                        entry.size = ftell(file);
                        fclose(file);
                    }

                    // Adicionar ao banco
                    if (mega_emu_rom_db_add_entry(&entry)) {
                        printf("ROM adicionada com sucesso!\n");
                    } else {
                        printf("Erro ao adicionar ROM.\n");
                    }
                }
            }
        } else {
            printf("Erro ao calcular hash do arquivo.\n");
        }
    }

    // Exemplo de pesquisa
    printf("\nExemplo de pesquisa no banco de dados:\n");
    mega_emu_rom_db_search_t search;
    mega_emu_rom_db_search_result_t result;

    memset(&search, 0, sizeof(mega_emu_rom_db_search_t));

    // Pesquisar por título parcial
    printf("Digite um termo para pesquisar (deixe em branco para listar todos): ");
    char search_term[128];
    scanf(" %127[^\n]", search_term);

    if (search_term[0] != '\0') {
        strncpy(search.title, search_term, sizeof(search.title) - 1);
    }

    // Filtrar por plataforma (opcional)
    printf("Filtrar por plataforma? (0=Não, 1=Sim): ");
    int filter_platform;
    scanf("%d", &filter_platform);

    if (filter_platform == 1) {
        printf("Plataforma (1=MD, 2=SMS, 3=GG, 4=NES, 5=SNES, 6=GB, 7=GBC): ");
        int platform;
        scanf("%d", &platform);

        if (platform > 0 && platform < ROM_DB_PLATFORM_COUNT) {
            search.platform = platform;
            search.use_platform = true;
        }
    }

    // Configurar paginação
    search.items_per_page = 10;
    search.page = 0;

    // Ordenação
    search.sort_by = 0; // Por título
    search.sort_ascending = true;

    // Realizar a pesquisa
    if (mega_emu_rom_db_search(&search, &result)) {
        printf("\nResultados encontrados: %u (mostrando %u)\n\n",
               result.total_matches, result.count);

        for (uint32_t i = 0; i < result.count; i++) {
            printf("%u. %s (%s, %s)\n",
                   i + 1,
                   result.entries[i].title,
                   mega_emu_rom_db_platform_to_string(result.entries[i].platform),
                   mega_emu_rom_db_region_to_string(result.entries[i].region));
        }

        // Se houver mais resultados disponíveis
        if (result.total_matches > result.count) {
            printf("\nMais %u resultados disponíveis.\n", result.total_matches - result.count);
        }

        // Ver detalhes de um item específico
        if (result.count > 0) {
            printf("\nDigite o número do item para ver detalhes (0 para sair): ");
            int item_num;
            scanf("%d", &item_num);

            if (item_num > 0 && item_num <= (int)result.count) {
                print_rom_info(&result.entries[item_num - 1]);
            }
        }

        // Liberar os resultados
        mega_emu_rom_db_free_search_result(&result);
    } else {
        printf("Erro na pesquisa: %s\n", result.error_message);
    }

    // Finalizar
    printf("Finalizando banco de dados...\n");
    mega_emu_rom_db_shutdown();

    return 0;
}
