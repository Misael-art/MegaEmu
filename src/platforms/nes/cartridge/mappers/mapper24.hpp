/**
 * @file mapper24.hpp
 * @brief Definição do Mapper 24 (VRC6) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MEGAEMU_NES_MAPPER24_HPP
#define MEGAEMU_NES_MAPPER24_HPP

#include "../mapper.hpp"
#include "../cartridge.hpp"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            /**
             * @brief Implementação do Mapper 24 (VRC6) para o NES
             *
             * O VRC6 (Konami VRC6) é um mapper avançado desenvolvido pela Konami,
             * que fornece capacidades de som expandidas além do bank switching.
             * O mapper oferece três canais extras de áudio: dois pulsos e um dente de serra.
             *
             * Características:
             * - Suporta até 512KB de PRG-ROM (32 bancos de 16KB)
             * - Suporta até 256KB de CHR-ROM (16 bancos de 8KB)
             * - Bank switching para PRG-ROM e CHR-ROM
             * - 8KB de PRG-RAM com bateria opcional
             * - IRQs programáveis baseados em contadores
             * - Modos de espelhamento configuráveis
             * - Hardware de áudio adicional integrado (2 pulsos e 1 dente de serra)
             *
             * Jogos populares que usam este mapper incluem:
             * - Akumajou Densetsu (Castlevania III no Japão)
             * - Esper Dream 2
             * - Madara
             * - Mouryou Senki Madara
             */
            class Mapper24 : public Mapper
            {
            public:
                /**
                 * @brief Construtor do Mapper 24 (VRC6)
                 * @param cartridge Ponteiro para o cartucho
                 */
                Mapper24(Cartridge *cartridge);

                /**
                 * @brief Destrutor do Mapper 24
                 */
                ~Mapper24();

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
                 * Usado para atualizar o contador de IRQ do VRC6
                 */
                void scanline() override;

                /**
                 * @brief Verifica o estado atual da linha de IRQ
                 * @return Estado do IRQ
                 */
                bool irqState() override;

                /**
                 * @brief Limpa o sinal de IRQ
                 */
                void irqClear() override;

                /**
                 * @brief Gera amostras de áudio do VRC6
                 * @note Esta função será chamada pelo APU para mixar os canais do VRC6
                 * @param buffer Buffer de áudio de saída
                 * @param sampleCount Número de amostras a gerar
                 */
                void generateAudio(float *buffer, int sampleCount);

            private:
                Cartridge *m_cartridge; ///< Ponteiro para o cartucho
                uint32_t m_prgRomSize;  ///< Tamanho da PRG-ROM em bytes
                uint32_t m_chrRomSize;  ///< Tamanho da CHR-ROM em bytes
                bool m_usesChrRam;      ///< Flag indicando se o mapper usa CHR-RAM
                bool m_hasPrgRam;       ///< Flag indicando se há PRG-RAM
                bool m_hasBattery;      ///< Flag indicando se tem bateria para salvar

                // Registradores de seleção de bancos
                uint8_t m_prgBank[2]; ///< Bancos de PRG-ROM selecionados
                uint8_t m_chrBank[8]; ///< Bancos de CHR-ROM selecionados

                // Registradores de controle
                MirrorMode m_mirrorMode;   ///< Modo de espelhamento
                bool m_prgRamEnabled;      ///< Flag de habilitação da PRG-RAM
                bool m_prgRamWriteProtect; ///< Flag de proteção de escrita da PRG-RAM

                // Registradores de IRQ
                bool m_irqEnabled;       ///< Flag de habilitação do IRQ
                bool m_irqPending;       ///< Flag indicando IRQ pendente
                uint8_t m_irqLatch;      ///< Valor a ser carregado no contador de IRQ
                uint8_t m_irqCounter;    ///< Contador de IRQ
                bool m_irqMode;          ///< Modo de IRQ (0: scanline, 1: ciclos do CPU)
                uint16_t m_irqPrescaler; ///< Prescaler do contador de IRQ

                // Registradores de áudio
                struct
                {
                    bool enabled;       ///< Canal ativado
                    uint8_t volume;     ///< Volume (0-15)
                    uint8_t duty;       ///< Ciclo de trabalho (0-7)
                    uint16_t frequency; ///< Frequência
                    uint16_t timer;     ///< Temporizador atual
                    uint8_t sequencer;  ///< Sequenciador de pulso
                } m_pulse[2];           ///< Canais de pulso do VRC6

                struct
                {
                    bool enabled;            ///< Canal ativado
                    uint8_t accumulatorRate; ///< Taxa de acumulação (0-42)
                    uint16_t frequency;      ///< Frequência
                    uint16_t timer;          ///< Temporizador atual
                    uint8_t accumulator;     ///< Acumulador atual
                } m_saw;                     ///< Canal de dente de serra do VRC6

                // Constantes
                static constexpr uint32_t PRG_BANK_SIZE = 16 * 1024; ///< Tamanho do banco PRG (16KB)
                static constexpr uint32_t CHR_BANK_SIZE = 1 * 1024;  ///< Tamanho do banco CHR (1KB)
                static constexpr uint32_t PRG_RAM_SIZE = 8 * 1024;   ///< Tamanho da PRG-RAM (8KB)

                // Funções auxiliares
                void updateIrqCounter();
                void updateAudio(int cycles);
            };
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu

#endif // MEGAEMU_NES_MAPPER24_HPP
