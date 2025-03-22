# Implementação da CPU RP2A03 (6502)

## Visão Geral Técnica

Este arquivo contém informações técnicas para os desenvolvedores sobre a implementação da CPU do NES (RP2A03) no emulador.

A documentação completa da CPU para usuários e referência geral está disponível em `docs/architecture/nes/CPU.md`.

## Estruturas de Dados

A CPU é implementada usando a estrutura `nes_cpu_t` que contém:

- Registradores (A, X, Y, S, PC, P)
- Contadores de ciclos
- Estado interno
- Ponteiros para componentes relacionados (PPU, APU, cartridge)

## Funções Principais

- `nes_cpu_init()`: Inicializa o estado da CPU
- `nes_cpu_reset()`: Reinicia a CPU para o estado inicial
- `nes_cpu_step()`: Executa um único passo da CPU (1 instrução)
- `nes_cpu_execute()`: Executa a CPU por um número específico de ciclos
- `cpu_read()` / `cpu_write()`: Funções de leitura/escrita para a CPU acessar o barramento

## Tabelas de Dispatcher

A implementação utiliza tabelas para despachar instruções:

- `cpu_opcodes`: Ponteiros para funções de instruções
- `cpu_addressing_modes`: Ponteiros para funções de modos de endereçamento
- `cpu_opcode_cycles`: Ciclos base para cada opcode
- `cpu_illegals`: Tabela de opcodes ilegais

## Opcodes e Addressing Modes

Cada instrução é implementada como função com o seguinte padrão:

```c
static uint8_t op_XXX(nes_cpu_t *cpu, uint16_t addr) {
    // Implementação da instrução
    return ciclos_extras;
}
```

Cada modo de endereçamento é implementado como função que retorna o endereço efetivo:

```c
static uint16_t addr_XXX(nes_cpu_t *cpu) {
    // Implementação do modo de endereçamento
    return endereco_efetivo;
}
```

## Interrupções

As interrupções são implementadas em:

- `nes_cpu_nmi()`: Non-Maskable Interrupt (ativada pela PPU)
- `nes_cpu_irq()`: Interrupt Request (ativada por mappers/APU)
- `nes_cpu_trigger_irq()`: Função interna para processamento de IRQ

## Página Zero

A página zero ($0000-$00FF) tem acesso especial e otimizado, implementado através de:

- Modos de endereçamento otimizados (Zero Page, Zero Page X, Zero Page Y)
- Acesso direto para operações rápidas

## Ciclos

A precisão de ciclos é crítica para emulação correta:

- Ciclos base definidos por instrução
- Ciclos extras para operações de página cruzada
- Sincronização com outros componentes (PPU/APU)

## Timing e Sincronização

- A CPU opera com clock de ~1.79MHz (NTSC) ou ~1.66MHz (PAL)
- Cada ciclo de CPU equivale a 3 ciclos de PPU
- A sincronização precisa é mantida através do contador de ciclos global

## Testes e Debug

- `nes_cpu_get_state()`: Retorna o estado atual da CPU para debug
- `nes_cpu_set_state()`: Define o estado da CPU para testes
- Logs detalhados podem ser ativados com `NES_CPU_DEBUG`

## Notas de Implementação

1. O modo decimal (BCD) é ignorado no RP2A03, mesmo quando o flag D está definido
2. Alguns jogos dependem de comportamentos específicos de opcodes ilegais
3. O timing preciso é essencial para compatibilidade com jogos que usam timing raster
4. A página zero é acessada com frequência - otimizações de acesso melhoram o desempenho

## Recursos Internos

- Tabela interna para instruções ilegais
- Cache para instruções frequentes
- Otimizações de boundary checking

## Trabalhos Futuros

Ver `docs/ROADMAP.md` para tarefas pendentes relacionadas à CPU.

## Referências para Desenvolvedores

- Código fonte em `src/platforms/nes/cpu/`
- Testes em `tests/platforms/nes/cpu/`
- Documentação completa em `docs/architecture/nes/CPU.md`
