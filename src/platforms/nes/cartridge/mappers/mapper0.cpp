/**
 * @file mapper0.cpp
 * @brief Implementação do Mapper 0 (NROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include "mapper0.hpp"
#include "../../../../utils/log_utils.h"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            Mapper0::Mapper0(Cartridge *cartridge) : m_cartridge(cartridge),
                                                     m_prgRomSize(0), m_chrRomSize(0),
                                                     m_usesChrRam(false), m_isNrom128(false)
            {
                // Verificar se o cartucho é válido
                if (!cartridge)
                {
                    LOG_ERROR("Mapper0: Cartucho inválido");
                    return;
                }

                // Obter tamanhos das ROMs
                m_prgRomSize = cartridge->prg_rom_size;
                m_chrRomSize = cartridge->chr_rom_size;

                // Verificar se é NROM-128 (16KB) ou NROM-256 (32KB)
                m_isNrom128 = (m_prgRomSize == 16 * 1024);

                // Verificar se usa CHR-RAM (se CHR-ROM estiver ausente)
                m_usesChrRam = (m_chrRomSize == 0);

                LOG_INFO("Mapper0 (NROM) inicializado: %s, PRG-ROM=%uKB, %s=%uKB",
                         m_isNrom128 ? "NROM-128" : "NROM-256",
                         m_prgRomSize / 1024,
                         m_usesChrRam ? "CHR-RAM" : "CHR-ROM",
                         (m_usesChrRam ? 8 : m_chrRomSize / 1024));
            }

            Mapper0::~Mapper0()
            {
                // Nada a fazer aqui
            }

            void Mapper0::reset()
            {
                // NROM não tem registradores para resetar
                LOG_INFO("Mapper0 (NROM) resetado");
            }

            uint8_t Mapper0::cpuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x8000)
                {
                    LOG_WARNING("Mapper0: Tentativa de leitura fora do intervalo: 0x%04X", address);
                    return 0;
                }

                // Calcular o endereço na PRG-ROM
                uint32_t addr = address - 0x8000;

                // Para NROM-128, espelhar os 16KB para preencher 32KB
                if (m_isNrom128 && addr >= 0x4000)
                {
                    addr -= 0x4000; // Espelhar para os primeiros 16KB
                }

                // Verificar limites
                if (addr >= m_prgRomSize)
                {
                    LOG_WARNING("Mapper0: Endereço fora dos limites: 0x%06X (máximo: 0x%06X)",
                                addr, m_prgRomSize - 1);
                    return 0;
                }

                return m_cartridge->prg_rom[addr];
            }

            void Mapper0::cpuWrite(uint16_t address, uint8_t data)
            {
                // NROM não permite escrita na ROM
                // Alguns jogos tentam escrever na ROM por engano ou como detecção de hardware
                LOG_WARNING("Mapper0: Tentativa de escrita ignorada: 0x%04X = 0x%02X", address, data);
            }

            uint8_t Mapper0::ppuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper0: Tentativa de leitura PPU fora do intervalo: 0x%04X", address);
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
                    if (address >= m_chrRomSize)
                    {
                        LOG_WARNING("Mapper0: Endereço CHR fora dos limites: 0x%04X (máximo: 0x%04X)",
                                    address, m_chrRomSize - 1);
                        address %= m_chrRomSize;
                    }
                    return m_cartridge->chr_rom[address];
                }
            }

            void Mapper0::ppuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper0: Tentativa de escrita PPU fora do intervalo: 0x%04X = 0x%02X", address, data);
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
                    LOG_WARNING("Mapper0: Tentativa de escrita em CHR-ROM: 0x%04X = 0x%02X", address, data);
                }
            }

            void Mapper0::scanline()
            {
                // NROM não suporta IRQs, nada a fazer
            }

            bool Mapper0::irqState()
            {
                // NROM não suporta IRQs
                return false;
            }

            void Mapper0::irqClear()
            {
                // NROM não suporta IRQs, nada a fazer
            }
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu
