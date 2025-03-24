/**
 * @file cartridge.hpp
 * @brief Arquivo de redirecionamento para nes_cartridge.hpp
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef CARTRIDGE_HPP
#define CARTRIDGE_HPP

#include "nes_cartridge.hpp"

// Definir um alias para a classe Cartridge baseado na implementação NESCartridge
namespace MegaEmu
{
    namespace Platforms
    {
        namespace NES
        {
            // Alias Cartridge para NESCartridge para compatibilidade
            using Cartridge = NESCartridge;
        }
    }
}

// Usando o alias no namespace global para compatibilidade com arquivos existentes
using Cartridge = MegaEmu::Platforms::NES::NESCartridge;

#endif // CARTRIDGE_HPP
