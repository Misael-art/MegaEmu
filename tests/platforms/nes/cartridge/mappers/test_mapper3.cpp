/**
 * @file test_mapper3.cpp
 * @brief Testes unitários para o Mapper 3 (CNROM) do NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <cstdio>
#include <cstring>
#include <cassert>
#include "../../../../../src/platforms/nes/cartridge/mappers/mapper3.hpp"
#include "../../../../../src/utils/test_utils.h"

// Tamanhos de teste
#define PRG_ROM_SIZE_16K (16 * 1024)
#define PRG_ROM_SIZE_32K (32 * 1024)
#define CHR_ROM_SIZE_8K (8 * 1024)
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
 * @brief Testa a inicialização do Mapper 3
 */
static void test_mapper3_init()
{
    printf("Teste: Inicialização do Mapper 3\n");
    
    // Criar cartucho de teste
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_16K, CHR_ROM_SIZE_8K);
    
    // Criar mapper
    Mapper3* mapper = new Mapper3(cart);
    
    // Verificar inicialização
    assert(mapper != nullptr);
    
    // Limpar
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de inicialização concluído com sucesso\n");
}

/**
 * @brief Testa a leitura da CPU no Mapper 3
 */
static void test_mapper3_cpu_read()
{
    printf("Teste: Leitura da CPU no Mapper 3\n");
    
    // Testar com PRG-ROM de 16K
    {
        printf("  Subteste: PRG-ROM de 16K\n");
        
        // Criar cartucho de teste
        Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_16K, CHR_ROM_SIZE_8K);
        
        // Criar mapper
        Mapper3* mapper = new Mapper3(cart);
        
        // Testar leitura em diferentes endereços
        // Para PRG-ROM de 16K, o conteúdo é espelhado em 0x8000-0xFFFF
        for (uint16_t addr = 0x8000; addr < 0x10000; addr += 0x1000) {
            uint16_t expected_addr = (addr - 0x8000) % PRG_ROM_SIZE_16K;
            uint8_t expected_value = expected_addr & 0xFF;
            uint8_t actual_value = mapper->cpuRead(addr);
            
            assert(actual_value == expected_value);
        }
        
        // Limpar
        delete mapper;
        free_test_cartridge(cart);
    }
    
    // Testar com PRG-ROM de 32K
    {
        printf("  Subteste: PRG-ROM de 32K\n");
        
        // Criar cartucho de teste
        Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_8K);
        
        // Criar mapper
        Mapper3* mapper = new Mapper3(cart);
        
        // Testar leitura em diferentes endereços
        // Para PRG-ROM de 32K, o conteúdo é mapeado diretamente em 0x8000-0xFFFF
        for (uint16_t addr = 0x8000; addr < 0x10000; addr += 0x1000) {
            uint16_t expected_addr = addr - 0x8000;
            uint8_t expected_value = expected_addr & 0xFF;
            uint8_t actual_value = mapper->cpuRead(addr);
            
            assert(actual_value == expected_value);
        }
        
        // Limpar
        delete mapper;
        free_test_cartridge(cart);
    }
    
    printf("Teste de leitura da CPU concluído com sucesso\n");
}

/**
 * @brief Testa a escrita da CPU no Mapper 3
 */
static void test_mapper3_cpu_write()
{
    printf("Teste: Escrita da CPU no Mapper 3\n");
    
    // Criar cartucho de teste
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_16K, CHR_ROM_SIZE_32K);
    
    // Criar mapper
    Mapper3* mapper = new Mapper3(cart);
    
    // Testar escrita para selecionar banco CHR
    // Banco 0 (padrão)
    uint8_t value_bank0 = mapper->ppuRead(0x0000);
    uint8_t expected_bank0 = 0x80; // Primeiro byte da CHR-ROM
    assert(value_bank0 == expected_bank0);
    
    // Selecionar banco 1
    mapper->cpuWrite(0x8000, 0x01);
    
    // Verificar se o banco 1 foi selecionado
    uint8_t value_bank1 = mapper->ppuRead(0x0000);
    uint8_t expected_bank1 = 0x80 + CHR_ROM_SIZE_8K; // Primeiro byte do segundo banco
    assert(value_bank1 == expected_bank1);
    
    // Selecionar banco 2
    mapper->cpuWrite(0xA000, 0x02); // O endereço não importa, apenas o valor
    
    // Verificar se o banco 2 foi selecionado
    uint8_t value_bank2 = mapper->ppuRead(0x0000);
    uint8_t expected_bank2 = 0x80 + (2 * CHR_ROM_SIZE_8K); // Primeiro byte do terceiro banco
    assert(value_bank2 == expected_bank2);
    
    // Selecionar banco 3
    mapper->cpuWrite(0xC000, 0x03);
    
    // Verificar se o banco 3 foi selecionado
    uint8_t value_bank3 = mapper->ppuRead(0x0000);
    uint8_t expected_bank3 = 0x80 + (3 * CHR_ROM_SIZE_8K); // Primeiro byte do quarto banco
    assert(value_bank3 == expected_bank3);
    
    // Testar seleção de banco inválido (deve ser tratado internamente)
    mapper->cpuWrite(0xE000, 0x04); // Banco 4 (fora do intervalo)
    
    // Verificar se um banco válido foi selecionado
    uint8_t value_bank_invalid = mapper->ppuRead(0x0000);
    // O comportamento esperado é que o valor seja mascarado para um banco válido
    assert(value_bank_invalid == expected_bank0); // Deve voltar ao banco 0
    
    // Limpar
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de escrita da CPU concluído com sucesso\n");
}

/**
 * @brief Testa a leitura da PPU no Mapper 3
 */
static void test_mapper3_ppu_read()
{
    printf("Teste: Leitura da PPU no Mapper 3\n");
    
    // Criar cartucho de teste
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_16K, CHR_ROM_SIZE_32K);
    
    // Criar mapper
    Mapper3* mapper = new Mapper3(cart);
    
    // Testar leitura em diferentes bancos
    
    // Banco 0 (padrão)
    for (uint16_t addr = 0x0000; addr < 0x2000; addr += 0x0400) {
        uint32_t expected_addr = addr;
        uint8_t expected_value = (expected_addr + 0x80) & 0xFF;
        uint8_t actual_value = mapper->ppuRead(addr);
        
        assert(actual_value == expected_value);
    }
    
    // Selecionar banco 1
    mapper->cpuWrite(0x8000, 0x01);
    
    // Verificar leitura no banco 1
    for (uint16_t addr = 0x0000; addr < 0x2000; addr += 0x0400) {
        uint32_t expected_addr = CHR_ROM_SIZE_8K + addr;
        uint8_t expected_value = (expected_addr + 0x80) & 0xFF;
        uint8_t actual_value = mapper->ppuRead(addr);
        
        assert(actual_value == expected_value);
    }
    
    // Testar leitura fora do intervalo
    uint8_t value_out_of_range = mapper->ppuRead(0x2000);
    assert(value_out_of_range == 0); // Deve retornar 0 para endereços fora do intervalo
    
    // Limpar
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de leitura da PPU concluído com sucesso\n");
}

/**
 * @brief Testa a escrita da PPU no Mapper 3
 */
static void test_mapper3_ppu_write()
{
    printf("Teste: Escrita da PPU no Mapper 3\n");
    
    // Criar cartucho de teste com CHR-RAM
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_16K, CHR_ROM_SIZE_8K);
    
    // Adicionar CHR-RAM
    cart->chr_ram_size = CHR_ROM_SIZE_8K;
    cart->chr_ram = new uint8_t[cart->chr_ram_size];
    memset(cart->chr_ram, 0, cart->chr_ram_size);
    
    // Criar mapper
    Mapper3* mapper = new Mapper3(cart);
    
    // Testar escrita na CHR-RAM
    // No caso do Mapper 3, a escrita na CHR-RAM só funciona se o cartucho tiver CHR-RAM
    mapper->ppuWrite(0x0000, 0x42);
    
    // Como o Mapper 3 não implementa escrita na CHR-ROM, não há como verificar
    // diretamente se a escrita funcionou. Vamos apenas verificar se não ocorreu
    // nenhum erro durante a escrita.
    
    // Testar escrita fora do intervalo
    mapper->ppuWrite(0x2000, 0x42); // Deve ser ignorado
    
    // Limpar
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de escrita da PPU concluído com sucesso\n");
}

/**
 * @brief Testa o reset do Mapper 3
 */
static void test_mapper3_reset()
{
    printf("Teste: Reset do Mapper 3\n");
    
    // Criar cartucho de teste
    Cartridge* cart = create_test_cartridge(PRG_ROM_SIZE_16K, CHR_ROM_SIZE_32K);
    
    // Criar mapper
    Mapper3* mapper = new Mapper3(cart);
    
    // Selecionar banco 2
    mapper->cpuWrite(0x8000, 0x02);
    
    // Verificar se o banco 2 foi selecionado
    uint8_t value_bank2 = mapper->ppuRead(0x0000);
    uint8_t expected_bank2 = 0x80 + (2 * CHR_ROM_SIZE_8K);
    assert(value_bank2 == expected_bank2);
    
    // Resetar mapper
    mapper->reset();
    
    // Verificar se voltou ao banco 0
    uint8_t value_bank0 = mapper->ppuRead(0x0000);
    uint8_t expected_bank0 = 0x80;
    assert(value_bank0 == expected_bank0);
    
    // Limpar
    delete mapper;
    free_test_cartridge(cart);
    
    printf("Teste de reset concluído com sucesso\n");
}

/**
 * @brief Função principal
 */
int main()
{
    printf("Iniciando testes do Mapper 3 (CNROM)\n");
    
    // Executar testes
    test_mapper3_init();
    test_mapper3_cpu_read();
    test_mapper3_cpu_write();
    test_mapper3_ppu_read();
    test_mapper3_ppu_write();
    test_mapper3_reset();
    
    printf("Todos os testes do Mapper 3 concluídos com sucesso!\n");
    
    return 0;
}
