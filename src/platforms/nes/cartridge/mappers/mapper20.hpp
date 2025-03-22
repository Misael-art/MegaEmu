/**
 * @file mapper20.hpp
 * @brief Definição do Mapper 20 (FDS - Famicom Disk System) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MEGAEMU_NES_MAPPER20_HPP
#define MEGAEMU_NES_MAPPER20_HPP

#include "../mapper.hpp"
#include "../cartridge.hpp"
#include <vector>

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            /**
             * @brief Implementação do Mapper 20 (FDS - Famicom Disk System) para o NES
             *
             * O FDS é uma expansão que permite ao console NES/Famicom ler jogos
             * a partir de disquetes proprietários. Ele possui hardware de som
             * adicional, I/O para controle do drive de disquete, e RAM interna.
             *
             * Características:
             * - Memória RAM de 32KB para código e dados
             * - RAM de expansão de 8KB
             * - Carrega dados de disquetes de até 65.500 bytes por lado
             * - Hardware de áudio adicional com canal de modulação de onda
             * - I/O para controle de leitura/escrita de disquete
             * - Registradores para controle de transferência de dados
             * - IRQs programáveis
             *
             * Jogos populares que usam FDS incluem:
             * - The Legend of Zelda (versão japonesa original)
             * - Metroid (versão japonesa original)
             * - Kid Icarus (versão japonesa original)
             * - Castlevania (versão japonesa original)
             * - Doki Doki Panic (base para Super Mario Bros. 2 ocidental)
             */
            class Mapper20 : public Mapper
            {
            public:
                /**
                 * @brief Construtor do Mapper 20 (FDS)
                 * @param cartridge Ponteiro para o cartucho
                 */
                Mapper20(Cartridge *cartridge);

                /**
                 * @brief Destrutor do Mapper 20
                 */
                ~Mapper20();

                /**
                 * @brief Reset do mapper para estado inicial
                 */
                void reset() override;

                /**
                 * @brief Leitura de memória pelo CPU
                 * @param address Endereço de memória (0x4020-0xFFFF)
                 * @return Byte lido
                 */
                uint8_t cpuRead(uint16_t address) override;

                /**
                 * @brief Escrita de memória pelo CPU
                 * @param address Endereço de memória (0x4020-0xFFFF)
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
                 * Usado para atualizar o contador de IRQ do FDS
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
                 * @brief Gera amostras de áudio do FDS
                 * @note Esta função será chamada pelo APU para mixar os canais FDS
                 * @param buffer Buffer de áudio de saída
                 * @param sampleCount Número de amostras a gerar
                 */
                void generateAudio(float *buffer, int sampleCount);

                /**
                 * @brief Carrega um arquivo de disquete FDS
                 * @param diskData Dados do arquivo de disquete
                 * @param size Tamanho dos dados
                 * @return true se o carregamento foi bem-sucedido
                 */
                bool loadDiskImage(const uint8_t *diskData, size_t size);

                /**
                 * @brief Ejeta o disquete atual
                 */
                void ejectDisk();

                /**
                 * @brief Insere um disquete
                 * @param diskNumber Número do disquete (geralmente 0 ou 1)
                 * @param side Lado do disquete (0 ou 1)
                 * @return true se o disquete foi inserido com sucesso
                 */
                bool insertDisk(uint8_t diskNumber, uint8_t side);

                /**
                 * @brief Retorna o número de disquetes disponíveis
                 * @return Quantidade de disquetes
                 */
                uint8_t getDiskCount() const;

            private:
                Cartridge *m_cartridge; ///< Ponteiro para o cartucho

                // RAM interna do FDS
                std::vector<uint8_t> m_ram;          ///< 32KB de RAM principal
                std::vector<uint8_t> m_expansionRam; ///< 8KB de RAM de expansão

                // Registradores de status/controle
                uint8_t m_irqCounter;  ///< Contador de IRQ
                uint8_t m_irqLatch;    ///< Valor de recarga do contador IRQ
                bool m_irqEnabled;     ///< Habilita/desabilita IRQ
                bool m_irqPending;     ///< Flag de IRQ pendente
                bool m_diskInserted;   ///< Status de inserção do disquete
                uint8_t m_currentDisk; ///< Disco atual
                uint8_t m_currentSide; ///< Lado atual do disco

                // Registradores de controle de disco
                uint8_t m_diskStatus;       ///< Status do disquete
                bool m_diskIRQEnabled;      ///< Habilita/desabilita IRQ do disco
                bool m_diskMotorOn;         ///< Status do motor do drive
                bool m_diskWriteMode;       ///< Modo de escrita/leitura
                uint16_t m_diskPosition;    ///< Posição atual no disco
                uint8_t m_diskReadWriteReg; ///< Registrador de leitura/escrita

                // Registradores de áudio (canal de onda+modulação)
                bool m_audioEnabled;       ///< Habilita/desabilita áudio
                uint8_t m_waveTable[64];   ///< Tabela de forma de onda
                uint8_t m_waveWriteEnable; ///< Habilita/desabilita escrita na tabela
                uint8_t m_masterVolume;    ///< Volume principal

                // Registradores de canal principal
                uint16_t m_frequency;       ///< Frequência do canal de onda
                uint8_t m_volume;           ///< Volume do canal de onda
                uint16_t m_waveAccumulator; ///< Acumulador da posição da tabela

                // Registradores de modulação
                bool m_modulationEnabled;      ///< Habilita/desabilita modulação
                uint16_t m_modulationFreq;     ///< Frequência de modulação
                uint8_t m_modulationDepth;     ///< Profundidade de modulação
                uint16_t m_modulationAccum;    ///< Acumulador de modulação
                uint8_t m_modulationTable[64]; ///< Tabela de modulação

                // Dados de disquetes
                struct Disk
                {
                    std::vector<uint8_t> data; ///< Dados do disquete
                    uint8_t sides;             ///< Número de lados
                };
                std::vector<Disk> m_disks; ///< Vetor de disquetes disponíveis

                // Constantes
                static constexpr uint32_t RAM_SIZE = 32 * 1024;          ///< 32KB RAM
                static constexpr uint32_t EXPANSION_RAM_SIZE = 8 * 1024; ///< 8KB RAM de expansão
                static constexpr uint32_t DISK_SIDE_CAPACITY = 65500;    ///< Capacidade de um lado do disquete

                // Funções auxiliares
                void updateIrqCounter();
                void updateAudio(int cycles);
                uint8_t readDiskData();
                void writeDiskData(uint8_t data);
                void advanceDiskHead();
            };
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu

#endif // MEGAEMU_NES_MAPPER20_HPP
