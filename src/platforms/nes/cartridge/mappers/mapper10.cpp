/**
 * @file mapper10.cpp
 * @brief Implementação do Mapper 10 (MMC4) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include "mapper10.hpp"
#include "../../../../core/logger.hpp"

using namespace MegaEmu::Platforms::NES;

Mapper10::Mapper10(Cartridge *cartridge) : Mapper(cartridge)
{
    if (!cartridge)
    {
        LOGE("Mapper10: Cartucho inválido!");
        return;
    }

    m_cartridge = cartridge;
    m_prgRomSize = cartridge->getPrgRomSize();
    m_chrRomSize = cartridge->getChrRomSize();

    // Verificar se o cartucho tem CHR-ROM, MMC4 não suporta CHR-RAM
    if (m_chrRomSize == 0)
    {
        LOGE("Mapper10: MMC4 não suporta CHR-RAM!");
    }

    // Inicializar variáveis do mapper
    m_prgBank = 0;
    m_chrBank0FD = 0;
    m_chrBank0FE = 0;
    m_chrBank1FD = 0;
    m_chrBank1FE = 0;
    m_latch0 = 0xFE; // Valor inicial dos latches
    m_latch1 = 0xFE;
    m_mirrorMode = cartridge->getMirrorMode();

    LOGI("Mapper10: Inicializado. PRG-ROM: %dKB, CHR-ROM: %dKB",
         m_prgRomSize / 1024, m_chrRomSize / 1024);
    LOGI("Mapper10: Modo de espelhamento inicial: %s",
         m_mirrorMode == MIRROR_HORIZONTAL ? "Horizontal" : m_mirrorMode == MIRROR_VERTICAL ? "Vertical"
                                                                                            : "Outro");
}

Mapper10::~Mapper10()
{
    LOGI("Mapper10: Destruído");
}

void Mapper10::reset()
{
    // Reiniciar variáveis do mapper
    m_prgBank = 0;
    m_chrBank0FD = 0;
    m_chrBank0FE = 0;
    m_chrBank1FD = 0;
    m_chrBank1FE = 0;
    m_latch0 = 0xFE;
    m_latch1 = 0xFE;
    m_mirrorMode = m_cartridge->getMirrorMode();

    LOGI("Mapper10: Reset realizado");
}

uint8_t Mapper10::cpuRead(uint16_t address)
{
    if (address < 0x8000 || address > 0xFFFF)
    {
        LOGW("Mapper10: Tentativa de leitura de CPU em endereço inválido: 0x%04X", address);
        return 0;
    }

    // No MMC4, o último banco (16KB) é fixo em 0xC000-0xFFFF
    if (address >= 0xC000)
    {
        // Último banco PRG (fixo no último banco disponível)
        uint32_t bankIndex = (m_prgRomSize / PRG_BANK_SIZE) - 1;
        uint32_t bankOffset = (address - 0xC000) % PRG_BANK_SIZE;
        uint32_t romAddress = bankIndex * PRG_BANK_SIZE + bankOffset;

        if (romAddress < m_prgRomSize)
        {
            return m_cartridge->prgRomRead(romAddress);
        }
        else
        {
            LOGW("Mapper10: Tentativa de leitura fora dos limites da PRG-ROM: 0x%04X", romAddress);
            return 0;
        }
    }
    else
    {
        // Banco selecionável (0x8000-0xBFFF)
        uint32_t bankOffset = (address - 0x8000) % PRG_BANK_SIZE;
        uint32_t romAddress = m_prgBank * PRG_BANK_SIZE + bankOffset;

        if (romAddress < m_prgRomSize)
        {
            return m_cartridge->prgRomRead(romAddress);
        }
        else
        {
            LOGW("Mapper10: Tentativa de leitura fora dos limites da PRG-ROM: 0x%04X", romAddress);
            return 0;
        }
    }
}

void Mapper10::cpuWrite(uint16_t address, uint8_t data)
{
    if (address < 0x8000 || address > 0xFFFF)
    {
        LOGW("Mapper10: Tentativa de escrita de CPU em endereço inválido: 0x%04X", address);
        return;
    }

    // MMC4 tem 5 registradores de controle mapeados no espaço de PRG-ROM
    if (address >= 0xA000 && address <= 0xAFFF)
    {
        // Seleção do banco PRG ($A000-$AFFF)
        uint8_t oldBank = m_prgBank;
        m_prgBank = data & 0x0F; // 16 bancos possíveis (4 bits)

        LOGI("Mapper10: Banco PRG alterado: %d -> %d", oldBank, m_prgBank);
    }
    else if (address >= 0xB000 && address <= 0xBFFF)
    {
        // Seleção do banco CHR 0/0xFD ($B000-$BFFF)
        uint8_t oldBank = m_chrBank0FD;
        m_chrBank0FD = data & 0x1F; // 32 bancos possíveis (5 bits)

        LOGI("Mapper10: Banco CHR 0/FD alterado: %d -> %d", oldBank, m_chrBank0FD);
    }
    else if (address >= 0xC000 && address <= 0xCFFF)
    {
        // Seleção do banco CHR 0/0xFE ($C000-$CFFF)
        uint8_t oldBank = m_chrBank0FE;
        m_chrBank0FE = data & 0x1F; // 32 bancos possíveis (5 bits)

        LOGI("Mapper10: Banco CHR 0/FE alterado: %d -> %d", oldBank, m_chrBank0FE);
    }
    else if (address >= 0xD000 && address <= 0xDFFF)
    {
        // Seleção do banco CHR 1/0xFD ($D000-$DFFF)
        uint8_t oldBank = m_chrBank1FD;
        m_chrBank1FD = data & 0x1F; // 32 bancos possíveis (5 bits)

        LOGI("Mapper10: Banco CHR 1/FD alterado: %d -> %d", oldBank, m_chrBank1FD);
    }
    else if (address >= 0xE000 && address <= 0xEFFF)
    {
        // Seleção do banco CHR 1/0xFE ($E000-$EFFF)
        uint8_t oldBank = m_chrBank1FE;
        m_chrBank1FE = data & 0x1F; // 32 bancos possíveis (5 bits)

        LOGI("Mapper10: Banco CHR 1/FE alterado: %d -> %d", oldBank, m_chrBank1FE);
    }
    else if (address >= 0xF000 && address <= 0xFFFF)
    {
        // Controle de espelhamento ($F000-$FFFF)
        MirrorMode oldMode = m_mirrorMode;
        m_mirrorMode = (data & 0x01) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL;

        m_cartridge->setMirrorMode(m_mirrorMode);

        LOGI("Mapper10: Modo de espelhamento alterado: %s -> %s",
             oldMode == MIRROR_HORIZONTAL ? "Horizontal" : oldMode == MIRROR_VERTICAL ? "Vertical"
                                                                                      : "Outro",
             m_mirrorMode == MIRROR_HORIZONTAL ? "Horizontal" : m_mirrorMode == MIRROR_VERTICAL ? "Vertical"
                                                                                                : "Outro");
    }
}

uint8_t Mapper10::ppuRead(uint16_t address)
{
    if (address > 0x1FFF)
    {
        LOGW("Mapper10: Tentativa de leitura de PPU em endereço inválido: 0x%04X", address);
        return 0;
    }

    // Verificar padrões que acionam os latches (baseado no Punch-Out!!)
    if (address == 0x0FD8)
    {
        m_latch0 = 0xFD;
    }
    else if (address == 0x0FE8)
    {
        m_latch0 = 0xFE;
    }

    if (address == 0x1FD8)
    {
        m_latch1 = 0xFD;
    }
    else if (address == 0x1FE8)
    {
        m_latch1 = 0xFE;
    }

    // Selecionar banco CHR baseado no estado dos latches
    if (address < 0x1000)
    {
        // Primeiro padrão (0x0000-0x0FFF)
        uint8_t bankIndex = (m_latch0 == 0xFD) ? m_chrBank0FD : m_chrBank0FE;
        uint32_t romAddress = bankIndex * CHR_BANK_SIZE + (address % CHR_BANK_SIZE);

        if (romAddress < m_chrRomSize)
        {
            return m_cartridge->chrRomRead(romAddress);
        }
        else
        {
            LOGW("Mapper10: Tentativa de leitura fora dos limites da CHR-ROM: 0x%04X", romAddress);
            return 0;
        }
    }
    else
    {
        // Segundo padrão (0x1000-0x1FFF)
        uint8_t bankIndex = (m_latch1 == 0xFD) ? m_chrBank1FD : m_chrBank1FE;
        uint32_t romAddress = bankIndex * CHR_BANK_SIZE + (address % CHR_BANK_SIZE);

        if (romAddress < m_chrRomSize)
        {
            return m_cartridge->chrRomRead(romAddress);
        }
        else
        {
            LOGW("Mapper10: Tentativa de leitura fora dos limites da CHR-ROM: 0x%04X", romAddress);
            return 0;
        }
    }
}

void Mapper10::ppuWrite(uint16_t address, uint8_t data)
{
    if (address > 0x1FFF)
    {
        LOGW("Mapper10: Tentativa de escrita de PPU em endereço inválido: 0x%04X", address);
        return;
    }

    // O MMC4 normalmente não usa CHR-RAM, mas implementamos por compatibilidade
    if (m_chrRomSize == 0)
    {
        // Usar CHR-RAM apenas se não houver CHR-ROM
        m_cartridge->chrRamWrite(address, data);
    }
    else
    {
        LOGW("Mapper10: Tentativa de escrita em CHR-ROM: 0x%04X", address);
    }
}

void Mapper10::scanline()
{
    // MMC4 não usa o contador de scanline
}

bool Mapper10::irqState()
{
    // MMC4 não gera IRQs
    return false;
}

void Mapper10::irqClear()
{
    // MMC4 não gera IRQs
}
