/**
 * @file mapper9.cpp
 * @brief Implementação do Mapper 9 (MMC2) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include <iostream>
#include "mapper9.hpp"
#include "../../../../utils/log_utils.h"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            Mapper9::Mapper9(Cartridge *cartridge) : m_cartridge(cartridge),
                                                     m_prgRomSize(0), m_chrRomSize(0),
                                                     m_usesChrRam(false),
                                                     m_prgBank(0),
                                                     m_chrBank0FD(0), m_chrBank0FE(0),
                                                     m_chrBank1FD(0), m_chrBank1FE(0),
                                                     m_latch0(0xFE), m_latch1(0xFE),
                                                     m_mirrorMode(MirrorMode::HORIZONTAL)
            {
                // Verificar se o cartucho é válido
                if (!cartridge)
                {
                    LOG_ERROR("Mapper9: Cartucho inválido");
                    return;
                }

                // Obter tamanhos das ROMs
                m_prgRomSize = cartridge->prg_rom_size;
                m_chrRomSize = cartridge->chr_rom_size;

                // Verificar tamanhos
                if (m_prgRomSize == 0)
                {
                    LOG_ERROR("Mapper9: Tamanho de PRG-ROM inválido: %u", m_prgRomSize);
                    return;
                }

                if (m_chrRomSize == 0)
                {
                    LOG_ERROR("Mapper9: MMC2 requer CHR-ROM, não suporta CHR-RAM");
                    return;
                }

                // Configurar espelhamento inicial com base no cabeçalho do cartucho
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

                LOG_INFO("Mapper9 (MMC2) inicializado: PRG-ROM=%uKB, CHR-ROM=%uKB, Mirroring=%d",
                         m_prgRomSize / 1024, m_chrRomSize / 1024,
                         static_cast<int>(m_mirrorMode));
            }

            Mapper9::~Mapper9()
            {
                // Nada a fazer aqui
            }

            void Mapper9::reset()
            {
                // MMC2 inicializa com valores padrão
                m_prgBank = 0;
                m_chrBank0FD = 0;
                m_chrBank0FE = 0;
                m_chrBank1FD = 0;
                m_chrBank1FE = 0;
                m_latch0 = 0xFE;
                m_latch1 = 0xFE;

                LOG_INFO("Mapper9 (MMC2) resetado");
            }

            uint8_t Mapper9::cpuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x8000)
                {
                    LOG_WARNING("Mapper9: Tentativa de leitura fora do intervalo: 0x%04X", address);
                    return 0;
                }

                // O MMC2 divide o espaço da PRG-ROM em 4 partes de 8KB
                uint32_t addr;

                if (address >= 0xA000 && address < 0xC000)
                {
                    // Banco selecionável em $A000-$BFFF
                    uint32_t bankOffset = m_prgBank * PRG_BANK_SIZE;
                    addr = bankOffset + (address - 0xA000);
                }
                else if (address >= 0x8000 && address < 0xA000)
                {
                    // $8000-$9FFF é sempre o primeiro banco de 8KB (fixo)
                    addr = (address - 0x8000);
                }
                else if (address >= 0xC000 && address < 0xE000)
                {
                    // $C000-$DFFF é sempre o penúltimo banco de 8KB (fixo)
                    uint32_t bankOffset = (m_prgRomSize - PRG_BANK_SIZE * 2);
                    addr = bankOffset + (address - 0xC000);
                }
                else // $E000-$FFFF
                {
                    // $E000-$FFFF é sempre o último banco de 8KB (fixo)
                    uint32_t bankOffset = (m_prgRomSize - PRG_BANK_SIZE);
                    addr = bankOffset + (address - 0xE000);
                }

                // Verificar limites
                if (addr >= m_prgRomSize)
                {
                    LOG_WARNING("Mapper9: Endereço fora dos limites: 0x%06X (máximo: 0x%06X)",
                                addr, m_prgRomSize - 1);
                    addr %= m_prgRomSize;
                }

                return m_cartridge->prg_rom[addr];
            }

            void Mapper9::cpuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x8000)
                {
                    LOG_WARNING("Mapper9: Tentativa de escrita fora do intervalo: 0x%04X = 0x%02X", address, data);
                    return;
                }

                // No MMC2, os registros de controle são mapeados para diferentes áreas da memória
                if (address >= 0x8000 && address <= 0xAFFF)
                {
                    // $8000-$AFFF: Selecionar banco PRG de 8KB em $A000-$BFFF
                    uint8_t bankNum = data & 0x0F; // 4 bits mais baixos
                    if (bankNum != m_prgBank)
                    {
                        m_prgBank = bankNum;
                        LOG_INFO("Mapper9: Banco PRG selecionado: %d", m_prgBank);
                    }
                }
                else if (address >= 0xB000 && address <= 0xBFFF)
                {
                    // $B000-$BFFF: Selecionar banco CHR de 4KB para $0000 quando latch 0 = $FD
                    uint8_t bankNum = data & 0x1F; // 5 bits mais baixos
                    if (bankNum != m_chrBank0FD)
                    {
                        m_chrBank0FD = bankNum;
                        LOG_INFO("Mapper9: Banco CHR0 (FD) selecionado: %d", m_chrBank0FD);
                    }
                }
                else if (address >= 0xC000 && address <= 0xCFFF)
                {
                    // $C000-$CFFF: Selecionar banco CHR de 4KB para $0000 quando latch 0 = $FE
                    uint8_t bankNum = data & 0x1F; // 5 bits mais baixos
                    if (bankNum != m_chrBank0FE)
                    {
                        m_chrBank0FE = bankNum;
                        LOG_INFO("Mapper9: Banco CHR0 (FE) selecionado: %d", m_chrBank0FE);
                    }
                }
                else if (address >= 0xD000 && address <= 0xDFFF)
                {
                    // $D000-$DFFF: Selecionar banco CHR de 4KB para $1000 quando latch 1 = $FD
                    uint8_t bankNum = data & 0x1F; // 5 bits mais baixos
                    if (bankNum != m_chrBank1FD)
                    {
                        m_chrBank1FD = bankNum;
                        LOG_INFO("Mapper9: Banco CHR1 (FD) selecionado: %d", m_chrBank1FD);
                    }
                }
                else if (address >= 0xE000 && address <= 0xEFFF)
                {
                    // $E000-$EFFF: Selecionar banco CHR de 4KB para $1000 quando latch 1 = $FE
                    uint8_t bankNum = data & 0x1F; // 5 bits mais baixos
                    if (bankNum != m_chrBank1FE)
                    {
                        m_chrBank1FE = bankNum;
                        LOG_INFO("Mapper9: Banco CHR1 (FE) selecionado: %d", m_chrBank1FE);
                    }
                }
                else // address >= 0xF000 && address <= 0xFFFF
                {
                    // $F000-$FFFF: Controle de espelhamento
                    bool mirroring = (data & 0x01) != 0; // Bit 0: 0 = horizontal, 1 = vertical

                    MirrorMode newMode = mirroring ? MirrorMode::VERTICAL : MirrorMode::HORIZONTAL;

                    if (newMode != m_mirrorMode)
                    {
                        m_mirrorMode = newMode;
                        LOG_INFO("Mapper9: Espelhamento alterado para %s",
                                 m_mirrorMode == MirrorMode::VERTICAL ? "Vertical" : "Horizontal");
                    }
                }
            }

            uint8_t Mapper9::ppuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper9: Tentativa de leitura PPU fora do intervalo: 0x%04X", address);
                    return 0;
                }

                // No MMC2, há comportamento especial para detecção de padrões específicos
                // Os latches são alterados quando certos endereços são lidos

                // Detectar leitura de tile especial para alteração dos latches
                if ((address & 0x0FF8) == 0x0FD8)
                {
                    // PPU está acessando o pattern $0FD8-$0FDF (ou múltiplos)
                    if (m_latch0 != 0xFD)
                    {
                        m_latch0 = 0xFD;
                        LOG_INFO("Mapper9: Latch 0 alterado para FD");
                    }
                }
                else if ((address & 0x0FF8) == 0x0FE8)
                {
                    // PPU está acessando o pattern $0FE8-$0FEF (ou múltiplos)
                    if (m_latch0 != 0xFE)
                    {
                        m_latch0 = 0xFE;
                        LOG_INFO("Mapper9: Latch 0 alterado para FE");
                    }
                }
                else if ((address & 0x1FF8) == 0x1FD8)
                {
                    // PPU está acessando o pattern $1FD8-$1FDF (ou múltiplos)
                    if (m_latch1 != 0xFD)
                    {
                        m_latch1 = 0xFD;
                        LOG_INFO("Mapper9: Latch 1 alterado para FD");
                    }
                }
                else if ((address & 0x1FF8) == 0x1FE8)
                {
                    // PPU está acessando o pattern $1FE8-$1FEF (ou múltiplos)
                    if (m_latch1 != 0xFE)
                    {
                        m_latch1 = 0xFE;
                        LOG_INFO("Mapper9: Latch 1 alterado para FE");
                    }
                }

                // Determinar o banco CHR a ser usado com base nos latches
                uint8_t chrBank;
                if (address < 0x1000)
                {
                    // $0000-$0FFF - Usar banco baseado no latch 0
                    chrBank = (m_latch0 == 0xFD) ? m_chrBank0FD : m_chrBank0FE;
                }
                else
                {
                    // $1000-$1FFF - Usar banco baseado no latch 1
                    chrBank = (m_latch1 == 0xFD) ? m_chrBank1FD : m_chrBank1FE;
                }

                // Calcular endereço final na CHR-ROM
                uint32_t bankOffset = chrBank * CHR_BANK_SIZE;
                uint32_t addr = bankOffset + (address & 0x0FFF); // Máscara para 4KB

                // Verificar limites
                if (addr >= m_chrRomSize)
                {
                    LOG_WARNING("Mapper9: Endereço CHR fora dos limites: 0x%06X (máximo: 0x%06X)",
                                addr, m_chrRomSize - 1);
                    addr %= m_chrRomSize;
                }

                return m_cartridge->chr_rom[addr];
            }

            void Mapper9::ppuWrite(uint16_t address, uint8_t data)
            {
                // O MMC2 não possui CHR-RAM, então todas as escritas são ignoradas
                LOG_WARNING("Mapper9: Tentativa de escrita em CHR-ROM: 0x%04X = 0x%02X", address, data);
            }

            void Mapper9::scanline()
            {
                // MMC2 não suporta IRQs, nada a fazer
            }

            bool Mapper9::irqState()
            {
                // MMC2 não suporta IRQs
                return false;
            }

            void Mapper9::irqClear()
            {
                // MMC2 não suporta IRQs, nada a fazer
            }
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu
