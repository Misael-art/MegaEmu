#include <unity.h>
#include "core/cpu/m68k/m68k.h"
#include "core/memory/memory.h"

static m68k_t *m68k_ctx;

void setUp(void)
{
    // Inicializar o subsistema de memória para testes
    memory_init();

    // Criar contexto do M68K
    m68k_ctx = m68k_create();

    // Configurar callbacks de memória
    m68k_memory_callbacks_t callbacks = {
        .read_byte = memory_read_byte,
        .read_word = memory_read_word,
        .read_long = memory_read_long,
        .write_byte = memory_write_byte,
        .write_word = memory_write_word,
        .write_long = memory_write_long};

    m68k_set_memory_callbacks(m68k_ctx, &callbacks);

    // Programar valores de teste na memória
    memory_write_long(0x000000, 0x00001000); // SP inicial
    memory_write_long(0x000004, 0x00002000); // PC inicial

    // Colocar algumas instruções na memória para teste
    memory_write_word(0x00002000, 0x7001); // MOVEQ #1, D0
    memory_write_word(0x00002002, 0x7202); // MOVEQ #2, D1
    memory_write_word(0x00002004, 0xD001); // ADD.B D1, D0

    // Resetar CPU
    m68k_reset(m68k_ctx);
}

void tearDown(void)
{
    if (m68k_ctx)
    {
        m68k_destroy(m68k_ctx);
        m68k_ctx = NULL;
    }

    // Limpar memória
    memory_shutdown();
}

// Testes básicos
void test_m68k_init(void)
{
    TEST_ASSERT_NOT_NULL(m68k_ctx);
    TEST_ASSERT_EQUAL_UINT32(0x00002000, m68k_ctx->regs.pc);
    TEST_ASSERT_EQUAL_UINT32(0x00001000, m68k_ctx->regs.a[7]);
}

// Teste de execução de instruções simples
void test_m68k_execute_moveq(void)
{
    // Executar primeira instrução (MOVEQ #1, D0)
    m68k_run_cycles(m68k_ctx, 1);

    TEST_ASSERT_EQUAL_UINT32(0x00000001, m68k_ctx->regs.d[0]);
    TEST_ASSERT_EQUAL_UINT32(0x00002002, m68k_ctx->regs.pc);

    // Verificar flags
    TEST_ASSERT_EQUAL_UINT16(0, m68k_ctx->regs.sr & SR_Z); // Z flag deve estar limpo
    TEST_ASSERT_EQUAL_UINT16(0, m68k_ctx->regs.sr & SR_N); // N flag deve estar limpo
}

// Teste de execução de várias instruções
void test_m68k_execute_sequence(void)
{
    // Executar três instruções
    m68k_run_cycles(m68k_ctx, 3);

    // MOVEQ #1, D0 => D0 = 1
    // MOVEQ #2, D1 => D1 = 2
    // ADD.B D1, D0 => D0 = 3

    TEST_ASSERT_EQUAL_UINT32(0x00000003, m68k_ctx->regs.d[0]);
    TEST_ASSERT_EQUAL_UINT32(0x00000002, m68k_ctx->regs.d[1]);
    TEST_ASSERT_EQUAL_UINT32(0x00002006, m68k_ctx->regs.pc);
}

// Teste do disassembler
void test_m68k_disassembler(void)
{
    char buffer[256];

    // Desassemblar MOVEQ #1, D0
    int bytes = m68k_disassemble(m68k_ctx, 0x00002000, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(2, bytes);
    TEST_ASSERT_EQUAL_STRING("MOVEQ   #$01, D0", buffer);

    // Desassemblar MOVEQ #2, D1
    bytes = m68k_disassemble(m68k_ctx, 0x00002002, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(2, bytes);
    TEST_ASSERT_EQUAL_STRING("MOVEQ   #$02, D1", buffer);

    // Desassemblar ADD.B D1, D0
    bytes = m68k_disassemble(m68k_ctx, 0x00002004, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(2, bytes);
    TEST_ASSERT_EQUAL_STRING("ADD.B   D1, D0", buffer);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_m68k_init);
    RUN_TEST(test_m68k_execute_moveq);
    RUN_TEST(test_m68k_execute_sequence);
    RUN_TEST(test_m68k_disassembler);

    return UNITY_END();
}
