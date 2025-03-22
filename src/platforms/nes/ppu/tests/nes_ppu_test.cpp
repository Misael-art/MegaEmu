#include <catch2/catch.hpp>#include "nes_ppu.hpp"#include <chrono>using namespace MegaEmu::Platforms::NES;TEST_CASE("PPU Initialization", "[ppu]"){    NESGPU ppu;    SECTION("Default State")    {        REQUIRE(ppu.getStatus() == 0);        REQUIRE(ppu.getControl() == 0);        REQUIRE(ppu.getMask() == 0);    }}TEST_CASE("PPU Register Access", "[ppu]"){    NESGPU ppu;    SECTION("Control Register")    {        ppu.writeControl(0x80);        REQUIRE(ppu.getControl() == 0x80);    }    SECTION("Mask Register")    {        ppu.writeMask(0x1E);        REQUIRE(ppu.getMask() == 0x1E);    }}TEST_CASE("PPU Memory Operations", "[ppu]"){    NESGPU ppu;    SECTION("VRAM Write/Read")    {        ppu.writeVRAM(0x2000, 0x55);        REQUIRE(ppu.readVRAM(0x2000) == 0x55);    }    SECTION("OAM Write/Read")    {        ppu.writeOAM(0, 0xAA);        REQUIRE(ppu.readOAM(0) == 0xAA);    }}TEST_CASE("PPU Rendering", "[ppu]"){    NESGPU ppu;    SECTION("Background Rendering")    {        ppu.writeControl(0x08); // Enable background        ppu.writeMask(0x0E);    // Show background        // Simular um ciclo de renderização        ppu.tick(341);        REQUIRE(ppu.isInVBlank() == false);    }    SECTION("Sprite Rendering")    {        ppu.writeControl(0x10); // Enable sprites        ppu.writeMask(0x10);    // Show sprites        // Configurar um sprite        ppu.writeOAM(0, 0x10); // Y position        ppu.writeOAM(1, 0x01); // Tile index        ppu.writeOAM(2, 0x00); // Attributes        ppu.writeOAM(3, 0x10); // X position        // Simular um ciclo de renderização        ppu.tick(341);        REQUIRE(ppu.getSpriteCount() > 0);    }}TEST_CASE("PPU Tile Operations", "[ppu][tiles]"){    NESGPU ppu;    SECTION("Tile Pattern Loading")    {        // Configurar padrão de teste em VRAM        for (int i = 0; i < 16; ++i)        {            ppu.writeVRAM(i, 0x55); // Padrão alternado 01010101        }        Tile tile = ppu.getTile(0x0000, 0);        // Verificar padrão do tile        for (int y = 0; y < 8; ++y)        {            for (int x = 0; x < 8; ++x)            {                uint8_t pixel = tile.getPixel(x, y);                REQUIRE(pixel == ((x % 2) == 0 ? 1 : 0));            }        }    }    SECTION("Tile Flipping")    {        // Configurar um padrão assimétrico para teste        ppu.writeVRAM(0, 0xFF); // Primeira linha toda preenchida        ppu.writeVRAM(8, 0x00); // Bit plano 1 vazio        Tile tile = ppu.getTile(0x0000, 0);        // Testar renderização normal        uint32_t frameBuffer[64 * 64] = {0};        ppu.renderTile(tile, 0, 0, 0, false, false);        // Testar flip horizontal        ppu.renderTile(tile, 8, 0, 0, true, false);        // Testar flip vertical        ppu.renderTile(tile, 0, 8, 0, false, true);        // Verificar resultados        REQUIRE(frameBuffer[0] != 0);      // Primeiro pixel deve estar preenchido        REQUIRE(frameBuffer[7] != 0);      // Último pixel da primeira linha        REQUIRE(frameBuffer[8 * 64] != 0); // Primeiro pixel da linha 8    }}TEST_CASE("PPU Palette Operations", "[ppu][palette]"){    NESGPU ppu;    SECTION("Palette Writing and Reading")    {        // Escrever valores na paleta        for (int i = 0; i < 32; ++i)        {            ppu.writeVRAM(0x3F00 + i, i);        }        // Verificar leitura da paleta        for (int i = 0; i < 32; ++i)        {            REQUIRE(ppu.readVRAM(0x3F00 + i) == i);        }    }    SECTION("Palette Mirroring")    {        // Testar espelhamento da paleta universal (índice 0)        ppu.writeVRAM(0x3F00, 0x30); // Paleta universal        REQUIRE(ppu.readVRAM(0x3F10) == 0x30);        REQUIRE(ppu.readVRAM(0x3F14) == ppu.readVRAM(0x3F04));    }}TEST_CASE("PPU Sprite Evaluation", "[ppu][sprites]"){    NESGPU ppu;    SECTION("Sprite Zero Hit")    {        // Configurar sprite 0        ppu.writeOAM(0, 50); // Y        ppu.writeOAM(1, 0);  // Tile index        ppu.writeOAM(2, 0);  // Attributes        ppu.writeOAM(3, 50); // X        // Configurar padrão do tile        for (int i = 0; i < 16; ++i)        {            ppu.writeVRAM(i, 0xFF); // Tile totalmente preenchido        }        // Habilitar sprites        ppu.writeControl(0x10);        ppu.writeMask(0x10);        // Simular renderização        ppu.tick(341 * 51); // Avançar até a linha do sprite        // Verificar sprite zero hit        REQUIRE((ppu.getStatus() & 0x40) != 0);    }    SECTION("Sprite Overflow")    {        // Configurar 9 sprites na mesma linha        for (int i = 0; i < 9; ++i)        {            ppu.writeOAM(i * 4 + 0, 100);   // Y            ppu.writeOAM(i * 4 + 1, i);     // Tile index            ppu.writeOAM(i * 4 + 2, 0);     // Attributes            ppu.writeOAM(i * 4 + 3, i * 8); // X        }        // Habilitar sprites        ppu.writeControl(0x10);        ppu.writeMask(0x10);        // Avaliar sprites        ppu.evaluateSprites();        // Verificar overflow        REQUIRE((ppu.getStatus() & 0x20) != 0);        REQUIRE(ppu.getSpriteCount() == 8);    }}TEST_CASE("PPU CPU Integration", "[ppu][cpu]"){    NESGPU ppu;    bool nmiCalled = false;    uint8_t cpuMemory[0x800] = {0};    NESGPU::CPUInterface cpuInterface;    cpuInterface.cpuMemory = cpuMemory;    cpuInterface.nmiCallback = [&nmiCalled]()    { nmiCalled = true; };    ppu.connectCPU(cpuInterface);    SECTION("NMI Generation")    {        // Habilitar NMI        ppu.writeRegister(0x2000, 0x80);        REQUIRE(ppu.isNMIPending() == false);        // Simular VBlank        ppu.tick(341 * 241); // Avançar até linha 241        REQUIRE(ppu.isNMIPending() == true);        REQUIRE(nmiCalled == true);    }    SECTION("OAM DMA Transfer")    {        // Preparar dados para DMA        for (int i = 0; i < 256; i++)        {            cpuMemory[i] = i;        }        // Iniciar DMA        ppu.startOAMDMA(cpuMemory);        REQUIRE(ppu.isDMAActive() == true);        // Simular ciclos DMA        for (int i = 0; i < 512; i++)        { // 256 bytes * 2 ciclos por byte            ppu.tickDMA();        }        REQUIRE(ppu.isDMAActive() == false);        // Verificar transferência        for (int i = 0; i < 256; i++)        {            REQUIRE(ppu.readOAM(i) == i);        }    }}TEST_CASE("PPU Register Access", "[ppu][registers]"){    NESGPU ppu;    SECTION("PPUCTRL ($2000)")    {        ppu.writeRegister(0x2000, 0x80);        REQUIRE(ppu.readRegister(0x2000) == 0x80);    }    SECTION("PPUMASK ($2001)")    {        ppu.writeRegister(0x2001, 0x1E);        REQUIRE(ppu.readRegister(0x2001) == 0x1E);    }    SECTION("PPUSTATUS ($2002)")    {        // Simular VBlank        ppu.tick(341 * 241);        uint8_t status = ppu.readRegister(0x2002);        REQUIRE((status & 0x80) != 0); // VBlank flag set        // Segunda leitura deve limpar VBlank        status = ppu.readRegister(0x2002);        REQUIRE((status & 0x80) == 0);    }    SECTION("OAMADDR/OAMDATA ($2003/$2004)")    {        ppu.writeRegister(0x2003, 0x10); // Set OAM address        ppu.writeRegister(0x2004, 0x55); // Write OAM data        REQUIRE(ppu.readRegister(0x2004) == 0x55);    }    SECTION("PPUSCROLL ($2005)")    {        ppu.writeRegister(0x2005, 0x20); // X scroll        ppu.writeRegister(0x2005, 0x30); // Y scroll        // Verificar efeito do scroll na renderização        ppu.writeRegister(0x2001, 0x08); // Enable background        ppu.renderFrame(nullptr);        // A verificação real seria feita observando o frame buffer    }    SECTION("PPUADDR/PPUDATA ($2006/$2007)")    {        ppu.writeRegister(0x2006, 0x23); // High byte        ppu.writeRegister(0x2006, 0x00); // Low byte        ppu.writeRegister(0x2007, 0xAA); // Write data        // Reset address        ppu.writeRegister(0x2006, 0x23);        ppu.writeRegister(0x2006, 0x00);        REQUIRE(ppu.readRegister(0x2007) == 0xAA);    }}TEST_CASE("PPU Timing", "[ppu][timing]"){    NESGPU ppu;    SECTION("Scanline Timing")    {        // Verificar ciclos por scanline        for (int i = 0; i < NESGPU::CYCLES_PER_SCANLINE; ++i)        {            ppu.tickPPU();        }        REQUIRE(ppu.getCurrentScanline() == 1);        REQUIRE(ppu.getCurrentCycle() == 0);    }    SECTION("Frame Timing")    {        // Simular um frame completo        for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)        {            for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)            {                ppu.tickPPU();            }        }        REQUIRE(ppu.getCurrentScanline() == 0);        REQUIRE(ppu.getCurrentCycle() == 0);    }    SECTION("CPU Synchronization")    {        // Verificar sincronização com CPU        for (int i = 0; i < NESGPU::CYCLES_PER_CPU; ++i)        {            ppu.tickPPU();        }        REQUIRE(ppu.getCPUCycles() == 1);    }    SECTION("Render States")    {        // Verificar estados de renderização        REQUIRE(ppu.getRenderState() == NESGPU::RenderState::Visible);        // Avançar até VBlank        for (int i = 0; i < NESGPU::CYCLES_PER_SCANLINE * NESGPU::VBLANK_START_SCANLINE; ++i)        {            ppu.tickPPU();        }        REQUIRE(ppu.getRenderState() == NESGPU::RenderState::VBlank);        // Avançar até PreRender        for (int i = 0; i < NESGPU::CYCLES_PER_SCANLINE * (NESGPU::PRE_RENDER_SCANLINE - NESGPU::VBLANK_START_SCANLINE); ++i)        {            ppu.tickPPU();        }        REQUIRE(ppu.getRenderState() == NESGPU::RenderState::PreRender);    }    SECTION("Odd Frame Skip")    {        // Habilitar background rendering        ppu.writeRegister(0x2001, 0x08);        // Simular um frame completo        for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)        {            for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)            {                ppu.tickPPU();            }        }        // No próximo frame (ímpar), deve pular um ciclo        ppu.tickPPU();        REQUIRE(ppu.getCurrentCycle() == 1);    }    SECTION("VBlank Timing")    {        // Avançar até VBlank        for (int i = 0; i < NESGPU::CYCLES_PER_SCANLINE * NESGPU::VBLANK_START_SCANLINE + 1; ++i)        {            ppu.tickPPU();        }        // Verificar flag VBlank e NMI        REQUIRE((ppu.getStatus() & 0x80) != 0);        ppu.writeRegister(0x2000, 0x80); // Habilitar NMI        REQUIRE(ppu.isNMIPending() == true);    }}TEST_CASE("PPU Performance Timing", "[ppu][timing][performance]"){    NESGPU ppu;    const int FRAMES_TO_TEST = 10;    SECTION("Frame Rate Timing")    {        // Simular múltiplos frames        for (int frame = 0; frame < FRAMES_TO_TEST; ++frame)        {            for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)            {                for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)                {                    ppu.tickPPU();                }            }        }        // Verificar contagem de frames        REQUIRE(ppu.getFrameCount() == FRAMES_TO_TEST);    }    SECTION("CPU Cycles per Frame")    {        int totalCPUCycles = 0;        // Simular um frame completo        for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)        {            for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)            {                ppu.tickPPU();                totalCPUCycles += ppu.getCPUCycles();            }        }        // Verificar número aproximado de ciclos CPU (deve ser próximo a 29780)        REQUIRE(totalCPUCycles >= 29780 - 100);        REQUIRE(totalCPUCycles <= 29780 + 100);    }}TEST_CASE("PPU Performance Optimizations", "[ppu][performance]"){    NESGPU ppu;    SECTION("Tile Cache Performance")    {        // Preparar dados de teste        for (int i = 0; i < 256; ++i)        {            for (int j = 0; j < 16; ++j)            {                ppu.writeVRAM(i * 16 + j, static_cast<uint8_t>(i + j));            }        }        auto start = std::chrono::high_resolution_clock::now();        // Testar acesso repetido aos mesmos tiles        for (int i = 0; i < 1000; ++i)        {            uint8_t tileIndex = i % 256;            Tile &tile = ppu.getCachedTile(0x0000, tileIndex);            REQUIRE(tile.data[0] == static_cast<uint8_t>(tileIndex));        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar se o cache está melhorando a performance        REQUIRE(duration.count() < 1000); // Menos de 1ms para 1000 acessos    }    SECTION("Scanline Rendering Performance")    {        // Configurar PPU para renderização        ppu.writeRegister(0x2000, 0x08); // Enable background        ppu.writeRegister(0x2001, 0x1E); // Show background and sprites        // Preencher nametable com padrão de teste        for (int i = 0; i < 32 * 30; ++i)        {            ppu.writeVRAM(0x2000 + i, static_cast<uint8_t>(i % 256));        }        auto start = std::chrono::high_resolution_clock::now();        // Renderizar múltiplas scanlines        for (int scanline = 0; scanline < NESGPU::SCREEN_HEIGHT; ++scanline)        {            ppu.renderScanline();        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance da renderização        REQUIRE(duration.count() < 16667); // Menos de 1/60 segundo    }    SECTION("Sprite Evaluation Performance")    {        // Configurar sprites para teste        for (int i = 0; i < 64; ++i)        {            ppu.writeOAM(i * 4 + 0, i * 2); // Y position            ppu.writeOAM(i * 4 + 1, i);     // Tile index            ppu.writeOAM(i * 4 + 2, 0);     // Attributes            ppu.writeOAM(i * 4 + 3, i * 2); // X position        }        auto start = std::chrono::high_resolution_clock::now();        // Avaliar sprites para múltiplas scanlines        for (int scanline = 0; scanline < NESGPU::SCREEN_HEIGHT; ++scanline)        {            auto eval = ppu.evaluateSpritesForScanline(scanline);            REQUIRE(eval.count <= 8); // Máximo 8 sprites por scanline        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance da avaliação de sprites        REQUIRE(duration.count() < 1000); // Menos de 1ms para todas as scanlines    }    SECTION("Memory Access Patterns")    {        auto start = std::chrono::high_resolution_clock::now();        // Simular padrões de acesso à memória típicos        for (int i = 0; i < 1000; ++i)        {            // Padrão de acesso à nametable            uint16_t addr = 0x2000 + (i % 0x3C0);            ppu.writeVRAM(addr, static_cast<uint8_t>(i));            REQUIRE(ppu.readVRAM(addr) == static_cast<uint8_t>(i));            // Padrão de acesso à attribute table            addr = 0x23C0 + (i % 0x40);            ppu.writeVRAM(addr, static_cast<uint8_t>(i));            REQUIRE(ppu.readVRAM(addr) == static_cast<uint8_t>(i));            // Padrão de acesso à pattern table            addr = (i % 0x1000);            ppu.writeVRAM(addr, static_cast<uint8_t>(i));            REQUIRE(ppu.readVRAM(addr) == static_cast<uint8_t>(i));        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance dos acessos à memória        REQUIRE(duration.count() < 5000); // Menos de 5ms para todos os acessos    }}TEST_CASE("PPU Frame Timing Optimization", "[ppu][timing][performance]"){    NESGPU ppu;    SECTION("Frame Rendering Time")    {        // Configurar PPU para renderização completa        ppu.writeRegister(0x2000, 0x88); // Enable NMI and background        ppu.writeRegister(0x2001, 0x1E); // Show background and sprites        auto start = std::chrono::high_resolution_clock::now();        // Simular um frame completo        for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)        {            for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)            {                ppu.tickPPU();            }        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar tempo total de frame        REQUIRE(duration.count() < 16667); // Menos de 1/60 segundo    }    SECTION("CPU Synchronization Overhead")    {        bool nmiCalled = false;        uint8_t cpuMemory[0x800] = {0};        NESGPU::CPUInterface cpuInterface;        cpuInterface.cpuMemory = cpuMemory;        cpuInterface.nmiCallback = [&nmiCalled]()        { nmiCalled = true; };        ppu.connectCPU(cpuInterface);        auto start = std::chrono::high_resolution_clock::now();        // Simular sincronização CPU-PPU        for (int i = 0; i < 1000; ++i)        {            ppu.synchronizeCPU();        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar overhead de sincronização        REQUIRE(duration.count() < 100); // Menos de 0.1ms para 1000 sincronizações    }}TEST_CASE("PPU Advanced Optimizations", "[ppu][optimization]"){    NESGPU ppu;    SECTION("Prefetch System")    {        // Configurar PPU para renderização        ppu.writeRegister(0x2000, 0x08); // Enable background        ppu.writeRegister(0x2001, 0x1E); // Show background and sprites        // Preencher nametable com padrão de teste        for (int i = 0; i < 32 * 30; ++i)        {            ppu.writeVRAM(0x2000 + i, static_cast<uint8_t>(i));        }        // Preencher attribute table        for (int i = 0; i < 64; ++i)        {            ppu.writeVRAM(0x23C0 + i, static_cast<uint8_t>(i));        }        auto start = std::chrono::high_resolution_clock::now();        // Simular renderização com prefetch        for (int scanline = 0; scanline < NESGPU::SCREEN_HEIGHT; ++scanline)        {            ppu.renderScanline();        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance com prefetch        REQUIRE(duration.count() < 8333); // Menos de 1/120 segundo    }    SECTION("Attribute Cache")    {        // Configurar attribute table        for (int i = 0; i < 64; ++i)        {            ppu.writeVRAM(0x23C0 + i, static_cast<uint8_t>(i));        }        auto start = std::chrono::high_resolution_clock::now();        // Testar múltiplos acessos aos mesmos atributos        for (int i = 0; i < 1000; ++i)        {            uint16_t addr = 0x23C0 + (i % 64);            uint8_t value = ppu.readVRAM(addr);            REQUIRE(value == static_cast<uint8_t>(i % 64));        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance do cache de atributos        REQUIRE(duration.count() < 500); // Menos de 0.5ms para 1000 acessos    }    SECTION("Memory Access Patterns")    {        auto start = std::chrono::high_resolution_clock::now();        // Testar padrões otimizados de acesso à memória        for (int scanline = 0; scanline < NESGPU::SCREEN_HEIGHT; ++scanline)        {            // Simular acesso sequencial à nametable            for (int x = 0; x < 32; ++x)            {                uint16_t addr = 0x2000 + scanline * 32 + x;                ppu.writeVRAM(addr, static_cast<uint8_t>(x));                REQUIRE(ppu.readVRAM(addr) == static_cast<uint8_t>(x));            }            // Simular acesso aos atributos            uint16_t attrAddr = 0x23C0 + (scanline / 32) * 8;            ppu.writeVRAM(attrAddr, static_cast<uint8_t>(scanline));            REQUIRE(ppu.readVRAM(attrAddr) == static_cast<uint8_t>(scanline));        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance dos padrões de acesso        REQUIRE(duration.count() < 10000); // Menos de 10ms para um frame completo    }    SECTION("Combined Optimizations")    {        // Configurar PPU        ppu.writeRegister(0x2000, 0x88); // Enable NMI and background        ppu.writeRegister(0x2001, 0x1E); // Show background and sprites        // Preencher dados de teste        for (int i = 0; i < NESGPU::VRAM_SIZE; ++i)        {            ppu.writeVRAM(i, static_cast<uint8_t>(i));        }        auto start = std::chrono::high_resolution_clock::now();        // Simular frame completo com todas as otimizações        for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)        {            for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)            {                ppu.tickPPU();            }        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance geral        REQUIRE(duration.count() < 16667); // Manter 60 FPS    }}TEST_CASE("PPU Otimizações Avançadas v2", "[ppu][otimizacao]"){    NESGPU ppu;    SECTION("Cache de Tiles Otimizado")    {        // Configurar PPU        ppu.writeRegister(0x2000, 0x08); // Habilitar background        ppu.writeRegister(0x2001, 0x1E); // Mostrar background e sprites        // Preencher pattern table com dados de teste        for (int i = 0; i < 256; ++i)        {            for (int j = 0; j < 16; ++j)            {                ppu.writeVRAM(i * 16 + j, static_cast<uint8_t>(i + j));            }        }        auto start = std::chrono::high_resolution_clock::now();        // Testar acesso repetido aos mesmos tiles        for (int i = 0; i < 1000; ++i)        {            uint8_t tileIndex = i % 256;            Tile tile = ppu.getTile(0x0000, tileIndex);            REQUIRE(tile.data[0] == static_cast<uint8_t>(tileIndex));        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance do cache        REQUIRE(duration.count() < 500);       // Menos de 0.5ms para 1000 acessos        REQUIRE(ppu.getCacheHitRate() > 0.90); // Taxa de hit > 90%    }    SECTION("Sistema de Prefetch Otimizado")    {        // Configurar dados de teste        for (int i = 0; i < 32 * 30; ++i)        {            ppu.writeVRAM(0x2000 + i, static_cast<uint8_t>(i));        }        auto start = std::chrono::high_resolution_clock::now();        // Simular renderização com prefetch        for (int scanline = 0; scanline < NESGPU::SCREEN_HEIGHT; ++scanline)        {            ppu.renderScanline();        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar eficiência do prefetch        REQUIRE(duration.count() < 8333);         // Menos de 1/120 segundo        REQUIRE(ppu.getPrefetchHitRate() > 0.85); // Taxa de acerto > 85%    }    SECTION("Performance Geral")    {        // Configurar PPU para teste completo        ppu.writeRegister(0x2000, 0x88); // Habilitar NMI e background        ppu.writeRegister(0x2001, 0x1E); // Mostrar background e sprites        // Preencher dados de teste        for (int i = 0; i < NESGPU::VRAM_SIZE; ++i)        {            ppu.writeVRAM(i, static_cast<uint8_t>(i));        }        auto start = std::chrono::high_resolution_clock::now();        // Simular frame completo        for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)        {            for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)            {                ppu.tickPPU();            }        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar métricas de performance        REQUIRE(duration.count() < 16667);                            // Manter 60 FPS        REQUIRE(ppu.getAverageRenderTime() < 100);                    // Menos de 100μs por scanline        REQUIRE(ppu.getTotalCacheHits() > ppu.getTotalCacheMisses()); // Mais hits que misses    }    SECTION("Stress Test")    {        // Configurar PPU        ppu.writeRegister(0x2000, 0x88);        ppu.writeRegister(0x2001, 0x1E);        auto start = std::chrono::high_resolution_clock::now();        // Simular múltiplos frames com padrões variados        for (int frame = 0; frame < 10; ++frame)        {            // Alterar padrões a cada frame            for (int i = 0; i < 256; ++i)            {                ppu.writeVRAM(0x2000 + i, static_cast<uint8_t>(frame + i));            }            // Renderizar frame            for (int scanline = 0; scanline <= NESGPU::PRE_RENDER_SCANLINE; ++scanline)            {                for (int cycle = 0; cycle < NESGPU::CYCLES_PER_SCANLINE; ++cycle)                {                    ppu.tickPPU();                }            }        }        auto end = std::chrono::high_resolution_clock::now();        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);        // Verificar performance sob stress        REQUIRE(duration.count() < 166670); // 10 frames em menos de 1/6 segundo        REQUIRE(ppu.getStressTestMetrics().averageFPS >= 60.0);    }}