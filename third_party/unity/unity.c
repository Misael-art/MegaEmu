/* ==========================================
    Unity Project - A Test Framework for C - Versão Simplificada
    Copyright (c) 2007-14 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
========================================== */

#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* Estrutura para estado global */
struct _Unity Unity = {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, {0}};

/* Buffer para salto entre testes */
jmp_buf Unity_AbortFrame;

/* Mensagens para saída */
const char UnityStrOk[] = "OK";
const char UnityStrPass[] = "PASS";
const char UnityStrFail[] = "FAIL";
const char UnityStrIgnore[] = "IGNORE";
const char UnityStrNull[] = "NULL";
const char UnityStrSpacer[] = ". ";
const char UnityStrExpected[] = " Expected ";
const char UnityStrWas[] = " Was ";
const char UnityStrTo[] = " To ";
const char UnityStrElement[] = " Element ";
const char UnityStrByte[] = " Byte ";
const char UnityStrMemory[] = " Memory Mismatch.";
const char UnityStrDelta[] = " Values Not Within Delta ";
const char UnityStrPointless[] = " You Asked Me To Compare Nothing, Which Was Pointless.";
const char UnityStrNullPointerForExpected[] = " Expected pointer to be NULL";
const char UnityStrNullPointerForActual[] = " Actual pointer was NULL";
const char UnityStrInf[] = "Infinity";
const char UnityStrNegInf[] = "Negative Infinity";
const char UnityStrNaN[] = "NaN";
const char UnityStrDet[] = " Determinate";

/* Funções de inicialização e finalização */
void UnityBegin(const char *filename)
{
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;
}

int UnityEnd(void)
{
    printf("\n");
    printf("-----------------------\n");
    printf("%d Tests %d Failures %d Ignored\n",
           (int)Unity.NumberOfTests,
           (int)Unity.TestFailures,
           (int)Unity.TestIgnores);

    if (Unity.TestFailures == 0)
    {
        printf("OK\n");
    }
    else
    {
        printf("FAIL\n");
    }

    return (int)(Unity.TestFailures);
}

/* Executar um único teste */
void UnityDefaultTestRun(void (*func)(void), const char *funcName, const int lineNumber)
{
    Unity.CurrentTestName = funcName;
    Unity.CurrentTestLineNumber = lineNumber;
    Unity.NumberOfTests++;

    printf("- %s", funcName);

    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;

    /* Configuração do teste */
    setUp();

    /* Executar o teste */
    func();

    /* Limpeza após o teste */
    tearDown();

    /* Atualizar status */
    if (Unity.CurrentTestIgnored)
    {
        printf(" [IGNORE]\n");
        Unity.TestIgnores++;
    }
    else if (Unity.CurrentTestFailed)
    {
        printf(" [FAIL]\n");
        Unity.TestFailures++;
    }
    else
    {
        printf(" [PASS]\n");
    }
}

/* Funções para falha e ignore */
void UnityFail(const char *msg, const unsigned int line)
{
    Unity.CurrentTestFailed = 1;

    printf("\n  At %s:%u: ",
           Unity.TestFile ? Unity.TestFile : "unknown",
           line);

    if (msg != NULL)
    {
        printf("%s", msg);
    }

    printf("\n");
}

void UnityIgnore(const char *msg, const unsigned int line)
{
    Unity.CurrentTestIgnored = 1;

    printf("\n  At %s:%u: ",
           Unity.TestFile ? Unity.TestFile : "unknown",
           line);

    if (msg != NULL)
    {
        printf("%s", msg);
    }

    printf(" [IGNORED]\n");
}

void UnityMessage(const char *msg, const unsigned int line)
{
    printf("\n  At %s:%u: ",
           Unity.TestFile ? Unity.TestFile : "unknown",
           line);

    if (msg != NULL)
    {
        printf("%s", msg);
    }

    printf("\n");
}

/* Funções para checagem de valores */
void UnityAssertEqualNumber(const UNITY_INT expected,
                            const UNITY_INT actual,
                            const char *msg,
                            const unsigned int lineNumber,
                            const UNITY_DISPLAY_STYLE_T style)
{
    if (expected != actual)
    {
        UnityFail(msg, lineNumber);
        printf("  Expected ");

        switch (style)
        {
        case UNITY_DISPLAY_STYLE_INT:
            printf("%d", (int)expected);
            break;
        case UNITY_DISPLAY_STYLE_UINT:
            printf("%u", (unsigned int)expected);
            break;
        case UNITY_DISPLAY_STYLE_HEX8:
            printf("0x%02X", (unsigned int)expected);
            break;
        case UNITY_DISPLAY_STYLE_HEX16:
            printf("0x%04X", (unsigned int)expected);
            break;
        case UNITY_DISPLAY_STYLE_HEX32:
            printf("0x%08X", (unsigned int)expected);
            break;
        default:
            printf("INVALID STYLE");
            break;
        }

        printf(" but was ");

        switch (style)
        {
        case UNITY_DISPLAY_STYLE_INT:
            printf("%d", (int)actual);
            break;
        case UNITY_DISPLAY_STYLE_UINT:
            printf("%u", (unsigned int)actual);
            break;
        case UNITY_DISPLAY_STYLE_HEX8:
            printf("0x%02X", (unsigned int)actual);
            break;
        case UNITY_DISPLAY_STYLE_HEX16:
            printf("0x%04X", (unsigned int)actual);
            break;
        case UNITY_DISPLAY_STYLE_HEX32:
            printf("0x%08X", (unsigned int)actual);
            break;
        default:
            printf("INVALID STYLE");
            break;
        }

        printf("\n");
    }
}

void UnityAssertNotEqualNumber(const UNITY_INT expected,
                               const UNITY_INT actual,
                               const char *msg,
                               const unsigned int lineNumber,
                               const UNITY_DISPLAY_STYLE_T style)
{
    if (expected == actual)
    {
        UnityFail(msg, lineNumber);
        printf("  Should Not Be Equal: ");

        switch (style)
        {
        case UNITY_DISPLAY_STYLE_INT:
            printf("%d", (int)expected);
            break;
        case UNITY_DISPLAY_STYLE_UINT:
            printf("%u", (unsigned int)expected);
            break;
        case UNITY_DISPLAY_STYLE_HEX8:
            printf("0x%02X", (unsigned int)expected);
            break;
        case UNITY_DISPLAY_STYLE_HEX16:
            printf("0x%04X", (unsigned int)expected);
            break;
        case UNITY_DISPLAY_STYLE_HEX32:
            printf("0x%08X", (unsigned int)expected);
            break;
        default:
            printf("INVALID STYLE");
            break;
        }

        printf("\n");
    }
}

void UnityAssertEqualString(const char *expected,
                            const char *actual,
                            const char *msg,
                            const unsigned int lineNumber)
{
    int equal = 1;

    /* Se ambos forem NULL, são iguais */
    if (expected == NULL && actual == NULL)
        return;

    /* Se um for NULL e o outro não, são diferentes */
    if (expected == NULL || actual == NULL)
        equal = 0;
    else
        equal = strcmp(expected, actual) == 0;

    if (!equal)
    {
        UnityFail(msg, lineNumber);
        printf("  Expected \"%s\" but was \"%s\"\n",
               expected ? expected : "(null)",
               actual ? actual : "(null)");
    }
}

void UnityAssertEqualMemory(const void *expected,
                            const void *actual,
                            const UNITY_UINT32 length,
                            const UNITY_UINT32 num_elements,
                            const char *msg,
                            const unsigned int lineNumber)
{
    const unsigned char *ptr_exp = (const unsigned char *)expected;
    const unsigned char *ptr_act = (const unsigned char *)actual;
    UNITY_UINT32 elements = num_elements;
    UNITY_UINT32 bytes;

    if ((elements == 0) || (length == 0))
    {
        UnityFail(msg, lineNumber);
        printf("  Invalid memory comparison parameters\n");
        return;
    }

    if (expected == actual)
        return;
    if (expected == NULL || actual == NULL)
    {
        UnityFail(msg, lineNumber);
        printf("  NULL pointer detected\n");
        return;
    }

    while (elements--)
    {
        bytes = length;
        while (bytes--)
        {
            if (*ptr_exp != *ptr_act)
            {
                UnityFail(msg, lineNumber);
                printf("  Memory mismatch at offset %lu. Expected 0x%02X but was 0x%02X\n",
                       (unsigned long)((ptr_act - (const unsigned char *)actual)),
                       *ptr_exp,
                       *ptr_act);
                return;
            }
            ptr_exp++;
            ptr_act++;
        }
    }
}

/* Funções para arrays */
void UnityAssertEqualIntArray(const UNITY_INT32 *expected,
                              const UNITY_INT32 *actual,
                              const UNITY_UINT32 num_elements,
                              const char *msg,
                              const unsigned int lineNumber,
                              const UNITY_DISPLAY_STYLE_T style)
{
    UNITY_UINT32 i;

    if (num_elements == 0)
    {
        UnityFail(msg, lineNumber);
        printf("  Zero array size\n");
        return;
    }

    if (expected == actual)
        return;
    if (expected == NULL || actual == NULL)
    {
        UnityFail(msg, lineNumber);
        printf("  NULL pointer detected\n");
        return;
    }

    for (i = 0; i < num_elements; i++)
    {
        if (expected[i] != actual[i])
        {
            UnityFail(msg, lineNumber);
            printf("  Array mismatch at index %u. ", (unsigned int)i);

            switch (style)
            {
            case UNITY_DISPLAY_STYLE_INT:
                printf("Expected %d but was %d\n",
                       (int)expected[i],
                       (int)actual[i]);
                break;
            case UNITY_DISPLAY_STYLE_UINT:
                printf("Expected %u but was %u\n",
                       (unsigned int)expected[i],
                       (unsigned int)actual[i]);
                break;
            case UNITY_DISPLAY_STYLE_HEX8:
                printf("Expected 0x%02X but was 0x%02X\n",
                       (unsigned int)expected[i],
                       (unsigned int)actual[i]);
                break;
            case UNITY_DISPLAY_STYLE_HEX16:
                printf("Expected 0x%04X but was 0x%04X\n",
                       (unsigned int)expected[i],
                       (unsigned int)actual[i]);
                break;
            case UNITY_DISPLAY_STYLE_HEX32:
                printf("Expected 0x%08X but was 0x%08X\n",
                       (unsigned int)expected[i],
                       (unsigned int)actual[i]);
                break;
            default:
                printf("INVALID STYLE\n");
                break;
            }
            return;
        }
    }
}

/* Para valores que não sejam exatamente iguais, mas estejam dentro de um delta */
void UnityAssertNumbersWithin(const UNITY_UINT delta,
                              const UNITY_INT expected,
                              const UNITY_INT actual,
                              const char *msg,
                              const unsigned int lineNumber,
                              const UNITY_DISPLAY_STYLE_T style)
{
    UNITY_INT diff = actual - expected;
    UNITY_UINT positiveDiff = (diff < 0) ? (UNITY_UINT)(-diff) : (UNITY_UINT)diff;

    if (positiveDiff > delta)
    {
        UnityFail(msg, lineNumber);
        printf("  Values not within delta. ");

        switch (style)
        {
        case UNITY_DISPLAY_STYLE_INT:
            printf("Expected %d +/- %u but was %d\n",
                   (int)expected,
                   (unsigned int)delta,
                   (int)actual);
            break;
        case UNITY_DISPLAY_STYLE_UINT:
            printf("Expected %u +/- %u but was %u\n",
                   (unsigned int)expected,
                   (unsigned int)delta,
                   (unsigned int)actual);
            break;
        case UNITY_DISPLAY_STYLE_HEX8:
            printf("Expected 0x%02X +/- 0x%02X but was 0x%02X\n",
                   (unsigned int)expected,
                   (unsigned int)delta,
                   (unsigned int)actual);
            break;
        case UNITY_DISPLAY_STYLE_HEX16:
            printf("Expected 0x%04X +/- 0x%04X but was 0x%04X\n",
                   (unsigned int)expected,
                   (unsigned int)delta,
                   (unsigned int)actual);
            break;
        case UNITY_DISPLAY_STYLE_HEX32:
            printf("Expected 0x%08X +/- 0x%08X but was 0x%08X\n",
                   (unsigned int)expected,
                   (unsigned int)delta,
                   (unsigned int)actual);
            break;
        default:
            printf("INVALID STYLE\n");
            break;
        }
    }
}

/* Funções de suporte */
void UnityAssertBits(const UNITY_INT mask,
                     const UNITY_INT expected,
                     const UNITY_INT actual,
                     const char *msg,
                     const unsigned int lineNumber)
{
    if ((mask & expected) != (mask & actual))
    {
        UnityFail(msg, lineNumber);
        printf("  Bits do not match. Mask 0x%08X, Expected 0x%08X, Actual 0x%08X\n",
               (unsigned int)mask,
               (unsigned int)expected,
               (unsigned int)actual);
    }
}

/* Inicialização padrão das funções de setup e teardown */
void setUp(void) {}
void tearDown(void) {}
