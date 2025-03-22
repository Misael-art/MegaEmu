/**
 * @file test_mapper0.c
 * @brief Testes unitários para o Mapper 0 (NROM) do NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../../../../../src/platforms/nes/cartridge/mappers/mapper0.hpp"
#include "../../../../../src/utils/test_utils.h"
#include "../../../../../deps/unity/unity.h"

// Tamanhos de teste
#define PRG_ROM_SIZE_16K (16 * 1024)
#define PRG_ROM_SIZE_32K (32 * 1024)
#define CHR_ROM_SIZE_8K (8 * 1024)

// Variáveis globais para os testes
static Cartridge* cart;
static Mapper0* mapper;

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
        // Liberar PRG-ROM
        if (cart->prg_rom) {
            delete[] cart->prg_rom;
            cart->prg_rom = nullptr;
        }
        
        // Liberar CHR-ROM
        if (cart->chr_rom) {
            delete[] cart->chr_rom;
            cart->chr_rom = nullptr;
        }
        
        // Liberar CHR-RAM se existir
        if (cart->chr_ram) {
            delete[] cart->chr_ram;
            cart->chr_ram = nullptr;
        }
        
        delete cart;
    }
}

/**
 * @brief Configura o ambiente para os testes
 */
void setUp(void)
{
    // Criar cartucho de teste com 32K PRG-ROM e 8K CHR-ROM
    cart = create_test_cartridge(PRG_ROM_SIZE_32K, CHR_ROM_SIZE_8K);
    
    // Criar instância do Mapper 0
    mapper = new Mapper0(cart);
    
    // Inicializar o mapper
    mapper->initialize();
}

/**
 * @brief Limpa o ambiente após os testes
 */
void tearDown(void)
{
    // Liberar mapper
    delete mapper;
    
    // Liberar cartucho
    free_test_cartridge(cart);
}

/**
 * @brief Testa a inicialização do Mapper 0
 */
void test_mapper0_init(void)
{
    printf("Testando inicialização do Mapper 0...\n");
    
    // Verificar se o mapper foi inicializado corretamente
    TEST_ASSERT_NOT_NULL(mapper);
    
    // Verificar se o cartucho foi associado corretamente
    TEST_ASSERT_EQUAL_PTR(cart, mapper->getCartridge());
    
    // Verificar se o mapeamento inicial está correto
    // Para PRG-ROM de 32K, o mapeamento deve ser:
    // 0x8000-0xBFFF -> 0x0000-0x3FFF (primeiros 16K)
    // 0xC000-0xFFFF -> 0x4000-0x7FFF (últimos 16K)
    TEST_ASSERT_EQUAL_UINT8(0x00, mapper->cpuRead(0x8000));
    TEST_ASSERT_EQUAL_UINT8(0x01, mapper->cpuRead(0x8001));
    TEST_ASSERT_EQUAL_UINT8(0xFF, mapper->cpuRead(0x80FF));
    
    // Verificar o último byte da primeira metade
    TEST_ASSERT_EQUAL_UINT8((0x3FFF & 0xFF), mapper->cpuRead(0xBFFF));
    
    // Verificar o primeiro byte da segunda metade
    TEST_ASSERT_EQUAL_UINT8((0x4000 & 0xFF), mapper->cpuRead(0xC000));
    
    // Verificar o último byte da segunda metade
    TEST_ASSERT_EQUAL_UINT8((0x7FFF & 0xFF), mapper->cpuRead(0xFFFF));
    
    printf("Teste de inicialização do Mapper 0 concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura da CPU no Mapper 0
 */
void test_mapper0_cpu_read(void)
{
    printf("Testando leitura da CPU no Mapper 0...\n");
    
    // Testar leitura fora do intervalo mapeado (deve retornar 0)
    TEST_ASSERT_EQUAL_UINT8(0, mapper->cpuRead(0x0000));
    TEST_ASSERT_EQUAL_UINT8(0, mapper->cpuRead(0x2000));
    TEST_ASSERT_EQUAL_UINT8(0, mapper->cpuRead(0x6000));
    TEST_ASSERT_EQUAL_UINT8(0, mapper->cpuRead(0x7FFF));
    
    // Testar leitura na primeira metade da PRG-ROM
    for (uint16_t addr = 0x8000; addr < 0xC000; addr += 0x100) {
        uint16_t prg_addr = addr - 0x8000;
        TEST_ASSERT_EQUAL_UINT8((prg_addr & 0xFF), mapper->cpuRead(addr));
    }
    
    // Testar leitura na segunda metade da PRG-ROM
    for (uint16_t addr = 0xC000; addr < 0x10000; addr += 0x100) {
        uint16_t prg_addr = (addr - 0xC000) + 0x4000;
        TEST_ASSERT_EQUAL_UINT8((prg_addr & 0xFF), mapper->cpuRead(addr));
    }
    
    // Criar um novo cartucho com PRG-ROM de 16K para testar o espelhamento
    Cartridge* small_cart = create_test_cartridge(PRG_ROM_SIZE_16K, CHR_ROM_SIZE_8K);
    Mapper0* small_mapper = new Mapper0(small_cart);
    small_mapper->initialize();
    
    // Para PRG-ROM de 16K, o mapeamento deve ser:
    // 0x8000-0xBFFF -> 0x0000-0x3FFF (16K)
    // 0xC000-0xFFFF -> 0x0000-0x3FFF (espelhamento)
    
    // Verificar espelhamento (os mesmos valores devem ser lidos em ambas as metades)
    for (uint16_t offset = 0; offset < 0x4000; offset += 0x100) {
        uint16_t addr1 = 0x8000 + offset;
        uint16_t addr2 = 0xC000 + offset;
        TEST_ASSERT_EQUAL_UINT8(small_mapper->cpuRead(addr1), small_mapper->cpuRead(addr2));
    }
    
    // Limpar recursos
    delete small_mapper;
    free_test_cartridge(small_cart);
    
    printf("Teste de leitura da CPU no Mapper 0 concluído com sucesso!\n");
}

/**
 * @brief Testa a escrita da CPU no Mapper 0
 */
void test_mapper0_cpu_write(void)
{
    printf("Testando escrita da CPU no Mapper 0...\n");
    
    // O Mapper 0 não suporta escrita na PRG-ROM, então as escritas devem ser ignoradas
    // Vamos verificar isso tentando escrever e depois lendo o mesmo endereço
    
    // Salvar valores originais
    uint8_t original_value_8000 = mapper->cpuRead(0x8000);
    uint8_t original_value_C000 = mapper->cpuRead(0xC000);
    
    // Tentar escrever novos valores
    mapper->cpuWrite(0x8000, 0xAA);
    mapper->cpuWrite(0xC000, 0xBB);
    
    // Verificar se os valores não foram alterados
    TEST_ASSERT_EQUAL_UINT8(original_value_8000, mapper->cpuRead(0x8000));
    TEST_ASSERT_EQUAL_UINT8(original_value_C000, mapper->cpuRead(0xC000));
    
    printf("Teste de escrita da CPU no Mapper 0 concluído com sucesso!\n");
}

/**
 * @brief Testa a leitura da PPU no Mapper 0
 */
void test_mapper0_ppu_read(void)
{
    printf("Testando leitura da PPU no Mapper 0...\n");
    
    // Testar leitura na CHR-ROM
    for (uint16_t addr = 0; addr < 0x2000; addr += 0x100) {
        TEST_ASSERT_EQUAL_UINT8(((addr + 0x80) & 0xFF), mapper->ppuRead(addr));
    }
    
    // Testar leitura fora do intervalo mapeado (deve retornar 0)
    TEST_ASSERT_EQUAL_UINT8(0, mapper->ppuRead(0x2000));
    TEST_ASSERT_EQUAL_UINT8(0, mapper->ppuRead(0x3000));
    
    // Criar um cartucho com CHR-RAM em vez de CHR-ROM
    Cartridge* ram_cart = create_test_cartridge(PRG_ROM_SIZE_32K, 0);
    ram_cart->chr_ram_size = CHR_ROM_SIZE_8K;
    ram_cart->chr_ram = new uint8_t[CHR_ROM_SIZE_8K];
    
    // Inicializar CHR-RAM com valores de teste
    for (uint32_t i = 0; i < CHR_ROM_SIZE_8K; i++) {
        ram_cart->chr_ram[i] = (i + 0x40) & 0xFF;
    }
    
    Mapper0* ram_mapper = new Mapper0(ram_cart);
    ram_mapper->initialize();
    
    // Testar leitura na CHR-RAM
    for (uint16_t addr = 0; addr < 0x2000; addr += 0x100) {
        TEST_ASSERT_EQUAL_UINT8(((addr + 0x40) & 0xFF), ram_mapper->ppuRead(addr));
    }
    
    // Limpar recursos
    delete ram_mapper;
    free_test_cartridge(ram_cart);
    
    printf("Teste de leitura da PPU no Mapper 0 concluído com sucesso!\n");
}

/**
 * @brief Testa a escrita da PPU no Mapper 0
 */
void test_mapper0_ppu_write(void)
{
    printf("Testando escrita da PPU no Mapper 0...\n");
    
    // O Mapper 0 não suporta escrita na CHR-ROM, então as escritas devem ser ignoradas
    // Vamos verificar isso tentando escrever e depois lendo o mesmo endereço
    
    // Salvar valor original
    uint8_t original_value = mapper->ppuRead(0x1000);
    
    // Tentar escrever novo valor
    mapper->ppuWrite(0x1000, 0xCC);
    
    // Verificar se o valor não foi alterado
    TEST_ASSERT_EQUAL_UINT8(original_value, mapper->ppuRead(0x1000));
    
    // Testar escrita na CHR-RAM
    Cartridge* ram_cart = create_test_cartridge(PRG_ROM_SIZE_32K, 0);
    ram_cart->chr_ram_size = CHR_ROM_SIZE_8K;
    ram_cart->chr_ram = new uint8_t[CHR_ROM_SIZE_8K];
    
    // Inicializar CHR-RAM com zeros
    memset(ram_cart->chr_ram, 0, CHR_ROM_SIZE_8K);
    
    Mapper0* ram_mapper = new Mapper0(ram_cart);
    ram_mapper->initialize();
    
    // Escrever valores na CHR-RAM
    for (uint16_t addr = 0; addr < 0x2000; addr += 0x100) {
        ram_mapper->ppuWrite(addr, (addr & 0xFF));
    }
    
    // Verificar se os valores foram escritos corretamente
    for (uint16_t addr = 0; addr < 0x2000; addr += 0x100) {
        TEST_ASSERT_EQUAL_UINT8((addr & 0xFF), ram_mapper->ppuRead(addr));
    }
    
    // Limpar recursos
    delete ram_mapper;
    free_test_cartridge(ram_cart);
    
    printf("Teste de escrita da PPU no Mapper 0 concluído com sucesso!\n");
}

/**
 * @brief Testa o reset do Mapper 0
 */
void test_mapper0_reset(void)
{
    printf("Testando reset do Mapper 0...\n");
    
    // O reset do Mapper 0 deve manter o mapeamento padrão
    // Vamos verificar isso comparando os valores antes e depois do reset
    
    // Salvar valores originais
    uint8_t original_value_8000 = mapper->cpuRead(0x8000);
    uint8_t original_value_C000 = mapper->cpuRead(0xC000);
    uint8_t original_value_1000 = mapper->ppuRead(0x1000);
    
    // Executar reset
    mapper->reset();
    
    // Verificar se os valores não foram alterados
    TEST_ASSERT_EQUAL_UINT8(original_value_8000, mapper->cpuRead(0x8000));
    TEST_ASSERT_EQUAL_UINT8(original_value_C000, mapper->cpuRead(0xC000));
    TEST_ASSERT_EQUAL_UINT8(original_value_1000, mapper->ppuRead(0x1000));
    
    printf("Teste de reset do Mapper 0 concluído com sucesso!\n");
}

/**
 * @brief Executa todos os testes
 */
int main(void)
{
    UNITY_BEGIN();
    
    printf("Iniciando testes do Mapper 0 (NROM) do NES\n");
    
    RUN_TEST(test_mapper0_init);
    RUN_TEST(test_mapper0_cpu_read);
    RUN_TEST(test_mapper0_cpu_write);
    RUN_TEST(test_mapper0_ppu_read);
    RUN_TEST(test_mapper0_ppu_write);
    RUN_TEST(test_mapper0_reset);
    
    printf("Todos os testes do Mapper 0 concluídos com sucesso!\n");
    
    return UNITY_END();
}
