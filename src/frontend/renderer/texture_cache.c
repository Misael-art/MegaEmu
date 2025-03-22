/** * @file texture_cache.c * @brief Implementação do sistema de cache de texturas */#include "texture_cache.h"#include <stdio.h>#include <stdlib.h>#include <string.h>// Definição de constantes#define DEFAULT_MAX_ENTRIES 64#define MAX_KEY_LENGTH 128// Estrutura para uma entrada no cachetypedef struct{    char key[MAX_KEY_LENGTH];    SDL_Texture *texture;    uint32_t last_access;    uint32_t creation_time;} cache_entry_t;// Estrutura global para o sistema de cachestatic struct{    SDL_Renderer *renderer;    cache_entry_t *entries;    int max_entries;    int entry_count;    uint32_t access_counter;    bool initialized;} g_texture_cache = {0};/** * @brief Encontra uma entrada no cache pela chave */static int find_entry(const char *key){    if (!g_texture_cache.initialized || !key)    {        return -1;    }    for (int i = 0; i < g_texture_cache.entry_count; i++)    {        if (strcmp(g_texture_cache.entries[i].key, key) == 0)        {            return i;        }    }    return -1;}/** * @brief Encontra o índice da entrada menos recentemente utilizada */static int find_lru_entry(void){    if (!g_texture_cache.initialized || g_texture_cache.entry_count == 0)    {        return -1;    }    int lru_index = 0;    uint32_t oldest_access = g_texture_cache.entries[0].last_access;    for (int i = 1; i < g_texture_cache.entry_count; i++)    {        if (g_texture_cache.entries[i].last_access < oldest_access)        {            oldest_access = g_texture_cache.entries[i].last_access;            lru_index = i;        }    }    return lru_index;}/** * @brief Inicializa o sistema de cache de texturas */bool texture_cache_init(SDL_Renderer *renderer, int max_entries){    if (!renderer)    {        return false;    }    // Limpar o estado anterior, se existir    if (g_texture_cache.initialized)    {        texture_cache_shutdown();    }    // Inicializar o estado global    memset(&g_texture_cache, 0, sizeof(g_texture_cache));    g_texture_cache.renderer = renderer;    g_texture_cache.max_entries = max_entries > 0 ? max_entries : DEFAULT_MAX_ENTRIES;    g_texture_cache.entries = (cache_entry_t *)calloc(g_texture_cache.max_entries, sizeof(cache_entry_t));    if (!g_texture_cache.entries)    {        return false;    }    g_texture_cache.entry_count = 0;    g_texture_cache.access_counter = 0;    g_texture_cache.initialized = true;    return true;}/** * @brief Finaliza o sistema de cache de texturas e libera recursos */void texture_cache_shutdown(void){    if (!g_texture_cache.initialized)    {        return;    }    // Limpar todas as texturas    texture_cache_clear();    // Liberar o array de entradas    free(g_texture_cache.entries);    g_texture_cache.entries = NULL;    // Resetar o estado    g_texture_cache.initialized = false;    g_texture_cache.entry_count = 0;    g_texture_cache.renderer = NULL;}/** * @brief Adiciona uma textura ao cache */bool texture_cache_add(const char *key, SDL_Texture *texture){    if (!g_texture_cache.initialized || !key || !texture)    {        return false;    }    // Verificar se a chave já existe    int index = find_entry(key);    if (index >= 0)    {        // Substituir a textura existente        SDL_DestroyTexture(g_texture_cache.entries[index].texture);        g_texture_cache.entries[index].texture = texture;        g_texture_cache.entries[index].last_access = g_texture_cache.access_counter++;        g_texture_cache.entries[index].creation_time = SDL_GetTicks();        return true;    }    // Verificar se o cache está cheio    if (g_texture_cache.entry_count >= g_texture_cache.max_entries)    {        // Encontrar e substituir a entrada menos recentemente utilizada        index = find_lru_entry();        if (index < 0)        {            return false;        }        // Liberar a textura antiga        SDL_DestroyTexture(g_texture_cache.entries[index].texture);        // Substituir a entrada        strncpy(g_texture_cache.entries[index].key, key, MAX_KEY_LENGTH - 1);        g_texture_cache.entries[index].key[MAX_KEY_LENGTH - 1] = '\0';        g_texture_cache.entries[index].texture = texture;        g_texture_cache.entries[index].last_access = g_texture_cache.access_counter++;        g_texture_cache.entries[index].creation_time = SDL_GetTicks();    }    else    {        // Adicionar nova entrada        strncpy(g_texture_cache.entries[g_texture_cache.entry_count].key, key, MAX_KEY_LENGTH - 1);        g_texture_cache.entries[g_texture_cache.entry_count].key[MAX_KEY_LENGTH - 1] = '\0';        g_texture_cache.entries[g_texture_cache.entry_count].texture = texture;        g_texture_cache.entries[g_texture_cache.entry_count].last_access = g_texture_cache.access_counter++;        g_texture_cache.entries[g_texture_cache.entry_count].creation_time = SDL_GetTicks();        g_texture_cache.entry_count++;    }    return true;}/** * @brief Obtém uma textura do cache */SDL_Texture *texture_cache_get(const char *key){    if (!g_texture_cache.initialized || !key)    {        return NULL;    }    int index = find_entry(key);    if (index < 0)    {        return NULL;    }    // Atualizar a contagem de acesso    g_texture_cache.entries[index].last_access = g_texture_cache.access_counter++;    return g_texture_cache.entries[index].texture;}/** * @brief Remove uma textura do cache */bool texture_cache_remove(const char *key){    if (!g_texture_cache.initialized || !key)    {        return false;    }    int index = find_entry(key);    if (index < 0)    {        return false;    }    // Liberar a textura    SDL_DestroyTexture(g_texture_cache.entries[index].texture);    // Remover a entrada movendo as entradas subsequentes    if (index < g_texture_cache.entry_count - 1)    {        memmove(&g_texture_cache.entries[index],                &g_texture_cache.entries[index + 1],                (g_texture_cache.entry_count - index - 1) * sizeof(cache_entry_t));    }    g_texture_cache.entry_count--;    return true;}/** * @brief Limpa todas as texturas do cache */void texture_cache_clear(void){    if (!g_texture_cache.initialized)    {        return;    }    // Liberar todas as texturas    for (int i = 0; i < g_texture_cache.entry_count; i++)    {        if (g_texture_cache.entries[i].texture)        {            SDL_DestroyTexture(g_texture_cache.entries[i].texture);            g_texture_cache.entries[i].texture = NULL;        }    }    g_texture_cache.entry_count = 0;}/** * @brief Configura o tamanho máximo do cache */void texture_cache_set_max_entries(int max_entries){    if (!g_texture_cache.initialized || max_entries <= 0)    {        return;    }    // Se o novo tamanho for menor que o atual, redimensionar o array    if (max_entries < g_texture_cache.max_entries)    {        // Remover entradas excedentes usando LRU        while (g_texture_cache.entry_count > max_entries)        {            int lru_index = find_lru_entry();            if (lru_index >= 0)            {                texture_cache_remove(g_texture_cache.entries[lru_index].key);            }        }        // Redimensionar o array        cache_entry_t *new_entries = (cache_entry_t *)realloc(g_texture_cache.entries,                                                              max_entries * sizeof(cache_entry_t));        if (!new_entries)        {            return;        }        g_texture_cache.entries = new_entries;    }    else if (max_entries > g_texture_cache.max_entries)    {        // Aumentar o tamanho do array        cache_entry_t *new_entries = (cache_entry_t *)realloc(g_texture_cache.entries,                                                              max_entries * sizeof(cache_entry_t));        if (!new_entries)        {            return;        }        g_texture_cache.entries = new_entries;    }    g_texture_cache.max_entries = max_entries;}/** * @brief Obtém o número atual de texturas no cache */int texture_cache_get_size(void){    if (!g_texture_cache.initialized)    {        return 0;    }    return g_texture_cache.entry_count;}