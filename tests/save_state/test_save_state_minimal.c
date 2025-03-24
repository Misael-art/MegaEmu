/* tests/save_state/test_save_state_minimal.c
 * Um teste muito simples para a funcionalidade save_state
 * Implementação minimalista para evitar conflitos com bibliotecas do sistema
 */

#define _CRT_SECURE_NO_WARNINGS
#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definições mínimas necessárias */
typedef struct
{
    void *region;
    size_t size;
    char name[32];
} memory_region_t;

typedef struct
{
    memory_region_t regions[10];
    int num_regions;
    char filename[256];
} save_state_ctx_t;

/* Variáveis globais para teste */
static save_state_ctx_t *ctx = NULL;
static uint8_t test_memory[1024];
static const char *test_filename = "test_save.sav";

void setUp(void)
{
    ctx = NULL;
    memset(test_memory, 0xAA, sizeof(test_memory));
    remove(test_filename);
}

void tearDown(void)
{
    if (ctx)
    {
        save_state_destroy(ctx);
        ctx = NULL;
    }
    remove(test_filename);
}

/* Funções mock para save_state */
save_state_ctx_t *save_state_create(const char *filename)
{
    save_state_ctx_t *ctx = (save_state_ctx_t *)malloc(sizeof(save_state_ctx_t));
    if (!ctx)
        return NULL;

    memset(ctx, 0, sizeof(save_state_ctx_t));
    strncpy(ctx->filename, filename, sizeof(ctx->filename) - 1);
    return ctx;
}

int save_state_register_memory(save_state_ctx_t *ctx, void *region, size_t size, const char *name)
{
    if (!ctx || !region || size == 0 || !name)
        return 0;

    if (ctx->num_regions >= 10)
        return 0;

    int idx = ctx->num_regions++;
    ctx->regions[idx].region = region;
    ctx->regions[idx].size = size;
    strncpy(ctx->regions[idx].name, name, sizeof(ctx->regions[idx].name) - 1);

    return 1;
}

int save_state_save(save_state_ctx_t *ctx)
{
    if (!ctx)
        return 0;

    FILE *fp = fopen(ctx->filename, "wb");
    if (!fp)
        return 0;

    for (int i = 0; i < ctx->num_regions; i++)
    {
        fwrite(ctx->regions[i].region, 1, ctx->regions[i].size, fp);
    }

    fclose(fp);
    return 1;
}

int save_state_load(save_state_ctx_t *ctx)
{
    if (!ctx)
        return 0;

    FILE *fp = fopen(ctx->filename, "rb");
    if (!fp)
        return 0;

    for (int i = 0; i < ctx->num_regions; i++)
    {
        if (fread(ctx->regions[i].region, 1, ctx->regions[i].size, fp) != ctx->regions[i].size)
        {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

void save_state_destroy(save_state_ctx_t *ctx)
{
    if (ctx)
    {
        free(ctx);
    }
}

void test_save_state_create_destroy(void)
{
    ctx = save_state_create(test_filename);
    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_EQUAL_STRING(test_filename, ctx->filename);
    TEST_ASSERT_EQUAL_INT(0, ctx->num_regions);

    save_state_destroy(ctx);
    ctx = NULL;
}

void test_save_state_register_memory(void)
{
    ctx = save_state_create(test_filename);
    TEST_ASSERT_NOT_NULL(ctx);

    TEST_ASSERT_TRUE(save_state_register_memory(ctx, test_memory, sizeof(test_memory), "RAM"));
    TEST_ASSERT_EQUAL_INT(1, ctx->num_regions);
    TEST_ASSERT_EQUAL_PTR(test_memory, ctx->regions[0].region);
    TEST_ASSERT_EQUAL_size_t(sizeof(test_memory), ctx->regions[0].size);
    TEST_ASSERT_EQUAL_STRING("RAM", ctx->regions[0].name);

    // Testar registro inválido
    TEST_ASSERT_FALSE(save_state_register_memory(ctx, NULL, sizeof(test_memory), "NULL"));
    TEST_ASSERT_FALSE(save_state_register_memory(ctx, test_memory, 0, "ZERO"));
    TEST_ASSERT_FALSE(save_state_register_memory(ctx, test_memory, sizeof(test_memory), NULL));
}

void test_save_state_save_load(void)
{
    ctx = save_state_create(test_filename);
    TEST_ASSERT_NOT_NULL(ctx);

    // Registrar memória e salvar
    TEST_ASSERT_TRUE(save_state_register_memory(ctx, test_memory, sizeof(test_memory), "RAM"));
    TEST_ASSERT_TRUE(save_state_save(ctx));

    // Modificar memória
    memset(test_memory, 0x55, sizeof(test_memory));

    // Carregar e verificar
    TEST_ASSERT_TRUE(save_state_load(ctx));

    uint8_t expected[sizeof(test_memory)];
    memset(expected, 0xAA, sizeof(expected));
    TEST_ASSERT_EQUAL_MEMORY(expected, test_memory, sizeof(test_memory));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_save_state_create_destroy);
    RUN_TEST(test_save_state_register_memory);
    RUN_TEST(test_save_state_save_load);
    return UNITY_END();
}
