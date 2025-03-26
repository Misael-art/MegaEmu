/**
 * @file gg_memory.c
 * @brief Implementação do sistema de memória do Game Gear
 */

#include "gg_memory.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_GG_MEM EMU_LOG_CAT_MEMORY

// Macros de log
#define GG_MEM_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_GG_MEM, __VA_ARGS__)
#define GG_MEM_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_GG_MEM, __VA_ARGS__)
#define GG_MEM_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_GG_MEM, __VA_ARGS__)
#define GG_MEM_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_GG_MEM, __VA_ARGS__)
#define GG_MEM_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_GG_MEM, __VA_ARGS__)

/**
 * @brief Estrutura do sistema de memória do Game Gear
 */
struct gg_memory_t {
  uint8_t *rom_banks[GG_TOTAL_BANKS]; // Bancos de ROM
  uint8_t ram[GG_RAM_SIZE];           // RAM
  uint8_t current_bank;               // Banco atual
  uint8_t total_banks;                // Total de bancos na ROM
  bool rom_loaded;                    // Flag de ROM carregada
};

/**
 * @brief Cria uma nova instância do sistema de memória
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gg_memory_t *gg_memory_create(void) {
  gg_memory_t *mem = (gg_memory_t *)malloc(sizeof(gg_memory_t));
  if (!mem) {
    GG_MEM_LOG_ERROR("Falha ao alocar memória para o sistema de memória");
    return NULL;
  }

  // Inicializa estrutura
  memset(mem, 0, sizeof(gg_memory_t));

  GG_MEM_LOG_INFO("Sistema de memória do Game Gear criado");
  return mem;
}

/**
 * @brief Destrói uma instância do sistema de memória
 * @param mem Ponteiro para a instância
 */
void gg_memory_destroy(gg_memory_t *mem) {
  if (!mem)
    return;

  // Libera bancos de ROM
  for (int i = 0; i < GG_TOTAL_BANKS; i++) {
    if (mem->rom_banks[i]) {
      free(mem->rom_banks[i]);
    }
  }

  free(mem);
  GG_MEM_LOG_INFO("Sistema de memória do Game Gear destruído");
}

/**
 * @brief Reseta o sistema de memória
 * @param mem Ponteiro para a instância
 */
void gg_memory_reset(gg_memory_t *mem) {
  if (!mem)
    return;

  // Limpa RAM
  memset(mem->ram, 0, GG_RAM_SIZE);

  // Reseta estado
  mem->current_bank = 0;

  GG_MEM_LOG_INFO("Sistema de memória do Game Gear resetado");
}

/**
 * @brief Carrega uma ROM no sistema de memória
 * @param mem Ponteiro para a instância
 * @param data Ponteiro para os dados da ROM
 * @param size Tamanho da ROM em bytes
 * @return true se sucesso, false caso contrário
 */
bool gg_memory_load_rom(gg_memory_t *mem, const uint8_t *data, size_t size) {
  if (!mem || !data || size == 0) {
    GG_MEM_LOG_ERROR("Parâmetros inválidos para carregamento de ROM");
    return false;
  }

  // Libera ROM anterior se existir
  for (int i = 0; i < GG_TOTAL_BANKS; i++) {
    if (mem->rom_banks[i]) {
      free(mem->rom_banks[i]);
      mem->rom_banks[i] = NULL;
    }
  }

  // Calcula número de bancos necessários
  mem->total_banks = (size + GG_ROM_BANK_SIZE - 1) / GG_ROM_BANK_SIZE;
  if (mem->total_banks > GG_TOTAL_BANKS) {
    GG_MEM_LOG_ERROR("ROM muito grande: %zu bytes (%d bancos)", size,
                     mem->total_banks);
    return false;
  }

  // Aloca e copia bancos
  for (int i = 0; i < mem->total_banks; i++) {
    mem->rom_banks[i] = (uint8_t *)malloc(GG_ROM_BANK_SIZE);
    if (!mem->rom_banks[i]) {
      GG_MEM_LOG_ERROR("Falha ao alocar banco %d", i);
      return false;
    }

    // Copia dados ou preenche com 0xFF se além do tamanho
    size_t offset = i * GG_ROM_BANK_SIZE;
    size_t copy_size =
        (offset + GG_ROM_BANK_SIZE <= size) ? GG_ROM_BANK_SIZE : size - offset;

    if (copy_size > 0) {
      memcpy(mem->rom_banks[i], data + offset, copy_size);
    }
    if (copy_size < GG_ROM_BANK_SIZE) {
      memset(mem->rom_banks[i] + copy_size, 0xFF, GG_ROM_BANK_SIZE - copy_size);
    }
  }

  // Reseta estado
  mem->current_bank = 0;
  mem->rom_loaded = true;

  GG_MEM_LOG_INFO("ROM carregada: %zu bytes (%d bancos)", size,
                  mem->total_banks);
  return true;
}

/**
 * @brief Lê um byte da memória
 * @param mem Ponteiro para a instância
 * @param addr Endereço a ser lido
 * @return Byte lido
 */
uint8_t gg_memory_read(gg_memory_t *mem, uint16_t addr) {
  if (!mem)
    return 0xFF;

  // Banco 0 (fixo)
  if (addr >= GG_ROM_BANK0_START && addr <= GG_ROM_BANK0_END) {
    if (mem->rom_banks[0]) {
      return mem->rom_banks[0][addr];
    }
    return 0xFF;
  }

  // Banco 1 (fixo)
  if (addr >= GG_ROM_BANK1_START && addr <= GG_ROM_BANK1_END) {
    if (mem->rom_banks[1]) {
      return mem->rom_banks[1][addr - GG_ROM_BANK1_START];
    }
    return 0xFF;
  }

  // Banco 2 (comutável)
  if (addr >= GG_ROM_BANK2_START && addr <= GG_ROM_BANK2_END) {
    if (mem->rom_banks[mem->current_bank]) {
      return mem->rom_banks[mem->current_bank][addr - GG_ROM_BANK2_START];
    }
    return 0xFF;
  }

  // RAM e espelho
  if ((addr >= GG_RAM_START && addr <= GG_RAM_END) ||
      (addr >= GG_RAM_MIRROR_START && addr <= GG_RAM_MIRROR_END)) {
    return mem->ram[addr & 0x1FFF];
  }

  GG_MEM_LOG_WARN("Leitura de endereço inválido: %04X", addr);
  return 0xFF;
}

/**
 * @brief Escreve um byte na memória
 * @param mem Ponteiro para a instância
 * @param addr Endereço a ser escrito
 * @param value Valor a ser escrito
 */
void gg_memory_write(gg_memory_t *mem, uint16_t addr, uint8_t value) {
  if (!mem)
    return;

  // RAM e espelho
  if ((addr >= GG_RAM_START && addr <= GG_RAM_END) ||
      (addr >= GG_RAM_MIRROR_START && addr <= GG_RAM_MIRROR_END)) {
    mem->ram[addr & 0x1FFF] = value;
    return;
  }

  // Controle de banco
  if (addr >= 0xFFFC && addr <= 0xFFFF) {
    uint8_t bank = value % mem->total_banks;
    if (bank != mem->current_bank) {
      mem->current_bank = bank;
      GG_MEM_LOG_TRACE("Banco alterado para %d", bank);
    }
    return;
  }

  GG_MEM_LOG_WARN("Escrita em endereço inválido: %04X = %02X", addr, value);
}

/**
 * @brief Obtém ponteiro para a RAM
 * @param mem Ponteiro para a instância
 * @return Ponteiro para a RAM ou NULL em caso de erro
 */
uint8_t *gg_memory_get_ram(gg_memory_t *mem) {
  if (!mem)
    return NULL;
  return mem->ram;
}

/**
 * @brief Obtém ponteiro para um banco de ROM
 * @param mem Ponteiro para a instância
 * @param bank Número do banco
 * @return Ponteiro para o banco ou NULL em caso de erro
 */
const uint8_t *gg_memory_get_rom_bank(gg_memory_t *mem, uint8_t bank) {
  if (!mem || bank >= mem->total_banks)
    return NULL;
  return mem->rom_banks[bank];
}

/**
 * @brief Registra campos do sistema de memória no sistema de save state
 * @param mem Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gg_memory_register_save_state(gg_memory_t *mem, save_state_t *state) {
  if (!mem || !state)
    return -1;

  save_state_register_field(state, "gg_memory_ram", mem->ram, sizeof(mem->ram));
  save_state_register_field(state, "gg_memory_current_bank", &mem->current_bank,
                            sizeof(mem->current_bank));

  return 0;
}
