/**
 * @file mapper20.hpp
 * @brief Implementação do Mapper 20 (FDS) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MAPPER20_HPP
#define MAPPER20_HPP

#include <cstdint>
#include "../mapper.hpp"
#include "../cartridge.hpp"

/**
 * @class Mapper20
 * @brief Implementação do Mapper 20 (FDS) para o NES
 *
 * O Mapper 20 (FDS) é usado para jogos do Famicom Disk System.
 */
class Mapper20 : public Mapper
{
public:
    /**
     * @brief Construtor para o Mapper 20
     * @param cartridge Ponteiro para o cartucho
     */
    Mapper20(Cartridge *cartridge);

    /**
     * @brief Destrutor para o Mapper 20
     */
    ~Mapper20();

    /**
     * @brief Reseta o estado do mapper
     */
    void reset() override;

    /**
     * @brief Lê um byte da CPU
     * @param address Endereço de memória
     * @return Byte lido
     */
    uint8_t cpuRead(uint16_t address) override;

    /**
     * @brief Escreve um byte pela CPU
     * @param address Endereço de memória
     * @param data Byte a ser escrito
     */
    void cpuWrite(uint16_t address, uint8_t data) override;

    /**
     * @brief Lê um byte da PPU
     * @param address Endereço de memória
     * @return Byte lido
     */
    uint8_t ppuRead(uint16_t address) override;

    /**
     * @brief Escreve um byte pela PPU
     * @param address Endereço de memória
     * @param data Byte a ser escrito
     */
    void ppuWrite(uint16_t address, uint8_t data) override;

    /**
     * @brief Notifica o mapper sobre uma nova scanline para IRQ
     */
    void scanline() override;

    /**
     * @brief Retorna o estado do sinal de IRQ
     * @return Estado do IRQ
     */
    bool irqState() override;

    /**
     * @brief Limpa o sinal de IRQ
     */
    void irqClear() override;

private:
    Cartridge *m_cartridge; ///< Ponteiro para o cartucho
};

#endif // MAPPER20_HPP
