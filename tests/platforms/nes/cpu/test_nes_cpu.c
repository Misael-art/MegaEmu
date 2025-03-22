/**
 * @file test_nes_cpu.c
 * @brief Testes unitários para o CPU do NES (6502/2A03)
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "../../../../src/platforms/nes/cpu/nes_cpu.hpp"
#include "../../../../src/utils/test_utils.h"

// Mocks e stubs necessários
typedef struct
{
    uint8_t data[0x10000];
    uint8_t (*read_byte)(void *, uint16_t);
    void (*write_byte)(void *, uint16_t, uint8_t);

    uint8_t read(uint16_t address)
    {
        return data[address];
    }

    void write(uint16_t address, uint8_t value)
    {
        data[address] = value;
    }
} MockMemory;

// Funções de callback para leitura/escrita de memória
uint8_t memory_read_callback(void *memory, uint16_t address)
{
    return ((MockMemory *)memory)->read(address);
}

void memory_write_callback(void *memory, uint16_t address, uint8_t value)
{
    ((MockMemory *)memory)->write(address, value);
}

// Variáveis globais para os testes
static MockMemory *memory;
static MegaEmu::Platforms::NES::NESCPU *cpu;

/**
 * @brief Configura o ambiente para os testes
 */
static void setup(void)
{
    // Alocar memória para os mocks
    memory = (MockMemory *)malloc(sizeof(MockMemory));

    // Inicializar memória com valores padrão
    memset(memory->data, 0, sizeof(memory->data));

    // Configurar callbacks de memória
    memory->read_byte = memory_read_callback;
    memory->write_byte = memory_write_callback;

    // Criar instância do CPU
    cpu = new MegaEmu::Platforms::NES::NESCPU(
        static_cast<void *>(memory));
}

/**
 * @brief Limpa o ambiente após os testes
 */
static void teardown(void)
{
    delete cpu;
    free(memory);
}

/**
 * @brief Carrega um programa simples na memória
 */
static void load_test_program(void)
{
    // Endereço de reset do 6502 (0xFFFC-0xFFFD)
    memory->data[0xFFFC] = 0x00;
    memory->data[0xFFFD] = 0x80;

    // Programa simples começando em 0x8000
    // LDA #$42   ; Carrega o valor 0x42 no acumulador
    // STA $0200  ; Armazena o valor na posição 0x0200
    // JMP $8000  ; Pula de volta para o início
    memory->data[0x8000] = 0xA9; // LDA #imm
    memory->data[0x8001] = 0x42; // valor 0x42
    memory->data[0x8002] = 0x8D; // STA abs
    memory->data[0x8003] = 0x00; // endereço low byte
    memory->data[0x8004] = 0x02; // endereço high byte
    memory->data[0x8005] = 0x4C; // JMP abs
    memory->data[0x8006] = 0x00; // endereço low byte
    memory->data[0x8007] = 0x80; // endereço high byte
}

/**
 * @brief Testa a inicialização do CPU
 */
void test_initialization(void)
{
    printf("Testando inicialização do CPU...\n");

    // Verificar se o CPU foi inicializado corretamente
    assert(cpu != NULL);

    // Verificar estado inicial
    cpu->reset();

    // Verificar se os registradores foram inicializados corretamente
    assert(cpu->getPC() == 0); // Após reset, o PC deve ser carregado do endereço 0xFFFC-0xFFFD

    printf("Teste de inicialização concluído com sucesso!\n");
}

/**
 * @brief Testa a execução de ciclos do CPU
 */
void test_cycle(void)
{
    printf("Testando execução de ciclos do CPU...\n");

    // Carregar programa de teste
    load_test_program();

    // Resetar CPU (carrega PC do endereço 0xFFFC-0xFFFD)
    cpu->reset();

    // Executar alguns ciclos
    int cycles_executed = cpu->cycle(10);

    // Verificar se os ciclos foram executados
    assert(cycles_executed > 0);

    printf("Teste de ciclo concluído com sucesso!\n");
}

/**
 * @brief Testa a execução de instrução
 */
void test_step(void)
{
    printf("Testando execução de instrução...\n");

    // Carregar programa de teste
    load_test_program();

    // Resetar CPU
    cpu->reset();

    // Executar uma instrução
    int cycles = cpu->step();

    // Verificar se a instrução foi executada
    assert(cycles > 0);
    assert(cpu->getA() == 0x42); // Deve ter executado LDA #$42

    printf("Teste de execução de instrução concluído com sucesso!\n");
}

/**
 * @brief Testa o tratamento de NMI
 */
void test_nmi(void)
{
    printf("Testando tratamento de NMI...\n");

    // Carregar programa de teste
    load_test_program();

    // Configurar vetor de NMI
    memory->data[0xFFFA] = 0x00;
    memory->data[0xFFFB] = 0x90; // NMI handler em 0x9000

    // Código simples no handler de NMI
    memory->data[0x9000] = 0xA9; // LDA #imm
    memory->data[0x9001] = 0xFF; // valor 0xFF
    memory->data[0x9002] = 0x40; // RTI

    // Resetar CPU
    cpu->reset();

    // Executar algumas instruções
    cpu->step(); // Executa LDA #$42

    // Gerar NMI
    cpu->triggerNMI();

    // Executar o handler de NMI
    cpu->step(); // Deve salvar estado e pular para 0x9000
    cpu->step(); // Executa LDA #$FF
    cpu->step(); // Executa RTI

    // Verificar se o NMI foi tratado corretamente
    assert(cpu->getA() == 0xFF); // Valor definido no handler de NMI

    printf("Teste de NMI concluído com sucesso!\n");
}

/**
 * @brief Testa o tratamento de IRQ
 */
void test_irq(void)
{
    printf("Testando tratamento de IRQ...\n");

    // Carregar programa de teste
    load_test_program();

    // Configurar vetor de IRQ
    memory->data[0xFFFE] = 0x00;
    memory->data[0xFFFF] = 0xA0; // IRQ handler em 0xA000

    // Código simples no handler de IRQ
    memory->data[0xA000] = 0xA9; // LDA #imm
    memory->data[0xA001] = 0x77; // valor 0x77
    memory->data[0xA002] = 0x40; // RTI

    // Resetar CPU
    cpu->reset();

    // Limpar flag de interrupção para permitir IRQs
    cpu->setP(cpu->getP() & ~0x04);

    // Executar algumas instruções
    cpu->step(); // Executa LDA #$42

    // Gerar IRQ
    cpu->triggerIRQ();

    // Executar o handler de IRQ
    cpu->step(); // Deve salvar estado e pular para 0xA000
    cpu->step(); // Executa LDA #$77
    cpu->step(); // Executa RTI

    // Verificar se o IRQ foi tratado corretamente
    assert(cpu->getA() == 0x77); // Valor definido no handler de IRQ

    printf("Teste de IRQ concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura/escrita de registradores
 */
void test_register_access(void)
{
    printf("Testando acesso aos registradores...\n");

    // Resetar CPU
    cpu->reset();

    // Testar acesso ao acumulador (A)
    cpu->setA(0x42);
    assert(cpu->getA() == 0x42);

    // Testar acesso ao registrador X
    cpu->setX(0x33);
    assert(cpu->getX() == 0x33);

    // Testar acesso ao registrador Y
    cpu->setY(0x55);
    assert(cpu->getY() == 0x55);

    // Testar acesso ao registrador de status (P)
    cpu->setP(0xA5);
    assert(cpu->getP() == 0xA5);

    printf("Teste de acesso aos registradores concluído com sucesso!\n");
}

/**
 * @brief Testa as flags do processador
 */
void test_processor_flags(void)
{
    printf("Testando flags do processador...\n");

    // Resetar CPU
    cpu->reset();

    // Testar flag de carry (C)
    cpu->setCarryFlag(1);
    assert((cpu->getP() & 0x01) == 0x01);
    cpu->setCarryFlag(0);
    assert((cpu->getP() & 0x01) == 0x00);

    // Testar flag de zero (Z)
    cpu->setZeroFlag(1);
    assert((cpu->getP() & 0x02) == 0x02);
    cpu->setZeroFlag(0);
    assert((cpu->getP() & 0x02) == 0x00);

    // Testar flag de interrupção (I)
    cpu->setInterruptFlag(1);
    assert((cpu->getP() & 0x04) == 0x04);
    cpu->setInterruptFlag(0);
    assert((cpu->getP() & 0x04) == 0x00);

    printf("Teste de flags do processador concluído com sucesso!\n");
}

/**
 * @brief Testa a execução de múltiplas instruções
 */
void test_execute_multiple_instructions(void)
{
    printf("Testando execução de múltiplas instruções...\n");

    // Carregar programa de teste
    load_test_program();

    // Resetar CPU
    cpu->reset();

    // Executar várias instruções
    cpu->step(); // LDA #$42
    cpu->step(); // STA $0200

    // Verificar se as instruções foram executadas corretamente
    assert(cpu->getA() == 0x42);
    assert(memory->data[0x0200] == 0x42);

    printf("Teste de execução de múltiplas instruções concluído com sucesso!\n");
}

/**
 * @brief Testa opcodes ilegais do 6502
 */
void test_illegal_opcodes(void)
{
    printf("Testando opcodes ilegais...\n");

    // Preparar memória
    memset(memory->data, 0, sizeof(memory->data));

    // Configurar vetor de reset
    memory->data[0xFFFC] = 0x00;
    memory->data[0xFFFD] = 0x80;

    // Programa para testar opcodes ilegais
    uint16_t addr = 0x8000;

    // 1. LAX (Load A and X) - 0xA7 Zero Page
    memory->data[addr++] = 0xA7; // LAX Zero Page
    memory->data[addr++] = 0x50; // Endereço Zero Page
    memory->data[0x0050] = 0x42; // Valor a ser carregado

    // 2. SAX (Store A AND X) - 0x87 Zero Page
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0xFF; // Valor 0xFF
    memory->data[addr++] = 0xA2; // LDX #imm
    memory->data[addr++] = 0x0F; // Valor 0x0F
    memory->data[addr++] = 0x87; // SAX Zero Page
    memory->data[addr++] = 0x51; // Endereço Zero Page

    // 3. DCP (Decrement and Compare) - 0xC7 Zero Page
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0x42; // Valor 0x42
    memory->data[addr++] = 0xC7; // DCP Zero Page
    memory->data[addr++] = 0x52; // Endereço Zero Page
    memory->data[0x0052] = 0x43; // Valor a ser decrementado

    // 4. ISC (Increment and Subtract) - 0xE7 Zero Page
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0x42; // Valor 0x42
    memory->data[addr++] = 0x38; // SEC
    memory->data[addr++] = 0xE7; // ISC Zero Page
    memory->data[addr++] = 0x53; // Endereço Zero Page
    memory->data[0x0053] = 0x10; // Valor a ser incrementado

    // 5. SLO (Shift Left and OR) - 0x07 Zero Page
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0x00; // Valor 0x00
    memory->data[addr++] = 0x07; // SLO Zero Page
    memory->data[addr++] = 0x54; // Endereço Zero Page
    memory->data[0x0054] = 0x81; // Valor a ser deslocado

    // 6. RLA (Rotate Left and AND) - 0x27 Zero Page
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0xFF; // Valor 0xFF
    memory->data[addr++] = 0x18; // CLC
    memory->data[addr++] = 0x27; // RLA Zero Page
    memory->data[addr++] = 0x55; // Endereço Zero Page
    memory->data[0x0055] = 0x42; // Valor a ser rotacionado

    // 7. ANC (AND + Set Carry as bit 7) - 0x0B Immediate
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0xFF; // Valor 0xFF
    memory->data[addr++] = 0x0B; // ANC #imm
    memory->data[addr++] = 0x80; // Valor 0x80

    // 8. ALR (AND + LSR) - 0x4B Immediate
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0xFF; // Valor 0xFF
    memory->data[addr++] = 0x4B; // ALR #imm
    memory->data[addr++] = 0x55; // Valor 0x55

    // 9. ARR (AND + ROR) - 0x6B Immediate
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0xFF; // Valor 0xFF
    memory->data[addr++] = 0x38; // SEC
    memory->data[addr++] = 0x6B; // ARR #imm
    memory->data[addr++] = 0x55; // Valor 0x55

    // 10. AXS (A AND X - value) - 0xCB Immediate
    memory->data[addr++] = 0xA9; // LDA #imm
    memory->data[addr++] = 0xFF; // Valor 0xFF
    memory->data[addr++] = 0xA2; // LDX #imm
    memory->data[addr++] = 0xF0; // Valor 0xF0
    memory->data[addr++] = 0xCB; // AXS #imm
    memory->data[addr++] = 0x10; // Valor 0x10

    // Adicionar uma instrução NOP ilegal - 0x1A (Implicit)
    memory->data[addr++] = 0x1A; // NOP Illegal

    // Adicionar RTS para finalizar o programa
    memory->data[addr++] = 0x60; // RTS

    // Resetar CPU
    cpu->reset();

    // Executar programa e verificar resultados

    // Teste 1: LAX Zero Page
    cpu->step();
    assert(cpu->getA() == 0x42 && cpu->getX() == 0x42);

    // Teste 2: SAX Zero Page
    cpu->step(); // LDA #$FF
    cpu->step(); // LDX #$0F
    cpu->step(); // SAX Zero Page
    assert(memory->data[0x0051] == (0xFF & 0x0F));

    // Teste 3: DCP Zero Page
    cpu->step(); // LDA #$42
    cpu->step(); // DCP Zero Page
    assert(memory->data[0x0052] == (0x43 - 1));
    assert((cpu->getP() & 0x03) == 0x01); // Carry deve estar setado, Zero não

    // Teste 4: ISC Zero Page
    cpu->step(); // LDA #$42
    cpu->step(); // SEC
    cpu->step(); // ISC Zero Page
    assert(memory->data[0x0053] == (0x10 + 1));
    assert(cpu->getA() == (0x42 - 0x11));

    // Teste 5: SLO Zero Page
    cpu->step(); // LDA #$00
    cpu->step(); // SLO Zero Page
    assert(memory->data[0x0054] == (0x81 << 1));
    assert(cpu->getA() == 0x02);
    assert((cpu->getP() & 0x01) == 0x01); // Carry deve estar setado

    // Teste 6: RLA Zero Page
    cpu->step(); // LDA #$FF
    cpu->step(); // CLC
    cpu->step(); // RLA Zero Page
    assert(memory->data[0x0055] == (0x42 << 1));
    assert(cpu->getA() == (0xFF & (0x42 << 1)));

    // Teste 7: ANC Immediate
    cpu->step(); // LDA #$FF
    cpu->step(); // ANC #$80
    assert(cpu->getA() == 0x80);
    assert((cpu->getP() & 0x01) == 0x01); // Carry deve estar setado

    // Teste 8: ALR Immediate
    cpu->step(); // LDA #$FF
    cpu->step(); // ALR #$55
    assert(cpu->getA() == ((0xFF & 0x55) >> 1));
    assert((cpu->getP() & 0x01) == 0x01); // Carry deve estar setado

    // Teste 9: ARR Immediate
    cpu->step(); // LDA #$FF
    cpu->step(); // SEC
    cpu->step(); // ARR #$55
    assert(cpu->getA() == (((0xFF & 0x55) >> 1) | 0x80));

    // Teste 10: AXS Immediate
    cpu->step(); // LDA #$FF
    cpu->step(); // LDX #$F0
    cpu->step(); // AXS #$10
    assert(cpu->getX() == ((0xFF & 0xF0) - 0x10));

    // Teste de NOP ilegal
    cpu->step(); // NOP Illegal
    // Não tem um resultado específico, apenas verificar que a execução continua

    printf("Teste de opcodes ilegais concluído com sucesso!\n");
}

/**
 * @brief Testa timing crítico para operações de página cruzada
 */
void test_page_crossing_timing(void)
{
    printf("Testando timing de página cruzada...\n");

    // Preparar memória
    memset(memory->data, 0, sizeof(memory->data));

    // Configurar vetor de reset
    memory->data[0xFFFC] = 0x00;
    memory->data[0xFFFD] = 0x80;

    // Configurar dados em diferentes páginas
    memory->data[0x00FF] = 0x42; // Último byte da página zero
    memory->data[0x0100] = 0x43; // Primeiro byte da página um

    memory->data[0x01FF] = 0x44; // Último byte da página um
    memory->data[0x0200] = 0x45; // Primeiro byte da página dois

    // Dados para testes de Absolute,X
    memory->data[0x02FF] = 0x46; // Último byte da página dois
    memory->data[0x0300] = 0x47; // Primeiro byte da página três

    // Programa para testar timing de página cruzada
    uint16_t addr = 0x8000;

    // 1. LDA Zero Page,X sem cruzamento de página
    memory->data[addr++] = 0xA2; // LDX #imm
    memory->data[addr++] = 0x01; // X = 1
    memory->data[addr++] = 0xB5; // LDA Zero Page,X
    memory->data[addr++] = 0x40; // Base em $40, efetivo $41
    memory->data[0x0041] = 0x55; // Valor a ser carregado

    // 2. LDA Zero Page,X com cruzamento de página (não adiciona ciclos, apenas altera endereço)
    memory->data[addr++] = 0xA2; // LDX #imm
    memory->data[addr++] = 0x40; // X = 0x40
    memory->data[addr++] = 0xB5; // LDA Zero Page,X
    memory->data[addr++] = 0xC0; // Base em $C0, efetivo $00 (wraparound)
    memory->data[0x0000] = 0x56; // Valor a ser carregado

    // 3. LDA Absolute,X sem cruzamento de página
    memory->data[addr++] = 0xA2; // LDX #imm
    memory->data[addr++] = 0x01; // X = 1
    memory->data[addr++] = 0xBD; // LDA Absolute,X
    memory->data[addr++] = 0x00; // Endereço low byte
    memory->data[addr++] = 0x20; // Endereço high byte (base em $2000, efetivo $2001)
    memory->data[0x2001] = 0x57; // Valor a ser carregado

    // 4. LDA Absolute,X com cruzamento de página (+1 ciclo)
    memory->data[addr++] = 0xA2; // LDX #imm
    memory->data[addr++] = 0x01; // X = 1
    memory->data[addr++] = 0xBD; // LDA Absolute,X
    memory->data[addr++] = 0xFF; // Endereço low byte
    memory->data[addr++] = 0x20; // Endereço high byte (base em $20FF, efetivo $2100)
    memory->data[0x2100] = 0x58; // Valor a ser carregado

    // 5. LDA Absolute,Y com cruzamento de página (+1 ciclo)
    memory->data[addr++] = 0xA0; // LDY #imm
    memory->data[addr++] = 0x01; // Y = 1
    memory->data[addr++] = 0xB9; // LDA Absolute,Y
    memory->data[addr++] = 0xFF; // Endereço low byte
    memory->data[addr++] = 0x21; // Endereço high byte (base em $21FF, efetivo $2200)
    memory->data[0x2200] = 0x59; // Valor a ser carregado

    // 6. LDA Indirect,Y com cruzamento de página (+1 ciclo)
    memory->data[addr++] = 0xA0; // LDY #imm
    memory->data[addr++] = 0x01; // Y = 1
    memory->data[addr++] = 0xB1; // LDA Indirect,Y
    memory->data[addr++] = 0x80; // Endereço zero page para indireção
    memory->data[0x0080] = 0xFF; // Low byte do endereço
    memory->data[0x0081] = 0x22; // High byte do endereço (base em $22FF, efetivo $2300)
    memory->data[0x2300] = 0x5A; // Valor a ser carregado

    // Adicionar uma instrução RTS para finalizar o programa
    memory->data[addr++] = 0x60; // RTS

    // Resetar CPU e contador de ciclos
    cpu->reset();
    uint32_t initial_cycles = cpu->getCycles();

    // Executar casos de teste e verificar resultados

    // Teste 1: LDA Zero Page,X sem cruzamento de página
    cpu->step(); // LDX #$01
    cpu->step(); // LDA Zero Page,X
    assert(cpu->getA() == 0x55);
    uint32_t cycles_zp_no_cross = cpu->getCycles() - initial_cycles - 2; // -2 para o LDX
    assert(cycles_zp_no_cross == 4);                                     // LDA Zero Page,X usa 4 ciclos
    initial_cycles = cpu->getCycles();

    // Teste 2: LDA Zero Page,X com cruzamento de página (não adiciona ciclos)
    cpu->step(); // LDX #$40
    cpu->step(); // LDA Zero Page,X
    assert(cpu->getA() == 0x56);
    uint32_t cycles_zp_with_cross = cpu->getCycles() - initial_cycles - 2; // -2 para o LDX
    assert(cycles_zp_with_cross == 4);                                     // Ainda usa 4 ciclos, apesar do wrap
    initial_cycles = cpu->getCycles();

    // Teste 3: LDA Absolute,X sem cruzamento de página
    cpu->step(); // LDX #$01
    cpu->step(); // LDA Absolute,X
    assert(cpu->getA() == 0x57);
    uint32_t cycles_abs_no_cross = cpu->getCycles() - initial_cycles - 2; // -2 para o LDX
    assert(cycles_abs_no_cross == 4);                                     // LDA Absolute,X sem cruzamento usa 4 ciclos
    initial_cycles = cpu->getCycles();

    // Teste 4: LDA Absolute,X com cruzamento de página (+1 ciclo)
    cpu->step(); // LDX #$01
    cpu->step(); // LDA Absolute,X com page crossing
    assert(cpu->getA() == 0x58);
    uint32_t cycles_abs_with_cross = cpu->getCycles() - initial_cycles - 2; // -2 para o LDX
    assert(cycles_abs_with_cross == 5);                                     // LDA Absolute,X com cruzamento usa 5 ciclos
    initial_cycles = cpu->getCycles();

    // Teste 5: LDA Absolute,Y com cruzamento de página (+1 ciclo)
    cpu->step(); // LDY #$01
    cpu->step(); // LDA Absolute,Y com page crossing
    assert(cpu->getA() == 0x59);
    uint32_t cycles_abs_y_with_cross = cpu->getCycles() - initial_cycles - 2; // -2 para o LDY
    assert(cycles_abs_y_with_cross == 5);                                     // LDA Absolute,Y com cruzamento usa 5 ciclos
    initial_cycles = cpu->getCycles();

    // Teste 6: LDA Indirect,Y com cruzamento de página (+1 ciclo)
    cpu->step(); // LDY #$01
    cpu->step(); // LDA Indirect,Y com page crossing
    assert(cpu->getA() == 0x5A);
    uint32_t cycles_ind_y_with_cross = cpu->getCycles() - initial_cycles - 2; // -2 para o LDY
    assert(cycles_ind_y_with_cross == 6);                                     // LDA Indirect,Y com cruzamento usa 6 ciclos

    printf("Teste de timing de página cruzada concluído com sucesso!\n");
}

/**
 * @brief Função principal para execução dos testes
 */
int main(void)
{
    printf("Iniciando testes do CPU do NES (6502/2A03)\n");

    // Executar testes
    setup();
    test_initialization();
    teardown();

    setup();
    test_cycle();
    teardown();

    setup();
    test_step();
    teardown();

    setup();
    test_nmi();
    teardown();

    setup();
    test_irq();
    teardown();

    setup();
    test_register_access();
    teardown();

    setup();
    test_processor_flags();
    teardown();

    setup();
    test_execute_multiple_instructions();
    teardown();

    setup();
    test_illegal_opcodes();
    teardown();

    setup();
    test_page_crossing_timing();
    teardown();

    printf("Todos os testes do CPU do NES concluídos com sucesso!\n");

    return 0;
}
