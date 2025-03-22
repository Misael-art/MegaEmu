/**
 * @file mapper5.hpp
 * @brief Implementação do Mapper 5 (MMC5/ExROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-28
 */

#ifndef MAPPER5_HPP
#define MAPPER5_HPP

#include <cstdint>
#include "../cartridge.hpp"
#include "mapper.hpp"

/**
 * @class Mapper5
 * @brief Implementação do Mapper 5 (MMC5/ExROM) para o NES
 *
 * O MMC5 é um mapper avançado com as seguintes características:
 * - Suporte a até 1MB de PRG-ROM
 * - Suporte a até 256KB de CHR-ROM
 * - 64KB de WRAM expansível
 * - Modos de Split Screen
 * - Gerador de pulso de áudio
 * - Multiplicador de hardware
 * - Múltiplos modos de nametable
 */
class Mapper5 : public Mapper
{
public:
    /**
     * @brief Construtor para o Mapper 5
     * @param cartridge Ponteiro para o cartucho
     */
    Mapper5(Cartridge *cartridge);

    /**
     * @brief Destrutor para o Mapper 5
     */
    ~Mapper5();

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

    /**
     * @brief Lê um byte do espaço de endereçamento de nametable/CIRAM
     * @param address Endereço (0x2000-0x3EFF)
     * @return Byte lido
     */
    uint8_t ntRead(uint16_t address);

    /**
     * @brief Escreve um byte no espaço de endereçamento de nametable/CIRAM
     * @param address Endereço (0x2000-0x3EFF)
     * @param data Byte a ser escrito
     */
    void ntWrite(uint16_t address, uint8_t data);

    /**
     * @brief Gera sample de áudio do canal de pulso do MMC5
     * @return Sample de áudio (-1.0 a 1.0)
     */
    float getAudioSample();

private:
    Cartridge *m_cartridge; ///< Ponteiro para o cartucho

    // Registradores do MMC5
    uint8_t m_prgMode;         ///< Modo de PRG ROM (0x5100)
    uint8_t m_chrMode;         ///< Modo de CHR ROM (0x5101)
    uint8_t m_prgRamProtect1;  ///< Proteção de PRG-RAM 1 (0x5102)
    uint8_t m_prgRamProtect2;  ///< Proteção de PRG-RAM 2 (0x5103)
    uint8_t m_extendedRamMode; ///< Modo de Extended RAM (0x5104)
    uint8_t m_ntMapping;       ///< Mapeamento de Nametable (0x5105)
    uint8_t m_fillModeTile;    ///< Tile para modo de preenchimento (0x5106)
    uint8_t m_fillModeColor;   ///< Cor para modo de preenchimento (0x5107)

    // Registradores de PRG
    uint8_t m_prgBankReg[5]; ///< Registradores de banco de PRG (0x5113-0x5117)

    // Registradores de CHR
    uint8_t m_chrBankReg[12]; ///< Registradores de banco de CHR (0x5120-0x512B)

    // Multiplicador
    uint8_t m_multiplicand; ///< Multiplicando (0x5205)
    uint8_t m_multiplier;   ///< Multiplicador (0x5206)

    // IRQ
    uint8_t m_irqScanlineCmp; ///< Scanline de comparação para IRQ (0x5203)
    uint8_t m_irqStatus;      ///< Status do IRQ (0x5204)
    bool m_irqEnabled;        ///< Flag de habilitação do IRQ

    // Registradores de Split Mode
    uint8_t m_splitModeCtrl;   ///< Controle do modo Split (0x5200)
    uint8_t m_splitModeTile;   ///< Tile para modo Split (0x5201)
    uint8_t m_splitModeScroll; ///< Scroll para modo Split (0x5202)

    // Áudio
    uint8_t m_audioCtrl;        ///< Controle de áudio (0x5015)
    uint8_t m_pulseCtrl;        ///< Controle do pulso (0x5000)
    uint8_t m_pulseSweep;       ///< Sweep do pulso (0x5001)
    uint8_t m_pulseTimer;       ///< Timer do pulso (0x5002)
    uint8_t m_pulseTimerHigh;   ///< Parte alta do timer do pulso (0x5003)
    uint16_t m_pulseTimerValue; ///< Valor atual do timer do pulso
    uint8_t m_pulseLength;      ///< Contador de length do pulso
    uint8_t m_pulseSeq;         ///< Sequenciador do pulso
    uint8_t m_pulseVol;         ///< Volume do pulso

    // EXRAM (1KB)
    uint8_t m_exram[1024]; ///< RAM estendida

    // Mapeamento de bancos
    uint32_t m_prgOffsets[5];  ///< Offset para bancos de PRG (8KB cada)
    uint32_t m_chrOffsets[12]; ///< Offset para bancos de CHR (1KB cada)

    // Informações do cartucho
    uint32_t m_prgRomSize; ///< Tamanho da PRG-ROM
    uint32_t m_chrRomSize; ///< Tamanho da CHR-ROM
    bool m_usesChrRam;     ///< Flag indicando se usa CHR-RAM

    // Estado interno
    uint16_t m_currentScanline; ///< Scanline atual
    bool m_inFrame;             ///< Flag indicando se está em um frame

    /**
     * @brief Atualiza o mapeamento de bancos após mudança nos registradores
     */
    void updatePrgBanks();

    /**
     * @brief Atualiza o mapeamento de CHR após mudança nos registradores
     */
    void updateChrBanks();

    /**
     * @brief Processa o tick do canal de áudio do pulso
     */
    void tickAudio();
};

#endif // MAPPER5_HPP
