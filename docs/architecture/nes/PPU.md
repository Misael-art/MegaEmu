# Documentação da PPU (Picture Processing Unit) do NES

## Visão Geral

A Picture Processing Unit (PPU) é o componente responsável pela geração de vídeo no Nintendo Entertainment System (NES). Este documento descreve a implementação da PPU em nosso emulador, cobrindo todos os aspectos essenciais de seu funcionamento, características especiais e considerações de compatibilidade.

## Características Principais

A PPU do NES tem as seguintes características principais:

- **Resolução**: 256×240 pixels (NTSC), embora apenas aproximadamente 224 linhas sejam visíveis em TVs CRT padrão devido ao overscan
- **Cores**: Paleta de 64 cores derivadas de um sinal NTSC/PAL, com 25 cores disponíveis simultaneamente
- **Sprites**: Até 64 sprites por quadro, com no máximo 8 por linha de varredura
- **Background**: Suporte a rolagem suave (fine scrolling) em X e Y
- **Tiles**: Padrões de 8×8 pixels organizados em tabelas de padrões (pattern tables)
- **Tabelas de Nomes (Name Tables)**: 2 tabelas de 1KB para definir o layout da tela (extensível para 4 através de espelhamento)
- **Paletas**: 8 paletas de 4 cores cada (4 para background, 4 para sprites)

## Arquitetura Interna

### Espaços de Memória

A PPU possui vários espaços de memória distintos:

1. **VRAM (Memória de Vídeo)**: 2KB para armazenar dados de tabelas de nomes (name tables) e atributos
2. **OAM (Object Attribute Memory)**: 256 bytes para armazenar dados de sprites
3. **Paletas**: 32 bytes para armazenar as paletas de cores

### Registradores

A PPU é controlada através de 8 registradores, acessíveis pela CPU nos endereços $2000-$2007:

- **$2000 (PPUCTRL)**: Controle geral da PPU
- **$2001 (PPUMASK)**: Controle de renderização
- **$2002 (PPUSTATUS)**: Status da PPU
- **$2003 (OAMADDR)**: Endereço OAM
- **$2004 (OAMDATA)**: Dados OAM
- **$2005 (PPUSCROLL)**: Controle de rolagem
- **$2006 (PPUADDR)**: Endereço VRAM
- **$2007 (PPUDATA)**: Dados VRAM

### Ciclo de Renderização

A PPU opera em uma sequência de ciclos precisos para renderizar cada quadro:

1. **Scanlines Visíveis (0-239)**: Renderização de fundo e sprites
2. **Scanline de Post-render (240)**: Finalização do quadro atual
3. **Scanlines de VBlank (241-260)**: Período em que a CPU pode acessar a memória da PPU sem interferir na renderização
4. **Scanline de Pre-render (-1 ou 261)**: Preparação para o próximo quadro

Cada scanline consiste em 341 ciclos de PPU, com diferentes operações ocorrendo em momentos específicos.

## Implementação Detalhada

### Renderização de Background

A renderização de fundo utiliza os seguintes componentes:

1. **Tabelas de Padrões**: Contêm os tiles de 8×8 pixels
2. **Tabelas de Nomes**: Definem quais tiles aparecem em cada posição da tela
3. **Tabelas de Atributos**: Definem qual paleta é usada para cada bloco de 2×2 tiles

O processo de renderização de background inclui:

- Busca de tiles e atributos da memória
- Deslocamento de bits para implementar rolagem suave
- Seleção de cores baseada em paletas

### Renderização de Sprites

Os sprites são renderizados usando os dados do OAM (Object Attribute Memory), que armazena 4 bytes por sprite:

1. **Posição Y**: Coordenada Y do topo do sprite
2. **Índice do Tile**: Aponta para o padrão do sprite na tabela de padrões
3. **Atributos**: Define paleta, prioridade e flags de espelhamento
4. **Posição X**: Coordenada X da esquerda do sprite

O processo de renderização de sprites inclui:

- Avaliação de sprites (sprite evaluation) para determinar quais sprites são visíveis na scanline atual
- Busca de dados de sprites da memória
- Detecção de colisão de sprite zero
- Tratamento de prioridade entre sprites e entre sprites e background

### Suporte a Diferentes Sistemas de TV

Nossa implementação suporta os três principais sistemas de TV usados com o NES:

1. **NTSC**: Usado no Japão e América do Norte (60Hz)
2. **PAL**: Usado na Europa (50Hz)
3. **Dendy**: Clone russo do NES (50Hz)

Cada sistema tem suas próprias características:

- Diferentes paletas de cores
- Diferentes taxa de quadros (framerate)
- Variações sutis no timing

### Efeitos Especiais

#### Split Scrolling

O split scrolling permite que diferentes partes da tela tenham configurações de rolagem diferentes. Isso é utilizado em muitos jogos para implementar HUDs fixos enquanto o resto da tela rola.

Nossa implementação suporta:

- Divisão horizontal (split horizontal)
- Divisão vertical (split vertical)
- Mudança dinâmica do ponto de divisão

#### Overscan

O overscan emula o comportamento de TVs CRT, onde as bordas da imagem não são visíveis. Nossa implementação permite:

- Configurar a quantidade de overscan em cada borda (superior, inferior, esquerda, direita)
- Habilitar/desabilitar o overscan para compatibilidade com diferentes displays

#### Efeitos de Distorção CRT

Efeitos opcionais que simulam as características de uma TV CRT:

- Distorção geométrica
- Variação de brilho
- Simulação de linhas de varredura

## Funcionalidades Avançadas

### Suporte a Modos de Espelhamento

A PPU suporta vários modos de espelhamento da VRAM:

1. **Horizontal**: Espelha horizontalmente as tabelas de nomes ($2000 = $2400, $2800 = $2C00)
2. **Vertical**: Espelha verticalmente as tabelas de nomes ($2000 = $2800, $2400 = $2C00)
3. **Single Screen**: Utiliza apenas uma tabela de nomes, espelhada para todas as posições
4. **Four Screen**: Utiliza quatro tabelas de nomes independentes (requer hardware especial no cartucho)

### Detecção de Sprite Zero

O sprite zero (primeiro sprite no OAM) tem a capacidade especial de gerar uma flag quando colide com um pixel opaco do background. Esta funcionalidade é frequentemente utilizada para sincronizar efeitos de rolagem e é completamente implementada em nosso emulador.

### Cache de Tiles

Para otimização de desempenho, implementamos um sistema de cache de tiles que armazena padrões recentemente utilizados, reduzindo significativamente a quantidade de acessos à memória durante a renderização.

## Validação e Testes

Nossa implementação foi validada usando uma variedade de testes:

1. **Testes de ROM específicos**: Como PPU test roms, Blargg's PPU tests, e Mesen test suite
2. **Teste de jogos comerciais**: Verificação em jogos conhecidos por utilizarem recursos avançados da PPU
3. **Testes unitários**: Cobertura completa de todas as funcionalidades em nível de componente
4. **Testes de integração**: Verificação da interação entre a PPU e outros componentes do sistema

## Problemas Conhecidos e Soluções

### Rolagem com Efeitos Especiais

Alguns jogos utilizam técnicas avançadas para implementar efeitos de rolagem que dependem de timing preciso e comportamentos específicos do hardware. Implementamos:

- Suporte para mid-scanline register updates
- Emulação precisa de race conditions
- Suporte a efeitos de mudança de rolagem no meio do frame

### Precisão de Timing

A PPU do NES tem um timing muito específico que muitos jogos exploram. Nossa implementação:

- Emula o timing em nível de ciclo
- Considera os delays apropriados para acessos à memória
- Implementa corretamente o comportamento de latch dos registradores

## Compatibilidade

Nossa implementação da PPU é capaz de executar corretamente todos os jogos comerciais lançados para o NES, incluindo aqueles que utilizam técnicas avançadas, como:

- Legend of Zelda
- Super Mario Bros. 3
- Battletoads
- Micro Machines
- MMC3 games com IRQs baseados em scanline

## Referências

1. NesDev Wiki - PPU Documentation: https://wiki.nesdev.org/w/index.php/PPU
2. Visual 2C02 Technical Reference: https://www.nesdev.org/2C02%20technical%20reference.TXT
3. Mesen Emulator Documentation
4. Blargg's NES Tests
5. Brad Taylor's NES Technical/Emulation/Development Resources

## Histórico de Atualizações

- **Maio 2024**: Implementação completa finalizada
  - Corrigidos problemas de scrolling com jogos específicos
  - Implementados efeitos de distorção (overscan)
  - Adicionado suporte completo a paletas alternativas (PAL, Dendy)
  - Documentação completa criada

- **Abril 2024**: Melhorias de desempenho
  - Implementação de cache de tiles otimizado
  - Redução de acessos desnecessários à memória
  - Testes de precisão e compatibilidade

- **Março 2024**: Implementação inicial
  - Suporte básico a renderização de background e sprites
  - Emulação correta dos registradores
  - Suporte a tabelas de nomes e padrões
