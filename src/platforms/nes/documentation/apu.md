# Implementação da APU (Audio Processing Unit) do NES

## Visão Geral Técnica

Este arquivo contém informações técnicas para os desenvolvedores sobre a implementação da APU do NES no emulador.

A documentação completa da APU para usuários e referência geral está disponível em `docs/architecture/nes/APU.md`.

## Estruturas de Dados

A APU é implementada usando a estrutura `nes_apu_t` que contém:

- Estados para cada canal (pulse1, pulse2, triangle, noise, dmc)
- Registradores de controle
- Frame counter e estado de timing
- Buffers de áudio e estado do mixer
- Ponteiros para componentes relacionados (CPU, cartridge)

## Funções Principais

- `nes_apu_init()`: Inicializa o estado da APU
- `nes_apu_reset()`: Reinicia a APU para o estado inicial
- `nes_apu_step()`: Executa um único passo da APU (1 ciclo)
- `nes_apu_execute()`: Executa a APU por um número específico de ciclos
- `nes_apu_read_register()` / `nes_apu_write_register()`: Acesso aos registradores ($4000-$4017)
- `nes_apu_get_sample()`: Obtém amostra de áudio para a saída

## Registradores

A APU possui diversos registradores acessíveis pela CPU:

- **$4000-$4003**: Controle do canal Pulse 1
- **$4004-$4007**: Controle do canal Pulse 2
- **$4008-$400B**: Controle do canal Triangle
- **$400C-$400F**: Controle do canal Noise
- **$4010-$4013**: Controle do canal DMC
- **$4015**: Status/controle dos canais
- **$4017**: Controle de frame counter

## Canais de Áudio

### Pulse (Canais 1 e 2)

Estrutura do canal:
```c
typedef struct {
    // Registradores
    uint8_t duty;           // Duty cycle (0-3)
    uint8_t volume;         // Volume fixo ou envelope
    bool length_counter_halt; // Halt do contador de comprimento
    bool constant_volume;   // Volume constante ou envelope

    // Estado do envelope
    uint8_t envelope_period; // Período do envelope
    uint8_t envelope_value;  // Valor atual do envelope
    uint8_t envelope_counter; // Contador do envelope
    bool envelope_start;    // Envelope iniciando

    // Estado do sweep
    bool sweep_enabled;     // Sweep habilitado
    uint8_t sweep_period;   // Período do sweep
    bool sweep_negate;      // Direção do sweep (0=incremento, 1=decremento)
    uint8_t sweep_shift;    // Shift do sweep
    uint8_t sweep_counter;  // Contador do sweep
    bool sweep_reload;      // Flag de recarga do sweep
    uint16_t sweep_target;  // Alvo atual do sweep

    // Estado do timer
    uint16_t timer_period;  // Período do timer
    uint16_t timer_value;   // Valor atual do timer

    // Estado do duty cycle
    uint8_t duty_value;     // Valor atual do duty cycle
    uint8_t duty_index;     // Índice atual no sequenciador de duty

    // Estado do comprimento
    uint8_t length_counter; // Contador de comprimento

    // Estado geral
    bool enabled;           // Canal habilitado
    uint8_t output;         // Valor de saída atual
} nes_pulse_channel_t;
```

Funções principais:
- `update_pulse_channel()`: Atualiza estado do canal
- `clock_pulse_envelope()`: Atualiza envelope
- `clock_pulse_sweep()`: Atualiza sweep
- `clock_pulse_timer()`: Atualiza timer
- `clock_pulse_length_counter()`: Atualiza contador de comprimento

### Triangle

Estrutura do canal:
```c
typedef struct {
    // Registradores
    bool length_counter_halt; // Halt do contador de comprimento (flag de controle linear)
    uint8_t linear_counter_period; // Período do contador linear

    // Estado do contador linear
    uint8_t linear_counter;  // Valor atual do contador linear
    bool linear_counter_reload; // Flag de recarga do contador linear

    // Estado do timer
    uint16_t timer_period;  // Período do timer
    uint16_t timer_value;   // Valor atual do timer

    // Estado do sequenciador
    uint8_t sequence_index; // Índice atual na sequência de 32 passos

    // Estado do comprimento
    uint8_t length_counter; // Contador de comprimento

    // Estado geral
    bool enabled;           // Canal habilitado
    uint8_t output;         // Valor de saída atual
} nes_triangle_channel_t;
```

Funções principais:
- `update_triangle_channel()`: Atualiza estado do canal
- `clock_triangle_linear_counter()`: Atualiza contador linear
- `clock_triangle_timer()`: Atualiza timer
- `clock_triangle_length_counter()`: Atualiza contador de comprimento

### Noise

Estrutura do canal:
```c
typedef struct {
    // Registradores
    uint8_t volume;         // Volume fixo ou envelope
    bool length_counter_halt; // Halt do contador de comprimento
    bool constant_volume;   // Volume constante ou envelope
    bool mode;              // Modo do shift register (0=15-bit, 1=7-bit)
    uint8_t noise_period;   // Índice na tabela de períodos

    // Estado do envelope
    uint8_t envelope_period; // Período do envelope
    uint8_t envelope_value;  // Valor atual do envelope
    uint8_t envelope_counter; // Contador do envelope
    bool envelope_start;    // Envelope iniciando

    // Estado do timer
    uint16_t timer_period;  // Período do timer
    uint16_t timer_value;   // Valor atual do timer

    // Estado do shift register
    uint16_t shift_register; // Shift register (15 bits)

    // Estado do comprimento
    uint8_t length_counter; // Contador de comprimento

    // Estado geral
    bool enabled;           // Canal habilitado
    uint8_t output;         // Valor de saída atual
} nes_noise_channel_t;
```

Funções principais:
- `update_noise_channel()`: Atualiza estado do canal
- `clock_noise_envelope()`: Atualiza envelope
- `clock_noise_timer()`: Atualiza timer
- `clock_noise_length_counter()`: Atualiza contador de comprimento

### DMC (Delta Modulation Channel)

Estrutura do canal:
```c
typedef struct {
    // Registradores
    bool irq_enable;        // IRQ habilitado
    bool loop;              // Loop da amostra
    uint8_t frequency;      // Índice na tabela de frequências
    uint8_t direct_load;    // Valor de load direto
    uint16_t sample_address; // Endereço da amostra
    uint16_t sample_length; // Comprimento da amostra

    // Estado do timer
    uint16_t timer_period;  // Período do timer
    uint16_t timer_value;   // Valor atual do timer

    // Estado da saída
    uint8_t output;         // Valor de saída atual

    // Estado da amostra
    uint16_t current_address; // Endereço atual na amostra
    uint16_t bytes_remaining; // Bytes restantes
    uint8_t sample_buffer;   // Buffer da amostra atual
    bool sample_buffer_empty; // Buffer vazio
    uint8_t shift_register;  // Shift register
    uint8_t bits_remaining;  // Bits restantes no shift register
    bool silence;           // Flag de silêncio

    // Estado geral
    bool enabled;           // Canal habilitado
    bool irq_flag;          // Flag de IRQ
} nes_dmc_channel_t;
```

Funções principais:
- `update_dmc_channel()`: Atualiza estado do canal
- `clock_dmc_timer()`: Atualiza timer
- `dmc_fetch_sample()`: Busca nova amostra
- `dmc_start_sample()`: Inicia reprodução de amostra

## Frame Counter

O frame counter controla o timing de vários processos:

```c
typedef struct {
    uint8_t mode;           // Modo (0=4-step, 1=5-step)
    uint16_t counter;       // Contador atual
    bool irq_inhibit;       // IRQ inibido
    bool irq_flag;          // Flag de IRQ
    bool frame_interrupt;   // Interrupção de frame pendente
} nes_frame_counter_t;
```

Funções principais:
- `update_frame_counter()`: Atualiza frame counter
- `clock_quarter_frame()`: Executa eventos a cada 1/4 de frame
- `clock_half_frame()`: Executa eventos a cada 1/2 de frame

## Mixer de Áudio

O mixer combina a saída dos cinco canais:

```c
typedef struct {
    float pulse1;           // Saída do canal pulse 1
    float pulse2;           // Saída do canal pulse 2
    float triangle;         // Saída do canal triangle
    float noise;            // Saída do canal noise
    float dmc;              // Saída do canal DMC

    float output;           // Saída combinada final

    // Tabelas de lookup para mistura não-linear
    float pulse_table[31];
    float tnd_table[203];
} nes_mixer_t;
```

Funções principais:
- `init_mixer()`: Inicializa tabelas de lookup
- `mix_audio()`: Mistura os canais para gerar saída final

## Timing e Sincronização

- A APU opera com um clock derivado do clock da CPU
- O frame counter opera em modo 4-step (60Hz) ou 5-step (48Hz)
- Eventos de atualização são sincronizados com o frame counter:
  - Quarter frame: envelope e contador linear (240Hz ou 192Hz)
  - Half frame: sweep e contador de comprimento (120Hz ou 96Hz)

## Notas de Implementação

1. O modo DMC precisa de acesso à memória durante a execução, necessitando coordenação com a CPU
2. O sweep negativo tem comportamento diferente nos dois canais pulse
3. A sequência do triangle consiste em 32 passos, não 16
4. O frame counter precisa de timing preciso para emular IRQs corretamente
5. A mistura de áudio deve seguir fórmula não-linear para emular hardware original

## Debug e Desenvolvimento

- Logs detalhados podem ser ativados com `NES_APU_DEBUG`
- Visualizadores de canais podem ser implementados para debug
- Estados de todos os canais estão disponíveis para inspeção

## Limitações Conhecidas

- Implementação do canal DMC pode ser melhorada para maior precisão
- Alguns efeitos de sweep podem precisar de refinamento
- Ver `docs/ROADMAP.md` para tarefas pendentes relacionadas à APU

## Referências para Desenvolvedores

- Código fonte em `src/platforms/nes/apu/`
- Testes em `tests/platforms/nes/apu/`
- Documentação completa em `docs/architecture/nes/APU.md`
