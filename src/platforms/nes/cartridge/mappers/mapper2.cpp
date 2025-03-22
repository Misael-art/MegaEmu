/**
 * @file mapper2.cpp
 * @brief Implementação do Mapper 2 (UNROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include "mapper2.hpp"
#include "../../../../utils/log_utils.h"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            Mapper2::Mapper2(Cartridge *cartridge) : m_cartridge(cartridge),
                                                     m_prgRomSize(0),
                                                     m_usesChrRam(false),
                                                     m_prgBank(0),
                                                     m_lastPrgBank(0)
            {
                // Verificar se o cartucho é válido
                if (!cartridge)
                {
                    LOG_ERROR("Mapper2: Cartucho inválido");
                    return;
                }

                // Obter tamanho da PRG-ROM
                m_prgRomSize = cartridge->prg_rom_size;

                // Verificar tamanho
                if (m_prgRomSize == 0)
                {
                    LOG_ERROR("Mapper2: Tamanho de PRG-ROM inválido: %u", m_prgRomSize);
                    return;
                }

                // Calcular o índice do último banco de PRG-ROM (em unidades de 16KB)
                m_lastPrgBank = (m_prgRomSize / 16384) - 1;

                // UNROM normalmente utiliza CHR-RAM, não CHR-ROM
                m_usesChrRam = (cartridge->chr_rom_size == 0);

                LOG_INFO("Mapper2 (UNROM) inicializado: PRG-ROM=%uKB, Bancos=%u, Último banco=%u, %s",
                         m_prgRomSize / 1024, m_prgRomSize / 16384, m_lastPrgBank,
                         m_usesChrRam ? "CHR-RAM" : "CHR-ROM");
            }

            Mapper2::~Mapper2()
            {
                // Nada a fazer aqui
            }

            void Mapper2::reset()
            {
                // Reset para valores iniciais
                m_prgBank = 0;

                LOG_INFO("Mapper2 (UNROM) resetado");
            }

            uint8_t Mapper2::cpuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x8000)
                {
                    LOG_WARNING("Mapper2: Tentativa de leitura fora do intervalo: 0x%04X", address);
                    return 0;
                }

                uint32_t addr;

                if (address < 0xC000)
                {
                    // Área inferior ($8000-$BFFF) - Banco selecionável
                    uint32_t bankOffset = m_prgBank * 16384; // 16KB por banco
                    addr = bankOffset + (address - 0x8000);
                }
                else
                {
                    // Área superior ($C000-$FFFF) - Último banco fixo
                    uint32_t bankOffset = m_lastPrgBank * 16384; // 16KB por banco
                    addr = bankOffset + (address - 0xC000);
                }

                // Verificar limites
                if (addr >= m_prgRomSize)
                {
                    LOG_WARNING("Mapper2: Endereço fora dos limites: 0x%06X (máximo: 0x%06X)",
                                addr, m_prgRomSize - 1);
                    addr %= m_prgRomSize;
                }

                return m_cartridge->prg_rom[addr];
            }

            void Mapper2::cpuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x8000)
                {
                    LOG_WARNING("Mapper2: Tentativa de escrita fora do intervalo: 0x%04X = 0x%02X", address, data);
                    return;
                }

                // No UNROM, a escrita em qualquer endereço de $8000-$FFFF
                // configura o banco de PRG-ROM para a área inferior

                // Normalmente, apenas os 3 ou 4 bits mais baixos são usados
                // Aqui estamos usando uma máscara para garantir que não selecionemos
                // um banco que não existe
                uint8_t maxBanks = m_prgRomSize / 16384;
                uint8_t newBank = data & 0x0F; // Usar 4 bits (até 16 bancos)

                // Verificar se o banco é válido
                if (newBank >= maxBanks)
                {
                    LOG_WARNING("Mapper2: Tentativa de selecionar banco PRG inválido: %u (máximo: %u)",
                                newBank, maxBanks - 1);
                    newBank %= maxBanks;
                }

                // Atualizar banco se necessário
                if (newBank != m_prgBank)
                {
                    m_prgBank = newBank;
                    LOG_INFO("Mapper2: Banco PRG selecionado: %d", m_prgBank);
                }
            }

            uint8_t Mapper2::ppuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper2: Tentativa de leitura PPU fora do intervalo: 0x%04X", address);
                    return 0;
                }

                if (m_usesChrRam)
                {
                    // Usar CHR-RAM
                    return m_cartridge->chr_ram[address];
                }
                else
                {
                    // Usar CHR-ROM
                    if (address >= m_cartridge->chr_rom_size)
                    {
                        LOG_WARNING("Mapper2: Endereço CHR fora dos limites: 0x%04X (máximo: 0x%04X)",
                                    address, m_cartridge->chr_rom_size - 1);
                        address %= m_cartridge->chr_rom_size;
                    }
                    return m_cartridge->chr_rom[address];
                }
            }

            void Mapper2::ppuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper2: Tentativa de escrita PPU fora do intervalo: 0x%04X = 0x%02X", address, data);
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
                    LOG_WARNING("Mapper2: Tentativa de escrita em CHR-ROM: 0x%04X = 0x%02X", address, data);
                }
            }

            void Mapper2::scanline()
            {
                // UNROM não suporta IRQs, nada a fazer
            }

            bool Mapper2::irqState()
            {
                // UNROM não suporta IRQs
                return false;
            }

            void Mapper2::irqClear()
            {
                // UNROM não suporta IRQs, nada a fazer
            }
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu
