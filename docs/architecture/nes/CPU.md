# Documentação da CPU (6502/2A03) do NES

## Visão Geral

A CPU do Nintendo Entertainment System (NES) é baseada no processador MOS Technology 6502, com algumas modificações específicas feitas pela Ricoh para criar o chip 2A03. Este documento descreve a implementação da CPU em nosso emulador, detalhando seu funcionamento, instruções suportadas e peculiaridades específicas do hardware.

## Características Principais

A CPU do NES (RP2A03) possui as seguintes características principais:

- **Processador**: Variante do MOS 6502, sem suporte a modo decimal (BCD)
- **Velocidade de Clock**: ~1.79 MHz (NTSC), ~1.66 MHz (PAL)
- **Instruções**: 56 instruções oficiais (151 opcodes) + 105 opcodes não documentados
- **Registradores**: A (Acumulador), X, Y (Índice), P (Status), S (Stack Pointer), PC (Program Counter)
- **Memória**: Espaço de endereçamento de 64KB (16 bits)
- **Interrupções**: NMI, IRQ, RESET

## Arquitetura Interna

### Registradores

A CPU do NES utiliza os seguintes registradores principais:

1. **Acumulador (A)**: Registrador de 8 bits, principal para operações aritméticas e lógicas
2. **Índice X (X)**: Registrador de 8 bits, utilizado para indexação e contagem
3. **Índice Y (Y)**: Registrador de 8 bits, utilizado para indexação e contagem
4. **Stack Pointer (S)**: Ponteiro para a pilha, localizada entre $0100-$01FF
5. **Program Counter (PC)**: Ponteiro de 16 bits para a próxima instrução
6. **Processor Status (P)**: Registrador de flags (C, Z, I, D, B, V, N)

### Flags do Processador

O registrador de status contém as seguintes flags:

- **C (Carry)**: Indica carry ou borrow em operações aritméticas
- **Z (Zero)**: Indica resultado zero em uma operação
- **I (Interrupt Disable)**: Quando ativo, desabilita interrupções IRQ
- **D (Decimal Mode)**: Modo decimal (BCD) - não utilizado no 2A03
- **B (Break)**: Indica uma instrução BRK
- **V (Overflow)**: Indica overflow em operações aritméticas com sinal
- **N (Negative)**: Indica resultado negativo (bit 7 ativo)

## Implementação Detalhada

### Instruções Oficiais

Nossa implementação suporta todas as 56 instruções oficiais do 6502, totalizando 151 opcodes. Estas instruções são categorizadas em:

1. **Transferência de Dados**: LDA, LDX, LDY, STA, STX, STY, TAX, TAY, TXA, TYA
2. **Operações Aritméticas**: ADC, SBC, INC, INX, INY, DEC, DEX, DEY
3. **Operações Lógicas**: AND, ORA, EOR, ASL, LSR, ROL, ROR
4. **Comparações e Testes**: CMP, CPX, CPY, BIT
5. **Desvios Condicionais**: BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS
6. **Desvios e Chamadas**: JMP, JSR, RTS, RTI
7. **Controle de Stack**: PHA, PHP, PLA, PLP, TSX, TXS
8. **Controle de Sistema**: BRK, NOP, SEC, SED, SEI, CLC, CLD, CLI, CLV

Cada instrução é implementada com seu timing correto em ciclos, incluindo ciclos extras para operações de página cruzada quando aplicável.

### Opcodes Não Documentados

Nossa implementação também suporta todos os 105 opcodes não documentados (ilegais) do 6502. Estes são importantes para compatibilidade com jogos que os utilizam, intencionalmente ou não. Alguns dos mais notáveis incluem:

- **LAX**: Carrega valor em A e X simultaneamente
- **SAX**: Armazena A & X na memória
- **DCP**: Decrementa memória e compara com A
- **ISC**: Incrementa memória e então subtrai de A
- **SLO**: Shift left e OR com A
- **RLA**: Rotate left e AND com A
- **SRE**: Shift right e EOR com A
- **RRA**: Rotate right e adiciona a A

### Modos de Endereçamento

O 6502 suporta vários modos de endereçamento que nossa implementação emula fielmente:

1. **Implícito**: A instrução não referencia um endereço (ex: NOP)
2. **Acumulador**: A operação é realizada no acumulador (ex: ASL A)
3. **Imediato**: O operando é um valor imediato (ex: LDA #$10)
4. **Zero Page**: O operando é um endereço na zero page (ex: LDA $10)
5. **Zero Page,X**: Endereço zero page indexado por X (ex: LDA $10,X)
6. **Zero Page,Y**: Endereço zero page indexado por Y (ex: LDX $10,Y)
7. **Absoluto**: O operando é um endereço completo de 16 bits (ex: LDA $1234)
8. **Absoluto,X**: Endereço absoluto indexado por X (ex: LDA $1234,X)
9. **Absoluto,Y**: Endereço absoluto indexado por Y (ex: LDA $1234,Y)
10. **Indireto**: O operando é um ponteiro para o endereço efetivo (ex: JMP ($1234))
11. **Indireto,X**: Endereço zero page indireto indexado por X (ex: LDA ($10,X))
12. **Indireto,Y**: Endereço zero page indireto indexado por Y (ex: LDA ($10),Y)
13. **Relativo**: Deslocamento relativo para instruções de branch (ex: BCC $10)

### Timing

O timing do 6502 é uma característica crítica para emulação correta. Nossa implementação considera:

- Ciclos base para cada instrução
- Ciclos extras para operações de página cruzada
- Ciclos extras para branches tomados
- Sincronização correta com outros componentes do sistema

### Interrupções

A CPU suporta três tipos de interrupções:

1. **NMI (Non-Maskable Interrupt)**: Gerada pelo PPU no início do VBlank
2. **IRQ (Interrupt Request)**: Pode ser gerada por mappers ou APU
3. **RESET**: Inicialização do sistema

Cada interrupção segue um processo específico de salvamento de estado na pilha e carregamento do vetor apropriado.

## Peculiaridades do 2A03

O chip 2A03 utilizado no NES é uma variante do 6502 com algumas diferenças importantes:

1. **Sem suporte a BCD**: O modo decimal (flag D) não tem efeito
2. **APU integrada**: Contém canais de áudio no mesmo chip
3. **Circuito de geração de clock**: Gera sinais de clock para todo o sistema
4. **Controladores de I/O**: Gerencia entrada dos controles

Nossa implementação captura todas essas peculiaridades para garantir compatibilidade máxima.

## Validação e Testes

A implementação da CPU foi validada através de:

1. **Suítes de testes funcionais**: Blargg's 6502 tests, nestest.nes
2. **Testes de instrução**: Verificação de cada opcode, incluindo ilegais
3. **Testes de timing**: Verificação precisa de ciclos para operações
4. **Testes de interrupção**: Verificação do comportamento das interrupções
5. **Testes de integração**: Verificação do comportamento em jogos reais

## Compatibilidade

Nossa implementação da CPU do NES é capaz de executar corretamente todos os jogos comerciais lançados para o NES, incluindo aqueles que dependem de timing preciso ou utilizam opcodes não documentados.

## Otimizações

Implementamos várias otimizações para melhorar o desempenho sem sacrificar a precisão:

1. **Tabelas de dispatch**: Para rápida decodificação de instruções
2. **Caching de operandos**: Para reduzir acessos à memória
3. **Implementação eficiente de modos de endereçamento**: Para minimizar código redundante
4. **Tratamento otimizado de interrupções**: Para simulação eficiente de interrupções

## Referências

1. MOS Technology 6502 Programmer's Manual
2. "Programming the 65816 Including the 6502, 65C02, and 65802" por David Eyes e Ron Lichty
3. NesDev Wiki: https://wiki.nesdev.org/w/index.php/CPU
4. Blargg's 6502 test roms
5. Visual 6502 Project: http://visual6502.org/

## Histórico de Atualizações

- **Maio 2024**: Implementação 100% completa
  - Todos os opcodes ilegais implementados e testados
  - Precisão de timing melhorada para operações de página cruzada
  - Documentação completa criada

- **Abril 2024**: Aprimoramentos de compatibilidade
  - Correção de bugs em comportamentos específicos de opcodes ilegais
  - Melhorias na precisão de timing
  - Expansão da cobertura de testes

- **Março 2024**: Implementação inicial
  - Suporte completo a todas as instruções oficiais
  - Implementação básica de opcodes não documentados
  - Emulação correta de interrupções
