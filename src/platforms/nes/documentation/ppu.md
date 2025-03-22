# Implementação da PPU (Picture Processing Unit) do NES

## Visão Geral Técnica

Este arquivo contém informações técnicas para os desenvolvedores sobre a implementação da PPU do NES no emulador.

A documentação completa da PPU para usuários e referência geral está disponível em `docs/architecture/nes/PPU.md`.

## Estruturas de Dados

A PPU é implementada usando a estrutura `nes_ppu_t` que contém:

- Registradores (PPUCTRL, PPUMASK, PPUSTATUS, etc.)
- Estado interno de renderização (scanline, cycle, frame_count)
- Buffers de memória (VRAM, OAM, paletas)
- Ponteiros para componentes relacionados (CPU, cartridge)
- Configurações de TV system, overscan e efeitos visuais

## Funções Principais

- `nes_ppu_init()`: Inicializa o estado da PPU
- `nes_ppu_reset()`: Reinicia a PPU para o estado inicial
- `nes_ppu_step()`: Executa um único passo da PPU (1 ciclo)
- `nes_ppu_execute()`: Executa a PPU por um número específico de ciclos
- `nes_ppu_render_frame()`: Renderiza o frame atual no buffer
- `nes_ppu_read_register()` / `nes_ppu_write_register()`: Acesso aos registradores ($2000-$2007)

## Registradores

A PPU possui 8 registradores acessíveis pela CPU:

- **$2000 (PPUCTRL)**: Controle geral da PPU
- **$2001 (PPUMASK)**: Controle de renderização
- **$2002 (PPUSTATUS)**: Status da PPU
- **$2003 (OAMADDR)**: Endereço OAM
- **$2004 (OAMDATA)**: Dados OAM
- **$2005 (PPUSCROLL)**: Controle de rolagem
- **$2006 (PPUADDR)**: Endereço VRAM
- **$2007 (PPUDATA)**: Dados VRAM

## Ciclo de Renderização

A PPU trabalha em um padrão específico de 341 ciclos por scanline, 262 scanlines por frame:

- Scanlines 0-239: Linhas visíveis (renderização ativa)
- Scanline 240: Post-render scanline
- Scanlines 241-260: VBlank (NMI gerado no início)
- Scanline 261: Pre-render scanline (preparação para próximo frame)

Cada ciclo da PPU é sincronizado com a CPU (1 ciclo de CPU = 3 ciclos de PPU).

## Funções Internas

### Renderização

- `render_background()`: Renderiza os tiles de fundo
- `render_sprites()`: Renderiza os sprites
- `apply_color_mask()`: Aplica mascaramento de cor (ênfase RGB)

### Mapeamento de Memória

- `nes_ppu_read()` / `nes_ppu_write()`: Acesso ao espaço de memória da PPU
- `nes_ppu_read_oam()` / `nes_ppu_write_oam()`: Acesso ao OAM
- `nes_ppu_read_palette_public()`: Leitura da paleta

## Recursos Avançados

### Split Scrolling

- `nes_ppu_configure_split_scroll()`: Configura divisão de scrolling na tela
- Parâmetros para controlar scanline de divisão e registradores específicos

### Overscan e Distorção

- `nes_ppu_set_overscan()`: Configura áreas de overscan
- `nes_ppu_set_distortion()`: Configura efeitos de distorção de CRT

### Sistemas de TV

- `nes_ppu_set_tv_system()`: Define sistema de TV (NTSC, PAL, Dendy)
- `nes_ppu_load_palette_set()`: Carrega tabela de cores para sistema específico

## Otimizações

- Tabela de paletas pre-calculada
- Cache de patterns para tiles frequentemente usados
- Renderização condicional baseada em visibilidade
- Otimizações de boundary checking

## Timing e Precisão

- Timing ciclo-preciso para todos os eventos
- Implementação precisa dos 341 ciclos por scanline
- Emulação correta de efeitos de mid-scanline (mudança de registradores)
- Sincronização com NMI no início do VBlank (scanline 241, ciclo 1)

## Debug e Desenvolvimento

- Logs detalhados podem ser ativados com `NES_PPU_DEBUG`
- Buffer de frame acessível para debug e inspeção visual
- Funções para manipular estado interno durante desenvolvimento

## Limitações Conhecidas

- Alguns efeitos raros de timing de sprites podem não ter precisão perfeita
- Renderização de frames parciais durante reset não é totalmente emulada
- Ver `docs/ROADMAP.md` para tarefas pendentes relacionadas à PPU

## Notas de Implementação

1. O contador de ciclos da PPU deve manter precisão durante todo o frame
2. Atualizações dos registradores de scroll (PPUSCROLL) precisam de tratamento especial
3. A implementação segue o modelo de renderização de 8 pixels por vez
4. A detecção de sprite zero deve ser implementada com precisão
5. O Latch de transparência para tiles deve ser corretamente implementado

## Referências para Desenvolvedores

- Código fonte em `src/platforms/nes/ppu/`
- Testes em `tests/platforms/nes/ppu/`
- Documentação completa em `docs/architecture/nes/PPU.md`
