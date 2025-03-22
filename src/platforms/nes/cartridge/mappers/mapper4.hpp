/**
 * @file mapper4.hpp
 * @brief Implementação do Mapper 4 (MMC3) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-28
 */

#ifndef MAPPER4_HPP
#define MAPPER4_HPP

#include <cstdint>
#include "../cartridge.hpp"
#include "mapper.hpp"

// Enumeração para o modo de mirroring
enum class MMC3MirrorMode
{
    VERTICAL,
    HORIZONTAL
};

/**
 * @class Mapper4
 * @brief Implementação do Mapper 4 (MMC3) para o NES
 *
 * O MMC3 é um mapper avançado com as seguintes características:
 * - Bancos de PRG-ROM controláveis (até 512KB)
 * - Bancos de CHR-ROM controláveis (até 256KB)
 * - IRQ baseado em scanline
 * - RAM com bateria para save
 * - Controle de mirroring
 */
class Mapper4 : public Mapper
{
public:
    /**
     * @brief Construtor para o Mapper 4
     * @param cartridge Ponteiro para o cartucho
     */
    Mapper4(Cartridge *cartridge);

    /**
     * @brief Destrutor para o Mapper 4
     */
    ~Mapper4();

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
     */
    void scanline() override;

    /**
     * @brief Retorna o estado do sinal de IRQ
     * @return true se IRQ está ativo, false caso contrário
     */
    bool irqState() override;

    /**
     * @brief Limpa o sinal de IRQ
     */
    void irqClear() override;

private:
    Cartridge *m_cartridge; ///< Ponteiro para o cartucho

    // Registradores do MMC3
    uint8_t m_bankSelect;    ///< Registrador de seleção de banco (0x8000-0x8001)
    uint8_t m_bankData[8];   ///< Dados dos bancos (0x8001)
    uint8_t m_mirrorMode;    ///< Modo de mirroring (0xA000-0xA001)
    uint8_t m_prgRamProtect; ///< Proteção da PRG-RAM (0xA001)
    uint8_t m_irqLatch;      ///< Valor de recarga do IRQ (0xC000-0xC001)
    uint8_t m_irqCounter;    ///< Contador de IRQ (0xC000-0xC001)
    bool m_irqEnable;        ///< Flag de habilitação do IRQ (0xE000-0xE001)
    bool m_irqPending;       ///< Flag indicando IRQ pendente

    // Mapeamento de bancos
    uint32_t m_prgBanks[4]; ///< Endereços dos bancos de PRG-ROM
    uint32_t m_chrBanks[8]; ///< Endereços dos bancos de CHR-ROM/RAM

    // Informações do cartucho
    uint32_t m_prgRomSize; ///< Tamanho da PRG-ROM
    uint32_t m_chrRomSize; ///< Tamanho da CHR-ROM
    bool m_usesChrRam;     ///< Flag indicando se usa CHR-RAM

    /**
     * @brief Atualiza o mapeamento de bancos após mudança nos registradores
     */
    void updateBanks();
};

#endif // MAPPER4_HPP
