/**
 * @file validation_utils.h
 * @brief Utilitários para validação de parâmetros e ponteiros
 */
#ifndef VALIDATION_UTILS_H
#define VALIDATION_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "log_utils.h"

/**
 * @brief Macro para verificar se um ponteiro é válido e retornar vazio se não
 * for
 *
 * @param ptr Ponteiro a verificar
 * @param msg Mensagem de erro a exibir
 */
#define CHECK_NULL_RETURN_VOID(ptr, msg)                                       \
  do {                                                                         \
    if (!(ptr)) {                                                              \
      LOG_ERROR("Ponteiro nulo detectado: %s", (msg));                         \
      return;                                                                  \
    }                                                                          \
  } while (0)

/**
 * @brief Macro para verificar se um ponteiro é válido e retornar false se não
 * for
 *
 * @param ptr Ponteiro a verificar
 * @param msg Mensagem de erro a exibir
 */
#define CHECK_NULL_RETURN_FALSE(ptr, msg)                                      \
  do {                                                                         \
    if (!(ptr)) {                                                              \
      LOG_ERROR("Ponteiro nulo detectado: %s", (msg));                         \
      return false;                                                            \
    }                                                                          \
  } while (0)

/**
 * @brief Macro para verificar se um ponteiro é válido e retornar zero se não
 * for
 *
 * @param ptr Ponteiro a verificar
 * @param msg Mensagem de erro a exibir
 */
#define CHECK_NULL_RETURN_ZERO(ptr, msg)                                       \
  do {                                                                         \
    if (!(ptr)) {                                                              \
      LOG_ERROR("Ponteiro nulo detectado: %s", (msg));                         \
      return 0;                                                                \
    }                                                                          \
  } while (0)

/**
 * @brief Macro para validar uma condição e retornar vazio se for falsa
 *
 * @param condition Condição a validar
 * @param msg Mensagem de erro a exibir
 */
#define VALIDATE_PARAM_RETURN_VOID(condition, msg, ...)                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      LOG_ERROR("Parâmetro inválido: " msg, ##__VA_ARGS__);                    \
      return;                                                                  \
    }                                                                          \
  } while (0)

/**
 * @brief Macro para validar uma condição e retornar false se for falsa
 *
 * @param condition Condição a validar
 * @param msg Mensagem de erro a exibir
 */
#define VALIDATE_PARAM_RETURN_FALSE(condition, msg, ...)                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      LOG_ERROR("Parâmetro inválido: " msg, ##__VA_ARGS__);                    \
      return false;                                                            \
    }                                                                          \
  } while (0)

/**
 * @brief Macro para verificar se um ponteiro é válido e retornar um valor se
 * não for
 *
 * @param ptr Ponteiro a verificar
 * @param ret_val Valor a retornar se o ponteiro for nulo
 * @param msg Mensagem de erro a exibir
 */
#define CHECK_NULL_RETURN(ptr, ret_val, msg)                                   \
  do {                                                                         \
    if (!(ptr)) {                                                              \
      LOG_ERROR("Ponteiro nulo detectado: %s", (msg));                         \
      return (ret_val);                                                        \
    }                                                                          \
  } while (0)

/**
 * @brief Macro para validar uma condição e retornar um valor se for falsa
 *
 * @param condition Condição a validar
 * @param ret_val Valor a retornar se a condição for falsa
 * @param msg Mensagem de erro a exibir
 */
#define VALIDATE_PARAM_RETURN(condition, ret_val, msg, ...)                    \
  do {                                                                         \
    if (!(condition)) {                                                        \
      LOG_ERROR("Parâmetro inválido: " msg, ##__VA_ARGS__);                    \
      return (ret_val);                                                        \
    }                                                                          \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif /* VALIDATION_UTILS_H */
