# Histórico de Versões do Mega_Emu

Versão atual: **1.2.7**

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

### v1.2.7 (27/04/2025) - ATUAL

**Mudanças Principais:**

- Implementação completa do Master System com suporte a jogos comerciais
- Desenvolvimento do Z80 como processador principal do Master System
- Implementação do Video Display Processor (VDP) específico para o Master System
- Implementação do Programmable Sound Generator (PSG) para síntese de áudio
- Sistema de mapeamento de memória otimizado para cartuchos do Master System
- Interface de entrada com suporte a controles padrão de 2 botões
- Suporte a múltiplos modos de vídeo e paletas de cores
- Suporte a interrupções em linhas específicas (HBLANK e VBLANK)
- Sistema de I/O para comunicação entre componentes

**Componentes Afetados:**

- Core/Z80: Nova implementação modular com adaptadores específicos de plataforma
- Platform/MasterSystem: Implementação completa de todos os componentes principais
- Core/Audio: Adição do sistema de áudio do Master System
- Core/Video: Implementação do VDP do Master System
- Platform/MegaDrive: Atualização do adaptador Z80 para compatibilidade
- Tests: Novos testes unitários para componentes do Master System
- Docs: Atualização da documentação com especificações do Master System
- Scripts/Build: Suporte ao Master System nos scripts de compilação

**Compatibilidade:**

- Suporte a ROMs comerciais populares como Alex Kidd in Miracle World e Sonic the Hedgehog
- Compatibilidade com jogos que usam várias regiões de memória e mapeamento de páginas
- Preservação de estado completo através do sistema de save states
- Manutenção da compatibilidade com as plataformas existentes (Mega Drive e NES)

### v1.2.6 (26/03/2025)

**Mudanças Principais:**

- Correção da integração do Electron com o frontend React
- Resolução de problemas de renderização e tela em branco no modo desktop
- Melhorias no sistema de layout e CSS para garantir exibição correta
- Correção de erros de tipagem TypeScript para melhor interoperabilidade
- Implementação de tela de carregamento para melhor experiência inicial
- Otimização do fluxo de inicialização do Electron
- Correção de problemas com declarações de tipos para a API Electron
- Atualização do sistema de arranque e build para ambientes de produção

**Componentes Afetados:**

- Frontend/App: Melhorias no layout e integração com Electron
- Frontend/Components: Correções de tipagem e props
- Frontend/Electron: Aprimoramento da integração com preload e IPC
- Frontend/CSS: Otimizações para garantir renderização correta
- Scripts/Build: Melhorias no sistema de build e inicialização

**Compatibilidade:**

- Mantém compatibilidade total com a versão anterior
- Melhora a experiência em modo desktop via Electron
- Resolve problemas de renderização em diferentes sistemas operacionais

### v1.2.5 (25/03/2025)

**Mudanças Principais:**

- Reestruturação completa da arquitetura do Z80
- Separação em biblioteca central e adaptadores específicos de plataforma
- Nova API para interação com o Z80
- Suporte melhorado para interrupções e I/O
- Documentação atualizada para a nova arquitetura
- Scripts de automação para verificação de conformidade das interfaces
- Implementação inicial da estrutura para frontend React/TypeScript
- Desenvolvimento de componentes básicos para a nova interface

**Componentes Afetados:**

- Core/Z80: Nova implementação modular
- Platform/MegaDrive: Adaptador Z80 específico
- Platform/MasterSystem: Adaptador Z80 específico
- Tests: Testes atualizados para nova API
- Docs: Nova documentação com guia de migração
- Frontend: Estrutura inicial para React/TypeScript

**Compatibilidade:**

- **Incompatível** com código que utilizava a API antiga do Z80
- Requer migração conforme guia em `docs/architecture/Z80.md`
- Formato de save state alterado para Z80
- O frontend antigo e novo coexistem durante a fase de transição

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

### v1.3.0 (01/07/2025) - PLANEJADA

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

- Desenvolvimento do Frontend React/TypeScript
  - Implementação dos componentes principais em React
  - Integração WebSocket para comunicação em tempo real
  - API REST para gerenciamento de ROMs e configurações
  - Sistema de painéis arrastáveis e customizáveis
  - Suporte a temas específicos de console

- Integração com Sistema de Mappers
  - Suporte a save states para todos os tipos de mappers
  - Salvamento/carregamento de estado para SRAM e EEPROM
  - Verificação de compatibilidade de ROMs via checksums
  - Suporte a todos os cartuchos especiais (SSF2, Pier Solar, etc.)
  - Suporte a mappers complexos do NES (MMC1, MMC3, MMC5, VRC6)

**Componentes Afetados:**

- Core/SaveStates: Nova implementação completa
- Frontend/React: Desenvolvimento de componentes e integração
- Bridge: Sistema de comunicação entre backend e frontend
- Platforms: Integração com sistema de save states em todas as plataformas
- Docs: Atualização da documentação para novos sistemas

**Compatibilidade:**

- Compatível com formatos anteriores de save states
- Maior precisão na restauração de estados
- API modernizada para desenvolvimento de plugins

### v2.0.0 (01/12/2025) - PLANEJADA

**Mudanças Principais:**

- Finalização do Frontend React + TypeScript
  - Nova arquitetura baseada em componentes modernos
  - Implementação de interface modular com painéis arrastáveis
  - Sistema de temas por console com personalização avançada
  - IDE visual baseada em nós para desenvolvimento de jogos
  - Ferramentas avançadas de edição e depuração
  - Migração completa de todas as ferramentas para o novo frontend

- Camada de Comunicação Emulador-Frontend
  - Implementação de servidor WebSocket para comunicação em tempo real
  - API REST para operações não-tempo-real
  - Serialização eficiente de dados binários
  - Sistema de bridge C++ para JavaScript
  - Protocolos otimizados para desempenho e baixa latência

- Ferramentas de Desenvolvimento Avançadas
  - Editor visual de sprites, tiles e paletas
  - Debugger avançado com suporte a breakpoints condicionais
  - Sistema de anotações e tags para regiões de memória
  - Visualização em tempo real de sprites, tiles e paletas
  - Depurador avançado de áudio com visualização de formas de onda
  - Ferramenta de profiling para identificação de bottlenecks

- Suporte a Multiplayer Online
  - Sistema de netplay com rollback para jogos locais
  - Compartilhamento de controle remoto
  - Co-desenvolvimento de jogos em tempo real
  - Salas de jogo com modos espectador

**Componentes Afetados:**

- Core: Sistema de ponte para comunicação com o frontend
- Frontend: Reescrita completa em React/TypeScript
- Documentação: Novo guia de desenvolvimento para frontend
- Ferramentas: Novas ferramentas de desenvolvimento visual

**Compatibilidade:**

- Manutenção da compatibilidade com ROMs existentes
- Manutenção da compatibilidade com save states anteriores
- Suporte aos mesmos atalhos de teclado da versão anterior
- Novas opções de acessibilidade e personalização

**Autores Principais:**

- Equipe Frontend: @team_frontend
- Equipe de Ponte: @team_bridge
- Equipe de Design: @team_design
- Equipe de Documentação: @team_docs

## Referências

- @[MEMORIA.md] para histórico detalhado de atividades
- @[ROADMAP.md] para planejamento futuro
- @[ESCOPO.md] para entendimento do projeto
