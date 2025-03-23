# Mega Emu Frontend

Este é o frontend moderno do emulador Mega Emu, implementado com React e TypeScript conforme o plano de migração do projeto.

## Visão Geral

O frontend do Mega Emu foi desenvolvido para oferecer uma interface moderna, responsiva e amigável para o emulador, seguindo os princípios de design especificados na documentação do projeto. A implementação utiliza React com TypeScript, Material UI para componentes de interface e Redux para gerenciamento de estado.

## Tecnologias Utilizadas

- **React**: Biblioteca JavaScript para construção de interfaces
- **TypeScript**: Superset tipado de JavaScript
- **Redux**: Gerenciamento de estado global
- **Material UI**: Biblioteca de componentes React
- **React Router**: Navegação entre páginas
- **Socket.io-client**: Conexão WebSocket com o backend
- **Axios**: Requisições HTTP

## Estrutura do Projeto

A estrutura do projeto segue o padrão especificado na documentação:

```
src/
│
├── components/              # Componentes compartilhados
│   ├── common/              # Componentes UI reutilizáveis
│   ├── emulator/            # Componentes específicos do emulador
│   └── layout/              # Componentes de layout
│
├── hooks/                   # Hooks personalizados
│
├── services/                # Serviços e APIs
│   └── emulator/            # Comunicação com o emulador
│
├── utils/                   # Funções utilitárias
│
├── state/                   # Gerenciamento de estado
│   ├── store.ts             # Configuração do Redux
│   └── slices/              # Slices do Redux
│
├── types/                   # Definições de tipos globais
│
└── pages/                   # Componentes de página
```

## Recursos Implementados

- **Camada de Comunicação**: Implementação da comunicação WebSocket e REST API com o backend do emulador
- **Interface do Emulador**: Exibição de frames e controles do emulador
- **Navegação**: Sistema de rotas e navegação entre diferentes seções
- **Tema**: Suporte a temas claro e escuro
- **Layout Responsivo**: Adaptação para diferentes tamanhos de tela

## Camada de Comunicação

A comunicação com o backend do emulador é feita através de duas camadas:

1. **WebSocket**: Para comunicação em tempo real (frames, inputs, atualizações de estado)
2. **REST API**: Para operações não relacionadas a tempo real (configurações, gerenciamento de ROMs)

## Como Executar o Projeto

### Pré-requisitos

- Node.js 14.x ou superior
- npm 6.x ou superior
- Backend do Mega Emu em execução

### Instalação

1. Clone o repositório:

   ```
   git clone https://github.com/seu-usuario/mega-emu.git
   cd mega-emu/frontend
   ```

2. Instale as dependências:

   ```
   npm install
   ```

3. Inicie o servidor de desenvolvimento:

   ```
   npm start
   ```

O aplicativo estará disponível em `http://localhost:3000`.

### Build para Produção

Para criar uma versão otimizada para produção:

```
npm run build
```

Os arquivos de build serão gerados na pasta `build/`.

## Comunicação com o Backend

O frontend espera que o backend do emulador esteja em execução com os seguintes serviços:

- **WebSocket**: `ws://localhost:8080`
- **REST API**: `http://localhost:8081/api`

Esses endereços podem ser configurados nos arquivos de serviço correspondentes.

## Próximos Passos

1. Implementação das páginas de ROMs e configurações
2. Implementação completa das ferramentas de desenvolvimento
3. Testes automatizados
4. Otimização de desempenho
5. Suporte a dispositivos móveis

## Documentação Adicional

Para mais informações sobre o projeto Mega Emu, consulte os seguintes documentos:

- [CODING_STANDARDS.md](../docs/CODING_STANDARDS.md): Padrões de codificação do projeto
- [AI_ESCOPO.md](../docs/AI_ESCOPO.md): Escopo e arquitetura do projeto
- [ROADMAP.md](../docs/ROADMAP.md): Roadmap de desenvolvimento

## Contribuição

Contribuições são bem-vindas! Por favor, consulte as diretrizes de contribuição antes de enviar pull requests.

## Lançamento do Frontend

Para facilitar o lançamento do frontend, foram criados scripts auxiliares:

### Usando o Script PowerShell

1. Para iniciar o frontend em modo de desenvolvimento:

   ```
   .\scripts\run_frontend.ps1 -dev
   ```

2. Para compilar e iniciar em modo de produção:

   ```
   .\scripts\run_frontend.ps1 -prod
   ```

3. Para apenas compilar o frontend:

   ```
   .\scripts\run_frontend.ps1 -build
   ```

### Usando o Script Batch (Windows)

Para iniciar o frontend em modo de desenvolvimento:

```
scripts\build\start_modern_frontend.bat
```

## Desenvolvimento

### Pré-requisitos

- Node.js 14.x ou superior
- npm 6.x ou superior

### Comandos Disponíveis

No diretório do projeto, você pode executar:

#### `npm start`

Inicia o aplicativo no modo de desenvolvimento.
Abra [http://localhost:3002](http://localhost:3002) para visualizá-lo no navegador.

#### `npm test`

Inicia o executor de teste no modo de observação interativa.

#### `npm run build`

Compila o aplicativo para produção na pasta `build`.

## Acessando o Frontend

Após iniciar o servidor de desenvolvimento ou produção, o frontend estará disponível em:

- **Desenvolvimento**: [http://localhost:3002](http://localhost:3002)
- **Produção**: [http://localhost:5000](http://localhost:5000) (usando `serve -s build`)

## Status da Implementação

Confira a página de Debug no frontend para ver o status atual da implementação de recursos.

## Avisos de Linting

O projeto usa ESLint para garantir a qualidade do código. Alguns avisos podem aparecer durante o desenvolvimento:

```
[eslint]
src\App.tsx
  Line 1:17:  'useEffect' is defined but never used  @typescript-eslint/no-unused-vars
```

Estes avisos não impedem o funcionamento do aplicativo, mas devem ser corrigidos para manter a qualidade do código.
