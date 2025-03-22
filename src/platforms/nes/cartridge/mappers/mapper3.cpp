/**
 * @file mapper3.cpp
 * @brief Implementação do Mapper 3 (CNROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <cstdio>
#include "mapper3.hpp"
#include "../../../../utils/log_utils.h"

/**
 * @brief Construtor para o Mapper 3
 * @param cartridge Ponteiro para o cartucho
 */
Mapper3::Mapper3(Cartridge* cartridge) : m_cartridge(cartridge)
{
    // Verificar se o cartucho é válido
    if (!cartridge) {
        LOG_ERROR("Mapper3: Cartucho inválido");
        return;
    }
    
    // Obter tamanhos das ROMs
    m_prgRomSize = cartridge->prg_rom_size;
    m_chrRomSize = cartridge->chr_rom_size;
    
    // Verificar tamanhos
    if (m_prgRomSize == 0 || m_chrRomSize == 0) {
        LOG_ERROR("Mapper3: Tamanhos de ROM inválidos: PRG=%u, CHR=%u", 
                 m_prgRomSize, m_chrRomSize);
        return;
    }
    
    // Inicializar seletor de banco
    m_chrBankSelect = 0;
    
    LOG_INFO("Mapper3 (CNROM) inicializado: PRG-ROM=%uKB, CHR-ROM=%uKB", 
             m_prgRomSize / 1024, m_chrRomSize / 1024);
}

/**
 * @brief Destrutor para o Mapper 3
 */
Mapper3::~Mapper3()
{
    // Nada a fazer aqui
}

/**
 * @brief Reseta o estado do mapper
 */
void Mapper3::reset()
{
    // Resetar seletor de banco
    m_chrBankSelect = 0;
    
    LOG_INFO("Mapper3 (CNROM) resetado");
}

/**
 * @brief Lê um byte da CPU
 * @param address Endereço de memória (0x8000-0xFFFF)
 * @return Byte lido
 */
uint8_t Mapper3::cpuRead(uint16_t address)
{
    // Verificar se o endereço está no intervalo correto
    if (address < 0x8000) {
        LOG_WARNING("Mapper3: Tentativa de leitura fora do intervalo: 0x%04X", address);
        return 0;
    }
    
    // CNROM tem apenas 16KB ou 32KB fixos de PRG-ROM
    if (m_prgRomSize == 16 * 1024) {
        // Para ROMs de 16KB, espelha em 0x8000-0xFFFF
        uint32_t addr = (address - 0x8000) % m_prgRomSize;
        return m_cartridge->prg_rom[addr];
    } else {
        // Para ROMs de 32KB, mapeia diretamente
        uint32_t addr = (address - 0x8000);
        return m_cartridge->prg_rom[addr];
    }
}

/**
 * @brief Escreve um byte pela CPU
 * @param address Endereço de memória (0x8000-0xFFFF)
 * @param data Byte a ser escrito
 */
void Mapper3::cpuWrite(uint16_t address, uint8_t data)
{
    // Verificar se o endereço está no intervalo correto
    if (address < 0x8000) {
        LOG_WARNING("Mapper3: Tentativa de escrita fora do intervalo: 0x%04X", address);
        return;
    }
    
    // No CNROM, qualquer escrita para 0x8000-0xFFFF seleciona o banco de CHR
    m_chrBankSelect = data & 0x03; // Apenas os 2 bits inferiores são usados (4 bancos máximo)
    
    // Verificar se o banco selecionado é válido
    if (m_chrBankSelect * 8 * 1024 >= m_chrRomSize) {
        LOG_WARNING("Mapper3: Seleção de banco CHR inválida: %u (máximo: %u)", 
                   m_chrBankSelect, (m_chrRomSize / (8 * 1024)) - 1);
        
        // Ajustar para um banco válido
        m_chrBankSelect %= (m_chrRomSize / (8 * 1024));
    }
    
    LOG_DEBUG("Mapper3: Banco CHR selecionado: %u", m_chrBankSelect);
}

/**
 * @brief Lê um byte da PPU (CHR)
 * @param address Endereço de memória (0x0000-0x1FFF)
 * @return Byte lido
 */
uint8_t Mapper3::ppuRead(uint16_t address)
{
    // Verificar se o endereço está no intervalo correto
    if (address >= 0x2000) {
        LOG_WARNING("Mapper3: Tentativa de leitura CHR fora do intervalo: 0x%04X", address);
        return 0;
    }
    
    // Verificar se temos CHR-ROM
    if (m_cartridge->chr_rom == nullptr) {
        LOG_WARNING("Mapper3: Tentativa de leitura de CHR-ROM inexistente");
        return 0;
    }
    
    // Calcular endereço na CHR-ROM
    uint32_t addr = (m_chrBankSelect * 8 * 1024) + address;
    
    // Verificar se o endereço está dentro dos limites
    if (addr >= m_chrRomSize) {
        LOG_WARNING("Mapper3: Endereço CHR fora dos limites: 0x%06X (máximo: 0x%06X)", 
                   addr, m_chrRomSize - 1);
        
        // Ajustar para um endereço válido
        addr %= m_chrRomSize;
    }
    
    return m_cartridge->chr_rom[addr];
}

/**
 * @brief Escreve um byte pela PPU (CHR)
 * @param address Endereço de memória (0x0000-0x1FFF)
 * @param data Byte a ser escrito
 */
void Mapper3::ppuWrite(uint16_t address, uint8_t data)
{
    // Verificar se o endereço está no intervalo correto
    if (address >= 0x2000) {
        LOG_WARNING("Mapper3: Tentativa de escrita CHR fora do intervalo: 0x%04X", address);
        return;
    }
    
    // CNROM geralmente usa CHR-ROM, que não pode ser escrita
    // No entanto, alguns jogos podem usar CHR-RAM
    if (m_cartridge->chr_ram) {
        // Calcular endereço na CHR-RAM
        uint32_t addr = (m_chrBankSelect * 8 * 1024) + address;
        
        // Verificar se o endereço está dentro dos limites
        if (addr >= m_cartridge->chr_ram_size) {
            LOG_WARNING("Mapper3: Endereço CHR-RAM fora dos limites: 0x%06X (máximo: 0x%06X)", 
                       addr, m_cartridge->chr_ram_size - 1);
            return;
        }
        
        m_cartridge->chr_ram[addr] = data;
    } else {
        // Tentativa de escrita em CHR-ROM, que é apenas leitura
        LOG_WARNING("Mapper3: Tentativa de escrita em CHR-ROM: 0x%04X = 0x%02X", address, data);
    }
}
