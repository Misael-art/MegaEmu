# Padrões de Codificação do Mega Emu

## Visão Geral

Este documento define os padrões de codificação para o projeto Mega Emu. Todos os contribuidores devem seguir estas diretrizes para manter a consistência do código.

## Sumário

1. [Estilo Geral](#estilo-geral)
2. [Nomenclatura](#nomenclatura)
3. [Indentação e Formatação](#indentação-e-formatação)
4. [Comentários](#comentários)
5. [Diretivas de Pré-processador](#diretivas-de-pré-processador)
6. [Tipos de Dados](#tipos-de-dados)
7. [Funções](#funções)
8. [Estruturas e Enumerações](#estruturas-e-enumerações)
9. [Tratamento de Erros](#tratamento-de-erros)
10. [Práticas Específicas por Plataforma](#práticas-específicas-por-plataforma)

## Estilo Geral

- O código deve ser escrito em C padrão (C99) para garantir compatibilidade.
- Evite códigos específicos de compilador, a menos que seja absolutamente necessário.
- Utilize as macros de plataforma definidas em `global_defines.h` quando precisar de código específico para uma plataforma.
- Limite as linhas a 100 caracteres quando possível.
- Use espaços em vez de tabulações.

## Nomenclatura

### Arquivos

- Nomes de arquivos devem ser em minúsculas com palavras separadas por underscore (`_`).
- Arquivos de cabeçalho usam a extensão `.h`.
- Arquivos de implementação usam a extensão `.c`.
- Nomeie os arquivos de forma descritiva: `megadrive_vdp.c`, `nes_ppu.c`.

### Variáveis

- Use snake_case para variáveis (todas as letras minúsculas com palavras separadas por underscore).
- Nomes de variáveis devem ser descritivos:

  ```c
  int counter;             // Bom
  int i;                   // Aceitável em loops curtos
  int x;                   // Ruim, a menos que seja uma coordenada
  ```

- Variáveis globais devem ter um prefixo indicando o módulo:

  ```c
  int md_current_scanline;  // Variável global para o Mega Drive
  ```

### Constantes e Macros

- Constantes e macros devem ser em MAIÚSCULAS com palavras separadas por underscore:

  ```c
  #define MAX_ROM_SIZE 8388608
  #define VDP_CONTROL_PORT 0xC00004
  ```

### Funções

- Funções devem usar snake_case.
- Funções públicas devem incluir o nome da plataforma ou módulo como prefixo:

  ```c
  void megadrive_init();
  int z80_execute_instruction();
  ```

- Funções estáticas (privadas) devem começar com underscore:

  ```c
  static void _update_vdp_registers();
  ```

### Tipos

- Tipos e Typedefs: `snake_case_t` (ex: `emulator_context_t`)
- Estruturas: `CamelCase` (ex: `EmulatorContext`)
- Tipos definidos com `typedef` devem terminar com `_t`:

  ```c
  typedef struct {
      uint16_t registers[8];
  } vdp_t;
  ```

## Indentação e Formatação

- Use 4 espaços para indentação.
- Chaves devem seguir o estilo K&R (chave de abertura na mesma linha da declaração, chave de fechamento em sua própria linha):

  ```c
  if (condition) {
      do_something();
  } else {
      do_alternative();
  }
  ```

- Coloque um espaço após palavras-chave (`if`, `for`, `while`, etc.).
- Coloque um espaço antes e depois de operadores binários (`+`, `-`, `*`, `/`, etc.).
- Não coloque espaços após um operador unário (`!`, `~`, `++`, `--`, etc.).
- Coloque um espaço entre uma função e seus parênteses:

  ```c
  void function_name (int parameter);
  ```

- Separe funções com duas linhas em branco.
- Separe seções lógicas do código com uma linha em branco.

## Comentários

- Use comentários de estilo Doxygen para funções públicas:

  ```c
  /**
   * @brief Inicializa o emulador do Mega Drive
   * @param rom_data Ponteiro para os dados da ROM
   * @param rom_size Tamanho da ROM em bytes
   * @return 0 em caso de sucesso, código de erro em caso de falha
   */
  int megadrive_init(const uint8_t *rom_data, size_t rom_size);
  ```

- Use comentários simples para código interno:

  ```c
  // Atualiza registradores de controle
  control_reg = (control_reg & 0xFFF0) | value;
  ```

- Agrupe comentários relacionados a uma seção específica:

  ```c
  /****************************
   * Controladores de Entrada
   ****************************/
  ```

- Comente decisões não óbvias e comportamentos complexos, não o que o código está fazendo.

## Diretivas de Pré-processador

- Diretivas de inclusão devem ser agrupadas e ordenadas:
  1. Cabeçalhos da biblioteca padrão
  2. Cabeçalhos do projeto
  3. Cabeçalhos locais

  ```c
  #include <stdio.h>
  #include <stdlib.h>

  #include "core/core.h"

  #include "megadrive.h"
  #include "megadrive_vdp.h"
  ```

- Use guardas de inclusão em todos os arquivos de cabeçalho:

  ```c
  #ifndef MEGADRIVE_H
  #define MEGADRIVE_H

  // Conteúdo do cabeçalho

  #endif /* MEGADRIVE_H */
  ```

- Use `#if` em vez de `#ifdef` quando verificar definições de macros:

  ```c
  #if defined(PLATFORM_WINDOWS)
      // Código específico do Windows
  #elif defined(PLATFORM_LINUX)
      // Código específico do Linux
  #endif
  ```

## Tipos de Dados

- Use os tipos definidos em `stdint.h` para garantir tamanhos consistentes:

  ```c
  uint8_t byte_value;
  int16_t signed_word;
  uint32_t long_value;
  ```

- Alternativamente, use os tipos abreviados definidos em `global_defines.h`:

  ```c
  u8 byte_value;
  s16 signed_word;
  u32 long_value;
  ```

- Use `size_t` para tamanhos e contagens.
- Use `bool` de `stdbool.h` para valores booleanos.

## Funções

- Funções devem ser pequenas e ter uma única responsabilidade.
- Limite o número de parâmetros a no máximo 5, quando possível.
- Retorne 0 ou um valor positivo para sucesso e valores negativos para erros.
- Documente os códigos de retorno em comentários da função.
- Valide parâmetros de entrada no início da função.

## Estruturas e Enumerações

- Use estruturas para agrupar dados relacionados.
- Alinhe os membros de estrutura para facilitar a leitura:

  ```c
  typedef struct {
      uint16_t  control;       // Registrador de controle
      uint8_t   status;        // Registrador de status
      uint32_t  address;       // Endereço atual
      uint8_t  *buffer;        // Buffer de dados
  } io_device_t;
  ```

- Use enumerações para grupos de constantes relacionadas:

  ```c
  typedef enum {
      REGION_JAPAN,
      REGION_US,
      REGION_EUROPE
  } region_t;
  ```

## Tratamento de Erros

- Verifique valores de retorno de funções.
- Use constantes de erro definidas em `global_defines.h`.
- Seja consistente no tratamento de erros:

  ```c
  int result = function_call();
  if (result < 0) {
      return result;  // Propaga o erro
  }
  ```

- Forneça mensagens de erro claras e descritivas.

## Práticas Específicas por Plataforma

### Mega Drive / Genesis

- Use prefixos `md_` ou `megadrive_` para funções e variáveis específicas.
- Siga as convenções de nomenclatura do hardware original quando relevante.

### Master System

- Use prefixos `sms_` ou `mastersystem_` para funções e variáveis específicas.

### NES / Famicom

- Use prefixos `nes_` para funções e variáveis específicas.

### SNES / Super Famicom

- Use prefixos `snes_` para funções e variáveis específicas.

## Tamanho e Complexidade

1. **Funções:**
   - Limite de 50 linhas por função (ideal)
   - Máximo de 100 linhas por função (absoluto)
   - Uma única responsabilidade por função

2. **Arquivos:**
   - Limite de 300-400 linhas por arquivo (ideal)
   - Máximo de 1000 linhas por arquivo (absoluto)
   - Separar em módulos lógicos quando ultrapassar o limite

3. **Complexidade:**
   - Evite aninhamento excessivo (máximo 3 níveis)
   - Prefira early returns para reduzir aninhamento
   - Limite de 10 para complexidade ciclomática

## Mensagens de Commit

- Use mensagens claras e descritivas
- Siga o formato: `tipo: descrição curta`
- Tipos: feat, fix, docs, style, refactor, test, chore

Exemplo:

```
feat: adiciona suporte ao mapper MMC3
fix: corrige timing da PPU
docs: atualiza documentação da API
```

## Frontend React/TypeScript

Com a migração do frontend para React e TypeScript, as seguintes convenções devem ser seguidas:

### Convenções de Nomenclatura

- **Componentes React**: Use `PascalCase` para nomes de componentes

  ```tsx
  // Correto
  function GameDisplay() { ... }
  const ControlPanel = () => { ... }

  // Incorreto
  function gameDisplay() { ... }
  const control_panel = () => { ... }
  ```

- **Arquivos**: Use `PascalCase` para arquivos de componentes e `camelCase` para utilitários e hooks

  ```
  GameScreen.tsx
  EmulatorControls.tsx
  useEmulatorState.ts
  formatTime.ts
  ```

- **Props de Componentes**: Use `camelCase` para props

  ```tsx
  interface ButtonProps {
    onClick: () => void;
    isDisabled?: boolean;
    buttonText: string;
  }
  ```

- **Interfaces e Types**: Use `PascalCase` com sufixo descritivo

  ```tsx
  interface EmulatorState { ... }
  type ButtonVariant = 'primary' | 'secondary' | 'danger';
  ```

### Estrutura de Arquivos

```
src/
│
├── components/              # Componentes compartilhados
│   ├── common/              # Componentes UI reutilizáveis
│   │   ├── Button/
│   │   │   ├── Button.tsx
│   │   │   ├── Button.test.tsx
│   │   │   └── Button.module.css
│   │   └── ...
│   │
│   ├── emulator/            # Componentes específicos do emulador
│   │   ├── ControlPanel/
│   │   ├── GameDisplay/
│   │   └── ...
│   │
│   └── layout/              # Componentes de layout
│       ├── Header/
│       ├── Sidebar/
│       └── ...
│
├── hooks/                   # Hooks personalizados
│   ├── useEmulatorState.ts
│   ├── useWebSocket.ts
│   └── ...
│
├── services/                # Serviços e APIs
│   ├── emulator/            # Comunicação com o emulador
│   │   ├── websocket.ts
│   │   └── restApi.ts
│   └── ...
│
├── utils/                   # Funções utilitárias
│   ├── formatters.ts
│   ├── validators.ts
│   └── ...
│
├── state/                   # Gerenciamento de estado
│   ├── store.ts             # Configuração do Redux ou Context
│   ├── slices/              # Slices do Redux
│   │   ├── emulatorSlice.ts
│   │   └── ...
│   └── ...
│
├── types/                   # Definições de tipos globais
│   ├── emulator.types.ts
│   └── ...
│
├── pages/                   # Componentes de página
│   ├── Home/
│   ├── Emulator/
│   └── ...
│
└── App.tsx                  # Componente raiz da aplicação
```

### Padrões para Hooks e Gerenciamento de Estado

1. **Hooks Personalizados**:
   - Prefixe hooks personalizados com `use`, ex: `useEmulatorState`
   - Mantenha hooks focados em uma única responsabilidade
   - Documente claramente a função, parâmetros e retornos

   ```tsx
   /**
    * Hook para gerenciar conexão WebSocket com o emulador
    * @param url URL do servidor WebSocket
    * @param options Opções de configuração
    * @returns Estado da conexão e métodos para interagir com o WebSocket
    */
   function useWebSocket(url: string, options?: WebSocketOptions) {
     // implementação
   }
   ```

2. **Gerenciamento de Estado**:
   - Use Redux para estado global complexo
   - Organize o estado em slices lógicos
   - Use React Context para estado de escopo limitado
   - Prefira hooks como `useState` e `useReducer` para estado de componente

3. **Imutabilidade**:
   - Mantenha o estado imutável
   - Use funções como `map`, `filter` ou bibliotecas como Immer

### Diretrizes de Estilo

1. **CSS Modules**:
   - Use CSS Modules para estilos específicos de componentes
   - Nomeie arquivos como `Component.module.css`
   - Use nomes de classe descritivos e específicos do componente

2. **Tailwind CSS**:
   - Siga a ordem recomendada de utilidades (layout, tipografia, cores, etc.)
   - Crie componentes para padrões de design repetitivos
   - Use o plugin `@apply` para estilos complexos reutilizáveis

3. **Responsividade**:
   - Projete para mobile-first
   - Use breakpoints consistentes para responsividade
   - Teste em vários tamanhos de tela

### Testes

- Escreva testes unitários para componentes e hooks
- Use Jest e React Testing Library
- Teste comportamentos, não implementações
- Nomeie testes de forma clara e descritiva

## Integração Backend-Frontend

### Comunicação WebSocket

1. **Protocolo de Mensagens**:
   - Todas as mensagens devem seguir o formato JSON:

     ```json
     {
       "type": "ACTION_TYPE",
       "payload": { ... }
     }
     ```

   - Tipos de mensagens devem ser documentados em `docs/api/websocket-protocol.md`

2. **Implementação do Servidor**:
   - O código C deve implementar um servidor WebSocket conforme especificado em `websocket_server.h`
   - Mensagens devem ser processadas de forma assíncrona
   - Erros devem ser relatados com códigos consistentes

   ```c
   /**
    * @brief Inicializa o servidor WebSocket
    * @param port Porta para o servidor
    * @return 0 em caso de sucesso, código de erro em caso de falha
    */
   int ws_server_init(uint16_t port);

   /**
    * @brief Envia mensagem para todos os clientes conectados
    * @param message_type Tipo da mensagem
    * @param payload Dados da mensagem em formato JSON
    * @return 0 em caso de sucesso, código de erro em caso de falha
    */
   int ws_broadcast(const char* message_type, const char* payload);
   ```

3. **Tratamento de Estado**:
   - O backend deve manter estado mínimo necessário
   - Mudanças de estado devem ser transmitidas proativamente
   - O frontend deve sincronizar seu estado com o backend

### API REST

1. **Endpoints**:
   - Siga princípios RESTful para design de API
   - Use verbos HTTP apropriadamente (GET, POST, PUT, DELETE)
   - Versione a API (ex: `/api/v1/roms`)

2. **Formato de Resposta**:
   - Use JSON para todas as respostas
   - Siga um formato consistente:

     ```json
     {
       "success": true,
       "data": { ... },
       "error": null
     }
     ```

3. **Implementação**:
   - Use a biblioteca HTTP especificada em `http_server.h`
   - Documente endpoints com Swagger/OpenAPI

4. **Autenticação**:
   - Implemente autenticação para endpoints sensíveis
   - Use padrões modernos (JWT, OAuth quando aplicável)

---

Este guia deve ser seguido por todos os contribuidores do projeto. Modificações a este guia devem ser discutidas e aprovadas pela equipe de desenvolvimento.
