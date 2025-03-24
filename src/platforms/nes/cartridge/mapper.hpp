/**
 * @file mapper.hpp
 * @brief Definição da classe base Mapper para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MAPPER_HPP
#define MAPPER_HPP

#include <cstdint>

// Forward declarations
class Cartridge;

/**
 * @class Mapper
 * @brief Classe base para implementação de mappers do NES
 *
 * Esta classe define a interface comum para todos os mappers do NES,
 * permitindo que diferentes implementações de mappers sejam utilizadas
 * de forma transparente pelo sistema.
 */
class Mapper
{
public:
    /**
     * @brief Destrutor virtual para permitir destruição adequada de classes derivadas
     */
    virtual ~Mapper() = default;

    /**
     * @brief Reseta o estado do mapper
     */
    virtual void reset() = 0;

    /**
     * @brief Lê um byte da CPU
     * @param address Endereço de memória (0x8000-0xFFFF)
     * @return Byte lido
     */
    virtual uint8_t cpuRead(uint16_t address) = 0;

    /**
     * @brief Escreve um byte pela CPU
     * @param address Endereço de memória (0x8000-0xFFFF)
     * @param data Byte a ser escrito
     */
    virtual void cpuWrite(uint16_t address, uint8_t data) = 0;

    /**
     * @brief Lê um byte da PPU (CHR)
     * @param address Endereço de memória (0x0000-0x1FFF)
     * @return Byte lido
     */
    virtual uint8_t ppuRead(uint16_t address) = 0;

    /**
     * @brief Escreve um byte pela PPU (CHR)
     * @param address Endereço de memória (0x0000-0x1FFF)
     * @param data Byte a ser escrito
     */
    virtual void ppuWrite(uint16_t address, uint8_t data) = 0;

    /**
     * @brief Notifica o mapper sobre uma nova scanline para IRQ
     */
    virtual void scanline() = 0;

    /**
     * @brief Retorna o estado do sinal de IRQ
     * @return true se IRQ está ativo, false caso contrário
     */
    virtual bool irqState() { return false; }

    /**
     * @brief Limpa o sinal de IRQ
     */
    virtual void irqClear() {}

    /**
     * @brief Salva o estado do mapper
     * @param state Objeto de estado para salvar
     * @return true se bem-sucedido, false caso contrário
     */
    virtual bool saveState(void *state) { return true; }

    /**
     * @brief Carrega o estado do mapper
     * @param state Objeto de estado para carregar
     * @return true se bem-sucedido, false caso contrário
     */
    virtual bool loadState(void *state) { return true; }
};

#endif // MAPPER_HPP
