/**
 * @file mapper0.hpp
 * @brief Implementação do Mapper 0 (NROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MAPPER0_HPP
#define MAPPER0_HPP

#include <cstdint>
#include "../cartridge.hpp"
#include "mapper.hpp"

/**
 * @class Mapper0
 * @brief Implementação do Mapper 0 (NROM) para o NES
 *
 * O Mapper 0 (NROM) é o mapper mais básico do NES, sem nenhum tipo de bank switching.
 * Suporta até 32KB de PRG-ROM e 8KB de CHR-ROM/RAM.
 * Existem duas variantes:
 * - NROM-128: 16KB PRG-ROM, espelhado em $8000-$BFFF e $C000-$FFFF
 * - NROM-256: 32KB PRG-ROM, mapeado diretamente em $8000-$FFFF
 */
class Mapper0 : public Mapper
{
public:
    /**
     * @brief Construtor para o Mapper 0
     * @param cartridge Ponteiro para o cartucho
     */
    Mapper0(Cartridge *cartridge);

    /**
     * @brief Destrutor para o Mapper 0
     */
    ~Mapper0();

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

    /**
     * @brief Notifica o mapper sobre uma nova scanline para IRQ
     * No caso do NROM, este método não faz nada, pois o mapper não suporta IRQs
     */
    void scanline() override;

    /**
     * @brief Retorna o estado do sinal de IRQ
     * @return false, pois NROM não suporta IRQs
     */
    bool irqState() override;

    /**
     * @brief Limpa o sinal de IRQ
     * No caso do NROM, este método não faz nada
     */
    void irqClear() override;

private:
    Cartridge *m_cartridge; ///< Ponteiro para o cartucho
    uint32_t m_prgRomSize;  ///< Tamanho da PRG-ROM
    uint32_t m_chrRomSize;  ///< Tamanho da CHR-ROM
    bool m_usesChrRam;      ///< Flag indicando se usa CHR-RAM
    bool m_isNROM128;       ///< Flag indicando se é NROM-128 (16KB espelhado)
};

#endif // MAPPER0_HPP
