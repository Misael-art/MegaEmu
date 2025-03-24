# Índice de Documentação - Mega_Emu

Este documento serve como ponto de entrada para toda a documentação do projeto Mega_Emu.

## Documentação para Usuários

- [Guia de Instalação](user/INSTALLATION.md)
- [Guia do Usuário](user/USER_GUIDE.md)
- [Perguntas Frequentes](user/FAQ.md)
- [Troubleshooting](user/TROUBLESHOOTING.md)

## Documentação Técnica

### Arquitetura

- [Visão Geral da Arquitetura](architecture/ARCHITECTURE.md) - Estrutura geral do projeto
- [Estrutura do Frontend](architecture/FRONTEND_STRUCTURE.md) - Documentação do frontend React/TypeScript
- [Estrutura do Backend](architecture/BACKEND_STRUCTURE.md) - Documentação do backend e emulador
- [Integração Frontend-Backend](architecture/INTEGRATION.md) - Como frontend e backend se comunicam

### Plataformas Suportadas

- [Mega Drive / Genesis](platforms/MEGA_DRIVE.md) - Implementação do Mega Drive
- [Master System](platforms/MASTER_SYSTEM.md) - Implementação do Master System
- [NES](platforms/NES.md) - Implementação do Nintendo Entertainment System
- [SNES](platforms/SNES.md) - Implementação do Super Nintendo (em desenvolvimento)

### Componentes Principais

- [CPU](components/CPU.md) - Implementações de CPUs (Z80, 68000, 6502)
- [Vídeo](components/VIDEO.md) - Sistemas de vídeo (VDP, PPU)
- [Áudio](components/AUDIO.md) - Sistemas de áudio (PSG, FM, APU)
- [Sistema de Memória](components/MEMORY.md) - Gerenciamento de memória
- [Sistema de Save States](components/SAVE_STATES.md) - Sistema de save states
- [Input System](components/INPUT_SYSTEM.md) - Sistema de entrada
- [Memory Management](components/MEMORY_MANAGEMENT.md) - Gerenciamento de memória
- [Video Rendering](components/VIDEO_RENDERING.md) - Renderização de vídeo
- [Audio System](components/AUDIO_SYSTEM.md) - Sistema de áudio

### Ferramentas de Desenvolvimento

- [Debugger](tools/DEBUGGER.md) - Ferramentas de debug
- [Memory Viewer](tools/MEMORY_VIEWER.md) - Visualizador/editor de memória
- [CPU State Viewer](tools/CPU_STATE_VIEWER.md) - Visualizador de estado da CPU
- [Tile Viewer](tools/TILE_VIEWER.md) - Visualizador de tiles
- [Disassembler](tools/DISASSEMBLER.md) - Disassembler para análise de código
- [Trace Logger](tools/TRACE_LOGGER.md) - Logger de execução

## Orientação para Desenvolvedores

- [Padrões de Codificação](CODING_STANDARDS.md) - Padrões de código do projeto
- [Diretrizes de Contribuição](CONTRIBUTING.md) - Como contribuir para o projeto
- [Processo de Build](BUILD_PROCESS.md) - Processo de construção do projeto
- [Testes e QA](TESTING.md) - Guia de testes

## Documentação de Planejamento

- [Roadmap](ROADMAP.md) - Planejamento futuro do projeto
- [Escopo do Projeto](AI_ESCOPO.md) - Escopo e objetivos do projeto
- [Diretrizes para IA](AI_GUIDELINE.md) - Diretrizes para o desenvolvimento assistido por IA
- [Memória de Atividades](AI_MEMORIA.md) - Histórico de desenvolvimento assistido por IA

## Meta-Documentação

- [Versão](VERSION.md) - Histórico de versões
- [Changelog](CHANGELOG.md) - Registro de mudanças
- [Contributors](CONTRIBUTORS.md) - Contribuidores do projeto

## Especificações Técnicas

- [Z80](architecture/Z80.md) - Detalhes técnicos do processador Z80
- [M68000](architecture/M68000.md) - Detalhes técnicos do processador 68000
- [Audio](architecture/AUDIO.md) - Detalhes técnicos do sistema de áudio
- [Especificações do VDP](specs/VDP.md) - Detalhes técnicos do Video Display Processor
- [Especificações do YM2612](specs/YM2612.md) - Detalhes técnicos do chip de som FM

## Recursos Externos

- [Especificação Técnica do Mega Drive](https://segaretro.org/Sega_Mega_Drive/Technical_specifications) - Especificações técnicas do Mega Drive
- [Documentação da API WebSocket](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API) - Documentação da API WebSocket
- [Especificação do 6502](http://www.obelisk.me.uk/6502/) - Especificação do processador 6502
- [Documentação de APIs](https://mega-emu.org/api) - Documentação de APIs públicas
- [Exemplos de Código](https://mega-emu.org/examples) - Exemplos de uso do emulador
- [Comunidade](https://mega-emu.org/community) - Recursos da comunidade

## Processo de Build

### Guias de Build

- [Guia de Build](build/BUILD_GUIDE.md) - Guia completo do processo de build
- [Estrutura de Build](build/BUILD_STRUCTURE.md) - Organização dos diretórios de build
- [Scripts de Build](build/BUILD_SCRIPTS.md) - Documentação dos scripts de build
- [Troubleshooting de Build](build/BUILD_TROUBLESHOOTING.md) - Solução de problemas comuns

### Configuração

- [Configuração de Ambiente](build/ENVIRONMENT_SETUP.md) - Setup do ambiente de desenvolvimento
- [Configuração do vcpkg](build/VCPKG_SETUP.md) - Configuração do gerenciador de pacotes
- [Configuração do CMake](build/CMAKE_SETUP.md) - Configuração do sistema de build

### Testes e Validação

- [Execução de Testes](build/RUNNING_TESTS.md) - Como executar e interpretar testes
- [Validação de Build](build/BUILD_VALIDATION.md) - Processo de validação de builds
- [Métricas de Build](build/BUILD_METRICS.md) - Métricas e análise de performance

### Release

- [Processo de Release](build/RELEASE_PROCESS.md) - Guia do processo de release
- [Versionamento](build/VERSIONING.md) - Sistema de versionamento
- [Changelog](build/CHANGELOG.md) - Registro de mudanças
