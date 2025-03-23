# Índice de Documentação - Mega_Emu

Este documento serve como ponto de entrada para toda a documentação do projeto Mega_Emu.

## Documentação do Usuário

- [README.md](../README.md) - Visão geral e informações básicas do projeto
- [Instalação e Execução](guides/INSTALLATION.md) - Guia detalhado de instalação e execução
- [Guia do Usuário](guides/USER_GUIDE.md) - Como utilizar o emulador
- [Perguntas Frequentes](guides/FAQ.md) - Perguntas e respostas comuns

## Documentação Técnica

### Arquitetura

- [Visão Geral da Arquitetura](architecture/OVERVIEW.md) - Estrutura geral do projeto
- [Estrutura do Frontend](architecture/FRONTEND_STRUCTURE.md) - Documentação do frontend React/TypeScript
- [Estrutura do Backend](architecture/BACKEND_STRUCTURE.md) - Documentação do backend e emulador
- [Integração Frontend-Backend](architecture/INTEGRATION.md) - Como frontend e backend se comunicam

### Plataformas Suportadas

- [Mega Drive / Genesis](platforms/MEGA_DRIVE.md) - Implementação do Mega Drive
- [Master System / Game Gear](platforms/MASTER_SYSTEM.md) - Implementação do Master System e Game Gear
- [NES](platforms/NES.md) - Implementação do Nintendo Entertainment System
- [SNES](platforms/SNES.md) - Implementação do Super Nintendo (em desenvolvimento)

### Componentes Principais

- [CPU](components/CPU.md) - Implementações de CPUs (Z80, 68000, 6502)
- [Vídeo](components/VIDEO.md) - Sistemas de vídeo (VDP, PPU)
- [Áudio](components/AUDIO.md) - Sistemas de áudio (PSG, FM, APU)
- [Sistema de Memória](components/MEMORY.md) - Gerenciamento de memória
- [Save States](components/SAVE_STATES.md) - Sistema de save states

### Ferramentas de Desenvolvimento

- [Debugger](tools/DEBUGGER.md) - Ferramentas de debug
- [Memory Viewer](tools/MEMORY_VIEWER.md) - Visualizador/editor de memória
- [Disassembler](tools/DISASSEMBLER.md) - Disassembler para análise de código
- [Trace Logger](tools/TRACE_LOGGER.md) - Logger de execução

## Orientação para Desenvolvedores

- [Guia de Contribuição](../CONTRIBUTING.md) - Como contribuir para o projeto
- [Guia de Estilo](guidelines/STYLE_GUIDE.md) - Padrões de código do projeto
- [Fluxo de Trabalho](guidelines/WORKFLOW.md) - Workflow de desenvolvimento
- [Testes](guidelines/TESTING.md) - Guia de testes

## Documentação de Planejamento

- [ROADMAP](../docs/ROADMAP.md) - Planejamento futuro do projeto
- [VERSION](../docs/VERSION.md) - Histórico de versões

## Meta-documentação

- [ESCOPO](../docs/ESCOPO.md) - Escopo e objetivos do projeto
- [AI_MEMORIA](../docs/AI_MEMORIA.md) - Histórico de desenvolvimento assistido por IA

## Especificações Técnicas

- [Especificações do Z80](specs/Z80.md) - Detalhes técnicos do processador Z80
- [Especificações do 68000](specs/68000.md) - Detalhes técnicos do processador 68000
- [Especificações do VDP](specs/VDP.md) - Detalhes técnicos do Video Display Processor
- [Especificações do YM2612](specs/YM2612.md) - Detalhes técnicos do chip de som FM

## Recursos Externos

- [Documentação de APIs](https://mega-emu.org/api) - Documentação de APIs públicas
- [Exemplos de Código](https://mega-emu.org/examples) - Exemplos de uso do emulador
- [Comunidade](https://mega-emu.org/community) - Recursos da comunidade
