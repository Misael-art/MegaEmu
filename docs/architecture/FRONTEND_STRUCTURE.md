# Estrutura do Frontend Mega_Emu

Este documento descreve a estrutura do frontend do projeto Mega_Emu, que foi migrado para React e TypeScript para proporcionar uma experiência de usuário moderna e responsiva.

## Visão Geral

O frontend do Mega_Emu foi desenvolvido utilizando React, TypeScript e diversas bibliotecas modernas para criar uma interface intuitiva para o emulador. A arquitetura foi projetada para funcionar tanto em ambiente desktop (via Electron) quanto diretamente no navegador, utilizando mocks para APIs nativas quando necessário.

## Tecnologias Principais

- **React 19**: Biblioteca JavaScript para construção de interfaces
- **TypeScript**: Superset tipado de JavaScript para desenvolvimento mais seguro
- **Redux**: Gerenciamento de estado global da aplicação
- **Material UI**: Biblioteca de componentes React para design moderno
- **React Router**: Navegação entre páginas da aplicação
- **Axios**: Cliente HTTP para comunicação com o backend

## Modos de Operação

O frontend foi projetado para funcionar em dois modos:

1. **Modo Desktop**: Acesso completo ao sistema de arquivos e recursos nativos
2. **Modo Browser**: Funciona diretamente no navegador, utilizando mocks para APIs nativas

## Estrutura de Diretórios

```
frontend/
├── public/              # Arquivos estáticos
├── src/                 # Código fonte
│   ├── api/             # Comunicação com o backend
│   │   └── emulator/    # Serviços relacionados ao emulador
│   ├── components/      # Componentes React
│   │   ├── common/      # Componentes compartilhados
│   │   ├── emulator/    # Componentes específicos do emulador
│   │   ├── layout/      # Componentes de layout
│   │   └── ...          # Outros grupos de componentes
│   ├── hooks/           # Hooks personalizados
│   ├── pages/           # Componentes de página
│   ├── services/        # Serviços
│   │   └── emulator/    # Serviços relacionados ao emulador
│   ├── state/           # Gerenciamento de estado (Redux)
│   │   ├── actions/     # Ações Redux
│   │   ├── reducers/    # Redutores Redux
│   │   ├── slices/      # Slices (Redux Toolkit)
│   │   └── store.ts     # Configuração da store
│   ├── styles/          # Estilos CSS
│   ├── types/           # Definições TypeScript
│   └── utils/           # Utilitários
│       └── environment.ts # Detecção de ambiente
└── package.json         # Dependências e scripts
```

## Principais Componentes

### Páginas

- **EmulatorPage**: Página principal do emulador com display de jogo e controles
- **RomsPage**: Gerenciamento da biblioteca de ROMs
- **SettingsPage**: Configurações do emulador
- **SaveStatesPage**: Gerenciamento de save states

### Componentes de Emulador

- **GameDisplay**: Renderiza os frames do emulador
- **ControlPanel**: Controles do emulador (play, pause, reset, etc.)
- **SaveStateCard**: Exibe e gerencia save states individuais
- **UploadRomDialog**: Dialog para upload de ROMs

### Layout

- **Header**: Cabeçalho da aplicação com navegação
- **Sidebar**: Barra lateral com acesso rápido às principais funções
- **Footer**: Rodapé com informações da aplicação
- **Theme**: Provedor de tema com suporte a modo claro/escuro

## Serviços Principais

### EmulatorApiService

Interface para comunicação com o backend do emulador. Utiliza o axios para chamadas REST e suporta operações como:

- Carregar ROMs
- Gerenciar save states
- Configurar o emulador
- Verificar status do backend

### RomService

Serviço para gerenciamento de ROMs locais. Realiza operações como:

- Busca de ROMs no sistema de arquivos
- Upload de novas ROMs
- Extração de metadados

Este serviço utiliza acesso direto ao sistema de arquivos quando no modo desktop, e utiliza mocks dessas funções quando no modo browser.

### SaveStateService

Gerencia as operações relacionadas a save states:

- Criar save states
- Carregar save states
- Remover save states
- Exportar/importar save states

## Sistema de Mocks

Para garantir a compatibilidade com o navegador, o frontend implementa mocks para APIs nativas:

### fsMock.ts

Mock para as funções do módulo `fs` do Node.js:

- `existsSync`: Simula verificação de existência de arquivos
- `readdirSync`: Simula listagem de diretórios
- `statSync`: Simula estatísticas de arquivos
- `readFileSync`: Simula leitura de arquivos

### pathMock.ts

Mock para as funções do módulo `path` do Node.js:

- `join`: Simula junção de caminhos
- `basename`: Simula extração do nome base de um caminho
- `dirname`: Simula extração do diretório de um caminho
- `extname`: Simula extração da extensão de um arquivo

## Detecção de Ambiente

O arquivo `utils/environment.ts` contém funções para detectar o ambiente em que o aplicativo está sendo executado:

```typescript
export const isElectron = (): boolean => {
  // @ts-ignore
  return typeof window !== 'undefined' &&
         typeof window.process === 'object' &&
         window.process.type === 'renderer';
};

export const isBrowser = (): boolean => {
  return !isElectron();
};
```

## Estado Global (Redux)

O estado global da aplicação é gerenciado através do Redux Toolkit, com os seguintes slices principais:

- **emulatorSlice**: Estado do emulador (rodando, pausado, console atual)
- **romsSlice**: Biblioteca de ROMs e ROM atualmente selecionada
- **saveStatesSlice**: Gerenciamento de save states
- **uiSlice**: Estado da interface (tema, layout, dialogs abertos)

## Integração com Backend

A comunicação com o backend ocorre de duas formas:

1. **API REST**: Para operações não relacionadas a tempo real
2. **WebSockets** (opcional): Para atualização em tempo real de frames e estados

## Configuração do Ambiente

O arquivo `.env` contém configurações importantes para o funcionamento do frontend:

```
REACT_APP_API_URL=http://localhost:3002
BROWSER_IGNORE_NODE_MODULES=fs,path,stream,buffer
```

## Modo Browser vs Modo Desktop

### Modo Desktop (via Electron)

No modo desktop, o frontend tem acesso direto ao sistema de arquivos através dos módulos Node.js:

```typescript
import fs from 'fs';
import path from 'path';

// Uso direto das APIs nativas
const files = fs.readdirSync(someDir);
```

### Modo Browser

No modo browser, o frontend utiliza mocks para simular as APIs nativas:

```typescript
import { fs, path } from './services/emulator/fsMock';

// Uso dos mocks que simulam o comportamento
const files = fs.readdirSync(someDir);
```

A detecção do ambiente é feita automaticamente através da função `isElectron()`.

## Guia de Desenvolvimento

### Adicionando um Novo Componente

1. Crie o arquivo do componente na pasta apropriada
2. Defina a interface de props usando TypeScript
3. Implemente o componente utilizando hooks React quando necessário
4. Exporte o componente como default

Exemplo:

```typescript
import React from 'react';

interface MyComponentProps {
  title: string;
  onAction: () => void;
}

const MyComponent: React.FC<MyComponentProps> = ({ title, onAction }) => {
  return (
    <div>
      <h2>{title}</h2>
      <button onClick={onAction}>Ação</button>
    </div>
  );
};

export default MyComponent;
```

### Adicionando uma Nova Página

1. Crie o arquivo da página na pasta `pages`
2. Implemente a página como um componente React
3. Adicione a rota no arquivo de rotas

### Adicionando um Novo Serviço

1. Crie o arquivo do serviço na pasta `services`
2. Implemente as funções necessárias
3. Se o serviço usa APIs nativas, certifique-se de criar mocks para modo browser

## Compilação e Execução

### Desenvolvimento

```bash
# Instalar dependências
npm install

# Executar em modo de desenvolvimento
npm start
```

### Produção

```bash
# Criar build de produção
npm run build
```

## Considerações de Performance

- Utilize React.memo para componentes que não mudam frequentemente
- Implemente code-splitting para carregamento sob demanda
- Otimize renderizações com useMemo e useCallback
- Prefira o uso de componentes funcionais e hooks

## Testes

Utilize Jest e Testing Library para testes:

```bash
# Executar todos os testes
npm test

# Executar testes com cobertura
npm test -- --coverage
```
