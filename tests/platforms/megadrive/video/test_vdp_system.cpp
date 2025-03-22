/**
 * @file test_vdp_system.cpp
 * @brief Testes unitários para o VDP (Video Display Processor) do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <gtest/gtest.h>
#include <memory>
#include <cstring>
#include <array>
#include "platforms/megadrive/video/vdp.hpp"

// Mock para o barramento de dados
struct MockBus {
    uint8_t data[0x10000];
    
    uint8_t read(uint32_t address) {
        return data[address & 0xFFFF];
    }
    
    void write(uint32_t address, uint8_t value) {
        data[address & 0xFFFF] = value;
    }
};

class MegaDriveVDPTest : public ::testing::Test {
protected:
    std::unique_ptr<MockBus> bus;
    std::unique_ptr<MegaEmu::Platforms::MegaDrive::VDP> vdp;
    
    void SetUp() override {
        bus = std::make_unique<MockBus>();
        
        // Inicializar memória com valores padrão
        std::memset(bus->data, 0, sizeof(bus->data));
        
        // Criar instância do VDP
        vdp = std::make_unique<MegaEmu::Platforms::MegaDrive::VDP>(
            static_cast<void*>(bus.get())
        );
    }
    
    void TearDown() override {
        vdp.reset();
        bus.reset();
    }
};

// Teste de inicialização
TEST_F(MegaDriveVDPTest, Initialization) {
    // Verificar valores iniciais dos registradores
    ASSERT_EQ(vdp->getMode1Register(), 0x04); // Modo 4 ativo por padrão
    ASSERT_EQ(vdp->getMode2Register(), 0x00);
    ASSERT_FALSE(vdp->isDisplayEnabled());
    ASSERT_FALSE(vdp->isVBlankInterruptEnabled());
    ASSERT_FALSE(vdp->isHBlankInterruptEnabled());
}

// Teste de escrita/leitura de registradores
TEST_F(MegaDriveVDPTest, RegisterAccess) {
    // Escrever valores nos registradores
    vdp->writeControlPort(0x8004); // Reg 0, valor 0x04
    vdp->writeControlPort(0x8144); // Reg 1, valor 0x44 (Display e VBlank IRQ habilitados)
    vdp->writeControlPort(0x8230); // Reg 2, valor 0x30 (Nametable A em 0xC000)
    
    // Verificar valores dos registradores
    ASSERT_EQ(vdp->getMode1Register(), 0x04);
    ASSERT_EQ(vdp->getMode2Register(), 0x44);
    ASSERT_TRUE(vdp->isDisplayEnabled());
    ASSERT_TRUE(vdp->isVBlankInterruptEnabled());
    ASSERT_FALSE(vdp->isHBlankInterruptEnabled());
    ASSERT_EQ(vdp->getNameTableAAddress(), 0xC000);
}

// Teste de acesso à VRAM
TEST_F(MegaDriveVDPTest, VRAMAccess) {
    // Configurar endereço VRAM para escrita
    vdp->writeControlPort(0x4000); // Endereço VRAM 0x0000, operação de escrita
    vdp->writeControlPort(0x0000);
    
    // Escrever dados na VRAM
    for (int i = 0; i < 16; i++) {
        vdp->writeDataPort(0x30 + i);
    }
    
    // Configurar endereço VRAM para leitura
    vdp->writeControlPort(0x0000); // Endereço VRAM 0x0000, operação de leitura
    vdp->writeControlPort(0x0000);
    
    // Verificar dados da VRAM
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(vdp->readDataPort(), 0x30 + i);
    }
}

// Teste de acesso à CRAM (Color RAM)
TEST_F(MegaDriveVDPTest, CRAMAccess) {
    // Configurar endereço CRAM para escrita
    vdp->writeControlPort(0xC000); // Endereço CRAM 0x0000, operação de escrita
    vdp->writeControlPort(0x0000);
    
    // Escrever dados na CRAM (cores)
    for (int i = 0; i < 64; i++) {
        vdp->writeDataPort(i * 2);
        vdp->writeDataPort(i * 2 + 1);
    }
    
    // Configurar endereço CRAM para leitura
    vdp->writeControlPort(0x0000); // Endereço CRAM 0x0000, operação de leitura
    vdp->writeControlPort(0x0000);
    
    // Verificar dados da CRAM
    for (int i = 0; i < 64; i++) {
        ASSERT_EQ(vdp->readDataPort(), i * 2);
        ASSERT_EQ(vdp->readDataPort(), i * 2 + 1);
    }
}

// Teste de acesso à VSRAM (Vertical Scroll RAM)
TEST_F(MegaDriveVDPTest, VSRAMAccess) {
    // Configurar endereço VSRAM para escrita
    vdp->writeControlPort(0x4000); // Endereço VSRAM 0x0000, operação de escrita
    vdp->writeControlPort(0x0010);
    
    // Escrever dados na VSRAM
    for (int i = 0; i < 40; i++) {
        vdp->writeDataPort(i);
        vdp->writeDataPort(0);
    }
    
    // Configurar endereço VSRAM para leitura
    vdp->writeControlPort(0x0000); // Endereço VSRAM 0x0000, operação de leitura
    vdp->writeControlPort(0x0010);
    
    // Verificar dados da VSRAM
    for (int i = 0; i < 40; i++) {
        ASSERT_EQ(vdp->readDataPort(), i);
        ASSERT_EQ(vdp->readDataPort(), 0);
    }
}

// Teste de status do VDP
TEST_F(MegaDriveVDPTest, StatusRegister) {
    // Ler registro de status
    uint16_t status = vdp->readStatusRegister();
    
    // Verificar bits iniciais do status
    ASSERT_FALSE(status & 0x0080); // Bit 7: VINT não ocorreu
    ASSERT_FALSE(status & 0x0040); // Bit 6: Sprite overflow não ocorreu
    ASSERT_FALSE(status & 0x0020); // Bit 5: Sprite collision não ocorreu
    ASSERT_TRUE(status & 0x0008);  // Bit 3: VBlank ativo no início
    
    // Simular um ciclo de renderização para sair do VBlank
    vdp->cycle(262 * 342); // Aproximadamente um frame
    
    // Verificar status após ciclo
    status = vdp->readStatusRegister();
    ASSERT_TRUE(status & 0x0080);  // Bit 7: VINT ocorreu após frame
}

// Teste de sistema de sprites
TEST_F(MegaDriveVDPTest, SpriteSystem) {
    // Configurar VDP para usar sprites
    vdp->writeControlPort(0x8144); // Reg 1, valor 0x44 (Display e VBlank IRQ habilitados)
    vdp->writeControlPort(0x8564); // Reg 5, valor 0x64 (Sprite table em 0x8800)
    
    // Configurar endereço da tabela de sprites para escrita
    vdp->writeControlPort(0x4000 | (0x8800 & 0x3FFF)); // Endereço 0x8800, operação de escrita
    vdp->writeControlPort(0x0000 | ((0x8800 & 0xC000) >> 14));
    
    // Escrever dados de sprite
    // Sprite 0: Y=100, Size=1x1, Link=1, X=120, Pattern=0x200
    vdp->writeDataPort(100);      // Y position
    vdp->writeDataPort(0);
    vdp->writeDataPort(0x01);     // Size and link
    vdp->writeDataPort(0);
    vdp->writeDataPort(120);      // X position
    vdp->writeDataPort(0);
    vdp->writeDataPort(0x20);     // Pattern
    vdp->writeDataPort(0);
    
    // Sprite 1: Y=150, Size=2x2, Link=0, X=200, Pattern=0x300
    vdp->writeDataPort(150);      // Y position
    vdp->writeDataPort(0);
    vdp->writeDataPort(0x0A);     // Size (2x2) and link (0)
    vdp->writeDataPort(0);
    vdp->writeDataPort(200);      // X position
    vdp->writeDataPort(0);
    vdp->writeDataPort(0x30);     // Pattern
    vdp->writeDataPort(0);
    
    // Executar ciclos para processar sprites
    vdp->cycle(100 * 342); // Ciclos até scanline 100
    
    // Verificar se o sprite 0 está ativo no scanline 100
    ASSERT_TRUE(vdp->isSpriteActiveOnScanline(0, 100));
    ASSERT_FALSE(vdp->isSpriteActiveOnScanline(0, 110)); // Fora do sprite
    
    // Verificar se o sprite 1 está ativo no scanline 150
    ASSERT_TRUE(vdp->isSpriteActiveOnScanline(1, 150));
    ASSERT_TRUE(vdp->isSpriteActiveOnScanline(1, 165)); // Dentro do sprite 2x2
    ASSERT_FALSE(vdp->isSpriteActiveOnScanline(1, 180)); // Fora do sprite
}

// Teste de DMA
TEST_F(MegaDriveVDPTest, DMATransfer) {
    // Preparar dados na memória para transferência DMA
    for (int i = 0; i < 1024; i++) {
        bus->data[0x2000 + i] = i & 0xFF;
    }
    
    // Configurar registradores de DMA
    vdp->writeControlPort(0x8F02); // Reg 15, valor 0x02 (Incremento automático 2)
    vdp->writeControlPort(0x9300); // Reg 19, valor 0x00 (DMA length high = 0)
    vdp->writeControlPort(0x9400); // Reg 20, valor 0x00 (DMA length low = 0)
    vdp->writeControlPort(0x9780); // Reg 23, valor 0x80 (DMA enabled)
    
    // Configurar endereço de destino na VRAM
    vdp->writeControlPort(0x4000); // Endereço VRAM 0x0000, operação de escrita
    vdp->writeControlPort(0x0000);
    
    // Iniciar transferência DMA da memória para VRAM
    vdp->writeControlPort(0x8700); // Reg 7, valor 0x00 (Background color)
    vdp->writeControlPort(0x9500); // Reg 21, valor 0x00 (DMA source address mid)
    vdp->writeControlPort(0x9620); // Reg 22, valor 0x20 (DMA source address high)
    vdp->writeControlPort(0x9780); // Reg 23, valor 0x80 (DMA enabled)
    
    // Configurar comprimento da transferência DMA
    vdp->writeControlPort(0x9300); // Reg 19, valor 0x00 (DMA length high = 0)
    vdp->writeControlPort(0x9404); // Reg 20, valor 0x04 (DMA length low = 4, total 1024 bytes)
    
    // Iniciar DMA
    vdp->startDMA();
    
    // Executar ciclos para completar DMA
    vdp->cycle(1000);
    
    // Verificar se os dados foram transferidos corretamente
    vdp->writeControlPort(0x0000); // Endereço VRAM 0x0000, operação de leitura
    vdp->writeControlPort(0x0000);
    
    for (int i = 0; i < 1024; i++) {
        ASSERT_EQ(vdp->readDataPort(), i & 0xFF);
    }
}

// Teste de interrupções
TEST_F(MegaDriveVDPTest, Interrupts) {
    // Configurar VDP para gerar interrupções
    vdp->writeControlPort(0x8144); // Reg 1, valor 0x44 (Display e VBlank IRQ habilitados)
    vdp->writeControlPort(0x8A10); // Reg 10, valor 0x10 (HBlank IRQ habilitado)
    
    // Executar ciclos para um frame completo
    bool vblankInterrupt = false;
    bool hblankInterrupt = false;
    
    for (int scanline = 0; scanline < 262; scanline++) {
        for (int pixel = 0; pixel < 342; pixel++) {
            vdp->cycle(1);
            
            // Verificar interrupções
            if (vdp->isVBlankInterruptPending()) {
                vblankInterrupt = true;
                vdp->acknowledgeVBlankInterrupt();
            }
            
            if (vdp->isHBlankInterruptPending()) {
                hblankInterrupt = true;
                vdp->acknowledgeHBlankInterrupt();
            }
        }
    }
    
    // Verificar se ambas as interrupções ocorreram
    ASSERT_TRUE(vblankInterrupt);
    ASSERT_TRUE(hblankInterrupt);
}

// Teste de geração de frame
TEST_F(MegaDriveVDPTest, FrameGeneration) {
    // Configurar VDP para renderização
    vdp->writeControlPort(0x8004); // Reg 0, valor 0x04 (Normal sync)
    vdp->writeControlPort(0x8144); // Reg 1, valor 0x44 (Display e VBlank IRQ habilitados)
    vdp->writeControlPort(0x8230); // Reg 2, valor 0x30 (Nametable A em 0xC000)
    vdp->writeControlPort(0x8407); // Reg 4, valor 0x07 (Nametable B em 0xE000)
    
    // Preencher nametable com padrões
    vdp->writeControlPort(0x4000 | (0xC000 & 0x3FFF)); // Endereço 0xC000, operação de escrita
    vdp->writeControlPort(0x0000 | ((0xC000 & 0xC000) >> 14));
    
    for (int i = 0; i < 1024; i++) {
        vdp->writeDataPort(i & 0xFF);
        vdp->writeDataPort((i >> 8) & 0x0F);
    }
    
    // Preencher pattern table com dados de teste
    vdp->writeControlPort(0x4000); // Endereço VRAM 0x0000, operação de escrita
    vdp->writeControlPort(0x0000);
    
    for (int i = 0; i < 8192; i++) {
        vdp->writeDataPort(i & 0xFF);
    }
    
    // Executar um frame completo
    for (int scanline = 0; scanline < 262; scanline++) {
        for (int pixel = 0; pixel < 342; pixel++) {
            vdp->cycle(1);
        }
    }
    
    // Obter o frame buffer gerado
    const uint32_t* frameBuffer = vdp->getFrameBuffer();
    
    // Verificar se o frame buffer não é nulo
    ASSERT_NE(frameBuffer, nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
