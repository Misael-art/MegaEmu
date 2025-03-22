/**
 * @file mapper2.hpp
 * @brief Definição do Mapper 2 (UNROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MEGAEMU_NES_MAPPER2_HPP
#define MEGAEMU_NES_MAPPER2_HPP

#include "../mapper.hpp"
#include "../cartridge.hpp"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            /**
             * @brief Implementação do Mapper 2 (UNROM) para o NES
             *
             * O UNROM é um mapper simples com bank switching que permite alternar
             * bancos de 16KB de PRG-ROM para o intervalo de $8000-$BFFF, enquanto
             * mantém fixo o último banco de 16KB em $C000-$FFFF.
             *
             * Características:
             * - Suporta até 512KB de PRG-ROM (32 bancos de 16KB)
             * - 8KB de CHR-RAM (normalmente não possui CHR-ROM)
             * - Espelhamento fixo definido pelo cabeçalho do cartucho
             * - Não possui PRG-RAM
             * - Não suporta IRQs
             *
             * Jogos populares que usam este mapper incluem:
             * - Mega Man
             * - Castlevania
             * - Contra
             * - Metal Gear
             * - Duck Tales
             */
            class Mapper2 : public Mapper
            {
            public:
                /**
                 * @brief Construtor do Mapper 2 (UNROM)
                 * @param cartridge Ponteiro para o cartucho
                 */
                Mapper2(Cartridge *cartridge);

                /**
                 * @brief Destrutor do Mapper 2
                 */
                ~Mapper2();

                /**
                 * @brief Reset do mapper para estado inicial
                 */
                void reset() override;

                /**
                 * @brief Leitura de memória pelo CPU
                 * @param address Endereço de memória (0x8000-0xFFFF)
                 * @return Byte lido
                 */
                uint8_t cpuRead(uint16_t address) override;

                /**
                 * @brief Escrita de memória pelo CPU
                 * @param address Endereço de memória (0x8000-0xFFFF)
                 * @param data Byte a ser escrito
                 */
                void cpuWrite(uint16_t address, uint8_t data) override;

                /**
                 * @brief Leitura de memória pelo PPU
                 * @param address Endereço de memória (0x0000-0x1FFF)
                 * @return Byte lido
                 */
                uint8_t ppuRead(uint16_t address) override;

                /**
                 * @brief Escrita de memória pelo PPU
                 * @param address Endereço de memória (0x0000-0x1FFF)
                 * @param data Byte a ser escrito
                 */
                void ppuWrite(uint16_t address, uint8_t data) override;

                /**
                 * @brief Notifica o mapper sobre uma nova scanline
                 * Não utilizado pelo UNROM, mas implementado por compatibilidade
                 */
                void scanline() override;

                /**
                 * @brief Verifica o estado atual da linha de IRQ
                 * @return Estado do IRQ (sempre falso para UNROM)
                 */
                bool irqState() override;

                /**
                 * @brief Limpa o sinal de IRQ
                 * Não utilizado pelo UNROM, mas implementado por compatibilidade
                 */
                void irqClear() override;

            private:
                Cartridge *m_cartridge; ///< Ponteiro para o cartucho
                uint32_t m_prgRomSize;  ///< Tamanho da PRG-ROM em bytes
                bool m_usesChrRam;      ///< Indica se o mapper usa CHR-RAM
                uint8_t m_prgBank;      ///< Banco de PRG-ROM selecionado para $8000-$BFFF
                uint8_t m_lastPrgBank;  ///< Número do último banco de PRG-ROM (fixo em $C000-$FFFF)
            };
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu

#endif // MEGAEMU_NES_MAPPER2_HPP
