# Arquitetura do Frontend Mega_Emu

## Visão Geral

O frontend do Mega_Emu é construído com tecnologias web modernas, focando em modularidade, extensibilidade e performance. A arquitetura segue o padrão de design atômico e princípios SOLID.

## Stack Tecnológico

- **Core:**
  - React 19 (Framework UI)
  - TypeScript (Linguagem)
  - Redux Toolkit (Gerenciamento de Estado)
  - WebSocket (Comunicação em Tempo Real)
  - REST API (Operações Assíncronas)

- **UI/UX:**
  - Tailwind CSS (Estilização)
  - Material UI (Componentes Base)
  - React DnD (Drag and Drop)
  - React Query (Cache e Gerenciamento de Estado de Servidor)

- **Build/Dev:**
  - Vite (Build Tool)
  - ESLint/Prettier (Linting/Formatação)
  - Jest/Testing Library (Testes)
  - Storybook (Documentação de Componentes)

## Estrutura de Diretórios

```
src/
├── @types/                 # Definições de tipos globais
├── assets/                 # Recursos estáticos
├── components/             # Componentes React (Design Atômico)
│   ├── atoms/             # Componentes básicos
│   ├── molecules/         # Composições simples
│   ├── organisms/         # Composições complexas
│   ├── templates/         # Layouts base
│   └── pages/             # Páginas completas
├── config/                # Configurações
├── core/                  # Lógica core do emulador
│   ├── emulator/         # Interface com emulador
│   ├── audio/            # Sistema de áudio
│   └── video/            # Sistema de vídeo
├── features/             # Features modulares
│   ├── rom-library/      # Gerenciamento de ROMs
│   ├── save-states/      # Sistema de saves
│   ├── debugger/         # Ferramentas de debug
│   └── settings/         # Configurações
├── hooks/                # Hooks React customizados
├── lib/                  # Bibliotecas utilitárias
├── services/             # Serviços externos
├── store/                # Estado global (Redux)
└── utils/                # Utilitários gerais

```

## Arquitetura de Features

Cada feature é um módulo independente seguindo a estrutura:

```
feature-name/
├── api/                  # Endpoints da feature
├── components/           # Componentes específicos
├── hooks/               # Hooks da feature
├── store/               # Estado local (Redux slice)
├── types/               # Tipos TypeScript
├── utils/               # Utilitários
└── index.ts             # API pública
```

## Padrões de Design

### 1. Design Atômico

Os componentes seguem a hierarquia:

- **Atoms**: Botões, inputs, ícones
- **Molecules**: Cards, forms, menus
- **Organisms**: Headers, sidebars, painéis complexos
- **Templates**: Layouts base
- **Pages**: Páginas completas

### 2. Feature-First

- Cada feature é um módulo independente
- Encapsulamento de lógica e UI
- Comunicação via interfaces bem definidas

### 3. Estado Global

```typescript
// Exemplo de slice Redux
interface EmulatorState {
  status: 'idle' | 'loading' | 'running' | 'paused';
  currentRom: Rom | null;
  fps: number;
  settings: EmulatorSettings;
}

const emulatorSlice = createSlice({
  name: 'emulator',
  initialState,
  reducers: {
    setStatus: (state, action: PayloadAction<EmulatorStatus>) => {
      state.status = action.payload;
    },
    // ... outros reducers
  }
});
```

### 4. Comunicação com Backend

```typescript
// WebSocket Hook
const useEmulatorSocket = () => {
  const socket = useWebSocket(EMULATOR_WS_URL);

  const sendCommand = useCallback((command: EmulatorCommand) => {
    socket.send(JSON.stringify(command));
  }, [socket]);

  return {
    sendCommand,
    status: socket.status,
    lastFrame: socket.lastMessage
  };
};

// API Service
const emulatorApi = {
  loadRom: async (romId: string) => {
    return api.post('/emulator/load', { romId });
  },
  // ... outros métodos
};
```

## Módulos Core

### 1. Emulador Core

Interface principal com o emulador:

```typescript
class EmulatorCore {
  private socket: WebSocket;
  private frameBuffer: FrameBuffer;
  private audioContext: AudioContext;

  constructor() {
    this.initializeSubsystems();
  }

  private initializeSubsystems() {
    this.initVideo();
    this.initAudio();
    this.initInput();
  }

  // ... métodos de controle
}
```

### 2. Sistema de Vídeo

Gerenciamento de renderização:

```typescript
class VideoSystem {
  private canvas: HTMLCanvasElement;
  private gl: WebGL2RenderingContext;
  private shaders: ShaderPrograms;

  constructor(canvas: HTMLCanvasElement) {
    this.initializeWebGL();
    this.setupShaders();
  }

  public renderFrame(frameData: FrameData) {
    // Lógica de renderização
  }
}
```

### 3. Sistema de Áudio

Processamento de áudio:

```typescript
class AudioSystem {
  private context: AudioContext;
  private bufferSize: number;
  private processor: ScriptProcessorNode;

  constructor() {
    this.setupAudioContext();
    this.initializeProcessor();
  }

  public processAudioData(data: AudioData) {
    // Lógica de processamento
  }
}
```

## Componentes de Interface

### 1. Painel de Jogo

```typescript
const GamePanel: React.FC = () => {
  const { currentFrame } = useEmulatorState();
  const videoSystem = useVideoSystem();

  useEffect(() => {
    if (currentFrame) {
      videoSystem.renderFrame(currentFrame);
    }
  }, [currentFrame]);

  return (
    <div className="game-panel">
      <canvas ref={videoSystem.canvasRef} />
      <ControlBar />
    </div>
  );
};
```

### 2. Ferramentas de Desenvolvimento

```typescript
const DevTools: React.FC = () => {
  const { selectedTool } = useDevToolsState();

  return (
    <DraggablePanel>
      <ToolSelector />
      {selectedTool === 'memory' && <MemoryViewer />}
      {selectedTool === 'cpu' && <CpuDebugger />}
      {selectedTool === 'ppu' && <PpuViewer />}
    </DraggablePanel>
  );
};
```

## Testes

### 1. Testes de Componentes

```typescript
describe('GamePanel', () => {
  it('should render frame when received', () => {
    const { getByTestId } = render(<GamePanel />);
    const canvas = getByTestId('game-canvas');

    // Simular frame
    act(() => {
      emitFrame(mockFrameData);
    });

    expect(canvas).toHaveAttribute('data-frame-rendered', 'true');
  });
});
```

### 2. Testes de Integração

```typescript
describe('Emulator Integration', () => {
  it('should handle ROM loading flow', async () => {
    const { getByText, findByTestId } = render(<EmulatorPage />);

    // Simular carregamento de ROM
    fireEvent.click(getByText('Load ROM'));
    await findByTestId('emulator-running');

    expect(getByTestId('game-canvas')).toBeVisible();
  });
});
```

## Considerações de Performance

1. **Renderização Eficiente**
   - Uso de `React.memo` para componentes puros
   - Virtualização de listas longas
   - Code-splitting por rota/feature

2. **Otimização de Estado**
   - Normalização de dados no Redux
   - Uso de seletores memorizados
   - Cache de dados com React Query

3. **WebGL para Renderização**
   - Shaders otimizados
   - Texture atlases
   - Double buffering

## Segurança

1. **Validação de Entrada**
   - Sanitização de dados de ROM
   - Validação de comandos WebSocket
   - Rate limiting de ações

2. **Proteção de Dados**
   - Encriptação de saves
   - Validação de checksums
   - Sandboxing de código carregado

## Acessibilidade

1. **Suporte a Teclado**
   - Navegação completa por teclado
   - Atalhos personalizáveis
   - Focus management

2. **Compatibilidade com Leitores de Tela**
   - ARIA labels
   - Descrições significativas
   - Hierarquia semântica

## Internacionalização

- Sistema de traduções baseado em chaves
- Formatação de números/datas
- Suporte a RTL

## Documentação

1. **Documentação Técnica**
   - Arquitetura detalhada
   - Fluxos de dados
   - Padrões de código

2. **Storybook**
   - Catálogo de componentes
   - Documentação interativa
   - Testes visuais

## Conclusão

Esta arquitetura fornece uma base sólida para o frontend do Mega_Emu, permitindo:

- Desenvolvimento modular e escalável
- Performance otimizada
- Manutenibilidade a longo prazo
- Extensibilidade para novas features

A estrutura feature-first e o design atômico garantem que novos recursos possam ser adicionados sem impactar o código existente, enquanto os sistemas core fornecem uma base robusta para a emulação.
