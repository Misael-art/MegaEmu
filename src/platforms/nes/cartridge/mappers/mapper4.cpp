/**
 * @file mapper4.cpp
 * @brief Implementação do Mapper 4 (MMC3) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-28
 */

#include <cstdio>
#include "mapper4.hpp"
#include "../../../../utils/log_utils.h"

/**
 * @brief Construtor para o Mapper 4
 * @param cartridge Ponteiro para o cartucho
 */
Mapper4::Mapper4(Cartridge *cartridge) : m_cartridge(cartridge)
{
    // Verificar se o cartucho é válido
    if (!cartridge)
    {
        LOG_ERROR("Mapper4: Cartucho inválido");
        return;
    }

    // Obter tamanhos das ROMs
    m_prgRomSize = cartridge->prg_rom_size;
    m_chrRomSize = cartridge->chr_rom_size;

    // Verificar tamanhos
    if (m_prgRomSize == 0)
    {
        LOG_ERROR("Mapper4: Tamanho de PRG-ROM inválido: %u", m_prgRomSize);
        return;
    }

    // Determinar se usa CHR-RAM
    m_usesChrRam = (m_chrRomSize == 0 && cartridge->chr_ram != nullptr);

    if (m_chrRomSize == 0 && !m_usesChrRam)
    {
        LOG_ERROR("Mapper4: Sem CHR-ROM ou CHR-RAM");
        return;
    }

    // Inicializar registradores
    m_bankSelect = 0;
    for (int i = 0; i < 8; i++)
    {
        m_bankData[i] = 0;
    }
    m_mirrorMode = 0;
    m_prgRamProtect = 0;
    m_irqLatch = 0;
    m_irqCounter = 0;
    m_irqEnable = false;
    m_irqPending = false;

    // Inicializar endereços dos bancos
    reset();

    LOG_INFO("Mapper4 (MMC3) inicializado: PRG-ROM=%uKB, CHR-%s=%uKB",
             m_prgRomSize / 1024,
             m_usesChrRam ? "RAM" : "ROM",
             (m_usesChrRam ? cartridge->chr_ram_size : m_chrRomSize) / 1024);
}

/**
 * @brief Destrutor para o Mapper 4
 */
Mapper4::~Mapper4()
{
    // Nada a fazer aqui
}

/**
 * @brief Reseta o estado do mapper
 */
void Mapper4::reset()
{
    // Resetar registradores
    m_bankSelect = 0;
    for (int i = 0; i < 8; i++)
    {
        m_bankData[i] = 0;
    }
    m_mirrorMode = 0;
    m_prgRamProtect = 0;
    m_irqLatch = 0;
    m_irqCounter = 0;
    m_irqEnable = false;
    m_irqPending = false;

    // Configuração inicial dos bancos
    updateBanks();

    LOG_INFO("Mapper4 (MMC3) resetado");
}

/**
 * @brief Atualiza o mapeamento de bancos após mudança nos registradores
 */
void Mapper4::updateBanks()
{
    // Tamanho de um banco PRG (8KB)
    const uint32_t prgBankSize = 8 * 1024;

    // Tamanho de um banco CHR (1KB)
    const uint32_t chrBankSize = 1 * 1024;

    // Número total de bancos PRG
    const uint32_t prgBanksCount = m_prgRomSize / prgBankSize;

    // Número total de bancos CHR
    const uint32_t chrBanksCount = m_usesChrRam ? (m_cartridge->chr_ram_size / chrBankSize) : (m_chrRomSize / chrBankSize);

    // Modo de mapeamento PRG (bit 6 do bank select)
    bool prgMode = (m_bankSelect & 0x40) != 0;

    // Modo de mapeamento CHR (bit 7 do bank select)
    bool chrMode = (m_bankSelect & 0x80) != 0;

    // Mapeamento de bancos PRG
    if (!prgMode)
    {
        // Modo 0: Banco em $8000 é fixo no penúltimo, banco em $A000 é variável
        m_prgBanks[0] = (prgBanksCount - 2) * prgBankSize;
        m_prgBanks[1] = (m_bankData[6] % prgBanksCount) * prgBankSize;
    }
    else
    {
        // Modo 1: Banco em $8000 é variável, banco em $A000 é fixo no penúltimo
        m_prgBanks[0] = (m_bankData[6] % prgBanksCount) * prgBankSize;
        m_prgBanks[1] = (prgBanksCount - 2) * prgBankSize;
    }

    // Os dois últimos bancos são fixos:
    // $C000-$DFFF: Variável (registrador 7)
    m_prgBanks[2] = (m_bankData[7] % prgBanksCount) * prgBankSize;
    // $E000-$FFFF: Fixo no último banco
    m_prgBanks[3] = (prgBanksCount - 1) * prgBankSize;

    // Mapeamento de bancos CHR
    if (!chrMode)
    {
        // Modo 0: Dois bancos de 2KB em $0000, quatro bancos de 1KB em $1000
        // $0000-$07FF: Registrador 0 (2KB)
        m_chrBanks[0] = ((m_bankData[0] & 0xFE) % chrBanksCount) * chrBankSize;
        // $0800-$0FFF: Registrador 1 (2KB)
        m_chrBanks[1] = ((m_bankData[1] & 0xFE) % chrBanksCount) * chrBankSize;
        // $1000-$13FF: Registrador 2 (1KB)
        m_chrBanks[2] = (m_bankData[2] % chrBanksCount) * chrBankSize;
        // $1400-$17FF: Registrador 3 (1KB)
        m_chrBanks[3] = (m_bankData[3] % chrBanksCount) * chrBankSize;
        // $1800-$1BFF: Registrador 4 (1KB)
        m_chrBanks[4] = (m_bankData[4] % chrBanksCount) * chrBankSize;
        // $1C00-$1FFF: Registrador 5 (1KB)
        m_chrBanks[5] = (m_bankData[5] % chrBanksCount) * chrBankSize;
    }
    else
    {
        // Modo 1: Dois bancos de 2KB em $1000, quatro bancos de 1KB em $0000
        // $0000-$03FF: Registrador 2 (1KB)
        m_chrBanks[0] = (m_bankData[2] % chrBanksCount) * chrBankSize;
        // $0400-$07FF: Registrador 3 (1KB)
        m_chrBanks[1] = (m_bankData[3] % chrBanksCount) * chrBankSize;
        // $0800-$0BFF: Registrador 4 (1KB)
        m_chrBanks[2] = (m_bankData[4] % chrBanksCount) * chrBankSize;
        // $0C00-$0FFF: Registrador 5 (1KB)
        m_chrBanks[3] = (m_bankData[5] % chrBanksCount) * chrBankSize;
        // $1000-$17FF: Registrador 0 (2KB)
        m_chrBanks[4] = ((m_bankData[0] & 0xFE) % chrBanksCount) * chrBankSize;
        // $1800-$1FFF: Registrador 1 (2KB)
        m_chrBanks[5] = ((m_bankData[1] & 0xFE) % chrBanksCount) * chrBankSize;
    }

    LOG_DEBUG("Mapper4: PRG banks - $8000: %06X, $A000: %06X, $C000: %06X, $E000: %06X",
              m_prgBanks[0], m_prgBanks[1], m_prgBanks[2], m_prgBanks[3]);
    LOG_DEBUG("Mapper4: CHR banks - $0000: %06X, $0400/$0800: %06X, $1000: %06X, $1400/$1800: %06X",
              m_chrBanks[0], m_chrBanks[2], m_chrBanks[4], m_chrBanks[5]);
}

/**
 * @brief Lê um byte da CPU
 * @param address Endereço de memória (0x8000-0xFFFF)
 * @return Byte lido
 */
uint8_t Mapper4::cpuRead(uint16_t address)
{
    // Verificar se o endereço está no intervalo correto
    if (address < 0x6000)
    {
        LOG_WARNING("Mapper4: Tentativa de leitura fora do intervalo: 0x%04X", address);
        return 0;
    }

    // PRG-RAM em $6000-$7FFF
    if (address >= 0x6000 && address < 0x8000)
    {
        // Verificar se a PRG-RAM está habilitada (depende do bit 7 de m_prgRamProtect)
        if ((m_prgRamProtect & 0x80) == 0)
        {
            // PRG-RAM desabilitada
            return 0;
        }

        // Calcular endereço na PRG-RAM
        uint32_t addr = address - 0x6000;
        if (addr >= m_cartridge->prg_ram_size)
        {
            LOG_WARNING("Mapper4: Endereço de PRG-RAM fora dos limites: 0x%04X (máximo: 0x%06X)",
                        addr, m_cartridge->prg_ram_size - 1);
            addr %= m_cartridge->prg_ram_size;
        }

        return m_cartridge->prg_ram[addr];
    }

    // Mapeamento de PRG-ROM em $8000-$FFFF
    uint32_t bank = (address - 0x8000) / 0x2000;   // Qual banco de 8KB (0-3)
    uint32_t offset = (address - 0x8000) % 0x2000; // Deslocamento dentro do banco
    uint32_t addr = m_prgBanks[bank] + offset;

    // Verificar limites
    if (addr >= m_prgRomSize)
    {
        LOG_WARNING("Mapper4: Endereço de PRG-ROM fora dos limites: 0x%06X (máximo: 0x%06X)",
                    addr, m_prgRomSize - 1);
        addr %= m_prgRomSize;
    }

    return m_cartridge->prg_rom[addr];
}

/**
 * @brief Escreve um byte pela CPU
 * @param address Endereço de memória (0x8000-0xFFFF)
 * @param data Byte a ser escrito
 */
void Mapper4::cpuWrite(uint16_t address, uint8_t data)
{
    // Verificar se o endereço está no intervalo correto
    if (address < 0x6000)
    {
        LOG_WARNING("Mapper4: Tentativa de escrita fora do intervalo: 0x%04X", address);
        return;
    }

    // PRG-RAM em $6000-$7FFF
    if (address >= 0x6000 && address < 0x8000)
    {
        // Verificar se a PRG-RAM está habilitada e permitida para escrita
        // Bit 7 = 1: PRG-RAM habilitada, Bit 6 = 0: PRG-RAM gravável
        if ((m_prgRamProtect & 0x80) != 0 && (m_prgRamProtect & 0x40) == 0)
        {
            // Calcular endereço na PRG-RAM
            uint32_t addr = address - 0x6000;
            if (addr >= m_cartridge->prg_ram_size)
            {
                LOG_WARNING("Mapper4: Endereço de PRG-RAM fora dos limites: 0x%04X (máximo: 0x%06X)",
                            addr, m_cartridge->prg_ram_size - 1);
                addr %= m_cartridge->prg_ram_size;
            }

            m_cartridge->prg_ram[addr] = data;
        }
        else
        {
            LOG_WARNING("Mapper4: Tentativa de escrita em PRG-RAM protegida: 0x%04X = 0x%02X",
                        address, data);
        }
        return;
    }

    // Registradores do MMC3 em $8000-$FFFF (pares de registradores)
    switch (address & 0xE001)
    {
    case 0x8000: // Bank select ($8000-$9FFE, even)
        m_bankSelect = data;
        updateBanks();
        break;

    case 0x8001: // Bank data ($8001-$9FFF, odd)
    {
        // Índice do banco selecionado (0-7)
        uint8_t bankIndex = m_bankSelect & 0x07;
        m_bankData[bankIndex] = data;
        updateBanks();
    }
    break;

    case 0xA000: // Mirroring ($A000-$BFFE, even)
        m_mirrorMode = data & 0x01;
        LOG_DEBUG("Mapper4: Modo de mirroring alterado para %s",
                  m_mirrorMode ? "Horizontal" : "Vertical");
        break;

    case 0xA001: // PRG RAM protect ($A001-$BFFF, odd)
        m_prgRamProtect = data;
        LOG_DEBUG("Mapper4: Proteção de PRG-RAM alterada: %02X (Habilitada: %d, Protegida: %d)",
                  m_prgRamProtect,
                  (m_prgRamProtect & 0x80) != 0,
                  (m_prgRamProtect & 0x40) != 0);
        break;

    case 0xC000: // IRQ latch ($C000-$DFFE, even)
        m_irqLatch = data;
        LOG_DEBUG("Mapper4: IRQ latch definido: %02X", m_irqLatch);
        break;

    case 0xC001:          // IRQ reload ($C001-$DFFF, odd)
        m_irqCounter = 0; // Força a recarga no próximo ciclo
        LOG_DEBUG("Mapper4: IRQ counter será recarregado no próximo ciclo");
        break;

    case 0xE000: // IRQ disable ($E000-$FFFE, even)
        m_irqEnable = false;
        m_irqPending = false; // Limpa o IRQ pendente
        LOG_DEBUG("Mapper4: IRQ desabilitado");
        break;

    case 0xE001: // IRQ enable ($E001-$FFFF, odd)
        m_irqEnable = true;
        LOG_DEBUG("Mapper4: IRQ habilitado");
        break;

    default:
        LOG_WARNING("Mapper4: Escrita em endereço desconhecido: 0x%04X = 0x%02X", address, data);
        break;
    }
}

/**
 * @brief Lê um byte da PPU (CHR)
 * @param address Endereço de memória (0x0000-0x1FFF)
 * @return Byte lido
 */
uint8_t Mapper4::ppuRead(uint16_t address)
{
    // Verificar se o endereço está no intervalo correto
    if (address >= 0x2000)
    {
        LOG_WARNING("Mapper4: Tentativa de leitura CHR fora do intervalo: 0x%04X", address);
        return 0;
    }

    // Determinar o banco e o deslocamento
    uint32_t bank;
    uint32_t offset;

    // Modo de mapeamento CHR (bit 7 do bank select)
    bool chrMode = (m_bankSelect & 0x80) != 0;

    if (!chrMode)
    {
        // Modo 0: Dois bancos de 2KB em $0000, quatro bancos de 1KB em $1000
        if (address < 0x0800)
        {
            // $0000-$07FF: 2KB
            bank = 0;
            offset = address;
        }
        else if (address < 0x1000)
        {
            // $0800-$0FFF: 2KB
            bank = 1;
            offset = address - 0x0800;
        }
        else if (address < 0x1400)
        {
            // $1000-$13FF: 1KB
            bank = 2;
            offset = address - 0x1000;
        }
        else if (address < 0x1800)
        {
            // $1400-$17FF: 1KB
            bank = 3;
            offset = address - 0x1400;
        }
        else if (address < 0x1C00)
        {
            // $1800-$1BFF: 1KB
            bank = 4;
            offset = address - 0x1800;
        }
        else
        {
            // $1C00-$1FFF: 1KB
            bank = 5;
            offset = address - 0x1C00;
        }
    }
    else
    {
        // Modo 1: Dois bancos de 2KB em $1000, quatro bancos de 1KB em $0000
        if (address < 0x0400)
        {
            // $0000-$03FF: 1KB
            bank = 0;
            offset = address;
        }
        else if (address < 0x0800)
        {
            // $0400-$07FF: 1KB
            bank = 1;
            offset = address - 0x0400;
        }
        else if (address < 0x0C00)
        {
            // $0800-$0BFF: 1KB
            bank = 2;
            offset = address - 0x0800;
        }
        else if (address < 0x1000)
        {
            // $0C00-$0FFF: 1KB
            bank = 3;
            offset = address - 0x0C00;
        }
        else if (address < 0x1800)
        {
            // $1000-$17FF: 2KB
            bank = 4;
            offset = address - 0x1000;
        }
        else
        {
            // $1800-$1FFF: 2KB
            bank = 5;
            offset = address - 0x1800;
        }
    }

    // Calcular o endereço final
    uint32_t addr = m_chrBanks[bank] + offset;

    // Ler de CHR-ROM ou CHR-RAM
    if (m_usesChrRam)
    {
        // Usar CHR-RAM
        if (addr >= m_cartridge->chr_ram_size)
        {
            LOG_WARNING("Mapper4: Endereço CHR-RAM fora dos limites: 0x%06X (máximo: 0x%06X)",
                        addr, m_cartridge->chr_ram_size - 1);
            addr %= m_cartridge->chr_ram_size;
        }
        return m_cartridge->chr_ram[addr];
    }
    else
    {
        // Usar CHR-ROM
        if (addr >= m_chrRomSize)
        {
            LOG_WARNING("Mapper4: Endereço CHR-ROM fora dos limites: 0x%06X (máximo: 0x%06X)",
                        addr, m_chrRomSize - 1);
            addr %= m_chrRomSize;
        }
        return m_cartridge->chr_rom[addr];
    }
}

/**
 * @brief Escreve um byte pela PPU (CHR)
 * @param address Endereço de memória (0x0000-0x1FFF)
 * @param data Byte a ser escrito
 */
void Mapper4::ppuWrite(uint16_t address, uint8_t data)
{
    // Verificar se o endereço está no intervalo correto
    if (address >= 0x2000)
    {
        LOG_WARNING("Mapper4: Tentativa de escrita CHR fora do intervalo: 0x%04X", address);
        return;
    }

    // Se não estiver usando CHR-RAM, não pode escrever
    if (!m_usesChrRam)
    {
        LOG_WARNING("Mapper4: Tentativa de escrita em CHR-ROM: 0x%04X = 0x%02X", address, data);
        return;
    }

    // Determinar o banco e o deslocamento (mesma lógica de ppuRead)
    uint32_t bank;
    uint32_t offset;

    // Modo de mapeamento CHR (bit 7 do bank select)
    bool chrMode = (m_bankSelect & 0x80) != 0;

    if (!chrMode)
    {
        // Modo 0: Dois bancos de 2KB em $0000, quatro bancos de 1KB em $1000
        if (address < 0x0800)
        {
            // $0000-$07FF: 2KB
            bank = 0;
            offset = address;
        }
        else if (address < 0x1000)
        {
            // $0800-$0FFF: 2KB
            bank = 1;
            offset = address - 0x0800;
        }
        else if (address < 0x1400)
        {
            // $1000-$13FF: 1KB
            bank = 2;
            offset = address - 0x1000;
        }
        else if (address < 0x1800)
        {
            // $1400-$17FF: 1KB
            bank = 3;
            offset = address - 0x1400;
        }
        else if (address < 0x1C00)
        {
            // $1800-$1BFF: 1KB
            bank = 4;
            offset = address - 0x1800;
        }
        else
        {
            // $1C00-$1FFF: 1KB
            bank = 5;
            offset = address - 0x1C00;
        }
    }
    else
    {
        // Modo 1: Dois bancos de 2KB em $1000, quatro bancos de 1KB em $0000
        if (address < 0x0400)
        {
            // $0000-$03FF: 1KB
            bank = 0;
            offset = address;
        }
        else if (address < 0x0800)
        {
            // $0400-$07FF: 1KB
            bank = 1;
            offset = address - 0x0400;
        }
        else if (address < 0x0C00)
        {
            // $0800-$0BFF: 1KB
            bank = 2;
            offset = address - 0x0800;
        }
        else if (address < 0x1000)
        {
            // $0C00-$0FFF: 1KB
            bank = 3;
            offset = address - 0x0C00;
        }
        else if (address < 0x1800)
        {
            // $1000-$17FF: 2KB
            bank = 4;
            offset = address - 0x1000;
        }
        else
        {
            // $1800-$1FFF: 2KB
            bank = 5;
            offset = address - 0x1800;
        }
    }

    // Calcular o endereço final
    uint32_t addr = m_chrBanks[bank] + offset;

    // Escrever na CHR-RAM
    if (addr >= m_cartridge->chr_ram_size)
    {
        LOG_WARNING("Mapper4: Endereço CHR-RAM fora dos limites: 0x%06X (máximo: 0x%06X)",
                    addr, m_cartridge->chr_ram_size - 1);
        addr %= m_cartridge->chr_ram_size;
    }

    m_cartridge->chr_ram[addr] = data;
}

/**
 * @brief Notifica o mapper sobre uma nova scanline para IRQ
 */
void Mapper4::scanline()
{
    // Se o contador estiver em 0, recarregar com o valor do latch
    if (m_irqCounter == 0)
    {
        m_irqCounter = m_irqLatch;
    }
    else
    {
        // Decrementar o contador
        m_irqCounter--;

        // Se o contador chegou a 0 e o IRQ está habilitado, gerar IRQ
        if (m_irqCounter == 0 && m_irqEnable)
        {
            m_irqPending = true;
            LOG_DEBUG("Mapper4: IRQ gerado");
        }
    }
}

/**
 * @brief Retorna o estado do sinal de IRQ
 * @return true se IRQ está ativo, false caso contrário
 */
bool Mapper4::irqState()
{
    return m_irqPending;
}

/**
 * @brief Limpa o sinal de IRQ
 */
void Mapper4::irqClear()
{
    m_irqPending = false;
}
