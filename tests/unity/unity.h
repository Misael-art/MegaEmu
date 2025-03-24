/**
 * @file unity.h
 * @brief Framework de testes unitários simplificado
 */

#ifndef UNITY_H
#define UNITY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

    // Estrutura para controle dos testes
    typedef struct
    {
        int TestFailed;
        int TestCount;
        int FailureCount;
    } Unity_t;

    extern Unity_t Unity;

    // Funções de inicialização e finalização
    void UnityBegin(void);
    void UnityEnd(void);
    void UnityDefaultTestRun(void (*func)(void));

// Macros principais
#define UNITY_BEGIN() UnityBegin()
#define UNITY_END() UnityEnd()
#define RUN_TEST(func) UnityDefaultTestRun(func)

// Macros para testes básicos
#define TEST_ASSERT(condition) Unity.TestFailed = !(condition)
#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))
#define TEST_ASSERT_NULL(pointer) TEST_ASSERT(pointer == NULL)
#define TEST_ASSERT_NOT_NULL(pointer) TEST_ASSERT(pointer != NULL)

// Macros para testes de igualdade
#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    TEST_ASSERT((expected) == (actual))

#define TEST_ASSERT_EQUAL_UINT8(expected, actual) \
    TEST_ASSERT((uint8_t)(expected) == (uint8_t)(actual))

#define TEST_ASSERT_EQUAL_UINT32(expected, actual) \
    TEST_ASSERT((uint32_t)(expected) == (uint32_t)(actual))

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    TEST_ASSERT((expected) != (actual))

#ifdef __cplusplus
}
#endif

#endif // UNITY_H
