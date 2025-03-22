/**
 * @file mapper20.cpp
 * @brief Implementação do Mapper 20 (FDS - Famicom Disk System) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include "mapper20.hpp"
#include "../../../../core/logger.hpp"
#include <cmath>

using namespace MegaEmu::Platforms::NES;

Mapper20::Mapper20(Cartridge *cartridge) : Mapper(cartridge)
{
    if (!cartridge)
    {
        LOGE("Mapper20: Cartucho inválido!");
        return;
    }

    m_cartridge = cartridge;

    // Inicializar RAM
    m_ram.resize(RAM_SIZE, 0);
    m_expansionRam.resize(EXPANSION_RAM_SIZE, 0);

    // Inicializar registradores de status/controle
    m_irqCounter = 0;
    m_irqLatch = 0;
    m_irqEnabled = false;
    m_irqPending = false;
    m_diskInserted = false;
    m_currentDisk = 0;
    m_currentSide = 0;

    // Inicializar registradores de controle de disco
    m_diskStatus = 0;
    m_diskIRQEnabled = false;
    m_diskMotorOn = false;
    m_diskWriteMode = false;
    m_diskPosition = 0;
    m_diskReadWriteReg = 0;

    // Inicializar registradores de áudio
    m_audioEnabled = false;
    for (int i = 0; i < 64; i++)
    {
        m_waveTable[i] = 0;
        m_modulationTable[i] = 0;
    }
    m_waveWriteEnable = 0;
    m_masterVolume = 0;
    m_frequency = 0;
    m_volume = 0;
    m_waveAccumulator = 0;
    m_modulationEnabled = false;
    m_modulationFreq = 0;
    m_modulationDepth = 0;
    m_modulationAccum = 0;

    LOGI("Mapper20: Inicializado. FDS (Famicom Disk System)");
}

Mapper20::~Mapper20()
{
    LOGI("Mapper20: Destruído");
}

void Mapper20::reset()
{
    // Limpar RAM
    std::fill(m_ram.begin(), m_ram.end(), 0);
    std::fill(m_expansionRam.begin(), m_expansionRam.end(), 0);

    // Reiniciar registradores de status/controle
    m_irqCounter = 0;
    m_irqLatch = 0;
    m_irqEnabled = false;
    m_irqPending = false;

    // Reiniciar registradores de controle de disco
    m_diskStatus = 0;
    m_diskIRQEnabled = false;
    m_diskMotorOn = false;
    m_diskWriteMode = false;
    m_diskPosition = 0;
    m_diskReadWriteReg = 0;

    // Reiniciar registradores de áudio
    m_audioEnabled = false;
    m_waveWriteEnable = 0;
    m_masterVolume = 0;
    m_frequency = 0;
    m_volume = 0;
    m_waveAccumulator = 0;
    m_modulationEnabled = false;
    m_modulationFreq = 0;
    m_modulationDepth = 0;
    m_modulationAccum = 0;

    // Não ejetar o disco durante o reset

    LOGI("Mapper20: Reset realizado");
}

uint8_t Mapper20::cpuRead(uint16_t address)
{
    // Registradores de I/O específicos do FDS (0x4020-0x40FF)
    if (address >= 0x4020 && address <= 0x40FF)
    {
        switch (address)
        {
        case 0x4030:
        { // Status do drive de disquete
            uint8_t status = 0;
            status |= m_irqPending ? 0x01 : 0x00;    // Bit 0: IRQ pendente
            status |= m_diskInserted ? 0x00 : 0x02;  // Bit 1: Disquete não inserido (invertido)
            status |= m_diskWriteMode ? 0x00 : 0x04; // Bit 2: Proteção contra escrita (invertido)
            status |= m_diskMotorOn ? 0x08 : 0x00;   // Bit 3: Motor ligado
            status |= m_diskWriteMode ? 0x40 : 0x00; // Bit 6: Modo de escrita

            // Limpar flag de IRQ
            m_irqPending = false;

            return status;
        }
        case 0x4031: // Registrador de leitura de dados
            if (m_diskInserted && m_diskMotorOn && !m_diskWriteMode)
            {
                return readDiskData();
            }
            return 0;
        case 0x4032:     // Status de controle do drive (não implementado totalmente)
            return 0x40; // Sempre pronto para transferência
        case 0x4033:     // Registrador de controle de bateria externa (não implementado)
            return 0x80; // Sempre tem energia
        }
    }

    // RAM de expansão (0x5000-0x5FFF)
    if (address >= 0x5000 && address <= 0x5FFF)
    {
        uint16_t offset = address - 0x5000;
        if (offset < m_expansionRam.size())
        {
            return m_expansionRam[offset];
        }
    }

    // RAM principal (0x6000-0xDFFF)
    if (address >= 0x6000 && address <= 0xDFFF)
    {
        uint16_t offset = address - 0x6000;
        if (offset < m_ram.size())
        {
            return m_ram[offset];
        }
    }

    // ROM de BIOS do FDS (0xE000-0xFFFF) - Não implementada, retorna 0
    // Em uma implementação completa, precisaríamos carregar e emular o BIOS do FDS
    if (address >= 0xE000 && address <= 0xFFFF)
    {
        LOGW("Mapper20: Tentativa de leitura do BIOS FDS: 0x%04X (não implementado)", address);
        return 0;
    }

    LOGW("Mapper20: Tentativa de leitura de CPU em endereço inválido: 0x%04X", address);
    return 0;
}

void Mapper20::cpuWrite(uint16_t address, uint8_t data)
{
    // Registradores de I/O específicos do FDS (0x4020-0x40FF)
    if (address >= 0x4020 && address <= 0x40FF)
    {
        switch (address)
        {
        case 0x4020: // IRQ Reload Low
            m_irqLatch = (m_irqLatch & 0xFF00) | data;
            LOGI("Mapper20: IRQ Latch Low definido para 0x%02X", data);
            break;
        case 0x4021: // IRQ Reload High
            m_irqLatch = (m_irqLatch & 0x00FF) | (data << 8);
            LOGI("Mapper20: IRQ Latch High definido para 0x%02X", data);
            break;
        case 0x4022: // IRQ Control
            m_irqEnabled = (data & 0x01) != 0;
            if (m_irqEnabled)
            {
                m_irqCounter = m_irqLatch;
            }
            m_irqPending = false;
            LOGI("Mapper20: IRQ %s", m_irqEnabled ? "habilitado" : "desabilitado");
            break;
        case 0x4023: // Controle do modo de acesso ao disco
            m_diskWriteMode = (data & 0x02) != 0;
            m_diskMotorOn = (data & 0x01) != 0;
            LOGI("Mapper20: Motor do disco %s, Modo de %s",
                 m_diskMotorOn ? "ligado" : "desligado",
                 m_diskWriteMode ? "escrita" : "leitura");
            break;
        case 0x4024: // Registrador de escrita de dados
            if (m_diskInserted && m_diskMotorOn && m_diskWriteMode)
            {
                writeDiskData(data);
            }
            break;
        case 0x4025: // Controle de transferência de disco
            m_diskIRQEnabled = (data & 0x80) != 0;
            LOGI("Mapper20: IRQ do disco %s", m_diskIRQEnabled ? "habilitado" : "desabilitado");
            break;
        // Registradores de áudio
        case 0x4040 ... 0x407F: // Escrita na tabela de ondas
            if (m_waveWriteEnable)
            {
                m_waveTable[address - 0x4040] = data & 0x3F;
            }
            break;
        case 0x4080: // Volume do canal de onda
            m_volume = data & 0x3F;
            m_waveWriteEnable = (data & 0x80) != 0;
            LOGI("Mapper20: Volume definido para %d, Escrita na tabela %s",
                 m_volume, m_waveWriteEnable ? "habilitada" : "desabilitada");
            break;
        case 0x4082: // Frequência do canal de onda (baixo)
            m_frequency = (m_frequency & 0xFF00) | data;
            LOGI("Mapper20: Frequência (baixo) definida para 0x%02X", data);
            break;
        case 0x4083: // Frequência do canal de onda (alto)
            m_frequency = (m_frequency & 0x00FF) | ((data & 0x0F) << 8);
            m_audioEnabled = (data & 0x80) == 0;
            LOGI("Mapper20: Frequência (alto) definida para 0x%01X, Áudio %s",
                 data & 0x0F, m_audioEnabled ? "habilitado" : "desabilitado");
            break;
        case 0x4084: // Registrador de controle da modulação
            m_modulationDepth = data & 0x3F;
            m_modulationEnabled = (data & 0x80) == 0;
            LOGI("Mapper20: Modulação %s, Profundidade %d",
                 m_modulationEnabled ? "habilitada" : "desabilitada", m_modulationDepth);
            break;
        case 0x4085: // Contador da modulação (não usado diretamente)
            LOGI("Mapper20: Contador de modulação definido para 0x%02X", data);
            break;
        case 0x4086: // Frequência da modulação (baixo)
            m_modulationFreq = (m_modulationFreq & 0xFF00) | data;
            LOGI("Mapper20: Frequência de modulação (baixo) definida para 0x%02X", data);
            break;
        case 0x4087: // Frequência da modulação (alto)
            m_modulationFreq = (m_modulationFreq & 0x00FF) | ((data & 0x0F) << 8);
            LOGI("Mapper20: Frequência de modulação (alto) definida para 0x%01X", data & 0x0F);
            break;
        case 0x4088: // Volume principal
            m_masterVolume = data;
            LOGI("Mapper20: Volume principal definido para 0x%02X", data);
            break;
        case 0x4089: // Registrador de envelope do FDS (não implementado totalmente)
            LOGI("Mapper20: Envelope definido para 0x%02X", data);
            break;
        case 0x408A: // Tabela de modulação write (não implementado totalmente)
            LOGI("Mapper20: Escrita na tabela de modulação: 0x%02X", data);
            break;
        }
        return;
    }

    // RAM de expansão (0x5000-0x5FFF)
    if (address >= 0x5000 && address <= 0x5FFF)
    {
        uint16_t offset = address - 0x5000;
        if (offset < m_expansionRam.size())
        {
            m_expansionRam[offset] = data;
        }
        return;
    }

    // RAM principal (0x6000-0xDFFF)
    if (address >= 0x6000 && address <= 0xDFFF)
    {
        uint16_t offset = address - 0x6000;
        if (offset < m_ram.size())
        {
            m_ram[offset] = data;
        }
        return;
    }

    // ROM de BIOS do FDS (0xE000-0xFFFF) - Não permite escrita
    if (address >= 0xE000 && address <= 0xFFFF)
    {
        LOGW("Mapper20: Tentativa de escrita no BIOS FDS: 0x%04X = 0x%02X (ignorado)", address, data);
        return;
    }

    LOGW("Mapper20: Tentativa de escrita de CPU em endereço inválido: 0x%04X = 0x%02X", address, data);
}

uint8_t Mapper20::ppuRead(uint16_t address)
{
    if (address > 0x1FFF)
    {
        LOGW("Mapper20: Tentativa de leitura de PPU em endereço inválido: 0x%04X", address);
        return 0;
    }

    // O FDS usa CHR-RAM
    return m_cartridge->chrRamRead(address);
}

void Mapper20::ppuWrite(uint16_t address, uint8_t data)
{
    if (address > 0x1FFF)
    {
        LOGW("Mapper20: Tentativa de escrita de PPU em endereço inválido: 0x%04X", address);
        return;
    }

    // O FDS usa CHR-RAM
    m_cartridge->chrRamWrite(address, data);
}

void Mapper20::scanline()
{
    // Atualizar contador de IRQ se habilitado
    if (m_irqEnabled)
    {
        if (m_irqCounter == 0)
        {
            // Gerar IRQ
            m_irqPending = true;
            // Recarregar contador
            m_irqCounter = m_irqLatch;
            LOGI("Mapper20: IRQ gerado");
        }
        else
        {
            // Decrementar contador
            m_irqCounter--;
        }
    }
}

bool Mapper20::irqState()
{
    return m_irqPending;
}

void Mapper20::irqClear()
{
    m_irqPending = false;
}

void Mapper20::updateIrqCounter()
{
    // Só é usada para implementação de IRQ baseado em ciclos de CPU (não implementado)
}

void Mapper20::updateAudio(int cycles)
{
    // Atualizar acumuladores de onda e modulação
    if (m_audioEnabled)
    {
        // Canal principal
        m_waveAccumulator += cycles * m_frequency;

        // Canal de modulação
        if (m_modulationEnabled)
        {
            m_modulationAccum += cycles * m_modulationFreq;
        }
    }
}

uint8_t Mapper20::readDiskData()
{
    if (!m_diskInserted || !m_diskMotorOn || m_diskWriteMode || m_disks.empty())
    {
        return 0;
    }

    // Verificar se o disco e lado são válidos
    if (m_currentDisk >= m_disks.size() || m_currentSide >= m_disks[m_currentDisk].sides)
    {
        LOGW("Mapper20: Tentativa de leitura em disco/lado inválido: %d/%d", m_currentDisk, m_currentSide);
        return 0;
    }

    // Calcular posição no vetor de dados
    size_t diskOffset = DISK_SIDE_CAPACITY * m_currentSide;
    size_t position = diskOffset + m_diskPosition;

    // Verificar se a posição é válida
    if (position < m_disks[m_currentDisk].data.size())
    {
        uint8_t data = m_disks[m_currentDisk].data[position];

        // Avançar cabeça de leitura
        advanceDiskHead();

        return data;
    }

    LOGW("Mapper20: Tentativa de leitura fora dos limites do disco: %zu", position);
    return 0;
}

void Mapper20::writeDiskData(uint8_t data)
{
    if (!m_diskInserted || !m_diskMotorOn || !m_diskWriteMode || m_disks.empty())
    {
        return;
    }

    // Verificar se o disco e lado são válidos
    if (m_currentDisk >= m_disks.size() || m_currentSide >= m_disks[m_currentDisk].sides)
    {
        LOGW("Mapper20: Tentativa de escrita em disco/lado inválido: %d/%d", m_currentDisk, m_currentSide);
        return;
    }

    // Calcular posição no vetor de dados
    size_t diskOffset = DISK_SIDE_CAPACITY * m_currentSide;
    size_t position = diskOffset + m_diskPosition;

    // Verificar se a posição é válida
    if (position < m_disks[m_currentDisk].data.size())
    {
        m_disks[m_currentDisk].data[position] = data;

        // Avançar cabeça de leitura
        advanceDiskHead();
    }
    else
    {
        LOGW("Mapper20: Tentativa de escrita fora dos limites do disco: %zu", position);
    }
}

void Mapper20::advanceDiskHead()
{
    // Avançar a posição do disco
    m_diskPosition++;

    // Verificar se chegou ao fim do lado
    if (m_diskPosition >= DISK_SIDE_CAPACITY)
    {
        LOGI("Mapper20: Fim do lado do disco alcançado");
        m_diskPosition = 0;

        // Gerar IRQ de fim de disco se habilitado
        if (m_diskIRQEnabled)
        {
            m_irqPending = true;
            LOGI("Mapper20: IRQ de disco gerado");
        }
    }
}

void Mapper20::generateAudio(float *buffer, int sampleCount)
{
    // Esta função é chamada pelo APU para gerar amostras de áudio do FDS
    for (int i = 0; i < sampleCount; i++)
    {
        float output = 0.0f;

        if (m_audioEnabled)
        {
            // Calcular índice na tabela de onda
            uint8_t waveIdx = (m_waveAccumulator >> 16) & 0x3F;

            // Aplicar modulação se habilitada
            if (m_modulationEnabled)
            {
                uint8_t modIdx = (m_modulationAccum >> 16) & 0x3F;
                int8_t modValue = m_modulationTable[modIdx] - 32; // Converter para -32 a +31

                // Ajustar a frequência baseado na modulação
                if (modValue != 0)
                {
                    int modAmount = ((int)m_frequency * modValue * m_modulationDepth) >> 8;
                    // Evitar frequência negativa
                    if ((int)m_frequency + modAmount > 0)
                    {
                        waveIdx = ((m_waveAccumulator >> 16) + modAmount) & 0x3F;
                    }
                }
            }

            // Obter amostra da tabela de onda (0-63)
            uint8_t waveData = m_waveTable[waveIdx];

            // Aplicar volume (0-63)
            float ampWave = (float)waveData * (float)m_volume / (63.0f * 63.0f);

            // Aplicar volume principal (mais 4 bits de precisão para atenuação)
            float ampMaster = ampWave * ((float)(m_masterVolume & 0x03) / 3.0f);
            if ((m_masterVolume & 0x80) != 0)
            {
                // Atenuação, usar 4 bits superiores para precisão adicional
                float attenuation = 1.0f - ((float)((m_masterVolume >> 2) & 0x1F) / 31.0f);
                ampMaster *= attenuation;
            }

            output = ampMaster * 0.5f; // Ajuste para misturar com APU
        }

        // Adicionar à saída
        buffer[i] += output;

        // Atualizar estado de áudio
        updateAudio(1);
    }
}

bool Mapper20::loadDiskImage(const uint8_t *diskData, size_t size)
{
    if (!diskData || size == 0)
    {
        LOGE("Mapper20: Dados de disco inválidos");
        return false;
    }

    // Limpar discos existentes
    m_disks.clear();

    // Tamanho mínimo para um lado do disco
    if (size < 65500)
    {
        LOGE("Mapper20: Tamanho de arquivo de disco muito pequeno: %zu bytes", size);
        return false;
    }

    // Determinar número de discos e lados
    Disk newDisk;

    // Assumir formato padrão: cabeçalho (16 bytes) + dados (65500 bytes por lado)
    uint8_t numSides = 1;
    if (size > 16 + 65500)
    {
        // Assumir 2 lados se houver dados suficientes
        numSides = 2;
    }

    // Copiar dados do disco, pulando cabeçalho se existir
    size_t dataOffset = 0;
    if (size >= 16 + 65500)
    {
        // Pular cabeçalho FDS de 16 bytes
        dataOffset = 16;
    }

    // Determinar tamanho total dos dados
    size_t dataSize = std::min(size - dataOffset, (size_t)(numSides * 65500));

    // Criar novo disco
    newDisk.data.resize(dataSize);
    std::copy(diskData + dataOffset, diskData + dataOffset + dataSize, newDisk.data.begin());
    newDisk.sides = numSides;

    m_disks.push_back(newDisk);
    LOGI("Mapper20: Carregado disco com %d lados, %zu bytes de dados", numSides, dataSize);

    return true;
}

void Mapper20::ejectDisk()
{
    m_diskInserted = false;
    m_diskMotorOn = false;
    m_diskPosition = 0;
    LOGI("Mapper20: Disco ejetado");
}

bool Mapper20::insertDisk(uint8_t diskNumber, uint8_t side)
{
    if (diskNumber >= m_disks.size())
    {
        LOGE("Mapper20: Número de disco inválido: %d", diskNumber);
        return false;
    }

    if (side >= m_disks[diskNumber].sides)
    {
        LOGE("Mapper20: Lado do disco inválido: %d", side);
        return false;
    }

    m_currentDisk = diskNumber;
    m_currentSide = side;
    m_diskInserted = true;
    m_diskPosition = 0;

    LOGI("Mapper20: Disco %d, lado %d inserido", diskNumber, side);
    return true;
}

uint8_t Mapper20::getDiskCount() const
{
    return static_cast<uint8_t>(m_disks.size());
}
