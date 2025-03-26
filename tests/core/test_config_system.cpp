/**
 * @file test_config_system.cpp
 * @brief Testes unitários para o sistema de configuração
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <gtest/gtest.h>
#include <string>
#include <filesystem>
#include "../../src/core/config_system.hpp"

// Fixture para testes do sistema de configuração
class ConfigSystemTest : public ::testing::Test {
protected:
    std::string testConfigFile;
    
    void SetUp() override {
        // Usar um arquivo de configuração temporário para testes
        testConfigFile = "test_config.ini";
        
        // Remover o arquivo de teste se ele já existir
        if (std::filesystem::exists(testConfigFile)) {
            std::filesystem::remove(testConfigFile);
        }
    }
    
    void TearDown() override {
        // Limpar após os testes
        if (std::filesystem::exists(testConfigFile)) {
            std::filesystem::remove(testConfigFile);
        }
    }
};

// Teste de inicialização
TEST_F(ConfigSystemTest, Initialization) {
    MegaEmu::Core::ConfigSystem config(testConfigFile);
    
    // Verificar se os valores padrão foram definidos corretamente
    EXPECT_EQ(config.getValue<bool>("Video.Fullscreen", false), false);
    EXPECT_EQ(config.getValue<int32_t>("Video.Width", 640), 640);
    EXPECT_EQ(config.getValue<int32_t>("Video.Height", 480), 480);
    EXPECT_EQ(config.getValue<std::string>("Audio.Device", "default"), "default");
}

// Teste de definição e obtenção de valores
TEST_F(ConfigSystemTest, SetAndGetValues) {
    MegaEmu::Core::ConfigSystem config(testConfigFile);
    
    // Definir valores
    config.setValue("Video.Fullscreen", true);
    config.setValue("Video.Width", 800);
    config.setValue("Video.Height", 600);
    config.setValue("Audio.Device", "custom");
    config.setValue("Audio.Volume", 0.8);
    
    // Verificar se os valores foram definidos corretamente
    EXPECT_EQ(config.getValue<bool>("Video.Fullscreen", false), true);
    EXPECT_EQ(config.getValue<int32_t>("Video.Width", 640), 800);
    EXPECT_EQ(config.getValue<int32_t>("Video.Height", 480), 600);
    EXPECT_EQ(config.getValue<std::string>("Audio.Device", "default"), "custom");
    EXPECT_FLOAT_EQ(config.getValue<double>("Audio.Volume", 0.5), 0.8);
}

// Teste de persistência
TEST_F(ConfigSystemTest, Persistence) {
    {
        // Escopo para garantir que o destrutor seja chamado
        MegaEmu::Core::ConfigSystem config(testConfigFile);
        
        // Definir valores
        config.setValue("Video.Fullscreen", true);
        config.setValue("Video.Width", 800);
        config.setValue("Video.Height", 600);
        config.setValue("Audio.Device", "custom");
        config.setValue("Audio.Volume", 0.8);
        
        // O destrutor deve salvar as configurações
    }
    
    // Criar uma nova instância para carregar as configurações salvas
    MegaEmu::Core::ConfigSystem config2(testConfigFile);
    
    // Verificar se os valores foram carregados corretamente
    EXPECT_EQ(config2.getValue<bool>("Video.Fullscreen", false), true);
    EXPECT_EQ(config2.getValue<int32_t>("Video.Width", 640), 800);
    EXPECT_EQ(config2.getValue<int32_t>("Video.Height", 480), 600);
    EXPECT_EQ(config2.getValue<std::string>("Audio.Device", "default"), "custom");
    EXPECT_FLOAT_EQ(config2.getValue<double>("Audio.Volume", 0.5), 0.8);
}

// Teste de valores padrão
TEST_F(ConfigSystemTest, DefaultValues) {
    MegaEmu::Core::ConfigSystem config(testConfigFile);
    
    // Verificar se os valores padrão são retornados para chaves inexistentes
    EXPECT_EQ(config.getValue<bool>("NonExistent.Boolean", true), true);
    EXPECT_EQ(config.getValue<int32_t>("NonExistent.Integer", 42), 42);
    EXPECT_EQ(config.getValue<std::string>("NonExistent.String", "default"), "default");
    EXPECT_FLOAT_EQ(config.getValue<double>("NonExistent.Double", 3.14), 3.14);
}

// Teste de seções
TEST_F(ConfigSystemTest, Sections) {
    MegaEmu::Core::ConfigSystem config(testConfigFile);
    
    // Definir valores em diferentes seções
    config.setValue("Section1.Key1", "Value1");
    config.setValue("Section1.Key2", 123);
    config.setValue("Section2.Key1", true);
    config.setValue("Section2.Key2", 3.14);
    
    // Verificar se os valores foram definidos corretamente
    EXPECT_EQ(config.getValue<std::string>("Section1.Key1", ""), "Value1");
    EXPECT_EQ(config.getValue<int32_t>("Section1.Key2", 0), 123);
    EXPECT_EQ(config.getValue<bool>("Section2.Key1", false), true);
    EXPECT_FLOAT_EQ(config.getValue<double>("Section2.Key2", 0.0), 3.14);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
