/* ==========================================
    Unity Project - A Test Framework for C - Versão Simplificada
    Copyright (c) 2007-14 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
========================================== */

#ifndef UNITY_FRAMEWORK_H
#define UNITY_FRAMEWORK_H

#define UNITY

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Macros para testes */
#define TEST_ASSERT_EQUAL_INT(expected, actual) ((expected) == (actual))
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) ((expected) == (actual))
#define TEST_ASSERT_EQUAL_STRING(expected, actual) (strcmp((expected), (actual)) == 0)
#define TEST_ASSERT_EQUAL(expected, actual) ((expected) == (actual))
#define TEST_ASSERT_NOT_NULL(ptr) ((ptr) != NULL)
#define UNITY_BEGIN() 0
#define UNITY_END() 0
#define RUN_TEST(func) func()

/* Definições de coisas necessárias para o teste */
typedef enum
{
    SAVE_STATE_OK = 0,
    SAVE_STATE_ERROR = -1
} save_state_result_t;

/* Funções de setup e teardown que os testes podem implementar */
void setUp(void);
void tearDown(void);

#endif /* UNITY_FRAMEWORK_H */
