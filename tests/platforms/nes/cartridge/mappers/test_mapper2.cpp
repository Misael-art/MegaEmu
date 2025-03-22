/**
 * @file test_mapper2.cpp
 * @brief Testes unitários para o Mapper 2 (UxROM) do NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <cstdio>
#include <cstring>
#include <cassert>
#include "../../../../../src/platforms/nes/cartridge/mappers/mapper2.hpp"
#include "../../../../../src/utils/test_utils.h"

// Tamanhos de teste
#define PRG_ROM_SIZE_32K (32 * 1024)
#define PRG_ROM_SIZE_64K (64 * 1024)
#define PRG_ROM_SIZE_128K (128 * 1024)
#define PRG_ROM_SIZE_256K (256 * 1024)
#define CHR_ROM_SIZE_8K (8 * 1024)

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
 * @brief Testa a inicialização do Mapper 2
 */
void test_mapper2_init()
{
    printf("Testando inicialização do Mapper 2...\n");
    
    // Criar cartucho de teste com 32K PRG-ROM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_8K);
    
    // Criar e inicializar mapper
    Mapper2* mapper = new Mapper2();
    bool result = mapper->init(cart);
    
    // Verificar resultado
    assert(result);
    assert(mapper->get_prg_bank_count() == 2);
    assert(mapper->get_chr_bank_count() == 1);
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    // Testar com 128K PRG-ROM
    cart = create_test_cartridge(PRG_ROM_SIZE_128K, CHR_ROM_SIZE_8K);
    
    mapper = new Mapper2();
    result = mapper->init(cart);
    
    // Verificar resultado
    assert(result);
    assert(mapper->get_prg_bank_count() == 8);
    assert(mapper->get_chr_bank_count() == 1);
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de inicialização concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura da CPU no Mapper 2
 */
void test_mapper2_cpu_read()
{
    printf("Testando leitura da CPU no Mapper 2...\n");
    
    // Criar cartucho de teste com 64K PRG-ROM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_64K, CHR_ROM_SIZE_8K);
    
    Mapper2* mapper = new Mapper2();
    mapper->init(cart);
    
    // Testar leitura no modo padrão
    // Primeiro banco (0x8000-0xBFFF) - variável, inicialmente banco 0
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
    
    // Mudar para o segundo banco na primeira área
    mapper->cpu_write(0x8000, 1);
    
    // Primeiro banco (0x8000-0xBFFF) - agora banco 1
    for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
        uint32_t addr = 0x8000 + i;
        uint8_t value = mapper->cpu_read(addr);
        uint8_t expected = (i + 0x4000) & 0xFF; // Segundo banco (1)
        assert(value == expected);
    }
    
    // Segundo banco (0xC000-0xFFFF) - ainda fixo no último
    for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
        uint32_t addr = 0xC000 + i;
        uint8_t value = mapper->cpu_read(addr);
        uint8_t expected = (i + 0x4000) & 0xFF; // Último banco (1)
        assert(value == expected);
    }
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    // Testar com 128K PRG-ROM (8 bancos de 16K)
    cart = create_test_cartridge(PRG_ROM_SIZE_128K, CHR_ROM_SIZE_8K);
    
    mapper = new Mapper2();
    mapper->init(cart);
    
    // Testar mudança para diferentes bancos
    for (uint8_t bank = 0; bank < 7; bank++) {
        // Mudar para o banco especificado
        mapper->cpu_write(0x8000, bank);
        
        // Verificar leitura no banco selecionado
        for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
            uint32_t addr = 0x8000 + i;
            uint8_t value = mapper->cpu_read(addr);
            uint8_t expected = (i + bank * 0x4000) & 0xFF;
            assert(value == expected);
        }
        
        // Verificar que o último banco permanece fixo
        for (uint32_t i = 0; i < 0x4000; i += 0x1000) {
            uint32_t addr = 0xC000 + i;
            uint8_t value = mapper->cpu_read(addr);
            uint8_t expected = (i + 7 * 0x4000) & 0xFF; // Último banco (7)
            assert(value == expected);
        }
    }
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de leitura da CPU concluído com sucesso!\n");
}

/**
 * @brief Testa a escrita da CPU no Mapper 2
 */
void test_mapper2_cpu_write()
{
    printf("Testando escrita da CPU no Mapper 2...\n");
    
    // Criar cartucho de teste com 128K PRG-ROM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_128K, CHR_ROM_SIZE_8K);
    
    Mapper2* mapper = new Mapper2();
    mapper->init(cart);
    
    // Testar escrita em diferentes endereços (todos devem mudar o banco)
    for (uint32_t addr = 0x8000; addr <= 0xFFFF; addr += 0x1000) {
        // Mudar para o banco 2
        mapper->cpu_write(addr, 2);
        
        // Verificar leitura no banco selecionado
        uint8_t value = mapper->cpu_read(0x8000);
        uint8_t expected = (2 * 0x4000) & 0xFF; // Banco 2
        assert(value == expected);
        
        // Resetar para o banco 0
        mapper->cpu_write(addr, 0);
    }
    
    // Testar escrita de valores inválidos (fora do intervalo)
    // Criar cartucho de teste com 64K PRG-ROM (4 bancos de 16K)
    delete mapper;
    free_test_cartridge(cart);
    
    cart = create_test_cartridge(PRG_ROM_SIZE_64K, CHR_ROM_SIZE_8K);
    
    mapper = new Mapper2();
    mapper->init(cart);
    
    // Tentar selecionar banco 4 (fora do intervalo)
    mapper->cpu_write(0x8000, 4);
    
    // Verificar que o banco selecionado é 0 (4 % 4 = 0)
    uint8_t value = mapper->cpu_read(0x8000);
    uint8_t expected = 0x00; // Banco 0
    assert(value == expected);
    
    // Liberar recursos
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de escrita da CPU concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura da PPU no Mapper 2
 */
void test_mapper2_ppu_read()
{
    printf("Testando leitura da PPU no Mapper 2...\n");
    
    // Criar cartucho de teste
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_8K);
    
    Mapper2* mapper = new Mapper2();
    mapper->init(cart);
    
    // Testar leitura na CHR-ROM
    for (uint32_t i = 0; i < CHR_ROM_SIZE_8K; i += 0x400) {
        uint8_t value = mapper->ppu_read(i);
        uint8_t expected = (i + 0x80) & 0xFF;
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
    
    mapper = new Mapper2();
    mapper->init(cart);
    
    // Testar leitura na CHR-RAM
    for (uint32_t i = 0; i < CHR_ROM_SIZE_8K; i += 0x400) {
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
 * @brief Testa a escrita da PPU no Mapper 2
 */
void test_mapper2_ppu_write()
{
    printf("Testando escrita da PPU no Mapper 2...\n");
    
    // Testar com CHR-ROM (não deve permitir escrita)
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_8K);
    
    Mapper2* mapper = new Mapper2();
    mapper->init(cart);
    
    // Tentar escrever na CHR-ROM
    for (uint32_t i = 0; i < CHR_ROM_SIZE_8K; i += 0x400) {
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
    
    mapper = new Mapper2();
    mapper->init(cart);
    
    // Escrever na CHR-RAM
    for (uint32_t i = 0; i < CHR_ROM_SIZE_8K; i += 0x400) {
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
 * @brief Testa o reset do Mapper 2
 */
void test_mapper2_reset()
{
    printf("Testando reset do Mapper 2...\n");
    
    // Criar cartucho de teste
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_128K, CHR_ROM_SIZE_8K);
    
    Mapper2* mapper = new Mapper2();
    mapper->init(cart);
    
    // Mudar para o banco 3
    mapper->cpu_write(0x8000, 3);
    
    // Verificar que o banco foi alterado
    uint8_t value = mapper->cpu_read(0x8000);
    uint8_t expected = (3 * 0x4000) & 0xFF;
    assert(value == expected);
    
    // Executar reset
    mapper->reset();
    
    // Verificar que o banco voltou para 0
    value = mapper->cpu_read(0x8000);
    expected = 0x00; // Banco 0
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
    printf("Iniciando testes do Mapper 2 (UxROM)...\n");
    
    // Executar testes
    test_mapper2_init();
    test_mapper2_cpu_read();
    test_mapper2_cpu_write();
    test_mapper2_ppu_read();
    test_mapper2_ppu_write();
    test_mapper2_reset();
    
    printf("Todos os testes do Mapper 2 concluídos com sucesso!\n");
    
    return 0;
}
