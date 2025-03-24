# Diretrizes de Desenvolvimento do Mega_Emu

Este documento estabelece as diretrizes técnicas e padrões para o desenvolvimento do projeto Mega_Emu. Todos os contribuidores devem seguir essas orientações para manter a consistência e qualidade do código.

## Padrões de Codificação

### Estilo e Formatação

1. **Nomenclatura:**
   - Tipos e Typedefs: `snake_case_t` (ex: `emulator_context_t`)
   - Estruturas: `CamelCase` (ex: `EmulatorContext`)
   - Macros e constantes: `UPPER_CASE` (ex: `MAX_ROM_SIZE`)
   - Funções e variáveis: `snake_case` (ex: `init_emulator`)

2. **Prefixos:**
   - APIs públicas: `emu_` (ex: `emu_initialize()`)
   - Funções/variáveis privadas: `_` (ex: `_internal_function()`)
   - Macros e constantes: `EMU_` (ex: `EMU_MAX_SCANLINES`)
   - Ferramentas de desenvolvimento: `dev_` (ex: `dev_sprite_viewer_init()`)
   - Componentes de interface: `ui_` (ex: `ui_button_create()`)

3. **Identação e Formatação:**
   - Indentação: 4 espaços (não tabs)
   - Limite de 100 caracteres por linha
   - Chaves em nova linha para funções, mesma linha para blocos
   - Espaço após palavras-chave (`if`, `for`, `while`, etc.)

4. **Comentários:**
   - Utilize Doxygen para documentação de API
   - Documente todos os parâmetros, retornos e efeitos colaterais
   - Inclua comentários explicativos para código complexo
   - Mantenha comentários atualizados ao modificar código

### Tamanho e Complexidade

1. **Funções:**
   - Limite de 50 linhas por função (ideal)
   - Máximo de 100 linhas por função (absoluto)
   - Uma única responsabilidade por função
   - Máximo de 5 parâmetros por função

2. **Arquivos:**
   - Limite de 300-400 linhas por arquivo (ideal)
   - Máximo de 1000 linhas por arquivo (absoluto)
   - Separar em módulos lógicos quando ultrapassar o limite

3. **Complexidade:**
   - Evite aninhamento excessivo (máximo 3 níveis)
   - Prefira early returns para reduzir aninhamento
   - Limite de 10 para complexidade ciclomática

## Práticas de Programação

### Gerais

1. **Gerenciamento de Memória:**
   - Sempre libere recursos alocados
   - Verifique retornos de funções de alocação
   - Evite vazamentos de memória com valgrind/memcheck
   - Utilize RAII (C++) quando apropriado

2. **Tratamento de Erros:**
   - Retorne códigos de erro consistentes (definidos em `error_codes.h`)
   - Verifique todos os retornos de funções
   - Documente condições de erro
   - Implemente logs detalhados para falhas

3. **Concorrência:**
   - Evite condições de corrida
   - Proteja recursos compartilhados
   - Evite bloqueios e deadlocks
   - Documente premissas sobre threading

4. **Segurança:**
   - Valide toda entrada do usuário
   - Evite buffer overflows
   - Não exponha informações sensíveis em logs
   - Implemente limites e verificações para recursos

### Específicas para Emulação

1. **Precisão vs. Desempenho:**
   - Precisão é prioritária em casos de conflito
   - Otimizações não devem comprometer a precisão
   - Documente compromissos quando necessário
   - Inclua opções para usuários priorizarem desempenho

2. **Emulação de CPU:**
   - Implemente emulação ciclo-a-ciclo precisa
   - Dê especial atenção ao timing de interrupções
   - Documente comportamentos não-documentados
   - Implemente instruções ilegais e casos de borda

3. **Emulação de Vídeo e Áudio:**
   - Garanta sincronização precisa com CPU
   - Implemente efeitos especiais e casos de borda
   - Suporte diferentes modos de vídeo e filtros
   - Mantenha latência mínima sem comprometer precisão

## Diretrizes para Ferramentas de Desenvolvimento

### Princípios de Design para Ferramentas

1. **Modularidade:**
   - Cada ferramenta deve ser um módulo separado
   - Defina interfaces claras entre ferramentas
   - Permita que ferramentas operem independentemente ou em conjunto
   - Siga o padrão MVC (Model-View-Controller)

2. **Consistência:**
   - Mantenha UI/UX consistente entre ferramentas
   - Use terminologia consistente em toda a interface
   - Reutilize componentes de UI quando possível
   - Implemente comportamentos padronizados (atalhos, menus, etc.)

3. **Extensibilidade:**
   - Projete para permitir extensões futuras
   - Implemente sistema de plugins
   - Separe interface de implementação
   - Forneça hooks para personalização

4. **Performance:**
   - Garanta que ferramentas não impactem significativamente a emulação
   - Implemente renderização e processamento eficientes
   - Use threading para operações pesadas
   - Implemente lazy loading e virtualização para grandes conjuntos de dados

### Implementação de Ferramentas Visuais

1. **Sprite Viewer:**
   - Utilize extração direta da VRAM em tempo real
   - Implemente visualização baseada em OpenGL para performance
   - Organize sprites por camadas e prioridade
   - Implemente sistema de cache para melhorar performance
   - Garanta que a extração de paletas seja específica por plataforma
   - Use uma estrutura de dados unificada para representação de sprites:

     ```c
     typedef struct {
         uint32_t id;              // ID único do sprite
         uint8_t* data;            // Dados brutos
         uint32_t width;           // Largura em pixels
         uint32_t height;          // Altura em pixels
         uint8_t bpp;              // Bits por pixel
         palette_t* palette;       // Paleta associada
         sprite_attributes_t attr; // Atributos específicos da plataforma
     } sprite_t;
     ```

2. **Dev Art Tools:**
   - Implemente ferramentas específicas para cada formato gráfico
   - Suporte importação/exportação em formatos modernos
   - Forneça previsualização em tempo real no hardware emulado
   - Siga as restrições de cada plataforma (paletas, tamanhos, etc.)
   - Implemente histórico de ações (undo/redo)
   - Use representação intermediária para conversão entre formatos:

     ```c
     typedef struct {
         graphics_format_t format;   // Formato dos dados
         conversion_rules_t rules;   // Regras de conversão
         platform_limits_t limits;   // Limitações da plataforma
     } graphics_converter_t;
     ```

3. **Memory Viewer:**
   - Implemente acesso direto à memória emulada
   - Forneça múltiplas visualizações (hex, decimal, ASCII, etc.)
   - Suporte bookmarks e regiões nomeadas
   - Implemente watches e breakpoints na memória
   - Otimize para grandes volumes de memória
   - Use acesso indireto via callbacks:

     ```c
     typedef struct {
         void* context;             // Contexto do emulador
         uint8_t (*read_byte)(void* context, uint32_t address);
         void (*write_byte)(void* context, uint32_t address, uint8_t value);
         memory_map_t* memory_map;  // Mapa de regiões de memória
     } memory_access_t;
     ```

4. **Event Viewer:**
   - Implemente sistema de captura de eventos em pontos-chave da emulação
   - Use estrutura de timeline para visualização
   - Suporte filtragem e busca de eventos
   - Garanta baixo overhead para não afetar a emulação
   - Implemente buffer circular para histórico de eventos
   - Use estrutura unificada para eventos:

     ```c
     typedef struct {
         uint64_t timestamp;        // Timestamp do evento
         event_type_t type;         // Tipo do evento
         uint32_t source;           // Fonte do evento
         uint32_t target;           // Alvo do evento
         uint32_t data[4];          // Dados específicos
         char description[64];      // Descrição legível
     } system_event_t;
     ```

5. **Sound Monitor:**
   - Implemente visualização em tempo real dos canais de áudio
   - Forneça análise de espectro e forma de onda
   - Permita controle individual de canais (mute, solo)
   - Suporte exportação de áudio em formatos modernos
   - Garanta sincronização precisa com a emulação
   - Use interfaces padronizadas:

     ```c
     typedef struct {
         uint32_t num_channels;           // Número de canais
         audio_channel_t* channels;       // Array de canais
         audio_format_t format;           // Formato de áudio
         void (*get_samples)(void* ctx, float** buffers, uint32_t num_samples);
         void* context;                   // Contexto do emulador
     } sound_monitor_t;
     ```

6. **Dev Editor:**
   - Implemente destaque de sintaxe para assembly de cada CPU
   - Suporte montagem em tempo real e injeção de código
   - Integre-se com o debugger para breakpoints e stepping
   - Forneça sugestões e autocompletar para instruções
   - Implemente gerenciamento de projetos
   - Use estrutura modular:

     ```c
     typedef struct {
         editor_buffer_t* buffer;         // Buffer de texto
         syntax_highlighter_t* highlighter; // Destacador de sintaxe
         assembler_t* assembler;          // Montador específico da CPU
         project_t* project;              // Projeto atual
         ui_component_t* ui;              // Componente de UI
     } dev_editor_t;
     ```

7. **Node IDE:**
   - Implemente sistema visual baseado em nodos e conexões
   - Forneça nodos para acesso a componentes do emulador
   - Suporte para scripting avançado via linguagem de alto nível
   - Sistema de execução em tempo real ou diferido
   - Integração com outras ferramentas
   - Estrutura básica:

     ```c
     typedef struct {
         node_graph_t* graph;             // Grafo de nodos
         node_library_t* library;         // Biblioteca de nodos disponíveis
         execution_context_t* exec_ctx;   // Contexto de execução
         ui_canvas_t* canvas;             // Canvas para renderização
     } node_ide_t;
     ```

### Implementação de Ferramentas Integradas

1. **Dev Tools Suite:**
   - Implemente sistema de janelas ancoráveis (docking)
   - Forneça gerenciamento de workspace e layouts
   - Implemente sistema de comunicação entre ferramentas
   - Suporte temas e personalização de UI
   - Sistema de extensões e plugins
   - Estrutura recomendada:

     ```c
     typedef struct {
         tool_registry_t* tools;          // Registro de ferramentas
         window_manager_t* window_mgr;    // Gerenciador de janelas
         plugin_manager_t* plugin_mgr;    // Gerenciador de plugins
         ui_theme_t* theme;               // Tema atual
         ui_workspace_t* workspace;       // Workspace atual
     } dev_tools_suite_t;
     ```

2. **ROM Analyzer:**
   - Implemente disassembly estático e análise de código
   - Forneça detecção de padrões (loops, tabelas, etc.)
   - Suporte anotações e comentários
   - Implemente análise de uso de recursos da plataforma
   - Gere relatórios detalhados
   - Estrutura básica:

     ```c
     typedef struct {
         rom_data_t* rom;                 // Dados da ROM
         disassembler_t* disasm;          // Disassembler
         pattern_detector_t* detector;    // Detector de padrões
         annotation_db_t* annotations;    // Banco de anotações
         resource_analyzer_t* res_analyzer; // Analisador de recursos
     } rom_analyzer_t;
     ```

3. **Patch Creator:**
   - Implemente suporte para formatos de patch (IPS, BPS, UPS)
   - Forneça interface visual para comparação de ROMs
   - Suporte criação de patches condicionais
   - Implemente sistema de versionamento
   - Integre com controle de versão
   - Estrutura recomendada:

     ```c
     typedef struct {
         rom_data_t* original_rom;        // ROM original
         rom_data_t* modified_rom;        // ROM modificada
         patch_format_t format;           // Formato do patch
         diff_engine_t* diff_engine;      // Motor de diferenciação
         patch_builder_t* builder;        // Construtor de patches
     } patch_creator_t;
     ```

4. **ROM Builder:**
   - Implemente sistema de build para ROMs
   - Suporte templates para diferentes plataformas
   - Forneça gerenciamento de assets
   - Integre com outras ferramentas de desenvolvimento
   - Suporte automação via scripts
   - Estrutura sugerida:

     ```c
     typedef struct {
         project_t* project;              // Projeto atual
         build_config_t* config;          // Configuração de build
         asset_manager_t* assets;         // Gerenciador de assets
         build_pipeline_t* pipeline;      // Pipeline de build
         platform_template_t* template;   // Template da plataforma
     } rom_builder_t;
     ```

### API para Ferramentas de Desenvolvimento

A integração das ferramentas com o emulador deve seguir uma API consistente:

```c
// Inicialização e finalização
int dev_tool_init(dev_tool_t* tool, emu_context_t* context);
int dev_tool_shutdown(dev_tool_t* tool);

// Ciclo de vida
int dev_tool_update(dev_tool_t* tool);
int dev_tool_render(dev_tool_t* tool);
int dev_tool_process_event(dev_tool_t* tool, const event_t* event);

// Comunicação com o emulador
int dev_tool_register_hooks(dev_tool_t* tool, hook_manager_t* hooks);
int dev_tool_notify(dev_tool_t* tool, notification_type_t type, const void* data);

// Serialização de estado
int dev_tool_save_state(dev_tool_t* tool, state_writer_t* writer);
int dev_tool_load_state(dev_tool_t* tool, state_reader_t* reader);
```

Cada ferramenta deve implementar essa interface, garantindo interoperabilidade e consistência.

## Arquitetura do Frontend

### Sistema de GUI

1. **Arquitetura em Camadas:**
   - **Core:** Componentes fundamentais do sistema de GUI
     - `gui_types.h`: Definição de tipos básicos (retângulos, cores, IDs)
     - `gui_manager.c/h`: Gerenciamento centralizado de elementos
     - `gui_element.c/h`: Base para todos os elementos de interface
   - **Widgets:** Componentes de interface reutilizáveis
     - Implementados como extensões do elemento base
     - Cada widget em arquivos separados (gui_[widget].c/h)
     - Interfaces públicas bem definidas para cada widget
   - **Adaptador de Plataforma:** Camada de abstração para SDL2
     - Encapsula chamadas específicas da plataforma
     - Facilita portabilidade para outras bibliotecas gráficas

2. **Widgets Implementados:**
   - **Button (Botão):**
     - Suporte a cliques e hover
     - Callbacks para eventos de clique
     - Personalização de cores e aparência
   - **Label (Rótulo):**
     - Exibição de texto com diferentes alinhamentos
     - Opções de transparência e cores
     - Alinhamento horizontal e vertical configurável
   - **Text Box (Caixa de Texto):**
     - Entrada e edição de texto pelo usuário
     - Sistema de foco e cursor de texto
     - Callbacks para mudanças de texto
     - Opções de somente leitura e tamanho máximo
     - Personalização de cores e bordas

3. **Widgets Planejados:**
   - List (Lista)
   - Progress Bar (Barra de Progresso)
   - Checkbox (Caixa de Seleção)
   - Radio Button (Botão de Opção)
   - Dropdown (Menu Suspenso)
   - Canvas (Área de Desenho)
   - Table (Tabela)
   - Tree View (Visualização em Árvore)
   - Splitter (Divisor de Painéis)
   - Tab View (Visualização em Abas)
   - Dockable Panels (Painéis Ancoráveis)

4. **Padrões de Implementação:**
   - Cada widget deve implementar funções para:
     - Criação e inicialização
     - Renderização
     - Processamento de eventos
     - Configuração de propriedades
     - Callbacks para interatividade
   - Prefixos de função consistentes: `gui_[widget]_[ação]`
   - Documentação completa em estilo Doxygen
   - Testes unitários para cada widget

5. **Integração com o Emulador:**
   - Interface unificada via frontend.c/h
   - Inicialização automática do sistema de GUI
   - Processamento de eventos integrado ao loop principal
   - Renderização sincronizada com o ciclo de atualização

6. **Exemplo de Demonstração:**
   - Disponível em `examples/gui_demo/`
   - Demonstra a criação e uso de todos os widgets
   - Serve como referência para implementações futuras
   - Atualizado com cada novo widget implementado

## Compatibilidade

### Multiplataforma

1. **Sistemas Operacionais:**
   - Windows 10+ (Visual Studio 2019+, MinGW)
   - Linux (GCC 9+, Clang 10+)
   - macOS 10.14+ (AppleClang)

2. **Práticas de Compatibilidade:**
   - Use abstrações para código específico de plataforma
   - Evite dependências exclusivas de uma plataforma
   - Teste em todas as plataformas suportadas
   - Prefira bibliotecas cross-platform

3. **Requisitos de Hardware:**
   - Mínimo: CPU dual-core, 2 GB RAM, GPU com OpenGL 3.3+
   - Recomendado: CPU quad-core, 4 GB RAM, GPU dedicada

### Dependências

1. **Bibliotecas e Ferramentas:**
   - SDL2 para gráficos, áudio e entrada
   - CMake 3.15+ para build system
   - C++17 ou superior para código moderno
   - Dear ImGui para interfaces gráficas avançadas
   - OpenGL para renderização acelerada
   - PortAudio para processamento de áudio avançado
   - Lua para scripting e extensibilidade
   - Manter lista atualizada em `docs/THIRD_PARTY.md`

2. **Gestão de Dependências:**
   - Preferir dependências disponíveis via package managers
   - Documentar versões específicas necessárias
   - Incluir scripts de configuração para facilitar setup
   - Manter dependências externas no diretório `deps/`

## Testes e Qualidade

### Testes

1. **Testes Unitários:**
   - Cobertura mínima de 70% para core e platforms
   - Usar framework consistente (GoogleTest, Catch2)
   - Teste cada componente isoladamente
   - Inclua casos de borda e caminhos de erro

2. **Testes de Integração:**
   - Teste interações entre componentes
   - Use ROMs de teste conhecidas (test_roms/)
   - Valide comportamento entre diferentes sistemas
   - Automatize verificação de resultados (scripts de teste : Teste Integração.bat e Teste Unitário.bat)
   - Documentar resultados em relatório de validação

3. **Testes de Regressão:**
   - Execute testes em cada PR e commit
   - Compare resultados com baselines conhecidos
   - Mantenha suite de testes para bugs corrigidos
   - Automatize via CI/CD

4. **Testes de UI:**
   - Teste funcionalidades da interface do usuário
   - Valide fluxos de usuário comuns
   - Teste responsividade e acessibilidade
   - Automatize testes de UI quando possível

### Qualidade de Código

1. **Análise Estática:**
   - Execute ferramentas como clang-tidy, cppcheck
   - Zero warnings em build de produção
   - Configure CI para verificar conformidade
   - Inclua regras de linting em .clang-format

2. **Profiling e Otimização:**
   - Identifique gargalos com ferramentas de profiling
   - Documente decisões de otimização
   - Benchmark antes e depois de otimizações
   - Mantenha testes de performance

## Processo de Desenvolvimento

### Fluxo de Trabalho

1. **Branches:**
   - `main` para código estável
   - `develop` para desenvolvimento ativo
   - `feature/*` para novas funcionalidades
   - `bugfix/*` para correções
   - `release/*` para preparação de releases

2. **Commits:**
   - Mensagens claras e descritivas
   - Formato: `tipo: descrição curta`
   - Tipos: feat, fix, docs, style, refactor, test, chore
   - Referência a issues quando aplicável

3. **Pull Requests:**
   - Descrição completa das mudanças
   - Testes para novas funcionalidades
   - Documentação atualizada
   - Conformidade com padrões de código

4. **Code Reviews:**
   - Pelo menos uma aprovação necessária
   - Verificação de qualidade e conformidade
   - Foco em correção, performance e segurança
   - Considerar impacto nas plataformas suportadas

## Referências

- @[ESCOPO.md] para entendimento do projeto
- @[CODING_STANDARDS.md] para detalhes específicos de codificação
- @[MEMORIA.md] para histórico de decisões técnicas
- @[ROADMAP.md] para planejamento futuro
- @[ARCHITECTURE.md] para detalhes da arquitetura do sistema
- @[TOOLS_ARCHITECTURE.md] para detalhes específicos da arquitetura de ferramentas

## Diretrizes para Migração de Frontend

A migração do frontend tradicional baseado em SDL2 para uma arquitetura moderna React/TypeScript é um objetivo estratégico do projeto. Estas diretrizes estabelecem como implementar esta transição de forma eficiente e eficaz.

### Princípios da Migração

1. **Separação de Responsabilidades:**
   - Separar claramente o frontend (interface de usuário) do backend (emulador)
   - Implementar uma camada de comunicação robusta entre eles
   - Garantir que cada componente possa evoluir independentemente

2. **Arquitetura de Comunicação:**
   - Utilizar WebSockets para comunicação em tempo real (frames, inputs, estados)
   - Implementar API REST para operações não-tempo-real (configurações, gerenciamento de ROMs)
   - Desenvolver um protocolo de mensagens bem definido

3. **Modernização Incremental:**
   - Migrar o frontend em fases, mantendo a compatibilidade
   - Implementar novas funcionalidades já no novo frontend
   - Permitir coexistência das interfaces durante a transição

### Padrões de Implementação

1. **Servidor WebSocket:**
   - Implementar em C++ usando uma biblioteca moderna (libwebsockets, Beast)
   - Otimizar para baixa latência e alta performance
   - Implementar protocolos de compressão para transmissão de frames
   - Código exemplo:

     ```cpp
     /**
      * @brief Inicializa o servidor WebSocket
      * @param port Porta para escuta
      * @return Status da inicialização
      */
     int websocket_server_init(uint16_t port) {
         // Inicialização básica do servidor
         server_context = ws_create_context();

         // Configuração de handlers
         ws_set_frame_handler(server_context, handle_frame_request);
         ws_set_input_handler(server_context, handle_input_event);

         // Iniciar servidor na porta especificada
         return ws_server_start(server_context, port);
     }
     ```

2. **API REST:**
   - Implementar endpoints RESTful para operações administrativas
   - Documentar completamente usando Swagger/OpenAPI
   - Implementar autenticação quando necessário
   - Seguir práticas modernas de design de API

3. **Frontend React:**
   - Utilizar React com TypeScript para tipagem estática
   - Implementar arquitetura baseada em componentes
   - Seguir padrões modernos de gerenciamento de estado
   - Otimizar a renderização para jogabilidade fluida

### Ferramentas de Desenvolvimento na Nova Arquitetura

1. **Implementação como Componentes React:**
   - Converter cada ferramenta existente para componentes React
   - Utilizar abordagem modular para facilitar manutenção
   - Aproveitar bibliotecas React especializadas quando apropriado
   - Exemplo de estrutura para uma ferramenta:

     ```typescript
     // SpriteViewer.tsx
     import React, { useEffect, useRef } from 'react';
     import { useWebSocket } from '@/hooks/useWebSocket';
     import { useEmulatorState } from '@/hooks/useEmulatorState';

     interface SpriteViewerProps {
       platform: EmulatorPlatform;
     }

     export const SpriteViewer: React.FC<SpriteViewerProps> = ({ platform }) => {
       const canvasRef = useRef<HTMLCanvasElement>(null);
       const { send, lastMessage } = useWebSocket();
       const { emulatorState } = useEmulatorState();

       useEffect(() => {
         // Solicitar sprites quando componente montar
         send({ type: 'GET_SPRITES', payload: { platform } });
       }, []);

       useEffect(() => {
         if (lastMessage?.type === 'SPRITES_DATA') {
           renderSprites(canvasRef.current, lastMessage.payload);
         }
       }, [lastMessage]);

       return (
         <div className="sprite-viewer">
           <div className="sprite-viewer-controls">
             {/* Controles da ferramenta */}
           </div>
           <canvas ref={canvasRef} width={800} height={600} />
         </div>
       );
     };
     ```

2. **Integração com Websocket:**
   - Implementar hooks React para conexão WebSocket
   - Desenvolver sistema de mensagens tipadas
   - Garantir reconexão automática e resiliência
   - Exemplo de hook WebSocket:

     ```typescript
     // useWebSocket.ts
     import { useState, useEffect, useCallback } from 'react';

     export function useWebSocket(url: string) {
       const [socket, setSocket] = useState<WebSocket | null>(null);
       const [lastMessage, setLastMessage] = useState<any>(null);
       const [isConnected, setIsConnected] = useState(false);

       // Função para enviar mensagens
       const send = useCallback((message: any) => {
         if (socket?.readyState === WebSocket.OPEN) {
           socket.send(JSON.stringify(message));
         }
       }, [socket]);

       // Inicializar socket
       useEffect(() => {
         const ws = new WebSocket(url);

         ws.onopen = () => setIsConnected(true);
         ws.onclose = () => setIsConnected(false);
         ws.onmessage = (event) => setLastMessage(JSON.parse(event.data));

         setSocket(ws);

         return () => ws.close();
       }, [url]);

       return { isConnected, lastMessage, send };
     }
     ```

### Diretrizes para IA no Desenvolvimento

A inteligência artificial pode auxiliar significativamente no processo de migração:

1. **Geração de Código:**
   - A IA pode gerar esqueletos de componentes React
   - Ajudar na conversão de código C/SDL para TypeScript/React
   - Gerar tipos TypeScript baseados nas estruturas C existentes
   - Propor implementações de hooks personalizados

2. **Verificação de Consistência:**
   - Verificar conformidade com padrões de codificação
   - Garantir que todos os componentes sigam a mesma arquitetura
   - Alertar sobre potenciais problemas de performance

3. **Documentação e Comentários:**
   - Gerar documentação JSDoc/TSDoc para APIs
   - Explicar padrões de design e decisões arquiteturais
   - Criar tutoriais e exemplos de uso

4. **Testes:**
   - Propor casos de teste para componentes e hooks
   - Gerar testes unitários e de integração
   - Identificar edge cases importantes para testar

### Abordagem Faseada

1. **Fase 1: Infraestrutura (Q1-Q2 2024)**
   - Implementação da camada de comunicação
   - Criação de protótipos de componentes React básicos
   - Definição de arquitetura e padrões

2. **Fase 2: Implementação Paralela (Q3-Q4 2024)**
   - Desenvolvimento da interface principal
   - Implementação de componentes específicos
   - Início da migração de ferramentas

3. **Fase 3: Transição Completa (Q1 2025)**
   - Finalização de todas as ferramentas de desenvolvimento
   - Testes completos e otimização
   - Descontinuação gradual da interface SDL2
   - Lançamento da versão 2.0

### Avaliação e Métricas

Para garantir que a migração seja bem-sucedida, as seguintes métricas serão monitoradas:

1. **Performance:**
   - Latência de input-to-display
   - FPS e estabilidade
   - Uso de memória e CPU

2. **Usabilidade:**
   - Feedback de usuários
   - Tempo para completar tarefas comuns
   - Taxa de adoção da nova interface

3. **Desenvolvimento:**
   - Velocidade de implementação de novas features
   - Número de bugs reportados
   - Qualidade e cobertura de testes

### Recursos e Referências para React/TypeScript

- Documentação oficial do React: <https://reactjs.org/docs>
- Documentação do TypeScript: <https://www.typescriptlang.org/docs>
- Material UI para componentes: <https://mui.com>
- Redux Toolkit para estado global: <https://redux-toolkit.js.org>
- Storybook para documentação de componentes: <https://storybook.js.org>
- React Testing Library: <https://testing-library.com/docs/react-testing-library/intro>

# Guia de Build do Mega_Emu

## Estrutura de Diretórios

O projeto utiliza a seguinte estrutura de diretórios para builds:

```
/build/
  ├── test/           # Builds de testes
  ├── temp/           # Arquivos temporários
  ├── emulators/      # Builds dos emuladores
  │   ├── nes/
  │   ├── megadrive/
  │   └── mastersystem/
  ├── frontend/       # Builds dos frontends
  │   ├── sdl/
  │   └── qt/
  ├── Mega_tools/     # Builds das ferramentas
  └── released/       # Builds para release
      └── [versão]/
```

## Scripts de Build

Os scripts de build estão localizados em `/scripts/build/` e são organizados da seguinte forma:

- `build_all.ps1`: Compila todos os componentes
- `build_nes.ps1`: Compila apenas o emulador NES
- `build_megadrive.ps1`: Compila apenas o emulador Mega Drive
- `build_frontend_sdl.ps1`: Compila apenas o frontend SDL
- `build_tools.ps1`: Compila apenas as ferramentas Mega_tools

### Uso dos Scripts

Cada script aceita os seguintes parâmetros:

```powershell
.\build_[componente].ps1 [-BuildType <Release|Debug>] [-Clean] [-Rebuild]
```

Exemplos:

```powershell
# Build completo em modo Release
.\build_all.ps1

# Build do NES em modo Debug
.\build_nes.ps1 -BuildType Debug

# Rebuild limpo do frontend SDL
.\build_frontend_sdl.ps1 -Rebuild
```

## Regras de Build

1. **Modularidade**: Cada componente pode ser compilado independentemente
2. **Consistência**: Todos os scripts usam as mesmas funções utilitárias de `/scripts/common/`
3. **Isolamento**: Cada build é feito em seu próprio diretório
4. **Limpeza**: Use `-Clean` para remover builds anteriores
5. **Dependências**: Gerenciadas automaticamente via vcpkg

## Dependências

O sistema gerencia automaticamente:

- Visual Studio Build Tools
- CMake
- Ninja
- vcpkg
- SDL2 (quando necessário)
- Qt (quando necessário)

## Estrutura do CMake

O projeto usa uma estrutura modular de CMake:

1. **Root CMakeLists.txt**: Configuração global e opções de build
2. **src/CMakeLists.txt**: Configuração dos componentes principais
3. **CMakeLists.txt** específicos para cada componente

### Opções de Build

```cmake
option(BUILD_TESTS "Build tests" OFF)
option(USE_SDL2 "Use SDL2 for graphics and audio" ON)
option(BUILD_NES "Build NES emulation" ON)
option(BUILD_MEGADRIVE "Build Mega Drive emulation" ON)
option(BUILD_MASTERSYSTEM "Build Master System emulation" ON)
option(BUILD_FRONTEND_SDL "Build SDL2 frontend" ON)
option(BUILD_FRONTEND_QT "Build Qt frontend" OFF)
option(BUILD_TOOLS "Build Mega_tools" ON)
```

## Verificações de Build

O sistema inclui verificações para:

1. Ambiente de compilação correto
2. Dependências instaladas
3. Configuração do vcpkg
4. Integridade dos arquivos gerados

## Fluxo de Build Recomendado

1. **Desenvolvimento**:
   - Use builds individuais dos componentes
   - Mantenha o modo Debug
   - Use o diretório `build/temp` para testes

2. **Testes**:
   - Compile com `-DBUILD_TESTS=ON`
   - Use o diretório `build/test`
   - Execute a suíte de testes completa

3. **Release**:
   - Use `build_all.ps1` em modo Release
   - Verifique a integridade dos binários
   - Gere a documentação
   - Crie o pacote de distribuição

## Boas Práticas

1. **Sempre use os scripts fornecidos** em vez de comandos CMake diretos
2. **Mantenha os diretórios de build limpos** usando `-Clean` periodicamente
3. **Documente mudanças** nas dependências ou no processo de build
4. **Teste em modo Debug** antes de fazer builds de Release
5. **Use o sistema de versionamento** adequadamente

## Troubleshooting

1. **Build falha com erro de dependência**:
   - Verifique se vcpkg está atualizado
   - Execute o script com `-Clean`

2. **Conflitos de versão**:
   - Limpe o cache do CMake
   - Verifique o arquivo de lock do vcpkg

3. **Erros de compilação**:
   - Verifique logs em `build/temp`
   - Use modo Debug para mais informações

## Notas Adicionais

- Os scripts são idempotentes e podem ser executados múltiplas vezes
- O sistema mantém logs detalhados em `build/temp`
- Use `build/released` apenas para builds finais
- Mantenha o vcpkg atualizado para evitar problemas de dependência

## Diretrizes de Compilação para IA

### Estrutura de Build

1. **Hierarquia de Diretórios**

   ```
   /build/
     ├── test/           # Builds de testes
     ├── temp/           # Arquivos temporários
     ├── emulators/      # Builds dos emuladores
     │   ├── nes/
     │   ├── megadrive/
     │   └── mastersystem/
     ├── frontend/       # Builds dos frontends
     │   ├── sdl/
     │   └── qt/
     ├── Mega_tools/     # Builds das ferramentas
     └── released/       # Builds para release
         └── [versão]/
   ```

2. **Scripts de Build**
   - `build_all.ps1`: Compila todos os componentes
   - `build_nes.ps1`: Compila apenas o emulador NES
   - `build_megadrive.ps1`: Compila apenas o emulador Mega Drive
   - `build_frontend_sdl.ps1`: Compila apenas o frontend SDL
   - `build_tools.ps1`: Compila apenas as ferramentas Mega_tools

### Padrões de Compilação

1. **Parâmetros de Build**

   ```powershell
   # Formato padrão
   .\build_[componente].ps1 [-BuildType <Release|Debug>] [-Clean] [-Rebuild]
   ```

2. **Opções de CMake**

   ```cmake
   # Opções padrão para cada componente
   option(BUILD_TESTS "Build tests" OFF)
   option(USE_SDL2 "Use SDL2 for graphics and audio" ON)
   option(BUILD_NES "Build NES emulation" ON)
   option(BUILD_MEGADRIVE "Build Mega Drive emulation" ON)
   option(BUILD_MASTERSYSTEM "Build Master System emulation" ON)
   option(BUILD_FRONTEND_SDL "Build SDL2 frontend" ON)
   option(BUILD_FRONTEND_QT "Build Qt frontend" OFF)
   option(BUILD_TOOLS "Build Mega_tools" ON)
   ```

3. **Flags de Compilação**

   ```cmake
   # Windows (MSVC)
   add_compile_options(/W4 /WX)

   # Unix (GCC/Clang)
   add_compile_options(-Wall -Wextra -Wpedantic -Werror)
   ```

### Regras para IA

1. **Verificações Obrigatórias**
   - Validar existência de dependências
   - Verificar estrutura de diretórios
   - Confirmar configuração do vcpkg
   - Checar integridade dos arquivos gerados

2. **Sequência de Build**

   ```powershell
   # Sequência padrão
   .\scripts\build\check_environment.ps1
   .\scripts\build\setup_vcpkg.ps1
   .\scripts\build\create_build_structure.ps1
   .\scripts\build\build_all.ps1
   .\scripts\build\verify_binaries.ps1
   ```

3. **Tratamento de Erros**
   - Documentar erros comuns e soluções
   - Fornecer mensagens claras de erro
   - Implementar recuperação de falhas
   - Manter logs detalhados

4. **Validação de Resultados**

   ```powershell
   # Verificações pós-build
   .\scripts\build\run_tests.ps1
   .\scripts\build\verify_binaries.ps1
   ```

### Diretrizes para Testes

1. **Estrutura de Testes**

   ```cmake
   # Configuração padrão de testes
   enable_testing()
   add_subdirectory(tests)
   ```

2. **Categorias de Teste**
   - Unitários: Componentes individuais
   - Integração: Interação entre componentes
   - Performance: Métricas de desempenho
   - Regressão: Verificação de bugs corrigidos
   - Benchmark: Testes de performance detalhados

3. **Execução de Testes**

   ```powershell
   # Execução padrão
   .\scripts\build\run_tests.ps1 [-Verbose] [-FailFast] [-Filter "*"]
   ```

### Manutenção e Documentação

1. **Atualização de Versão**

   ```powershell
   # Processo de atualização
   .\scripts\build\update_version.ps1 -Version "x.y.z"
   .\scripts\build\update_changelog.ps1 -Version "x.y.z" -Type "Added"
   ```

2. **Geração de Documentação**

   ```powershell
   # Gerar documentação
   .\scripts\build\generate_docs.ps1 [-Force] [-IncludePrivate]
   ```

3. **Criação de Release**

   ```powershell
   # Processo de release
   .\scripts\build\create_release.ps1 -Version "x.y.z"
   ```

### Integração Contínua

1. **GitHub Actions**

   ```yaml
   # Workflow padrão
   name: CI/CD
   on: [push, pull_request]
   jobs:
     build:
       runs-on: windows-latest
       steps:
         - uses: actions/checkout@v3
         - name: Build
           run: .\scripts\build\build_all.ps1
         - name: Test
           run: .\scripts\build\run_tests.ps1
   ```

2. **Verificações Automáticas**
   - Lint do código
   - Testes unitários
   - Testes de integração
   - Verificação de cobertura
   - Análise estática
