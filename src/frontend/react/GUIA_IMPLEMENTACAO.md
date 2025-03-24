# Guia de Implementação do Frontend Mega Emu

Este documento detalha como a implementação do frontend do Mega Emu foi estruturada e como integrá-lo com o backend quando ele estiver disponível.

## Versão Atual: 0.1.0-alpha

**Status do Projeto**: O frontend está em fase de desenvolvimento ativo. As principais interfaces e componentes foram implementados, mas a integração com o backend ainda não está completa.

## Visão Geral

O frontend do Mega Emu é uma interface web moderna desenvolvida com React, TypeScript e Material UI. Ele se comunica com o backend do emulador através de duas camadas:

1. **WebSocket**: Para comunicação em tempo real (frames de vídeo, áudio, entradas de controle)
2. **REST API**: Para operações não relacionadas a tempo real (configurações, gerenciamento de ROMs)

## Estrutura do Projeto

```
frontend/
├── src/
│   ├── components/              # Componentes reutilizáveis
│   │   ├── common/              # Componentes UI genéricos
│   │   ├── emulator/            # Componentes específicos do emulador
│   │   └── layout/              # Componentes de layout
│   │
│   ├── hooks/                   # Hooks personalizados
│   │   └── useEmulatorState.ts  # Hook principal para o estado do emulador
│   │
│   ├── pages/                   # Páginas principais
│   │   ├── Emulator/            # Página do emulador
│   │   ├── Roms/                # Gerenciador de ROMs
│   │   ├── Settings/            # Configurações
│   │   ├── Tools/               # Ferramentas de desenvolvimento
│   │   └── Debug/               # Informações e debug
│   │
│   ├── services/                # Serviços para comunicação
│   │   └── emulator/
│   │       ├── websocket.ts     # Cliente WebSocket
│   │       └── restApi.ts       # Cliente REST API
│   │
│   ├── state/                   # Gerenciamento de estado (Redux)
│   │   ├── slices/              # Slices do Redux
│   │   └── store.ts             # Configuração da store
│   │
│   ├── types/                   # Definições de tipos TypeScript
│   │   └── emulator.types.ts    # Tipos relacionados ao emulador
│   │
│   └── utils/                   # Funções utilitárias
└── public/                      # Arquivos estáticos
```

## Protocolo de Comunicação

### WebSocket

O frontend se conecta ao backend via WebSocket na porta `8080`. O formato das mensagens segue este padrão:

**De Frontend para Backend (Comandos)**:

```typescript
interface EmulatorCommand {
  type: string;  // Tipo do comando
  payload: any;  // Dados do comando
}
```

**De Backend para Frontend (Respostas)**:

```typescript
interface EmulatorResponse {
  type: string;       // Tipo da resposta
  success: boolean;   // Indica se a operação foi bem-sucedida
  payload?: any;      // Dados da resposta (se success=true)
  error?: string;     // Mensagem de erro (se success=false)
}
```

#### Tipos de Comandos

| Comando      | Payload                    | Descrição                           |
|--------------|----------------------------|-------------------------------------|
| START        | {}                         | Inicia a emulação                   |
| PAUSE        | {}                         | Pausa a emulação                    |
| RESUME       | {}                         | Retoma a emulação                   |
| STOP         | {}                         | Para a emulação                     |
| RESET        | {}                         | Reinicia a emulação                 |
| LOAD_ROM     | { romId: string }          | Carrega uma ROM específica          |
| INPUT_KEYDOWN| { key: string, timestamp: number } | Botão pressionado           |
| INPUT_KEYUP  | { key: string, timestamp: number } | Botão liberado              |

#### Tipos de Respostas

| Resposta     | Payload                    | Descrição                           |
|--------------|----------------------------|-------------------------------------|
| STATE_UPDATE | Partial<EmulatorState>     | Atualização de estado do emulador   |
| FRAME        | FrameData                  | Novo frame para renderizar          |
| ERROR        | { message: string }        | Erro ocorrido no backend            |

### REST API

A API REST está disponível na porta `8081`. Endpoints principais:

| Endpoint              | Método | Descrição                          |
|-----------------------|--------|----------------------------------- |
| /api/roms             | GET    | Lista todas as ROMs disponíveis    |
| /api/roms/:id         | GET    | Obtém detalhes de uma ROM          |
| /api/roms/upload      | POST   | Faz upload de uma nova ROM         |
| /api/savestates       | GET    | Lista estados salvos               |
| /api/savestates/:id   | GET    | Obtém um estado salvo específico   |
| /api/config           | GET    | Obtém configurações do emulador    |
| /api/config           | PATCH  | Atualiza configurações             |

## Integrações com o Backend

### 1. Protocolo WebSocket

O cliente WebSocket está implementado em `src/services/emulator/websocket.ts`. Quando o backend estiver disponível, basta garantir que ele implemente o servidor WebSocket na porta 8080 com o protocolo definido acima.

#### Conectando ao Servidor

```typescript
// O frontend já tenta se conectar automaticamente
import emulatorWebSocket from './services/emulator/websocket';

// Para conectar manualmente
await emulatorWebSocket.connect();

// Para enviar comandos
emulatorWebSocket.sendCommand({
  type: 'START',
  payload: {}
});

// Para ouvir eventos
emulatorWebSocket.onMessage('FRAME', (response) => {
  // Processar o frame recebido
  if (response.success && response.payload) {
    const frameData = response.payload as FrameData;
    // Renderizar o frame
  }
});
```

### 2. Integração REST API

O cliente REST API está implementado em `src/services/emulator/restApi.ts`. O backend deve implementar os endpoints REST conforme definido acima.

```typescript
import emulatorApiService from './services/emulator/restApi';

// Obter lista de ROMs
const roms = await emulatorApiService.getRomsList();

// Carregar configurações
const config = await emulatorApiService.getEmulatorConfig();

// Atualizar configurações
await emulatorApiService.updateEmulatorConfig({
  frameSkip: 2,
  audioEnabled: true
});
```

## Formato dos Dados

### FrameData

O formato esperado para os frames de vídeo:

```typescript
interface FrameData {
  imageData: Uint8Array;  // Buffer contendo os dados RGBA da imagem
  width: number;          // Largura do frame em pixels
  height: number;         // Altura do frame em pixels
  timestamp: number;      // Timestamp do frame (para sincronização)
}
```

### EmulatorState

O estado do emulador:

```typescript
interface EmulatorState {
  isRunning: boolean;           // Emulador em execução
  isPaused: boolean;            // Emulador pausado
  currentConsole: ConsoleType;  // Console atual (megadrive, nes, etc.)
  fps: number;                  // FPS atual
  loadedRom: string | null;     // ROM carregada (ID)
  frameSkip: number;            // Configuração de frameSkip
  audioEnabled: boolean;        // Áudio habilitado
  volume: number;               // Volume (0-100)
  error: string | null;         // Erro atual, se houver
  saveStates: SaveState[];      // Estados salvos
  rewindEnabled: boolean;       // Recurso de rewind ativado
  rewindBufferSize: number;     // Tamanho do buffer de rewind (em segundos)
  controllerConfig: ControllerConfig[]; // Configuração de controles
}
```

## Implementação do Servidor WebSocket

Para que o frontend funcione corretamente, o backend deve implementar um servidor WebSocket que siga este protocolo. Aqui está um pseudocódigo para a implementação do lado do servidor:

```c
// Pseudocódigo para o servidor WebSocket no backend

// Inicializar o servidor WebSocket na porta 8080
ws_server_init(8080);

// Registrar handler para novas conexões
ws_on_connection(handle_connection);

// Registrar handler para mensagens recebidas
ws_on_message(handle_message);

// Handler para novas conexões
void handle_connection(connection) {
  // Enviar o estado atual do emulador ao cliente
  EmulatorState current_state = get_current_emulator_state();

  ws_send(connection, {
    type: "STATE_UPDATE",
    success: true,
    payload: current_state
  });
}

// Handler para mensagens recebidas
void handle_message(connection, message) {
  // Parsear a mensagem JSON
  EmulatorCommand command = parse_json(message);

  // Processar o comando baseado no tipo
  switch (command.type) {
    case "START":
      start_emulation();
      break;
    case "PAUSE":
      pause_emulation();
      break;
    case "LOAD_ROM":
      string rom_id = command.payload.romId;
      load_rom(rom_id);
      break;
    case "INPUT_KEYDOWN":
      handle_key_down(command.payload.key);
      break;
    // Outros comandos...
  }

  // Enviar resposta de sucesso
  ws_send(connection, {
    type: command.type + "_RESPONSE",
    success: true
  });
}

// Função para enviar frames para os clientes (chamada do loop de emulação)
void send_frame(FrameData frame) {
  ws_broadcast({
    type: "FRAME",
    success: true,
    payload: frame
  });
}

// Função para enviar atualizações de estado
void update_emulator_state(EmulatorState new_state) {
  ws_broadcast({
    type: "STATE_UPDATE",
    success: true,
    payload: new_state
  });
}
```

## Implementação da API REST

O backend também deve implementar os endpoints REST conforme definido anteriormente. Aqui está um pseudocódigo para a implementação:

```c
// Pseudocódigo para a API REST no backend

// Inicializar o servidor HTTP na porta 8081
http_server_init(8081);

// Registrar rotas
http_register_route("GET", "/api/roms", handle_get_roms);
http_register_route("GET", "/api/roms/:id", handle_get_rom);
http_register_route("POST", "/api/roms/upload", handle_upload_rom);
http_register_route("GET", "/api/savestates", handle_get_savestates);
http_register_route("GET", "/api/config", handle_get_config);
http_register_route("PATCH", "/api/config", handle_update_config);
// Outras rotas...

// Handler para obter a lista de ROMs
void handle_get_roms(request, response) {
  RomInfo[] roms = get_available_roms();

  http_send_json_response(response, 200, roms);
}

// Handler para atualizar configurações
void handle_update_config(request, response) {
  // Parsear o corpo da requisição
  json body = parse_request_body(request);

  // Atualizar configurações
  bool success = update_emulator_config(body);

  if (success) {
    // Obter o estado atualizado
    EmulatorState updated_state = get_current_emulator_state();

    http_send_json_response(response, 200, updated_state);
  } else {
    http_send_json_response(response, 400, { error: "Failed to update config" });
  }
}

// Outros handlers...
```

## Próximos Passos

1. **Implementação do Backend**: Desenvolver o servidor WebSocket e a API REST no backend do emulador conforme especificado neste documento.

2. **Integração**: Conectar o frontend ao backend e testar a comunicação bidirecionalmente.

3. **Testes de Desempenho**: Verificar o desempenho da transmissão de frames e input lag.

4. **Refinamentos**: Ajustar a interface com base no feedback de uso real.

## Requisitos de Sistema

### Backend

- Servidor WebSocket na porta 8080
- Servidor HTTP para API REST na porta 8081
- Capacidade de enviar frames de vídeo em tempo real
- Processamento de entrada de teclado e gamepad

### Frontend

- Navegador moderno com suporte a WebGL e WebSockets
- Resolução mínima recomendada: 1280x720
- Suporte a JavaScript ES6+

## Contato e Suporte

Para dúvidas sobre a implementação do frontend ou integração com o backend, entre em contato com a equipe de desenvolvimento:

- **GitHub**: [Mega Emu Repository](https://github.com/mega-emu/mega-emu)
- **Issue Tracker**: Reporte bugs e problemas no GitHub
- **Documentação Completa**: Disponível em `/docs`
