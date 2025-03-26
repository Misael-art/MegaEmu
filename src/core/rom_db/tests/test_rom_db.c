/**
 * @file test_rom_db.c
 * @brief Testes automatizados para o banco de dados de ROMs
 */

#include "../rom_db.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Função auxiliar para criar dados de teste
static void setup_test_entry(mega_emu_rom_db_entry_t *entry) {
  memset(entry, 0, sizeof(mega_emu_rom_db_entry_t));

  strcpy(entry->title, "Teste ROM");
  strcpy(entry->alt_title, "Test ROM");
  strcpy(entry->developer, "Mega_Emu Test Team");
  strcpy(entry->publisher, "Mega_Emu");
  strcpy(entry->release_date, "2024-01-01");

  entry->platform = ROM_DB_PLATFORM_MEGADRIVE;
  entry->region = ROM_DB_REGION_USA;
  entry->compatibility = ROM_DB_COMPAT_PERFECT;
  entry->media_type = ROM_DB_MEDIA_CARTRIDGE;
  entry->genre = ROM_DB_GENRE_ACTION;
  entry->input_type = ROM_DB_INPUT_GAMEPAD;

  strcpy(entry->description,
         "ROM de teste para verificar o funcionamento do banco de dados.");

  // Gerar hashes de teste
  for (int i = 0; i < 16; i++)
    entry->hash.md5[i] = i + 1;
  for (int i = 0; i < 20; i++)
    entry->hash.sha1[i] = i + 1;
  for (int i = 0; i < 4; i++)
    entry->hash.crc32[i] = i + 1;

  entry->size = 1024 * 1024; // 1MB
  entry->players = 1;

  strcpy(entry->serial, "TEST-01");
  strcpy(entry->version, "1.0");
  strcpy(entry->save_type, "SRAM");
  entry->has_battery = true;

  entry->flags = 0;
  strcpy(entry->extra_data, "{\"test\": true}");

  entry->db_revision = 1;
  strcpy(entry->added_date, "2024-03-26");
  strcpy(entry->updated_date, "2024-03-26");
}

// Função auxiliar para comparar duas entradas
static bool compare_entries(const mega_emu_rom_db_entry_t *entry1,
                            const mega_emu_rom_db_entry_t *entry2) {
  return strcmp(entry1->title, entry2->title) == 0 &&
         entry1->platform == entry2->platform &&
         entry1->region == entry2->region &&
         mega_emu_rom_db_compare_hash(&entry1->hash, &entry2->hash,
                                      1); // Comparar MD5
}

// Teste de inicialização e finalização
static void test_init_shutdown() {
  printf("Teste: Inicialização e finalização... ");

  // Usar um banco de dados temporário para testes
  assert(mega_emu_rom_db_init(":memory:"));
  assert(mega_emu_rom_db_is_initialized());

  // Obter metadados
  mega_emu_rom_db_metadata_t metadata;
  assert(mega_emu_rom_db_get_metadata(&metadata));

  // Finalizar
  mega_emu_rom_db_shutdown();
  assert(!mega_emu_rom_db_is_initialized());

  printf("OK\n");
}

// Teste de adição e obtenção de entradas
static void test_add_get_entry() {
  printf("Teste: Adição e obtenção de entradas... ");

  // Inicializar com banco em memória
  assert(mega_emu_rom_db_init(":memory:"));

  // Criar entrada de teste
  mega_emu_rom_db_entry_t test_entry;
  setup_test_entry(&test_entry);

  // Adicionar ao banco
  assert(mega_emu_rom_db_add_entry(&test_entry));

  // Recuperar pelo hash
  mega_emu_rom_db_entry_t retrieved_entry;
  assert(mega_emu_rom_db_get_by_hash(&test_entry.hash, &retrieved_entry));

  // Verificar se os dados são iguais
  assert(compare_entries(&test_entry, &retrieved_entry));

  // Recuperar pelo ID
  mega_emu_rom_db_entry_t retrieved_by_id;
  assert(mega_emu_rom_db_get_by_id(retrieved_entry.id, &retrieved_by_id));

  // Verificar se os dados são iguais
  assert(compare_entries(&test_entry, &retrieved_by_id));

  mega_emu_rom_db_shutdown();
  printf("OK\n");
}

// Teste de atualização de entradas
static void test_update_entry() {
  printf("Teste: Atualização de entradas... ");

  // Inicializar com banco em memória
  assert(mega_emu_rom_db_init(":memory:"));

  // Criar e adicionar entrada de teste
  mega_emu_rom_db_entry_t test_entry;
  setup_test_entry(&test_entry);
  assert(mega_emu_rom_db_add_entry(&test_entry));

  // Recuperar para obter o ID
  mega_emu_rom_db_entry_t retrieved_entry;
  assert(mega_emu_rom_db_get_by_hash(&test_entry.hash, &retrieved_entry));

  // Modificar alguns campos
  strcpy(retrieved_entry.title, "Teste ROM Atualizado");
  retrieved_entry.compatibility = ROM_DB_COMPAT_PLAYABLE;

  // Atualizar no banco
  assert(mega_emu_rom_db_update_entry(&retrieved_entry));

  // Recuperar novamente
  mega_emu_rom_db_entry_t updated_entry;
  assert(mega_emu_rom_db_get_by_id(retrieved_entry.id, &updated_entry));

  // Verificar as alterações
  assert(strcmp(updated_entry.title, "Teste ROM Atualizado") == 0);
  assert(updated_entry.compatibility == ROM_DB_COMPAT_PLAYABLE);

  mega_emu_rom_db_shutdown();
  printf("OK\n");
}

// Teste de remoção de entradas
static void test_remove_entry() {
  printf("Teste: Remoção de entradas... ");

  // Inicializar com banco em memória
  assert(mega_emu_rom_db_init(":memory:"));

  // Criar e adicionar entrada de teste
  mega_emu_rom_db_entry_t test_entry;
  setup_test_entry(&test_entry);
  assert(mega_emu_rom_db_add_entry(&test_entry));

  // Recuperar para obter o ID
  mega_emu_rom_db_entry_t retrieved_entry;
  assert(mega_emu_rom_db_get_by_hash(&test_entry.hash, &retrieved_entry));

  // Remover do banco
  assert(mega_emu_rom_db_remove_entry(retrieved_entry.id));

  // Tentar recuperar novamente (deve falhar)
  mega_emu_rom_db_entry_t removed_entry;
  assert(!mega_emu_rom_db_get_by_id(retrieved_entry.id, &removed_entry));

  mega_emu_rom_db_shutdown();
  printf("OK\n");
}

// Teste de pesquisa
static void test_search() {
  printf("Teste: Pesquisa... ");

  // Inicializar com banco em memória
  assert(mega_emu_rom_db_init(":memory:"));

  // Adicionar várias entradas
  for (int i = 0; i < 5; i++) {
    mega_emu_rom_db_entry_t entry;
    setup_test_entry(&entry);

    char title[64];
    snprintf(title, sizeof(title), "Teste ROM %d", i + 1);
    strcpy(entry.title, title);

    // Alternar plataformas
    entry.platform =
        (i % 2 == 0) ? ROM_DB_PLATFORM_MEGADRIVE : ROM_DB_PLATFORM_NES;

    // Gerar hash único
    for (int j = 0; j < 16; j++)
      entry.hash.md5[j] = i * 16 + j;
    for (int j = 0; j < 20; j++)
      entry.hash.sha1[j] = i * 20 + j;
    for (int j = 0; j < 4; j++)
      entry.hash.crc32[j] = i * 4 + j;

    assert(mega_emu_rom_db_add_entry(&entry));
  }

  // Pesquisar por todos
  mega_emu_rom_db_search_t search;
  mega_emu_rom_db_search_result_t result;

  memset(&search, 0, sizeof(search));
  strcpy(search.title, "Teste");

  assert(mega_emu_rom_db_search(&search, &result));
  assert(result.count == 5);
  assert(result.total_matches == 5);

  mega_emu_rom_db_free_search_result(&result);

  // Pesquisar com filtro de plataforma
  memset(&search, 0, sizeof(search));
  strcpy(search.title, "Teste");
  search.platform = ROM_DB_PLATFORM_MEGADRIVE;
  search.use_platform = true;

  assert(mega_emu_rom_db_search(&search, &result));
  assert(result.count == 3); // Apenas as entradas com Mega Drive

  mega_emu_rom_db_free_search_result(&result);

  // Pesquisar com paginação
  memset(&search, 0, sizeof(search));
  strcpy(search.title, "Teste");
  search.items_per_page = 2;
  search.page = 0;

  assert(mega_emu_rom_db_search(&search, &result));
  assert(result.count == 2);
  assert(result.total_matches == 5);

  mega_emu_rom_db_free_search_result(&result);

  mega_emu_rom_db_shutdown();
  printf("OK\n");
}

// Teste de funções de utilidade
static void test_utility_functions() {
  printf("Teste: Funções de utilidade... ");

  // Conversão de plataforma
  assert(mega_emu_rom_db_string_to_platform("Mega Drive") ==
         ROM_DB_PLATFORM_MEGADRIVE);
  assert(mega_emu_rom_db_string_to_platform("Genesis") ==
         ROM_DB_PLATFORM_MEGADRIVE);
  assert(mega_emu_rom_db_string_to_platform("NES") == ROM_DB_PLATFORM_NES);

  // Conversão de região
  assert(mega_emu_rom_db_string_to_region("Japan") == ROM_DB_REGION_JAPAN);
  assert(mega_emu_rom_db_string_to_region("USA") == ROM_DB_REGION_USA);
  assert(mega_emu_rom_db_string_to_region("Europe") == ROM_DB_REGION_EUROPE);

  // Conversão de gênero
  assert(mega_emu_rom_db_string_to_genre("Action") == ROM_DB_GENRE_ACTION);
  assert(mega_emu_rom_db_string_to_genre("RPG") == ROM_DB_GENRE_RPG);
  assert(mega_emu_rom_db_string_to_genre("Platformer") ==
         ROM_DB_GENRE_PLATFORMER);

  // Hash para string
  mega_emu_rom_db_hash_t hash;
  for (int i = 0; i < 16; i++)
    hash.md5[i] = i;

  char hash_str[33];
  assert(mega_emu_rom_db_hash_to_string(&hash, hash_str, sizeof(hash_str), 1));
  assert(strlen(hash_str) == 32);

  // String para hash
  mega_emu_rom_db_hash_t converted_hash;
  assert(mega_emu_rom_db_string_to_hash(hash_str, &converted_hash, 1));

  // Verificar se os hashes são iguais
  assert(mega_emu_rom_db_compare_hash(&hash, &converted_hash, 1));

  printf("OK\n");
}

int main() {
  printf("Iniciando testes do banco de dados de ROMs...\n\n");

  test_init_shutdown();
  test_add_get_entry();
  test_update_entry();
  test_remove_entry();
  test_search();
  test_utility_functions();

  printf("\nTodos os testes concluídos com sucesso!\n");
  return 0;
}
