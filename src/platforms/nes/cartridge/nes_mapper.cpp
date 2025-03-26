/**
 * @file nes_mapper.cpp
 * @brief Implementação da interface C para os mappers NES
 *
 * Este arquivo serve como ponte entre a interface C (nes_mapper.h) e a
 * implementação C++ (mapper.hpp e suas derivadas) seguindo as diretrizes
 * da arquitetura híbrida.
 */

#include "nes_mapper.h"
#include "mapper.hpp"
#include <new>
#include <memory>

// Forward declarations das classes específicas de mapper
class Mapper0;
class Mapper1;
class Mapper2;
class Mapper3;
class Mapper4;

// Hack para acessar a implementação C++ através da interface C
struct nes_mapper_s {
    Mapper* impl; // Ponteiro para a implementação real
};

/**
 * @brief Cria um novo mapper do tipo especificado
 */
nes_mapper_t* nes_mapper_create(int mapper_id, const uint8_t* rom_data, size_t prg_size, size_t chr_size) {
    if (!rom_data)
        return nullptr;

    // Aloca a estrutura wrapper
    auto* wrapper = new (std::nothrow) nes_mapper_s();
    if (!wrapper)
        return nullptr;

    // Cria a implementação apropriada baseada no ID do mapper
    Mapper* mapper_impl = nullptr;

    try {
        switch (mapper_id) {
            case NES_MAPPER_NROM:
                // Aqui faríamos um new Mapper0(...) com os parâmetros apropriados
                // Para simplicidade, apenas um placeholder
                // mapper_impl = new Mapper0(rom_data, prg_size, chr_size);
                break;
            case NES_MAPPER_MMC1:
                // mapper_impl = new Mapper1(rom_data, prg_size, chr_size);
                break;
            case NES_MAPPER_UNROM:
                // mapper_impl = new Mapper2(rom_data, prg_size, chr_size);
                break;
            case NES_MAPPER_CNROM:
                // mapper_impl = new Mapper3(rom_data, prg_size, chr_size);
                break;
            case NES_MAPPER_MMC3:
                // mapper_impl = new Mapper4(rom_data, prg_size, chr_size);
                break;
            // Adicionar mais mappers conforme necessário
            default:
                // Mapper desconhecido
                delete wrapper;
                return nullptr;
        }
    } catch (const std::exception&) {
        delete wrapper;
        return nullptr;
    }

    if (!mapper_impl) {
        delete wrapper;
        return nullptr;
    }

    wrapper->impl = mapper_impl;
    return wrapper;
}

/**
 * @brief Destrói um mapper previamente criado
 */
void nes_mapper_destroy(nes_mapper_t* mapper) {
    if (!mapper)
        return;

    delete mapper->impl;
    delete mapper;
}

/**
 * @brief Reseta o estado do mapper
 */
void nes_mapper_reset(nes_mapper_t* mapper) {
    if (mapper && mapper->impl)
        mapper->impl->reset();
}

/**
 * @brief Lê um byte da CPU
 */
uint8_t nes_mapper_cpu_read(nes_mapper_t* mapper, uint16_t address) {
    if (!mapper || !mapper->impl)
        return 0;

    return mapper->impl->cpuRead(address);
}

/**
 * @brief Escreve um byte pela CPU
 */
void nes_mapper_cpu_write(nes_mapper_t* mapper, uint16_t address, uint8_t data) {
    if (mapper && mapper->impl)
        mapper->impl->cpuWrite(address, data);
}

/**
 * @brief Lê um byte da PPU (CHR)
 */
uint8_t nes_mapper_ppu_read(nes_mapper_t* mapper, uint16_t address) {
    if (!mapper || !mapper->impl)
        return 0;

    return mapper->impl->ppuRead(address);
}

/**
 * @brief Escreve um byte pela PPU (CHR)
 */
void nes_mapper_ppu_write(nes_mapper_t* mapper, uint16_t address, uint8_t data) {
    if (mapper && mapper->impl)
        mapper->impl->ppuWrite(address, data);
}

/**
 * @brief Notifica o mapper sobre uma nova scanline para IRQ
 */
void nes_mapper_scanline(nes_mapper_t* mapper) {
    if (mapper && mapper->impl)
        mapper->impl->scanline();
}

/**
 * @brief Verifica o estado do sinal de IRQ
 */
bool nes_mapper_irq_state(nes_mapper_t* mapper) {
    if (!mapper || !mapper->impl)
        return false;

    return mapper->impl->irqState();
}

/**
 * @brief Limpa o sinal de IRQ
 */
void nes_mapper_irq_clear(nes_mapper_t* mapper) {
    if (mapper && mapper->impl)
        mapper->impl->irqClear();
}

/**
 * @brief Salva o estado do mapper
 */
bool nes_mapper_save_state(nes_mapper_t* mapper, void* state, size_t state_size) {
    if (!mapper || !mapper->impl || !state)
        return false;

    return mapper->impl->saveState(state);
}

/**
 * @brief Carrega o estado do mapper
 */
bool nes_mapper_load_state(nes_mapper_t* mapper, const void* state, size_t state_size) {
    if (!mapper || !mapper->impl || !state)
        return false;

    return mapper->impl->loadState(const_cast<void*>(state));
}

/**
 * @brief Obtém o tipo de espelhamento de nametable do mapper
 *
 * Esta funcionalidade não está definida na interface base Mapper,
 * então precisaríamos implementá-la adequadamente em um cenário real.
 */
int nes_mapper_get_mirroring(nes_mapper_t* mapper) {
    // Implementação de exemplo - seria necessário estender a classe Mapper
    // ou fazer um cast para o tipo específico do mapper.
    return 0; // Valor padrão (horizontal mirroring)
}
