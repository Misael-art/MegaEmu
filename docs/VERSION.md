# Histórico de Versões do Mega_Emu

Versão atual: **1.2.5**

## Formato de Versão

MAJOR.MINOR.PATCH

- MAJOR: Mudanças incompatíveis ou grande reestruturação
- MINOR: Novas funcionalidades compatíveis
- PATCH: Correções de bugs e pequenas melhorias

## Processo de Versionamento

1. Atualize este arquivo ao lançar uma nova versão
2. Registre todas as mudanças significativas na seção da versão
3. Mantenha uma lista de arquivos modificados e componentes afetados
4. Documente quaisquer mudanças incompatíveis com versões anteriores
5. Siga o formato abaixo para cada entrada

## Histórico de Versões

### v1.2.5 (25/03/2025) - ATUAL

**Mudanças Principais:**

- Reestruturação completa da arquitetura do Z80
- Separação em biblioteca central e adaptadores específicos de plataforma
- Nova API para interação com o Z80
- Suporte melhorado para interrupções e I/O
- Documentação atualizada para a nova arquitetura
- Scripts de automação para verificação de conformidade das interfaces

**Componentes Afetados:**

- Core/Z80: Nova implementação modular
- Platform/MegaDrive: Adaptador Z80 específico
- Platform/MasterSystem: Adaptador Z80 específico
- Tests: Testes atualizados para nova API
- Docs: Nova documentação com guia de migração

**Compatibilidade:**

- **Incompatível** com código que utilizava a API antiga do Z80
- Requer migração conforme guia em `docs/architecture/Z80.md`
- Formato de save state alterado para Z80

### v1.2.4 (22/03/2025)

**Mudanças Principais:**

- Correção da arquitetura dual de processadores do Mega Drive (M68K + Z80)
- Reorganização da estrutura de diretórios para comportar múltiplos processadores
- Implementação do Z80 como co-processador para Mega Drive
- Implementação inicial do Master System com Z80 como processador principal
- Introdução de uma estrutura de comunicação entre processadores para o Mega Drive
- Melhoria na documentação da arquitetura do sistema
- Reestruturação do sistema de compilação (CMake) para novos componentes

**Componentes Afetados:**

- Core/CPU: Separação do M68K e Z80 em diretórios diferentes
- Core/MegaDrive: Novo controlador de CPUs duais
- Core/MasterSystem: Implementação inicial
- Docs: Nova documentação de arquitetura

**Compatibilidade:**

- Mega Drive: Corrigida a implementação para refletir a arquitetura dual correta
- Master System: Implementação inicial com funcionalidades básicas

### v1.2.3 (10/03/2025)

**Mudanças Principais:**

- Refatoração completa da arquitetura do frontend para melhor modularidade
- Implementação de um sistema de gerenciamento de GUI completo
- Criação de uma camada de abstração para elementos de interface gráfica
- Integração do adaptador SDL com o sistema de GUI
- Implementação de widgets básicos (botões)
- Melhoria na documentação do frontend
- Correção de inconsistências nos cabeçalhos e implementações

**Componentes Afetados:**

- Frontend/Common: Atualização da interface principal
- Frontend/GUI/Core: Implementação do sistema de gerenciamento de GUI
- Frontend/GUI/Widgets: Adição de widgets básicos
- Frontend/Platform/SDL: Adaptador para integração com SDL
- Docs: Atualização da documentação no AI_MEMORIA

### v1.2.2 (01/03/2025)

**Mudanças Principais:**

- Migração dos testes unitários do NES (APU, CPU, PPU) do framework GoogleTest para Unity
- Padronização da estrutura de testes unitários em todo o projeto
- Renomeação dos arquivos de teste de `.cpp` para `.c` para manter consistência
- Implementação de funções de setup e teardown padronizadas para testes
- Atualização do script de teste unitário para incluir os testes do NES com Unity
- Melhoria na documentação dos testes unitários no ROADMAP
- Registro detalhado das adaptações de teste no AI_MEMORIA

**Componentes Afetados:**

- Tests/Platforms/NES/APU: Migração para Unity
- Tests/Platforms/NES/CPU: Migração para Unity
- Tests/Platforms/NES/PPU: Migração para Unity
- Scripts/Build/Tests: Atualização do script de teste unitário
- Docs: Atualização do ROADMAP e AI_MEMORIA

### v1.2.1 (15/02/2025)

**Mudanças Principais:**

- Implementação do Mapper 3 (CNROM) para o NES
- Implementação do sistema de sprites para o VDP do Mega Drive
- Implementação do sistema de DMA para o VDP do Mega Drive
- Implementação do chip de áudio YM2612 (FM) para o Mega Drive
- Implementação do chip de áudio SN76489 (PSG) para o Mega Drive
- Implementação de testes unitários para sistemas base (configuração, memória, eventos)
- Implementação de testes unitários para mappers do NES (0, 1, 2, 3)
- Atualização do script de teste unitário para incluir todos os novos testes
- Implementação do sistema de áudio integrado para o Mega Drive
- Desenvolvimento de testes unitários automatizados para o sistema de áudio

**Componentes Afetados:**

- Platforms/NES/Mappers: Adição dos Mappers 2 e 3
- Platforms/MegaDrive/VDP: Sistema de sprites e DMA
- Platforms/MegaDrive/Audio: Implementação dos chips YM2612 e SN76489
- Platforms/MegaDrive/Audio: Sistema de áudio integrado e mixagem
- Tests: Novos testes unitários para validação do sistema de áudio

### v1.2.0 (01/02/2025)

**Mudanças Principais:**

- Primeira implementação da plataforma NES
- Suporte a Mappers 0 (NROM) e 1 (MMC1)
- Implementação do PPU do NES com suporte completo a paletas e sprites
- Implementação inicial do APU do NES com 2 canais de pulso
- Sistema de input para o NES com suporte a 2 controles
- Interface de depuração para memória e PPU
- Scripts para testes automatizados de ROMs
- Atualização da documentação para incluir especificações do NES

**Componentes Afetados:**

- Core: Adição de suporte a 6502
- Platforms/NES: Nova implementação completa
- Frontend: Adição de UI específica para NES
- Tools: Utilitários para debugging do NES
- Tests: Testes unitários para componentes do NES

**Compatibilidade:**

- Suporte a ROMs comerciais populares como Super Mario Bros. e Donkey Kong
- Compatibilidade limitada com outros títulos devido à implementação parcial dos mappers

### v1.1.0 (15/12/2024)

**Mudanças Principais:**

- Implementação completa do VDP (Video Display Processor) do Mega Drive
- Suporte a todos os modos de vídeo incluindo modo 5 (320x224)
- Implementação do controle preciso de scroll e sprites
- Suporte para interrupções HBLANK e VBLANK
- Implementação de vários efeitos especiais (parallax, window, etc.)
- Renderização de linha por linha com timing preciso
- Aprimoramentos significativos na precisão da emulação
- Adição de opções de escala e filtros para o frontend

**Componentes Afetados:**

- Platforms/MegaDrive/VDP: Reescrita completa
- Platforms/MegaDrive/CPU: Melhorias na integração com VDP
- Platforms/MegaDrive/Memory: Atualizações para mapeamento de registradores VDP
- Frontend/Render: Novos modos de escala e filtros
- Tests: Testes extensivos para funcionalidades do VDP

**Compatibilidade:**

- Melhoria significativa na compatibilidade com jogos comerciais
- Correção de glitches gráficos em diversos títulos populares
- Suporte a efeitos especiais usados em jogos como Sonic, Gunstar Heroes

### v1.0.0 (01/10/2024)

**Mudanças Principais:**

- Primeira versão estável com suporte completo ao Mega Drive
- Implementação completa do processador M68000
- Emulação básica do sistema de vídeo
- Suporte a controles padrão
- Sistema de carregamento de ROMs
- Interface de usuário básica

**Componentes Afetados:**

- Core: Primeira versão estável
- Platforms/MegaDrive: Implementação inicial completa
- Frontend: Interface de usuário básica

**Compatibilidade:**

- Capaz de executar jogos comerciais simples
- Compatibilidade limitada com títulos que usam recursos avançados

## Próxima Versão Planejada

### v1.3.0 (01/04/2025) - ATUAL

**Mudanças Principais:**

- Sistema avançado de Save States
  - Implementação completa de save states para o Mega Drive
  - Implementação robusta de save states para o NES
  - Thumbnails WebP com tarja "Save" integrada na imagem
  - Metadados expandidos (região do jogo, versão do emulador, notas de usuário, tags)
  - Compressão delta avançada para dados de memória
  - Compressão Zstandard para estado da GPU
  - Sistema de rewind otimizado com buffer circular (200 snapshots)
  - Efeito visual de rewind (escala de cinza, velocidade 0.5x)
  - Verificação de compatibilidade via checksums de ROM
  - Sistema completo de exportação e importação de saves

- Integração com Sistema de Mappers
  - Suporte a save states para todos os tipos de mappers
  - Salvamento/carregamento de estado para SRAM e EEPROM
  - Verificação de compatibilidade de ROMs via checksums
  - Suporte a todos os cartuchos especiais (SSF2, Pier Solar, etc.)
  - Suporte a mappers complexos do NES (MMC1, MMC3, MMC5, VRC6)

**Componentes Afetados:**

- Core/SaveState: Novo sistema avançado
- Core/DeltaCompression: Nova implementação para compressão eficiente
- Core/ThumbnailGenerator: Gerador de thumbnails WebP com tarja
- Core/RewindBuffer: Sistema otimizado de buffer circular para rewind
- Platforms/MegaDrive/State: Implementação específica para Mega Drive
- Platforms/NES/Save: Implementação específica para NES
- Platforms/MegaDrive/Memory: Integração com sistema de mappers
- Platforms/NES/Cartridge: Integração com mappers do NES

**Compatibilidade:**

- Suporta todos os jogos do Mega Drive, incluindo aqueles com chips especiais
- Suporta todos os jogos do NES com diversos tipos de mappers
- Compatível com o formato de SRAM/EEPROM existente
- Compatível com versões anteriores do emulador

## Versão 1.2.5 (2025-03-22)

### Descrição

Esta versão introduz uma reestruturação significativa da implementação do Z80, com uma arquitetura mais modular que facilita o reuso entre diferentes plataformas (Mega Drive e Master System).

### Principais Mudanças

- Criação de uma biblioteca base do Z80 em `src/core/cpu/z80/`
- Implementação de adaptadores específicos para Mega Drive e Master System
- Separação clara de responsabilidades entre a implementação base e os adaptadores específicos
- Sistema de callbacks para acesso à memória e I/O
- Implementação inicial de instruções básicas do Z80
- Integração com os sistemas existentes de ambas as plataformas

### Correções

- Resolvido o problema de duplicação de código entre as implementações do Mega Drive e Master System
- Melhorada a manutenibilidade do código do Z80

### Componentes Afetados

- core/cpu/z80
- platforms/megadrive/cpu
- platforms/mastersystem/cpu
- platforms/megadrive/megadrive.c
- platforms/mastersystem/mastersystem.c
- docs/Z80.md
- docs/AI_MEMORIA.md
- docs/VERSION.md

## Referências

- @[MEMORIA.md] para histórico detalhado de atividades
- @[ROADMAP.md] para planejamento futuro
- @[ESCOPO.md] para entendimento do projeto
