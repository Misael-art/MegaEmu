#ifndef UNITY_INTERNALS_H
#define UNITY_INTERNALS_H

#include <stddef.h>
#include <stdint.h>

// Definições internas do Unity
#define UNITY_FAIL_AND_BAIL          \
    do                               \
    {                                \
        Unity.CurrentTestFailed = 1; \
        return;                      \
    } while (0)
#define UNITY_TEST_ASSERT(condition, line, message) \
    if (condition)                                  \
    {                                               \
        Unity.CurrentTestFailed = 1;                \
        Unity.CurrentTestLine = line;               \
        return;                                     \
    }

// Estrutura interna do Unity
typedef struct UNITY_STORAGE_T
{
    const char *TestFile;
    const char *CurrentTestName;
    uint16_t CurrentTestLine;
    uint16_t CounterFail;
    uint16_t CounterTest;
    uint8_t CurrentTestIgnored;
    uint8_t CurrentTestFailed;
    uint8_t CurrentDetail1;
    uint8_t CurrentDetail2;
} UNITY_STORAGE_T;

// Variáveis globais internas
extern UNITY_STORAGE_T Unity;

// Funções internas
void UnityPrintNumber(const uint32_t number);
void UnityPrintNumberUnsigned(const uint32_t number);
void UnityPrintNumberHex(const uint32_t number, const uint8_t nibbles);
void UnityPrint(const char *string);
void UnityPrintLen(const char *string, const uint16_t length);
void UnityTestResultsBegin(const char *file, const uint16_t line);
void UnityTestResultsFailBegin(const uint16_t line);
void UnityAddMsgIfSpecified(const char *msg);
void UnityPrintExpectedAndActualStrings(const char *expected, const char *actual);
void UnityPrintExpectedAndActualNumbers(const char *expected, const uint32_t actual);

#endif // UNITY_INTERNALS_H
