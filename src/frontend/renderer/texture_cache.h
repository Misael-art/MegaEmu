/**
 * @file texture_cache.h
 * @brief Sistema de cache de texturas para otimizar a renderização
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2024-07-22
 */

#ifndef TEXTURE_CACHE_H
#define TEXTURE_CACHE_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Estrutura de dados para o cache de texturas
     */
    typedef struct texture_cache
    {
        SDL_Renderer *renderer;
        int max_entries;
        int current_entries;
        void *cache_data; /* Ponteiro para os dados internos do cache */
    } texture_cache_t;

    /**
     * @brief Inicializa o sistema de cache de texturas
     *
     * @param renderer Ponteiro para o renderer SDL
     * @param max_entries Número máximo de entradas no cache (0 para padrão)
     * @return true se a inicialização foi bem-sucedida, false caso contrário
     */
    bool texture_cache_init(SDL_Renderer *renderer, int max_entries);

    /**
     * @brief Finaliza o sistema de cache de texturas e libera recursos
     */
    void texture_cache_shutdown(void);

    /**
     * @brief Adiciona uma textura ao cache
     *
     * @param key Chave para identificar a textura
     * @param texture Textura a ser armazenada
     * @return true se foi adicionada com sucesso, false caso contrário
     */
    bool texture_cache_add(const char *key, SDL_Texture *texture);

    /**
     * @brief Obtém uma textura do cache
     *
     * @param key Chave da textura
     * @return SDL_Texture* Ponteiro para a textura, ou NULL se não encontrada
     */
    SDL_Texture *texture_cache_get(const char *key);

    /**
     * @brief Remove uma textura do cache
     *
     * @param key Chave da textura
     * @return true se removida com sucesso, false caso contrário
     */
    bool texture_cache_remove(const char *key);

    /**
     * @brief Limpa todas as texturas do cache
     */
    void texture_cache_clear(void);

    /**
     * @brief Configura o tamanho máximo do cache
     *
     * @param max_entries Número máximo de entradas
     */
    void texture_cache_set_max_entries(int max_entries);

    /**
     * @brief Obtém o número atual de texturas no cache
     *
     * @return int Número de texturas
     */
    int texture_cache_get_size(void);

#ifdef __cplusplus
}
#endif

#endif /* TEXTURE_CACHE_H */
