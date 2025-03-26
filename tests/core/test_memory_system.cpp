/**
 * @file test_memory_system.cpp
 * @brief Testes unitários para o sistema de gerenciamento de memória
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <gtest/gtest.h>
#include <cstring>
#include "../../src/core/memory/memory_interface.h"

// Fixture para testes do sistema de memória
class MemorySystemTest : public ::testing::Test {
protected:
    emu_memory_t memory;
    uint8_t *test_data;
    const uint32_t TEST_SIZE = 1024;
    
    void SetUp() override {
        // Criar instância de memória
        memory = emu_memory_create();
        ASSERT_NE(memory, nullptr);
        
        // Inicializar o sistema de memória
        ASSERT_TRUE(emu_memory_init(memory));
        
        // Alocar dados de teste
        test_data = new uint8_t[TEST_SIZE];
        for (uint32_t i = 0; i < TEST_SIZE; i++) {
            test_data[i] = i & 0xFF;
        }
    }
    
    void TearDown() override {
        // Liberar recursos
        emu_memory_shutdown(memory);
        emu_memory_destroy(memory);
        delete[] test_data;
    }
};

// Callbacks de teste para operações de memória
uint8_t test_read_8(memory_region_t *region, uint32_t address) {
    return region->data[address - region->start];
}

uint16_t test_read_16(memory_region_t *region, uint32_t address) {
    uint16_t value = region->data[address - region->start];
    value |= (uint16_t)region->data[address - region->start + 1] << 8;
    return value;
}

uint32_t test_read_32(memory_region_t *region, uint32_t address) {
    uint32_t value = region->data[address - region->start];
    value |= (uint32_t)region->data[address - region->start + 1] << 8;
    value |= (uint32_t)region->data[address - region->start + 2] << 16;
    value |= (uint32_t)region->data[address - region->start + 3] << 24;
    return value;
}

void test_write_8(memory_region_t *region, uint32_t address, uint8_t value) {
    region->data[address - region->start] = value;
}

void test_write_16(memory_region_t *region, uint32_t address, uint16_t value) {
    region->data[address - region->start] = value & 0xFF;
    region->data[address - region->start + 1] = (value >> 8) & 0xFF;
}

void test_write_32(memory_region_t *region, uint32_t address, uint32_t value) {
    region->data[address - region->start] = value & 0xFF;
    region->data[address - region->start + 1] = (value >> 8) & 0xFF;
    region->data[address - region->start + 2] = (value >> 16) & 0xFF;
    region->data[address - region->start + 3] = (value >> 24) & 0xFF;
}

// Teste de inicialização
TEST_F(MemorySystemTest, Initialization) {
    // Verificar se a inicialização foi bem-sucedida
    EXPECT_TRUE(memory->initialized);
    EXPECT_EQ(memory->num_regions, 0);
}

// Teste de adição de região
TEST_F(MemorySystemTest, AddRegion) {
    memory_callbacks_t callbacks = {
        .read_8 = test_read_8,
        .read_16 = test_read_16,
        .read_32 = test_read_32,
        .write_8 = test_write_8,
        .write_16 = test_write_16,
        .write_32 = test_write_32
    };
    
    // Adicionar região de RAM
    EXPECT_TRUE(emu_memory_add_region(memory, 0x1000, TEST_SIZE, test_data, EMU_MEMORY_RAM, &callbacks));
    
    // Verificar se a região foi adicionada corretamente
    EXPECT_EQ(memory->num_regions, 1);
    EXPECT_EQ(memory->regions[0].start, 0x1000);
    EXPECT_EQ(memory->regions[0].size, TEST_SIZE);
    EXPECT_EQ(memory->regions[0].data, test_data);
    EXPECT_EQ(memory->regions[0].flags, EMU_MEMORY_RAM);
    
    // Adicionar região de ROM
    uint8_t *rom_data = new uint8_t[TEST_SIZE];
    memset(rom_data, 0xAA, TEST_SIZE);
    
    EXPECT_TRUE(emu_memory_add_region(memory, 0x2000, TEST_SIZE, rom_data, EMU_MEMORY_ROM, &callbacks));
    
    // Verificar se a região foi adicionada corretamente
    EXPECT_EQ(memory->num_regions, 2);
    EXPECT_EQ(memory->regions[1].start, 0x2000);
    EXPECT_EQ(memory->regions[1].size, TEST_SIZE);
    EXPECT_EQ(memory->regions[1].data, rom_data);
    EXPECT_EQ(memory->regions[1].flags, EMU_MEMORY_ROM);
    
    // Limpar
    delete[] rom_data;
}

// Teste de remoção de região
TEST_F(MemorySystemTest, RemoveRegion) {
    memory_callbacks_t callbacks = {
        .read_8 = test_read_8,
        .read_16 = test_read_16,
        .read_32 = test_read_32,
        .write_8 = test_write_8,
        .write_16 = test_write_16,
        .write_32 = test_write_32
    };
    
    // Adicionar região
    EXPECT_TRUE(emu_memory_add_region(memory, 0x1000, TEST_SIZE, test_data, EMU_MEMORY_RAM, &callbacks));
    EXPECT_EQ(memory->num_regions, 1);
    
    // Remover região
    EXPECT_TRUE(emu_memory_remove_region(memory, 0x1000));
    EXPECT_EQ(memory->num_regions, 0);
    
    // Tentar remover região inexistente
    EXPECT_FALSE(emu_memory_remove_region(memory, 0x2000));
}

// Teste de operações de leitura
TEST_F(MemorySystemTest, ReadOperations) {
    memory_callbacks_t callbacks = {
        .read_8 = test_read_8,
        .read_16 = test_read_16,
        .read_32 = test_read_32,
        .write_8 = test_write_8,
        .write_16 = test_write_16,
        .write_32 = test_write_32
    };
    
    // Adicionar região
    EXPECT_TRUE(emu_memory_add_region(memory, 0x1000, TEST_SIZE, test_data, EMU_MEMORY_RAM, &callbacks));
    
    // Testar leitura de 8 bits
    for (uint32_t i = 0; i < 10; i++) {
        uint32_t addr = 0x1000 + i;
        EXPECT_EQ(emu_memory_read_8(memory, addr), i & 0xFF);
    }
    
    // Testar leitura de 16 bits
    for (uint32_t i = 0; i < 10; i += 2) {
        uint32_t addr = 0x1000 + i;
        uint16_t expected = (i & 0xFF) | ((i + 1) & 0xFF) << 8;
        EXPECT_EQ(emu_memory_read_16(memory, addr), expected);
    }
    
    // Testar leitura de 32 bits
    for (uint32_t i = 0; i < 10; i += 4) {
        uint32_t addr = 0x1000 + i;
        uint32_t expected = (i & 0xFF) | 
                           ((i + 1) & 0xFF) << 8 | 
                           ((i + 2) & 0xFF) << 16 | 
                           ((i + 3) & 0xFF) << 24;
        EXPECT_EQ(emu_memory_read_32(memory, addr), expected);
    }
}

// Teste de operações de escrita
TEST_F(MemorySystemTest, WriteOperations) {
    memory_callbacks_t callbacks = {
        .read_8 = test_read_8,
        .read_16 = test_read_16,
        .read_32 = test_read_32,
        .write_8 = test_write_8,
        .write_16 = test_write_16,
        .write_32 = test_write_32
    };
    
    // Adicionar região
    EXPECT_TRUE(emu_memory_add_region(memory, 0x1000, TEST_SIZE, test_data, EMU_MEMORY_RAM, &callbacks));
    
    // Testar escrita de 8 bits
    for (uint32_t i = 0; i < 10; i++) {
        uint32_t addr = 0x1000 + i;
        emu_memory_write_8(memory, addr, 0xAA);
        EXPECT_EQ(test_data[i], 0xAA);
    }
    
    // Testar escrita de 16 bits
    for (uint32_t i = 0; i < 10; i += 2) {
        uint32_t addr = 0x1000 + i;
        emu_memory_write_16(memory, addr, 0xBBCC);
        EXPECT_EQ(test_data[i], 0xCC);
        EXPECT_EQ(test_data[i + 1], 0xBB);
    }
    
    // Testar escrita de 32 bits
    for (uint32_t i = 0; i < 10; i += 4) {
        uint32_t addr = 0x1000 + i;
        emu_memory_write_32(memory, addr, 0xDDEEFFAA);
        EXPECT_EQ(test_data[i], 0xAA);
        EXPECT_EQ(test_data[i + 1], 0xFF);
        EXPECT_EQ(test_data[i + 2], 0xEE);
        EXPECT_EQ(test_data[i + 3], 0xDD);
    }
}

// Teste de proteção de memória
TEST_F(MemorySystemTest, MemoryProtection) {
    memory_callbacks_t callbacks = {
        .read_8 = test_read_8,
        .read_16 = test_read_16,
        .read_32 = test_read_32,
        .write_8 = test_write_8,
        .write_16 = test_write_16,
        .write_32 = test_write_32
    };
    
    // Adicionar região de ROM (somente leitura)
    EXPECT_TRUE(emu_memory_add_region(memory, 0x2000, TEST_SIZE, test_data, EMU_MEMORY_ROM, &callbacks));
    
    // Testar leitura (deve funcionar)
    EXPECT_EQ(emu_memory_read_8(memory, 0x2000), 0);
    EXPECT_EQ(emu_memory_read_16(memory, 0x2000), 0x0100);
    EXPECT_EQ(emu_memory_read_32(memory, 0x2000), 0x03020100);
    
    // Testar escrita (não deve afetar a memória)
    uint8_t original = test_data[0];
    emu_memory_write_8(memory, 0x2000, 0xFF);
    EXPECT_EQ(test_data[0], original); // Não deve mudar
}

// Teste de reset
TEST_F(MemorySystemTest, Reset) {
    memory_callbacks_t callbacks = {
        .read_8 = test_read_8,
        .read_16 = test_read_16,
        .read_32 = test_read_32,
        .write_8 = test_write_8,
        .write_16 = test_write_16,
        .write_32 = test_write_32
    };
    
    // Adicionar regiões
    EXPECT_TRUE(emu_memory_add_region(memory, 0x1000, TEST_SIZE, test_data, EMU_MEMORY_RAM, &callbacks));
    EXPECT_TRUE(emu_memory_add_region(memory, 0x2000, TEST_SIZE, test_data, EMU_MEMORY_ROM, &callbacks));
    EXPECT_EQ(memory->num_regions, 2);
    
    // Reset
    EXPECT_TRUE(emu_memory_reset(memory));
    
    // Verificar se as regiões foram mantidas
    EXPECT_EQ(memory->num_regions, 2);
    EXPECT_EQ(memory->regions[0].start, 0x1000);
    EXPECT_EQ(memory->regions[1].start, 0x2000);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
