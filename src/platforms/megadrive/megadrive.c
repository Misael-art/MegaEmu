/**
 * @file megadrive.c
 * @brief Implementação da plataforma Mega Drive/Genesis
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../core/core.h"
#include "../../core/interfaces/memory_interface.h"
#include "../../core/interfaces/platform_interface.h"
#include "../../core/interfaces/video_interface.h"
#include "../../video/vdp.h"
#include "cpu/m68k.h"
#include "cpu/z80_adapter.h"
#include "megadrive.h"

// Redefinição só se não estiver já definido
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Definições de portas do VDP
#define VDP_PORT_DATA 0x00    // Porta de dados
#define VDP_PORT_CONTROL 0x04 // Porta de controle

// Estrutura opaca do Mega Drive
struct megadrive_t {
  md_platform_data_t platform_data;
  bool initialized;
};

// Forward declaration do tipo md_mapper_t
typedef struct md_mapper_s *md_mapper_t;

// Instância estática dos dados da plataforma
static md_platform_data_t platform_data = {0};

// Estrutura da plataforma Mega Drive
typedef struct {
    // Componentes principais
    CPU* cpu;
    PPU* ppu;

    // Memórias
    uint8_t* rom;        // Cartucho ROM
    uint8_t* ram;        // RAM principal
    uint8_t* vram;       // Video RAM
    uint8_t* z80_ram;    // Z80 RAM

    // Estado
    bool running;
    uint32_t cycles;
} MegaDrive;

// Funções de gerenciamento de memória
bool md_memory_save_sram(const char *filename) {
  (void)filename; // Para evitar aviso de parâmetro não utilizado
  return true;
}

bool md_memory_load_rom(uint8_t *rom_data, size_t rom_size) {
  if (!rom_data || rom_size == 0) {
    return false;
  }
  // Implementação futura
  return true;
}

bool md_memory_load_sram(const char *filename) {
  (void)filename; // Para evitar aviso de parâmetro não utilizado
  return true;
}

// Constantes para save states
#define SAVE_STATE_ERROR_NONE 0

// Funções de save state
void md_save_state_create(emu_platform_t *platform, emu_state_t *state) {
  if (!platform || !state || !platform->platform_data)
    return;

  md_platform_data_t *platform_data =
      (md_platform_data_t *)platform->platform_data;
  if (!platform_data->is_initialized)
    return;

  // Configurar informações básicas do estado
  strncpy(state->info.platform_id, "MD", sizeof(state->info.platform_id) - 1);
  strncpy(state->info.rom_name, platform_data->rom_header.game_title_domestic,
          sizeof(state->info.rom_name) - 1);
  state->info.state_version = 1;
  state->info.flags = EMU_STATE_FLAG_CPU | EMU_STATE_FLAG_MEMORY |
                      EMU_STATE_FLAG_VIDEO | EMU_STATE_FLAG_AUDIO;
  state->info.timestamp = time(NULL);
  state->info.emulator_version = 0x010000; // v1.0.0

  // Alocar espaço para os dados do estado
  size_t data_size = sizeof(md_platform_data_t);
  state->data = calloc(1, data_size);
  if (!state->data)
    return;

  state->data_size = data_size;

  // Salvar dados do Z80 se disponível
  if (platform_data->z80) {
    md_z80_adapter_register_save_state(platform_data->z80, state);
  }
}

bool md_save_state_save(emu_platform_t *platform, emu_state_t *state) {
  if (!platform || !state || !platform->platform_data)
    return false;

  md_platform_data_t *platform_data =
      (md_platform_data_t *)platform->platform_data;
  if (!platform_data->is_initialized || !state->data)
    return false;

  // Copiar dados da plataforma para o estado
  memcpy(state->data, platform_data, sizeof(md_platform_data_t));

  return true;
}

bool md_save_state_load(emu_platform_t *platform, const emu_state_t *state) {
  if (!platform || !state || !platform->platform_data || !state->data)
    return false;

  md_platform_data_t *platform_data =
      (md_platform_data_t *)platform->platform_data;
  if (!platform_data->is_initialized)
    return false;

  // Verificar versão e compatibilidade
  if (state->info.state_version != 1) {
    return false;
  }

  // Restaurar dados da plataforma
  memcpy(platform_data, state->data, sizeof(md_platform_data_t));

  return true;
}

bool md_save_state_config_rewind(emu_platform_t *platform, uint32_t capacity,
                                 uint32_t frames_per_snapshot) {
  (void)platform;            // Para evitar aviso de parâmetro não utilizado
  (void)capacity;            // Para evitar aviso de parâmetro não utilizado
  (void)frames_per_snapshot; // Para evitar aviso de parâmetro não utilizado
  return true;
}

bool md_save_state_capture_rewind(emu_platform_t *platform) {
  if (!platform)
    return false;

  // TODO: Implementar captura de rewind
  return false;
}

bool md_save_state_rewind(emu_platform_t *platform) {
  if (!platform)
    return false;

  // TODO: Implementar rewind
  return false;
}

// Funções principais da plataforma
emu_platform_t *emu_platform_megadrive_create(void) {
  md_platform_data_t *g_platform =
      (md_platform_data_t *)calloc(1, sizeof(md_platform_data_t));
  if (!g_platform)
    return NULL;

  // Inicialização básica
  strncpy(g_platform->name, "Sega Mega Drive", sizeof(g_platform->name) - 1);
  strncpy(g_platform->id, "MD", sizeof(g_platform->id) - 1);

  // Configurações padrão
  g_platform->cpu_clock = 7670000;   // 7.67 MHz
  g_platform->vdp_clock = 13423000;  // 13.423 MHz
  g_platform->sound_clock = 3579545; // 3.58 MHz
  g_platform->screen_width = 320;
  g_platform->screen_height = 224;
  g_platform->has_secondary_cpu = true;
  g_platform->has_color = true;
  g_platform->max_sprites = 80;
  g_platform->max_colors = 64;

  return (emu_platform_t *)g_platform;
}

// Funções de inicialização
bool md_platform_init(emu_platform_t *platform) {
  if (!platform)
    return false;

  // Inicializa a estrutura de dados
  memset(&platform_data, 0, sizeof(md_platform_data_t));

  // Aloca memória para RAM principal (64KB)
  platform_data.ram_size = 64 * 1024;
  platform_data.ram = (uint8_t *)calloc(1, platform_data.ram_size);
  if (!platform_data.ram)
    return false;

  // Aloca memória para RAM do Z80 (8KB)
  platform_data.z80_ram_size = 8 * 1024;
  platform_data.z80_ram = (uint8_t *)calloc(1, platform_data.z80_ram_size);
  if (!platform_data.z80_ram) {
    free(platform_data.ram);
    return false;
  }

  // Configura informações da plataforma
  strncpy(platform_data.name, "Sega Mega Drive",
          sizeof(platform_data.name) - 1);
  strncpy(platform_data.id, "MD", sizeof(platform_data.id) - 1);
  platform_data.cpu_clock = 7670000;   // 7.67 MHz
  platform_data.vdp_clock = 13423294;  // 13.42 MHz
  platform_data.sound_clock = 3579545; // 3.58 MHz
  platform_data.screen_width = 320;
  platform_data.screen_height = 224;
  platform_data.has_secondary_cpu = true;
  platform_data.has_color = true;
  platform_data.max_sprites = 80;
  platform_data.max_colors = 512;

  // Inicializa o sistema de memória
  platform_data.memory = emu_memory_create();
  if (!platform_data.memory) {
    free(platform_data.ram);
    free(platform_data.z80_ram);
    return false;
  }
  emu_memory_init(platform_data.memory);

  // Configura regiões de memória
  memory_callbacks_t callbacks = {0};
  emu_memory_add_region(platform_data.memory, 0x000000, 0x400000, NULL,
                        EMU_MEMORY_ROM, &callbacks);
  emu_memory_add_region(platform_data.memory, 0xFF0000, 0x10000,
                        platform_data.ram, EMU_MEMORY_RAM, &callbacks);
  emu_memory_add_region(platform_data.memory, 0xA00000, 0x2000,
                        platform_data.z80_ram, EMU_MEMORY_RAM, &callbacks);

  // Inicializa o Z80
  platform_data.z80 = md_z80_adapter_create();
  if (!platform_data.z80) {
    emu_memory_shutdown(platform_data.memory);
    emu_memory_destroy(platform_data.memory);
    free(platform_data.ram);
    free(platform_data.z80_ram);
    return false;
  }
  md_z80_adapter_connect_memory(platform_data.z80, platform_data.memory);

  // Inicializa valores padrão
  platform_data.pad1_state = 0xFF;  // Nenhum botão pressionado
  platform_data.pad2_state = 0xFF;  // Nenhum botão pressionado
  platform_data.z80_control = 0x01; // Z80 em reset por padrão

  // Inicialização completa
  platform_data.is_initialized = true;
  platform->platform_data = &platform_data;

  return true;
}

void md_platform_shutdown(emu_platform_t *platform) {
  if (!platform || !platform->platform_data)
    return;

  md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;

  // Desativa a plataforma
  data->is_initialized = false;

  // Finaliza e libera o Z80
  if (data->z80) {
    md_z80_adapter_destroy(data->z80);
    data->z80 = NULL;
  }

  // Libera memória RAM
  if (data->ram) {
    free(data->ram);
    data->ram = NULL;
    data->ram_size = 0;
  }

  // Libera memória do Z80
  if (data->z80_ram) {
    free(data->z80_ram);
    data->z80_ram = NULL;
    data->z80_ram_size = 0;
  }

  // Libera ROM
  if (data->rom_data) {
    free(data->rom_data);
    data->rom_data = NULL;
    data->rom_size = 0;
  }

  // Libera ROM do cartucho (se for diferente da rom_data)
  if (data->cart_rom && data->cart_rom != data->rom_data) {
    free(data->cart_rom);
  }
  data->cart_rom = NULL;
  data->cart_rom_size = 0;

  // Finaliza e libera o sistema de memória
  if (data->memory) {
    emu_memory_shutdown(data->memory);
    emu_memory_destroy(data->memory);
    data->memory = NULL;
  }

  // Finaliza e libera o sistema de vídeo
  if (data->video) {
    // TODO: Implementar finalização do vídeo
    data->video = NULL;
  }

  // Finaliza e libera o sistema de áudio
  if (data->audio) {
    // TODO: Implementar finalização do áudio
    data->audio = NULL;
  }

  // Limpa outras informações
  memset(data, 0, sizeof(md_platform_data_t));
}

bool md_platform_reset(emu_platform_t *platform) {
  if (!platform || !platform->platform_data)
    return false;

  md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;
  if (!data->is_initialized)
    return false;

  // Limpa RAM principal
  if (data->ram && data->ram_size > 0) {
    memset(data->ram, 0, data->ram_size);
  }

  // Limpa RAM do Z80
  if (data->z80_ram && data->z80_ram_size > 0) {
    memset(data->z80_ram, 0, data->z80_ram_size);
  }

  // Reseta registradores
  data->vdp_data_buffer = 0;
  data->vdp_control_buffer = 0;
  data->vdp_hv_counter = 0;
  data->pad1_state = 0xFF; // Padrão para nenhum botão pressionado
  data->pad2_state = 0xFF; // Padrão para nenhum botão pressionado
  data->sram_control = 0;
  data->z80_control = 0x01; // Z80 em reset por padrão
  data->z80_bank_register = 0;

  // Reseta o Z80
  if (data->z80) {
    md_z80_adapter_reset(data->z80);
    md_z80_adapter_set_reset(data->z80, true); // Iniciar com Z80 em reset
    md_z80_adapter_set_busreq(data->z80,
                              true); // Iniciar com Z80 em bus request
  } else {
    // Inicializa o Z80 se não estiver inicializado
    data->z80 = md_z80_adapter_create();
    if (data->z80) {
      md_z80_adapter_connect_memory(data->z80, data->memory);
      md_z80_adapter_connect_audio(data->z80, data->audio);
    }
  }

  // Reset de componentes de sistema
  if (data->video) {
    // TODO: Implementar reset do VDP
  }

  if (data->audio) {
    // TODO: Implementar reset do sistema de áudio
  }

  // Sinaliza que a plataforma está pronta
  data->is_initialized = true;

  return true;
}

bool md_platform_load_rom(emu_platform_t *platform, const char *filename) {
  if (!platform || !platform->platform_data || !filename)
    return false;

  md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;

  // Abre o arquivo ROM
  FILE *rom_file = fopen(filename, "rb");
  if (!rom_file)
    return false;

  // Obtém o tamanho do arquivo
  fseek(rom_file, 0, SEEK_END);
  long file_size = ftell(rom_file);
  fseek(rom_file, 0, SEEK_SET);

  // Aloca memória para a ROM
  if (data->rom_data) {
    free(data->rom_data);
  }
  if (data->cart_rom) {
    free(data->cart_rom);
  }

  // Aloca memória e atualiza ambos os campos para consistência
  data->rom_data = (uint8_t *)malloc(file_size);
  if (!data->rom_data) {
    fclose(rom_file);
    return false;
  }

  data->rom_size = file_size;

  // Atribuir mesmos valores aos campos cart_rom e cart_rom_size
  data->cart_rom = data->rom_data;
  data->cart_rom_size = file_size;

  // Lê o arquivo ROM
  size_t bytes_read = fread(data->rom_data, 1, file_size, rom_file);
  if (bytes_read != (size_t)file_size) {
    free(data->rom_data);
    data->rom_data = NULL;
    data->cart_rom = NULL;
    fclose(rom_file);
    return false;
  }

  fclose(rom_file);

  // Lê o cabeçalho da ROM
  if ((size_t)file_size >= sizeof(md_rom_header_t)) {
    memcpy(&data->rom_header, data->rom_data + 0x100, sizeof(md_rom_header_t));
  }

  return true;
}

bool md_platform_run_frame(emu_platform_t *platform) {
  if (!platform || !platform->platform_data)
    return false;

  md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;
  if (!data->is_initialized)
    return false;

  // Número de ciclos por frame baseado na região
  const uint32_t CYCLES_PER_FRAME = data->is_pal ? 128000 : 127000;
  const float Z80_M68K_RATIO = 0.467; // 3.58MHz / 7.67MHz

  // Tamanho do slice para melhor granularidade
  const uint32_t SLICE_SIZE = 100;

  uint32_t cycles_m68k = 0;
  uint32_t cycles_z80 = 0;
  uint32_t vdp_cycles = 0;

  // Loop principal do frame
  while (cycles_m68k < CYCLES_PER_FRAME) {
    uint32_t remaining = CYCLES_PER_FRAME - cycles_m68k;
    uint32_t slice = (remaining > SLICE_SIZE) ? SLICE_SIZE : remaining;

    // Executar M68K
    if (data->m68k) {
      uint32_t m68k_executed = md_m68k_run_cycles(data->m68k, slice);
      cycles_m68k += m68k_executed;

      // Verificar interrupções do VDP
      if (data->video && md_vdp_check_interrupts(data->video)) {
        md_m68k_interrupt(data->m68k, 6); // Nível 6 para VBlank
      }
    }

    // Executar Z80 se não estiver em reset/busreq
    if (data->z80 && !md_z80_adapter_get_reset(data->z80) &&
        !md_z80_adapter_get_busreq(data->z80)) {
      uint32_t z80_slice = (uint32_t)(slice * Z80_M68K_RATIO);
      uint32_t z80_executed = md_z80_adapter_run_cycles(data->z80, z80_slice);
      cycles_z80 += z80_executed;
    }

    // Atualizar VDP
    if (data->video) {
      vdp_cycles += slice;
      md_vdp_update(data->video, slice);

      // Verificar se completou uma linha
      if (vdp_cycles >= data->video->cycles_per_line) {
        vdp_cycles -= data->video->cycles_per_line;
        md_vdp_end_line(data->video);
      }
    }

    // Atualizar APU
    if (data->audio) {
      md_apu_update(data->audio, slice);
    }

    // Atualizar I/O
    if (data->io) {
      md_io_update(data->io, slice);
    }

    // Sincronizar componentes se necessário
    if (md_m68k_should_sync(data->m68k)) {
      md_m68k_sync_cycles(data->m68k);
      md_z80_adapter_sync_cycles(data->z80);
      md_vdp_sync_cycles(data->video);
    }
  }

  // Atualizar contadores H/V do VDP
  if (data->video) {
    data->video->h_counter =
        (data->video->h_counter + 1) % data->video->h_total;
    if (data->video->h_counter == 0) {
      data->video->v_counter =
          (data->video->v_counter + 1) % data->video->v_total;
    }
  }

  return true;
}

uint32_t md_platform_run_cycles(emu_platform_t *platform, uint32_t cycles) {
  if (!platform || !platform->platform_data)
    return 0;

  md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;
  if (!data->is_initialized)
    return 0;

  // Ciclos executados pelo M68K
  uint32_t cycles_executed = 0;

  // Executar ciclos do M68K
  // TODO: Implementar execução do M68K

  // Executar ciclos do Z80 se não estiver em reset ou busreq
  if (data->z80 && !md_z80_adapter_get_reset(data->z80) &&
      !md_z80_adapter_get_busreq(data->z80)) {
    uint32_t z80_cycles = (uint32_t)(cycles * 0.467); // 3.58MHz / 7.67MHz
    md_z80_adapter_run_cycles(data->z80, z80_cycles);
  }

  // Atualizar VDP
  if (data->video) {
    // TODO: Implementar atualização do VDP
  }

  cycles_executed = cycles;
  return cycles_executed;
}

uint8_t md_memory_read_u8(emu_platform_t *platform, uint32_t address) {
  if (!platform || !platform->platform_data) {
    return 0xFF; // Valor padrão em caso de erro
  }

  md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;
  if (!data->is_initialized) {
    return 0xFF; // Emulador não inicializado
  }

  // Verificação de ROM
  if (address < data->rom_size && data->rom_data) {
    return data->rom_data[address];
  }
  // Verificação de RAM
  else if (address >= 0xFF0000 && address < 0xFF0000 + data->ram_size &&
           data->ram) {
    return data->ram[address - 0xFF0000];
  }
  // Verificação de RAM Z80
  else if (address >= 0xA00000 && address < 0xA00000 + data->z80_ram_size &&
           data->z80_ram) {
    return data->z80_ram[address - 0xA00000];
  }

  // Endereço não mapeado
  return 0xFF;
}

void md_memory_write_u8(emu_platform_t *platform, uint32_t address,
                        uint8_t value) {
  if (!platform || !platform->platform_data) {
    return;
  }

  md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;
  if (!data->is_initialized) {
    return; // Emulador não inicializado
  }

  // Verificação de RAM
  if (address >= 0xFF0000 && address < 0xFF0000 + data->ram_size && data->ram) {
    data->ram[address - 0xFF0000] = value;
  }
  // Verificação de RAM Z80
  else if (address >= 0xA00000 && address < 0xA00000 + data->z80_ram_size &&
           data->z80_ram) {
    data->z80_ram[address - 0xA00000] = value;
  }
  // Escrita em registradores especiais
  else if (address >= 0xA10000 && address <= 0xA10020) {
    // Controles e registradores I/O
    if (address == 0xA10008) {
      data->pad1_state = value;
    } else if (address == 0xA10009) {
      data->pad2_state = value;
    }
  }
  // VDP control/data ports
  else if (address >= 0xC00000 && address <= 0xC0001F) {
    // Portas do VDP
    if ((address & 0xE) == 0) {
      data->vdp_data_buffer = value;
      // TODO: Implementar escrita no VDP
    } else if ((address & 0xE) == 4) {
      data->vdp_control_buffer = value;
      // TODO: Implementar escrita de controle no VDP
    }
  }
}

// Inicializa a plataforma Mega Drive
bool megadrive_init(MegaDrive* md) {
    // Aloca memória para os componentes
    md->cpu = (CPU*)malloc(sizeof(CPU));
    md->ppu = (PPU*)malloc(sizeof(PPU));

    // Aloca memórias
    md->ram = (uint8_t*)malloc(0x10000);    // 64KB RAM
    md->vram = (uint8_t*)malloc(0x10000);   // 64KB VRAM
    md->z80_ram = (uint8_t*)malloc(0x2000); // 8KB Z80 RAM

    // Inicializa componentes
    cpu_init(md->cpu);
    ppu_init(md->ppu);

    // Limpa memórias
    memset(md->ram, 0, 0x10000);
    memset(md->vram, 0, 0x10000);
    memset(md->z80_ram, 0, 0x2000);

    md->running = false;
    md->cycles = 0;

    return true;
}

// Carrega uma ROM
bool megadrive_load_rom(MegaDrive* md, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Aloca e carrega a ROM
    md->rom = (uint8_t*)malloc(size);
    fread(md->rom, 1, size, file);
    fclose(file);

    return true;
}

// Executa um frame
void megadrive_run_frame(MegaDrive* md) {
    // TODO: Implementar lógica de execução de frame
    // - Executar ciclos do CPU
    // - Atualizar PPU
    // - Processar interrupções
}

// Reset da plataforma
void megadrive_reset(MegaDrive* md) {
    cpu_reset(md->cpu);
    ppu_reset(md->ppu);
    md->cycles = 0;
}

// Libera recursos
void megadrive_destroy(MegaDrive* md) {
    free(md->cpu);
    free(md->ppu);
    free(md->ram);
    free(md->vram);
    free(md->z80_ram);
    if (md->rom) {
        free(md->rom);
    }
}
