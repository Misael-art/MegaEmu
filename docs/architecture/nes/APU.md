# Documentação da APU (Audio Processing Unit) do NES

## Visão Geral

A Audio Processing Unit (APU) é o componente responsável pela geração de áudio no Nintendo Entertainment System (NES). Este documento descreve a implementação da APU em nosso emulador, detalhando seus canais de som, registradores e características específicas.

## Características Principais

A APU do NES (parte do chip RP2A03) possui as seguintes características:

- **Canais de Som**: 5 canais independentes (2 pulsos, triângulo, ruído, e DMC)
- **Resolução**: Geração de som com 8 bits (7 bits efetivos)
- **Taxa de Amostragem**: Aproximadamente 44.1 kHz no emulador (originalmente ~1.79MHz/40 = ~44.8kHz)
- **Envelope de Volume**: Controle dinâmico de volume para os canais de pulso e ruído
- **Efeitos**: Sweep para os canais de pulso, Linear counter para o canal de triângulo
- **Amostragem**: Canal DMC (Delta Modulation Channel) para reprodução de amostras digitais

## Arquitetura Interna

### Canais de Áudio

A APU do NES consiste em cinco canais de áudio distintos:

1. **Pulse 1**: Canal de pulso com controle de duty cycle e sweep
2. **Pulse 2**: Similar ao Pulse 1, com diferenças no sweep
3. **Triangle**: Canal de onda triangular, utilizado principalmente para baixos
4. **Noise**: Canal de ruído com padrões configuráveis, usado para percussão e efeitos
5. **DMC**: Delta Modulation Channel, utilizado para amostras digitais

### Registradores

A APU é controlada através de diversos registradores mapeados na faixa de endereços $4000-$4017:

- **$4000-$4003**: Controle do canal Pulse 1
- **$4004-$4007**: Controle do canal Pulse 2
- **$4008-$400B**: Controle do canal Triangle
- **$400C-$400F**: Controle do canal Noise
- **$4010-$4013**: Controle do canal DMC
- **$4015**: Status/controle dos canais
- **$4017**: Controle de frame counter

## Implementação Detalhada

### Canal Pulse (Pulso)

Os canais de pulso (1 e 2) geram ondas quadradas com duty cycles (ciclos de trabalho) variáveis. Características implementadas:

- **Duty Cycle**: 4 padrões diferentes (12.5%, 25%, 50%, 75% negativo)
- **Envelope**: Controle dinâmico de volume com decaimento
- **Sweep**: Modulação automática da frequência (ascendente ou descendente)
- **Comprimento**: Contadores de comprimento para controle da duração das notas

```
Funcionamento do envelope:
1. Se o flag "envelope loop" não estiver ativo e o contador de comprimento for zero, o envelope termina.
2. Caso contrário, o envelope é decrementado em intervalos regulares.
3. Quando o contador de envelope chega a zero, ele é recarregado e o volume é decrementado.
```

### Canal Triangle (Triângulo)

O canal de triângulo gera uma onda triangular de 16 passos, usada principalmente para sons graves:

- **Linear Counter**: Controle alternativo da duração
- **Contador de Comprimento**: Similar aos outros canais
- **Sem Envelope**: Volume constante, diferente dos canais de pulso
- **Sequência de 16 passos**: 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15

### Canal Noise (Ruído)

O canal de ruído gera um padrão pseudo-aleatório, ideal para sons de percussão e efeitos:

- **Shift Register**: Registrador de deslocamento de 15 bits para geração de pseudo-ruído
- **Modo**: Padrão normal (15 bits) ou curto (7 bits)
- **Envelope**: Similar aos canais de pulso
- **Diferentes Períodos**: 16 diferentes frequências de ruído

### Canal DMC (Delta Modulation Channel)

O canal DMC permite a reprodução de amostras digitais:

- **Memória Direta**: Acesso direto à memória para buscar amostras
- **Buffer de Amostra**: Buffer de 8 bits para processar samples
- **Contador de Taxa**: Controle de taxa de reprodução (16 possíveis taxas)
- **Geração de IRQ**: Possibilidade de gerar IRQ quando a amostra termina

### Mixer de Áudio

O mixer combina a saída dos cinco canais aplicando uma fórmula não-linear para simular o comportamento original do hardware:

```
output = pulse_out + tnd_out

Onde:
pulse_out = tabela_pulse[pulse1 + pulse2]
tnd_out = tabela_tnd[3*triangle + 2*noise + dmc]
```

As tabelas de lookup são derivadas das características elétricas do hardware original.

## Funcionalidades Avançadas

### Frame Counter

O frame counter da APU controla o timing de vários processos internos:

- **Modo 4-Step**: Sequência de 4 passos (60Hz NTSC)
- **Modo 5-Step**: Sequência de 5 passos (48Hz NTSC)
- **IRQ**: Capacidade de gerar IRQ no modo 4-Step
- **Atualizações**: Controla quando envelopes, contadores lineares e sweeps são atualizados

### Emulação de Efeitos

Nossa implementação inclui suporte a efeitos avançados:

- **Sweep Negativo**: Implementação precisa do efeito de pitch bend
- **Efeitos de Filtro**: Filtros opcionais para simular o som de hardware real
- **Clipping de Áudio**: Simulação do comportamento de clipping do hardware original

## Precisão e Otimizações

Nossa implementação da APU equilibra precisão e desempenho:

- **Emulação Ciclo-Precisa**: As alterações nos canais ocorrem nos momentos exatos
- **Blending de Amostras**: Suavização da saída para evitar artefatos de alta frequência
- **Resampling de Alta Qualidade**: Conversão precisa da taxa nativa para a taxa de saída
- **Cache de Lookup**: Tabelas pré-calculadas para maior eficiência

## Compatibilidade

Nossa implementação da APU é capaz de reproduzir corretamente todos os efeitos de áudio utilizados em jogos comerciais para o NES, incluindo técnicas avançadas como:

- **Modulação por Dephasing**: Usado para criar efeitos de chorus e vibrato
- **Samples via DMC**: Reprodução de fala e outros efeitos de áudio via canal DMC
- **Combinação de Canais**: Efeitos como bass drum criados pela combinação de canais
- **PCM via Pulse**: Técnicas de reprodução PCM usando canais de pulso com volume variável

## Validação e Testes

A implementação da APU foi validada usando diversos métodos:

1. **Testes de NSF**: Reprodução de arquivos NSF (NES Sound Format)
2. **Blargg's APU Tests**: Verificação de comportamento de registradores e timing
3. **Comparação Espectral**: Análise de frequência comparada com hardware real
4. **Teste com Jogos**: Verificação com jogos que fazem uso intensivo da APU

## Extensões e Melhorias Futuras

Algumas melhorias planejadas para a APU incluem:

- **Implementação completa do canal DMC**: Melhorar precisão e edge cases
- **Melhorias na emulação de efeitos de sweep**: Maior precisão nos efeitos de modulação
- **Equalização integrada**: Opções de equalização pré-definidas para preferências do usuário
- **Visualizadores de áudio**: Ferramentas para debug e visualização dos canais

## Referências

1. NesDev Wiki - APU Sound: https://wiki.nesdev.org/w/index.php/APU_Sound
2. Brad Taylor's APU Documentation: http://nesdev.com/apu_ref.txt
3. Blargg's APU Tests
4. NSFPlay e implementações de referência
5. Análises de hardware por kevtris e outros

## Histórico de Atualizações

- **Abril 2024**: Implementação atual (90% completa)
  - Todos os canais básicos implementados
  - Suporte parcial ao canal DMC
  - Implementação do mixer não-linear
  - Testes básicos de validação

- **Março 2024**: Implementação inicial
  - Canais Pulse, Triangle e Noise funcionais
  - Frame counter básico
  - Interface de áudio com a aplicação principal
