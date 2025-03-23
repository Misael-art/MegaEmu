# Estrutura do Frontend React/TypeScript

Este documento descreve a estrutura de diretórios e organização do novo frontend do Mega_Emu baseado em React e TypeScript.

## Visão Geral

O frontend do Mega_Emu foi redesenhado utilizando tecnologias modernas web para fornecer uma experiência mais flexível, customizável e extensível. A nova arquitetura é baseada em:

- **React**: Biblioteca para construção de interfaces de usuário
- **TypeScript**: Tipagem estática para reduzir erros e melhorar a manutenção
- **Redux Toolkit**: Gerenciamento de estado global
- **Tailwind CSS**: Framework de CSS utilitário para estilização
- **WebSockets**: Comunicação em tempo real com o emulador
- **REST API**: Operações de configuração e gerenciamento

## Estrutura de Diretórios

```
mega-emu-frontend/
│
├── public/                      # Arquivos estáticos
│   ├── favicon.ico
│   ├── index.html
│   └── assets/
│       ├── icons/               # Ícones da interface
│       ├── fonts/               # Fontes personalizadas
│       └── images/              # Imagens estáticas
│
├── src/
│   ├── api/                     # Comunicação com o backend
│   │   ├── client.ts            # Cliente HTTP/REST
│   │   ├── websocket.ts         # Cliente WebSocket
│   │   └── endpoints/           # Endpoints específicos
│   │       ├── roms.ts
│   │       ├── states.ts
│   │       └── config.ts
│   │
│   ├── components/              # Componentes React reutilizáveis
│   │   ├── common/              # Componentes genéricos UI
│   │   │   ├── Button/
│   │   │   ├── Dropdown/
│   │   │   ├── Modal/
│   │   │   ├── Panel/
│   │   │   └── Tabs/
│   │   │
│   │   ├── emulator/            # Componentes específicos do emulador
│   │   │   ├── GameDisplay/     # Tela de jogo
│   │   │   ├── ControlPad/      # Controles virtuais
│   │   │   ├── StateManager/    # Interface de save states
│   │   │   └── EmulatorStatus/  # Barra de status
│   │   │
│   │   ├── layout/              # Componentes de estrutura
│   │   │   ├── Header/
│   │   │   ├── Sidebar/
│   │   │   ├── Footer/
│   │   │   └── Workspace/
│   │   │
│   │   └── tools/               # Ferramentas de desenvolvimento
│   │       ├── MemoryViewer/
│   │       ├── SpriteViewer/
│   │       ├── Debugger/
│   │       ├── TileEditor/
│   │       └── NodeIDE/
│   │
│   ├── config/                  # Configurações da aplicação
│   │   ├── routes.ts            # Definição de rotas
│   │   ├── themes.ts            # Temas da interface
│   │   └── constants.ts         # Constantes globais
│   │
│   ├── hooks/                   # Hooks personalizados
│   │   ├── useEmulator.ts       # Interação com o emulador
│   │   ├── useWebSocket.ts      # Comunicação WebSocket
│   │   ├── useSettings.ts       # Configurações
│   │   └── useDraggable.ts      # Manipulação de elementos arrastáveis
│   │
│   ├── pages/                   # Componentes de página
│   │   ├── Home/
│   │   ├── Emulator/
│   │   ├── Settings/
│   │   ├── RomLibrary/
│   │   ├── DevTools/
│   │   └── About/
│   │
│   ├── services/                # Lógica de negócio
│   │   ├── emulator.ts          # Serviço de interação com emulador
│   │   ├── storage.ts           # Persistência local
│   │   ├── auth.ts              # Autenticação (se aplicável)
│   │   └── analytics.ts         # Telemetria (se aplicável)
│   │
│   ├── state/                   # Gerenciamento de estado (Redux)
│   │   ├── store.ts             # Configuração da store
│   │   ├── slices/              # Redux slices
│   │   │   ├── emulatorSlice.ts
│   │   │   ├── uiSlice.ts
│   │   │   ├── romsSlice.ts
│   │   │   └── settingsSlice.ts
│   │   │
│   │   └── middleware/          # Middleware personalizado
│   │       ├── websocket.ts
│   │       └── localStorage.ts
│   │
│   ├── styles/                  # Estilos globais
│   │   ├── global.css           # Estilos globais
│   │   ├── tailwind.css         # Configuração Tailwind
│   │   └── themes/              # Temas
│   │       ├── default.css
│   │       ├── dark.css
│   │       └── consoles/        # Temas específicos por console
│   │           ├── nes.css
│   │           ├── snes.css
│   │           ├── megadrive.css
│   │           └── mastersystem.css
│   │
│   ├── types/                   # Definições de tipos TypeScript
│   │   ├── emulator.ts          # Tipos relacionados ao emulador
│   │   ├── ui.ts                # Tipos de interface
│   │   ├── api.ts               # Tipos de API
│   │   └── index.ts             # Exportações de tipos
│   │
│   ├── utils/                   # Utilitários
│   │   ├── format.ts            # Formatadores
│   │   ├── validation.ts        # Validadores
│   │   ├── canvas.ts            # Utilitários de Canvas/WebGL
│   │   └── keyboard.ts          # Mapeamento de teclado
│   │
│   ├── App.tsx                  # Componente raiz
│   ├── index.tsx                # Ponto de entrada
│   └── react-app-env.d.ts       # Declarações para CRA
│
├── .eslintrc                    # Configuração ESLint
├── .prettierrc                  # Configuração Prettier
├── tailwind.config.js           # Configuração Tailwind CSS
├── tsconfig.json                # Configuração TypeScript
├── package.json                 # Dependências e scripts
└── README.md                    # Documentação do projeto
```

## Componentes Principais

### 1. GameDisplay

Responsável por renderizar os frames do emulador e gerenciar a interação do usuário.

```typescript
// Componente simplificado
const GameDisplay: React.FC = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const { frameData, isRunning } = useEmulatorState();
  const { sendInputEvent } = useEmulatorControls();

  // Renderizar frame recebido via WebSocket
  useEffect(() => {
    if (frameData && canvasRef.current) {
      const ctx = canvasRef.current.getContext('2d');
      if (ctx) {
        const imageData = new ImageData(
          new Uint8ClampedArray(frameData.data),
          frameData.width,
          frameData.height
        );
        ctx.putImageData(imageData, 0, 0);
      }
    }
  }, [frameData]);

  // Manipular eventos de teclado para controles
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      const button = mapKeyToButton(e.key);
      if (button) {
        sendInputEvent({ type: 'BUTTON_PRESS', button, pressed: true });
      }
    };

    const handleKeyUp = (e: KeyboardEvent) => {
      const button = mapKeyToButton(e.key);
      if (button) {
        sendInputEvent({ type: 'BUTTON_PRESS', button, pressed: false });
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    window.addEventListener('keyup', handleKeyUp);

    return () => {
      window.removeEventListener('keydown', handleKeyDown);
      window.removeEventListener('keyup', handleKeyUp);
    };
  }, [sendInputEvent]);

  return (
    <div className="game-display">
      <canvas
        ref={canvasRef}
        width={512}
        height={448}
        className={`${isRunning ? 'running' : 'paused'}`}
      />
    </div>
  );
};
```

### 2. ControlPanel

Painel de controle para ações básicas do emulador.

```typescript
const ControlPanel: React.FC = () => {
  const { status, togglePause, resetEmulator, loadRom } = useEmulatorState();
  const { saveStates, saveState, loadState } = useSaveStates();
  const { recentRoms } = useRomState();

  return (
    <div className="control-panel">
      <div className="control-buttons">
        <Button
          icon={status === 'running' ? 'pause' : 'play'}
          onClick={togglePause}
          disabled={status === 'idle' || status === 'loading'}
        >
          {status === 'running' ? 'Pause' : 'Play'}
        </Button>

        <Button
          icon="reset"
          onClick={resetEmulator}
          disabled={status === 'idle' || status === 'loading'}
        >
          Reset
        </Button>

        <SaveStateSelector
          states={saveStates}
          onSave={saveState}
          onLoad={loadState}
        />
      </div>

      <div className="recent-roms">
        <h3>Recent ROMs</h3>
        <ul>
          {recentRoms.map(rom => (
            <li key={rom.id}>
              <button onClick={() => loadRom(rom.id)}>
                {rom.name}
              </button>
            </li>
          ))}
        </ul>
      </div>
    </div>
  );
};
```

### 3. DraggablePanel

Implementação base dos painéis arrastáveis.

```typescript
interface DraggablePanelProps {
  id: string;
  title: string;
  defaultPosition: { x: number; y: number };
  defaultSize: { width: number; height: number };
  minSize?: { width: number; height: number };
  isResizable?: boolean;
  isDraggable?: boolean;
  isClosable?: boolean;
  onClose?: () => void;
  children: React.ReactNode;
}

const DraggablePanel: React.FC<DraggablePanelProps> = ({
  id,
  title,
  defaultPosition,
  defaultSize,
  minSize = { width: 200, height: 100 },
  isResizable = true,
  isDraggable = true,
  isClosable = true,
  onClose,
  children
}) => {
  const { updatePanelPosition, updatePanelSize } = useLayout();

  return (
    <Draggable
      handle=".panel-header"
      defaultPosition={defaultPosition}
      disabled={!isDraggable}
      onStop={(e, data) => updatePanelPosition(id, { x: data.x, y: data.y })}
    >
      <Resizable
        width={defaultSize.width}
        height={defaultSize.height}
        minConstraints={[minSize.width, minSize.height]}
        onResizeStop={(e, data) => updatePanelSize(id, { width: data.size.width, height: data.size.height })}
        handle={isResizable ? { bottomRight: true } : {}}
      >
        <div className="panel">
          <div className="panel-header">
            <div className="panel-title">{title}</div>
            {isClosable && (
              <button className="panel-close" onClick={onClose}>×</button>
            )}
          </div>
          <div className="panel-content">{children}</div>
        </div>
      </Resizable>
    </Draggable>
  );
};
```

### 4. MemoryViewer

Exemplo de ferramenta de desenvolvimento para visualização de memória.

```typescript
const MemoryViewer: React.FC = () => {
  const [address, setAddress] = useState(0);
  const [displaySize, setDisplaySize] = useState(256);
  const [displayMode, setDisplayMode] = useState<'hex' | 'dec' | 'ascii'>('hex');
  const [memoryData, setMemoryData] = useState<Uint8Array | null>(null);

  const { sendCommand } = useEmulatorWebSocket();

  // Solicitar dados de memória
  useEffect(() => {
    const fetchMemoryData = () => {
      sendCommand({
        type: 'GET_MEMORY',
        payload: {
          address,
          size: displaySize
        }
      });
    };

    fetchMemoryData();
    const intervalId = setInterval(fetchMemoryData, 500);

    return () => clearInterval(intervalId);
  }, [address, displaySize, sendCommand]);

  // Processar mensagens recebidas
  useEffect(() => {
    const handleMessage = (message: WebSocketMessage) => {
      if (message.type === 'MEMORY_DATA') {
        setMemoryData(new Uint8Array(message.payload.data));
      }
    };

    // Registrar handler
    addMessageHandler(handleMessage);

    return () => {
      // Remover handler
      removeMessageHandler(handleMessage);
    };
  }, []);

  // Renderizar visualização de memória
  const renderMemoryView = () => {
    if (!memoryData) return <div>Loading...</div>;

    const rows = [];

    for (let i = 0; i < memoryData.length; i += 16) {
      const rowAddress = address + i;
      const rowData = memoryData.slice(i, i + 16);

      rows.push(
        <div key={rowAddress} className="memory-row">
          <div className="address">{formatAddress(rowAddress)}</div>
          <div className="bytes">
            {Array.from(rowData).map((byte, index) => (
              <span key={index} className="byte">
                {formatByte(byte, displayMode)}
              </span>
            ))}
          </div>
          <div className="ascii">
            {Array.from(rowData).map((byte, index) => (
              <span key={index} className="ascii-char">
                {formatAscii(byte)}
              </span>
            ))}
          </div>
        </div>
      );
    }

    return <div className="memory-table">{rows}</div>;
  };

  return (
    <div className="memory-viewer">
      <div className="memory-toolbar">
        <input
          type="text"
          value={formatAddress(address)}
          onChange={(e) => setAddress(parseInt(e.target.value, 16))}
          className="address-input"
        />

        <select
          value={displayMode}
          onChange={(e) => setDisplayMode(e.target.value as any)}
        >
          <option value="hex">Hexadecimal</option>
          <option value="dec">Decimal</option>
          <option value="ascii">ASCII</option>
        </select>

        <button onClick={() => setAddress(address - displaySize)}>
          Previous
        </button>
        <button onClick={() => setAddress(address + displaySize)}>
          Next
        </button>
      </div>

      {renderMemoryView()}
    </div>
  );
};
```

## Comunicação com o Emulador

### WebSocket Hook

```typescript
const useEmulatorWebSocket = (url: string) => {
  const [status, setStatus] = useState<'connecting' | 'connected' | 'disconnected'>('disconnected');
  const [error, setError] = useState<string | null>(null);
  const socketRef = useRef<WebSocket | null>(null);
  const messageHandlersRef = useRef<Map<string, (data: any) => void>>(new Map());

  // Inicializar conexão WebSocket
  useEffect(() => {
    const socket = new WebSocket(url);

    socket.onopen = () => {
      console.log('WebSocket connected');
      setStatus('connected');
      setError(null);
    };

    socket.onclose = (event) => {
      console.log('WebSocket disconnected', event);
      setStatus('disconnected');

      // Tentar reconectar após timeout
      setTimeout(() => {
        setStatus('connecting');
      }, 3000);
    };

    socket.onerror = (error) => {
      console.error('WebSocket error', error);
      setError('WebSocket connection error');
    };

    socket.onmessage = (event) => {
      try {
        const message = JSON.parse(event.data);

        // Processar mensagem
        if (message && message.type) {
          const handler = messageHandlersRef.current.get(message.type);
          if (handler) {
            handler(message.payload);
          }

          // Dispatcher para qualquer handler registrado para 'all'
          const allHandler = messageHandlersRef.current.get('all');
          if (allHandler) {
            allHandler(message);
          }
        }
      } catch (error) {
        console.error('Error parsing WebSocket message', error);
      }
    };

    socketRef.current = socket;

    return () => {
      socket.close();
      socketRef.current = null;
    };
  }, [url]);

  // Registrar handler para tipo de mensagem
  const registerHandler = useCallback((type: string, handler: (data: any) => void) => {
    messageHandlersRef.current.set(type, handler);

    return () => {
      messageHandlersRef.current.delete(type);
    };
  }, []);

  // Enviar mensagem
  const sendMessage = useCallback((type: string, payload: any) => {
    if (socketRef.current && socketRef.current.readyState === WebSocket.OPEN) {
      socketRef.current.send(JSON.stringify({
        type,
        payload
      }));
      return true;
    }
    return false;
  }, []);

  return {
    status,
    error,
    registerHandler,
    sendMessage
  };
};
```

## Considerações de Design

### Temas por Console

Cada console terá seu próprio tema visual inspirado no hardware original:

- **NES**: Cinza claro, vermelho e preto
- **SNES**: Roxo, cinza e lilás
- **Mega Drive**: Preto, azul e vermelho
- **Master System**: Branco, azul e vermelho

### Layouts Adaptativos

A interface será projetada para funcionar bem em diferentes tamanhos de tela:

1. **Desktop**: Layout completo com todas as ferramentas
2. **Tablet**: Layout simplificado com ferramentas em abas
3. **Mobile**: Modo jogo simplificado com controles virtuais

### Modos de Interface

O frontend suportará diferentes modos de uso:

1. **Modo Jogar**: Interface simplificada focada no jogo
2. **Modo Desenvolver**: Interface completa com ferramentas de desenvolvimento
3. **Modo Personalizado**: Layout definido pelo usuário

## Roadmap de Implementação

1. **Fase 1: Fundação (Q1-Q2 2024)**
   - Estrutura básica do projeto React/TypeScript
   - Implementação da camada de comunicação WebSocket
   - Componentes fundamentais (GameDisplay, controles básicos)

2. **Fase 2: Core Components (Q3-Q4 2024)**
   - Sistema de painéis arrastáveis
   - Implementação do gerenciamento de estado (Redux)
   - ROM selector e gerenciamento de estados salvos

3. **Fase 3: Ferramentas de Desenvolvimento (Q1 2025)**
   - Ferramentas visuais (Memory Viewer, Sprite Viewer, etc.)
   - Debugger avançado
   - Node IDE para desenvolvimento visual
