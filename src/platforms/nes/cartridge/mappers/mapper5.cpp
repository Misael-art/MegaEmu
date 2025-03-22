/**
 * @file mapper5.cpp
 * @brief Implementação do Mapper 5 (MMC5/ExROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-28
 */

#include <cstdio>
#include <cstring>
#include "mapper5.hpp"
#include "../../../../utils/log_utils.h"

/**
 * @brief Construtor para o Mapper 5
 * @param cartridge Ponteiro para o cartucho
 */
Mapper5::Mapper5(Cartridge *cartridge) : m_cartridge(cartridge)
{
    // Verificar se o cartucho é válido
    if (!cartridge)
    {
        LOG_ERROR("Mapper5: Cartucho inválido");
        return;
    }

    // Obter tamanhos das ROMs
    m_prgRomSize = cartridge->prg_rom_size;
    m_chrRomSize = cartridge->chr_rom_size;

    // Verificar tamanhos
    if (m_prgRomSize == 0)
    {
        LOG_ERROR("Mapper5: Tamanho de PRG-ROM inválido: %u", m_prgRomSize);
        return;
    }

    // Determinar se usa CHR-RAM
    m_usesChrRam = (m_chrRomSize == 0 && cartridge->chr_ram != nullptr);

    if (m_chrRomSize == 0 && !m_usesChrRam)
    {
        LOG_ERROR("Mapper5: Sem CHR-ROM ou CHR-RAM");
        return;
    }

    // Inicializar registradores
    m_prgMode = 0;
    m_chrMode = 0;
    m_prgRamProtect1 = 0;
    m_prgRamProtect2 = 0;
    m_extendedRamMode = 0;
    m_ntMapping = 0;
    m_fillModeTile = 0;
    m_fillModeColor = 0;

    // Inicializar registradores de PRG
    for (int i = 0; i < 5; i++)
    {
        m_prgBankReg[i] = 0;
    }

    // Inicializar registradores de CHR
    for (int i = 0; i < 12; i++)
    {
        m_chrBankReg[i] = 0;
    }

    // Inicializar multiplicador
    m_multiplicand = 0;
    m_multiplier = 0;

    // Inicializar IRQ
    m_irqScanlineCmp = 0;
    m_irqStatus = 0;
    m_irqEnabled = false;

    // Inicializar Split Mode
    m_splitModeCtrl = 0;
    m_splitModeTile = 0;
    m_splitModeScroll = 0;

    // Inicializar áudio
    m_audioCtrl = 0;
    m_pulseCtrl = 0;
    m_pulseSweep = 0;
    m_pulseTimer = 0;
    m_pulseTimerHigh = 0;
    m_pulseTimerValue = 0;
    m_pulseLength = 0;
    m_pulseSeq = 0;
    m_pulseVol = 0;

    // Inicializar EXRAM
    memset(m_exram, 0, sizeof(m_exram));

    // Inicializar estado
    m_currentScanline = 0;
    m_inFrame = false;

    // Executar reset para inicializar os mapeamentos
    reset();

    LOG_INFO("Mapper5 (MMC5/ExROM) inicializado: PRG-ROM=%uKB, CHR-%s=%uKB",
             m_prgRomSize / 1024,
             m_usesChrRam ? "RAM" : "ROM",
             (m_usesChrRam ? cartridge->chr_ram_size : m_chrRomSize) / 1024);
}

/**
 * @brief Destrutor para o Mapper 5
 */
Mapper5::~Mapper5()
{
    // Nada a fazer aqui
}

/**
 * @brief Reseta o estado do mapper
 */
void Mapper5::reset()
{
    // Resetar registradores
    m_prgMode = 3; // Modo 3 por padrão
    m_chrMode = 0;
    m_prgRamProtect1 = 0;
    m_prgRamProtect2 = 0;
    m_extendedRamMode = 0;
    m_ntMapping = 0;
    m_fillModeTile = 0;
    m_fillModeColor = 0;

    // Inicializar registradores de PRG
    for (int i = 0; i < 5; i++)
    {
        m_prgBankReg[i] = 0;
    }
    // Último banco de PRG é fixo no último banco
    m_prgBankReg[4] = 0xFF;

    // Inicializar registradores de CHR
    for (int i = 0; i < 12; i++)
    {
        m_chrBankReg[i] = 0;
    }

    // Resetar multiplicador
    m_multiplicand = 0;
    m_multiplier = 0;

    // Resetar IRQ
    m_irqScanlineCmp = 0;
    m_irqStatus = 0;
    m_irqEnabled = false;

    // Resetar Split Mode
    m_splitModeCtrl = 0;
    m_splitModeTile = 0;
    m_splitModeScroll = 0;

    // Resetar áudio
    m_audioCtrl = 0;
    m_pulseCtrl = 0;
    m_pulseSweep = 0;
    m_pulseTimer = 0;
    m_pulseTimerHigh = 0;
    m_pulseTimerValue = 0;
    m_pulseLength = 0;
    m_pulseSeq = 0;
    m_pulseVol = 0;

    // Limpar EXRAM
    memset(m_exram, 0, sizeof(m_exram));

    // Resetar estado
    m_currentScanline = 0;
    m_inFrame = false;

    // Atualizar mapeamentos de banco
    updatePrgBanks();
    updateChrBanks();

    LOG_INFO("Mapper5 (MMC5/ExROM) resetado");
}

/**
 * @brief Atualiza o mapeamento de bancos após mudança nos registradores
 */
void Mapper5::updatePrgBanks()
{
    uint32_t prgBankSize = 0;
    uint32_t prgBanksTotal = 0;

    // Determinar o tamanho do banco com base no modo
    switch (m_prgMode)
    {
    case 0: // Modo 0: Um banco de 32KB
        prgBankSize = 32 * 1024;
        prgBanksTotal = m_prgRomSize / prgBankSize;

        // Em modo 0, apenas m_prgBankReg[4] é usado
        m_prgOffsets[0] = (m_prgBankReg[4] & 0x7C) % prgBanksTotal * prgBankSize;
        break;

    case 1: // Modo 1: Dois bancos de 16KB
        prgBankSize = 16 * 1024;
        prgBanksTotal = m_prgRomSize / prgBankSize;

        // $8000-$BFFF: Selecionado pelo reg 2 (bit 7-1)
        m_prgOffsets[0] = (m_prgBankReg[2] & 0xFE) % prgBanksTotal * prgBankSize;

        // $C000-$FFFF: Selecionado pelo reg 4 (bit 7-1)
        m_prgOffsets[1] = (m_prgBankReg[4] & 0xFE) % prgBanksTotal * prgBankSize;
        break;

    case 2: // Modo 2: Um banco de 16KB e dois de 8KB
        // $8000-$BFFF: Banco de 16KB
        m_prgOffsets[0] = (m_prgBankReg[2] & 0xFE) % (m_prgRomSize / (16 * 1024)) * (16 * 1024);

        // $C000-$DFFF: Primeiro banco de 8KB
        m_prgOffsets[1] = m_prgBankReg[3] % (m_prgRomSize / (8 * 1024)) * (8 * 1024);

        // $E000-$FFFF: Segundo banco de 8KB
        m_prgOffsets[2] = m_prgBankReg[4] % (m_prgRomSize / (8 * 1024)) * (8 * 1024);
        break;

    case 3: // Modo 3: Quatro bancos de 8KB
    default:
        prgBankSize = 8 * 1024;
        prgBanksTotal = m_prgRomSize / prgBankSize;

        // $8000-$9FFF: Banco 0
        m_prgOffsets[0] = m_prgBankReg[0] % prgBanksTotal * prgBankSize;

        // $A000-$BFFF: Banco 1
        m_prgOffsets[1] = m_prgBankReg[1] % prgBanksTotal * prgBankSize;

        // $C000-$DFFF: Banco 2
        m_prgOffsets[2] = m_prgBankReg[2] % prgBanksTotal * prgBankSize;

        // $E000-$FFFF: Banco 3
        m_prgOffsets[3] = m_prgBankReg[3] % prgBanksTotal * prgBankSize;
        break;
    }

    LOG_DEBUG("Mapper5: PRG modo %d, bancos atualizados", m_prgMode);
}

/**
 * @brief Atualiza o mapeamento de CHR após mudança nos registradores
 */
void Mapper5::updateChrBanks()
{
    uint32_t chrBankSize = 0;
    uint32_t chrBanksTotal = 0;

    // Tamanho total da CHR (ROM ou RAM)
    uint32_t chrSize = m_usesChrRam ? m_cartridge->chr_ram_size : m_chrRomSize;

    // Determinar o tamanho do banco com base no modo
    switch (m_chrMode)
    {
    case 0: // Modo 0: Um banco de 8KB
        chrBankSize = 8 * 1024;
        chrBanksTotal = chrSize / chrBankSize;

        // Em modo 0, apenas m_chrBankReg[0] é usado (8KB em $0000-$1FFF)
        m_chrOffsets[0] = (m_chrBankReg[0] & 0xFE) % chrBanksTotal * chrBankSize;
        break;

    case 1: // Modo 1: Dois bancos de 4KB
        chrBankSize = 4 * 1024;
        chrBanksTotal = chrSize / chrBankSize;

        // $0000-$0FFF: Selecionado pelo reg 0
        m_chrOffsets[0] = m_chrBankReg[0] % chrBanksTotal * chrBankSize;

        // $1000-$1FFF: Selecionado pelo reg 1
        m_chrOffsets[1] = m_chrBankReg[1] % chrBanksTotal * chrBankSize;
        break;

    case 2: // Modo 2: Quatro bancos de 2KB
        chrBankSize = 2 * 1024;
        chrBanksTotal = chrSize / chrBankSize;

        // $0000-$07FF: Selecionado pelo reg 0
        m_chrOffsets[0] = m_chrBankReg[0] % chrBanksTotal * chrBankSize;

        // $0800-$0FFF: Selecionado pelo reg 1
        m_chrOffsets[1] = m_chrBankReg[1] % chrBanksTotal * chrBankSize;

        // $1000-$17FF: Selecionado pelo reg 2
        m_chrOffsets[2] = m_chrBankReg[2] % chrBanksTotal * chrBankSize;

        // $1800-$1FFF: Selecionado pelo reg 3
        m_chrOffsets[3] = m_chrBankReg[3] % chrBanksTotal * chrBankSize;
        break;

    case 3: // Modo 3: Oito bancos de 1KB
    default:
        chrBankSize = 1 * 1024;
        chrBanksTotal = chrSize / chrBankSize;

        // Oito bancos de 1KB
        for (int i = 0; i < 8; i++)
        {
            m_chrOffsets[i] = m_chrBankReg[i] % chrBanksTotal * chrBankSize;
        }
        break;
    }

    LOG_DEBUG("Mapper5: CHR modo %d, bancos atualizados", m_chrMode);
}

/**
 * @brief Processa o tick do canal de áudio do pulso
 */
void Mapper5::tickAudio()
{
    // Apenas se o canal estiver habilitado
    if ((m_audioCtrl & 0x01) == 0)
    {
        return;
    }

    // Processar timer
    if (m_pulseTimerValue > 0)
    {
        m_pulseTimerValue--;
    }
    else
    {
        // Recarregar timer
        m_pulseTimerValue = ((m_pulseTimerHigh & 0x07) << 8) | m_pulseTimer;

        // Avançar o sequenciador
        m_pulseSeq = (m_pulseSeq + 1) & 0x07;
    }

    // Processar length counter se habilitado
    if ((m_pulseCtrl & 0x20) == 0 && m_pulseLength > 0)
    {
        m_pulseLength--;
    }

    // Processar envelope de volume se habilitado
    if ((m_pulseCtrl & 0x10) == 0)
    {
        // Decaimento simples
        if (m_pulseVol > 0)
        {
            m_pulseVol--;
        }
    }
    else
    {
        // Volume constante
        m_pulseVol = m_pulseCtrl & 0x0F;
    }
}

/**
 * @brief Gera sample de áudio do canal de pulso do MMC5
 * @return Sample de áudio (-1.0 a 1.0)
 */
float Mapper5::getAudioSample()
{
    // Se o canal estiver desabilitado ou silenciado, retorna 0
    if ((m_audioCtrl & 0x01) == 0 || m_pulseLength == 0)
    {
        return 0.0f;
    }

    // Determinar o ciclo de trabalho (duty cycle)
    uint8_t duty = (m_pulseCtrl >> 6) & 0x03;

    // Tabela de duty cycles
    static const uint8_t dutyLookup[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
        {0, 0, 0, 0, 0, 0, 1, 1}, // 25%
        {0, 0, 0, 0, 1, 1, 1, 1}, // 50%
        {1, 1, 1, 1, 1, 1, 0, 0}  // 75% (inverso de 25%)
    };

    // Saída do canal
    uint8_t output = dutyLookup[duty][m_pulseSeq] ? m_pulseVol : 0;

    // Converter para float normalizado (-1.0 a 1.0)
    return (output / 15.0f) * 2.0f - 1.0f;
}

/**
 * @brief Lê um byte da memória mapeada pela CPU
 * @param address Endereço a ser lido (0x0000-0xFFFF)
 * @return Byte lido
 */
uint8_t Mapper5::cpuRead(uint16_t address)
{
    // RAM Espelhada (0x0000-0x1FFF)
    if (address < 0x2000)
    {
        return m_cartridge->nes_ram[address & 0x07FF];
    }

    // Registros (0x2000-0x3FFF)
    else if (address < 0x4020)
    {
        // Retorna 0; implementação dos registros é feita pelo NES
        return 0;
    }

    // PRG-RAM (0x5000-0x5FFF)
    else if (address >= 0x5000 && address < 0x6000)
    {
        // MMC5 registradores específicos
        return readRegister(address);
    }

    // EXRAM (0x5C00-0x5FFF)
    else if (address >= 0x5C00 && address < 0x6000)
    {
        if (m_extendedRamMode <= 1)
        {
            return m_exram[address - 0x5C00];
        }
        else
        {
            return 0; // Somente leitura em modos 2-3
        }
    }

    // PRG-RAM (0x6000-0x7FFF)
    else if (address >= 0x6000 && address < 0x8000)
    {
        // Verificar proteção da RAM
        if ((m_prgRamProtect1 & 0x03) == 0x02 && m_cartridge->prg_ram)
        {
            uint8_t bank = m_prgBankReg[0] & 0x0F; // Bancos 0-15
            uint32_t prgRamOffset = bank * 8192 + (address - 0x6000);

            // Verificar se o endereço está dentro dos limites
            if (prgRamOffset < m_cartridge->prg_ram_size)
            {
                return m_cartridge->prg_ram[prgRamOffset];
            }
        }
        return 0; // RAM protegida ou inexistente
    }

    // PRG-ROM (0x8000-0xFFFF)
    else if (address >= 0x8000)
    {
        uint32_t addr_offset = 0;

        // Mapear de acordo com o modo PRG
        switch (m_prgMode)
        {
        case 0: // Modo 0: 32KB
            addr_offset = m_prgOffsets[0] + (address - 0x8000);
            break;

        case 1: // Modo 1: Dois bancos de 16KB
            if (address < 0xC000)
            {
                addr_offset = m_prgOffsets[0] + (address - 0x8000);
            }
            else
            {
                addr_offset = m_prgOffsets[1] + (address - 0xC000);
            }
            break;

        case 2: // Modo 2: Um banco de 16KB e dois de 8KB
            if (address < 0xC000)
            {
                addr_offset = m_prgOffsets[0] + (address - 0x8000);
            }
            else if (address < 0xE000)
            {
                addr_offset = m_prgOffsets[1] + (address - 0xC000);
            }
            else
            {
                addr_offset = m_prgOffsets[2] + (address - 0xE000);
            }
            break;

        case 3: // Modo 3: Quatro bancos de 8KB
        default:
            if (address < 0xA000)
            {
                addr_offset = m_prgOffsets[0] + (address - 0x8000);
            }
            else if (address < 0xC000)
            {
                addr_offset = m_prgOffsets[1] + (address - 0xA000);
            }
            else if (address < 0xE000)
            {
                addr_offset = m_prgOffsets[2] + (address - 0xC000);
            }
            else
            {
                addr_offset = m_prgOffsets[3] + (address - 0xE000);
            }
            break;
        }

        // Verificar limites
        if (addr_offset < m_prgRomSize)
        {
            return m_cartridge->prg_rom[addr_offset];
        }
    }

    LOG_WARNING("Mapper5: Leitura de CPU não mapeada em $%04X", address);
    return 0;
}

/**
 * @brief Escreve um byte na memória mapeada pela CPU
 * @param address Endereço a ser escrito (0x0000-0xFFFF)
 * @param data Byte a ser escrito
 */
void Mapper5::cpuWrite(uint16_t address, uint8_t data)
{
    // RAM Espelhada (0x0000-0x1FFF)
    if (address < 0x2000)
    {
        m_cartridge->nes_ram[address & 0x07FF] = data;
    }

    // Registros (0x2000-0x3FFF)
    else if (address < 0x4020)
    {
        // Implementado pelo NES
    }

    // MMC5 registradores específicos (0x5000-0x5FFF)
    else if (address >= 0x5000 && address < 0x6000)
    {
        writeRegister(address, data);
    }

    // EXRAM (0x5C00-0x5FFF)
    else if (address >= 0x5C00 && address < 0x6000)
    {
        // Modo 0-1: Escrita permitida
        if (m_extendedRamMode <= 1)
        {
            m_exram[address - 0x5C00] = data;
        }
    }

    // PRG-RAM (0x6000-0x7FFF)
    else if (address >= 0x6000 && address < 0x8000)
    {
        // Verificar proteção de escrita
        if ((m_prgRamProtect1 & 0x03) == 0x02 && (m_prgRamProtect2 & 0x03) == 0x01 && m_cartridge->prg_ram)
        {
            uint8_t bank = m_prgBankReg[0] & 0x0F; // Bancos 0-15
            uint32_t prgRamOffset = bank * 8192 + (address - 0x6000);

            // Verificar se o endereço está dentro dos limites
            if (prgRamOffset < m_cartridge->prg_ram_size)
            {
                m_cartridge->prg_ram[prgRamOffset] = data;
            }
        }
    }

    // PRG-ROM (0x8000-0xFFFF)
    else if (address >= 0x8000)
    {
        // Registradores via escrita em PRG ROM
        switch (address & 0xE001)
        {
        case 0x8000: // $8000 - Modo de PRG/CHR
            m_prgMode = (data >> 6) & 0x03;
            m_chrMode = (data >> 4) & 0x03;
            updatePrgBanks();
            updateChrBanks();
            break;

        case 0x8001: // $8001 - Seleção de banco
            uint8_t reg = m_prgMode;
            if (reg < 8)
            {
                // CHR Banco
                m_chrBankReg[reg] = data;
                updateChrBanks();
            }
            else if (reg < 12)
            {
                // PRG Banco 0-3
                m_prgBankReg[reg - 8] = data & 0x7F;
                updatePrgBanks();
            }
            break;
        }
    }
}

/**
 * @brief Lê um registrador específico do MMC5
 * @param address Endereço (0x5000-0x5FFF)
 * @return Valor do registrador
 */
uint8_t Mapper5::readRegister(uint16_t address)
{
    switch (address)
    {
    case 0x5010: // Status do IRQ
        return m_irqStatus;

    case 0x5015: // Status do canal de som
        // Bit 0: Status do canal de pulso
        return (m_pulseLength > 0) ? 0x01 : 0x00;

    case 0x5204: // Status do IRQ
    {
        uint8_t result = m_irqStatus;
        m_irqStatus &= ~0x80; // Limpar flag de IRQ após leitura
        return result;
    }

    case 0x5205: // Resultado da multiplicação (byte baixo)
        return (m_multiplicand * m_multiplier) & 0xFF;

    case 0x5206: // Resultado da multiplicação (byte alto)
        return ((m_multiplicand * m_multiplier) >> 8) & 0xFF;

    default:
        if (address >= 0x5C00 && address < 0x6000)
        {
            // EXRAM
            if (m_extendedRamMode <= 1)
            {
                return m_exram[address - 0x5C00];
            }
        }
        return 0;
    }
}

/**
 * @brief Escreve em um registrador específico do MMC5
 * @param address Endereço (0x5000-0x5FFF)
 * @param data Dado a ser escrito
 */
void Mapper5::writeRegister(uint16_t address, uint8_t data)
{
    switch (address)
    {
    case 0x5000: // Canal de pulso: controle
        m_pulseCtrl = data;
        break;

    case 0x5001: // Canal de pulso: sweep
        m_pulseSweep = data;
        break;

    case 0x5002: // Canal de pulso: timer baixo
        m_pulseTimer = data;
        break;

    case 0x5003: // Canal de pulso: timer alto e length
        m_pulseTimerHigh = data;
        if ((m_audioCtrl & 0x01) != 0)
        {
            // Ajustar length counter
            static const uint8_t lengthTable[32] = {
                10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
                12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};
            m_pulseLength = lengthTable[(data >> 3) & 0x1F];

            // Reiniciar envelope
            m_pulseVol = 15;

            // Reiniciar sequência
            m_pulseSeq = 0;
        }
        break;

    case 0x5010: // Controle de áudio PCM
        m_audioCtrl = data;
        break;

    case 0x5100: // Modo de PRG
        m_prgMode = data & 0x03;
        updatePrgBanks();
        break;

    case 0x5101: // Modo de CHR
        m_chrMode = data & 0x03;
        updateChrBanks();
        break;

    case 0x5102: // Proteção de PRG RAM 1
        m_prgRamProtect1 = data & 0x03;
        break;

    case 0x5103: // Proteção de PRG RAM 2
        m_prgRamProtect2 = data & 0x03;
        break;

    case 0x5104: // Modo de EXRAM
        m_extendedRamMode = data & 0x03;
        break;

    case 0x5105: // Mapeamento de Nametable
        m_ntMapping = data;
        break;

    case 0x5106: // Fill Mode Tile
        m_fillModeTile = data;
        break;

    case 0x5107: // Fill Mode Color
        m_fillModeColor = data & 0x3F;
        break;

    case 0x5113: // PRG RAM Banco
        m_prgBankReg[0] = data & 0x0F;
        break;

    case 0x5114: // PRG Banco 0
    case 0x5115: // PRG Banco 1
    case 0x5116: // PRG Banco 2
    case 0x5117: // PRG Banco 3
        m_prgBankReg[address - 0x5114 + 1] = data;
        updatePrgBanks();
        break;

    case 0x5120: // CHR Banco 0
    case 0x5121: // CHR Banco 1
    case 0x5122: // CHR Banco 2
    case 0x5123: // CHR Banco 3
    case 0x5124: // CHR Banco 4
    case 0x5125: // CHR Banco 5
    case 0x5126: // CHR Banco 6
    case 0x5127: // CHR Banco 7
        m_chrBankReg[address - 0x5120] = data;
        updateChrBanks();
        break;

    case 0x5128: // CHR Banco 8 (Sprites)
    case 0x5129: // CHR Banco 9
    case 0x512A: // CHR Banco 10
    case 0x512B: // CHR Banco 11
        m_chrBankReg[address - 0x5128 + 8] = data;
        updateChrBanks();
        break;

    case 0x5130: // Split Mode CHR Page
        m_splitModeCtrl = data;
        break;

    case 0x5200: // Split Mode Control
        m_splitModeCtrl = data;
        break;

    case 0x5201: // Split Mode Scroll
        m_splitModeScroll = data;
        break;

    case 0x5202: // Split Mode CHR Banco
        m_splitModeTile = data;
        break;

    case 0x5203: // IRQ Scanline Compare
        m_irqScanlineCmp = data;
        break;

    case 0x5204: // IRQ Enable
        m_irqEnabled = (data & 0x80) != 0;
        break;

    case 0x5205: // Multiplicando
        m_multiplicand = data;
        break;

    case 0x5206: // Multiplicador
        m_multiplier = data;
        break;
    }
}

/**
 * @brief Sinaliza o final de um frame
 */
void Mapper5::signalScanline()
{
    // Atualizar status de frame
    if (!m_inFrame)
    {
        m_inFrame = true;
        m_irqStatus |= 0x40; // Setar flag de in-frame
    }

    // Incrementar contador de scanline
    m_currentScanline++;

    // Verificar IRQ
    if (m_inFrame && m_currentScanline == m_irqScanlineCmp)
    {
        m_irqStatus |= 0x80; // Setar flag de IRQ

        // Gerar interrupção se habilitada
        if (m_irqEnabled)
        {
            m_cartridge->triggerIRQ();
        }
    }

    // Final do frame
    if (m_currentScanline >= 240)
    {
        m_currentScanline = 0;
        m_inFrame = false;
        m_irqStatus &= ~0x40; // Limpar flag de in-frame
    }
}

/**
 * @brief Lê um byte da memória de vídeo (PPU)
 * @param address Endereço a ser lido (0x0000-0x3FFF)
 * @return Byte lido
 */
uint8_t Mapper5::ppuRead(uint16_t address)
{
    // CHR-ROM/RAM (0x0000-0x1FFF)
    if (address < 0x2000)
    {
        bool isSpriteAddress = false;
        uint32_t chrOffset = 0;

        // Verificar se está em modo split mode
        if ((m_splitModeCtrl & 0x80) != 0)
        {
            uint8_t splitY = m_splitModeScroll;
            uint8_t currentY = (m_cartridge->ppu_scanline >= 240) ? 0 : m_cartridge->ppu_scanline;

            // Verificar se está na área de split
            bool inSplitRegion = false;

            if ((m_splitModeCtrl & 0x40) != 0)
            {
                // Split à direita
                uint8_t splitX = ((m_splitModeCtrl & 0x1F) << 3) + 8;
                uint8_t currentX = m_cartridge->ppu_dot;
                inSplitRegion = (currentX >= splitX);
            }
            else
            {
                // Split à esquerda
                uint8_t splitX = ((m_splitModeCtrl & 0x1F) << 3);
                uint8_t currentX = m_cartridge->ppu_dot;
                inSplitRegion = (currentX < splitX);
            }

            // Se estiver na área de split, usar o banco específico
            if (inSplitRegion && currentY >= splitY && currentY < (splitY + 8))
            {
                uint8_t pattern = address & 0x0FFF;
                uint8_t bank = m_splitModeTile;
                chrOffset = bank * 4096 + pattern;

                // Verificar limites
                if (m_usesChrRam)
                {
                    if (chrOffset < m_cartridge->chr_ram_size)
                    {
                        return m_cartridge->chr_ram[chrOffset];
                    }
                }
                else
                {
                    if (chrOffset < m_chrRomSize)
                    {
                        return m_cartridge->chr_rom[chrOffset];
                    }
                }
            }
        }

        // Determinar se é acesso de sprite ou background
        // No NES, sprites são buscados durante a avaliação de sprites
        // que acontece em pontos específicos da renderização
        if (m_cartridge->ppu_sprite_evaluation)
        {
            isSpriteAddress = true;
        }

        // Modo CHR e seleção de banco
        uint32_t bankSize = 0;
        uint8_t bankIndex = 0;

        switch (m_chrMode)
        {
        case 0: // Modo 0: Um banco de 8KB
            bankSize = 8 * 1024;
            bankIndex = 0;
            chrOffset = m_chrOffsets[0] + (address & 0x1FFF);
            break;

        case 1: // Modo 1: Dois bancos de 4KB
            bankSize = 4 * 1024;
            bankIndex = (address >> 12) & 0x01;
            chrOffset = m_chrOffsets[bankIndex] + (address & 0x0FFF);
            break;

        case 2: // Modo 2: Quatro bancos de 2KB
            bankSize = 2 * 1024;
            bankIndex = (address >> 11) & 0x03;
            chrOffset = m_chrOffsets[bankIndex] + (address & 0x07FF);
            break;

        case 3: // Modo 3: Oito bancos de 1KB
        default:
            bankSize = 1 * 1024;
            bankIndex = (address >> 10) & 0x07;

            // Se for acesso de sprite, usar bancos 8-11 (específicos para sprites)
            if (isSpriteAddress && (address >= 0x1000))
            {
                chrOffset = m_chrOffsets[bankIndex + 8] + (address & 0x03FF);
            }
            else
            {
                chrOffset = m_chrOffsets[bankIndex] + (address & 0x03FF);
            }
            break;
        }

        // Retornar CHR-ROM ou CHR-RAM
        if (m_usesChrRam)
        {
            if (chrOffset < m_cartridge->chr_ram_size)
            {
                return m_cartridge->chr_ram[chrOffset];
            }
            return 0;
        }
        else
        {
            if (chrOffset < m_chrRomSize)
            {
                return m_cartridge->chr_rom[chrOffset];
            }
            return 0;
        }
    }

    // Nametables (0x2000-0x3EFF)
    else if (address < 0x3F00)
    {
        address &= 0x0FFF;
        uint8_t table = (address >> 10) & 0x03;
        uint16_t offset = address & 0x03FF;

        // Mapear tabela de acordo com m_ntMapping
        uint8_t ntSource = (m_ntMapping >> (table * 2)) & 0x03;

        switch (ntSource)
        {
        case 0: // Usar VRAM A (0x2000)
            return m_cartridge->vram[offset];

        case 1: // Usar VRAM B (0x2400)
            return m_cartridge->vram[0x400 + offset];

        case 2: // Usar EXRAM
            if (m_extendedRamMode < 2)
            {
                // Modos 0-1: Uso como NT
                return m_exram[offset];
            }
            else
            {
                // Modos 2-3: Uso como RAM
                return 0xFF; // Leitura não disponível
            }

        case 3: // Fill mode
            if (offset & 0x03FF < 0x03C0)
            {
                // Pattern
                return m_fillModeTile;
            }
            else
            {
                // Attributo
                return m_fillModeColor;
            }
        }
    }

    // Paletas (0x3F00-0x3FFF)
    else if (address < 0x4000)
    {
        // Implementado pelo PPU
        return 0;
    }

    LOG_WARNING("Mapper5: Leitura de PPU não mapeada em $%04X", address);
    return 0;
}

/**
 * @brief Escreve um byte na memória de vídeo (PPU)
 * @param address Endereço a ser escrito (0x0000-0x3FFF)
 * @param data Byte a ser escrito
 */
void Mapper5::ppuWrite(uint16_t address, uint8_t data)
{
    // CHR-ROM/RAM (0x0000-0x1FFF)
    if (address < 0x2000)
    {
        // Apenas escrever se for CHR-RAM
        if (m_usesChrRam)
        {
            bool isSpriteAddress = false;
            uint32_t chrOffset = 0;

            // Determinar se é acesso de sprite ou background
            if (m_cartridge->ppu_sprite_evaluation)
            {
                isSpriteAddress = true;
            }

            // Modo CHR e seleção de banco
            uint32_t bankSize = 0;
            uint8_t bankIndex = 0;

            switch (m_chrMode)
            {
            case 0: // Modo 0: Um banco de 8KB
                bankSize = 8 * 1024;
                bankIndex = 0;
                chrOffset = m_chrOffsets[0] + (address & 0x1FFF);
                break;

            case 1: // Modo 1: Dois bancos de 4KB
                bankSize = 4 * 1024;
                bankIndex = (address >> 12) & 0x01;
                chrOffset = m_chrOffsets[bankIndex] + (address & 0x0FFF);
                break;

            case 2: // Modo 2: Quatro bancos de 2KB
                bankSize = 2 * 1024;
                bankIndex = (address >> 11) & 0x03;
                chrOffset = m_chrOffsets[bankIndex] + (address & 0x07FF);
                break;

            case 3: // Modo 3: Oito bancos de 1KB
            default:
                bankSize = 1 * 1024;
                bankIndex = (address >> 10) & 0x07;

                // Se for acesso de sprite, usar bancos 8-11 (específicos para sprites)
                if (isSpriteAddress && (address >= 0x1000))
                {
                    chrOffset = m_chrOffsets[bankIndex + 8] + (address & 0x03FF);
                }
                else
                {
                    chrOffset = m_chrOffsets[bankIndex] + (address & 0x03FF);
                }
                break;
            }

            // Escrever na CHR-RAM
            if (chrOffset < m_cartridge->chr_ram_size)
            {
                m_cartridge->chr_ram[chrOffset] = data;
            }
        }
    }

    // Nametables (0x2000-0x3EFF)
    else if (address < 0x3F00)
    {
        address &= 0x0FFF;
        uint8_t table = (address >> 10) & 0x03;
        uint16_t offset = address & 0x03FF;

        // Mapear tabela de acordo com m_ntMapping
        uint8_t ntSource = (m_ntMapping >> (table * 2)) & 0x03;

        switch (ntSource)
        {
        case 0: // Usar VRAM A (0x2000)
            m_cartridge->vram[offset] = data;
            break;

        case 1: // Usar VRAM B (0x2400)
            m_cartridge->vram[0x400 + offset] = data;
            break;

        case 2: // Usar EXRAM
            if (m_extendedRamMode == 0)
            {
                // Modo 0: Uso como NT
                m_exram[offset] = data;
            }
            else if (m_extendedRamMode == 1)
            {
                // Modo 1: Uso como NT com atributos
                m_exram[offset] = data;
            }
            else
            {
                // Modos 2-3: Nenhuma escrita
            }
            break;
        }
    }

    // Paletas (0x3F00-0x3FFF)
    else if (address < 0x4000)
    {
        // Implementado pelo PPU
    }
}

/**
 * @brief Verifica se há uma interrupção pendente
 * @return true se há IRQ pendente, false caso contrário
 */
bool Mapper5::irqPending()
{
    return (m_irqStatus & 0x80) && m_irqEnabled;
}

/**
 * @brief Limpa o estado da interrupção
 */
void Mapper5::clearIrq()
{
    m_irqStatus &= ~0x80;
}

/**
 * @brief Salva o estado do mapper
 * @param state Estado do emulador
 * @return true se bem-sucedido, false caso contrário
 */
bool Mapper5::saveState(State &state)
{
    state.writeBlock("MMC5_EXRAM", m_exram, sizeof(m_exram));
    state.write("MMC5_PRG_MODE", m_prgMode);
    state.write("MMC5_CHR_MODE", m_chrMode);
    state.write("MMC5_PRG_RAM_PROTECT1", m_prgRamProtect1);
    state.write("MMC5_PRG_RAM_PROTECT2", m_prgRamProtect2);
    state.write("MMC5_EXTENDED_RAM_MODE", m_extendedRamMode);
    state.write("MMC5_NT_MAPPING", m_ntMapping);
    state.write("MMC5_FILL_MODE_TILE", m_fillModeTile);
    state.write("MMC5_FILL_MODE_COLOR", m_fillModeColor);

    state.writeBlock("MMC5_PRG_BANK_REG", m_prgBankReg, sizeof(m_prgBankReg));
    state.writeBlock("MMC5_CHR_BANK_REG", m_chrBankReg, sizeof(m_chrBankReg));
    state.writeBlock("MMC5_PRG_OFFSETS", m_prgOffsets, sizeof(m_prgOffsets));
    state.writeBlock("MMC5_CHR_OFFSETS", m_chrOffsets, sizeof(m_chrOffsets));

    state.write("MMC5_MULTIPLICAND", m_multiplicand);
    state.write("MMC5_MULTIPLIER", m_multiplier);
    state.write("MMC5_IRQ_SCANLINE_CMP", m_irqScanlineCmp);
    state.write("MMC5_IRQ_STATUS", m_irqStatus);
    state.write("MMC5_IRQ_ENABLED", m_irqEnabled);
    state.write("MMC5_SPLIT_MODE_CTRL", m_splitModeCtrl);
    state.write("MMC5_SPLIT_MODE_TILE", m_splitModeTile);
    state.write("MMC5_SPLIT_MODE_SCROLL", m_splitModeScroll);

    state.write("MMC5_AUDIO_CTRL", m_audioCtrl);
    state.write("MMC5_PULSE_CTRL", m_pulseCtrl);
    state.write("MMC5_PULSE_SWEEP", m_pulseSweep);
    state.write("MMC5_PULSE_TIMER", m_pulseTimer);
    state.write("MMC5_PULSE_TIMER_HIGH", m_pulseTimerHigh);
    state.write("MMC5_PULSE_TIMER_VALUE", m_pulseTimerValue);
    state.write("MMC5_PULSE_LENGTH", m_pulseLength);
    state.write("MMC5_PULSE_SEQ", m_pulseSeq);
    state.write("MMC5_PULSE_VOL", m_pulseVol);

    state.write("MMC5_CURRENT_SCANLINE", m_currentScanline);
    state.write("MMC5_IN_FRAME", m_inFrame);

    return true;
}

/**
 * @brief Carrega o estado do mapper
 * @param state Estado do emulador
 * @return true se bem-sucedido, false caso contrário
 */
bool Mapper5::loadState(State &state)
{
    state.readBlock("MMC5_EXRAM", m_exram, sizeof(m_exram));
    state.read("MMC5_PRG_MODE", m_prgMode);
    state.read("MMC5_CHR_MODE", m_chrMode);
    state.read("MMC5_PRG_RAM_PROTECT1", m_prgRamProtect1);
    state.read("MMC5_PRG_RAM_PROTECT2", m_prgRamProtect2);
    state.read("MMC5_EXTENDED_RAM_MODE", m_extendedRamMode);
    state.read("MMC5_NT_MAPPING", m_ntMapping);
    state.read("MMC5_FILL_MODE_TILE", m_fillModeTile);
    state.read("MMC5_FILL_MODE_COLOR", m_fillModeColor);

    state.readBlock("MMC5_PRG_BANK_REG", m_prgBankReg, sizeof(m_prgBankReg));
    state.readBlock("MMC5_CHR_BANK_REG", m_chrBankReg, sizeof(m_chrBankReg));
    state.readBlock("MMC5_PRG_OFFSETS", m_prgOffsets, sizeof(m_prgOffsets));
    state.readBlock("MMC5_CHR_OFFSETS", m_chrOffsets, sizeof(m_chrOffsets));

    state.read("MMC5_MULTIPLICAND", m_multiplicand);
    state.read("MMC5_MULTIPLIER", m_multiplier);
    state.read("MMC5_IRQ_SCANLINE_CMP", m_irqScanlineCmp);
    state.read("MMC5_IRQ_STATUS", m_irqStatus);
    state.read("MMC5_IRQ_ENABLED", m_irqEnabled);
    state.read("MMC5_SPLIT_MODE_CTRL", m_splitModeCtrl);
    state.read("MMC5_SPLIT_MODE_TILE", m_splitModeTile);
    state.read("MMC5_SPLIT_MODE_SCROLL", m_splitModeScroll);

    state.read("MMC5_AUDIO_CTRL", m_audioCtrl);
    state.read("MMC5_PULSE_CTRL", m_pulseCtrl);
    state.read("MMC5_PULSE_SWEEP", m_pulseSweep);
    state.read("MMC5_PULSE_TIMER", m_pulseTimer);
    state.read("MMC5_PULSE_TIMER_HIGH", m_pulseTimerHigh);
    state.read("MMC5_PULSE_TIMER_VALUE", m_pulseTimerValue);
    state.read("MMC5_PULSE_LENGTH", m_pulseLength);
    state.read("MMC5_PULSE_SEQ", m_pulseSeq);
    state.read("MMC5_PULSE_VOL", m_pulseVol);

    state.read("MMC5_CURRENT_SCANLINE", m_currentScanline);
    state.read("MMC5_IN_FRAME", m_inFrame);

    return true;
}

/**
 * @brief Notifica o mapper sobre uma nova scanline para IRQ
 */
void Mapper5::scanline()
{
    signalScanline();
}

/**
 * @brief Retorna o estado do sinal de IRQ
 * @return true se IRQ está ativo, false caso contrário
 */
bool Mapper5::irqState()
{
    return irqPending();
}

/**
 * @brief Limpa o sinal de IRQ
 */
void Mapper5::irqClear()
{
    clearIrq();
}

/**
 * @brief Lê um byte do espaço de endereçamento de nametable/CIRAM
 * @param address Endereço (0x2000-0x3EFF)
 * @return Byte lido
 */
uint8_t Mapper5::ntRead(uint16_t address)
{
    address &= 0x0FFF;
    uint8_t table = (address >> 10) & 0x03;
    uint16_t offset = address & 0x03FF;

    // Mapear tabela de acordo com m_ntMapping
    uint8_t ntSource = (m_ntMapping >> (table * 2)) & 0x03;

    switch (ntSource)
    {
    case 0: // Usar VRAM A (0x2000)
        return m_cartridge->vram[offset];

    case 1: // Usar VRAM B (0x2400)
        return m_cartridge->vram[0x400 + offset];

    case 2: // Usar EXRAM
        if (m_extendedRamMode < 2)
        {
            // Modos 0-1: Uso como NT
            return m_exram[offset];
        }
        else
        {
            // Modos 2-3: Uso como RAM
            return 0xFF; // Leitura não disponível
        }

    case 3: // Fill mode
        if (offset < 0x03C0)
        {
            // Pattern
            return m_fillModeTile;
        }
        else
        {
            // Attributo
            return m_fillModeColor;
        }
    }

    // Não deveria chegar aqui
    return 0;
}

/**
 * @brief Escreve um byte no espaço de endereçamento de nametable/CIRAM
 * @param address Endereço (0x2000-0x3EFF)
 * @param data Byte a ser escrito
 */
void Mapper5::ntWrite(uint16_t address, uint8_t data)
{
    address &= 0x0FFF;
    uint8_t table = (address >> 10) & 0x03;
    uint16_t offset = address & 0x03FF;

    // Mapear tabela de acordo com m_ntMapping
    uint8_t ntSource = (m_ntMapping >> (table * 2)) & 0x03;

    switch (ntSource)
    {
    case 0: // Usar VRAM A (0x2000)
        m_cartridge->vram[offset] = data;
        break;

    case 1: // Usar VRAM B (0x2400)
        m_cartridge->vram[0x400 + offset] = data;
        break;

    case 2: // Usar EXRAM
        if (m_extendedRamMode == 0)
        {
            // Modo 0: Uso como NT com escrita
            m_exram[offset] = data;
        }
        else if (m_extendedRamMode == 1)
        {
            // Modo 1: Uso como NT com escrita
            m_exram[offset] = data;
        }
        // Modos 2-3: Nenhuma escrita para NT
        break;

    case 3: // Fill mode - sem escrita
        break;
    }
}
