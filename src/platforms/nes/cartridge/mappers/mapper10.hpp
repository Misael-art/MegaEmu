/**
 * @file mapper10.hpp
 * @brief Definição do Mapper 10 (MMC4) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MEGAEMU_NES_MAPPER10_HPP
#define MEGAEMU_NES_MAPPER10_HPP

#include "../mapper.hpp"
#include "../cartridge.hpp"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            /**
             * @brief Implementação do Mapper 10 (MMC4) para o NES
             *
             * O MMC4 é um mapper especializado similar ao MMC2, utilizado em
             * versões posteriores do jogo Punch-Out!!. Ele oferece suporte a
             * bank switching para PRG-ROM e CHR-ROM, com seleção dinâmica de
             * bancos de CHR-ROM baseada em padrões de tiles específicos.
             *
             * Características:
             * - Suporta até 256KB de PRG-ROM (16 bancos de 16KB)
             * - Suporta até 128KB de CHR-ROM (32 bancos de 4KB)
             * - Não suporta CHR-RAM
             * - Espelhamento configurável entre horizontal e vertical
             * - Seleção de bancos CHR-ROM baseada em latches de padrões
             * - Não suporta IRQs
             *
             * Jogos populares que usam este mapper incluem:
             * - Punch-Out!! (revisions)
             */
            class Mapper10 : public Mapper
            {
            public:
                /**
                 * @brief Construtor do Mapper 10 (MMC4)
                 * @param cartridge Ponteiro para o cartucho
                 */
                Mapper10(Cartridge *cartridge);

                /**
                 * @brief Destrutor do Mapper 10
                 */
                ~Mapper10();

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
                 * Não utilizado pelo MMC4, mas implementado por compatibilidade
                 */
                void scanline() override;

                /**
                 * @brief Verifica o estado atual da linha de IRQ
                 * @return Estado do IRQ (sempre falso para MMC4)
                 */
                bool irqState() override;

                /**
                 * @brief Limpa o sinal de IRQ
                 * Não utilizado pelo MMC4, mas implementado por compatibilidade
                 */
                void irqClear() override;

            private:
                Cartridge *m_cartridge; ///< Ponteiro para o cartucho
                uint32_t m_prgRomSize;  ///< Tamanho da PRG-ROM em bytes
                uint32_t m_chrRomSize;  ///< Tamanho da CHR-ROM em bytes

                uint8_t m_prgBank;       ///< Banco de PRG-ROM selecionado para $8000-$BFFF
                uint8_t m_chrBank0FD;    ///< Banco CHR selecionado para $0000 quando latch 0 = 0xFD
                uint8_t m_chrBank0FE;    ///< Banco CHR selecionado para $0000 quando latch 0 = 0xFE
                uint8_t m_chrBank1FD;    ///< Banco CHR selecionado para $1000 quando latch 1 = 0xFD
                uint8_t m_chrBank1FE;    ///< Banco CHR selecionado para $1000 quando latch 1 = 0xFE
                uint8_t m_latch0;        ///< Latch para o padrão de CHR em $0000-$0FFF (0xFD ou 0xFE)
                uint8_t m_latch1;        ///< Latch para o padrão de CHR em $1000-$1FFF (0xFD ou 0xFE)
                MirrorMode m_mirrorMode; ///< Modo de espelhamento

                // Constantes
                static constexpr uint32_t PRG_BANK_SIZE = 16 * 1024; ///< Tamanho do banco PRG (16KB)
                static constexpr uint32_t CHR_BANK_SIZE = 4 * 1024;  ///< Tamanho do banco CHR (4KB)
            };
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu

#endif // MEGAEMU_NES_MAPPER10_HPP
