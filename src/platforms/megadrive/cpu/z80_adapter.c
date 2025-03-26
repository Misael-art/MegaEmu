/**
 * @file z80_adapter.c
 * @brief Implementação do adaptador Z80 para o Mega Drive
 */

#include "z80_adapter.h"
#include "../../../core/cpu/z80/z80.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o Z80 Mega Drive
#define EMU_LOG_CAT_MD_Z80 EMU_LOG_CAT_MEGADRIVE

// Endereços do espaço Z80 no Mega Drive
#define MD_Z80_RAM_START 0x0000
#define MD_Z80_RAM_END 0x1FFF
#define MD_Z80_RAM_SIZE 0x2000   // 8KB
#define MD_Z80_YM2612 0x4000     // Registradores do YM2612
#define MD_Z80_BANK_REG 0x6000   // Registrador de banco
#define MD_Z80_PSG 0x7F11        // Porta do PSG
#define MD_Z80_BANK_START 0x8000 // Início do banco de ROM

/**
 * @brief Cria uma nova instância do adaptador Z80 para o Mega Drive
 */
md_z80_adapter_t md_z80_adapter_create(void) {
  md_z80_adapter_t adapter =
      (md_z80_adapter_t)calloc(1, sizeof(struct md_z80_adapter_s));
  if (!adapter) {
    return NULL;
  }

  // Configurar valores iniciais
  adapter->is_reset = true;
  adapter->is_busreq = true;
  adapter->bank = 0;

  return adapter;
}

/**
 * @brief Conecta o sistema de memória ao Z80
 */
void md_z80_adapter_connect_memory(md_z80_adapter_t adapter,
                                   emu_memory_t memory) {
  if (adapter) {
    adapter->memory = memory;
  }
}

/**
 * @brief Conecta o sistema de áudio ao Z80
 */
void md_z80_adapter_connect_audio(md_z80_adapter_t adapter, emu_audio_t audio) {
  if (adapter) {
    adapter->audio = audio;
  }
}

/**
 * @brief Destrói uma instância do adaptador Z80 e libera recursos
 */
void md_z80_adapter_destroy(md_z80_adapter_t adapter) {
  if (adapter) {
    free(adapter);
  }
}

/**
 * @brief Reseta o Z80 para o estado inicial
 */
void md_z80_adapter_reset(md_z80_adapter_t adapter) {
  if (adapter) {
    adapter->is_reset = true;
    adapter->is_busreq = true;
    adapter->bank = 0;
  }
}

/**
 * @brief Define o estado de reset do Z80
 */
void md_z80_adapter_set_reset(md_z80_adapter_t adapter, bool reset) {
  if (adapter) {
    adapter->is_reset = reset;
  }
}

/**
 * @brief Define o estado de bus request do Z80
 */
void md_z80_adapter_set_busreq(md_z80_adapter_t adapter, bool busreq) {
  if (adapter) {
    adapter->is_busreq = busreq;
  }
}

/**
 * @brief Obtém o estado atual de bus request
 */
bool md_z80_adapter_get_busreq(md_z80_adapter_t adapter) {
  return adapter ? adapter->is_busreq : true;
}

/**
 * @brief Obtém o estado atual de reset
 */
bool md_z80_adapter_get_reset(md_z80_adapter_t adapter) {
  return adapter ? adapter->is_reset : true;
}

/**
 * @brief Define o banco de memória do Z80
 */
void md_z80_adapter_set_bank(md_z80_adapter_t adapter, uint16_t bank) {
  if (adapter) {
    adapter->bank = bank;
  }
}

/**
 * @brief Obtém o banco de memória atual do Z80
 */
uint16_t md_z80_adapter_get_bank(md_z80_adapter_t adapter) {
  return adapter ? adapter->bank : 0;
}

/**
 * @brief Executa um número específico de ciclos no Z80
 */
uint32_t md_z80_adapter_run_cycles(md_z80_adapter_t adapter, uint32_t cycles) {
  if (!adapter || adapter->is_reset || adapter->is_busreq) {
    return 0;
  }

  // TODO: Implementar emulação completa do Z80
  return cycles;
}

/**
 * @brief Salva o estado atual do Z80 para um save state
 */
void md_z80_adapter_save_state(md_z80_adapter_t adapter, void *state) {
  if (adapter && state) {
    // TODO: Implementar salvamento de estado para compatibilidade
  }
}

/**
 * @brief Registra callbacks para salvar estado do Z80
 */
void md_z80_adapter_register_save_state(md_z80_adapter_t adapter,
                                        emu_state_t *state) {
  if (!adapter || !state) {
    return;
  }

  // Criar uma área para salvar o estado do Z80
  size_t z80_state_size = sizeof(struct md_z80_adapter_s);
  uint8_t *z80_state_data = (uint8_t *)calloc(1, z80_state_size);

  if (!z80_state_data) {
    return;
  }

  // Copiar o estado atual do Z80
  memcpy(z80_state_data, adapter, z80_state_size);

  // Aqui seria necessário adicionar o estado ao emu_state_t
  // Na implementação completa, você adicionaria ao state->sections ou similar
  // mas isso depende da implementação específica do sistema de save state

  // Por enquanto, apenas liberamos a memória alocada
  free(z80_state_data);
}
