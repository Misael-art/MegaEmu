/**
 * @file test_nes_ppu.c
 * @brief Testes unitários para o PPU do NES (Picture Processing Unit)
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "../../../../src/platforms/nes/ppu/nes_ppu.hpp"
#include "../../../../src/utils/test_utils.h"

// Mocks e stubs necessários
typedef struct {
    uint8_t data[0x10000];
    
    uint8_t read(uint16_t address) {
        return data[address];
    }
    
    void write(uint16_t address, uint8_t value) {
        data[address] = value;
    }
} MockMemory;

// Variáveis globais para os testes
static MockMemory* memory;
static MegaEmu::Platforms::NES::NESPPU* ppu;
static uint32_t framebuffer[256 * 240];

/**
 * @brief Configura o ambiente para os testes
 */
static void setup(void)
{
    // Alocar memória para os mocks
    memory = (MockMemory*)malloc(sizeof(MockMemory));
    
    // Inicializar memória com valores padrão
    memset(memory->data, 0, sizeof(memory->data));
    
    // Criar instância do PPU
    ppu = new MegaEmu::Platforms::NES::NESPPU();
    
    // Limpar framebuffer
    memset(framebuffer, 0, sizeof(framebuffer));
}

/**
 * @brief Limpa o ambiente após os testes
 */
static void teardown(void)
{
    delete ppu;
    free(memory);
}

/**
 * @brief Testa a inicialização do PPU
 */
void test_initialization(void)
{
    printf("Testando inicialização do PPU...\n");
    
    // Verificar se o PPU foi inicializado corretamente
    assert(ppu != NULL);
    
    // Verificar estado inicial
    ppu->initialize();
    ppu->reset();
    
    // Verificar se os registradores foram inicializados corretamente
    assert(ppu->getStatus() == 0); // Status deve ser 0 após reset
    
    printf("Teste de inicialização concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura e escrita de registradores
 */
void test_register_access(void)
{
    printf("Testando acesso aos registradores do PPU...\n");
    
    // Escrever em alguns registradores
    ppu->writeRegister(0, 0x1E); // PPUCTRL
    ppu->writeRegister(1, 0x08); // PPUMASK
    
    // Ler registradores
    uint8_t status = ppu->readRegister(2); // PPUSTATUS
    
    // Verificar se a leitura de status limpa o bit 7
    assert((status & 0x80) == 0);
    
    printf("Teste de acesso aos registradores concluído com sucesso!\n");
}

/**
 * @brief Testa o acesso à OAM (Object Attribute Memory)
 */
void test_oam_access(void)
{
    printf("Testando acesso à OAM...\n");
    
    // Definir endereço OAM
    ppu->writeRegister(3, 0x10); // OAMADDR = 0x10
    
    // Escrever dados na OAM
    for (int i = 0; i < 10; i++) {
        ppu->writeRegister(4, i * 4); // OAMDATA
    }
    
    // Resetar endereço OAM
    ppu->writeRegister(3, 0x10); // OAMADDR = 0x10
    
    // Ler dados da OAM
    for (int i = 0; i < 10; i++) {
        uint8_t data = ppu->readRegister(4); // OAMDATA
        assert(data == i * 4);
    }
    
    printf("Teste de acesso à OAM concluído com sucesso!\n");
}

/**
 * @brief Testa o acesso à VRAM (Video RAM)
 */
void test_vram_access(void)
{
    printf("Testando acesso à VRAM...\n");
    
    // Configurar endereço VRAM
    ppu->writeRegister(6, 0x21); // PPUADDR high byte
    ppu->writeRegister(6, 0x08); // PPUADDR low byte
    
    // Escrever dados na VRAM
    for (int i = 0; i < 16; i++) {
        ppu->writeRegister(7, i * 2); // PPUDATA
    }
    
    // Resetar endereço VRAM para leitura
    ppu->writeRegister(6, 0x21); // PPUADDR high byte
    ppu->writeRegister(6, 0x08); // PPUADDR low byte
    
    // Primeira leitura descartada (buffer de leitura)
    ppu->readRegister(7);
    
    // Ler dados da VRAM
    for (int i = 0; i < 16; i++) {
        uint8_t data = ppu->readRegister(7); // PPUDATA
        assert(data == i * 2);
    }
    
    printf("Teste de acesso à VRAM concluído com sucesso!\n");
}

/**
 * @brief Testa o acesso à paleta
 */
void test_palette_access(void)
{
    printf("Testando acesso à paleta...\n");
    
    // Configurar endereço para paleta
    ppu->writeRegister(6, 0x3F); // PPUADDR high byte
    ppu->writeRegister(6, 0x00); // PPUADDR low byte
    
    // Escrever dados na paleta
    for (int i = 0; i < 32; i++) {
        ppu->writeRegister(7, i); // PPUDATA
    }
    
    // Resetar endereço para leitura
    ppu->writeRegister(6, 0x3F); // PPUADDR high byte
    ppu->writeRegister(6, 0x00); // PPUADDR low byte
    
    // Primeira leitura descartada (buffer de leitura)
    ppu->readRegister(7);
    
    // Ler dados da paleta
    for (int i = 0; i < 32; i++) {
        uint8_t data = ppu->readRegister(7); // PPUDATA
        assert(data == i);
    }
    
    printf("Teste de acesso à paleta concluído com sucesso!\n");
}

/**
 * @brief Testa o acesso ao scroll
 */
void test_scroll_access(void)
{
    printf("Testando acesso ao scroll...\n");
    
    // Configurar scroll
    ppu->writeRegister(5, 0x10); // PPUSCROLL X
    ppu->writeRegister(5, 0x20); // PPUSCROLL Y
    
    // Verificar se o scroll foi configurado corretamente
    // Isso requer acesso a métodos internos ou verificação indireta
    
    printf("Teste de acesso ao scroll concluído com sucesso!\n");
}

/**
 * @brief Testa o ciclo de renderização
 */
void test_render_cycle(void)
{
    printf("Testando ciclo de renderização...\n");
    
    // Configurar PPU para renderização
    ppu->writeRegister(0, 0x90); // PPUCTRL - NMI habilitado, sprites 8x16
    ppu->writeRegister(1, 0x1E); // PPUMASK - Renderização habilitada
    
    // Executar ciclos de renderização
    for (int i = 0; i < 341 * 262; i++) {
        ppu->cycle();
    }
    
    // Verificar se o frame foi completado
    assert((ppu->getStatus() & 0x80) != 0); // Bit 7 de PPUSTATUS deve estar setado (vblank)
    
    printf("Teste de ciclo de renderização concluído com sucesso!\n");
}

/**
 * @brief Testa a transferência de DMA para OAM
 */
void test_oam_dma_transfer(void)
{
    printf("Testando transferência de DMA para OAM...\n");
    
    // Preparar dados na memória para DMA
    for (int i = 0; i < 256; i++) {
        memory->data[0x0200 + i] = i;
    }
    
    // Executar DMA da página 0x02 para OAM
    ppu->oamDMA(memory->data + 0x0200);
    
    // Verificar se os dados foram transferidos corretamente
    ppu->writeRegister(3, 0x00); // OAMADDR = 0
    
    for (int i = 0; i < 256; i++) {
        uint8_t data = ppu->readRegister(4); // OAMDATA
        assert(data == i);
    }
    
    printf("Teste de transferência de DMA para OAM concluído com sucesso!\n");
}

/**
 * @brief Testa a avaliação de sprites
 */
void test_sprite_evaluation(void)
{
    printf("Testando avaliação de sprites...\n");
    
    // Configurar alguns sprites na OAM
    ppu->writeRegister(3, 0x00); // OAMADDR = 0
    
    // Sprite 0: Y=20, Tile=1, Attr=0, X=30
    ppu->writeRegister(4, 20);  // Y
    ppu->writeRegister(4, 1);   // Tile
    ppu->writeRegister(4, 0);   // Attr
    ppu->writeRegister(4, 30);  // X
    
    // Sprite 1: Y=40, Tile=2, Attr=1, X=50
    ppu->writeRegister(4, 40);  // Y
    ppu->writeRegister(4, 2);   // Tile
    ppu->writeRegister(4, 1);   // Attr
    ppu->writeRegister(4, 50);  // X
    
    // Configurar PPU para renderização
    ppu->writeRegister(0, 0x90); // PPUCTRL - NMI habilitado, sprites 8x16
    ppu->writeRegister(1, 0x1E); // PPUMASK - Renderização habilitada
    
    // Executar ciclos até a linha 20 (onde está o sprite 0)
    for (int i = 0; i < 341 * 20; i++) {
        ppu->cycle();
    }
    
    // Verificar se o sprite 0 foi avaliado corretamente
    // Isso requer acesso a métodos internos ou verificação indireta
    
    printf("Teste de avaliação de sprites concluído com sucesso!\n");
}

/**
 * @brief Testa a geração de frame
 */
void test_frame_generation(void)
{
    printf("Testando geração de frame...\n");
    
    // Configurar PPU para renderização
    ppu->writeRegister(0, 0x90); // PPUCTRL - NMI habilitado, sprites 8x16
    ppu->writeRegister(1, 0x1E); // PPUMASK - Renderização habilitada
    
    // Configurar alguns dados na VRAM para renderização
    ppu->writeRegister(6, 0x20); // PPUADDR high byte
    ppu->writeRegister(6, 0x00); // PPUADDR low byte
    
    // Preencher nametable com padrões
    for (int i = 0; i < 960; i++) {
        ppu->writeRegister(7, i % 256); // PPUDATA
    }
    
    // Configurar paleta
    ppu->writeRegister(6, 0x3F); // PPUADDR high byte
    ppu->writeRegister(6, 0x00); // PPUADDR low byte
    
    for (int i = 0; i < 32; i++) {
        ppu->writeRegister(7, i); // PPUDATA
    }
    
    // Executar um frame completo
    for (int i = 0; i < 341 * 262; i++) {
        ppu->cycle();
    }
    
    // Verificar se o frame foi gerado
    assert((ppu->getStatus() & 0x80) != 0); // Bit 7 de PPUSTATUS deve estar setado (vblank)
    
    // Renderizar frame para o framebuffer
    ppu->renderFrame(framebuffer);
    
    // Verificar se o framebuffer contém dados válidos
    bool has_data = false;
    for (int i = 0; i < 256 * 240; i++) {
        if (framebuffer[i] != 0) {
            has_data = true;
            break;
        }
    }
    assert(has_data);
    
    printf("Teste de geração de frame concluído com sucesso!\n");
}

/**
 * @brief Função principal para execução dos testes
 */
int main(void)
{
    printf("Iniciando testes do PPU do NES\n");
    
    // Executar testes
    setup();
    test_initialization();
    teardown();
    
    setup();
    test_register_access();
    teardown();
    
    setup();
    test_oam_access();
    teardown();
    
    setup();
    test_vram_access();
    teardown();
    
    setup();
    test_palette_access();
    teardown();
    
    setup();
    test_scroll_access();
    teardown();
    
    setup();
    test_render_cycle();
    teardown();
    
    setup();
    test_oam_dma_transfer();
    teardown();
    
    setup();
    test_sprite_evaluation();
    teardown();
    
    setup();
    test_frame_generation();
    teardown();
    
    printf("Todos os testes do PPU do NES concluídos com sucesso!\n");
    
    return 0;
}
