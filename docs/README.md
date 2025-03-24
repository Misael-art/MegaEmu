# Documentação do Mega_Emu

Este diretório contém a documentação completa do emulador Mega_Emu, organizada por seções.

## Documentação Geral

- [ROADMAP.md](ROADMAP.md) - Planejamento e status atual do desenvolvimento
- [VERSION.md](VERSION.md) - Histórico de versões e mudanças
- [CODING_STANDARDS.md](CODING_STANDARDS.md) - Padrões de codificação adotados no projeto

## Diretrizes de IA

- [AI_GUIDELINE.md](AI_GUIDELINE.md) - Regras para uso de IA no desenvolvimento
- [AI_ESCOPO.md](AI_ESCOPO.md) - Escopo das tarefas delegadas à IA
- [AI_MEMORIA.md](AI_MEMORIA.md) - Documentação sobre como a IA interage com a memória do emulador

## Arquitetura

### Geral

- [architecture/ARCHITECTURE.md](architecture/ARCHITECTURE.md) - Visão geral da arquitetura do emulador

### CPUs

- [architecture/Z80.md](architecture/Z80.md) - Documentação da CPU Z80
- [architecture/M68000.md](architecture/M68000.md) - Documentação da CPU Motorola 68000

### Sistemas

#### NES (Nintendo Entertainment System)

- [architecture/nes/CPU.md](architecture/nes/CPU.md) - Documentação da CPU 6502/2A03 do NES
- [architecture/nes/PPU.md](architecture/nes/PPU.md) - Documentação da PPU (Picture Processing Unit) do NES
- [architecture/nes/APU.md](architecture/nes/APU.md) - Documentação da APU (Audio Processing Unit) do NES
- [architecture/nes/mappers/MAPPERS.md](architecture/nes/mappers/MAPPERS.md) - Visão geral dos mappers do NES

## Tutoriais

- [tutorials/README.md](tutorials/README.md) - Índice de tutoriais disponíveis

## Diretrizes

- [guidelines/README.md](guidelines/README.md) - Índice de diretrizes para desenvolvimento

## API

- [api/README.md](api/README.md) - Documentação da API pública do emulador

## Memória

- [memory/README.md](memory/README.md) - Documentação sobre a gestão de memória no emulador

## Contribuindo

Para contribuir com a documentação, siga as diretrizes em [CODING_STANDARDS.md](CODING_STANDARDS.md) e verifique o [ROADMAP.md](ROADMAP.md) para áreas que precisam de atenção.

Ao adicionar novos documentos, atualize este índice para que eles sejam facilmente localizáveis.

## Processo de Build

### Estrutura

A documentação do processo de build está organizada nas seguintes seções:

- **Guias de Build**: Documentação completa do processo de build
- **Configuração**: Setup do ambiente e dependências
- **Testes e Validação**: Execução e validação de testes
- **Release**: Processo de release e versionamento

### Arquivos Principais

- `BUILD_GUIDE.md`: Guia completo do processo de build
- `BUILD_STRUCTURE.md`: Organização dos diretórios
- `BUILD_SCRIPTS.md`: Documentação dos scripts
- `BUILD_TROUBLESHOOTING.md`: Solução de problemas

### Configuração

- `ENVIRONMENT_SETUP.md`: Setup do ambiente
- `VCPKG_SETUP.md`: Configuração do vcpkg
- `CMAKE_SETUP.md`: Configuração do CMake

### Testes e Release

- `RUNNING_TESTS.md`: Execução de testes
- `BUILD_VALIDATION.md`: Validação de builds
- `RELEASE_PROCESS.md`: Processo de release
