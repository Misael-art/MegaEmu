/**
 * @file test_nes_ppu.c
 * @brief Testes unitários para o PPU do NES (Picture Processing Unit)
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../../../src/platforms/nes/ppu/nes_ppu.hpp"

// Mocks e stubs necessários
typedef struct
{
    uint8_t data[0x10000];

    uint8_t read(uint16_t address)
    {
        return data[address];
    }

    void write(uint16_t address, uint8_t value)
    {
        data[address] = value;
    }
} MockMemory;

// Variáveis globais para os testes
static MockMemory *memory;
static MegaEmu::Platforms::NES::NESPPU *ppu;
static uint32_t framebuffer[256 * 240];

void setUp(void)
{
    // Alocar memória para os mocks
    memory = (MockMemory *)malloc(sizeof(MockMemory));

    // Inicializar memória com valores padrão
    memset(memory->data, 0, sizeof(memory->data));

    // Criar instância do PPU
    ppu = new MegaEmu::Platforms::NES::NESPPU();

    // Limpar framebuffer
    memset(framebuffer, 0, sizeof(framebuffer));
}

void tearDown(void)
{
    delete ppu;
    free(memory);
}

void test_initialization(void)
{
    // Verificar se o PPU foi inicializado corretamente
    TEST_ASSERT_NOT_NULL(ppu);

    // Verificar estado inicial
    ppu->initialize();
    ppu->reset();

    // Verificar se os registradores foram inicializados corretamente
    TEST_ASSERT_EQUAL_UINT8(0, ppu->getStatus()); // Status deve ser 0 após reset
}

void test_register_access(void)
{
    // Escrever em alguns registradores
    ppu->writeRegister(0, 0x1E); // PPUCTRL
    ppu->writeRegister(1, 0x08); // PPUMASK

    // Ler registradores
    uint8_t status = ppu->readRegister(2); // PPUSTATUS

    // Verificar se a leitura de status limpa o bit 7
    TEST_ASSERT_EQUAL_UINT8(0, status & 0x80);
}

void test_oam_access(void)
{
    // Testar escrita no OAM
    uint8_t sprite_data[] = {0x10, 0x20, 0x30, 0x40};
    ppu->writeOAM(0, sprite_data[0]);
    ppu->writeOAM(1, sprite_data[1]);
    ppu->writeOAM(2, sprite_data[2]);
    ppu->writeOAM(3, sprite_data[3]);

    // Verificar leitura do OAM
    TEST_ASSERT_EQUAL_UINT8(sprite_data[0], ppu->readOAM(0));
    TEST_ASSERT_EQUAL_UINT8(sprite_data[1], ppu->readOAM(1));
    TEST_ASSERT_EQUAL_UINT8(sprite_data[2], ppu->readOAM(2));
    TEST_ASSERT_EQUAL_UINT8(sprite_data[3], ppu->readOAM(3));
}

void test_vram_access(void)
{
    // Testar escrita na VRAM
    ppu->writeVRAM(0x2000, 0xAA);
    ppu->writeVRAM(0x2400, 0xBB);
    ppu->writeVRAM(0x2800, 0xCC);
    ppu->writeVRAM(0x2C00, 0xDD);

    // Verificar leitura da VRAM
    TEST_ASSERT_EQUAL_UINT8(0xAA, ppu->readVRAM(0x2000));
    TEST_ASSERT_EQUAL_UINT8(0xBB, ppu->readVRAM(0x2400));
    TEST_ASSERT_EQUAL_UINT8(0xCC, ppu->readVRAM(0x2800));
    TEST_ASSERT_EQUAL_UINT8(0xDD, ppu->readVRAM(0x2C00));
}

void test_palette_access(void)
{
    // Testar escrita na paleta
    ppu->writePalette(0, 0x3F);
    ppu->writePalette(1, 0x30);
    ppu->writePalette(2, 0x21);
    ppu->writePalette(3, 0x12);

    // Verificar leitura da paleta
    TEST_ASSERT_EQUAL_UINT8(0x3F, ppu->readPalette(0));
    TEST_ASSERT_EQUAL_UINT8(0x30, ppu->readPalette(1));
    TEST_ASSERT_EQUAL_UINT8(0x21, ppu->readPalette(2));
    TEST_ASSERT_EQUAL_UINT8(0x12, ppu->readPalette(3));
}

void test_scroll_access(void)
{
    // Testar configuração de scroll
    ppu->writeRegister(5, 0x20); // PPUSCROLL X
    ppu->writeRegister(5, 0x10); // PPUSCROLL Y

    // Verificar valores internos de scroll (através de métodos de teste específicos)
    TEST_ASSERT_EQUAL_UINT8(0x20, ppu->getScrollX());
    TEST_ASSERT_EQUAL_UINT8(0x10, ppu->getScrollY());
}

void test_render_cycle(void)
{
    // Configurar PPU para renderização
    ppu->writeRegister(0, 0x80); // PPUCTRL - NMI habilitado
    ppu->writeRegister(1, 0x1E); // PPUMASK - Renderização habilitada

    // Executar alguns ciclos
    for (int i = 0; i < 100; i++)
    {
        ppu->tick();
    }

    // Verificar estado após ciclos
    uint8_t status = ppu->readRegister(2);
    TEST_ASSERT_NOT_EQUAL(0, status & 0x40); // Verificar bit de sprite hit
}

void test_oam_dma_transfer(void)
{
    // Preparar dados para DMA
    uint8_t dma_data[256];
    for (int i = 0; i < 256; i++)
    {
        dma_data[i] = i;
    }

    // Executar DMA
    ppu->doDMA(dma_data);

    // Verificar se os dados foram transferidos corretamente
    for (int i = 0; i < 256; i++)
    {
        TEST_ASSERT_EQUAL_UINT8(dma_data[i], ppu->readOAM(i));
    }
}

void test_sprite_evaluation(void)
{
    // Configurar sprites para teste
    uint8_t sprite_data[] = {
        0x10, 0x01, 0x02, 0x03, // Sprite 0
        0x20, 0x04, 0x05, 0x06, // Sprite 1
        0x30, 0x07, 0x08, 0x09  // Sprite 2
    };

    // Carregar sprites no OAM
    for (int i = 0; i < sizeof(sprite_data); i++)
    {
        ppu->writeOAM(i, sprite_data[i]);
    }

    // Executar avaliação de sprites
    ppu->evaluateSprites();

    // Verificar resultados da avaliação
    TEST_ASSERT_EQUAL_UINT8(3, ppu->getSpriteCount());
    TEST_ASSERT_EQUAL_UINT8(0, ppu->getSpriteOverflow());
}

void test_frame_generation(void)
{
    // Configurar PPU para renderização
    ppu->writeRegister(0, 0x80); // PPUCTRL - NMI habilitado
    ppu->writeRegister(1, 0x1E); // PPUMASK - Renderização habilitada

    // Executar ciclos suficientes para um frame
    for (int i = 0; i < 89342; i++)
    { // Ciclos em um frame NTSC
        ppu->tick();
    }

    // Verificar se o frame foi gerado
    TEST_ASSERT_EQUAL(1, ppu->getFrameCount());

    // Verificar estado do VBlank
    uint8_t status = ppu->readRegister(2);
    TEST_ASSERT_NOT_EQUAL(0, status & 0x80);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_initialization);
    RUN_TEST(test_register_access);
    RUN_TEST(test_oam_access);
    RUN_TEST(test_vram_access);
    RUN_TEST(test_palette_access);
    RUN_TEST(test_scroll_access);
    RUN_TEST(test_render_cycle);
    RUN_TEST(test_oam_dma_transfer);
    RUN_TEST(test_sprite_evaluation);
    RUN_TEST(test_frame_generation);

    return UNITY_END();
}
