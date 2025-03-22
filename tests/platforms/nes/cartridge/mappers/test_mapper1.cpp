/**
 * @file test_mapper1.cpp
 * @brief Testes unitários para o Mapper 1 (MMC1) do NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <cstdio>
#include <cstring>
#include <cassert>
#include "../../../../../src/platforms/nes/cartridge/mappers/mapper1.hpp"
#include "../../../../../src/utils/test_utils.h"

// Tamanhos de teste
#define PRG_ROM_SIZE_32K (32 * 1024)
#define PRG_ROM_SIZE_64K (64 * 1024)
#define PRG_ROM_SIZE_128K (128 * 1024)
#define PRG_ROM_SIZE_256K (256 * 1024)
#define CHR_ROM_SIZE_8K (8 * 1024)
#define CHR_ROM_SIZE_16K (16 * 1024)
#define CHR_ROM_SIZE_32K (32 * 1024)

/**
 * @brief Cria um cartucho de teste
 * @param prg_size Tamanho da PRG-ROM
 * @param chr_size Tamanho da CHR-ROM
 * @return Ponteiro para o cartucho criado
 */
static Cartridge* create_test_cartridge(uint32_t prg_size, uint32_t chr_size)
{
    Cartridge* cart = new Cartridge();
    
    // Alocar e inicializar PRG-ROM
    cart->prg_rom_size = prg_size;
    cart->prg_rom = new uint8_t[prg_size];
    
    // Preencher PRG-ROM com padrão de teste
    for (uint32_t i = 0; i < prg_size; i++) {
        cart->prg_rom[i] = i & 0xFF;
    }
    
    // Alocar e inicializar CHR-ROM
    cart->chr_rom_size = chr_size;
    cart->chr_rom = new uint8_t[chr_size];
    
    // Preencher CHR-ROM com padrão de teste
    for (uint32_t i = 0; i < chr_size; i++) {
        cart->chr_rom[i] = (i + 0x80) & 0xFF;
    }
    
    // Sem CHR-RAM por padrão
    cart->chr_ram = nullptr;
    cart->chr_ram_size = 0;
    
    return cart;
}

/**
 * @brief Libera um cartucho de teste
 * @param cart Ponteiro para o cartucho
 */
static void free_test_cartridge(Cartridge* cart)
{
    if (cart) {
        if (cart->prg_rom) {
            delete[] cart->prg_rom;
        }
        
        if (cart->chr_rom) {
            delete[] cart->chr_rom;
        }
        
        if (cart->chr_ram) {
            delete[] cart->chr_ram;
        }
        
        delete cart;
    }
}

/**
 * @brief Testa a inicialização do Mapper 1
 */
void test_mapper1_init()
{
    printf("Testando inicialização do Mapper 1...\n");
    
    // Criar cartucho de teste com 32K PRG-ROM e 8K CHR-ROM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_8K);
    
    // Criar e inicializar mapper
    Mapper1* mapper = new Mapper1();
    bool result = mapper->init(cart);
    
    // Verificar resultado
    assert(result);
    assert(mapper->get_prg_bank_count() == 2);
    assert(mapper->get_chr_bank_count() == 1);
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    // Testar com 128K PRG-ROM e 32K CHR-ROM
    cart = create_test_cartridge(PRG_ROM_SIZE_128K, CHR_ROM_SIZE_32K);
    
    mapper = new Mapper1();
    result = mapper->init(cart);
    
    // Verificar resultado
    assert(result);
    assert(mapper->get_prg_bank_count() == 8);
    assert(mapper->get_chr_bank_count() == 4);
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de inicialização concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura da CPU no Mapper 1
 */
void test_mapper1_cpu_read()
{
    printf("Testando leitura da CPU no Mapper 1...\n");
    
    // Criar cartucho de teste com 64K PRG-ROM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_64K, CHR_ROM_SIZE_8K);
    
    Mapper1* mapper = new Mapper1();
    mapper->init(cart);
    
    // Testar leitura no modo padrão (PRG Mode 3: fix last bank)
    // Primeiro banco (0x8000-0xBFFF) - variável
    for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
        uint32_t addr = 0x8000 + i;
        uint8_t value = mapper->cpu_read(addr);
        uint8_t expected = i & 0xFF; // Primeiro banco (0)
        assert(value == expected);
    }
    
    // Segundo banco (0xC000-0xFFFF) - fixo no último
    for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
        uint32_t addr = 0xC000 + i;
        uint8_t value = mapper->cpu_read(addr);
        uint8_t expected = (i + 0x4000) & 0xFF; // Último banco (1)
        assert(value == expected);
    }
    
    // Mudar para PRG Mode 2: fix first bank
    // Escrever no registro de controle (0x8000-0xFFFF)
    // Bit 2-3: 01 (PRG Mode 2)
    mapper->cpu_write(0x8000, 0x01); // Bit 0
    mapper->cpu_write(0x8000, 0x01); // Bit 1
    mapper->cpu_write(0x8000, 0x00); // Bit 2
    mapper->cpu_write(0x8000, 0x01); // Bit 3
    mapper->cpu_write(0x8000, 0x00); // Bit 4
    
    // Primeiro banco (0x8000-0xBFFF) - fixo no primeiro
    for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
        uint32_t addr = 0x8000 + i;
        uint8_t value = mapper->cpu_read(addr);
        uint8_t expected = i & 0xFF; // Primeiro banco (0)
        assert(value == expected);
    }
    
    // Segundo banco (0xC000-0xFFFF) - variável
    for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
        uint32_t addr = 0xC000 + i;
        uint8_t value = mapper->cpu_read(addr);
        uint8_t expected = (i + 0x4000) & 0xFF; // Banco 1 (padrão)
        assert(value == expected);
    }
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de leitura da CPU concluído com sucesso!\n");
}

/**
 * @brief Testa a escrita da CPU no Mapper 1
 */
void test_mapper1_cpu_write()
{
    printf("Testando escrita da CPU no Mapper 1...\n");
    
    // Criar cartucho de teste com 128K PRG-ROM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_128K, CHR_ROM_SIZE_8K);
    
    Mapper1* mapper = new Mapper1();
    mapper->init(cart);
    
    // Testar escrita no registro de controle (0x8000-0x9FFF)
    // Escrever sequencialmente para configurar o registro
    // Bit 7 = 1 (reset), Bits 0-4 = valor
    mapper->cpu_write(0x8000, 0x80); // Reset
    
    // Escrever novo valor: 0x0C (PRG Mode 0, CHR Mode 0)
    mapper->cpu_write(0x8000, 0x00); // Bit 0
    mapper->cpu_write(0x8000, 0x00); // Bit 1
    mapper->cpu_write(0x8000, 0x01); // Bit 2
    mapper->cpu_write(0x8000, 0x01); // Bit 3
    mapper->cpu_write(0x8000, 0x00); // Bit 4
    
    // Testar escrita no registro CHR Bank 0 (0xA000-0xBFFF)
    mapper->cpu_write(0xA000, 0x80); // Reset
    
    // Escrever novo valor: 0x01 (selecionar segundo banco CHR)
    mapper->cpu_write(0xA000, 0x01); // Bit 0
    mapper->cpu_write(0xA000, 0x00); // Bit 1
    mapper->cpu_write(0xA000, 0x00); // Bit 2
    mapper->cpu_write(0xA000, 0x00); // Bit 3
    mapper->cpu_write(0xA000, 0x00); // Bit 4
    
    // Verificar leitura da PPU após a mudança
    uint8_t value = mapper->ppu_read(0x0000);
    uint8_t expected = (0x1000 + 0x80) & 0xFF; // Segundo banco CHR
    assert(value == expected);
    
    // Testar escrita no registro CHR Bank 1 (0xC000-0xDFFF)
    mapper->cpu_write(0xC000, 0x80); // Reset
    
    // Escrever novo valor: 0x02 (selecionar terceiro banco CHR)
    mapper->cpu_write(0xC000, 0x00); // Bit 0
    mapper->cpu_write(0xC000, 0x01); // Bit 1
    mapper->cpu_write(0xC000, 0x00); // Bit 2
    mapper->cpu_write(0xC000, 0x00); // Bit 3
    mapper->cpu_write(0xC000, 0x00); // Bit 4
    
    // Verificar leitura da PPU após a mudança (no modo 8K, isso não afeta)
    value = mapper->ppu_read(0x1000);
    expected = (0x1000 + 0x80) & 0xFF; // Ainda segundo banco CHR
    assert(value == expected);
    
    // Mudar para modo CHR 4K
    mapper->cpu_write(0x8000, 0x80); // Reset
    
    // Escrever novo valor: 0x10 (PRG Mode 0, CHR Mode 1)
    mapper->cpu_write(0x8000, 0x00); // Bit 0
    mapper->cpu_write(0x8000, 0x00); // Bit 1
    mapper->cpu_write(0x8000, 0x00); // Bit 2
    mapper->cpu_write(0x8000, 0x00); // Bit 3
    mapper->cpu_write(0x8000, 0x01); // Bit 4
    
    // Verificar leitura da PPU após a mudança para modo 4K
    value = mapper->ppu_read(0x0000);
    expected = (0x1000 + 0x80) & 0xFF; // Segundo banco CHR (0-0xFFF)
    assert(value == expected);
    
    value = mapper->ppu_read(0x1000);
    expected = (0x2000 + 0x80) & 0xFF; // Terceiro banco CHR (0x1000-0x1FFF)
    assert(value == expected);
    
    // Testar escrita no registro PRG Bank (0xE000-0xFFFF)
    mapper->cpu_write(0xE000, 0x80); // Reset
    
    // Escrever novo valor: 0x03 (selecionar quarto banco PRG)
    mapper->cpu_write(0xE000, 0x01); // Bit 0
    mapper->cpu_write(0xE000, 0x01); // Bit 1
    mapper->cpu_write(0xE000, 0x00); // Bit 2
    mapper->cpu_write(0xE000, 0x00); // Bit 3
    mapper->cpu_write(0xE000, 0x00); // Bit 4
    
    // Verificar leitura da CPU após a mudança (no modo 0, 32K)
    value = mapper->cpu_read(0x8000);
    expected = (0x6000) & 0xFF; // Banco 3 (0x6000-0x7FFF)
    assert(value == expected);
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de escrita da CPU concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura da PPU no Mapper 1
 */
void test_mapper1_ppu_read()
{
    printf("Testando leitura da PPU no Mapper 1...\n");
    
    // Criar cartucho de teste com 32K PRG-ROM e 16K CHR-ROM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_16K);
    
    Mapper1* mapper = new Mapper1();
    mapper->init(cart);
    
    // Testar leitura no modo padrão (CHR Mode 0: 8K)
    for (uint32_t i = 0; i < 0x2000; i += 0x400) {
        uint8_t value = mapper->ppu_read(i);
        uint8_t expected = (i + 0x80) & 0xFF;
        assert(value == expected);
    }
    
    // Mudar para modo CHR 4K
    mapper->cpu_write(0x8000, 0x80); // Reset
    
    // Escrever novo valor: 0x10 (CHR Mode 1)
    mapper->cpu_write(0x8000, 0x00); // Bit 0
    mapper->cpu_write(0x8000, 0x00); // Bit 1
    mapper->cpu_write(0x8000, 0x00); // Bit 2
    mapper->cpu_write(0x8000, 0x00); // Bit 3
    mapper->cpu_write(0x8000, 0x01); // Bit 4
    
    // Configurar CHR Bank 0 (0-0xFFF)
    mapper->cpu_write(0xA000, 0x80); // Reset
    mapper->cpu_write(0xA000, 0x01); // Bit 0
    mapper->cpu_write(0xA000, 0x00); // Bit 1
    mapper->cpu_write(0xA000, 0x00); // Bit 2
    mapper->cpu_write(0xA000, 0x00); // Bit 3
    mapper->cpu_write(0xA000, 0x00); // Bit 4
    
    // Configurar CHR Bank 1 (0x1000-0x1FFF)
    mapper->cpu_write(0xC000, 0x80); // Reset
    mapper->cpu_write(0xC000, 0x00); // Bit 0
    mapper->cpu_write(0xC000, 0x01); // Bit 1
    mapper->cpu_write(0xC000, 0x00); // Bit 2
    mapper->cpu_write(0xC000, 0x00); // Bit 3
    mapper->cpu_write(0xC000, 0x00); // Bit 4
    
    // Testar leitura no primeiro banco (0-0xFFF)
    for (uint32_t i = 0; i < 0x1000; i += 0x400) {
        uint8_t value = mapper->ppu_read(i);
        uint8_t expected = (i + 0x1000 + 0x80) & 0xFF; // Segundo banco 4K
        assert(value == expected);
    }
    
    // Testar leitura no segundo banco (0x1000-0x1FFF)
    for (uint32_t i = 0; i < 0x1000; i += 0x400) {
        uint8_t value = mapper->ppu_read(i + 0x1000);
        uint8_t expected = (i + 0x2000 + 0x80) & 0xFF; // Terceiro banco 4K
        assert(value == expected);
    }
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    // Testar com CHR-RAM
    cart = create_test_cartridge(PRG_ROM_SIZE_32K, 0);
    cart->chr_ram_size = CHR_ROM_SIZE_8K;
    cart->chr_ram = new uint8_t[CHR_ROM_SIZE_8K];
    
    // Inicializar CHR-RAM
    for (uint32_t i = 0; i < CHR_ROM_SIZE_8K; i++) {
        cart->chr_ram[i] = i & 0xFF;
    }
    
    mapper = new Mapper1();
    mapper->init(cart);
    
    // Testar leitura na CHR-RAM
    for (uint32_t i = 0; i < 0x2000; i += 0x400) {
        uint8_t value = mapper->ppu_read(i);
        uint8_t expected = i & 0xFF;
        assert(value == expected);
    }
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de leitura da PPU concluído com sucesso!\n");
}

/**
 * @brief Testa a escrita da PPU no Mapper 1
 */
void test_mapper1_ppu_write()
{
    printf("Testando escrita da PPU no Mapper 1...\n");
    
    // Testar com CHR-ROM (não deve permitir escrita)
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_8K);
    
    Mapper1* mapper = new Mapper1();
    mapper->init(cart);
    
    // Tentar escrever na CHR-ROM
    for (uint32_t i = 0; i < 0x2000; i += 0x400) {
        mapper->ppu_write(i, 0xFF);
        
        // Verificar que a leitura ainda retorna o valor original
        uint8_t value = mapper->ppu_read(i);
        uint8_t expected = (i + 0x80) & 0xFF;
        assert(value == expected);
    }
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    // Testar com CHR-RAM (deve permitir escrita)
    cart = create_test_cartridge(PRG_ROM_SIZE_32K, 0);
    cart->chr_ram_size = CHR_ROM_SIZE_8K;
    cart->chr_ram = new uint8_t[CHR_ROM_SIZE_8K];
    
    // Inicializar CHR-RAM
    for (uint32_t i = 0; i < CHR_ROM_SIZE_8K; i++) {
        cart->chr_ram[i] = i & 0xFF;
    }
    
    mapper = new Mapper1();
    mapper->init(cart);
    
    // Escrever na CHR-RAM
    for (uint32_t i = 0; i < 0x2000; i += 0x400) {
        mapper->ppu_write(i, 0xFF);
        
        // Verificar que a leitura retorna o novo valor
        uint8_t value = mapper->ppu_read(i);
        assert(value == 0xFF);
    }
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de escrita da PPU concluído com sucesso!\n");
}

/**
 * @brief Testa o reset do Mapper 1
 */
void test_mapper1_reset()
{
    printf("Testando reset do Mapper 1...\n");
    
    // Criar cartucho de teste
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_128K, CHR_ROM_SIZE_16K);
    
    Mapper1* mapper = new Mapper1();
    mapper->init(cart);
    
    // Configurar registros
    // Controle: CHR Mode 1 (4K)
    mapper->cpu_write(0x8000, 0x80); // Reset
    mapper->cpu_write(0x8000, 0x00); // Bit 0
    mapper->cpu_write(0x8000, 0x00); // Bit 1
    mapper->cpu_write(0x8000, 0x00); // Bit 2
    mapper->cpu_write(0x8000, 0x00); // Bit 3
    mapper->cpu_write(0x8000, 0x01); // Bit 4
    
    // CHR Bank 0: 2
    mapper->cpu_write(0xA000, 0x80); // Reset
    mapper->cpu_write(0xA000, 0x00); // Bit 0
    mapper->cpu_write(0xA000, 0x01); // Bit 1
    mapper->cpu_write(0xA000, 0x00); // Bit 2
    mapper->cpu_write(0xA000, 0x00); // Bit 3
    mapper->cpu_write(0xA000, 0x00); // Bit 4
    
    // CHR Bank 1: 3
    mapper->cpu_write(0xC000, 0x80); // Reset
    mapper->cpu_write(0xC000, 0x01); // Bit 0
    mapper->cpu_write(0xC000, 0x01); // Bit 1
    mapper->cpu_write(0xC000, 0x00); // Bit 2
    mapper->cpu_write(0xC000, 0x00); // Bit 3
    mapper->cpu_write(0xC000, 0x00); // Bit 4
    
    // PRG Bank: 4
    mapper->cpu_write(0xE000, 0x80); // Reset
    mapper->cpu_write(0xE000, 0x00); // Bit 0
    mapper->cpu_write(0xE000, 0x00); // Bit 1
    mapper->cpu_write(0xE000, 0x01); // Bit 2
    mapper->cpu_write(0xE000, 0x00); // Bit 3
    mapper->cpu_write(0xE000, 0x00); // Bit 4
    
    // Executar reset
    mapper->reset();
    
    // Verificar estado após reset (deve voltar ao padrão)
    // Verificar leitura da CPU (deve voltar ao primeiro banco)
    uint8_t value = mapper->cpu_read(0x8000);
    uint8_t expected = 0x00; // Primeiro banco
    assert(value == expected);
    
    // Verificar leitura da PPU (deve voltar ao primeiro banco)
    value = mapper->ppu_read(0x0000);
    expected = (0x0000 + 0x80) & 0xFF; // Primeiro banco
    assert(value == expected);
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de reset concluído com sucesso!\n");
}

/**
 * @brief Função principal
 */
int main()
{
    printf("Iniciando testes do Mapper 1 (MMC1)...\n");
    
    // Executar testes
    test_mapper1_init();
    test_mapper1_cpu_read();
    test_mapper1_cpu_write();
    test_mapper1_ppu_read();
    test_mapper1_ppu_write();
    test_mapper1_reset();
    
    printf("Todos os testes do Mapper 1 concluídos com sucesso!\n");
    
    return 0;
}
