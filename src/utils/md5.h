/**
 * @file md5.h
 * @brief Implementação simples do algoritmo MD5
 */

#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Contexto para cálculo do MD5
 */
typedef struct {
    uint32_t state[4];  /**< Estado do hash */
    uint32_t count[2];  /**< Número de bits processados */
    uint8_t buffer[64]; /**< Buffer de entrada */
} MD5_CTX;

/**
 * @brief Inicializa o contexto MD5
 *
 * @param context Contexto a ser inicializado
 */
void MD5_Init(MD5_CTX *context);

/**
 * @brief Atualiza o hash com mais dados
 *
 * @param context Contexto MD5
 * @param input Dados de entrada
 * @param inputLen Tamanho dos dados de entrada
 */
void MD5_Update(MD5_CTX *context, const void *input, size_t inputLen);

/**
 * @brief Finaliza o cálculo do hash
 *
 * @param digest Buffer para receber o hash (16 bytes)
 * @param context Contexto MD5
 */
void MD5_Final(uint8_t digest[16], MD5_CTX *context);

#ifdef __cplusplus
}
#endif

#endif /* MD5_H */
