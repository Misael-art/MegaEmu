/**
 * @file mapper1.hpp
 * @brief Definição do Mapper 1 (MMC1) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MEGAEMU_NES_MAPPER1_HPP
#define MEGAEMU_NES_MAPPER1_HPP

#include "../mapper.hpp"
#include "../cartridge.hpp"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            /**
             * @brief Implementação do Mapper 1 (MMC1) para o NES
             *
             * O MMC1 é um mapper relativamente sofisticado que utiliza escrita serial
             * para configurar registradores que controlam o bank switching.
             *
             * Características:
             * - Suporta até 512KB de PRG-ROM
             * - Suporta até 128KB de CHR-ROM ou 8KB de CHR-RAM
             * - Possui 8KB de PRG-RAM com suporte opcional a bateria
             * - Oferece múltiplos modos de controle para bancos de PRG-ROM
             * - Oferece múltiplos modos de controle para bancos de CHR-ROM
             * - Permite controle flexível de espelhamento de nametable
             *
             * Jogos populares que usam este mapper incluem:
             * - The Legend of Zelda
             * - Metroid
             * - Final Fantasy
             * - Mega Man 2
             * - Dragon Warrior
             */
            class Mapper1 : public Mapper
            {
            public:
                /**
                 * @brief Construtor do Mapper 1 (MMC1)
                 * @param cartridge Ponteiro para o cartucho
                 */
                Mapper1(Cartridge *cartridge);

                /**
                 * @brief Destrutor do Mapper 1
                 */
                ~Mapper1();

                /**
                 * @brief Reset do mapper para estado inicial
                 */
                void reset() override;

                /**
                 * @brief Leitura de memória pelo CPU
                 * @param address Endereço de memória (0x6000-0xFFFF)
                 * @return Byte lido
                 */
                uint8_t cpuRead(uint16_t address) override;

                /**
                 * @brief Escrita de memória pelo CPU
                 * @param address Endereço de memória (0x6000-0xFFFF)
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
                 * Não utilizado pelo MMC1, mas implementado por compatibilidade
                 */
                void scanline() override;

                /**
                 * @brief Verifica o estado atual da linha de IRQ
                 * @return Estado do IRQ (sempre falso para MMC1)
                 */
                bool irqState() override;

                /**
                 * @brief Limpa o sinal de IRQ
                 * Não utilizado pelo MMC1, mas implementado por compatibilidade
                 */
                void irqClear() override;

            private:
                /**
                 * @brief Atualiza os bancos de PRG-ROM com base nos registros
                 */
                void updatePrgBanks();

                /**
                 * @brief Atualiza os bancos de CHR-ROM com base nos registros
                 */
                void updateChrBanks();

                /**
                 * @brief Atualiza o modo de espelhamento com base nos registros
                 */
                void updateMirroring();

                // Definição de enums para modos de configuração
                enum class PrgMode : uint8_t
                {
                    SWITCH_32K = 0,       // Controle de 32KB
                    SWITCH_FIX_FIRST = 1, // Fix primeiro banco, troca último
                    SWITCH_FIX_LAST = 2,  // Fix último banco, troca primeiro
                    SWITCH_16K = 3        // Controle de 16KB
                };

                enum class ChrMode : uint8_t
                {
                    SWITCH_8K = 0, // Controle de 8KB (um único banco)
                    SWITCH_4K = 1  // Controle de 4KB (dois bancos independentes)
                };

                // Campos privados
                Cartridge *m_cartridge;  ///< Ponteiro para o cartucho
                uint32_t m_prgRomSize;   ///< Tamanho da PRG-ROM em bytes
                uint32_t m_chrRomSize;   ///< Tamanho da CHR-ROM em bytes
                bool m_usesChrRam;       ///< Indica se o mapper usa CHR-RAM
                bool m_hasPrgRam;        ///< Indica se o cartucho tem PRG-RAM
                bool m_hasBatteryBacked; ///< Indica se a PRG-RAM é salva em bateria

                // Registradores de status
                uint8_t m_shiftRegister; ///< Registrador de deslocamento para escrita serial
                uint8_t m_shiftCount;    ///< Contador de bits escritos
                uint8_t m_control;       ///< Registrador de controle (0x8000-0x9FFF)
                uint8_t m_chrBank0;      ///< Seleção de banco CHR 0 (0xA000-0xBFFF)
                uint8_t m_chrBank1;      ///< Seleção de banco CHR 1 (0xC000-0xDFFF)
                uint8_t m_prgBank;       ///< Seleção de banco PRG (0xE000-0xFFFF)

                // Status derivado dos registradores
                PrgMode m_prgMode;       ///< Modo atual de bank switching de PRG
                ChrMode m_chrMode;       ///< Modo atual de bank switching de CHR
                MirrorMode m_mirrorMode; ///< Modo atual de espelhamento

                // Bancos selecionados
                uint8_t m_prgBank0;         ///< Banco PRG em $8000-$BFFF
                uint8_t m_prgBank1;         ///< Banco PRG em $C000-$FFFF
                uint8_t m_chrBank0Selected; ///< Banco CHR 0 selecionado
                uint8_t m_chrBank1Selected; ///< Banco CHR 1 selecionado

                // Constantes para tamanhos de banco
                static constexpr uint32_t PRG_BANK_SIZE = 16 * 1024; ///< Tamanho de um banco PRG (16KB)
                static constexpr uint32_t CHR_BANK_SIZE = 4 * 1024;  ///< Tamanho de um banco CHR (4KB)
                static constexpr uint32_t PRG_RAM_SIZE = 8 * 1024;   ///< Tamanho da PRG-RAM (8KB)
            };
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu

#endif // MEGAEMU_NES_MAPPER1_HPP
