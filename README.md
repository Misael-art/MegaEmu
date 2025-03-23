# Mega_Emu - Emulador Multi-plataforma

![Mega_Emu Logo](docs/images/logo.png)

## Sobre o Projeto

Mega_Emu é um emulador multi-plataforma de código aberto que suporta vários consoles retro, incluindo Mega Drive/Genesis, NES, Master System e Game Gear. O projeto foi desenvolvido com foco em precisão, desempenho e usabilidade, oferecendo ferramentas avançadas para jogar e desenvolver jogos retro.

**Versão Atual:** 1.2.5

## Plataformas Suportadas

- **Mega Drive/Genesis**: Emulação completa do Sega Mega Drive/Genesis
- **Master System**: Emulação do Sega Master System
- **Game Gear**: Emulação do Sega Game Gear
- **SG-1000**: Emulação do Sega SG-1000
- **NES (Nintendo Entertainment System)**: Emulação do console 8-bit da Nintendo com suporte a diversos mappers
- **SNES (Super Nintendo)**: Emulação do Super Nintendo Entertainment System (em desenvolvimento)
- **Game Boy / Game Boy Color**: Emulação dos portáteis clássicos da Nintendo (em desenvolvimento)
- **Game Boy Advance**: Emulação do Game Boy Advance (planejado)

## Características

- **Interface gráfica moderna e intuitiva** com React/TypeScript
- **Emulação precisa** com timing ciclo-a-ciclo dos processadores originais
- **Compatibilidade com dois modos**:
  - Modo Desktop: Acesso completo ao sistema de arquivos local
  - Modo Browser: Experiência web com mocks para funções do sistema de arquivos
- **Suporte a múltiplos controles** (teclado, gamepad)
- **Sistema avançado de save states** com thumbnails e metadados
- **Gerenciamento de ROMs** com suporte a upload e organização
- **Configurações avançadas** de vídeo e áudio
- **Ferramentas de desenvolvimento profissionais**:
  - Debugger avançado com breakpoints condicionais
  - Memory Viewer/Editor com suporte a edição em tempo real
  - Disassembler para análise de código
  - Visualizadores de PPU/VDP e áudio
  - Trace Logger para debugging detalhado

## Requisitos do Sistema

### Windows

- Windows 10 ou superior
- Processador de 2 GHz ou superior
- 2 GB de RAM
- Placa de vídeo com suporte a OpenGL 3.3 ou superior
- 100 MB de espaço em disco

### Linux

- Distribuição Linux moderna (Ubuntu 20.04+, Fedora 34+, etc.)
- Processador de 2 GHz ou superior
- 2 GB de RAM
- Placa de vídeo com suporte a OpenGL 3.3 ou superior
- 100 MB de espaço em disco

### macOS

- macOS 10.14 (Mojave) ou superior
- Processador Intel ou Apple Silicon
- 2 GB de RAM
- 100 MB de espaço em disco

### Browser

- Navegador moderno com suporte a WebGL e WebAssembly
- Chrome 80+, Firefox 75+, Safari 13.1+, Edge 80+

## Instalação

### Compilando do Código Fonte

#### Pré-requisitos

- CMake 3.15 ou superior
- Compilador C++ com suporte a C++17
- SDL2 (Simple DirectMedia Layer)
- Node.js 14+ e npm (para frontend React)
- Git

#### Windows

```powershell
# Clonar o repositório
git clone https://github.com/mega-emu/mega_emu.git
cd mega_emu

# Configurar e compilar backend
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Configurar e compilar frontend
cd ../frontend
npm install
npm run build

# Executar
cd ..
.\bin\Release\mega_emu.exe
```

#### Linux/macOS

```bash
# Clonar o repositório
git clone https://github.com/mega-emu/mega_emu.git
cd mega_emu

# Configurar e compilar backend
mkdir build
cd build
cmake ..
make

# Configurar e compilar frontend
cd ../frontend
npm install
npm run build

# Executar
cd ..
./bin/mega_emu
```

### Usando Binários Pré-compilados

Visite a [página de releases](https://github.com/mega-emu/mega_emu/releases) para baixar a versão mais recente para seu sistema operacional.

## Uso

1. Inicie o Mega_Emu
2. Selecione "Carregar ROM" no menu
3. Navegue até o arquivo da ROM que deseja executar
4. O emulador detectará automaticamente o tipo de ROM e iniciará a emulação

## Estrutura do Projeto

```
mega_emu/
├── build/                 # Diretório de compilação
├── cmake/                 # Módulos e configurações do CMake
├── deps/                  # Dependências externas
├── docs/                  # Documentação
│   ├── architecture/      # Documentação de arquitetura
│   ├── images/            # Imagens para documentação
│   └── guidelines/        # Guias para desenvolvedores
├── frontend/              # Frontend React/TypeScript
│   ├── public/            # Arquivos estáticos
│   └── src/               # Código fonte do frontend
│       ├── api/           # Comunicação com backend
│       ├── components/    # Componentes React
│       ├── hooks/         # Hooks personalizados
│       ├── pages/         # Componentes de página
│       ├── services/      # Serviços (incluindo emulação)
│       ├── state/         # Gerenciamento de estado (Redux)
│       ├── styles/        # Estilos CSS
│       ├── types/         # Definições TypeScript
│       └── utils/         # Utilitários
├── resources/             # Recursos (ícones, fontes, etc.)
├── scripts/               # Scripts de automação
├── src/                   # Código-fonte backend
│   ├── core/              # Núcleo do emulador
│   │   ├── cpu/           # Implementações de CPUs (Z80, 68000, etc.)
│   │   ├── memory/        # Sistema de memória
│   │   └── audio/         # Sistema de áudio
│   ├── platforms/         # Implementações específicas de plataforma
│   │   ├── megadrive/     # Emulação do Mega Drive
│   │   ├── mastersystem/  # Emulação do Master System
│   │   ├── nes/           # Emulação do NES
│   │   ├── snes/          # Emulação do SNES (em desenvolvimento)
│   │   └── gameboy/       # Emulação do Game Boy (em desenvolvimento)
│   ├── bridge/            # Ponte entre backend e frontend
│   │   ├── websocket/     # Servidor WebSocket
│   │   └── rest/          # API REST
│   └── utils/             # Utilitários
└── tests/                 # Testes automatizados
```

## Migração para React/TypeScript

A interface do Mega_Emu está em processo de migração de C/SDL2 para uma arquitetura moderna baseada em React e TypeScript. Os principais benefícios desta migração incluem:

1. **Interface Moderna e Responsiva**
   - Design adaptável a diferentes dispositivos
   - Interface personalizável com painéis arrastáveis
   - Temas específicos para cada console emulado
   - Experiência web completa

2. **Desenvolvimento Mais Rápido**
   - Componentização e reutilização de código
   - Ecossistema robusto de bibliotecas React
   - Tipagem estática com TypeScript
   - Ferramentas de desenvolvimento modernas

3. **Compatibilidade Multi-plataforma**
   - Capacidade de funcionar tanto como aplicativo desktop quanto como aplicação web
   - Adaptação automática às capacidades da plataforma
   - Mocks para APIs nativas quando necessário

A arquitetura do novo frontend utiliza uma API REST para comunicação com o backend, permitindo operações como gerenciamento de ROMs, configurações e save states. No modo desktop, o frontend acessa diretamente o sistema de arquivos, enquanto no modo browser, utiliza mocks para simular estas funções.

### Estado Atual da Migração

A migração está sendo implementada em fases:

- **Fase 1 (Concluída)**: Estrutura inicial do frontend React/TypeScript
  - Componentes básicos da interface
  - Gerenciamento de estado com Redux
  - Sistema de roteamento

- **Fase 2 (Em andamento)**: Implementação dos serviços principais
  - Serviço de ROMs com suporte a múltiplos diretórios
  - Sistema de save states
  - Configurações de emulador

- **Fase 3 (Planejada)**: Ferramentas avançadas de desenvolvimento
  - Debugger visual
  - Editor de memória
  - Visualizadores específicos para cada console

## Sistema de Debug

O Mega_Emu inclui um poderoso sistema de debug projetado para desenvolvedores de jogos retro:

- **Debugger de CPU (85%)**: Inspeção de registradores, stepping, breakpoints condicionais
- **Memory Viewer/Editor (80%)**: Visualização e edição da memória em tempo real
- **Disassembler (85%)**: Análise de código em linguagem de máquina
- **Visualização de PPU/VDP (60%)**: Inspeção de tiles, sprites e paletas
- **Visualização de Áudio (50%)**: Análise de canais e formas de onda
- **Trace Logger (90%)**: Registro detalhado de execução
- **Breakpoints Condicionais (85%)**: Paradas de execução baseadas em condições complexas

## Modos de Compatibilidade

O Mega_Emu oferece dois modos de operação:

### Modo Desktop

- Acesso completo ao sistema de arquivos
- Integração com o sistema operacional
- Desempenho máximo sem limitações
- Todas as ferramentas de desenvolvimento disponíveis

### Modo Browser

- Execução diretamente no navegador
- Utiliza mocks para funções de sistema de arquivos
- Armazenamento local para ROMs e save states
- Interface adaptada para experiência web
- Compartilhamento fácil via URLs

## Contribuindo

Contribuições são bem-vindas! Por favor, leia o [guia de contribuição](CONTRIBUTING.md) para mais detalhes sobre como contribuir para o projeto.

### Áreas de Contribuição Atuais

1. **Frontend React/TypeScript**: Implementação de componentes e integração com o backend
2. **Ferramentas de Desenvolvimento**: Melhoria nos sistemas de debug e análise
3. **Suporte a Consoles**: Expansão e refinamento da emulação de consoles
4. **Documentação**: Atualização e expansão da documentação técnica

### Fluxo de Trabalho para Contribuições

1. Faça um fork do repositório
2. Crie uma branch para sua feature (`git checkout -b feature/nova-feature`)
3. Faça commit das suas mudanças (`git commit -am 'Adiciona nova feature'`)
4. Faça push para a branch (`git push origin feature/nova-feature`)
5. Abra um Pull Request

## Roadmap

### Próximas Versões

- **v1.3.0 (Julho/2025)**:
  - Sistema avançado de Save States com thumbnails e metadados expandidos
  - Desenvolvimento dos componentes principais do frontend React/TypeScript
  - Integração com WebSocket para comunicação em tempo real
  - API REST para gerenciamento de ROMs e configurações
  - Sistema de painéis arrastáveis e customizáveis

- **v2.0.0 (Dezembro/2025)**:
  - Finalização do Frontend React/TypeScript
  - Ferramentas de desenvolvimento avançadas
  - IDE visual baseada em nós para desenvolvimento de jogos
  - Suporte a multiplayer online
  - Camada de comunicação otimizada entre emulador e frontend

## Licença

Este projeto está licenciado sob a licença MIT - veja o arquivo [LICENSE](LICENSE) para mais detalhes.

## Contato

- **Website**: [mega-emu.org](https://mega-emu.org)
- **Email**: <contato@mega-emu.org>
- **Discord**: [Servidor Mega_Emu](https://discord.gg/mega-emu)

## Agradecimentos

- [SDL2](https://www.libsdl.org/) - Biblioteca para acesso a hardware de baixo nível
- [React](https://reactjs.org/) - Biblioteca JavaScript para construção de interfaces
- [Comunidade de emulação](https://emulation.gametechwiki.com/) - Documentação e recursos
- Todos os contribuidores e testadores
