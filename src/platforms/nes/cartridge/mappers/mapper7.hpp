/**
 * @file mapper7.hpp
 * @brief Definição do Mapper 7 (AxROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MEGAEMU_NES_MAPPER7_HPP
#define MEGAEMU_NES_MAPPER7_HPP

#include "../mapper.hpp"
#include "../cartridge.hpp"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            /**
             * @brief Implementação do Mapper 7 (AxROM) para o NES
             *
             * O AxROM é um mapper simples que permite a seleção de bancos de 32KB
             * de PRG-ROM e oferece suporte a espelhamento de 1 tela único.
             *
             * Características:
             * - Suporta até 512KB de PRG-ROM (16 bancos de 32KB)
             * - 8KB de CHR-RAM (não possui CHR-ROM)
             * - Espelhamento de 1 tela (ou nametable A ou nametable B)
             * - Não possui PRG-RAM
             * - Não suporta IRQs
             *
             * Jogos populares que usam este mapper incluem:
             * - Battletoads
             * - Marble Madness
             * - Wizards and Warriors
             * - Ikari Warriors
             */
            class Mapper7 : public Mapper
            {
            public:
                /**
                 * @brief Construtor do Mapper 7 (AxROM)
                 * @param cartridge Ponteiro para o cartucho
                 */
                Mapper7(Cartridge *cartridge);

                /**
                 * @brief Destrutor do Mapper 7
                 */
                ~Mapper7();

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
                 * Não utilizado pelo AxROM, mas implementado por compatibilidade
                 */
                void scanline() override;

                /**
                 * @brief Verifica o estado atual da linha de IRQ
                 * @return Estado do IRQ (sempre falso para AxROM)
                 */
                bool irqState() override;

                /**
                 * @brief Limpa o sinal de IRQ
                 * Não utilizado pelo AxROM, mas implementado por compatibilidade
                 */
                void irqClear() override;

            private:
                Cartridge *m_cartridge;    ///< Ponteiro para o cartucho
                uint32_t m_prgRomSize;     ///< Tamanho da PRG-ROM em bytes
                uint8_t m_prgBank;         ///< Banco de PRG-ROM selecionado
                uint8_t m_oneScreenMirror; ///< Tipo de espelhamento (0 = primeira tela, 1 = segunda tela)
            };
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu

#endif // MEGAEMU_NES_MAPPER7_HPP
