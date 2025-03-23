# Protocolo WebSocket do Mega_Emu

Este documento descreve o protocolo WebSocket utilizado para comunicação entre o frontend React e o backend em C/C++ do Mega_Emu.

## Visão Geral

O protocolo WebSocket é utilizado para comunicação em tempo real entre o frontend e o backend do emulador, permitindo:

1. Streaming de frames do emulador para o frontend
2. Transmissão de comandos de entrada do frontend para o emulador
3. Sincronização de estado em tempo real
4. Depuração e monitoramento de componentes internos

## Conexão

### Endpoint

```
ws://[host]:[port]/ws
```

Exemplo para desenvolvimento local:

```
ws://localhost:8080/ws
```

### Handshake Inicial

Após estabelecer a conexão WebSocket, o cliente (frontend) deve enviar uma mensagem de handshake para iniciar a comunicação:

```json
{
  "type": "HANDSHAKE",
  "payload": {
    "clientId": "unique-client-id",
    "clientType": "web-frontend",
    "version": "2.0.0",
    "capabilities": ["audio", "input", "highres-video", "debug"]
  }
}
```

O servidor responderá com:

```json
{
  "type": "HANDSHAKE_ACK",
  "payload": {
    "serverId": "mega-emu-server",
    "version": "2.0.0",
    "status": "ok",
    "serverCapabilities": ["nes", "megadrive", "sms", "compression", "debug"],
    "sessionId": "session-uuid-12345"
  }
}
```

## Formato das Mensagens

Todas as mensagens seguem um formato JSON padronizado:

```json
{
  "type": "MESSAGE_TYPE",
  "payload": {
    // Dados específicos do tipo de mensagem
  },
  "id": "optional-message-id-for-request-response",
  "timestamp": 1647098762123
}
```

- `type`: Identifica o tipo da mensagem (string)
- `payload`: Contém os dados específicos da mensagem (objeto)
- `id` (opcional): Identificador único para correlacionar requisições e respostas
- `timestamp` (opcional): Timestamp em milissegundos

## Tipos de Mensagens

### 1. Controle do Emulador

#### 1.1. Gerenciamento do Ciclo de Vida

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `EMU_START` | Cliente → Servidor | Inicia a emulação |
| `EMU_PAUSE` | Cliente → Servidor | Pausa a emulação |
| `EMU_RESUME` | Cliente → Servidor | Retoma a emulação |
| `EMU_RESET` | Cliente → Servidor | Reinicia a emulação |
| `EMU_POWER_CYCLE` | Cliente → Servidor | Simula desligar/ligar o console |
| `EMU_STATUS` | Servidor → Cliente | Notifica mudanças de status |

Exemplo de `EMU_START`:

```json
{
  "type": "EMU_START",
  "payload": {
    "romId": "super-mario-bros-3",
    "options": {
      "region": "us",
      "frameSkip": 0,
      "audioEnabled": true
    }
  },
  "id": "start-request-1"
}
```

Exemplo de `EMU_STATUS`:

```json
{
  "type": "EMU_STATUS",
  "payload": {
    "status": "running",
    "fps": 60.1,
    "platform": "nes",
    "romLoaded": true,
    "romInfo": {
      "title": "Super Mario Bros. 3",
      "region": "us",
      "mapper": 4,
      "size": 393216
    }
  }
}
```

#### 1.2. Controle de ROM

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `ROM_LOAD` | Cliente → Servidor | Carrega uma ROM |
| `ROM_UNLOAD` | Cliente → Servidor | Descarrega a ROM atual |
| `ROM_INFO` | Servidor → Cliente | Informações sobre a ROM carregada |

Exemplo de `ROM_LOAD`:

```json
{
  "type": "ROM_LOAD",
  "payload": {
    "romId": "metroid",
    "format": "direct"
  },
  "id": "load-request-1"
}
```

Resposta:

```json
{
  "type": "ROM_INFO",
  "payload": {
    "success": true,
    "romId": "metroid",
    "title": "Metroid",
    "platform": "nes",
    "metadata": {
      "region": "us",
      "releaseYear": 1986,
      "publisher": "Nintendo",
      "mapper": 1,
      "size": 131072
    }
  },
  "id": "load-request-1"
}
```

### 2. Streaming de Vídeo e Áudio

#### 2.1. Vídeo

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `VIDEO_FRAME` | Servidor → Cliente | Frame de vídeo codificado |
| `VIDEO_CONFIG` | Cliente → Servidor | Configura parâmetros de vídeo |

Exemplo de `VIDEO_FRAME`:

```json
{
  "type": "VIDEO_FRAME",
  "payload": {
    "frameNumber": 1234,
    "timestamp": 1647098762123,
    "format": "base64",
    "width": 256,
    "height": 240,
    "data": "iVBORw0KGgoAAAANSUhEUgAAAQAA...",
    "frameIndex": 60
  }
}
```

Exemplo de `VIDEO_CONFIG`:

```json
{
  "type": "VIDEO_CONFIG",
  "payload": {
    "format": "base64",
    "quality": "high",
    "frameSkip": 0,
    "maxFps": 60,
    "scaleMode": "nearest"
  }
}
```

#### 2.2. Áudio

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `AUDIO_DATA` | Servidor → Cliente | Chunk de áudio codificado |
| `AUDIO_CONFIG` | Cliente → Servidor | Configura parâmetros de áudio |

Exemplo de `AUDIO_DATA`:

```json
{
  "type": "AUDIO_DATA",
  "payload": {
    "timestamp": 1647098762123,
    "format": "base64",
    "sampleRate": 44100,
    "channels": 2,
    "data": "UklGRiXuAgBXQVZFZm1..."
  }
}
```

### 3. Entrada do Usuário

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `INPUT_STATE` | Cliente → Servidor | Estado completo dos controles |
| `INPUT_EVENT` | Cliente → Servidor | Evento de entrada específico |
| `INPUT_CONFIG` | Cliente → Servidor | Configuração de mapeamento de controles |

Exemplo de `INPUT_EVENT`:

```json
{
  "type": "INPUT_EVENT",
  "payload": {
    "controller": 0,
    "button": "A",
    "pressed": true,
    "timestamp": 1647098762123
  }
}
```

Exemplo de `INPUT_STATE`:

```json
{
  "type": "INPUT_STATE",
  "payload": {
    "controller": 0,
    "buttons": {
      "A": true,
      "B": false,
      "START": false,
      "SELECT": false,
      "UP": false,
      "DOWN": false,
      "LEFT": true,
      "RIGHT": false
    },
    "timestamp": 1647098762123
  }
}
```

### 4. Estados Salvos

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `SAVE_STATE` | Cliente → Servidor | Solicita criar estado salvo |
| `LOAD_STATE` | Cliente → Servidor | Solicita carregar estado salvo |
| `LIST_STATES` | Cliente → Servidor | Solicita lista de estados salvos |
| `STATE_LIST` | Servidor → Cliente | Lista de estados salvos |

Exemplo de `SAVE_STATE`:

```json
{
  "type": "SAVE_STATE",
  "payload": {
    "slot": 1,
    "description": "Before boss fight",
    "thumbnail": true
  },
  "id": "save-request-1"
}
```

Resposta:

```json
{
  "type": "ACTION_RESULT",
  "payload": {
    "action": "SAVE_STATE",
    "success": true,
    "stateInfo": {
      "slot": 1,
      "timestamp": 1647098762123,
      "description": "Before boss fight",
      "thumbnail": "data:image/png;base64,iVBOR..."
    }
  },
  "id": "save-request-1"
}
```

### 5. Depuração e Ferramentas de Desenvolvimento

#### 5.1. Memória

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `READ_MEMORY` | Cliente → Servidor | Solicita leitura de região da memória |
| `WRITE_MEMORY` | Cliente → Servidor | Escreve em região da memória |
| `MEMORY_DATA` | Servidor → Cliente | Dados de memória solicitados |
| `MEMORY_WATCH` | Cliente → Servidor | Estabelece/remove watch de memória |
| `MEMORY_UPDATE` | Servidor → Cliente | Notificação de atualização de memória observada |

Exemplo de `READ_MEMORY`:

```json
{
  "type": "READ_MEMORY",
  "payload": {
    "address": "0x0000",
    "length": 256,
    "memoryDomain": "cpu"
  },
  "id": "mem-request-1"
}
```

Resposta:

```json
{
  "type": "MEMORY_DATA",
  "payload": {
    "address": "0x0000",
    "length": 256,
    "memoryDomain": "cpu",
    "format": "base64",
    "data": "AQIDBAUG..."
  },
  "id": "mem-request-1"
}
```

#### 5.2. CPU e Registradores

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `CPU_STATE` | Servidor → Cliente | Estado atual da CPU |
| `CPU_STEP` | Cliente → Servidor | Avança um ciclo/instrução |
| `CPU_RUN_UNTIL` | Cliente → Servidor | Executa até endereço especificado |
| `SET_REGISTER` | Cliente → Servidor | Define valor de registrador |

Exemplo de `CPU_STATE`:

```json
{
  "type": "CPU_STATE",
  "payload": {
    "cpu": "6502",
    "registers": {
      "A": "0x45",
      "X": "0x00",
      "Y": "0x10",
      "PC": "0xC000",
      "SP": "0xFD",
      "P": "0b00110000"
    },
    "flags": {
      "N": false,
      "V": false,
      "B": false,
      "D": false,
      "I": true,
      "Z": true,
      "C": false
    },
    "currentOpcode": "0xA9",
    "currentInstruction": "LDA #$45"
  }
}
```

#### 5.3. Sprites e Gráficos

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `GET_SPRITES` | Cliente → Servidor | Solicita dados de sprites |
| `SPRITE_DATA` | Servidor → Cliente | Dados de sprites solicitados |
| `GET_PALETTES` | Cliente → Servidor | Solicita paletas de cores |
| `PALETTE_DATA` | Servidor → Cliente | Dados de paletas solicitados |

Exemplo de `GET_SPRITES`:

```json
{
  "type": "GET_SPRITES",
  "payload": {
    "format": "individual",
    "includeAttributes": true
  },
  "id": "sprite-request-1"
}
```

Resposta:

```json
{
  "type": "SPRITE_DATA",
  "payload": {
    "count": 64,
    "format": "individual",
    "sprites": [
      {
        "id": 0,
        "x": 128,
        "y": 120,
        "tileIndex": 42,
        "attributes": {
          "palette": 1,
          "priority": false,
          "flipH": false,
          "flipV": false
        },
        "data": "iVBORw0KGgoAAAA..."
      },
      // ... outros sprites
    ]
  },
  "id": "sprite-request-1"
}
```

#### 5.4. Breakpoints e Watches

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `SET_BREAKPOINT` | Cliente → Servidor | Define um breakpoint |
| `CLEAR_BREAKPOINT` | Cliente → Servidor | Remove um breakpoint |
| `LIST_BREAKPOINTS` | Cliente → Servidor | Solicita lista de breakpoints |
| `BREAKPOINT_LIST` | Servidor → Cliente | Lista de breakpoints ativos |
| `BREAKPOINT_HIT` | Servidor → Cliente | Notifica hit em breakpoint |

Exemplo de `SET_BREAKPOINT`:

```json
{
  "type": "SET_BREAKPOINT",
  "payload": {
    "type": "execute",
    "address": "0xC123",
    "condition": "A == 0x00",
    "temporary": false,
    "enabled": true
  },
  "id": "bp-request-1"
}
```

Resposta:

```json
{
  "type": "ACTION_RESULT",
  "payload": {
    "action": "SET_BREAKPOINT",
    "success": true,
    "breakpointId": "bp-12345"
  },
  "id": "bp-request-1"
}
```

### 6. Mensagens de Sistema

| Tipo | Direção | Descrição |
|------|---------|-----------|
| `PING` | Bidirecional | Verifica latência/conexão |
| `PONG` | Bidirecional | Resposta a PING |
| `ERROR` | Servidor → Cliente | Notifica erro |
| `LOG` | Servidor → Cliente | Mensagem de log do emulador |
| `ACTION_RESULT` | Servidor → Cliente | Resultado de uma ação solicitada |

Exemplo de `PING`:

```json
{
  "type": "PING",
  "payload": {
    "timestamp": 1647098762123
  },
  "id": "ping-1"
}
```

Resposta:

```json
{
  "type": "PONG",
  "payload": {
    "timestamp": 1647098762123,
    "serverTimestamp": 1647098762145
  },
  "id": "ping-1"
}
```

Exemplo de `ERROR`:

```json
{
  "type": "ERROR",
  "payload": {
    "code": "ROM_LOAD_FAILED",
    "message": "Failed to load ROM: file not found",
    "details": {
      "romId": "invalid-rom",
      "reason": "FILE_NOT_FOUND"
    }
  },
  "id": "load-request-1"
}
```

## Compressão e Otimização

Para reduzir a latência e o consumo de largura de banda, o protocolo suporta compressão dos dados:

1. **Formato Binário**: Para dados grandes (frames, áudio, memória), o servidor pode enviar dados em formato binário em vez de Base64

2. **Compressão**: O protocolo suporta os seguintes métodos de compressão:
   - `gzip`: Para compressão geral
   - `rle`: Para compressão de imagens simples
   - `delta`: Para compressão de frames sucessivos

3. **Otimizações de Frames**:
   - Transmissão apenas de regiões modificadas
   - Frames delta (diferenças entre frames consecutivos)
   - Redução dinâmica de qualidade em caso de largura de banda limitada

Para ativar a compressão, o cliente pode incluir as preferências no handshake inicial:

```json
{
  "type": "HANDSHAKE",
  "payload": {
    "clientId": "web-frontend-1",
    "clientType": "web-frontend",
    "compression": ["gzip", "delta"],
    "binarySupport": true
  }
}
```

## Considerações de Segurança

1. **Autenticação**: Para ambientes multi-usuário, é recomendado implementar autenticação via tokens JWT ou similar

2. **Validação de Entrada**: O servidor deve validar rigorosamente todas as entradas para evitar exploits

3. **Limitação de Taxa**: Implementar rate limiting para evitar sobrecarga do servidor

4. **Isolamento**: Em ambientes compartilhados, garantir isolamento entre sessões de usuários diferentes

## Exemplo de Fluxo de Comunicação

Abaixo está um exemplo de fluxo de comunicação típico:

1. **Estabelecimento de Conexão**:
   - Cliente conecta ao endpoint WebSocket
   - Cliente envia `HANDSHAKE`
   - Servidor responde com `HANDSHAKE_ACK`

2. **Carregamento da ROM**:
   - Cliente envia `ROM_LOAD`
   - Servidor responde com `ROM_INFO`

3. **Início da Emulação**:
   - Cliente envia `EMU_START`
   - Servidor começa a enviar frames:
     - Servidor envia `VIDEO_FRAME` (60fps)
     - Servidor envia `AUDIO_DATA` (em chunks)
     - Servidor envia `CPU_STATE` (se modo debug ativado)

4. **Interação do Usuário**:
   - Cliente envia `INPUT_EVENT` quando usuário pressiona botões
   - Cliente envia `INPUT_STATE` periodicamente

5. **Salvamento de Estado**:
   - Cliente envia `SAVE_STATE`
   - Servidor responde com `ACTION_RESULT`

6. **Depuração**:
   - Cliente envia `READ_MEMORY`
   - Servidor responde com `MEMORY_DATA`
   - Cliente envia `SET_BREAKPOINT`
   - Servidor responde com `ACTION_RESULT`
   - Quando breakpoint é atingido, servidor envia `BREAKPOINT_HIT`

7. **Finalização**:
   - Cliente envia `EMU_PAUSE` ou `ROM_UNLOAD`
   - Cliente fecha conexão WebSocket

## Implementação no Cliente (Frontend)

Exemplo de implementação do cliente WebSocket em TypeScript:

```typescript
export class EmulatorWebSocket {
  private socket: WebSocket | null = null;
  private messageHandlers: Map<string, (payload: any) => void> = new Map();
  private connected: boolean = false;
  private reconnectTimer: any = null;
  private messageQueue: any[] = [];

  constructor(private url: string) {}

  public connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.socket = new WebSocket(this.url);

      this.socket.onopen = () => {
        console.log('WebSocket connected');
        this.connected = true;

        // Send handshake
        this.sendMessage('HANDSHAKE', {
          clientId: this.generateClientId(),
          clientType: 'web-frontend',
          version: '2.0.0',
          capabilities: ['audio', 'input', 'highres-video']
        });

        // Process queued messages
        while (this.messageQueue.length > 0) {
          const msg = this.messageQueue.shift();
          this.socket?.send(JSON.stringify(msg));
        }

        resolve();
      };

      this.socket.onclose = () => {
        console.log('WebSocket disconnected');
        this.connected = false;
        this.scheduleReconnect();
      };

      this.socket.onerror = (error) => {
        console.error('WebSocket error:', error);
        reject(error);
      };

      this.socket.onmessage = (event) => {
        try {
          const message = JSON.parse(event.data);
          this.handleMessage(message);
        } catch (error) {
          console.error('Error parsing message:', error);
        }
      };
    });
  }

  public disconnect(): void {
    if (this.socket) {
      this.socket.close();
      this.socket = null;
    }

    this.connected = false;

    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
  }

  public sendMessage(type: string, payload: any, id?: string): void {
    const message = {
      type,
      payload,
      id: id || this.generateMessageId(),
      timestamp: Date.now()
    };

    if (this.connected && this.socket) {
      this.socket.send(JSON.stringify(message));
    } else {
      // Queue message for when connection is established
      this.messageQueue.push(message);

      if (!this.socket) {
        this.connect();
      }
    }
  }

  public registerHandler(messageType: string, handler: (payload: any) => void): void {
    this.messageHandlers.set(messageType, handler);
  }

  public unregisterHandler(messageType: string): void {
    this.messageHandlers.delete(messageType);
  }

  private handleMessage(message: any): void {
    const { type, payload, id } = message;

    // Handle system messages
    if (type === 'PING') {
      this.sendMessage('PONG', { timestamp: payload.timestamp }, id);
      return;
    }

    // Find handler for message type
    const handler = this.messageHandlers.get(type);
    if (handler) {
      handler(payload);
    } else {
      console.log(`No handler registered for message type: ${type}`);
    }
  }

  private scheduleReconnect(): void {
    if (!this.reconnectTimer) {
      this.reconnectTimer = setTimeout(() => {
        console.log('Attempting to reconnect...');
        this.connect();
        this.reconnectTimer = null;
      }, 3000);
    }
  }

  private generateClientId(): string {
    return 'web-client-' + Math.random().toString(36).substring(2, 15);
  }

  private generateMessageId(): string {
    return 'msg-' + Date.now() + '-' + Math.random().toString(36).substring(2, 9);
  }
}
```

## Implementação no Servidor (Backend)

Exemplo simplificado de implementação em C++ (usando libwebsockets):

```cpp
// Estrutura da mensagem
struct WebSocketMessage {
    std::string type;
    json payload;
    std::string id;
    int64_t timestamp;
};

// Handler de mensagens
void handle_message(struct lws *wsi, const WebSocketMessage& msg) {
    // Processar mensagem com base no tipo
    if (msg.type == "HANDSHAKE") {
        handle_handshake(wsi, msg);
    } else if (msg.type == "ROM_LOAD") {
        handle_rom_load(wsi, msg);
    } else if (msg.type == "EMU_START") {
        handle_emu_start(wsi, msg);
    } else if (msg.type == "INPUT_EVENT") {
        handle_input_event(wsi, msg);
    }
    // ... outros tipos de mensagem
}

// Envio de mensagem para cliente
void send_message(struct lws *wsi, const std::string& type, const json& payload, const std::string& id = "") {
    WebSocketMessage msg;
    msg.type = type;
    msg.payload = payload;
    msg.id = id.empty() ? generate_message_id() : id;
    msg.timestamp = get_current_timestamp_ms();

    json j = {
        {"type", msg.type},
        {"payload", msg.payload},
        {"id", msg.id},
        {"timestamp", msg.timestamp}
    };

    std::string message = j.dump();

    // Prepara buffer para envio
    unsigned char *buf = (unsigned char*)malloc(LWS_PRE + message.length());
    memcpy(buf + LWS_PRE, message.c_str(), message.length());

    // Envia mensagem
    lws_write(wsi, buf + LWS_PRE, message.length(), LWS_WRITE_TEXT);

    free(buf);
}

// Exemplo de handler para carregamento de ROM
void handle_rom_load(struct lws *wsi, const WebSocketMessage& msg) {
    std::string rom_id = msg.payload["romId"];

    // Carregar ROM
    bool success = emulator_load_rom(rom_id);

    if (success) {
        // Obter informações da ROM
        RomInfo info = emulator_get_rom_info();

        // Enviar resposta
        json payload = {
            {"success", true},
            {"romId", rom_id},
            {"title", info.title},
            {"platform", info.platform},
            {"metadata", {
                {"region", info.region},
                {"releaseYear", info.release_year},
                {"publisher", info.publisher},
                {"mapper", info.mapper},
                {"size", info.size}
            }}
        };

        send_message(wsi, "ROM_INFO", payload, msg.id);
    } else {
        // Enviar erro
        json payload = {
            {"success", false},
            {"error", "Failed to load ROM"},
            {"details", {
                {"romId", rom_id},
                {"reason", "FILE_NOT_FOUND"}
            }}
        };

        send_message(wsi, "ERROR", payload, msg.id);
    }
}
```

## Considerações de Desempenho

Para garantir o melhor desempenho possível:

1. **Priorização de Mensagens**:
   - Frames de vídeo e áudio têm prioridade máxima
   - Atualizações de estado têm prioridade média
   - Dados de depuração têm prioridade baixa

2. **Ajuste Dinâmico**:
   - Qualidade de frame e taxa de amostragem ajustáveis com base na largura de banda
   - Suporte a frame skipping em condições de rede ruins

3. **Bufferização**:
   - Cliente deve implementar buffer adequado para áudio/vídeo para evitar interrupções
   - Servidor deve limitar throughput para evitar overflow no cliente

4. **Throttling**:
   - Limitar frequência de atualizações de ferramentas de depuração
   - Implementar throttling de eventos de entrada para evitar sobrecarga

## Extensões Futuras

O protocolo foi projetado para permitir extensões futuras:

1. **Suporte a Multiplayer**:
   - Sincronização de estados entre múltiplos clientes
   - Compartilhamento de controles e observação

2. **Streaming Avançado**:
   - Suporte a codecs de vídeo (H.264, VP9) para maior eficiência
   - Renderização WebGL para processamento local de filtros

3. **Análise e Telemetria**:
   - Coleta de métricas de desempenho
   - Análise de jogabilidade e padrões

4. **Treinamento de IA**:
   - Interface para algoritmos de aprendizado de máquina
   - Automação e scripting avançados

---

**Última atualização**: Março 2024
**Versão do Protocolo**: 1.0
