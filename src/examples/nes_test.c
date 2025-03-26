#include "core/cpu/6502/cpu_6502.h"
#include "core/video/ppu_2c02/ppu_2c02.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Memória do sistema
static uint8_t ram[0x800];       // 2KB de RAM
static uint8_t ppu_registers[8]; // Registradores da PPU ($2000-$2007)
static uint8_t rom[0x8000];      // ROM (32KB)

// Interfaces
static emu_cpu_interface_t *cpu;
static emu_ppu_interface_t *ppu;

// Funções de acesso à memória
static uint8_t read_memory(void *ctx, uint32_t addr) {
  (void)ctx;

  if (addr < 0x2000) {
    // RAM com espelhamento
    return ram[addr & 0x7FF];
  } else if (addr < 0x4000) {
    // Registradores da PPU com espelhamento
    uint8_t reg = addr & 7;
    if (reg == EMU_2C02_REG_PPUSTATUS) {
      return ppu->read_register(ppu->context, reg);
    }
    return ppu_registers[reg];
  } else if (addr >= 0x8000) {
    // ROM
    return rom[addr - 0x8000];
  }

  return 0;
}

static void write_memory(void *ctx, uint32_t addr, uint8_t val) {
  (void)ctx;

  if (addr < 0x2000) {
    // RAM com espelhamento
    ram[addr & 0x7FF] = val;
  } else if (addr < 0x4000) {
    // Registradores da PPU com espelhamento
    uint8_t reg = addr & 7;
    ppu_registers[reg] = val;
    ppu->write_register(ppu->context, reg, val);
  }
}

int main(void) {
  printf("Teste de integração CPU 6502 + PPU 2C02\n\n");

  // Cria as interfaces
  cpu = emu_cpu_6502_create();
  ppu = emu_ppu_2c02_create();

  if (!cpu || !ppu) {
    printf("Erro ao criar interfaces!\n");
    return 1;
  }

  // Inicializa os componentes
  cpu->init(cpu->context);
  ppu->init(ppu->context);

  // Conecta a memória à CPU
  emu_6502_context_t *cpu_ctx = (emu_6502_context_t *)cpu->context;
  cpu_ctx->memory = cpu;

  // Carrega um programa de teste na ROM
  // Este programa escreve valores nos registradores da PPU
  uint8_t test_program[] = {
      0xA9, 0x80,       // LDA #$80   ; Habilita NMI
      0x8D, 0x00, 0x20, // STA $2000  ; Escreve no PPUCTRL
      0xA9, 0x18,       // LDA #$18   ; Habilita background e sprites
      0x8D, 0x01, 0x20, // STA $2001  ; Escreve no PPUMASK
      0x4C, 0x06, 0x80  // JMP $8006  ; Loop infinito
  };

  memcpy(&rom[6], test_program, sizeof(test_program));
  rom[0x7FFC] = 0x06; // Reset vector low byte
  rom[0x7FFD] = 0x80; // Reset vector high byte

  // Reset dos componentes
  cpu->reset(cpu->context);
  ppu->reset(ppu->context);

  printf("Executando programa de teste...\n");

  // Executa alguns frames
  for (int frame = 0; frame < 10; frame++) {
    // Um frame NES = 89342 ciclos de PPU = 29781 ciclos de CPU
    for (int cpu_cycle = 0; cpu_cycle < 29781; cpu_cycle++) {
      cpu->execute(cpu->context, 1);
      ppu->execute(ppu->context, 3); // PPU é 3x mais rápida

      // Verifica NMI
      emu_2c02_context_t *ppu_ctx = (emu_2c02_context_t *)ppu->context;
      if (ppu_ctx->nmi_occurred) {
        cpu->trigger_nmi(cpu->context);
        ppu_ctx->nmi_occurred = 0;
      }
    }

    // Status ao final do frame
    emu_cpu_state_t cpu_state;
    emu_ppu_state_t ppu_state;

    cpu->get_state(cpu->context, &cpu_state);
    ppu->get_state(ppu->context, &ppu_state);

    printf("\nFrame %d:\n", frame + 1);
    printf("CPU - PC: $%04X, Ciclos: %u\n",
           cpu->get_register(cpu->context, EMU_6502_REG_PC), cpu_state.cycles);
    printf("PPU - Scanline: %u, Frame: %u, Flags: 0x%02X\n", ppu_state.scanline,
           ppu_state.frame, ppu_state.flags);
  }

  // Cleanup
  cpu->shutdown(cpu->context);
  ppu->shutdown(ppu->context);
  free(cpu);
  free(ppu);

  printf("\nTeste concluído!\n");
  return 0;
}
