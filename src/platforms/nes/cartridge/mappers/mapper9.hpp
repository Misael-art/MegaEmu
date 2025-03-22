/**
 * @file mapper9.hpp
 * @brief Implementação do Mapper 9 (MMC2) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MAPPER9_HPP
#define MAPPER9_HPP

#include <cstdint>
#include "../cartridge.hpp"
#include "mapper.hpp"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            /**
             * @class Mapper9
             * @brief Implementação do Mapper 9 (MMC2) para o NES
             *
             * O Mapper 9 (MMC2) é um mapper especializado usado principalmente pelo jogo Punch-Out!!
             * Ele possui recursos para chaveamento de bancos PRG-ROM e CHR-ROM, com seleção especial
             * de padrões CHR com base na leitura de tiles específicos, permitindo gráficos mais complexos
             * sem sobrecarregar a PPU com constantes trocas de banco.
             */
            class Mapper9 : public Mapper
            {
            public:
                /**
                 * @brief Construtor para o Mapper 9
                 * @param cartridge Ponteiro para o cartucho
                 */
                Mapper9(Cartridge *cartridge);

                /**
                 * @brief Destrutor para o Mapper 9
                 */
                ~Mapper9();

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
                 * No caso do MMC2, este método não faz nada, pois o mapper não suporta IRQs
                 */
                void scanline() override;

                /**
                 * @brief Retorna o estado do sinal de IRQ
                 * @return false, pois MMC2 não suporta IRQs
                 */
                bool irqState() override;

                /**
                 * @brief Limpa o sinal de IRQ
                 * No caso do MMC2, este método não faz nada
                 */
                void irqClear() override;

            private:
                Cartridge *m_cartridge; ///< Ponteiro para o cartucho
                uint32_t m_prgRomSize;  ///< Tamanho da PRG-ROM
                uint32_t m_chrRomSize;  ///< Tamanho da CHR-ROM
                bool m_usesChrRam;      ///< Flag indicando se usa CHR-RAM

                // Registros de seleção de banco
                uint8_t m_prgBank;    ///< Banco selecionado para PRG-ROM (8K)
                uint8_t m_chrBank0FD; ///< Banco CHR selecionado para $0000 quando latch 0 = 0xFD
                uint8_t m_chrBank0FE; ///< Banco CHR selecionado para $0000 quando latch 0 = 0xFE
                uint8_t m_chrBank1FD; ///< Banco CHR selecionado para $1000 quando latch 1 = 0xFD
                uint8_t m_chrBank1FE; ///< Banco CHR selecionado para $1000 quando latch 1 = 0xFE

                // Latches de CHR
                uint8_t m_latch0; ///< Latch para o padrão de CHR em $0000-$0FFF (0xFD ou 0xFE)
                uint8_t m_latch1; ///< Latch para o padrão de CHR em $1000-$1FFF (0xFD ou 0xFE)

                // Espelhamento
                MirrorMode m_mirrorMode; ///< Modo de espelhamento

                // Constantes
                static constexpr uint32_t PRG_BANK_SIZE = 8 * 1024; ///< Tamanho do banco PRG (8KB)
                static constexpr uint32_t CHR_BANK_SIZE = 4 * 1024; ///< Tamanho do banco CHR (4KB)
                static constexpr uint16_t LATCH0_TRIGGER = 0x0FD8;  ///< Endereço que aciona o latch 0
                static constexpr uint16_t LATCH1_TRIGGER = 0x1FD8;  ///< Endereço que aciona o latch 1
            };
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu

#endif // MAPPER9_HPP
