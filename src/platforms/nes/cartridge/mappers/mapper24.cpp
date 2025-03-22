/**
 * @file mapper24.cpp
 * @brief Implementação do Mapper 24 (VRC6) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include "mapper24.hpp"
#include "../../../../core/logger.hpp"
#include <cmath>

using namespace MegaEmu::Platforms::NES;

Mapper24::Mapper24(Cartridge *cartridge) : Mapper(cartridge)
{
    if (!cartridge)
    {
        LOGE("Mapper24: Cartucho inválido!");
        return;
    }

    m_cartridge = cartridge;
    m_prgRomSize = cartridge->getPrgRomSize();
    m_chrRomSize = cartridge->getChrRomSize();
    m_usesChrRam = (m_chrRomSize == 0);
    m_hasPrgRam = cartridge->hasPrgRam();
    m_hasBattery = cartridge->hasBattery();
    m_mirrorMode = cartridge->getMirrorMode();

    // Inicializar bancos
    m_prgBank[0] = 0;
    m_prgBank[1] = 1;
    for (int i = 0; i < 8; i++)
    {
        m_chrBank[i] = i;
    }

    // Inicializar registradores de controle
    m_prgRamEnabled = false;
    m_prgRamWriteProtect = true;

    // Inicializar registradores de IRQ
    m_irqEnabled = false;
    m_irqPending = false;
    m_irqLatch = 0;
    m_irqCounter = 0;
    m_irqMode = false;
    m_irqPrescaler = 0;

    // Inicializar registradores de áudio
    for (int i = 0; i < 2; i++)
    {
        m_pulse[i].enabled = false;
        m_pulse[i].volume = 0;
        m_pulse[i].duty = 0;
        m_pulse[i].frequency = 0;
        m_pulse[i].timer = 0;
        m_pulse[i].sequencer = 0;
    }

    m_saw.enabled = false;
    m_saw.accumulatorRate = 0;
    m_saw.frequency = 0;
    m_saw.timer = 0;
    m_saw.accumulator = 0;

    LOGI("Mapper24: Inicializado. PRG-ROM: %dKB, CHR-%s: %dKB, PRG-RAM: %s",
         m_prgRomSize / 1024,
         m_usesChrRam ? "RAM" : "ROM",
         m_usesChrRam ? 8 : (m_chrRomSize / 1024),
         m_hasPrgRam ? (m_hasBattery ? "8KB (com bateria)" : "8KB") : "Não");
}

Mapper24::~Mapper24()
{
    LOGI("Mapper24: Destruído");
}

void Mapper24::reset()
{
    // Reiniciar bancos
    m_prgBank[0] = 0;
    m_prgBank[1] = 1;
    for (int i = 0; i < 8; i++)
    {
        m_chrBank[i] = i;
    }

    // Reiniciar registradores de controle
    m_prgRamEnabled = false;
    m_prgRamWriteProtect = true;
    m_mirrorMode = m_cartridge->getMirrorMode();

    // Reiniciar registradores de IRQ
    m_irqEnabled = false;
    m_irqPending = false;
    m_irqLatch = 0;
    m_irqCounter = 0;
    m_irqMode = false;
    m_irqPrescaler = 0;

    // Reiniciar registradores de áudio
    for (int i = 0; i < 2; i++)
    {
        m_pulse[i].enabled = false;
        m_pulse[i].volume = 0;
        m_pulse[i].duty = 0;
        m_pulse[i].frequency = 0;
        m_pulse[i].timer = 0;
        m_pulse[i].sequencer = 0;
    }

    m_saw.enabled = false;
    m_saw.accumulatorRate = 0;
    m_saw.frequency = 0;
    m_saw.timer = 0;
    m_saw.accumulator = 0;

    LOGI("Mapper24: Reset realizado");
}

uint8_t Mapper24::cpuRead(uint16_t address)
{
    if (address < 0x6000 || address > 0xFFFF)
    {
        LOGW("Mapper24: Tentativa de leitura de CPU em endereço inválido: 0x%04X", address);
        return 0;
    }

    // PRG-RAM em $6000-$7FFF
    if (address >= 0x6000 && address < 0x8000)
    {
        if (m_hasPrgRam && m_prgRamEnabled)
        {
            return m_cartridge->prgRamRead(address - 0x6000);
        }
        else
        {
            LOGW("Mapper24: Tentativa de leitura de PRG-RAM desabilitada: 0x%04X", address);
            return 0;
        }
    }

    // PRG-ROM em $8000-$FFFF
    if (address >= 0x8000)
    {
        uint32_t bankIndex;
        uint32_t bankOffset;
        uint32_t romAddress;

        if (address < 0xC000)
        {
            // Banco selecionável 0 ($8000-$BFFF)
            bankIndex = m_prgBank[0];
            bankOffset = address - 0x8000;
        }
        else
        {
            // Banco selecionável 1 ($C000-$FFFF)
            bankIndex = m_prgBank[1];
            bankOffset = address - 0xC000;
        }

        romAddress = bankIndex * PRG_BANK_SIZE + bankOffset;

        if (romAddress < m_prgRomSize)
        {
            return m_cartridge->prgRomRead(romAddress);
        }
        else
        {
            LOGW("Mapper24: Tentativa de leitura fora dos limites da PRG-ROM: 0x%04X", romAddress);
            return 0;
        }
    }

    return 0;
}

void Mapper24::cpuWrite(uint16_t address, uint8_t data)
{
    if (address < 0x6000 || address > 0xFFFF)
    {
        LOGW("Mapper24: Tentativa de escrita de CPU em endereço inválido: 0x%04X", address);
        return;
    }

    // PRG-RAM em $6000-$7FFF
    if (address >= 0x6000 && address < 0x8000)
    {
        if (m_hasPrgRam && m_prgRamEnabled && !m_prgRamWriteProtect)
        {
            m_cartridge->prgRamWrite(address - 0x6000, data);
        }
        else
        {
            LOGW("Mapper24: Tentativa de escrita em PRG-RAM protegida: 0x%04X", address);
        }
        return;
    }

    // O VRC6 tem um mapeamento de registradores especial
    uint16_t regAddr = address & 0xF003;

    // Registradores de banco PRG
    if (regAddr == 0x8000)
    {
        // Banco PRG 0 ($8000-$BFFF)
        uint8_t oldBank = m_prgBank[0];
        m_prgBank[0] = data & 0x0F;
        LOGI("Mapper24: Banco PRG 0 alterado: %d -> %d", oldBank, m_prgBank[0]);
    }
    else if (regAddr == 0xC000)
    {
        // Banco PRG 1 ($C000-$FFFF)
        uint8_t oldBank = m_prgBank[1];
        m_prgBank[1] = data & 0x1F;
        LOGI("Mapper24: Banco PRG 1 alterado: %d -> %d", oldBank, m_prgBank[1]);
    }
    // Registradores de controle
    else if (regAddr == 0x9000)
    {
        // Controle de espelhamento
        MirrorMode oldMode = m_mirrorMode;
        switch (data & 0x03)
        {
        case 0:
            m_mirrorMode = MIRROR_VERTICAL;
            break;
        case 1:
            m_mirrorMode = MIRROR_HORIZONTAL;
            break;
        case 2:
            m_mirrorMode = MIRROR_SINGLE_SCREEN_LOW;
            break;
        case 3:
            m_mirrorMode = MIRROR_SINGLE_SCREEN_HIGH;
            break;
        }

        m_cartridge->setMirrorMode(m_mirrorMode);

        LOGI("Mapper24: Modo de espelhamento alterado: %d -> %d", oldMode, m_mirrorMode);

        // Bits 4-5: Controle da PRG-RAM
        if (m_hasPrgRam)
        {
            m_prgRamEnabled = (data & 0x10) != 0;
            m_prgRamWriteProtect = (data & 0x20) != 0;
            LOGI("Mapper24: PRG-RAM %s, Proteção de escrita %s",
                 m_prgRamEnabled ? "ativada" : "desativada",
                 m_prgRamWriteProtect ? "ativada" : "desativada");
        }
    }
    // Registradores de banco CHR
    else if (regAddr >= 0xD000 && regAddr <= 0xE003)
    {
        // Bancos CHR (1KB cada)
        uint8_t bankIndex = ((regAddr - 0xD000) >> 8) * 2 + ((regAddr & 0x03) > 0 ? 1 : 0);

        if (bankIndex < 8)
        {
            uint8_t oldBank = m_chrBank[bankIndex];
            m_chrBank[bankIndex] = data;
            LOGI("Mapper24: Banco CHR %d alterado: %d -> %d", bankIndex, oldBank, m_chrBank[bankIndex]);
        }
    }
    // Registradores de IRQ
    else if (regAddr == 0xF000)
    {
        // IRQ latch
        m_irqLatch = data;
        LOGI("Mapper24: IRQ latch definido para %d", m_irqLatch);
    }
    else if (regAddr == 0xF001)
    {
        // IRQ control
        m_irqMode = (data & 0x04) != 0;
        m_irqEnabled = (data & 0x02) != 0;

        if (data & 0x01)
        {
            // Recarregar contador com valor do latch
            m_irqCounter = m_irqLatch;
        }
        else
        {
            // Desabilitar IRQ
            m_irqPending = false;
        }

        LOGI("Mapper24: IRQ control: Modo=%s, Habilitado=%s%s",
             m_irqMode ? "ciclos" : "scanlines",
             m_irqEnabled ? "sim" : "não",
             (data & 0x01) ? ", contador recarregado" : "");
    }
    else if (regAddr == 0xF002)
    {
        // IRQ acknowledge
        m_irqPending = false;
        LOGI("Mapper24: IRQ reconhecido");
    }
    // Registradores de áudio - Pulso 1
    else if (regAddr == 0x9000)
    {
        // Pulso 1: Volume/Duty
        m_pulse[0].volume = data & 0x0F;
        m_pulse[0].duty = (data >> 4) & 0x07;
        m_pulse[0].enabled = (data & 0x80) == 0;
        LOGI("Mapper24: Pulso 1 configurado - Volume: %d, Duty: %d, Habilitado: %s",
             m_pulse[0].volume, m_pulse[0].duty, m_pulse[0].enabled ? "sim" : "não");
    }
    else if (regAddr == 0x9001)
    {
        // Pulso 1: Frequência (baixo)
        m_pulse[0].frequency = (m_pulse[0].frequency & 0xFF00) | data;
        LOGI("Mapper24: Pulso 1 - Frequência (baixo): %d", data);
    }
    else if (regAddr == 0x9002)
    {
        // Pulso 1: Frequência (alto)
        m_pulse[0].frequency = (m_pulse[0].frequency & 0x00FF) | ((data & 0x0F) << 8);
        LOGI("Mapper24: Pulso 1 - Frequência (alto): %d", data & 0x0F);
    }
    // Registradores de áudio - Pulso 2
    else if (regAddr == 0xA000)
    {
        // Pulso 2: Volume/Duty
        m_pulse[1].volume = data & 0x0F;
        m_pulse[1].duty = (data >> 4) & 0x07;
        m_pulse[1].enabled = (data & 0x80) == 0;
        LOGI("Mapper24: Pulso 2 configurado - Volume: %d, Duty: %d, Habilitado: %s",
             m_pulse[1].volume, m_pulse[1].duty, m_pulse[1].enabled ? "sim" : "não");
    }
    else if (regAddr == 0xA001)
    {
        // Pulso 2: Frequência (baixo)
        m_pulse[1].frequency = (m_pulse[1].frequency & 0xFF00) | data;
        LOGI("Mapper24: Pulso 2 - Frequência (baixo): %d", data);
    }
    else if (regAddr == 0xA002)
    {
        // Pulso 2: Frequência (alto)
        m_pulse[1].frequency = (m_pulse[1].frequency & 0x00FF) | ((data & 0x0F) << 8);
        LOGI("Mapper24: Pulso 2 - Frequência (alto): %d", data & 0x0F);
    }
    // Registradores de áudio - Dente de serra
    else if (regAddr == 0xB000)
    {
        // Dente de serra: Taxa de acumulação
        m_saw.accumulatorRate = data & 0x3F;
        LOGI("Mapper24: Dente de serra - Taxa de acumulação: %d", m_saw.accumulatorRate);
    }
    else if (regAddr == 0xB001)
    {
        // Dente de serra: Frequência (baixo)
        m_saw.frequency = (m_saw.frequency & 0xFF00) | data;
        LOGI("Mapper24: Dente de serra - Frequência (baixo): %d", data);
    }
    else if (regAddr == 0xB002)
    {
        // Dente de serra: Frequência (alto) e habilitação
        m_saw.frequency = (m_saw.frequency & 0x00FF) | ((data & 0x0F) << 8);
        m_saw.enabled = (data & 0x80) == 0;
        LOGI("Mapper24: Dente de serra - Frequência (alto): %d, Habilitado: %s",
             data & 0x0F, m_saw.enabled ? "sim" : "não");
    }
}

uint8_t Mapper24::ppuRead(uint16_t address)
{
    if (address > 0x1FFF)
    {
        LOGW("Mapper24: Tentativa de leitura de PPU em endereço inválido: 0x%04X", address);
        return 0;
    }

    // Selecionar banco CHR
    uint8_t bankIndex = address / CHR_BANK_SIZE;
    uint32_t bankOffset = address % CHR_BANK_SIZE;
    uint32_t romAddress = m_chrBank[bankIndex] * CHR_BANK_SIZE + bankOffset;

    if (m_usesChrRam)
    {
        // Usar CHR-RAM
        return m_cartridge->chrRamRead(address);
    }
    else
    {
        // Usar CHR-ROM
        if (romAddress < m_chrRomSize)
        {
            return m_cartridge->chrRomRead(romAddress);
        }
        else
        {
            LOGW("Mapper24: Tentativa de leitura fora dos limites da CHR-ROM: 0x%04X", romAddress);
            return 0;
        }
    }
}

void Mapper24::ppuWrite(uint16_t address, uint8_t data)
{
    if (address > 0x1FFF)
    {
        LOGW("Mapper24: Tentativa de escrita de PPU em endereço inválido: 0x%04X", address);
        return;
    }

    if (m_usesChrRam)
    {
        // Escrever na CHR-RAM
        m_cartridge->chrRamWrite(address, data);
    }
    else
    {
        LOGW("Mapper24: Tentativa de escrita em CHR-ROM: 0x%04X", address);
    }
}

void Mapper24::scanline()
{
    if (!m_irqEnabled || m_irqMode)
    {
        // IRQ desabilitado ou no modo de ciclos
        return;
    }

    // Atualizar contador de IRQ no modo scanline
    if (m_irqCounter == 0)
    {
        // Recarregar contador e gerar IRQ
        m_irqCounter = m_irqLatch;
        m_irqPending = true;
        LOGI("Mapper24: IRQ gerado (scanline)");
    }
    else
    {
        // Decrementar contador
        m_irqCounter--;
    }
}

bool Mapper24::irqState()
{
    return m_irqPending;
}

void Mapper24::irqClear()
{
    m_irqPending = false;
}

void Mapper24::updateIrqCounter()
{
    if (!m_irqEnabled || !m_irqMode)
    {
        // IRQ desabilitado ou no modo de scanline
        return;
    }

    // Atualizar prescaler
    m_irqPrescaler++;
    if (m_irqPrescaler >= 114)
    { // Aproximadamente 114 ciclos por scanline
        m_irqPrescaler = 0;

        // Atualizar contador de IRQ no modo de ciclos
        if (m_irqCounter == 0)
        {
            // Recarregar contador e gerar IRQ
            m_irqCounter = m_irqLatch;
            m_irqPending = true;
            LOGI("Mapper24: IRQ gerado (ciclos)");
        }
        else
        {
            // Decrementar contador
            m_irqCounter--;
        }
    }
}

void Mapper24::updateAudio(int cycles)
{
    // Atualizar temporizadores de áudio
    for (int i = 0; i < 2; i++)
    {
        if (m_pulse[i].enabled)
        {
            m_pulse[i].timer -= cycles;
            if (m_pulse[i].timer <= 0)
            {
                // Reiniciar temporizador
                m_pulse[i].timer += (m_pulse[i].frequency + 1) * 16;

                // Avançar sequenciador
                m_pulse[i].sequencer = (m_pulse[i].sequencer + 1) & 0x0F;
            }
        }
    }

    if (m_saw.enabled)
    {
        m_saw.timer -= cycles;
        if (m_saw.timer <= 0)
        {
            // Reiniciar temporizador
            m_saw.timer += (m_saw.frequency + 1) * 14;

            // Atualizar acumulador
            m_saw.accumulator = (m_saw.accumulator + m_saw.accumulatorRate) & 0xFF;
            if (m_saw.accumulator & 0x20)
            {
                m_saw.accumulator = 0;
            }
        }
    }
}

void Mapper24::generateAudio(float *buffer, int sampleCount)
{
    // Esta função é chamada pelo APU para gerar amostras de áudio do VRC6
    for (int i = 0; i < sampleCount; i++)
    {
        float output = 0.0f;

        // Pulse 1
        if (m_pulse[0].enabled)
        {
            // Checar duty cycle
            bool pulseOut = ((m_pulse[0].sequencer >> m_pulse[0].duty) & 1) == 0;
            if (pulseOut)
            {
                output += m_pulse[0].volume / 15.0f;
            }
        }

        // Pulse 2
        if (m_pulse[1].enabled)
        {
            // Checar duty cycle
            bool pulseOut = ((m_pulse[1].sequencer >> m_pulse[1].duty) & 1) == 0;
            if (pulseOut)
            {
                output += m_pulse[1].volume / 15.0f;
            }
        }

        // Sawtooth
        if (m_saw.enabled)
        {
            output += (m_saw.accumulator >> 3) / 32.0f;
        }

        // Adicionar à saída (atenuado para misturar com APU principal)
        buffer[i] += output * 0.25f;

        // Simular a frequência de atualização do áudio
        updateAudio(1);
    }
}
