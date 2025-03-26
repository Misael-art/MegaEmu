/**
 * @file m68k_adapter.h
 * @brief Adaptador para o processador Motorola 68000 do Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef MEGADRIVE_M68K_ADAPTER_H
#define MEGADRIVE_M68K_ADAPTER_H

#include "core/emu_cpu.h"
#include <stdbool.h>
#include <stdint.h>

// Constantes
#define MD_M68K_RAM_SIZE 0x10000       // 64KB
#define MD_M68K_ROM_BANK_SIZE 0x200000 // 2MB
#define MD_M68K_MAX_ROM_BANKS 32       // Máximo de 64MB

// Tipos de interrupção
typedef enum {
  MD_M68K_INT_VBLANK = 6, // Interrupção de VBLANK (nível 6)
  MD_M68K_INT_HBLANK = 4, // Interrupção de HBLANK (nível 4)
  MD_M68K_INT_Z80 = 2     // Interrupção do Z80 (nível 2)
} md_m68k_interrupt_t;

// Callbacks de acesso à memória
typedef uint16_t (*md_m68k_read_callback_t)(uint32_t address, void *user_data);
typedef void (*md_m68k_write_callback_t)(uint32_t address, uint16_t value,
                                         void *user_data);

// Estrutura de contexto
typedef struct {
  // Estado do processador
  uint32_t pc;            // Program Counter
  uint32_t registers[16]; // D0-D7/A0-A7
  uint16_t sr;            // Status Register
  bool stopped;           // Flag de STOP

  // Memória
  uint8_t *ram;      // RAM principal
  uint8_t *rom;      // ROM atual
  uint32_t rom_size; // Tamanho da ROM
  uint8_t rom_banks[MD_M68K_MAX_ROM_BANKS]
                   [MD_M68K_ROM_BANK_SIZE]; // Bancos de ROM
  uint8_t current_bank;                     // Banco atual

  // Callbacks
  md_m68k_read_callback_t read_callback;
  md_m68k_write_callback_t write_callback;
  void *callback_data;

  // Estado de interrupção
  uint8_t interrupt_level; // Nível atual de interrupção
  bool interrupt_pending;  // Flag de interrupção pendente

  // Ciclos
  uint32_t cycles;        // Ciclos executados
  uint32_t target_cycles; // Ciclos alvo
} megadrive_m68k_context_t;

// Funções de criação/destruição
emu_cpu_interface_t *megadrive_m68k_adapter_create(void);
void megadrive_m68k_adapter_destroy(emu_cpu_interface_t *cpu);

// Funções de contexto
megadrive_m68k_context_t *megadrive_m68k_get_context(emu_cpu_interface_t *cpu);
void megadrive_m68k_set_context(emu_cpu_interface_t *cpu,
                                megadrive_m68k_context_t *context);

// Funções de memória
void m68k_set_memory_callbacks(megadrive_m68k_context_t *ctx,
                               md_m68k_read_callback_t read_cb,
                               md_m68k_write_callback_t write_cb,
                               void *user_data);

void m68k_load_rom(megadrive_m68k_context_t *ctx, const uint8_t *data,
                   uint32_t size);
void m68k_set_rom_bank(megadrive_m68k_context_t *ctx, uint8_t bank);

// Funções de interrupção
void m68k_trigger_interrupt(megadrive_m68k_context_t *ctx,
                            md_m68k_interrupt_t level);
void m68k_clear_interrupt(megadrive_m68k_context_t *ctx,
                          md_m68k_interrupt_t level);

// Funções de estado
uint32_t m68k_get_pc(const megadrive_m68k_context_t *ctx);
uint32_t m68k_get_register(const megadrive_m68k_context_t *ctx, uint8_t reg);
uint16_t m68k_get_sr(const megadrive_m68k_context_t *ctx);
bool m68k_is_stopped(const megadrive_m68k_context_t *ctx);

#endif // MEGADRIVE_M68K_ADAPTER_H
