/**
 * @file z80_adapter.h
 * @brief Adaptador do Z80 para o Mega Drive
 *
 * Este arquivo contém a interface do adaptador do Z80 para o Mega Drive,
 * que conecta a implementação base do Z80 com o sistema do Mega Drive.
 */

#ifndef MD_Z80_ADAPTER_H
#define MD_Z80_ADAPTER_H

#include "../../../core/interfaces/audio_interface.h"
#include "../../../core/interfaces/cpu_interface.h"
#include "../../../core/interfaces/memory_interface.h"
#include "../../../core/interfaces/state_interface.h"
#include <stdbool.h>
#include <stdint.h>

// Forward declarations
struct md_z80_adapter_s;
typedef struct md_z80_adapter_s *md_z80_adapter_t;

// Estrutura do adaptador Z80
struct md_z80_adapter_s {
  emu_cpu_t cpu;       /**< CPU Z80 */
  emu_memory_t memory; /**< Sistema de memória */
  emu_audio_t audio;   /**< Sistema de áudio */
  bool is_reset;       /**< Flag de reset */
  bool is_busreq;      /**< Flag de bus request */
  uint16_t bank;       /**< Banco de memória atual */
};

// Funções de criação/destruição
md_z80_adapter_t md_z80_adapter_create(void);
void md_z80_adapter_destroy(md_z80_adapter_t adapter);

// Funções de controle
void md_z80_adapter_reset(md_z80_adapter_t adapter);
void md_z80_adapter_set_reset(md_z80_adapter_t adapter, bool reset);
void md_z80_adapter_set_busreq(md_z80_adapter_t adapter, bool busreq);
void md_z80_adapter_set_bank(md_z80_adapter_t adapter, uint16_t bank);

// Funções de conexão
void md_z80_adapter_connect_memory(md_z80_adapter_t adapter,
                                   emu_memory_t memory);
void md_z80_adapter_connect_audio(md_z80_adapter_t adapter, emu_audio_t audio);

// Funções de execução
uint32_t md_z80_adapter_run_cycles(md_z80_adapter_t adapter, uint32_t cycles);

// Funções de estado
bool md_z80_adapter_get_reset(md_z80_adapter_t adapter);
bool md_z80_adapter_get_busreq(md_z80_adapter_t adapter);
uint16_t md_z80_adapter_get_bank(md_z80_adapter_t adapter);

// Funções de save state
void md_z80_adapter_save_state(md_z80_adapter_t adapter, void *state);
void md_z80_adapter_register_save_state(md_z80_adapter_t adapter,
                                        emu_state_t *state);

#endif /* MD_Z80_ADAPTER_H */
