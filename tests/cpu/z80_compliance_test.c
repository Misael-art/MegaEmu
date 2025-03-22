/**
 * @file z80_compliance_test.c
 * @brief Testes de conformidade para o processador Z80
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../../src/core/cpu/z80/z80.h"
#include "../../src/core/cpu/z80/z80_debug.h"

// Buffer de memória para o contexto do Z80
#define MEM_SIZE 65536
static uint8_t memory[MEM_SIZE];

// Estado para verificação
typedef struct
{
    uint16_t af, bc, de, hl;
    uint16_t af_prime, bc_prime, de_prime, hl_prime;
    uint16_t ix, iy, sp, pc;
    bool iff1, iff2;
    uint8_t i, r, im;
} z80_test_state_t;

// Contexto de teste
typedef struct
{
    z80_t *cpu;
    z80_debug_t *debug;
    z80_test_state_t expected;
    int cycles_executed;
    bool test_passed;
    char error_message[256];
} z80_test_context_t;

// Funções de callback para o Z80
static uint8_t mem_read(void *context, uint16_t address)
{
    (void)context; // Não utilizado
    return memory[address];
}

static void mem_write(void *context, uint16_t address, uint8_t value)
{
    (void)context; // Não utilizado
    memory[address] = value;
}

static uint8_t io_read(void *context, uint16_t port)
{
    (void)context; // Não utilizado
    // Simular porta de I/O para teste
    if (port == 0xFE)
    { // Porta ZX Spectrum
        return 0xFF;
    }
    return 0xFF;
}

static void io_write(void *context, uint16_t port, uint8_t value)
{
    (void)context; // Não utilizado
    (void)port;    // Não utilizado
    (void)value;   // Não utilizado
    // Simular porta de I/O para teste
}

// Inicializar CPU para teste
static z80_test_context_t *init_test_context()
{
    z80_test_context_t *context = (z80_test_context_t *)malloc(sizeof(z80_test_context_t));
    if (!context)
    {
        return NULL;
    }

    // Limpar memória
    memset(memory, 0, MEM_SIZE);
    memset(context, 0, sizeof(z80_test_context_t));

    // Inicializar CPU
    context->cpu = z80_create();
    if (!context->cpu)
    {
        free(context);
        return NULL;
    }

    // Configurar callbacks
    z80_set_read_byte_callback(context->cpu, mem_read, NULL);
    z80_set_write_byte_callback(context->cpu, mem_write, NULL);
    z80_set_read_io_callback(context->cpu, io_read, NULL);
    z80_set_write_io_callback(context->cpu, io_write, NULL);

    // Inicializar debugger
    context->debug = z80_debug_create(context->cpu);

    // Reset inicial
    z80_reset(context->cpu);

    return context;
}

// Liberar contexto de teste
static void free_test_context(z80_test_context_t *context)
{
    if (context)
    {
        if (context->debug)
        {
            z80_debug_destroy(context->debug);
        }
        if (context->cpu)
        {
            z80_destroy(context->cpu);
        }
        free(context);
    }
}

// Verificar estado após execução
static bool check_state(z80_test_context_t *context)
{
    z80_t *cpu = context->cpu;
    z80_test_state_t *expected = &context->expected;

    if (cpu->regs.af != expected->af)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: AF=0x%04X, esperado 0x%04X", cpu->regs.af, expected->af);
        return false;
    }

    if (cpu->regs.bc != expected->bc)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: BC=0x%04X, esperado 0x%04X", cpu->regs.bc, expected->bc);
        return false;
    }

    if (cpu->regs.de != expected->de)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: DE=0x%04X, esperado 0x%04X", cpu->regs.de, expected->de);
        return false;
    }

    if (cpu->regs.hl != expected->hl)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: HL=0x%04X, esperado 0x%04X", cpu->regs.hl, expected->hl);
        return false;
    }

    if (cpu->regs.af_prime != expected->af_prime)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: AF'=0x%04X, esperado 0x%04X", cpu->regs.af_prime, expected->af_prime);
        return false;
    }

    if (cpu->regs.bc_prime != expected->bc_prime)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: BC'=0x%04X, esperado 0x%04X", cpu->regs.bc_prime, expected->bc_prime);
        return false;
    }

    if (cpu->regs.de_prime != expected->de_prime)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: DE'=0x%04X, esperado 0x%04X", cpu->regs.de_prime, expected->de_prime);
        return false;
    }

    if (cpu->regs.hl_prime != expected->hl_prime)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: HL'=0x%04X, esperado 0x%04X", cpu->regs.hl_prime, expected->hl_prime);
        return false;
    }

    if (cpu->regs.ix != expected->ix)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: IX=0x%04X, esperado 0x%04X", cpu->regs.ix, expected->ix);
        return false;
    }

    if (cpu->regs.iy != expected->iy)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: IY=0x%04X, esperado 0x%04X", cpu->regs.iy, expected->iy);
        return false;
    }

    if (cpu->regs.sp != expected->sp)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: SP=0x%04X, esperado 0x%04X", cpu->regs.sp, expected->sp);
        return false;
    }

    if (cpu->regs.pc != expected->pc)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: PC=0x%04X, esperado 0x%04X", cpu->regs.pc, expected->pc);
        return false;
    }

    if (cpu->i != expected->i)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: I=0x%02X, esperado 0x%02X", cpu->i, expected->i);
        return false;
    }

    if ((cpu->r & 0x7F) != (expected->r & 0x7F))
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: R=0x%02X, esperado 0x%02X", cpu->r, expected->r);
        return false;
    }

    if (cpu->iff1 != expected->iff1)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: IFF1=%d, esperado %d", cpu->iff1, expected->iff1);
        return false;
    }

    if (cpu->iff2 != expected->iff2)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: IFF2=%d, esperado %d", cpu->iff2, expected->iff2);
        return false;
    }

    if (cpu->im != expected->im)
    {
        snprintf(context->error_message, sizeof(context->error_message),
                 "Erro: IM=%d, esperado %d", cpu->im, expected->im);
        return false;
    }

    return true;
}

// Carregar um programa de teste na memória
static void load_test_program(uint16_t address, const uint8_t *program, int size)
{
    if (address + size <= MEM_SIZE)
    {
        memcpy(memory + address, program, size);
    }
}

// Testar uma instrução específica
static bool test_instruction(const char *name, const uint8_t *program, int size,
                             z80_test_state_t *initial, z80_test_state_t *expected, int max_cycles)
{
    z80_test_context_t *context = init_test_context();
    if (!context)
    {
        printf("Erro ao inicializar contexto de teste para %s\n", name);
        return false;
    }

    // Carregar programa na memória
    load_test_program(0x0000, program, size);

    // Configurar estado inicial
    z80_t *cpu = context->cpu;
    cpu->regs.af = initial->af;
    cpu->regs.bc = initial->bc;
    cpu->regs.de = initial->de;
    cpu->regs.hl = initial->hl;
    cpu->regs.af_prime = initial->af_prime;
    cpu->regs.bc_prime = initial->bc_prime;
    cpu->regs.de_prime = initial->de_prime;
    cpu->regs.hl_prime = initial->hl_prime;
    cpu->regs.ix = initial->ix;
    cpu->regs.iy = initial->iy;
    cpu->regs.sp = initial->sp;
    cpu->regs.pc = initial->pc;
    cpu->i = initial->i;
    cpu->r = initial->r;
    cpu->iff1 = initial->iff1;
    cpu->iff2 = initial->iff2;
    cpu->im = initial->im;

    // Configurar estado esperado
    context->expected = *expected;

    // Executar até atingir o número máximo de ciclos
    context->cycles_executed = z80_execute(context->cpu, max_cycles);

    // Verificar estado
    context->test_passed = check_state(context);

    // Exibir resultado do teste
    if (context->test_passed)
    {
        printf("Teste '%s': PASSOU (%d ciclos)\n", name, context->cycles_executed);
    }
    else
    {
        printf("Teste '%s': FALHOU - %s\n", name, context->error_message);
    }

    bool result = context->test_passed;
    free_test_context(context);
    return result;
}

// Testes para cada grupo de instruções
static void test_load_8bit_instructions()
{
    printf("\n=== Testes de instruções de carga 8-bit ===\n");

    // LD r,r' (Load register from register)
    {
        // LD A,B
        uint8_t program[] = {0x78}; // LD A,B

        z80_test_state_t initial = {0};
        initial.af = 0x0000;
        initial.bc = 0x4200;
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.af = 0x4200; // A = B = 0x42
        expected.pc = 0x0001;

        test_instruction("LD A,B", program, sizeof(program), &initial, &expected, 4);
    }

    // LD r,n (Load register with immediate value)
    {
        // LD C,0x42
        uint8_t program[] = {0x0E, 0x42}; // LD C,0x42

        z80_test_state_t initial = {0};
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.bc = 0x0042;
        expected.pc = 0x0002;

        test_instruction("LD C,n", program, sizeof(program), &initial, &expected, 7);
    }

    // LD r,(HL) (Load register from memory)
    {
        // LD A,(HL)
        uint8_t program[] = {0x7E}; // LD A,(HL)

        // Colocar valor na memória
        memory[0x1234] = 0x42;

        z80_test_state_t initial = {0};
        initial.hl = 0x1234;
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.af = 0x4200;
        expected.pc = 0x0001;

        test_instruction("LD A,(HL)", program, sizeof(program), &initial, &expected, 7);
    }

    // LD (HL),r (Store register to memory)
    {
        // LD (HL),B
        uint8_t program[] = {0x70}; // LD (HL),B

        z80_test_state_t initial = {0};
        initial.bc = 0x4200;
        initial.hl = 0x1234;
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.pc = 0x0001;

        test_instruction("LD (HL),B", program, sizeof(program), &initial, &expected, 7);

        // Verificar se o valor foi escrito na memória
        if (memory[0x1234] == 0x42)
        {
            printf("  Valor 0x42 escrito corretamente na memória 0x1234\n");
        }
        else
        {
            printf("  ERRO: Memória 0x1234 = 0x%02X, esperado 0x42\n", memory[0x1234]);
        }
    }
}

static void test_load_16bit_instructions()
{
    printf("\n=== Testes de instruções de carga 16-bit ===\n");

    // LD dd,nn (Load register pair with immediate value)
    {
        // LD BC,0x1234
        uint8_t program[] = {0x01, 0x34, 0x12}; // LD BC,0x1234

        z80_test_state_t initial = {0};
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.bc = 0x1234;
        expected.pc = 0x0003;

        test_instruction("LD BC,nn", program, sizeof(program), &initial, &expected, 10);
    }

    // LD HL,(nn) (Load HL from memory)
    {
        // LD HL,(0x1000)
        uint8_t program[] = {0x2A, 0x00, 0x10}; // LD HL,(0x1000)

        // Colocar valor na memória
        memory[0x1000] = 0x34;
        memory[0x1001] = 0x12;

        z80_test_state_t initial = {0};
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.hl = 0x1234;
        expected.pc = 0x0003;

        test_instruction("LD HL,(nn)", program, sizeof(program), &initial, &expected, 16);
    }

    // LD (nn),HL (Store HL to memory)
    {
        // LD (0x1000),HL
        uint8_t program[] = {0x22, 0x00, 0x10}; // LD (0x1000),HL

        z80_test_state_t initial = {0};
        initial.hl = 0x1234;
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.pc = 0x0003;

        test_instruction("LD (nn),HL", program, sizeof(program), &initial, &expected, 16);

        // Verificar se o valor foi escrito na memória
        if (memory[0x1000] == 0x34 && memory[0x1001] == 0x12)
        {
            printf("  Valor 0x1234 escrito corretamente na memória 0x1000-0x1001\n");
        }
        else
        {
            printf("  ERRO: Memória 0x1000-0x1001 = 0x%02X%02X, esperado 0x1234\n",
                   memory[0x1001], memory[0x1000]);
        }
    }
}

static void test_arithmetic_instructions()
{
    printf("\n=== Testes de instruções aritméticas ===\n");

    // ADD A,r (Add register to A)
    {
        // ADD A,B
        uint8_t program[] = {0x80}; // ADD A,B

        z80_test_state_t initial = {0};
        initial.af = 0x0100; // A=0x01, F=0x00
        initial.bc = 0x0200; // B=0x02, C=0x00
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.af = 0x0300; // A=0x03, F=0x00
        expected.pc = 0x0001;

        test_instruction("ADD A,B", program, sizeof(program), &initial, &expected, 4);
    }

    // SUB r (Subtract register from A)
    {
        // SUB B
        uint8_t program[] = {0x90}; // SUB B

        z80_test_state_t initial = {0};
        initial.af = 0x0500; // A=0x05, F=0x00
        initial.bc = 0x0200; // B=0x02, C=0x00
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.af = 0x0342; // A=0x03, F=0x42 (SUB sets N flag)
        expected.pc = 0x0001;

        test_instruction("SUB B", program, sizeof(program), &initial, &expected, 4);
    }

    // INC r (Increment register)
    {
        // INC B
        uint8_t program[] = {0x04}; // INC B

        z80_test_state_t initial = {0};
        initial.bc = 0x0100; // B=0x01
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.bc = 0x0200; // B=0x02
        expected.pc = 0x0001;

        test_instruction("INC B", program, sizeof(program), &initial, &expected, 4);
    }

    // DEC r (Decrement register)
    {
        // DEC C
        uint8_t program[] = {0x0D}; // DEC C

        z80_test_state_t initial = {0};
        initial.bc = 0x0002; // C=0x02
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.bc = 0x0001; // C=0x01
        expected.pc = 0x0001;

        test_instruction("DEC C", program, sizeof(program), &initial, &expected, 4);
    }
}

static void test_bit_manipulation_instructions()
{
    printf("\n=== Testes de instruções de manipulação de bits ===\n");

    // BIT b,r (Test bit in register)
    {
        // BIT 0,B (bit 0 set)
        uint8_t program[] = {0xCB, 0x40}; // BIT 0,B

        z80_test_state_t initial = {0};
        initial.bc = 0x0100; // B=0x01 (bit 0 set)
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.af = (initial.af & 0xFF00) | 0x10; // Z=0 (bit is set), H=1
        expected.pc = 0x0002;

        test_instruction("BIT 0,B (bit set)", program, sizeof(program), &initial, &expected, 8);
    }

    // RES b,r (Reset bit in register)
    {
        // RES 0,B
        uint8_t program[] = {0xCB, 0x80}; // RES 0,B

        z80_test_state_t initial = {0};
        initial.bc = 0x0100; // B=0x01 (bit 0 set)
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.bc = 0x0000; // B=0x00 (bit 0 reset)
        expected.pc = 0x0002;

        test_instruction("RES 0,B", program, sizeof(program), &initial, &expected, 8);
    }

    // SET b,r (Set bit in register)
    {
        // SET 1,C
        uint8_t program[] = {0xCB, 0xC9}; // SET 1,C

        z80_test_state_t initial = {0};
        initial.bc = 0x0000; // C=0x00 (bit 1 not set)
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.bc = 0x0002; // C=0x02 (bit 1 set)
        expected.pc = 0x0002;

        test_instruction("SET 1,C", program, sizeof(program), &initial, &expected, 8);
    }
}

static void test_jump_and_call_instructions()
{
    printf("\n=== Testes de instruções de salto e chamada ===\n");

    // JP nn (Jump to address)
    {
        // JP 0x1234
        uint8_t program[] = {0xC3, 0x34, 0x12}; // JP 0x1234

        z80_test_state_t initial = {0};
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.pc = 0x1234;

        test_instruction("JP nn", program, sizeof(program), &initial, &expected, 10);
    }

    // CALL nn (Call subroutine)
    {
        // CALL 0x1234
        uint8_t program[] = {0xCD, 0x34, 0x12}; // CALL 0x1234

        z80_test_state_t initial = {0};
        initial.pc = 0x0000;
        initial.sp = 0x2000;

        z80_test_state_t expected = initial;
        expected.pc = 0x1234;
        expected.sp = 0x1FFE;

        test_instruction("CALL nn", program, sizeof(program), &initial, &expected, 17);

        // Verificar se o endereço de retorno foi empilhado corretamente
        if (memory[0x1FFE] == 0x03 && memory[0x1FFF] == 0x00)
        {
            printf("  Endereço de retorno 0x0003 empilhado corretamente\n");
        }
        else
        {
            printf("  ERRO: Pilha 0x1FFE-0x1FFF = 0x%02X%02X, esperado 0x0003\n",
                   memory[0x1FFF], memory[0x1FFE]);
        }
    }

    // RET (Return from subroutine)
    {
        // RET
        uint8_t program[] = {0xC9}; // RET

        // Colocar endereço de retorno na pilha
        memory[0x1FFE] = 0x34;
        memory[0x1FFF] = 0x12;

        z80_test_state_t initial = {0};
        initial.pc = 0x0000;
        initial.sp = 0x1FFE;

        z80_test_state_t expected = initial;
        expected.pc = 0x1234;
        expected.sp = 0x2000;

        test_instruction("RET", program, sizeof(program), &initial, &expected, 10);
    }
}

static void test_io_instructions()
{
    printf("\n=== Testes de instruções de I/O ===\n");

    // IN A,(n) (Input from port)
    {
        // IN A,(0xFE)
        uint8_t program[] = {0xDB, 0xFE}; // IN A,(0xFE)

        z80_test_state_t initial = {0};
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.af = 0xFF00; // A=0xFF (valor retornado por io_read)
        expected.pc = 0x0002;

        test_instruction("IN A,(n)", program, sizeof(program), &initial, &expected, 11);
    }

    // OUT (n),A (Output to port)
    {
        // OUT (0xFE),A
        uint8_t program[] = {0xD3, 0xFE}; // OUT (0xFE),A

        z80_test_state_t initial = {0};
        initial.af = 0x4200; // A=0x42
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.pc = 0x0002;

        test_instruction("OUT (n),A", program, sizeof(program), &initial, &expected, 11);
    }
}

static void test_extended_and_indexed_instructions()
{
    printf("\n=== Testes de instruções estendidas e indexadas ===\n");

    // LD (IX+d),n (Load memory with immediate value)
    {
        // LD (IX+0x05),0x42
        uint8_t program[] = {0xDD, 0x36, 0x05, 0x42}; // LD (IX+0x05),0x42

        z80_test_state_t initial = {0};
        initial.ix = 0x1000;
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.pc = 0x0004;

        test_instruction("LD (IX+d),n", program, sizeof(program), &initial, &expected, 19);

        // Verificar se o valor foi escrito na memória
        if (memory[0x1005] == 0x42)
        {
            printf("  Valor 0x42 escrito corretamente na memória 0x1005 (IX+0x05)\n");
        }
        else
        {
            printf("  ERRO: Memória 0x1005 = 0x%02X, esperado 0x42\n", memory[0x1005]);
        }
    }

    // ADD IX,BC (Add register pair to IX)
    {
        // ADD IX,BC
        uint8_t program[] = {0xDD, 0x09}; // ADD IX,BC

        z80_test_state_t initial = {0};
        initial.ix = 0x1000;
        initial.bc = 0x0234;
        initial.pc = 0x0000;

        z80_test_state_t expected = initial;
        expected.ix = 0x1234;
        expected.pc = 0x0002;

        test_instruction("ADD IX,BC", program, sizeof(program), &initial, &expected, 15);
    }
}

// Função de teste principal
int main()
{
    printf("=== Teste de Conformidade do Z80 ===\n");

    // Executar testes para cada grupo de instruções
    test_load_8bit_instructions();
    test_load_16bit_instructions();
    test_arithmetic_instructions();
    test_bit_manipulation_instructions();
    test_jump_and_call_instructions();
    test_io_instructions();
    test_extended_and_indexed_instructions();

    printf("\nTestes de conformidade concluídos.\n");
    return 0;
}
