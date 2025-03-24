/**
 * @file test_mappers.c
 * @brief Arquivo principal de testes dos mappers NES
 */

#include <unity.h>
#include <stdio.h>

// Declarações externas das suítes de teste
extern nes_test_suite_t mmc3_test_suite;
extern nes_test_suite_t mmc5_test_suite;

// Array com todas as suítes de testes
static nes_test_suite_t *test_suites[] = {
    &mmc3_test_suite,
    &mmc5_test_suite
};

// Declarações das funções de teste
void test_mmc3_init(void);
void test_mmc3_prg_read(void);
void test_mmc3_chr_read(void);
void test_mmc3_bank_switching(void);

void test_mmc5_init(void);
void test_mmc5_prg_read(void);
void test_mmc5_chr_read(void);
void test_mmc5_bank_switching(void);

// Função principal que executa todos os testes
int main(void) {
    UNITY_BEGIN();

    // Testes do MMC3
    printf("\nExecutando testes do MMC3...\n");
    RUN_TEST(test_mmc3_init);
    RUN_TEST(test_mmc3_prg_read);
    RUN_TEST(test_mmc3_chr_read);
    RUN_TEST(test_mmc3_bank_switching);

    // Testes do MMC5
    printf("\nExecutando testes do MMC5...\n");
    RUN_TEST(test_mmc5_init);
    RUN_TEST(test_mmc5_prg_read);
    RUN_TEST(test_mmc5_chr_read);
    RUN_TEST(test_mmc5_bank_switching);

    return UNITY_END();
}
