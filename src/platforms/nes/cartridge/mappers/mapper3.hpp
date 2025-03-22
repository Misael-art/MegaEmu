/**
 * @file mapper3.hpp
 * @brief Implementação do Mapper 3 (CNROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#ifndef MAPPER3_HPP
#define MAPPER3_HPP

#include <cstdint>
#include "../cartridge.hpp"
#include "mapper.hpp"

/**
 * @class Mapper3
 * @brief Implementação do Mapper 3 (CNROM) para o NES
 * 
 * O Mapper 3 (CNROM) é um mapper simples que permite a troca de bancos de CHR-ROM.
 * Ele possui PRG-ROM fixa (16KB ou 32KB) e permite a seleção de um banco de 8KB de CHR-ROM.
 */
class Mapper3 : public Mapper {
public:
    /**
     * @brief Construtor para o Mapper 3
     * @param cartridge Ponteiro para o cartucho
     */
    Mapper3(Cartridge* cartridge);
    
    /**
     * @brief Destrutor para o Mapper 3
     */
    ~Mapper3();
    
    /**
     * @brief Reseta o estado do mapper
     */
    void reset() override;
    
    /**
     * @brief Lê um byte da CPU
     * @param address Endereço de memória (0x8000-0xFFFF)
     * @return Byte lido
     */
    uint8_t cpuRead(uint16_t address) override;
    
    /**
     * @brief Escreve um byte pela CPU
     * @param address Endereço de memória (0x8000-0xFFFF)
     * @param data Byte a ser escrito
     */
    void cpuWrite(uint16_t address, uint8_t data) override;
    
    /**
     * @brief Lê um byte da PPU (CHR)
     * @param address Endereço de memória (0x0000-0x1FFF)
     * @return Byte lido
     */
    uint8_t ppuRead(uint16_t address) override;
    
    /**
     * @brief Escreve um byte pela PPU (CHR)
     * @param address Endereço de memória (0x0000-0x1FFF)
     * @param data Byte a ser escrito
     */
    void ppuWrite(uint16_t address, uint8_t data) override;
    
private:
    Cartridge* m_cartridge; ///< Ponteiro para o cartucho
    uint8_t m_chrBankSelect; ///< Seletor de banco CHR (0-3 para 32KB CHR-ROM)
    uint32_t m_prgRomSize; ///< Tamanho da PRG-ROM
    uint32_t m_chrRomSize; ///< Tamanho da CHR-ROM
};

#endif // MAPPER3_HPP
