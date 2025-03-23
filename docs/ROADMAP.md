# ROADMAP - MEGA EMU

> ⚠️ **ALERTA DE IMPLEMENTAÇÃO**: Diversas ferramentas de desenvolvimento estão em **estado de stub** (implementação mínima). Este documento utiliza simbolos visuais para destacar claramente estes componentes. Consulte a legenda abaixo.

> 📢 **ATENÇÃO DESENVOLVEDORES**: A porcentagem de implementação nas tabelas refere-se ao progresso de design/arquitetura e não necessariamente indica código funcional. Ferramentas marcadas com 🔌 **Stub** possuem apenas esqueletos de código sem funcionalidade real. Esta distinção é crucial para evitar confusão durante o desenvolvimento.

## LEGENDA DE STATUS DE IMPLEMENTAÇÃO

| Símbolo | Significado | Descrição |
|---------|-------------|-----------|
| ✅ | Implementado | Componente totalmente implementado e funcional |
| 🔄 | Em Desenvolvimento | Componente em desenvolvimento ativo |
| 🚧 | Implementação Parcial | Componente parcialmente implementado |
| 🔌 | **Stub/Implementação Mínima** | **Apenas estrutura básica implementada (placeholder)** |
| 📝 | Planejado | Componente planejado mas ainda não iniciado |
| ❌ | Não Iniciado | Implementação ainda não começou |

## 1. VISÃO GERAL DO PROJETO

### 1.1 Objetivo Principal

Desenvolver um emulador multi-plataforma de consoles retro com alta precisão, desempenho otimizado e usabilidade intuitiva, permitindo a preservação e estudo de jogos clássicos.

### 1.2 Princípios Fundamentais

- **Precisão**: Emulação fiel ao hardware original (ciclos, timing, comportamentos não-documentados)
- **Modularidade**: Código reutilizável entre plataformas para facilitar manutenção e expansão
- **Usabilidade**: Interface intuitiva para jogadores e ferramentas avançadas para desenvolvedores
- **Documentação**: Documentação completa de código, arquitetura e APIs
- **Testabilidade**: Testes automatizados para validar a precisão da emulação

### 1.3 Escopo de Plataformas

| Prioridade | Plataforma | Status | Recursos Alocados |
|------------|------------|--------|-------------------|
| Alta | NES | Em desenvolvimento (80%) | 2 desenvolvedores |
| Alta | Mega Drive | Em desenvolvimento (65%) | 3 desenvolvedores |
| Média | Master System | Implementado (95%) | 1 desenvolvedor |
| Média | SNES | Planejado (5%) | - |
| Baixa | Game Boy | Planejado (0%) | - |
| Baixa | Game Boy Advance | Planejado (0%) | - |

## 2. ESTRUTURA DE COMPONENTES

### 2.1 Core

Componentes fundamentais reutilizáveis entre plataformas.

#### 2.1.1 CPU

Processadores implementados e planejados:

| Processador | Status | Precisão | Plataformas | Responsáveis |
|-------------|--------|----------|-------------|--------------|
| Z80 | Implementado (100%) | Alta | Master System, Mega Drive, ColecoVision | @dev_team_z80 |
| M68000 | Implementado (85%) | Alta | Mega Drive, Neo Geo | @dev_team_m68k |
| 6502 | Implementado (95%) | Alta | NES, Atari 7800 | @dev_team_6502 |
| 65C816 | Planejado (10%) | Baixa | SNES | - |
| LR35902 | Planejado (5%) | Baixa | Game Boy, Game Boy Color | - |
| ARM7TDMI | Planejado (0%) | N/A | Game Boy Advance | - |

#### 2.1.2 Áudio

| Chip | Status | Precisão | Plataformas | Responsáveis |
|------|--------|----------|-------------|--------------|
| SN76489 (PSG) | Implementado (100%) | Alta | Mega Drive, Master System | @dev_team_audio |
| YM2612 (FM) | Implementado (85%) | Média | Mega Drive | @dev_team_audio |
| APU (NES) | Implementado (90%) | Alta | NES | @dev_team_nes_audio |
| SPC700 | Planejado (0%) | N/A | SNES | - |
| GB APU | Planejado (0%) | N/A | Game Boy | - |

#### 2.1.3 Vídeo

| Chip | Status | Precisão | Plataformas | Responsáveis |
|------|--------|----------|-------------|--------------|
| VDP (Mega Drive) | Implementado (70%) | Média | Mega Drive | @dev_team_md_video |
| PPU (NES) | Implementado (95%) | Alta | NES | @dev_team_nes_video |
| VDP (Master System) | Implementado (95%) | Alta | Master System | @dev_team_sms_video |
| PPU (SNES) | Planejado (0%) | N/A | SNES | - |
| GB PPU | Planejado (0%) | N/A | Game Boy | - |

#### 2.1.4 Sistemas de Suporte

| Sistema | Status | Responsáveis |
|---------|--------|--------------|
| Gerenciamento de Memória | Implementado (95%) | @dev_team_core |
| Sistema de Eventos | Implementado (100%) | @dev_team_core |
| Sistema de Logs | Implementado (100%) | @dev_team_core |
| Sistema de Configuração | Implementado (90%) | @dev_team_core |
| Sistema de Save States | Implementado (85%) | @dev_team_core |
| Ferramentas de Debug | Implementado (80%) | @dev_team_debug |
| Testes Unitários | Implementado (85%) | Todos |

### 2.1.5 Detalhamento das Atualizações nos Sistemas de Suporte

#### Sistema de Save States (85%)

**Implementado:**

- Expandimos o header para incluir compressão de dados
- Adicionamos suporte a múltiplos slots de save state
- Implementamos funcionalidade de rewind
- Adicionamos suporte para auto-save configurável
- Criamos funções para exportação/importação de save states

**Pendente:**

- Implementar a compressão específica de dados nos save states
  - Compressão delta para dados de memória
  - Compressão leve via Zstandard para estado da GPU
- Adicionar capturas de tela nos save states
  - Thumbnails em formato WebP com tarja "Save"
  - Metadados expandidos (região, versão, notas, tags)
- Sistema de perfis de usuário
  - Armazenamento em SQLite
  - Autenticação segura
  - Integração com serviços de nuvem
- Segurança e privacidade
  - Checksums via SHA-256
  - Encriptografia AES-256 para dados sensíveis

#### Ferramentas de Debug (80%)

**Implementado:**

- Implementamos o Memory Viewer com capacidades avançadas
- Criamos um sistema robusto de breakpoints condicionais
- Adicionamos vários tipos de breakpoints: execução, leitura/escrita de memória, registradores
- Implementamos watchpoints para monitorar valores na memória
- Desenvolvemos ferramentas para histórico de alterações de memória

**Pendente:**

- Desenvolver a interface gráfica para o Memory Viewer
- Implementar o avaliador de expressões para breakpoints complexos
- Criar visualizadores específicos para outras áreas do emulador (CPU, vídeo, áudio)

## 3. ESTADO ATUAL POR PLATAFORMA

### 3.1 NES (Nintendo Entertainment System)

**Status Geral**: 90% implementado

#### 3.1.1 Componentes

| Componente | Status | Precisão | Problemas Conhecidos | Responsáveis |
|------------|--------|----------|---------------------|--------------|
| CPU (6502/2A03) | ✅ Alta precisão | 100% dos opcodes e timing implementados | Nenhum | @dev_team_6502 |
| PPU | ✅ Alta precisão | Implementação completa com suporte a todos os efeitos | Nenhum | @dev_team_nes_video |
| APU | ✅ Média precisão | Alguns efeitos de áudio não implementados | @dev_team_nes_audio |
| Mappers | ✅ Parcial | 0, 1, 2, 3, 4, 5, 7, 9, 10, 24 implementados | @dev_team_nes_mappers |
| Controladores | ✅ Completo | Suporte a gamepad, zapper e powerpad | @dev_team_nes |

#### 3.1.2 Compatibilidade

- **Jogos Comerciais**: ~97% funcionam corretamente
- **Demos/Homebrews**: ~90% funcionam corretamente
- **Principais problemas**: Sincronização precisa de IRQs em alguns jogos, mappers incomuns

#### 3.1.3 Tarefas Pendentes Detalhadas (NES)

1. **CPU**
   - [x] ID-NES-CPU-001: Implementar opcodes ilegais restantes
   - [x] ID-NES-CPU-002: Melhorar precisão de timing em operações de página cruzada
   - [x] ID-NES-CPU-003: Adicionar testes unitários para todos os opcodes ilegais

2. **PPU**
   - [x] ID-NES-PPU-001: Corrigir problemas de scrolling com jogos específicos
   - [x] ID-NES-PPU-002: Implementar efeitos de distorção (overscan)
   - [x] ID-NES-PPU-003: Suporte a paletas alternativas (PAL, Dendy)

3. **APU**
   - [ ] ID-NES-APU-001: Implementar suporte completo ao canal DMC
   - [ ] ID-NES-APU-002: Melhorar emulação de efeitos de sweeps
   - [ ] ID-NES-APU-003: Adicionar equalização e filtros

4. **Mappers**
   - [x] ID-NES-MAP-001: Completar implementação do MMC3 (Mapper 4) ✅
   - [x] ID-NES-MAP-002: Implementar MMC5 (Mapper 5) ✅
   - [x] ID-NES-MAP-003: Adicionar suporte a VRC6 (Mapper 24/26)
   - [x] ID-NES-MAP-004: Implementar FDS (Famicom Disk System)

### 3.1.4 Detalhamento dos Mappers NES

**Status Geral**: 85% implementado

| Mapper | Tipo | Status | Responsáveis | Jogos Populares |
|--------|------|--------|--------------|-----------------|
| 0 - NROM | Simples | ✅ Implementado | @dev_team_nes_mappers | Super Mario Bros., Tetris |
| 1 - MMC1 | Comum | ✅ Implementado | @dev_team_nes_mappers | The Legend of Zelda, Metroid |
| 2 - UNROM | Comum | ✅ Implementado | @dev_team_nes_mappers | Super Mario Bros. 2, Kirby's Adventure |
| 3 - CNROM | Comum | ✅ Implementado | @dev_team_nes_mappers | Duck Hunt, Gradius, Mega Man 2 |
| 4 - MMC3 | Avançado | ✅ Implementado | @dev_team_nes_mappers | Super Mario Bros. 3, Ninja Gaiden |
| 5 - MMC5 | Complexo | ✅ Implementado | @dev_team_nes_mappers | Castlevania III, Just Breed |
| 7 - AxROM | Comum | ✅ Implementado | @dev_team_nes_mappers | Battletoads, Star Trek 25th Anniversary |
| 9 - MMC2 | Especializado | ✅ Implementado | @dev_team_nes_mappers | Punch-Out!! |
| 10 - MMC4 | Variante | ✅ Implementado | @dev_team_nes_mappers | Punch-Out!! (versões posteriores) |
| 11 - Color Dreams | Alternativo | ❌ Não Iniciado | - | Baby Boomer, Menace Beach |
| 20 - FDS | Regional | 🔌 Stub | @dev_team_nes_mappers | The Legend of Zelda (JP), Super Mario Bros. 2 (JP) |
| 24 - VRC6 | Terceiros | ✅ Implementado | @dev_team_nes_mappers | Akumajō Densetsu (Famicom) |
| 26 - VRC6 Variante | ❌ Não Iniciado | - | Esper Dream 2 (Famicom) |
| 71 - Camerica/BF9096 | Não-licenciado | ❌ Não Iniciado | - | Bart vs. the World, Micro Machines |
| 75 - VRC1 | Terceiros | ❌ Não Iniciado | - | Lagrange Point (Famicom) |
| 85 - VRC7 | Terceiros (Áudio) | ❌ Não Iniciado | - | Lone Wolf & Cub (Famicom) |
| 90 - Hong Kong SJM | Regional | ❌ Não Iniciado | - | Somari, Mario Bros. 3 (pirata) |
| 230 - Action 53 | Homebrew | ❌ Não Iniciado | - | Action 53 (coleção homebrew) |
| 255 - Experimental | Homebrew | ❌ Não Iniciado | - | Projetos não oficiais |

#### 3.1.4.1 Descrição dos Mappers

- **NROM (0)**: Mapper mais simples, sem bank switching. Suporta até 32 KB PRG ROM e 8 KB CHR ROM.
- **MMC1 (1)**: Suporta bank switching para PRG e CHR ROM, além de controle de mirroring.
- **UNROM (2)**: Mapeia PRG ROM em 16 KB banks. CHR ROM fixo.
- **CNROM (3)**: Mapeia CHR ROM em 8 KB banks. PRG ROM fixo.
- **MMC3 (4)**: Mapper avançado com suporte a IRQs, CHR banking e PRG banking.
- **MMC5 (5)**: Mapper complexo com múltiplos modos (incluindo suporte a expansão de RAM).
- **AxROM (7)**: Mapeia PRG ROM em 32 KB banks. CHR RAM suportada.
- **MMC2 (9)**: Mapper especializado para Punch-Out!!, com CHR banking dinâmico.
- **MMC4 (10)**: Variante do MMC2 com suporte a CHR banking.
- **Color Dreams (11)**: Mapper alternativo para jogos não licenciados.
- **FDS (20)**: Mapper exclusivo para jogos em disquete do Famicom Disk System.
- **VRC6 (24/26)**: Suporta CHR banking e canais de áudio extras (PSG).
- **VRC1 (75)**: CHR banking e controle de mirroring.
- **VRC7 (85)**: Inclui um chip de áudio FM (Yamaha YM2413).
- **Camerica/BF9096 (71)**: Usado em jogos não licenciados.
- **Hong Kong SJM (90)**: Mapper para jogos asiáticos não licenciados.
- **Action 53 (230)**: Mapper para multicarts com múltiplos jogos.
- **Experimental (255)**: Usado em projetos homebrew ou ROMs modificadas.

#### 3.1.4.2 Tarefas Pendentes de Mappers

1. **Mappers Essenciais (Alta Prioridade)**
   - [X] ID-NES-MAP-101: Implementar NROM (Mapper 0) ✅
   - [X] ID-NES-MAP-102: Implementar MMC1 (Mapper 1) ✅
   - [X] ID-NES-MAP-103: Implementar AxROM (Mapper 7) ✅

2. **Mappers Adicionais (Média Prioridade)**
   - [X] ID-NES-MAP-201: Implementar MMC2 (Mapper 9) ✅
   - [X] ID-NES-MAP-202: Implementar MMC4 (Mapper 10) ✅
   - [X] ID-NES-MAP-203: Completar implementação do VRC6 (Mapper 24) ✅
   - [X] ID-NES-MAP-204: Completar implementação do FDS (Mapper 20) 🔌

3. **Mappers Especializados (Baixa Prioridade)**
   - [ ] ID-NES-MAP-301: Implementar VRC7 (Mapper 85)
   - [ ] ID-NES-MAP-302: Implementar VRC1 (Mapper 75)
   - [ ] ID-NES-MAP-303: Implementar Camerica (Mapper 71)
   - [ ] ID-NES-MAP-304: Implementar Color Dreams (Mapper 11)
   - [ ] ID-NES-MAP-305: Implementar Hong Kong SJM (Mapper 90)

4. **Mappers Homebrew (Baixa Prioridade)**
   - [ ] ID-NES-MAP-401: Implementar Action 53 (Mapper 230)
   - [ ] ID-NES-MAP-402: Implementar suporte a mappers experimentais (255)

### 3.2 Mega Drive/Genesis

**Status Geral**: 65% implementado

#### 3.2.1 Componentes

| Componente | Status | Precisão | Problemas Conhecidos | Responsáveis |
|------------|--------|----------|---------------------|--------------|
| CPU (M68000) | 85% | Alta | Algumas instruções específicas | @dev_team_m68k |
| Z80 (Som) | 90% | Alta | Timing de integração com 68000 | @dev_team_z80 |
| VDP | 70% | Média | Efeitos de sombra/highlight, window | @dev_team_md_video |
| YM2612 | 85% | Média | Alguns modos FM específicos | @dev_team_audio |
| SN76489 | 95% | Alta | - | @dev_team_audio |
| Sistema de Memória | 80% | Média | Algumas operações DMA complexas | @dev_team_md_memory |
| Controladores | 90% | Alta | Suporte limitado a periféricos raros | @dev_team_md |

#### 3.2.2 Compatibilidade

- **Jogos Comerciais**: ~75% funcionam corretamente
- **Demos/Homebrews**: ~80% funcionam corretamente
- **Principais problemas**: Jogos que usam efeitos avançados de VDP, timing preciso de Z80/68000

#### 3.2.3 Tarefas Pendentes Detalhadas (Mega Drive)

1. **CPU (M68000)**
   - [ ] ID-MD-68K-001: Implementar instruções restantes
   - [ ] ID-MD-68K-002: Corrigir timing de acesso à memória
   - [ ] ID-MD-68K-003: Adicionar mais testes de exceções
   - [ ] ID-MD-68K-004: Otimizar ciclos para instruções comuns

2. **Z80**
   - [ ] ID-MD-Z80-001: Melhorar integração com barramento do 68000
   - [ ] ID-MD-Z80-002: Implementar ciclos de espera (wait states) precisos
   - [ ] ID-MD-Z80-003: Melhorar manipulação de interrupções

3. **VDP**
   - [ ] ID-MD-VDP-001: Implementar modo shadow/highlight corretamente
   - [ ] ID-MD-VDP-002: Corrigir implementação da window feature
   - [ ] ID-MD-VDP-003: Melhorar precisão dos modos de interlace
   - [ ] ID-MD-VDP-004: Criar mais testes unitários para cenários complexos

4. **Áudio**
   - [ ] ID-MD-AUD-001: Melhorar precisão do YM2612 em modos complexos
   - [ ] ID-MD-AUD-002: Implementar ferramentas de visualização para debug
   - [ ] ID-MD-AUD-003: Otimizar desempenho em dispositivos de baixo recurso

5. **Sistema**
   - [ ] ID-MD-SYS-001: Implementar suporte ao TMSS (verificação de marca registrada)
   - [ ] ID-MD-SYS-002: Melhorar precisão do sistema de DMA
   - [ ] ID-MD-SYS-003: Documentar melhor a integração entre componentes

### 3.3 Master System

**Status Geral**: 95% implementado

#### 3.3.1 Componentes

| Componente | Status | Precisão | Problemas Conhecidos | Responsáveis |
|------------|--------|----------|---------------------|--------------|
| CPU (Z80) | 100% | Alta | Nenhum problema significativo | @dev_team_z80 |
| VDP | 95% | Alta | Pequenos ajustes em efeitos especiais | @dev_team_sms_video |
| PSG | 100% | Alta | Nenhum problema significativo | @dev_team_audio |
| Sistema de Memória | 95% | Alta | Alguns mappers específicos precisam de ajustes | @dev_team_sms |
| Controladores | 90% | Alta | Suporte a periféricos especiais em desenvolvimento | @dev_team_sms |

#### 3.3.2 Compatibilidade

- **Jogos Comerciais**: ~95% funcionam corretamente
- **Demos/Homebrews**: ~98% funcionam corretamente
- **Principais problemas**: Alguns jogos com efeitos especiais de VDP, periféricos não padrão

#### 3.3.3 Tarefas Pendentes Detalhadas (Master System)

1. **VDP** - 95% Concluído
   - [x] ID-SMS-VDP-001: Implementação básica do VDP ✅
   - [x] ID-SMS-VDP-002: Suporte a todos os modos de vídeo ✅
   - [x] ID-SMS-VDP-003: Implementação de sprites com todas as funcionalidades ✅
   - [x] ID-SMS-VDP-004: Suporte a interrupções (HBLANK, VBLANK) ✅
   - [x] ID-SMS-VDP-005: Implementação precisa de timing do VDP ✅
   - [ ] ID-SMS-VDP-006: Ajustes em efeitos especiais raramente usados 🔄

2. **Z80** - 100% Concluído
   - [x] ID-SMS-Z80-001: Implementação completa do Z80 ✅
   - [x] ID-SMS-Z80-002: Integração com sistema de memória ✅
   - [x] ID-SMS-Z80-003: Suporte a interrupções ✅
   - [x] ID-SMS-Z80-004: Testes de conformidade com jogos comerciais ✅
   - [x] ID-SMS-Z80-005: Suporte a timing preciso para instruções ✅

3. **PSG** - 100% Concluído
   - [x] ID-SMS-PSG-001: Implementação básica do SN76489 ✅
   - [x] ID-SMS-PSG-002: Suporte aos 3 canais de tom ✅
   - [x] ID-SMS-PSG-003: Implementação do canal de ruído ✅
   - [x] ID-SMS-PSG-004: Implementação da tabela de volume logarítmica ✅
   - [x] ID-SMS-PSG-005: Integração com o sistema geral de áudio ✅

4. **Sistema de Memória** - 95% Concluído
   - [x] ID-SMS-MEM-001: Implementação básica do sistema de memória ✅
   - [x] ID-SMS-MEM-002: Suporte a ROMs de vários tamanhos ✅
   - [x] ID-SMS-MEM-003: Implementação do sistema de paginação ✅
   - [x] ID-SMS-MEM-004: Suporte a SRAM para salvamento ✅
   - [ ] ID-SMS-MEM-005: Suporte a mappers especiais raros 🔄

5. **Controladores e I/O** - 90% Concluído
   - [x] ID-SMS-IO-001: Suporte a controles padrão ✅
   - [x] ID-SMS-IO-002: Implementação do teclado para SG-1000 ✅
   - [x] ID-SMS-IO-003: Implementação de portas I/O padrão ✅
   - [ ] ID-SMS-IO-004: Suporte a periféricos especiais (Light Phaser, etc.) 🔄
   - [ ] ID-SMS-IO-005: Integração com dispositivos de entrada modernos 🔄

### 3.4 SNES

**Status Geral**: 5% implementado

#### 3.4.1 Componentes

| Componente | Status | Precisão | Problemas Conhecidos | Responsáveis |
|------------|--------|----------|---------------------|--------------|
| CPU (M68000) | 85% | Alta | Algumas instruções específicas | @dev_team_m68k |
| Z80 (Som) | 90% | Alta | Timing de integração com 68000 | @dev_team_z80 |
| VDP | 70% | Média | Efeitos de sombra/highlight, window | @dev_team_sn_video |
| YM2612 | 85% | Média | Alguns modos FM específicos | @dev_team_audio |
| SN76489 | 95% | Alta | - | @dev_team_audio |
| Sistema de Memória | 80% | Média | Algumas operações DMA complexas | @dev_team_sn_memory |
| Controladores | 90% | Alta | Suporte a periféricos especiais | @dev_team_sn |

#### 3.4.2 Compatibilidade

- **Jogos Comerciais**: ~75% funcionam corretamente
- **Demos/Homebrews**: ~80% funcionam corretamente
- **Principais problemas**: Jogos que usam efeitos avançados de VDP, timing preciso de Z80/68000

#### 3.4.3 Tarefas Pendentes Detalhadas (SNES)

1. **CPU (M68000)**
   - [ ] ID-SN-68K-001: Implementar instruções restantes
   - [ ] ID-SN-68K-002: Corrigir timing de acesso à memória
   - [ ] ID-SN-68K-003: Adicionar mais testes de exceções
   - [ ] ID-SN-68K-004: Otimizar ciclos para instruções comuns

2. **Z80 (Som)**
   - [ ] ID-SN-Z80-001: Melhorar integração com barramento do 68000
   - [ ] ID-SN-Z80-002: Implementar ciclos de espera (wait states) precisos
   - [ ] ID-SN-Z80-003: Melhorar manipulação de interrupções

3. **VDP**
   - [ ] ID-SN-VDP-001: Implementar modo shadow/highlight corretamente
   - [ ] ID-SN-VDP-002: Corrigir implementação da window feature
   - [ ] ID-SN-VDP-003: Melhorar precisão dos modos de interlace
   - [ ] ID-SN-VDP-004: Criar mais testes unitários para cenários complexos

4. **Áudio**
   - [ ] ID-SN-AUD-001: Melhorar precisão do YM2612 em modos complexos
   - [ ] ID-SN-AUD-002: Implementar ferramentas de visualização para debug
   - [ ] ID-SN-AUD-003: Otimizar desempenho em dispositivos de baixo recurso

5. **Sistema**
   - [ ] ID-SN-SYS-001: Implementar suporte ao TMSS (verificação de marca registrada)
   - [ ] ID-SN-SYS-002: Melhorar precisão do sistema de DMA
   - [ ] ID-SN-SYS-003: Documentar melhor a integração entre componentes

## 4. FERRAMENTAS DE DESENVOLVIMENTO

### 4.1 Ferramentas de Debug

**Status Geral**: 65% implementado

#### 4.1.1 Componentes

| Componente | Status | Status de Código | Responsáveis |
|------------|--------|------------------|--------------|
| Debugger de CPU | 85% | 🚧 Parcial | @dev_team_debug |
| Memory Viewer/Editor | 80% | 🚧 Parcial | @dev_team_debug |
| Disassembler | 85% | 🚧 Parcial | @dev_team_debug |
| Visualização de PPU/VDP | 60% | 🔌 **Stub** | @dev_team_debug |
| Visualização de Áudio | 50% | 🔌 **Stub** | @dev_team_debug |
| Trace Logger | 90% | 🚧 Parcial | @dev_team_debug |
| Breakpoints Condicionais | 85% | 🚧 Parcial | @dev_team_debug |

> **NOTA**: Componentes marcados com 🔌 **Stub** possuem apenas estrutura básica para compilação, sem funcionalidade real implementada.

#### 4.1.2 Tarefas Pendentes Detalhadas (Ferramentas de Debug)

1. **Debugger de CPU**
   - [x] ID-DBG-CPU-001: Melhorar interface de inspeção de registradores
   - [x] ID-DBG-CPU-002: Implementar step-over/step-out para todas as CPUs
   - [ ] ID-DBG-CPU-003: Adicionar visualização de pilha

2. **Memory Viewer/Editor**
   - [x] ID-DBG-MEM-001: Implementar capacidades avançadas de visualização
   - [x] ID-DBG-MEM-002: Adicionar suporte a histórico de alterações de memória
   - [ ] ID-DBG-MEM-003: Desenvolver interface gráfica completa
   - [ ] ID-DBG-MEM-004: Adicionar ferramentas de busca avançada

3. **Breakpoints Condicionais**
   - [x] ID-DBG-BRK-001: Implementar sistema robusto de breakpoints
   - [x] ID-DBG-BRK-002: Adicionar suporte a diversos tipos de breakpoints
   - [x] ID-DBG-BRK-003: Implementar watchpoints para monitoramento de memória
   - [ ] ID-DBG-BRK-004: Implementar avaliador de expressões para breakpoints complexos

4. **Visualizadores Específicos**
   - [ ] ID-DBG-VIS-001: Criar visualizador específico para CPU
   - [ ] ID-DBG-VIS-002: Desenvolver visualizador de vídeo (PPU/VDP)
   - [ ] ID-DBG-VIS-003: Implementar visualizador de áudio

### 4.2 Ferramentas de Teste

**Status Geral**: 75% implementado

#### 4.2.1 Componentes

| Componente | Status | Responsáveis |
|------------|--------|--------------|
| Testes Unitários | 80% | Todos |
| Testes de Integração | 70% | @dev_team_test |
| Testes de Conformidade | 75% | @dev_team_test |
| Benchmarks | 60% | @dev_team_test |
| CI/CD | 85% | @dev_team_infra |

#### 4.2.2 Tarefas Pendentes Detalhadas (Ferramentas de Teste)

1. **Testes Unitários**
   - [ ] ID-TEST-UNIT-001: Aumentar cobertura para componentes críticos
   - [ ] ID-TEST-UNIT-002: Criar testes específicos para bugs conhecidos
   - [ ] ID-TEST-UNIT-003: Implementar testes para todos os opcodes de CPU

2. **Testes de Integração**
   - [ ] ID-TEST-INT-001: Implementar testes de sistema para cada plataforma
   - [ ] ID-TEST-INT-002: Criar testes para interações entre componentes
   - [ ] ID-TEST-INT-003: Melhorar validação automática de resultados

3. **Testes de Conformidade**
   - [ ] ID-TEST-CONF-001: Implementar testes ZEXALL para Z80
   - [ ] ID-TEST-CONF-002: Adaptação de testes NESTEST completos
   - [ ] ID-TEST-CONF-003: Implementar testes de hardware real comparativos

4. **CI/CD**
   - [ ] ID-TEST-CICD-001: Melhorar pipeline de testes automatizados
   - [ ] ID-TEST-CICD-002: Implementar análise de cobertura de código
   - [ ] ID-TEST-CICD-003: Adicionar detecção automática de regressões

## 5. INTERFACE DE USUÁRIO

### 5.1 Interface Gráfica

**Status Geral**: 60% implementado

#### 5.1.1 Componentes

| Componente | Status | Responsáveis |
|------------|--------|--------------|
| Interface Básica | 85% | @dev_team_ui |
| Gerenciamento de ROMs | 70% | @dev_team_ui |
| Configurações | 65% | @dev_team_ui |
| Mapeamento de Controles | 75% | @dev_team_ui |
| Shaders/Filtros | 40% | @dev_team_ui |
| Interface Específica por Plataforma | 30% | @dev_team_ui |

#### 5.1.2 Tarefas Pendentes Detalhadas (Interface Gráfica)

1. **Interface Básica**
   - [ ] ID-UI-BASIC-001: Melhorar layout responsivo
   - [ ] ID-UI-BASIC-002: Adicionar suporte para múltiplos idiomas
   - [ ] ID-UI-BASIC-003: Implementar sistema de temas

2. **Gerenciamento de ROMs**
   - [ ] ID-UI-ROM-001: Melhorar detecção automática de plataforma
   - [ ] ID-UI-ROM-002: Adicionar visualização de informações da ROM
   - [ ] ID-UI-ROM-003: Implementar sistema de favoritos/tags

3. **Configurações**
   - [ ] ID-UI-CFG-001: Reorganizar opções em categorias lógicas
   - [ ] ID-UI-CFG-002: Adicionar presets para configurações comuns
   - [ ] ID-UI-CFG-003: Implementar perfis de configuração

4. **Shaders/Filtros**
   - [ ] ID-UI-SHD-001: Implementar shaders CRT básicos
   - [ ] ID-UI-SHD-002: Adicionar filtros de escalamento (xBR, HQ2x)
   - [ ] ID-UI-SHD-003: Suporte a shaders personalizados

### 5.2 Recursos Avançados

**Status Geral**: 40% implementado

#### 5.2.1 Componentes

| Componente | Status | Responsáveis |
|------------|--------|--------------|
| Save States | 85% | @dev_team_core |
| Cheats | 45% | @dev_team_cheats |
| Screenshots/Gravação | 60% | @dev_team_media |
| Netplay | 10% | @dev_team_netplay |
| Traduções/Patches | 30% | @dev_team_patches |

#### 5.2.2 Tarefas Pendentes Detalhadas (Recursos Avançados)

1. **Save States**
   - [x] ID-ADV-SAVE-001: Implementar sistema básico de save/load
   - [x] ID-ADV-SAVE-002: Sistema de compressão de dados
   - [x] ID-ADV-SAVE-003: Registro de metadados (tempo, data, etc)
   - [x] ID-ADV-SAVE-004: Adicionar suporte para auto-save configurável
   - [x] ID-ADV-SAVE-005: Criar funções para exportação/importação de save states
   - [x] ID-ADV-SAVE-006: Implementar compressão delta para dados de memória
   - [x] ID-ADV-SAVE-007: Implementar compressão Zstandard para estado da GPU
   - [x] ID-ADV-SAVE-008: Adicionar thumbnails WebP com tarja "Save"
   - [x] ID-ADV-SAVE-009: Implementar metadados expandidos (região, versão, notas, tags)
   - [x] ID-ADV-SAVE-010: Otimizar buffer circular para rewind (200 snapshots)
   - [x] ID-ADV-SAVE-011: Implementar efeito visual de rewind (PB, velocidade 0.5x)
   - [ ] ID-ADV-SAVE-012: Criar sistema de perfis com armazenamento SQLite
   - [ ] ID-ADV-SAVE-013: Implementar integração com serviços de nuvem (Google Drive/OneDrive)
   - [ ] ID-ADV-SAVE-014: Adicionar resolução de conflitos para sincronização na nuvem
   - [ ] ID-ADV-SAVE-015: Implementar compartilhamento em redes sociais
   - [ ] ID-ADV-SAVE-016: Adicionar verificação de integridade via checksums SHA-256
   - [ ] ID-ADV-SAVE-017: Implementar encriptografia AES-256 para dados sensíveis
   - [ ] ID-ADV-SAVE-018: Criar interface avançada para gerenciamento de saves

2. **Cheats**
   - [ ] ID-ADV-CHT-001: Implementar sistema de busca de valores
   - [ ] ID-ADV-CHT-002: Suporte a formatos comuns (Game Genie, etc)
   - [ ] ID-ADV-CHT-003: Interface para criação/edição de cheats

3. **Netplay**
   - [ ] ID-ADV-NET-001: Implementar sincronização básica
   - [ ] ID-ADV-NET-002: Adicionar lobby/sala de espera
   - [ ] ID-ADV-NET-003: Implementar spectator mode

#### 5.2.3 Plano de Implementação em Fases para Save States

**Fase 1: Core Features (2 semanas) - CONCLUÍDA**

- [x] Implementar geração de thumbnail WebP com tarja
- [x] Adicionar metadados básicos (tempo de jogo, saves/loads, conquistas)
- [x] Otimizar buffer circular com 200 snapshots para rewind

**Fase 2: Usabilidade e Segurança (3 semanas) - EM ANDAMENTO**

- [ ] Desenvolver interface de gerenciamento de saves
- [x] Implementar compressão delta para memória
- [ ] Criar sistema de perfis com armazenamento SQLite

**Fase 3: Recursos Avançados (4 semanas) - PLANEJADA**

- [ ] Implementar integração com serviços de nuvem
- [ ] Adicionar verificação de integridade via checksums
- [ ] Implementar encriptografia para dados sensíveis
- [ ] Desenvolver sistema de compartilhamento e sincronização

#### 5.2.4 Sistema de Save States Multi-plataforma

**Mega Drive - CONCLUÍDO**

- [x] Implementação robusta com todas as funcionalidades avançadas
- [x] Suporte a todos os tipos de mappers e hardware especial
- [x] Integração completa com o sistema de rewind

**NES - EM IMPLEMENTAÇÃO**

- [x] Arquitetura base compatível com Mega Drive
- [x] Compressão delta para dados de memória
- [x] Thumbnails WebP com tarja "Save"
- [x] Metadados expandidos
- [x] Sistema de rewind otimizado
- [x] Efeito visual de rewind
- [x] Integração com Mappers
- [ ] Testes de compatibilidade com todos os mappers

**Master System - PLANEJADO**

- [ ] Adaptação do sistema base para Master System
- [ ] Suporte a cartuchos com SRAM
- [ ] Integração com chip FM (versão japonesa)

**SNES - PLANEJADO**

- [ ] Adaptação do sistema base para SNES
- [ ] Suporte a cartuchos com bateria e chips especiais
- [ ] Suporte a SRAM, FRAM e outros tipos de memória de save

### 5.3 Integração com Outros Sistemas

## 6. OTIMIZAÇÕES E PERFORMANCE

### 6.1 Otimizações Gerais

**Status Geral**: 55% implementado

#### 6.1.1 Componentes

| Componente | Status | Responsáveis |
|------------|--------|--------------|
| Renderização | 65% | @dev_team_render |
| Emulação de CPU | 70% | @dev_team_cpu_opt |
| Subsistema de Áudio | 50% | @dev_team_audio_opt |
| Gerenciamento de Memória | 60% | @dev_team_mem_opt |
| Recursos para Dispositivos Móveis | 30% | @dev_team_mobile |

#### 6.1.2 Tarefas Pendentes Detalhadas (Otimizações)

1. **Renderização**
   - [ ] ID-OPT-RND-001: Implementar renderização por hardware onde possível
   - [ ] ID-OPT-RND-002: Otimizar pipeline de renderização
   - [ ] ID-OPT-RND-003: Reduzir overhead em resolução nativa

2. **Emulação de CPU**
   - [ ] ID-OPT-CPU-001: Implementar JIT para plataformas principais
   - [ ] ID-OPT-CPU-002: Otimizar decodificação de instruções comuns
   - [ ] ID-OPT-CPU-003: Cache de instruções para todas as CPUs

3. **Subsistema de Áudio**
   - [ ] ID-OPT-AUD-001: Otimizar geração de samples
   - [ ] ID-OPT-AUD-002: Implementar buffer adaptativo
   - [ ] ID-OPT-AUD-003: Reduzir overhead de sincronização

## 7. FRONTEND E INTERFACES DE USUÁRIO

### 7.1 Arquitetura Frontend

**Status Geral**: 55% implementado (↑5% desde a última atualização)

#### 7.1.1 Componentes

| Componente | Status | Responsáveis | Descrição |
|------------|--------|--------------|-----------|
| Core UI Framework | 85% | @dev_team_ui | Framework base de UI cross-platform |
| SDL Integration | 90% | @dev_team_ui | Integração com bibliotecas SDL para renderização |
| Renderização Multiplatforma | 75% ↑ | @dev_team_ui | Adaptadores para diferentes sistemas operacionais |
| Gerenciador de Temas | 40% | @dev_team_ui | Sistema de temas e personalização visual |
| Acessibilidade | 25% | @dev_team_ui | Recursos para acessibilidade (alto contraste, leitor de tela) |
| Input Management | 80% | @dev_team_ui | Sistema para gerenciamento de entrada em diferentes plataformas |
| Electron Integration | 80% ↑ | @dev_team_electron | Integração com Electron para modo desktop |

#### 7.1.2 Tarefas Pendentes Detalhadas (Frontend)

1. **Core UI Framework**
   - [x] ID-FE-CORE-001: Refatorar componentes reutilizáveis ✅
   - [ ] ID-FE-CORE-002: Implementar sistema de layout responsivo 🔄
   - [ ] ID-FE-CORE-003: Otimizar performance em interfaces complexas

2. **Renderização Multiplatforma**
   - [x] ID-FE-RENDER-001: Melhorar compatibilidade com OpenGL ES ✅
   - [x] ID-FE-RENDER-002: Implementar fallbacks para hardware limitado ✅
   - [ ] ID-FE-RENDER-003: Adicionar suporte a DirectX quando disponível
   - [ ] ID-FE-RENDER-004: Otimizar renderização em dispositivos móveis

3. **Gerenciador de Temas**
   - [ ] ID-FE-THEME-001: Implementar sistema de temas completo
   - [ ] ID-FE-THEME-002: Criar temas específicos para cada plataforma emulada
   - [ ] ID-FE-THEME-003: Permitir personalização pelo usuário
   - [ ] ID-FE-THEME-004: Implementar exportação/importação de temas

4. **Acessibilidade**
   - [ ] ID-FE-ACCESS-001: Implementar suporte a leitores de tela
   - [ ] ID-FE-ACCESS-002: Adicionar modos de alto contraste
   - [ ] ID-FE-ACCESS-003: Suporte a controles alternativos
   - [ ] ID-FE-ACCESS-004: Testes com diferentes perfis de usuários

5. **Integração Electron**
   - [x] ID-FE-ELECTRON-001: Integração básica com Electron ✅
   - [x] ID-FE-ELECTRON-002: Exposição segura de APIs nativas via preload ✅
   - [x] ID-FE-ELECTRON-003: Correção de problemas de renderização ✅
   - [x] ID-FE-ELECTRON-004: Otimização do processo de inicialização ✅
   - [ ] ID-FE-ELECTRON-005: Implementação de menu nativo do sistema 🔄
   - [ ] ID-FE-ELECTRON-006: Suporte a notificações nativas do sistema

### 7.2 Interfaces Específicas de Plataforma

**Status Geral**: 50% implementado (↑5% desde a última atualização)

#### 7.2.1 Componentes

| Componente | Status | Responsáveis | Descrição |
|------------|--------|--------------|-----------|
| Desktop UI (Windows/Linux/Mac) | 80% ↑ | @dev_team_desktop | Interface completa para desktop |
| Mobile UI (Android/iOS) | 20% | @dev_team_mobile | Interface otimizada para touch e telas pequenas |
| Web Interface | 10% | @dev_team_web | Versão web para acesso via navegador |
| Console Mode | 30% | @dev_team_console | Interface minimalista para uso em consoles/embarcados |
| Fullscreen Mode | 85% ↑ | @dev_team_desktop | Modo de tela cheia otimizado |

#### 7.2.2 Tarefas Pendentes Detalhadas (Interfaces Específicas)

1. **Desktop UI**
   - [x] ID-FE-DESK-001: Melhorar adaptação para múltiplos monitores ✅
   - [ ] ID-FE-DESK-002: Implementar sistema de janelas flutuantes 🔄
   - [ ] ID-FE-DESK-003: Adicionar suporte a atalhos de teclado personalizáveis

2. **Mobile UI**
   - [ ] ID-FE-MOB-001: Desenvolver controles touch otimizados
   - [ ] ID-FE-MOB-002: Implementar gestos para ações comuns
   - [ ] ID-FE-MOB-003: Otimizar layout para diferentes tamanhos de tela
   - [ ] ID-FE-MOB-004: Suporte a gamepads externos

3. **Web Interface**
   - [ ] ID-FE-WEB-001: Implementar versão básica funcional
   - [ ] ID-FE-WEB-002: Integrar com sistema de armazenamento remoto
   - [ ] ID-FE-WEB-003: Otimizar para diferentes browsers
   - [ ] ID-FE-WEB-004: Implementar controles via teclado/mouse

4. **Console Mode**
   - [ ] ID-FE-CON-001: Desenvolver interface minimalista
   - [ ] ID-FE-CON-002: Otimizar para controles de gamepad
   - [ ] ID-FE-CON-003: Implementar navegação eficiente via D-pad

5. **Fullscreen Mode**
   - [x] ID-FE-FULL-001: Implementar transição suave para modo fullscreen ✅
   - [x] ID-FE-FULL-002: Otimizar proporções para diferentes resoluções ✅
   - [ ] ID-FE-FULL-003: Implementar UI minimalista sobreposta 🔄
   - [ ] ID-FE-FULL-004: Suporte a múltiplos monitores em fullscreen

## 8. FERRAMENTAS DE DESENVOLVIMENTO AVANÇADAS

### 8.1 Ferramentas Visuais de Desenvolvimento

**Status Geral**: 35% implementado

#### 8.1.1 Componentes

| Ferramenta | Status | Status de Código | Responsáveis | Descrição |
|------------|--------|------------------|--------------|-----------|
| Sprite Viewer | 60% | 🔌 **Stub** | @dev_team_tools | Visualizador de sprites e padrões de tile |
| Dev Art Tools | 25% | 🔌 **Stub** | @dev_team_art | Ferramentas para criação/visualização de arte |
| Dev Editor | 30% | 🔌 **Stub** | @dev_team_editor | Editor integrado para desenvolvimento de ROMs |
| Event Viewer | 40% | 🔌 **Stub** | @dev_team_events | Visualizador de eventos do sistema emulado |
| Memory Viewer | 70% | 🔌 **Stub** | @dev_team_mem | Visualizador/editor avançado de memória |
| Node IDE | 15% | 🔌 **Stub** | @dev_team_node | Ambiente de desenvolvimento integrado baseado em nodos |
| Sound Monitor | 45% | 🔌 **Stub** | @dev_team_audio | Ferramenta de análise e visualização de áudio |

> **IMPORTANTE**: Todas as ferramentas listadas acima possuem apenas implementações mínimas (stubs) - estruturas básicas para permitir a compilação, mas sem funcionalidade completa. Não confundir a porcentagem de progresso com implementação real.

#### 8.1.2 Tarefas Pendentes Detalhadas (Ferramentas Visuais)

1. **Sprite Viewer**
   - [ ] ID-TOOL-SPR-001: Implementar suporte a todos os formatos de sprite
   - [ ] ID-TOOL-SPR-002: Adicionar extração de paleta em tempo real
   - [ ] ID-TOOL-SPR-003: Permitir exportação em formatos modernos (PNG, GIF)
   - [ ] ID-TOOL-SPR-004: Visualização de animações de sprites
   - [ ] ID-TOOL-SPR-005: Integrar com mapa de VRAM em tempo real

2. **Dev Art Tools**
   - [ ] ID-TOOL-ART-001: Implementar editor de paletas
   - [ ] ID-TOOL-ART-002: Criar editor de tiles/padrões
   - [ ] ID-TOOL-ART-003: Desenvolver ferramenta de conversão de formatos gráficos
   - [ ] ID-TOOL-ART-004: Implementar preview em tempo real no emulador
   - [ ] ID-TOOL-ART-005: Adicionar suporte a importação de imagens modernas

3. **Dev Editor**
   - [ ] ID-TOOL-EDIT-001: Implementar editor de código assembly
   - [ ] ID-TOOL-EDIT-002: Adicionar suporte a montagem em tempo real
   - [ ] ID-TOOL-EDIT-003: Integrar com ferramentas de debug
   - [ ] ID-TOOL-EDIT-004: Implementar destaque de sintaxe para diferentes CPUs
   - [ ] ID-TOOL-EDIT-005: Criar wizards para operações comuns

4. **Event Viewer**
   - [ ] ID-TOOL-EVT-001: Visualização de timeline de eventos
   - [ ] ID-TOOL-EVT-002: Filtros para tipos específicos de eventos
   - [ ] ID-TOOL-EVT-003: Análise de performance baseada em eventos
   - [ ] ID-TOOL-EVT-004: Exportação de logs de eventos
   - [ ] ID-TOOL-EVT-005: Trigger de breakpoints baseados em eventos

5. **Memory Viewer**
   - [ ] ID-TOOL-MEM-001: Adicionar visualização em diferentes formatos (hex, decimal, ASCII)
   - [ ] ID-TOOL-MEM-002: Implementar histórico de alterações
   - [ ] ID-TOOL-MEM-003: Criação de bookmarks e anotações
   - [ ] ID-TOOL-MEM-004: Busca avançada com padrões
   - [ ] ID-TOOL-MEM-005: Visualização diferenciada por região de memória

6. **Node IDE**
   - [ ] ID-TOOL-NODE-001: Criar interface básica do IDE baseado em nodos
   - [ ] ID-TOOL-NODE-002: Implementar sistema de nodos para lógica visual
   - [ ] ID-TOOL-NODE-003: Desenvolver conjunto básico de nodos
   - [ ] ID-TOOL-NODE-004: Integrar com sistema de debug
   - [ ] ID-TOOL-NODE-005: Permitir exportação/importação de projetos

7. **Sound Monitor**
   - [ ] ID-TOOL-SND-001: Implementar visualização em tempo real de canais de áudio
   - [ ] ID-TOOL-SND-002: Adicionar analisador de espectro
   - [ ] ID-TOOL-SND-003: Criar ferramenta de composição de áudio
   - [ ] ID-TOOL-SND-004: Permitir extrair/salvar samples
   - [ ] ID-TOOL-SND-005: Visualizar registradores de áudio em tempo real

### 8.2 Ferramentas de Desenvolvimento Integradas

**Status Geral**: 40% implementado

#### 8.2.1 Componentes

| Ferramenta | Status | Status de Código | Responsáveis | Descrição |
|------------|--------|------------------|--------------|-----------|
| Dev Tools Suite | 50% | 🔌 **Stub** | @dev_team_tools | Suite integrada de ferramentas de desenvolvimento |
| ROM Analyzer | 65% | 🔌 **Stub** | @dev_team_analysis | Ferramenta para análise estática de ROMs |
| Patch Creator | 35% | 🔌 **Stub** | @dev_team_patch | Criador de patches e hacks para ROMs |
| ROM Builder | 45% | 🔌 **Stub** | @dev_team_build | Sistema de build para desenvolvimento de ROMs |
| Asset Manager | 30% | 🔌 **Stub** | @dev_team_assets | Gerenciador de assets (gráficos, áudio, etc.) |

> **ATENÇÃO DESENVOLVEDORES**: A porcentagem de status indica o progresso do design e arquitetura, não a implementação real. Todas estas ferramentas ainda precisam de implementação completa.

#### 8.2.2 Tarefas Pendentes Detalhadas (Ferramentas Integradas)

1. **Dev Tools Suite**
   - [ ] ID-TOOL-SUITE-001: Integrar todas as ferramentas em uma interface comum
   - [ ] ID-TOOL-SUITE-002: Implementar sistema de plugins para extensibilidade
   - [ ] ID-TOOL-SUITE-003: Criar sistema de comunicação entre ferramentas
   - [ ] ID-TOOL-SUITE-004: Adicionar perfis de workspace para diferentes tipos de desenvolvimento

2. **ROM Analyzer**
   - [ ] ID-TOOL-ANAL-001: Melhorar disassembly automático
   - [ ] ID-TOOL-ANAL-002: Implementar detecção de padrões comuns
   - [ ] ID-TOOL-ANAL-003: Adicionar análise de utilização de recursos
   - [ ] ID-TOOL-ANAL-004: Criar relatório detalhado de compatibilidade

3. **Patch Creator**
   - [ ] ID-TOOL-PATCH-001: Implementar criação de patches IPS/BPS
   - [ ] ID-TOOL-PATCH-002: Adicionar editor visual de patches
   - [ ] ID-TOOL-PATCH-003: Criar sistema de versionamento de patches
   - [ ] ID-TOOL-PATCH-004: Permitir patches condicionais baseados em checksum

4. **ROM Builder**
   - [ ] ID-TOOL-BUILD-001: Criar sistema de projetos para ROMs
   - [ ] ID-TOOL-BUILD-002: Implementar pipeline de build configurável
   - [ ] ID-TOOL-BUILD-003: Adicionar templates para diferentes plataformas
   - [ ] ID-TOOL-BUILD-004: Integrar com ferramentas de terceiros (montadores, compiladores)

5. **Asset Manager**
   - [ ] ID-TOOL-ASSET-001: Implementar sistema de importação de assets
   - [ ] ID-TOOL-ASSET-002: Criar sistema de organização de assets
   - [ ] ID-TOOL-ASSET-003: Adicionar conversores para diferentes formatos
   - [ ] ID-TOOL-ASSET-004: Integrar com pipeline de build

### 8.3 Infraestrutura de Desenvolvimento

**Status Geral**: 55% implementado

#### 8.3.1 Componentes

| Componente | Status | Responsáveis | Descrição |
|------------|--------|--------------|-----------|
| API para Plugins | 40% | @dev_team_api | API para desenvolvimento de plugins de terceiros |
| Ferramentas de CI/CD | 70% | @dev_team_devops | Ferramentas para integração e deploy contínuos |
| Documentação Automática | 45% | @dev_team_docs | Geração automática de documentação |
| Telemetria e Analytics | 35% | @dev_team_telemetry | Coleta e análise de dados de uso |
| Sistema de Feedback | 25% | @dev_team_feedback | Infraestrutura para feedback de usuários |

#### 8.3.2 Tarefas Pendentes Detalhadas (Infraestrutura)

1. **API para Plugins**
   - [ ] ID-INFRA-API-001: Definir interfaces estáveis para plugins
   - [ ] ID-INFRA-API-002: Criar sistema de gerenciamento de plugins
   - [ ] ID-INFRA-API-003: Implementar sandbox para segurança
   - [ ] ID-INFRA-API-004: Desenvolver documentação e exemplos

2. **Ferramentas de CI/CD**
   - [ ] ID-INFRA-CICD-001: Melhorar pipeline de testes automatizados
   - [ ] ID-INFRA-CICD-002: Implementar deploy automático para múltiplas plataformas
   - [ ] ID-INFRA-CICD-003: Criar verificações de qualidade de código
   - [ ] ID-INFRA-CICD-004: Adicionar benchmarks automatizados

3. **Documentação Automática**
   - [ ] ID-INFRA-DOC-001: Melhorar extração de documentação do código
   - [ ] ID-INFRA-DOC-002: Implementar geração de diagramas
   - [ ] ID-INFRA-DOC-003: Criar sistema de publicação de documentação
   - [ ] ID-INFRA-DOC-004: Adicionar validação de exemplos de código

## 9. DIVISÃO DE TAREFAS PARA MODELOS DE BAIXO CONTEXTO

Esta seção divide as tarefas em unidades menores e autocontidas, ideais para processamento por modelos com capacidade limitada de contexto.

### 9.1 Tarefas de Documentação

Cada tarefa deve ser autocontida e incluir exemplos sempre que possível.

| ID | Descrição | Prioridade | Complexidade | Prazo |
|----|-----------|------------|--------------|-------|
| DOC-001 | Documentar API completa do subsistema Z80 | Alta | Média | Junho 2025 |
| DOC-002 | Criar tutorial passo-a-passo para usar debugger | Média | Baixa | Maio 2025 |
| DOC-003 | Documentar formato de save states | Média | Baixa | Maio 2025 |
| DOC-004 | Atualizar diagramas de arquitetura do sistema | Alta | Média | Abril 2025 |
| DOC-005 | Criar guia de configuração para desenvolvedores | Alta | Baixa | Abril 2025 |

### 9.2 Tarefas de Implementação Unitária

Cada tarefa deve ser isolada e implementável sem conhecimento profundo do sistema completo.

| ID | Descrição | Plataforma | Prioridade | Complexidade | Prazo |
|----|-----------|------------|------------|--------------|-------|
| IMP-001 | Implementar opcode 0x27 (DAA) para Z80 | Core | Alta | Baixa | Abril 2025 |
| IMP-002 | Criar visualizador de sprites para debug | Ferramentas | Média | Média | Maio 2025 |
| IMP-003 | Implementar filtro CRT básico | UI | Baixa | Média | Junho 2025 |
| IMP-004 | Corrigir cálculo de timing em instruções paginadas | NES | Alta | Média | Abril 2025 |
| IMP-005 | Implementar detecção de cabeçalho iNES 2.0 | NES | Média | Baixa | Maio 2025 |

### 9.3 Tarefas de Teste

Testes específicos que podem ser escritos sem conhecimento completo do sistema.

| ID | Descrição | Componente | Prioridade | Complexidade | Prazo |
|----|-----------|------------|------------|--------------|-------|
| TEST-001 | Criar testes para todos os modos de endereçamento do 6502 | NES | Alta | Média | Abril 2025 |
| TEST-002 | Implementar testes para opcodes não documentados do Z80 | Z80 | Média | Média | Maio 2025 |
| TEST-003 | Criar teste de regressão para bug de scrolling do PPU | NES | Alta | Baixa | Abril 2025 |
| TEST-004 | Testar timing preciso do VDP em diferentes modos | Mega Drive | Média | Alta | Junho 2025 |
| TEST-005 | Validar compatibilidade com suite ZEXALL | Z80 | Alta | Média | Maio 2025 |

## 10. MÉTRICAS DE PROGRESSO

### 10.1 Métricas por Plataforma

| Plataforma | Jogos Testados | Taxa de Compatibilidade | Bugs Críticos | Bugs Menores |
|------------|----------------|--------------------------|---------------|--------------|
| NES | 850 | 95% | 4 | 17 |
| Mega Drive | 420 | 75% | 12 | 35 |
| Master System | 120 | 20% | 18 | 40 |
| Game Boy | 0 | 0% | - | - |

### 10.2 Métricas de Código

| Métrica | Valor Atual | Meta |
|---------|-------------|------|
| Cobertura de Testes | 78% | 90% |
| Componentes Documentados | 65% | 95% |
| Módulos Refatorados | 40% | 90% |
| Commits por Semana | ~25 | >30 |
| Pull Requests Abertos | 12 | <10 |
| Issues Abertas | 57 | <30 |

## 11. CONTRIBUIÇÃO

### 11.1 Como Contribuir

Para contribuir com o projeto, siga estes passos:

1. Escolha uma tarefa do ROADMAP (preferencialmente marcada como "help wanted")
2. Crie um fork do repositório
3. Implemente a funcionalidade ou correção
4. Adicione testes adequados (unitários, integração)
5. Documente mudanças seguindo o padrão estabelecido
6. Envie um Pull Request referenciando a tarefa

#### 11.1.1 Contribuindo para Implementações Stub 🔌

Muitas ferramentas de desenvolvimento atualmente existem apenas como implementações mínimas (stubs). Ao trabalhar nestes componentes:

1. **Identifique as implementações stub** - Procure pelo símbolo 🔌 nas tabelas ou comentários como `// Implementação mínima` no código
2. **Leia a especificação completa** - Veja `docs/AI_GUIDELINE.md` para requisitos detalhados da ferramenta
3. **Implemente incrementalmente** - Adicione funcionalidades aos poucos, mantendo o código sempre compilável
4. **Atualize a documentação** - Após implementar, atualize o status de código no ROADMAP
5. **Comunique claramente** - Em seu PR, documente quais partes da implementação stub foram completadas

> **IMPORTANTE**: Ao converter um stub em implementação funcional, não remova comentários que indicam implementação mínima até que a implementação esteja 100% concluída.

### 11.2 Padrões de Código

Todo código contribuído deve seguir os padrões estabelecidos:

- Estilo de código consistente (ver `docs/STYLE_GUIDE.md`)
- Testes unitários para novas funcionalidades
- Documentação de API para interfaces públicas
- Mensagens de commit descritivas seguindo o formato estabelecido

## 12. ANEXOS

### 12.1 Glossário

- **CPU**: Central Processing Unit, o processador principal do sistema
- **PPU**: Picture Processing Unit, responsável pelo processamento de vídeo no NES
- **VDP**: Video Display Processor, processador de vídeo do Mega Drive/Master System
- **APU**: Audio Processing Unit, responsável pelo processamento de áudio
- **DMA**: Direct Memory Access, método para acessar memória diretamente sem CPU
- **Mapper**: Circuito no cartucho que expande as capacidades do console

### 12.2 Recursos e Referências

- Documentação técnica: `docs/technical/`
- Diagramas de arquitetura: `docs/architecture/`
- Especificações de hardware: `docs/hardware/`
- Testes de referência: `tests/reference/`

---

Última atualização: Abril 2025
Próxima revisão programada: Julho 2025
Responsável pela manutenção do ROADMAP: @tech_lead

## Detalhamento dos Componentes NES

### CPU 6502/2A03

**Status: 100% implementado**

A CPU do NES é baseada no processador 6502 com modificações para incluir o canal de áudio DMC. Agora está 100% implementada e testada.

Características:

- Todos os 151 opcodes oficiais implementados com precisão de timing
- Todos os 105 opcodes ilegais implementados com comportamento adequado
- Suporte preciso a operações de página cruzada
- Testes abrangentes para todos os opcodes, incluindo os ilegais
- Documentação detalhada sobre a implementação

*Nota (Maio 2024): Todas as tarefas pendentes da CPU foram concluídas. Os opcodes ilegais restantes foram implementados, o timing foi melhorado para operações de página cruzada, e testes unitários foram adicionados para verificar todos os casos de uso. Uma documentação detalhada foi criada em src/platforms/nes/documentation/cpu.md.*

## PPU (Picture Processing Unit)

**Status: 100% implementado**

A PPU do NES é responsável pela geração de vídeo e agora está completamente implementada com todos os recursos esperados.

Características:

- Renderização precisa de backgrounds e sprites
- Suporte a todos os modos de espelhamento (horizontal, vertical, de 4 telas, etc.)
- Gerenciamento preciso de paletas e cores
- Timing correto para geração de NMI e acesso à VRAM
- Efeitos avançados como split scrolling para títulos com HUD
- Suporte completo a diferentes sistemas de TV (NTSC, PAL, Dendy)
- Efeitos de overscan e distorção de CRT
- Testes abrangentes para todas as funcionalidades

*Nota (Maio 2024): Todas as tarefas pendentes da PPU foram concluídas. Problemas de scrolling foram corrigidos, efeitos de distorção (overscan) foram implementados, e suporte completo a paletas alternativas para sistemas PAL e Dendy foi adicionado. Uma documentação detalhada foi criada em src/platforms/nes/documentation/ppu.md.*

## 5. IMPLEMENTAÇÃO TÉCNICA DA MIGRAÇÃO FRONTEND

### 5.1 Comunicação com o Emulador

A nova arquitetura implementará uma comunicação robusta entre o backend (emulador C/C++) e o frontend (React/TypeScript) através de dois canais principais:

#### 5.1.1 WebSocket para Dados em Tempo Real

**Status**: 📝 Planejado (Q1-Q2 2024)

| Componente | Descrição | Prioridade | Responsáveis |
|------------|-----------|------------|--------------|
| Frame Streaming | Transmissão eficiente de frames renderizados | Alta | @frontend_team, @bridge_team |
| Input Events | Envio de comandos de controle em tempo real | Alta | @frontend_team, @bridge_team |
| Emulator State | Sincronização do estado do emulador | Alta | @frontend_team, @bridge_team |
| Debug Data | Dados para ferramentas de desenvolvimento | Média | @tools_team |
| Audio Streaming | Transmissão de áudio em tempo real | Alta | @audio_team, @bridge_team |

**Implementação detalhada**:

1. **Servidor WebSocket (C/C++)**

   ```cpp
   // Implementação no backend (C/C++)
   typedef struct {
       uint8_t* frame_data;      // Dados do frame atual
       size_t frame_size;        // Tamanho em bytes
       uint32_t width;           // Largura em pixels
       uint32_t height;          // Altura em pixels
       uint64_t timestamp;       // Timestamp para sincronização
   } frame_data_t;

   // Função para enviar frame via WebSocket
   int ws_send_frame(ws_connection_t* conn, const frame_data_t* frame) {
       // Serialização e envio do frame
       json_object* obj = json_object_new_object();
       json_object_object_add(obj, "type", json_object_new_string("FRAME_DATA"));

       // Converter dados do frame para Base64 ou formato binário eficiente
       char* encoded_data = base64_encode(frame->frame_data, frame->frame_size);
       json_object_object_add(obj, "data", json_object_new_string(encoded_data));

       // Adicionar metadados
       json_object_object_add(obj, "width", json_object_new_int(frame->width));
       json_object_object_add(obj, "height", json_object_new_int(frame->height));
       json_object_object_add(obj, "timestamp", json_object_new_int64(frame->timestamp));

       // Enviar via WebSocket
       const char* json_str = json_object_to_json_string(obj);
       int result = ws_send_text(conn, json_str, strlen(json_str));

       // Limpeza
       json_object_put(obj);
       free(encoded_data);

       return result;
   }
   ```

2. **Cliente WebSocket (TypeScript/React)**

   ```typescript
   // Hook para comunicação WebSocket (frontend)
   function useEmulatorWebSocket(url: string) {
     const [frameData, setFrameData] = useState<FrameData | null>(null);
     const [connection, setConnection] = useState<WebSocket | null>(null);
     const [isConnected, setIsConnected] = useState(false);

     // Estabelecer conexão
     useEffect(() => {
       const ws = new WebSocket(url);

       ws.onopen = () => {
         console.log('WebSocket connected');
         setIsConnected(true);
       };

       ws.onclose = () => {
         console.log('WebSocket disconnected');
         setIsConnected(false);
       };

       ws.onmessage = (event) => {
         const message = JSON.parse(event.data);

         switch (message.type) {
           case 'FRAME_DATA':
             // Processar frame recebido
             const frameData: FrameData = {
               data: base64ToArrayBuffer(message.data),
               width: message.width,
               height: message.height,
               timestamp: message.timestamp
             };
             setFrameData(frameData);
             break;

           // Outros tipos de mensagens
           // ...
         }
       };

       setConnection(ws);

       // Cleanup na desmontagem
       return () => {
         ws.close();
       };
     }, [url]);

     // Função para enviar comandos para o emulador
     const sendCommand = useCallback((command: EmulatorCommand) => {
       if (connection && isConnected) {
         connection.send(JSON.stringify(command));
       }
     }, [connection, isConnected]);

     return {
       frameData,
       isConnected,
       sendCommand
     };
   }
   ```

#### 5.1.2 REST API para Operações Não-Tempo-Real

**Status**: 📝 Planejado (Q1-Q2 2024)

| Endpoint | Método | Descrição | Prioridade |
|----------|--------|-----------|------------|
| `/api/v1/roms` | GET | Lista ROMs disponíveis | Alta |
| `/api/v1/roms/:id` | GET | Detalhes da ROM | Média |
| `/api/v1/roms/:id/load` | POST | Carrega ROM no emulador | Alta |
| `/api/v1/states` | GET | Lista estados salvos | Alta |
| `/api/v1/states/:id` | GET | Detalhes do estado | Média |
| `/api/v1/states/:id/load` | POST | Carrega estado salvo | Alta |
| `/api/v1/states/save` | POST | Salva estado atual | Alta |
| `/api/v1/config` | GET | Obtém configurações | Alta |
| `/api/v1/config` | PUT | Atualiza configurações | Alta |

**Implementação detalhada**:

1. **Servidor HTTP (C/C++)**

   ```cpp
   // Implementação da API REST no backend
   void http_init_routes(http_server_t* server) {
       // Rota para listar ROMs
       http_register_route(server, HTTP_GET, "/api/v1/roms", handle_list_roms);

       // Rota para carregar ROM
       http_register_route(server, HTTP_POST, "/api/v1/roms/:id/load", handle_load_rom);

       // Rota para listar estados salvos
       http_register_route(server, HTTP_GET, "/api/v1/states", handle_list_states);

       // ... outras rotas
   }

   // Handler de exemplo
   int handle_list_roms(http_request_t* req, http_response_t* res) {
       // Diretório de ROMs
       const char* rom_dir = config_get_rom_directory();

       // Listar arquivos
       file_list_t files;
       list_files(rom_dir, &files, ".nes,.md,.sms,.sfc");

       // Criar resposta JSON
       json_object* root = json_object_new_object();
       json_object* roms = json_object_new_array();

       for (size_t i = 0; i < files.count; i++) {
           json_object* rom = json_object_new_object();
           json_object_object_add(rom, "id", json_object_new_string(files.items[i].filename));
           json_object_object_add(rom, "name", json_object_new_string(files.items[i].display_name));
           json_object_object_add(rom, "size", json_object_new_int64(files.items[i].size));
           json_object_object_add(rom, "path", json_object_new_string(files.items[i].path));

           json_object_array_add(roms, rom);
       }

       json_object_object_add(root, "roms", roms);

       // Enviar resposta
       http_response_set_status(res, 200);
       http_response_set_content_type(res, "application/json");
       http_response_set_body(res, json_object_to_json_string(root), -1);

       // Limpeza
       json_object_put(root);
       free_file_list(&files);

       return 0;
   }
   ```

2. **Cliente HTTP (TypeScript/React)**

   ```typescript
   // Cliente API para interação com o servidor REST
   export class EmulatorApiClient {
     private readonly baseUrl: string;

     constructor(baseUrl: string) {
       this.baseUrl = baseUrl;
     }

     // Obtém lista de ROMs
     async getRoms(): Promise<Rom[]> {
       const response = await fetch(`${this.baseUrl}/api/v1/roms`);
       if (!response.ok) {
         throw new Error(`Failed to fetch ROMs: ${response.statusText}`);
       }

       const data = await response.json();
       return data.roms;
     }

     // Carrega ROM no emulador
     async loadRom(romId: string): Promise<boolean> {
       const response = await fetch(`${this.baseUrl}/api/v1/roms/${romId}/load`, {
         method: 'POST',
       });

       if (!response.ok) {
         throw new Error(`Failed to load ROM: ${response.statusText}`);
       }

       const result = await response.json();
       return result.success;
     }

     // Obtém lista de estados salvos
     async getSaveStates(): Promise<SaveState[]> {
       const response = await fetch(`${this.baseUrl}/api/v1/states`);
       if (!response.ok) {
         throw new Error(`Failed to fetch save states: ${response.statusText}`);
       }

       const data = await response.json();
       return data.states;
     }

     // ... outros métodos para interação com a API
   }
   ```

#### 5.1.3 Protocolo de Mensagens

A comunicação seguirá um protocolo bem definido baseado em JSON:

| Tipo de Mensagem | Direção | Payload | Descrição |
|------------------|---------|---------|-----------|
| `FRAME_DATA` | Backend → Frontend | `{data: string, width: number, height: number, timestamp: number}` | Frame renderizado codificado em Base64 |
| `INPUT_EVENT` | Frontend → Backend | `{type: string, button: string, pressed: boolean}` | Evento de entrada (botão pressionado/solto) |
| `STATE_UPDATE` | Backend → Frontend | `{cpu: object, memory: object, registers: object}` | Atualização do estado do emulador |
| `COMMAND` | Frontend → Backend | `{action: string, params: object}` | Comando para o emulador (pause, resume, reset) |

### 5.2 Implementação de Paineis Draggable

**Status**: 📝 Planejado (Q3-Q4 2024)

A nova interface implementará um sistema de painéis draggable e resizable, permitindo aos usuários personalizar completamente o layout da interface de acordo com suas necessidades.

#### 5.2.1 Arquitetura do Sistema de Painéis

O sistema de painéis será construído utilizando uma combinação de bibliotecas React especializadas e componentes personalizados:

```typescript
// Definição de tipos para o sistema de painéis
interface PanelDefinition {
  id: string;
  title: string;
  type: PanelType;
  defaultSize: { width: number; height: number };
  defaultPosition: { x: number; y: number };
  minSize?: { width: number; height: number };
  maxSize?: { width: number; height: number };
  isResizable?: boolean;
  isDraggable?: boolean;
  isClosable?: boolean;
  zIndex?: number;
}

// Componente principal de painel
const DraggablePanel: React.FC<{
  panel: PanelDefinition;
  onClose?: () => void;
  onResize?: (size: { width: number; height: number }) => void;
  onDrag?: (position: { x: number; y: number }) => void;
  children: React.ReactNode;
}> = ({ panel, onClose, onResize, onDrag, children }) => {
  // Implementação utilizando react-draggable e react-resizable
  return (
    <Draggable
      handle=".panel-header"
      defaultPosition={panel.defaultPosition}
      disabled={!panel.isDraggable}
      onDrag={(e, data) => onDrag?.({ x: data.x, y: data.y })}
    >
      <Resizable
        width={panel.defaultSize.width}
        height={panel.defaultSize.height}
        minConstraints={panel.minSize ? [panel.minSize.width, panel.minSize.height] : undefined}
        maxConstraints={panel.maxSize ? [panel.maxSize.width, panel.maxSize.height] : undefined}
        onResizeStop={(e, data) => onResize?.({ width: data.size.width, height: data.size.height })}
        resizeHandles={panel.isResizable ? ['se', 'sw', 'ne', 'nw', 'e', 's', 'w', 'n'] : []}
      >
        <div className="panel" style={{ zIndex: panel.zIndex || 1 }}>
          <div className="panel-header">
            <span className="panel-title">{panel.title}</span>
            {panel.isClosable && (
              <button className="panel-close-btn" onClick={onClose}>
                ×
              </button>
            )}
          </div>
          <div className="panel-content">{children}</div>
        </div>
      </Resizable>
    </Draggable>
  );
};
```

#### 5.2.2 Sistema de Layout

O sistema permitirá diferentes modos de layout:

1. **Modo Livre (Floating)**
   - Painéis podem ser posicionados livremente
   - Suporte a sobreposição (z-index)
   - Persistência de posição/tamanho

2. **Modo Docking**
   - Painéis podem ser ancorados em regiões específicas
   - Sistema de guias para múltiplos painéis em uma região
   - Divisores ajustáveis entre regiões

3. **Layouts Pré-definidos**
   - Configurações rápidas para casos de uso comuns
   - Perfis de layout específicos por plataforma

#### 5.2.3 Implementação de Persistência

Os layouts serão salvos automaticamente usando `localStorage` e opcionalmente através do sistema de configuração da API REST:

```typescript
// Hook para gerenciamento de layout
function useLayoutManager() {
  // Estado do layout
  const [panels, setPanels] = useState<PanelState[]>(() => {
    // Carregar do localStorage
    const savedLayout = localStorage.getItem('emulator-layout');
    if (savedLayout) {
      try {
        return JSON.parse(savedLayout);
      } catch (e) {
        console.error('Failed to parse saved layout', e);
      }
    }
    // Layout padrão
    return defaultPanelState;
  });

  // Atualizar posição de um painel
  const updatePanelPosition = useCallback((id: string, position: { x: number; y: number }) => {
    setPanels(prevPanels => {
      const newPanels = prevPanels.map(panel =>
        panel.id === id ? { ...panel, position } : panel
      );

      // Salvar no localStorage
      localStorage.setItem('emulator-layout', JSON.stringify(newPanels));

      return newPanels;
    });
  }, []);

  // Atualizar tamanho de um painel
  const updatePanelSize = useCallback((id: string, size: { width: number; height: number }) => {
    setPanels(prevPanels => {
      const newPanels = prevPanels.map(panel =>
        panel.id === id ? { ...panel, size } : panel
      );

      // Salvar no localStorage
      localStorage.setItem('emulator-layout', JSON.stringify(newPanels));

      return newPanels;
    });
  }, []);

  // Alternar visibilidade de um painel
  const togglePanelVisibility = useCallback((id: string) => {
    setPanels(prevPanels => {
      const newPanels = prevPanels.map(panel =>
        panel.id === id ? { ...panel, visible: !panel.visible } : panel
      );

      // Salvar no localStorage
      localStorage.setItem('emulator-layout', JSON.stringify(newPanels));

      return newPanels;
    });
  }, []);

  // Salvar layout atual no servidor
  const saveLayoutToServer = useCallback(async (layoutName: string) => {
    try {
      const apiClient = new EmulatorApiClient(API_BASE_URL);
      await apiClient.saveLayout(layoutName, panels);
      return true;
    } catch (e) {
      console.error('Failed to save layout to server', e);
      return false;
    }
  }, [panels]);

  // Carregar layout do servidor
  const loadLayoutFromServer = useCallback(async (layoutName: string) => {
    try {
      const apiClient = new EmulatorApiClient(API_BASE_URL);
      const layout = await apiClient.getLayout(layoutName);
      setPanels(layout);
      localStorage.setItem('emulator-layout', JSON.stringify(layout));
      return true;
    } catch (e) {
      console.error('Failed to load layout from server', e);
      return false;
    }
  }, []);

  return {
    panels,
    updatePanelPosition,
    updatePanelSize,
    togglePanelVisibility,
    saveLayoutToServer,
    loadLayoutFromServer
  };
}
```

### 5.3 Redux para Estado Global

**Status**: 📝 Planejado (Q3-Q4 2024)

O gerenciamento de estado global da aplicação será implementado utilizando Redux Toolkit, que oferece uma API simplificada sobre o Redux tradicional, reduzindo boilerplate e facilitando a implementação.

#### 5.3.1 Estrutura do Estado Global

```typescript
// Estado global da aplicação
interface RootState {
  emulator: EmulatorState;
  ui: UIState;
  roms: RomState;
  saveStates: SaveStateState;
  settings: SettingsState;
  tools: ToolsState;
}

// Estado do emulador
interface EmulatorState {
  status: 'idle' | 'loading' | 'running' | 'paused' | 'error';
  currentRom: Rom | null;
  platform: Platform | null;
  fps: number;
  frameSkip: number;
  error: string | null;
  lastFrameTime: number;
  isFullscreen: boolean;
}

// Estado da interface
interface UIState {
  activeTheme: Theme;
  panels: PanelState[];
  activeTabId: string | null;
  modals: {
    [key: string]: boolean;
  };
  sidebarOpen: boolean;
  notifications: Notification[];
}

// Estado de ROMs
interface RomState {
  items: Rom[];
  isLoading: boolean;
  searchQuery: string;
  filters: RomFilters;
  recentlyPlayed: RecentRom[];
  favorites: string[];
}

// Estado de saves
interface SaveStateState {
  items: SaveState[];
  isLoading: boolean;
  currentSlot: number;
  autoSaveEnabled: boolean;
  rewindEnabled: boolean;
  rewindBufferSize: number;
}

// Configurações
interface SettingsState {
  video: VideoSettings;
  audio: AudioSettings;
  input: InputSettings;
  performance: PerformanceSettings;
  paths: PathSettings;
  advanced: AdvancedSettings;
}

// Estado das ferramentas
interface ToolsState {
  activeTool: string | null;
  memoryViewer: MemoryViewerState;
  spriteViewer: SpriteViewerState;
  debugger: DebuggerState;
  nodeIDE: NodeIDEState;
  // ... outras ferramentas
}
```

#### 5.3.2 Slices e Reducers

Cada seção do estado será gerenciada por um "slice" separado:

```typescript
// Exemplo de slice para o estado do emulador
import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { Rom, Platform } from '@/types';

const initialState: EmulatorState = {
  status: 'idle',
  currentRom: null,
  platform: null,
  fps: 0,
  frameSkip: 0,
  error: null,
  lastFrameTime: 0,
  isFullscreen: false
};

const emulatorSlice = createSlice({
  name: 'emulator',
  initialState,
  reducers: {
    // Inicia carregamento de ROM
    loadRomStart(state, action: PayloadAction<Rom>) {
      state.status = 'loading';
      state.error = null;
    },

    // ROM carregada com sucesso
    loadRomSuccess(state, action: PayloadAction<{ rom: Rom; platform: Platform }>) {
      state.status = 'running';
      state.currentRom = action.payload.rom;
      state.platform = action.payload.platform;
      state.error = null;
    },

    // Falha ao carregar ROM
    loadRomFailure(state, action: PayloadAction<string>) {
      state.status = 'error';
      state.error = action.payload;
    },

    // Atualiza FPS
    updateFps(state, action: PayloadAction<number>) {
      state.fps = action.payload;
      state.lastFrameTime = Date.now();
    },

    // Pausa/continua emulação
    togglePause(state) {
      if (state.status === 'running') {
        state.status = 'paused';
      } else if (state.status === 'paused') {
        state.status = 'running';
      }
    },

    // Reseta emulador
    resetEmulator(state) {
      if (state.status !== 'idle' && state.status !== 'loading') {
        state.status = 'running';
      }
    },

    // Alterna modo fullscreen
    toggleFullscreen(state) {
      state.isFullscreen = !state.isFullscreen;
    },

    // Configura frame skip
    setFrameSkip(state, action: PayloadAction<number>) {
      state.frameSkip = action.payload;
    }
  }
});

export const {
  loadRomStart,
  loadRomSuccess,
  loadRomFailure,
  updateFps,
  togglePause,
  resetEmulator,
  toggleFullscreen,
  setFrameSkip
} = emulatorSlice.actions;

export default emulatorSlice.reducer;
```

#### 5.3.3 Thunks para Operações Assíncronas

Para operações assíncronas como comunicação com o servidor, usaremos Redux Thunks:

```typescript
import { createAsyncThunk } from '@reduxjs/toolkit';
import { EmulatorApiClient } from '@/services/api';
import { loadRomStart, loadRomSuccess, loadRomFailure } from './emulatorSlice';
import { Rom, Platform } from '@/types';

// API client
const apiClient = new EmulatorApiClient(process.env.API_BASE_URL);

// Thunk para carregar ROM
export const loadRom = createAsyncThunk<
  void,
  string,
  { rejectValue: string }
>('emulator/loadRom', async (romId, { dispatch, rejectWithValue }) => {
  try {
    // Buscar detalhes da ROM
    const rom = await apiClient.getRomDetails(romId);

    // Iniciar carregamento
    dispatch(loadRomStart(rom));

    // Carregar ROM no emulador
    const result = await apiClient.loadRom(romId);

    if (!result.success) {
      return dispatch(loadRomFailure(result.error || 'Failed to load ROM'));
    }

    // Determinar plataforma
    const platform = detectPlatform(rom);

    // ROM carregada com sucesso
    dispatch(loadRomSuccess({ rom, platform }));
  } catch (error) {
    return dispatch(loadRomFailure(error.message || 'An unknown error occurred'));
  }
});

// Thunk para salvar estado
export const saveState = createAsyncThunk<
  void,
  { slot: number; description?: string },
  { rejectValue: string }
>('saveStates/saveState', async ({ slot, description }, { dispatch, getState, rejectWithValue }) => {
  try {
    // Indicar início do salvamento
    dispatch(saveStateStart(slot));

    // Fazer request para API
    const result = await apiClient.saveState(slot, description);

    if (!result.success) {
      return dispatch(saveStateFailure({ slot, error: result.error || 'Failed to save state' }));
    }

    // Estado salvo com sucesso
    dispatch(saveStateSuccess({ slot, state: result.state }));
  } catch (error) {
    return dispatch(saveStateFailure({ slot, error: error.message || 'An unknown error occurred' }));
  }
});
```

#### 5.3.4 Hooks para Acesso ao Estado

Para facilitar o acesso ao estado Redux, criaremos hooks customizados:

```typescript
import { useDispatch, useSelector } from 'react-redux';
import type { TypedUseSelectorHook } from 'react-redux';
import type { RootState, AppDispatch } from '@/store';

// Hooks tipados para Redux
export const useAppDispatch = () => useDispatch<AppDispatch>();
export const useAppSelector: TypedUseSelectorHook<RootState> = useSelector;

// Hook para estado do emulador
export function useEmulatorState() {
  const emulatorState = useAppSelector(state => state.emulator);
  const dispatch = useAppDispatch();

  return {
    ...emulatorState,
    loadRom: (romId: string) => dispatch(loadRom(romId)),
    togglePause: () => dispatch(togglePause()),
    resetEmulator: () => dispatch(resetEmulator()),
    toggleFullscreen: () => dispatch(toggleFullscreen()),
    setFrameSkip: (frameSkip: number) => dispatch(setFrameSkip(frameSkip))
  };
}

// Hook para estado de ROMs
export function useRomState() {
  const romState = useAppSelector(state => state.roms);
  const dispatch = useAppDispatch();

  return {
    ...romState,
    searchRoms: (query: string) => dispatch(searchRoms(query)),
    filterRoms: (filters: RomFilters) => dispatch(filterRoms(filters)),
    toggleFavorite: (romId: string) => dispatch(toggleFavorite(romId)),
    loadRomList: () => dispatch(fetchRoms())
  };
}

// ... outros hooks especializados
```
