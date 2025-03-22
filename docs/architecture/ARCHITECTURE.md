# Arquitetura do Mega_Emu

## Visão Geral

O Mega_Emu é um emulador multi-plataforma projetado para emular diversos consoles retro com foco em precisão, desempenho e modularidade. A arquitetura do projeto é estruturada para maximizar o reuso de código entre diferentes plataformas, mantendo as especificidades de cada sistema isoladas através de interfaces bem definidas.

O emulador é dividido em camadas:
- **Core**: Componentes fundamentais reutilizáveis (CPUs, memória, vídeo, áudio)
- **Platforms**: Implementações específicas para cada console
- **Utils**: Utilitários compartilhados por todos os módulos
- **Frontend**: Interface para o usuário final

## Estrutura de Diretórios

```
src/
├── core/                # Código core reutilizável
│   ├── cpu/             # Implementações de CPUs
│   │   ├── z80/         # Implementação do Z80
│   │   ├── m68k/        # Implementação do 68000
│   │   ├── 6502/        # Implementação do 6502/2A03
│   │   ├── 65c816/      # Implementação do 65C816 (SNES)
│   │   ├── lr35902/     # Implementação do LR35902 (Game Boy)
│   │   ├── arm/         # Implementações ARM (GBA, 3DO)
│   │   ├── mips/        # Implementações MIPS (PlayStation, N64)
│   │   ├── sh/          # Implementações SH2/SH4 (Saturn, Dreamcast)
│   │   └── huc6280/     # Implementação do HuC6280 (PC Engine)
│   │
│   ├── audio/           # Framework de áudio
│   │   ├── chips/       # Implementações de chips de áudio
│   │   │   ├── sn76489/ # PSG (Master System, Mega Drive)
│   │   │   ├── ym2612/  # FM Synthesis (Mega Drive)
│   │   │   ├── apu/     # APU (NES)
│   │   │   ├── spc700/  # SPC700 (SNES)
│   │   │   ├── gb_apu/  # Game Boy APU
│   │   │   ├── pokey/   # POKEY (Atari)
│   │   │   ├── scsp/    # SCSP (Saturn)
│   │   │   ├── spu/     # SPU (PlayStation)
│   │   │   └── aica/    # AICA (Dreamcast)
│   │
│   ├── video/           # Framework gráfico
│   │   ├── chips/       # Implementações de chips de vídeo
│   │   │   ├── vdp_md/  # VDP (Mega Drive)
│   │   │   ├── ppu_nes/ # PPU (NES)
│   │   │   ├── ppu_snes/# PPU (SNES)
│   │   │   ├── vdp_sms/ # VDP (Master System)
│   │   │   ├── tms9918/ # TMS9918 (ColecoVision)
│   │   │   ├── gb_ppu/  # Game Boy PPU
│   │   │   ├── gba_ppu/ # Game Boy Advance PPU
│   │   │   ├── vdc/     # VDC (PC Engine)
│   │   │   ├── neogeo/  # Neo Geo Video
│   │   │   ├── ps_gpu/  # PlayStation GPU
│   │   │   ├── saturn_vdp/ # Saturn VDP1/VDP2
│   │   │   └── powervr2/# PowerVR2 (Dreamcast)
│   │
│   ├── input/           # Gerenciamento de entrada
│   ├── memory/          # Sistema de memória
│   │   ├── emu_memory/  # Sistema de memória do emulador
│   │   ├── mappers/     # Mappers de cartuchos
│   │   ├── cart_io/     # E/S de cartuchos
│   │   └── save/        # Sistema de salvamento
│   │
│   ├── events/          # Sistema de eventos
│   ├── save_state/      # Sistema de save state
│   ├── debugging/       # Ferramentas de debug
│   ├── logging/         # Sistema de logging
│   ├── config/          # Sistema de configuração
│   ├── interfaces/      # Interfaces comuns
│   └── global_defines.h # Definições globais
│
├── platforms/           # Código específico de plataforma
│   ├── common/          # Código compartilhado entre plataformas
│   │
│   ├── megadrive/       # Implementação do Mega Drive
│   │   ├── cpu/         # Adaptação de CPUs para Mega Drive
│   │   │   ├── m68k.c   # Integração do 68000
│   │   │   └── z80_adapter.c # Adaptador do Z80
│   │   ├── memory/      # Mapeamento de memória
│   │   ├── video/       # Video Display Processor
│   │   ├── audio/       # Subsistema de áudio
│   │   │   ├── ym2612.c # Integração do YM2612
│   │   │   └── sn76489_adapter.c # Adaptador do SN76489
│   │   └── peripherals/ # Periféricos
│   │       ├── megacd/  # Mega CD
│   │       └── 32x/     # 32X
│   │
│   ├── mastersystem/    # Implementação do Master System
│   │   ├── cpu/         # Adaptação do Z80
│   │   ├── memory/      # Mapeamento de memória
│   │   ├── video/       # Video Display Processor
│   │   ├── audio/       # Subsistema de áudio
│   │   ├── io/          # Entradas/Saídas
│   │   └── peripherals/ # Periféricos
│   │       └── gamegear/ # Game Gear
│   │
│   ├── nes/             # Implementação do NES
│   │   ├── cpu/         # Adaptação do 6502/2A03
│   │   ├── ppu/         # Picture Processing Unit
│   │   ├── apu/         # Audio Processing Unit
│   │   ├── cartridge/   # Sistema de cartuchos/mappers
│   │   ├── memory/      # Mapeamento de memória
│   │   ├── debug/       # Ferramentas de debug específicas
│   │   ├── input/       # Controladores
│   │   ├── save/        # Salvamento
│   │   └── peripherals/ # Periféricos
│   │       └── fds/     # Famicom Disk System
│   │
│   ├── snes/            # Implementação do SNES
│   │   ├── cpu/         # Adaptação do 65C816
│   │   ├── ppu/         # Picture Processing Unit
│   │   ├── apu/         # Audio Processing Unit (SPC700)
│   │   ├── memory/      # Mapeamento de memória
│   │   └── io/          # Controladores e I/O
│   │
│   ├── gameboy/         # Implementação do Game Boy
│   │   ├── cpu/         # Adaptação do LR35902
│   │   ├── ppu/         # Picture Processing Unit
│   │   ├── apu/         # Audio Processing Unit
│   │   └── memory/      # Mapeamento de memória
│   │
│   ├── gameboycolor/    # Implementação do Game Boy Color
│   │   ├── cpu/         # Adaptação do LR35902
│   │   ├── ppu/         # Picture Processing Unit
│   │   ├── apu/         # Audio Processing Unit
│   │   └── memory/      # Mapeamento de memória
│   │
│   ├── gba/             # Implementação do Game Boy Advance
│   │   ├── cpu/         # ARM7TDMI
│   │   ├── ppu/         # Picture Processing Unit
│   │   ├── apu/         # Audio Processing Unit
│   │   └── memory/      # Mapeamento de memória
│   │
│   ├── atari7800/       # Implementação do Atari 7800
│   ├── colecovision/    # Implementação do ColecoVision
│   ├── atarilynx/       # Implementação do Atari Lynx
│   ├── pcengine/        # Implementação do PC Engine/TurboGrafx-16
│   ├── neogeo/          # Implementação do Neo Geo AES
│   ├── playstation/     # Implementação do PlayStation
│   ├── saturn/          # Implementação do Sega Saturn
│   ├── atarijaguar/     # Implementação do Atari Jaguar
│   ├── 3do/             # Implementação do 3DO
│   ├── n64/             # Implementação do Nintendo 64
│   └── dreamcast/       # Implementação do Dreamcast
│
├── utils/               # Utilitários
│   ├── enhanced_log/    # Sistema de logging
│   ├── debug/           # Ferramentas de debug
│   ├── file/            # Operações de arquivo
│   ├── time/            # Temporizadores e sincronização
│   └── compression/     # Compressão de dados
│
└── frontend/            # Interface do usuário
    ├── sdl/             # Interface SDL
    ├── audio/           # Interface de áudio
    ├── video/           # Interface de vídeo
    ├── input/           # Interface de entrada
    └── common/          # Código UI comum
```

## Processadores Implementados/Planejados

### Zilog Z80
**Especificações:**
- CPU 8-bit com barramento de 16-bit
- Utilizado em múltiplos sistemas:
  - Master System: Processador principal (3.58MHz)
  - Mega Drive: Co-processador de áudio (3.58MHz)
  - ColecoVision: Processador principal (3.58MHz)
  - Neo Geo: Co-processador de áudio (4MHz)

**Implementação:**
- Localização: `src/core/cpu/z80/`
- Componentes:
  - `z80.h/z80.c`: API e implementação base do Z80
  - `z80_instructions.h/z80_instructions.c`: Instruções e decodificação
- Adaptadores específicos por plataforma:
  - `src/platforms/megadrive/cpu/z80_adapter.c`
  - `src/platforms/mastersystem/cpu/z80_adapter.c`
  - `src/platforms/colecovision/cpu/z80_adapter.c`
  - `src/platforms/neogeo/cpu/z80_adapter.c`

### Motorola 68000 (M68K)
**Especificações:**
- Processador 16/32-bit
- Utilizado em:
  - Mega Drive: Processador principal (7.67MHz PAL / 7.61MHz NTSC)
  - Neo Geo: Processador principal (12MHz)
  - Atari Jaguar: Processador de controle (13.3MHz)

**Implementação:**
- Localização: `src/core/cpu/m68k/`
- Componentes:
  - `m68k.h/m68k.c`: API e implementação base
  - `m68k_instructions.h/m68k_instructions.c`: Instruções e decodificação
- Adaptadores específicos por plataforma:
  - `src/platforms/megadrive/cpu/m68k.c`
  - `src/platforms/neogeo/cpu/m68k_neogeo.c`
  - `src/platforms/atarijaguar/cpu/m68000.c`

### MOS 6502 e Variantes
**Especificações:**
- CPU 8-bit com barramento de 16-bit
- Utilizado em:
  - NES: Variante RP2A03 (1.79MHz)
  - Atari 7800: Variante 6502C (1.79MHz)
  - Atari Lynx: Variante 65C02 (4MHz)

**Implementação:**
- Localização: `src/core/cpu/6502/`
- Componentes:
  - `m6502.h/m6502.c`: API e implementação base
  - `m6502_instructions.h/m6502_instructions.c`: Instruções e decodificação
- Adaptadores específicos por plataforma:
  - `src/platforms/nes/cpu/nes_cpu.c`
  - `src/platforms/atari7800/cpu/m6502.c`
  - `src/platforms/atarilynx/cpu/wdc65c02.c`

### 65C816 (SNES)
**Especificações:**
- CPU 16-bit com modos de compatibilidade 8-bit
- Utilizado no SNES (3.58MHz efetivo)

**Implementação:**
- Localização: `src/core/cpu/65c816/`
- Componentes:
  - `w65c816.h/w65c816.c`: API e implementação base
  - `w65c816_instructions.h/w65c816_instructions.c`: Instruções e decodificação
- Adaptador específico:
  - `src/platforms/snes/cpu/w65c816.c`

### LR35902 (Game Boy/Game Boy Color)
**Especificações:**
- Híbrido entre Z80 e 8080 (8-bit, 4.19MHz)
- Utilizado em Game Boy e Game Boy Color

**Implementação:**
- Localização: `src/core/cpu/lr35902/`
- Componentes:
  - `lr35902.h/lr35902.c`: API e implementação base
  - `lr35902_instructions.h/lr35902_instructions.c`: Instruções e decodificação
- Adaptadores específicos:
  - `src/platforms/gameboy/cpu/lr35902.c`
  - `src/platforms/gameboycolor/cpu/lr35902_gbc.c`

### ARM
**Especificações:**
- Arquitetura RISC 32-bit
- Variantes:
  - ARM7TDMI (GBA): 16.78MHz
  - ARM60 (3DO): 12.5MHz

**Implementação:**
- Localização: `src/core/cpu/arm/`
- Componentes:
  - `arm7tdmi.h/arm7tdmi.c`: Implementação ARM7TDMI
  - `arm_instructions.h/arm_instructions.c`: Instruções ARM
  - `thumb_instructions.h/thumb_instructions.c`: Instruções Thumb
  - `arm60.h/arm60.c`: Implementação ARM60
- Adaptadores específicos:
  - `src/platforms/gba/cpu/arm7tdmi.c`
  - `src/platforms/3do/cpu/arm60.c`

### MIPS
**Especificações:**
- Arquitetura RISC 32/64-bit
- Variantes:
  - R3000A (PlayStation): 33.8688MHz
  - VR4300 (N64): 93.75MHz

**Implementação:**
- Localização: `src/core/cpu/mips/`
- Componentes:
  - `r3000a.h/r3000a.c`: Implementação R3000A
  - `r3000a_instructions.h/r3000a_instructions.c`: Instruções R3000A
  - `vr4300.h/vr4300.c`: Implementação VR4300
  - `vr4300_instructions.h/vr4300_instructions.c`: Instruções VR4300
- Adaptadores específicos:
  - `src/platforms/playstation/cpu/r3000a.c`
  - `src/platforms/n64/cpu/vr4300.c`

### Hitachi SH
**Especificações:**
- Arquitetura RISC de 32-bit
- Variantes:
  - SH-2 (Saturn): Dual CPU @ 28.6MHz cada
  - SH-4 (Dreamcast): 200MHz

**Implementação:**
- Localização: `src/core/cpu/sh/`
- Componentes:
  - `sh2.h/sh2.c`: Implementação SH2
  - `sh2_instructions.h/sh2_instructions.c`: Instruções SH2
  - `sh4.h/sh4.c`: Implementação SH4
  - `sh4_instructions.h/sh4_instructions.c`: Instruções SH4
- Adaptadores específicos:
  - `src/platforms/saturn/cpu/sh2.c`
  - `src/platforms/dreamcast/cpu/sh4.c`

### HuC6280 (PC Engine)
**Especificações:**
- Variante do 65C02 com instruções estendidas (8-bit, 7.16MHz)
- Utilizado no PC Engine/TurboGrafx-16

**Implementação:**
- Localização: `src/core/cpu/huc6280/`
- Componentes:
  - `huc6280.h/huc6280.c`: API e implementação base
  - `huc6280_instructions.h/huc6280_instructions.c`: Instruções e decodificação
- Adaptador específico:
  - `src/platforms/pcengine/cpu/huc6280.c`

## Arquitetura de Plataformas

### Mega Drive/Genesis

```
+----------------+       +----------------+
|    M68000      |<----->|      Z80       |
| (Main CPU)     |       | (Sound CPU)    |
+----------------+       +----------------+
        |                        |
        v                        v
+----------------+       +----------------+
|  Main Memory   |       |  Sound Memory  |
+----------------+       +----------------+
        |                        |
        v                        v
+----------------+       +----------------+
|      VDP       |       |    YM2612      |
| (Video)        |       | (FM Sound)     |
+----------------+       +----------------+
                                 |
                                 v
                         +----------------+
                         |    SN76489     |
                         | (PSG Sound)    |
                         +----------------+
```

**Componentes:**
- **Processadores:**
  - Motorola 68000 (16/32-bit, 7.67MHz)
  - Zilog Z80 (8-bit, 3.58MHz, áudio)
- **Memória:**
  - 64KB RAM principal
  - 8KB RAM de vídeo
  - 8KB RAM de som
- **Vídeo:** VDP (Video Display Processor)
- **Áudio:**
  - YM2612 (FM 6-channel)
  - SN76489 (PSG 4-channel)
- **Periféricos:**
  - Mega CD
  - 32X

### Master System

```
+----------------+
|      Z80       |
| (Main CPU)     |
+----------------+
        |
        v
+----------------+
|    Memory      |
+----------------+
     /     \
    /       \
   v         v
+------+  +------+
| VDP  |  | PSG  |
+------+  +------+
```

**Componentes:**
- **Processador:** Zilog Z80 (8-bit, 3.58MHz)
- **Memória:** 8KB RAM
- **Vídeo:** VDP baseado no TMS9918
- **Áudio:** SN76489 (PSG)
- **Periféricos:**
  - Game Gear (portátil)

### NES/Famicom

```
+----------------+       +----------------+
|   RP2A03/2A07  |------>|      PPU       |
| (CPU+APU)      |       | (Video)        |
+----------------+       +----------------+
        |                        |
        v                        v
+----------------+       +----------------+
|     Memory     |       |  Video Memory  |
+----------------+       +----------------+
        |
        v
+----------------+
|    Cartridge   |
|   (w/ Mapper)  |
+----------------+
```

**Componentes:**
- **Processador:** Ricoh 2A03/2A07 (baseado no 6502, 1.79MHz)
- **Memória:** 2KB RAM
- **Vídeo:** PPU (Picture Processing Unit)
- **Áudio:** APU (2 square, triangle, noise, DMC)
- **Periféricos:**
  - Famicom Disk System (FDS)

### SNES

```
+----------------+       +----------------+       +----------------+
|    65C816      |<----->|      PPU       |<----->|    Cartridge   |
| (Main CPU)     |       | (Video)        |       | (w/ Enhancement|
+----------------+       +----------------+       |    Chips)      |
        |                                         +----------------+
        v                                                 |
+----------------+                                        |
|     WRAM       |<---------------------------------------+
+----------------+
        |
        v
+----------------+       +----------------+
|    S-SMP       |<----->|     S-DSP      |
| (Sound CPU)    |       | (Sound DSP)    |
+----------------+       +----------------+
```

**Componentes:**
- **Processadores:**
  - 65C816 (16-bit, 3.58MHz)
  - Sony SPC700 (som, 1.024MHz)
- **Memória:**
  - 128KB WRAM
  - 64KB VRAM
  - 64KB som
- **Vídeo:** Dual PPU (8 modos)
- **Áudio:** S-DSP (8 canais)
- **Enhancement Chips:**
  - SuperFX
  - SA-1
  - DSP-1/2/3/4
  - Vários outros

### Game Boy

```
+----------------+       +----------------+
|    LR35902     |<----->|      PPU       |
| (CPU+Sound)    |       | (Video)        |
+----------------+       +----------------+
        |                        |
        v                        v
+----------------+       +----------------+
|     Memory     |       |  Video Memory  |
+----------------+       +----------------+
        |
        v
+----------------+
|    Cartridge   |
|   (w/ MBC)     |
+----------------+
```

**Componentes:**
- **Processador:** Sharp LR35902 (8-bit, 4.19MHz)
- **Memória:** 8KB RAM
- **Vídeo:** PPU (monotone)
- **Áudio:** 4 canais (2 square, wave, noise)

### Game Boy Color

Similar ao Game Boy, mas com:
- PPU colorido (32768 cores, 56 na tela)
- 32KB RAM
- Dupla velocidade seletiva (4.19/8.38MHz)

### Game Boy Advance

```
+----------------+       +----------------+
|   ARM7TDMI     |<----->|      PPU       |
| (Main CPU)     |       | (Video)        |
+----------------+       +----------------+
        |                        |
        v                        v
+----------------+       +----------------+
|     Memory     |       |  Video Memory  |
+----------------+       +----------------+
        |
        |
        v
+----------------+       +----------------+
|    Cartridge   |<----->|    Sound       |
|   (w/ Mapper)  |       |   Hardware     |
+----------------+       +----------------+
```

**Componentes:**
- **Processador:** ARM7TDMI (32-bit, 16.78MHz)
- **Memória:** 32KB WRAM interna + 256KB externa
- **Vídeo:** PPU (32768 cores, modos tile e bitmap)
- **Áudio:** 6 canais (4 PSG, 2 PCM)

### PlayStation

```
+----------------+       +----------------+
|    R3000A      |<----->|      GPU       |
| (Main CPU)     |       | (Graphics)     |
+----------------+       +----------------+
        |                        |
        v                        v
+----------------+       +----------------+
|     Memory     |       |  Video Memory  |
+----------------+       +----------------+
        |
        v
+----------------+       +----------------+
|     CDROM      |<----->|      SPU       |
|   Controller   |       | (Sound)        |
+----------------+       +----------------+
```

**Componentes:**
- **Processador:** R3000A (MIPS, 33.8688MHz)
- **Memória:** 2MB RAM principal, 1MB VRAM
- **Vídeo:** GPU (polígonos texturizados)
- **Áudio:** SPU (24 canais ADPCM)
- **Mídia:** CD-ROM

## Fluxo entre Componentes

O fluxo de dados e controle no Mega_Emu segue o seguinte padrão:

```
    +-----------------------------------------------------------+
    |                     Mega_Emu Core                         |
    |                                                           |
    |  +----------+     +----------+     +----------+          |
    |  |   CPU    |<--->|  Memory  |<--->|  Video   |          |
    |  | Module   |     |  System  |     |  System  |          |
    |  +----------+     +----------+     +----------+          |
    |       ^                ^                ^                 |
    |       |                |                |                 |
    |       v                v                v                 |
    |  +---------+      +---------+      +---------+           |
    |  |  Audio  |<---->| Events  |<---->|  Input  |           |
    |  | System  |      | System  |      | System  |           |
    |  +---------+      +---------+      +---------+           |
    |       ^                ^                ^                 |
    +-------|----------------|----------------|----------------+
            |                |                |
            v                v                v
    +-----------------+  +--------+  +------------------+
    | Audio Frontend  |  | System |  | Input Frontend   |
    | (SDL Audio)     |  | Events |  | (SDL Input)      |
    +-----------------+  +--------+  +------------------+
            ^                ^                ^
            |                |                |
            +----------------+----------------+
                             |
                             v
                      +-------------+
                      |  Frontend   |
                      | (SDL, etc.) |
                      +-------------+
                             ^
                             |
                             v
                        +--------+
                        |  User  |
                        +--------+
```

### Fluxo de Execução Principal

1. **Inicialização**:
   - O `Frontend` inicializa o SDL e configura a janela/tela
   - A plataforma selecionada é carregada (ex: `NES`, `MegaDrive`)
   - A ROM é analisada e carregada na memória
   - O sistema é inicializado (CPUs, vídeo, áudio)

2. **Loop Principal**:
   - Sincronização baseada em frames (60fps típico)
   - Os eventos de entrada são processados
   - A CPU executa um número definido de ciclos
   - O subsistema de vídeo é atualizado e renderizado
   - O subsistema de áudio gera amostras
   - Timing e sincronização são ajustados

3. **Comunicação entre Componentes**:
   - **CPU -> Memória**: Leituras/escritas de memória
   - **CPU -> Vídeo**: Atualizações de registradores e DMA
   - **CPU -> Áudio**: Atualizações de registradores
   - **Vídeo -> CPU**: Interrupções (VBLANK, etc.)
   - **Entrada -> CPU**: Leituras de portas/registradores
   - **Eventos -> Todos**: Distribuição de eventos do sistema

## Hardware Compartilhado entre Plataformas

A arquitetura do Mega_Emu é projetada para reutilizar componentes de hardware comuns entre plataformas. Isso reduz a duplicação de código e melhora a manutenibilidade.

### Processadores Compartilhados

| Processador | Plataformas |
|-------------|-------------|
| **Z80**     | Mega Drive, Master System, ColecoVision, Neo Geo |
| **M68000**  | Mega Drive, Neo Geo, Atari Jaguar |
| **6502**    | NES, Atari 7800, Atari Lynx |
| **65C816**  | SNES |
| **LR35902** | Game Boy, Game Boy Color |
| **ARM**     | Game Boy Advance, 3DO |
| **MIPS**    | PlayStation, Nintendo 64 |
| **SH**      | Saturn (SH-2), Dreamcast (SH-4) |
| **HuC6280** | PC Engine |

### Chips de Áudio Compartilhados

| Chip de Áudio | Plataformas |
|---------------|-------------|
| **SN76489**   | Mega Drive, Master System, Game Gear, ColecoVision |
| **YM2612**    | Mega Drive |
| **NES APU**   | NES |
| **SPC700**    | SNES |
| **GB APU**    | Game Boy, Game Boy Color |
| **POKEY**     | Atari 7800, Atari Lynx |
| **SPU**       | PlayStation |
| **SCSP**      | Saturn |
| **AICA**      | Dreamcast |

### Chips de Vídeo Compartilhados

| Chip de Vídeo | Plataformas |
|---------------|-------------|
| **VDP (MD)**  | Mega Drive |
| **VDP (SMS)** | Master System, Game Gear |
| **PPU (NES)** | NES |
| **PPU (SNES)**| SNES |
| **GB PPU**    | Game Boy, Game Boy Color |
| **GBA PPU**   | Game Boy Advance |
| **TMS9918**   | ColecoVision |
| **TIA**       | Atari 7800 |
| **VDC**       | PC Engine |
| **PS GPU**    | PlayStation |
| **VDP1/2**    | Saturn |
| **PowerVR2**  | Dreamcast |
| **RDP/RSP**   | Nintendo 64 |

## Interface e Adaptação

Cada componente compartilhado segue um padrão comum:

1. **Implementação do Core**: Implementação base independente de plataforma
   ```c
   // Exemplo para o Z80
   typedef struct {
       // Registradores e estado
       uint16_t registers[8];
       uint8_t flags;
       // Ponteiros para callbacks de acesso à memória
       uint8_t (*read_byte)(void* context, uint16_t address);
       void (*write_byte)(void* context, uint16_t address, uint8_t value);
       // Contexto do sistema
       void* context;
   } z80_t;

   // Funções da API pública
   void z80_init(z80_t* cpu);
   void z80_reset(z80_t* cpu);
   int z80_execute(z80_t* cpu, int cycles);
   ```

2. **Adaptador Específico da Plataforma**: Liga o componente core ao sistema específico
   ```c
   // Exemplo para o adaptador Z80 do Mega Drive
   typedef struct {
       z80_t cpu;             // Instância do Z80
       md_context_t* context; // Contexto do Mega Drive
   } md_z80_t;

   // Callbacks específicos da plataforma
   static uint8_t md_z80_read_byte(void* context, uint16_t address) {
       md_context_t* md = (md_context_t*)context;
       // Lógica específica do Mega Drive para acesso à memória Z80
       // ...
   }

   static void md_z80_write_byte(void* context, uint16_t address, uint8_t value) {
       md_context_t* md = (md_context_t*)context;
       // Lógica específica do Mega Drive para acesso à memória Z80
       // ...
   }

   // Função de inicialização específica da plataforma
   void md_z80_init(md_z80_t* z80, md_context_t* context) {
       z80->context = context;
       z80_init(&z80->cpu);
       z80->cpu.read_byte = md_z80_read_byte;
       z80->cpu.write_byte = md_z80_write_byte;
       z80->cpu.context = context;
   }
   ```

Este padrão de design permite:
- Alterar a implementação core sem afetar os adaptadores
- Adicionar novas plataformas reaproveitando componentes existentes
- Testar componentes isoladamente

## Conclusão

A arquitetura do Mega_Emu é projetada para ser modular e extensível, permitindo a adição de novas plataformas com esforço mínimo. O compartilhamento de hardware comum entre plataformas, como CPUs, chips de áudio e vídeo, reduz a duplicação de código e facilita a manutenção.

Os componentes fundamentais são implementados como bibliotecas independentes no diretório `core`, enquanto as especificidades de cada plataforma são isoladas em seus respectivos diretórios sob `platforms`. Isso permite que o sistema seja facilmente estendido para suportar novas plataformas no futuro.

O fluxo de execução e comunicação entre componentes segue um modelo bem definido, com interfaces claras entre os subsistemas, facilitando a depuração e o desenvolvimento colaborativo.
