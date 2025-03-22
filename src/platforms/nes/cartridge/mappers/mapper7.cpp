/**
 * @file mapper7.cpp
 * @brief Implementação do Mapper 7 (AxROM) para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#include "mapper7.hpp"
#include "../../../../utils/log_utils.h"

namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            Mapper7::Mapper7(Cartridge *cartridge) : m_cartridge(cartridge),
                                                     m_prgRomSize(0),
                                                     m_prgBank(0),
                                                     m_oneScreenMirror(0)
            {
                // Verificar se o cartucho é válido
                if (!cartridge)
                {
                    LOG_ERROR("Mapper7: Cartucho inválido");
                    return;
                }

                // Obter tamanho da PRG-ROM
                m_prgRomSize = cartridge->prg_rom_size;

                // Verificar tamanho
                if (m_prgRomSize == 0)
                {
                    LOG_ERROR("Mapper7: Tamanho de PRG-ROM inválido: %u", m_prgRomSize);
                    return;
                }

                // AxROM utiliza CHR-RAM, não CHR-ROM
                if (cartridge->chr_rom_size > 0)
                {
                    LOG_WARNING("Mapper7: Este mapper normalmente não usa CHR-ROM, mas CHR-RAM");
                }

                LOG_INFO("Mapper7 (AxROM) inicializado: PRG-ROM=%uKB, Bancos=%u",
                         m_prgRomSize / 1024, m_prgRomSize / 32768);
            }

            Mapper7::~Mapper7()
            {
                // Nada a fazer aqui
            }

            void Mapper7::reset()
            {
                // Reset para valores iniciais
                m_prgBank = 0;
                m_oneScreenMirror = 0;

                LOG_INFO("Mapper7 (AxROM) resetado");
            }

            uint8_t Mapper7::cpuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x8000)
                {
                    LOG_WARNING("Mapper7: Tentativa de leitura fora do intervalo: 0x%04X", address);
                    return 0;
                }

                // AxROM mapeia bancos de 32KB para todo o espaço de $8000-$FFFF
                uint32_t bankOffset = m_prgBank * 32768; // 32KB por banco
                uint32_t addr = bankOffset + (address - 0x8000);

                // Verificar limites
                if (addr >= m_prgRomSize)
                {
                    LOG_WARNING("Mapper7: Endereço fora dos limites: 0x%06X (máximo: 0x%06X)",
                                addr, m_prgRomSize - 1);
                    addr %= m_prgRomSize;
                }

                return m_cartridge->prg_rom[addr];
            }

            void Mapper7::cpuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address < 0x8000)
                {
                    LOG_WARNING("Mapper7: Tentativa de escrita fora do intervalo: 0x%04X = 0x%02X", address, data);
                    return;
                }

                // No AxROM, a escrita em qualquer endereço de $8000-$FFFF
                // configura o banco de PRG-ROM e o espelhamento

                // Bits 0-2: Seleção do banco de PRG-ROM (0-7)
                // Mais bancos (até 16) se o cartucho suportar
                uint8_t newPrgBank = data & 0x07;

                // Bit 4: Seleção da nametable (0 = $2000, 1 = $2400)
                uint8_t newMirror = (data & 0x10) >> 4;

                // Registrar mudanças
                if (newPrgBank != m_prgBank)
                {
                    m_prgBank = newPrgBank;
                    LOG_INFO("Mapper7: Banco PRG selecionado: %d", m_prgBank);
                }

                if (newMirror != m_oneScreenMirror)
                {
                    m_oneScreenMirror = newMirror;
                    LOG_INFO("Mapper7: Espelhamento alterado para nametable %s",
                             m_oneScreenMirror ? "B ($2400)" : "A ($2000)");
                }
            }

            uint8_t Mapper7::ppuRead(uint16_t address)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper7: Tentativa de leitura PPU fora do intervalo: 0x%04X", address);
                    return 0;
                }

                // AxROM usa CHR-RAM
                return m_cartridge->chr_ram[address];
            }

            void Mapper7::ppuWrite(uint16_t address, uint8_t data)
            {
                // Verificar se o endereço está no intervalo correto
                if (address >= 0x2000)
                {
                    LOG_WARNING("Mapper7: Tentativa de escrita PPU fora do intervalo: 0x%04X = 0x%02X", address, data);
                    return;
                }

                // Escrever na CHR-RAM
                m_cartridge->chr_ram[address] = data;
            }

            void Mapper7::scanline()
            {
                // AxROM não suporta IRQs, nada a fazer
            }

            bool Mapper7::irqState()
            {
                // AxROM não suporta IRQs
                return false;
            }

            void Mapper7::irqClear()
            {
                // AxROM não suporta IRQs, nada a fazer
            }
        } // namespace NES
    } // namespace Platforms
} // namespace MegaEmu
