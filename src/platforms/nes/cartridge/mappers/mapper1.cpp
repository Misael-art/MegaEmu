/**
 * @file mapper1.cpp
 * @brief Implementação do Mapper 1 (MMC1) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include <iostream>
#include "mapper1.hpp"
#include "../../../../utils/log_utils.h"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            // Constantes para o MMC1
            constexpr uint8_t CONTROL_MIRROR_MASK = 0x03;
            constexpr uint8_t CONTROL_PRG_BANK_MODE_MASK = 0x0C;
            constexpr uint8_t CONTROL_CHR_BANK_MODE_MASK = 0x10;

            // Espelhamento
            constexpr uint8_t MIRROR_ONE_SCREEN_LOW = 0x00;
            constexpr uint8_t MIRROR_ONE_SCREEN_HIGH = 0x01;
            constexpr uint8_t MIRROR_VERTICAL = 0x02;
            constexpr uint8_t MIRROR_HORIZONTAL = 0x03;

            // Modos de banco PRG
            constexpr uint8_t PRG_MODE_SWITCH_32KB = 0x00;
            constexpr uint8_t PRG_MODE_SWITCH_16KB_FIX_FIRST = 0x04;
            constexpr uint8_t PRG_MODE_SWITCH_16KB_FIX_LAST = 0x08;

            // Modos de banco CHR
            constexpr uint8_t CHR_MODE_SWITCH_8KB = 0x00;
            constexpr uint8_t CHR_MODE_SWITCH_4KB = 0x10;

            // Tamanho dos bancos em bytes
            constexpr int32_t PRG_BANK_SIZE = 16 * 1024;    // 16KB
            constexpr int32_t CHR_BANK_SIZE_4KB = 4 * 1024; // 4KB
            constexpr int32_t CHR_BANK_SIZE_8KB = 8 * 1024; // 8KB

            Mapper1::Mapper1(Cartridge *cartridge) : m_cartridge(cartridge),
                                                     m_prgRomSize(0), m_chrRomSize(0),
                                                     m_usesChrRam(false), m_hasPrgRam(false),
                                                     m_hasBatteryBacked(false),
                                                     m_shiftRegister(0x10), m_shiftCount(0),
                                                     m_control(0x0C), m_chrBank0(0), m_chrBank1(0),
                                                     m_prgBank(0), m_prgMode(PrgMode::SWITCH_FIX_LAST),
                                                     m_chrMode(ChrMode::SWITCH_8K),
                                                     m_mirrorMode(MirrorMode::HORIZONTAL),
                                                     m_prgBank0(0), m_prgBank1(0),
                                                     m_chrBank0Selected(0), m_chrBank1Selected(0)
            {
                // Verificar se o cartucho é válido
                if (!cartridge)
                {
                    LOG_ERROR("Mapper1: Cartucho inválido");
                    return;
                }

                // Obter tamanhos das ROMs
                m_prgRomSize = cartridge->prg_rom_size;
                m_chrRomSize = cartridge->chr_rom_size;

                // Verificar tamanhos
                if (m_prgRomSize == 0)
                {
                    LOG_ERROR("Mapper1: Tamanho de PRG-ROM inválido: %u", m_prgRomSize);
                    return;
                }

                // Determinar se usa CHR-RAM (se CHR-ROM estiver ausente)
                m_usesChrRam = (m_chrRomSize == 0);

                // Verificar se tem PRG-RAM
                m_hasPrgRam = (cartridge->prg_ram_size > 0);

                // Determinar se tem bateria para salvar PRG-RAM
                m_hasBatteryBacked = (cartridge->flags & 0x02) != 0;

                // Inicializar espelhamento conforme cabeçalho
                if (cartridge->mirroring == MirrorType::VERTICAL)
                {
                    m_mirrorMode = MirrorMode::VERTICAL;
                }
                else if (cartridge->mirroring == MirrorType::HORIZONTAL)
                {
                    m_mirrorMode = MirrorMode::HORIZONTAL;
                }
                else if (cartridge->mirroring == MirrorType::FOUR_SCREEN)
                {
                    m_mirrorMode = MirrorMode::FOUR_SCREEN;
                }

                // Reset interno para configurar valores iniciais
                reset();

                LOG_INFO("Mapper1 (MMC1) inicializado: PRG-ROM=%uKB, %s=%uKB, PRG-RAM=%s, Bateria=%s, Mirror=%s",
                         m_prgRomSize / 1024,
                         m_usesChrRam ? "CHR-RAM" : "CHR-ROM",
                         m_usesChrRam ? 8 : (m_chrRomSize / 1024),
                         m_hasPrgRam ? "Sim" : "Não",
                         m_hasBatteryBacked ? "Sim" : "Não",
                         m_mirrorMode == MirrorMode::HORIZONTAL ? "Horizontal" : (m_mirrorMode == MirrorMode::VERTICAL ? "Vertical" : (m_mirrorMode == MirrorMode::FOUR_SCREEN ? "Four Screen" : "Single Screen")));
            }

            Mapper1::~Mapper1()
            {
                // Nada a fazer aqui
            }

            void Mapper1::reset()
            {
                // Registros de controle
                m_shiftRegister = 0x10; // Bit 4 ativo, padrão de power-on
                m_shiftCount = 0;

                // MMC1 inicializa com:
                // - Controle: $0C (PRG-ROM mode 3, CHR-ROM mode 0)
                // - CHR Bank 0: $00
                // - CHR Bank 1: $00
                // - PRG Bank: $00
                m_control = 0x0C;
                m_chrBank0 = 0;
                m_chrBank1 = 0;
                m_prgBank = 0;

                // Configurar modos com base no registro de controle
                m_prgMode = static_cast<PrgMode>((m_control >> 2) & 0x03);
                m_chrMode = static_cast<ChrMode>((m_control >> 4) & 0x01);

                // Aplicar valores iniciais
                updateMirroring();
                updatePrgBanks();
                updateChrBanks();

                LOG_INFO("Mapper1 (MMC1) resetado");
            }

            uint8_t Mapper1::cpuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x6000)
                {
                    LOG_WARNING("Mapper1: Tentativa de leitura fora do intervalo: 0x%04X", address);
                    return 0;
                }

                // Acesso à PRG-RAM ($6000-$7FFF)
                if (address >= 0x6000 && address < 0x8000)
                {
                    if (m_hasPrgRam)
                    {
                        uint32_t addr = address - 0x6000;
                        if (addr < PRG_RAM_SIZE)
                        {
                            return m_cartridge->prg_ram[addr];
                        }
                    }
                    return 0;
                }

                // Acesso à PRG-ROM ($8000-$FFFF)
                uint32_t addr;

                if (address < 0xC000)
                {
                    // $8000-$BFFF (16KB)
                    uint32_t bankOffset = m_prgBank0 * PRG_BANK_SIZE;
                    addr = bankOffset + (address - 0x8000);
                }
                else
                {
                    // $C000-$FFFF (16KB)
                    uint32_t bankOffset = m_prgBank1 * PRG_BANK_SIZE;
                    addr = bankOffset + (address - 0xC000);
                }

                // Verificar limites
                if (addr >= m_prgRomSize)
                {
                    LOG_WARNING("Mapper1: Endereço fora dos limites: 0x%06X (máximo: 0x%06X)",
                                addr, m_prgRomSize - 1);
                    addr %= m_prgRomSize;
                }

                return m_cartridge->prg_rom[addr];
            }

            void Mapper1::cpuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x6000)
                {
                    LOG_WARNING("Mapper1: Tentativa de escrita fora do intervalo: 0x%04X = 0x%02X", address, data);
                    return;
                }

                // Acesso à PRG-RAM ($6000-$7FFF)
                if (address >= 0x6000 && address < 0x8000)
                {
                    if (m_hasPrgRam)
                    {
                        uint32_t addr = address - 0x6000;
                        if (addr < PRG_RAM_SIZE)
                        {
                            m_cartridge->prg_ram[addr] = data;
                        }
                    }
                    return;
                }

                // Escrita em registros de controle ($8000-$FFFF)

                // MMC1 tem um comportamento especial: escreve bit a bit
                // Bit 7 ativo (0x80) reseta o registrador de deslocamento
                if (data & 0x80)
                {
                    m_shiftRegister = 0x10; // Resetar (bit 4 ativo)
                    m_shiftCount = 0;

                    // Também retorna para o modo 3 (último banco fixado)
                    m_control |= 0x0C;
                    m_prgMode = PrgMode::SWITCH_FIX_LAST;
                    updatePrgBanks();

                    LOG_INFO("Mapper1: Reset do registrador de deslocamento");
                    return;
                }

                // Escrita serial: a cada escrita, desloca-se os bits
                // e adiciona o bit 0 do dado escrito ao registrador
                m_shiftRegister = (m_shiftRegister >> 1) | ((data & 0x01) << 4);
                m_shiftCount++;

                // Após 5 bits, atualiza o registrador correspondente
                if (m_shiftCount >= 5)
                {
                    // Determinar qual registrador atualizar com base no endereço
                    if (address <= 0x9FFF)
                    {
                        // $8000-$9FFF: Registrador de Controle
                        m_control = m_shiftRegister;

                        // Extrair modos dos bits do controle
                        m_prgMode = static_cast<PrgMode>((m_control >> 2) & 0x03);
                        m_chrMode = static_cast<ChrMode>((m_control >> 4) & 0x01);

                        // Atualizar espelhamento
                        updateMirroring();

                        // Atualizar bancos devido mudança de modo
                        updatePrgBanks();
                        updateChrBanks();

                        LOG_INFO("Mapper1: Registro de Controle atualizado = 0x%02X (Prg:%u, Chr:%u)",
                                 m_control,
                                 static_cast<uint8_t>(m_prgMode),
                                 static_cast<uint8_t>(m_chrMode));
                    }
                    else if (address <= 0xBFFF)
                    {
                        // $A000-$BFFF: CHR Bank 0
                        m_chrBank0 = m_shiftRegister;
                        updateChrBanks();
                        LOG_INFO("Mapper1: CHR Bank 0 atualizado = 0x%02X", m_chrBank0);
                    }
                    else if (address <= 0xDFFF)
                    {
                        // $C000-$DFFF: CHR Bank 1
                        m_chrBank1 = m_shiftRegister;
                        updateChrBanks();
                        LOG_INFO("Mapper1: CHR Bank 1 atualizado = 0x%02X", m_chrBank1);
                    }
                    else
                    {
                        // $E000-$FFFF: PRG Bank
                        m_prgBank = m_shiftRegister & 0x0F; // Apenas 4 bits inferiores
                        updatePrgBanks();
                        LOG_INFO("Mapper1: PRG Bank atualizado = 0x%02X", m_prgBank);
                    }

                    // Resetar registrador de deslocamento para próxima operação
                    m_shiftRegister = 0x10;
                    m_shiftCount = 0;
                }
            }

            uint8_t Mapper1::ppuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper1: Tentativa de leitura PPU fora do intervalo: 0x%04X", address);
                    return 0;
                }

                if (m_usesChrRam)
                {
                    // Usar CHR-RAM (8KB)
                    return m_cartridge->chr_ram[address];
                }
                else
                {
                    // Usar CHR-ROM
                    uint32_t addr;

                    if (m_chrMode == ChrMode::SWITCH_8K)
                    {
                        // 8KB mode - um único banco contíguo
                        uint32_t bank8k = m_chrBank0Selected / 2;
                        addr = (bank8k * 8 * 1024) + address;
                    }
                    else // SWITCH_4K
                    {
                        // 4KB mode - dois bancos independentes
                        if (address < 0x1000)
                        {
                            // $0000-$0FFF: CHR Bank 0
                            addr = (m_chrBank0Selected * CHR_BANK_SIZE) + (address & 0x0FFF);
                        }
                        else
                        {
                            // $1000-$1FFF: CHR Bank 1
                            addr = (m_chrBank1Selected * CHR_BANK_SIZE) + (address & 0x0FFF);
                        }
                    }

                    // Verificar limites
                    if (addr >= m_chrRomSize)
                    {
                        LOG_WARNING("Mapper1: Endereço CHR fora dos limites: 0x%06X (máximo: 0x%06X)",
                                    addr, m_chrRomSize - 1);
                        addr %= m_chrRomSize;
                    }

                    return m_cartridge->chr_rom[addr];
                }
            }

            void Mapper1::ppuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper1: Tentativa de escrita PPU fora do intervalo: 0x%04X = 0x%02X", address, data);
                    return;
                }

                if (m_usesChrRam)
                {
                    // Escrever na CHR-RAM
                    m_cartridge->chr_ram[address] = data;
                }
                else
                {
                    // Não é possível escrever na CHR-ROM
                    LOG_WARNING("Mapper1: Tentativa de escrita em CHR-ROM: 0x%04X = 0x%02X", address, data);
                }
            }

            void Mapper1::scanline()
            {
                // MMC1 não suporta IRQs, nada a fazer
            }

            bool Mapper1::irqState()
            {
                // MMC1 não suporta IRQs
                return false;
            }

            void Mapper1::irqClear()
            {
                // MMC1 não suporta IRQs, nada a fazer
            }

            void Mapper1::updatePrgBanks()
            {
                uint8_t prgBankCount = m_prgRomSize / PRG_BANK_SIZE;

                switch (m_prgMode)
                {
                case PrgMode::SWITCH_32K:
                    // 32KB switching: bits 0-3 de prgBank selecionam banco de 32KB
                    // (cada banco de 32KB consiste em dois bancos de 16KB consecutivos)
                    {
                        uint8_t bank32k = m_prgBank & 0x0E; // Ignorar bit 0, multiplicar por 2
                        m_prgBank0 = bank32k;
                        m_prgBank1 = bank32k + 1;
                    }
                    break;

                case PrgMode::SWITCH_FIX_FIRST:
                    // Primeiro banco fixo em $8000, seleciona banco para $C000
                    m_prgBank0 = 0;
                    m_prgBank1 = m_prgBank;
                    break;

                case PrgMode::SWITCH_FIX_LAST:
                    // Último banco fixo em $C000, seleciona banco para $8000
                    m_prgBank0 = m_prgBank;
                    m_prgBank1 = prgBankCount - 1;
                    break;

                case PrgMode::SWITCH_16K:
                    // 16KB switching: seleciona bancos independentes
                    m_prgBank0 = m_prgBank;
                    m_prgBank1 = m_prgBank + 1; // O banco seguinte
                    break;
                }

                // Garantir que os bancos estejam dentro dos limites
                m_prgBank0 %= prgBankCount;
                m_prgBank1 %= prgBankCount;

                LOG_INFO("Mapper1: PRG Bancos atualizados - $8000:banco %u, $C000:banco %u, modo: %u",
                         m_prgBank0, m_prgBank1, static_cast<uint8_t>(m_prgMode));
            }

            void Mapper1::updateChrBanks()
            {
                if (m_usesChrRam)
                {
                    // Para CHR-RAM, sempre usar banco 0
                    m_chrBank0Selected = 0;
                    m_chrBank1Selected = 0;
                    return;
                }

                uint8_t chrBankCount = m_chrRomSize / CHR_BANK_SIZE;

                if (m_chrMode == ChrMode::SWITCH_8K)
                {
                    // 8KB mode: ignora chrBank1, usa apenas chrBank0 (bits 1-4)
                    // para selecionar bancos de 8KB (cada par de bancos de 4KB consecutivos)
                    m_chrBank0Selected = (m_chrBank0 & 0x1E); // Ignora bit 0, multiplica por 2
                    m_chrBank1Selected = m_chrBank0Selected + 1;
                }
                else // SWITCH_4K
                {
                    // 4KB mode: chrBank0 e chrBank1 selecionam bancos independentes de 4KB
                    m_chrBank0Selected = m_chrBank0;
                    m_chrBank1Selected = m_chrBank1;
                }

                // Garantir que os bancos estejam dentro dos limites
                if (chrBankCount > 0)
                {
                    m_chrBank0Selected %= chrBankCount;
                    m_chrBank1Selected %= chrBankCount;
                }

                LOG_INFO("Mapper1: CHR Bancos atualizados - $0000:banco %u, $1000:banco %u, modo: %u",
                         m_chrBank0Selected, m_chrBank1Selected, static_cast<uint8_t>(m_chrMode));
            }

            void Mapper1::updateMirroring()
            {
                // Os bits 0-1 do registro de controle determinam o espelhamento
                switch (m_control & 0x03)
                {
                case 0:
                    m_mirrorMode = MirrorMode::SINGLE_SCREEN_LOWER;
                    break;
                case 1:
                    m_mirrorMode = MirrorMode::SINGLE_SCREEN_UPPER;
                    break;
                case 2:
                    m_mirrorMode = MirrorMode::VERTICAL;
                    break;
                case 3:
                    m_mirrorMode = MirrorMode::HORIZONTAL;
                    break;
                }

                LOG_INFO("Mapper1: Modo de espelhamento atualizado: %u", static_cast<uint8_t>(m_mirrorMode));
            }
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu
