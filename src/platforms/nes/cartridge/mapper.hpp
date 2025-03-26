/**
 * @file mapper.hpp
 * @brief Definição da classe base Mapper para o NES
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-04-30
 */

#ifndef MAPPER_HPP
#define MAPPER_HPP

#include <cstdint>

/**
 * @brief Guia de Arquitetura Híbrida para Mappers
 *
 * De acordo com as diretrizes da arquitetura híbrida do Mega_Emu:
 * 1. As implementações de mappers devem ser feitas em C++ para aproveitar
 * polimorfismo
 * 2. Wrappers em C devem ser criados para compatibilidade com código core
 * 3. A interface C deve seguir snake_case e convenções C
 * 4. Código original deve ser mantido em C++
 *
 * Cada mapper específico deve:
 * - Herdar desta classe base (Mapper)
 * - Implementar todos os métodos virtuais
 * - Ter um wrapper em C correspondente se necessário para integração
 *
 * Exemplo de integração:
 *
 * No arquivo C:
 * ```c
 * // Implementação opaca que esconde os detalhes C++
 * typedef struct nes_mapper_s nes_mapper_t;
 *
 * nes_mapper_t* nes_mapper_create(int mapper_id, uint8_t* rom_data, size_t
 * rom_size); void nes_mapper_destroy(nes_mapper_t* mapper); uint8_t
 * nes_mapper_cpu_read(nes_mapper_t* mapper, uint16_t address);
 * ```
 *
 * No arquivo CPP de implementação:
 * ```cpp
 * #include "mapper.hpp"
 * #include "nes_mapper.h" // Interface C
 *
 * // Implementações de wrapper
 * nes_mapper_t* nes_mapper_create(int mapper_id, uint8_t* rom_data, size_t
 * rom_size) { Mapper* mapper_impl = nullptr; switch (mapper_id) { case 0:
 * mapper_impl = new Mapper0(rom_data, rom_size); break;
 *         // Outros mappers
 *     }
 *     return reinterpret_cast<nes_mapper_t*>(mapper_impl);
 * }
 * ```
 */

// Forward declarations
class Cartridge;

/**
 * @class Mapper
 * @brief Classe base para implementação de mappers do NES
 *
 * Esta classe define a interface comum para todos os mappers do NES,
 * permitindo que diferentes implementações de mappers sejam utilizadas
 * de forma transparente pelo sistema.
 */
class Mapper {
public:
  /**
   * @brief Destrutor virtual para permitir destruição adequada de classes
   * derivadas
   */
  virtual ~Mapper() = default;

  /**
   * @brief Reseta o estado do mapper
   */
  virtual void reset() = 0;

  /**
   * @brief Lê um byte da CPU
   * @param address Endereço de memória (0x8000-0xFFFF)
   * @return Byte lido
   */
  virtual uint8_t cpuRead(uint16_t address) = 0;

  /**
   * @brief Escreve um byte pela CPU
   * @param address Endereço de memória (0x8000-0xFFFF)
   * @param data Byte a ser escrito
   */
  virtual void cpuWrite(uint16_t address, uint8_t data) = 0;

  /**
   * @brief Lê um byte da PPU (CHR)
   * @param address Endereço de memória (0x0000-0x1FFF)
   * @return Byte lido
   */
  virtual uint8_t ppuRead(uint16_t address) = 0;

  /**
   * @brief Escreve um byte pela PPU (CHR)
   * @param address Endereço de memória (0x0000-0x1FFF)
   * @param data Byte a ser escrito
   */
  virtual void ppuWrite(uint16_t address, uint8_t data) = 0;

  /**
   * @brief Notifica o mapper sobre uma nova scanline para IRQ
   */
  virtual void scanline() = 0;

  /**
   * @brief Retorna o estado do sinal de IRQ
   * @return true se IRQ está ativo, false caso contrário
   */
  virtual bool irqState() { return false; }

  /**
   * @brief Limpa o sinal de IRQ
   */
  virtual void irqClear() {}

  /**
   * @brief Salva o estado do mapper
   * @param state Objeto de estado para salvar
   * @return true se bem-sucedido, false caso contrário
   */
  virtual bool saveState(void *state) { return true; }

  /**
   * @brief Carrega o estado do mapper
   * @param state Objeto de estado para carregar
   * @return true se bem-sucedido, false caso contrário
   */
  virtual bool loadState(void *state) { return true; }
};

#endif // MAPPER_HPP
