#include "unity.h"
#include <stdio.h>

Unity_t Unity = {0};

void UnityBegin(void)
{
    Unity.TestCount = 0;
    Unity.TestFailed = 0;
    printf("\n-----------------------\n");
    printf("Iniciando testes\n");
    printf("-----------------------\n");
}

void UnityEnd(void)
{
    printf("\n-----------------------\n");
    printf("Testes concluídos\n");
    printf("Total de testes: %d\n", Unity.TestCount);
    printf("Falhas: %d\n", Unity.TestFailed);
    printf("-----------------------\n");
}

void UnityDefaultTestRun(void (*func)(void))
{
    Unity.TestFailed = 0;
    Unity.TestCount++;
    func();
}
