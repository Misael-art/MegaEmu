# Guia de Contribuição para o Frontend do Mega_Emu

Bem-vindo ao guia de contribuição para o frontend do Mega_Emu! Este documento fornece instruções detalhadas para configurar seu ambiente de desenvolvimento e contribuir efetivamente para o projeto.

## Índice

- [Visão Geral](#visão-geral)
- [Configuração do Ambiente](#configuração-do-ambiente)
- [Estrutura do Projeto](#estrutura-do-projeto)
- [Padrões de Código](#padrões-de-código)
- [Fluxo de Trabalho de Desenvolvimento](#fluxo-de-trabalho-de-desenvolvimento)
- [Testes](#testes)
- [Criando Pull Requests](#criando-pull-requests)
- [Diretrizes de UI/UX](#diretrizes-de-uiux)
- [Perguntas Frequentes](#perguntas-frequentes)

## Visão Geral

O frontend do Mega_Emu está em processo de migração para uma arquitetura moderna baseada em:

- **React 18+** - Biblioteca para construção de interfaces
- **TypeScript 5+** - Linguagem tipada para melhorar a manutenção
- **Redux Toolkit** - Gerenciamento de estado global
- **Tailwind CSS** - Estilização com classes utilitárias
- **WebSockets** - Comunicação em tempo real com o emulador
- **Jest & Testing Library** - Testes unitários e de componentes

Esta migração visa melhorar a experiência do usuário, permitir maior extensibilidade e facilitar a contribuição de novos desenvolvedores.

## Configuração do Ambiente

### Pré-requisitos

- Node.js 18.x ou superior
- npm 8.x ou superior (ou yarn 1.22+)
- Git
- Editor de código com suporte a TypeScript (recomendamos VS Code)

### Configuração Inicial

1. **Clone o repositório**

```bash
git clone https://github.com/Misael-art/MegaEmu.git
cd MegaEmu/frontend
```

2. **Instale as dependências**

```bash
npm install
# ou se preferir yarn
yarn install
```

3. **Configure a comunicação com o backend**

O frontend se comunica com o backend do emulador via WebSockets e REST API. Para desenvolvimento local, você precisa executar o backend separadamente.

```bash
# Em um terminal separado, na raiz do projeto
cd backend
make run
```

Por padrão, o frontend tentará se conectar a:

- WebSocket: `ws://localhost:8080/ws`
- REST API: `http://localhost:8080/api`

Você pode configurar estes endpoints no arquivo `.env.development`.

4. **Inicie o servidor de desenvolvimento**

```bash
npm start
# ou
yarn start
```

O servidor de desenvolvimento estará disponível em `http://localhost:3000` por padrão.

### Extensões Recomendadas para VS Code

Para melhorar sua experiência de desenvolvimento, recomendamos as seguintes extensões:

- **ESLint** - Linting de JavaScript/TypeScript
- **Prettier** - Formatação de código
- **Tailwind CSS IntelliSense** - Autocomplete para classes Tailwind
- **Jest Runner** - Executar testes diretamente do editor
- **Redux DevTools** - Depuração do estado Redux

## Estrutura do Projeto

O projeto segue uma estrutura organizada para facilitar a navegação e manutenção:

```
mega-emu-frontend/
├── public/                # Arquivos estáticos
├── src/
│   ├── api/               # Cliente para WebSocket e REST API
│   ├── components/        # Componentes React reutilizáveis
│   ├── config/            # Configurações da aplicação
│   ├── hooks/             # Hooks React personalizados
│   ├── pages/             # Componentes de página
│   ├── services/          # Lógica de negócio
│   ├── state/             # Estado global (Redux)
│   ├── styles/            # Estilos globais e temas
│   ├── types/             # Definições de tipos TypeScript
│   └── utils/             # Utilitários
└── ...
```

Consulte `docs/architecture/frontend-structure.md` para uma descrição mais detalhada.

## Padrões de Código

### Convenções de Nomenclatura

- **Arquivos de componentes**: Use `PascalCase` para nomes de arquivos e componentes
  - Exemplos: `Button.tsx`, `GameDisplay.tsx`

- **Arquivos de utilitários e hooks**: Use `camelCase`
  - Exemplos: `useEmulator.ts`, `formatTime.ts`

- **Hooks personalizados**: Sempre comece com `use`
  - Exemplos: `useWebSocket`, `useEmulatorState`

- **Interfaces e Types**: Use `PascalCase` e nomes descritivos
  - Exemplos: `EmulatorState`, `RomDetails`

### Estilização

Utilizamos Tailwind CSS como framework principal para estilização:

```tsx
// Exemplo de componente com Tailwind
const Button: React.FC<ButtonProps> = ({ children, onClick, variant = 'primary' }) => {
  const baseClasses = "px-4 py-2 rounded font-medium focus:outline-none transition";

  const variantClasses = {
    primary: "bg-blue-600 hover:bg-blue-700 text-white",
    secondary: "bg-gray-200 hover:bg-gray-300 text-gray-800",
    danger: "bg-red-600 hover:bg-red-700 text-white"
  };

  return (
    <button
      className={`${baseClasses} ${variantClasses[variant]}`}
      onClick={onClick}
    >
      {children}
    </button>
  );
};
```

Para casos mais complexos, utilize CSS Modules:

```tsx
// Button.module.css
.button {
  /* estilos base */
}

.primary {
  /* estilos variante primária */
}

// Button.tsx
import styles from './Button.module.css';

const Button = ({ variant = 'primary', ...props }) => {
  return (
    <button
      className={`${styles.button} ${styles[variant]}`}
      {...props}
    />
  );
};
```

### Estado Global

Utilizamos Redux Toolkit para gerenciamento de estado global. Consulte a documentação oficial para boas práticas: <https://redux-toolkit.js.org/usage/usage-guide>

```tsx
// Exemplo de slice Redux
import { createSlice, PayloadAction } from '@reduxjs/toolkit';

interface EmulatorState {
  status: 'idle' | 'loading' | 'running' | 'paused';
  currentRom: Rom | null;
}

const initialState: EmulatorState = {
  status: 'idle',
  currentRom: null
};

const emulatorSlice = createSlice({
  name: 'emulator',
  initialState,
  reducers: {
    setStatus(state, action: PayloadAction<EmulatorState['status']>) {
      state.status = action.payload;
    },
    setCurrentRom(state, action: PayloadAction<Rom>) {
      state.currentRom = action.payload;
      state.status = 'running';
    }
  }
});

export const { setStatus, setCurrentRom } = emulatorSlice.actions;
export default emulatorSlice.reducer;
```

### Regras Gerais

1. **Imutabilidade**: Sempre trate os dados de forma imutável, especialmente no estado Redux
2. **Componentes pequenos**: Mantenha componentes focados em uma única responsabilidade
3. **Tipos explícitos**: Defina explicitamente tipos TypeScript para maior segurança
4. **Evite any**: Evite o tipo `any` sempre que possível
5. **Comentários**: Documente funções/componentes complexos, mas evite comentários óbvios

## Fluxo de Trabalho de Desenvolvimento

### Branches

Siga a convenção de nomenclatura para branches:

- `feature/<nome-da-feature>` - Para novas funcionalidades
- `bugfix/<descrição-do-bug>` - Para correção de bugs
- `refactor/<descrição>` - Para refatorações
- `docs/<descrição>` - Para atualizações na documentação

### Commits

Siga a convenção de mensagens de commit:

```
<tipo>(<escopo>): <descrição>

[corpo opcional]

[rodapé opcional]
```

Tipos comuns:

- `feat`: Nova funcionalidade
- `fix`: Correção de bug
- `docs`: Alterações na documentação
- `style`: Formatação, ponto e vírgula faltando, etc; sem alteração de código
- `refactor`: Refatoração de código
- `test`: Adição ou correção de testes
- `chore`: Alterações em ferramentas, configurações, etc

Exemplo:

```
feat(emulator): adicionar suporte a controles virtuais na interface móvel

Implementar controles na tela para dispositivos móveis com suporte a multi-touch.
Inclui feedback visual e customização de layout.

Closes #123
```

### Processo de Desenvolvimento

1. **Sincronize seu branch local com a última versão do `develop`**

   ```bash
   git checkout develop
   git pull origin develop
   ```

2. **Crie uma nova branch para sua feature/bugfix**

   ```bash
   git checkout -b feature/nova-feature
   ```

3. **Desenvolva sua feature com commits frequentes**

4. **Execute os testes para garantir que nada foi quebrado**

   ```bash
   npm test
   ```

5. **Atualize a documentação se necessário**

6. **Faça push da sua branch para o repositório remoto**

   ```bash
   git push origin feature/nova-feature
   ```

7. **Crie um Pull Request para o branch `develop`**

## Testes

Utilizamos Jest e React Testing Library para testes de componentes e lógica:

```tsx
// Button.test.tsx
import { render, screen, fireEvent } from '@testing-library/react';
import Button from './Button';

describe('Button', () => {
  it('renders correctly', () => {
    render(<Button>Click me</Button>);
    expect(screen.getByText('Click me')).toBeInTheDocument();
  });

  it('calls onClick when clicked', () => {
    const handleClick = jest.fn();
    render(<Button onClick={handleClick}>Click me</Button>);
    fireEvent.click(screen.getByText('Click me'));
    expect(handleClick).toHaveBeenCalledTimes(1);
  });
});
```

### Executando Testes

```bash
# Executar todos os testes
npm test

# Executar testes com watch mode
npm test -- --watch

# Executar testes com coverage
npm test -- --coverage
```

### Diretrizes de Teste

1. **Teste comportamentos, não implementações**
2. **Use mocks com moderação**
3. **Escreva testes isolados e independentes**
4. **Cubra casos de borda e condições de erro**
5. **Mantenha uma cobertura de testes adequada (mínimo 70%)**

## Criando Pull Requests

Quando criar um Pull Request:

1. **Título descritivo**: Descreva claramente o que seu PR faz
2. **Descrição detalhada**: Explique as mudanças, motivação e impacto
3. **Link issues**: Referencie issues relacionadas (`Fixes #123`)
4. **Screenshots/videos**: Inclua screenshots/videos para alterações visuais
5. **Passos para teste**: Descreva como testar suas alterações

### Template de PR

```markdown
## Descrição
[Descreva as alterações em detalhes]

## Tipo de Mudança
- [ ] Nova feature
- [ ] Correção de bug
- [ ] Refatoração (sem alteração funcional)
- [ ] Atualização de documentação

## Como foi testado?
[Descreva os testes realizados]

## Screenshots
[Se aplicável, inclua screenshots]

## Issues relacionadas
[Mencione issues relacionadas, ex: "Closes #123"]
```

## Diretrizes de UI/UX

### Acessibilidade

- Use elementos semânticos HTML5 apropriados
- Garanta contraste adequado de cores (WCAG AA no mínimo)
- Adicione atributos `aria-*` quando necessário
- Teste a navegação por teclado
- Garanta que formulários são acessíveis

### Responsividade

O frontend deve funcionar bem em diferentes tamanhos de tela:

- **Desktop**: 1200px e acima
- **Tablet**: 768px a 1199px
- **Mobile**: 320px a 767px

Use os breakpoints do Tailwind:

- `sm`: 640px
- `md`: 768px
- `lg`: 1024px
- `xl`: 1280px
- `2xl`: 1536px

### Temas

O frontend suporta temas claros e escuros, além de temas específicos para cada console emulado:

- Use variáveis CSS para cores e estilos temáticos
- Nunca hardcode cores diretamente nos componentes
- Teste seu componente em todos os temas

## Perguntas Frequentes

### Como depurar problemas de comunicação com o backend?

Use as ferramentas de desenvolvedor do navegador para monitorar o tráfego WebSocket e as requisições HTTP. Você também pode ativar logs detalhados:

```tsx
// Para ativar logs detalhados de WebSocket
localStorage.setItem('debug', 'websocket:*');
```

### Como implementar uma nova ferramenta de desenvolvimento?

1. Crie um novo componente na pasta `src/components/tools/`
2. Registre a ferramenta no gerenciador de ferramentas em `src/state/slices/toolsSlice.ts`
3. Implemente a comunicação com o backend para obter os dados necessários
4. Adicione testes e documentação para sua ferramenta

### Como adicionar um novo tema?

1. Crie um novo arquivo CSS em `src/styles/themes/`
2. Defina as variáveis CSS para seu tema
3. Registre o tema em `src/config/themes.ts`
4. Adicione uma opção para selecionar o tema nas configurações

### Como posso contribuir se não tenho experiência com emuladores?

Existem várias áreas onde você pode contribuir, mesmo sem conhecimento profundo de emulação:

- Melhorias na UI/UX
- Documentação
- Testes
- Acessibilidade
- Otimizações de performance
- Traduções

---

Esperamos que este guia ajude você a contribuir efetivamente para o frontend do Mega_Emu. Se você tiver dúvidas ou sugestões, não hesite em abrir uma issue ou entrar em contato com a equipe de desenvolvimento.

Bem-vindo ao time! 🎮
