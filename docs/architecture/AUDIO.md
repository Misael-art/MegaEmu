# Sistema de Áudio no Mega_Emu

## Visão Geral

O sistema de áudio do Mega_Emu foi projetado para ser modular e extensível, permitindo a emulação precisa de diversos chips de áudio encontrados nos consoles retro. A arquitetura modular permite a fácil adição de novos chips e a reutilização de componentes comuns.

## Chips de Áudio Suportados

O Mega_Emu implementa a emulação dos seguintes chips de áudio:

1. **YM2612** - Sintetizador FM utilizado no Mega Drive.
2. **SN76489** - Gerador de som PSG utilizado no Mega Drive, Master System e Game Gear.
3. **2A03/2A07 APU** - Processador de áudio do NES/Famicom.
4. **SPC700** - Processador de áudio do SNES.
5. **Game Boy APU** - Processador de áudio do Game Boy/Game Boy Color.
6. **POKEY** - Chip de áudio do Atari 7800 e Atari Lynx.
7. **SPU** - Sound Processing Unit do PlayStation.
8. **SCSP** - Saturn Custom Sound Processor.
9. **AICA** - Chip de áudio do Dreamcast.

## Arquitetura do Sistema de Áudio

A arquitetura do sistema de áudio segue a estrutura modular descrita em ARCHITECTURE.md:

```
                          ┌───────────────┐
                          │ CPU Emulado   │
                          │ (Z80, M68000, │
                          │ 6502, etc.)   │
                          └───────┬───────┘
                                  │
                                  ▼
                          ┌───────────────┐
                          │    Data Bus   │
                          └───────┬───────┘
                                  │
                                  ▼
┌───────────────┐        ┌───────────────┐       ┌───────────────┐
│   Chip de     │        │   Sistema de  │       │   Sistema de  │
│   Áudio 1     │◄─────► │   Áudio       │◄─────►│   Mixagem     │
│   (YM2612)    │        │               │       │               │
└───────────────┘        └───────┬───────┘       └───────┬───────┘
                                  │                       │
┌───────────────┐                 │                       │
│   Chip de     │                 │                       │
│   Áudio 2     │◄────────────────┘                       │
│   (SN76489)   │                                         │
└───────────────┘                                         │
                                                          │
┌───────────────┐                                         │
│   Chip de     │                                         │
│   Áudio 3     │◄────────────────────────────────────────┘
│   (APU, etc)  │                                         │
└───────────────┘                                         │
                                                          ▼
                                               ┌───────────────────┐
                                               │ Gerenciamento de  │
                                               │ Buffer de Áudio   │
                                               └─────────┬─────────┘
                                                         │
                                                         ▼
                                               ┌───────────────────┐
                                               │ Interface de      │
                                               │ Áudio (SDL)       │
                                               └───────────────────┘
```

## Estrutura de Diretórios

```
src/
└── core/
    └── audio/
        ├── audio.h                # Interface principal do sistema de áudio
        ├── audio.c                # Implementação principal
        ├── mixing.h               # Sistema de mixagem
        ├── mixing.c               # Implementação do sistema de mixagem
        ├── buffer.h               # Gerenciamento de buffer de áudio
        ├── buffer.c               # Implementação do gerenciamento de buffer
        └── chips/                 # Implementações de chips de áudio
            ├── ym2612.h           # Interface do YM2612
            ├── ym2612.c           # Implementação do YM2612
            ├── sn76489.h          # Interface do SN76489
            ├── sn76489.c          # Implementação do SN76489
            ├── nes_apu.h          # Interface do APU do NES
            ├── nes_apu.c          # Implementação do APU do NES
            ├── spc700.h           # Interface do SPC700 do SNES
            ├── spc700.c           # Implementação do SPC700 do SNES
            ├── gb_apu.h           # Interface do APU do Game Boy
            ├── gb_apu.c           # Implementação do APU do Game Boy
            ├── pokey.h            # Interface do POKEY
            ├── pokey.c            # Implementação do POKEY
            ├── psx_spu.h          # Interface do SPU do PlayStation
            ├── psx_spu.c          # Implementação do SPU do PlayStation
            ├── scsp.h             # Interface do SCSP do Saturn
            ├── scsp.c             # Implementação do SCSP do Saturn
            └── aica.h             # Interface do AICA do Dreamcast
                └── aica.c         # Implementação do AICA do Dreamcast
```

## Chips Implementados

### YM2612 (Yamaha FM Synthesizer)

O YM2612 é o principal chip de síntese FM do Mega Drive.

**Características:**
- 6 canais de síntese FM
- 4 operadores por canal
- Envelope ADSR completo
- LFO para modulação
- Canal especial de PCM (DAC)

**Estrutura de dados:**
```c
typedef struct {
    // Estado dos canais FM
    ym2612_channel_t channels[6];

    // Estado do LFO global
    uint8_t lfo_enable;
    uint8_t lfo_frequency;

    // Registradores globais
    uint8_t registers[0x200];

    // Estado do timer
    uint8_t timer_a_val[2];
    uint8_t timer_b_val;
    uint8_t timer_control;

    // Saída do canal DAC
    uint16_t dac_output;
    uint8_t dac_enable;

    // Callbacks e parâmetros de timing
    int32_t sample_rate;
    void* user_data;
    void (*write_callback)(void* user_data, int32_t left, int32_t right);
} ym2612_t;
```

### SN76489 (PSG - Programmable Sound Generator)

O SN76489 é um chip PSG usado no Mega Drive, Master System e Game Gear.

**Características:**
- 3 canais de onda quadrada
- 1 canal de ruído
- Controle de volume por canal
- Gerador de ruído com feedback configurável

**Estrutura de dados:**
```c
typedef struct {
    // Estado dos canais
    uint16_t tone_registers[4];     // Registradores de frequência
    uint8_t volume_registers[4];    // Registradores de volume (atenuação)

    // Estado do gerador de ruído
    uint8_t noise_control;          // Registro de controle de ruído
    uint16_t noise_shift;           // Registrador de deslocamento para geração de ruído

    // Estado de saída dos canais
    uint8_t channel_output[4];      // Saída atual de cada canal

    // Parâmetros de timing
    uint32_t clock_rate;            // Taxa de clock do chip
    uint32_t sample_rate;           // Taxa de amostragem da saída
} sn76489_t;
```

### NES APU (2A03/2A07)

O APU do NES consiste em múltiplos geradores de som integrados ao chip principal.

**Características:**
- 2 canais de onda quadrada com duty cycle variável
- 1 canal de onda triangular
- 1 canal de ruído
- 1 canal de DPCM (Delta Pulse Code Modulation)
- Controle de envelope e sweep

**Estrutura de dados:**
```c
typedef struct {
    // Canais de pulso (onda quadrada)
    nes_pulse_t pulse[2];

    // Canal triangular
    nes_triangle_t triangle;

    // Canal de ruído
    nes_noise_t noise;

    // Canal DMC (DPCM)
    nes_dmc_t dmc;

    // Status e quadro (frame) sequencer
    uint8_t status;
    uint8_t frame_counter;
    bool frame_interrupt;
    bool dmc_interrupt;

    // Callbacks de memória e timing
    uint32_t cycle_count;
    uint8_t (*read_memory)(void* context, uint16_t address);
    void* context;
} nes_apu_t;
```

### SPC700 (SNES Audio Processor)

O SPC700 é um processador dedicado para áudio no SNES.

**Características:**
- CPU dedicada para processamento de áudio
- 8 canais de samples
- 64KB de RAM dedicada
- DSP integrado para efeitos

**Estrutura de dados:**
```c
typedef struct {
    // Registradores da CPU SPC700
    uint16_t pc;
    uint8_t a, x, y, sp;
    uint8_t psw;

    // RAM e ROM dedicada
    uint8_t ram[0x10000];
    uint8_t ipl_rom[0x40];

    // Estado do DSP
    spc_dsp_t dsp;

    // Estado de I/O e timers
    uint8_t port_in[4];
    uint8_t port_out[4];
    spc_timer_t timer[3];

    // Controle e timing
    uint64_t cycle_count;
} spc700_t;
```

### Game Boy APU

O APU do Game Boy consiste em 4 canais de som.

**Características:**
- 2 canais de onda quadrada com envelope e sweep
- 1 canal de forma de onda programável
- 1 canal de ruído

**Estrutura de dados:**
```c
typedef struct {
    // Canais
    gb_square_t square1;    // Canal 1 (onda quadrada com sweep)
    gb_square_t square2;    // Canal 2 (onda quadrada)
    gb_wave_t wave;         // Canal 3 (onda programável)
    gb_noise_t noise;       // Canal 4 (ruído)

    // Controle
    uint8_t power;
    uint8_t channel_control;
    uint8_t volume[2];      // Volume esquerdo/direito
    uint8_t status;

    // Timing
    uint32_t frame_sequencer_count;
    uint8_t frame_sequencer_step;
} gb_apu_t;
```

## Sistema de Mixagem

O sistema de mixagem é responsável por combinar as saídas de diferentes chips de áudio e entregar um stream de áudio consistente para o frontend.

### Características do Sistema de Mixagem

1. **Mixagem Multi-Chip**: Combina a saída de múltiplos chips de áudio com diferentes características e taxas de amostragem.

2. **Balanceamento Estéreo**: Controle de panorama e volume para cada chip e canal.

3. **Processamento de Efeitos em Tempo Real**: Suporte opcional para efeitos como reverb, chorus, e equalização.

4. **Resampling**: Conversão entre diferentes taxas de amostragem para garantir uma saída consistente.

### Implementação

```c
typedef struct {
    // Configuração
    uint32_t sample_rate;       // Taxa de amostragem de saída
    uint32_t buffer_size;       // Tamanho do buffer em amostras

    // Canais de entrada
    audio_channel_t* channels;  // Lista de canais de entrada
    uint32_t num_channels;      // Número de canais

    // Estado do processamento
    float* mix_buffer_left;     // Buffer temporário para mixagem (esquerdo)
    float* mix_buffer_right;    // Buffer temporário para mixagem (direito)

    // Callbacks
    void (*output_callback)(void* context, int16_t* buffer, uint32_t num_samples);
    void* user_data;
} audio_mixer_t;
```

### Fluxo de Mixagem

1. Cada chip de áudio gera amostras em sua própria taxa
2. As amostras são bufferizadas e, se necessário, passam por resampling
3. As saídas de todos os chips são mixadas em um buffer estéreo
4. Efeitos globais são aplicados (se configurados)
5. O buffer mixado é convertido para o formato final (geralmente int16)
6. Os dados são enviados para o driver de áudio

## Gerenciamento de Buffer de Áudio

O sistema utiliza buffers de áudio para lidar com as diferenças de timing entre a geração de áudio e a reprodução.

### Características

1. **Buffer Duplo**: Um buffer para gravação, outro para reprodução, com troca quando o buffer de reprodução é esvaziado.

2. **Buffer Circular**: Implementado como um buffer circular para fluxo contínuo sem interrupções.

3. **Controle de Latência Adaptativo**: Ajuste dinâmico do tamanho do buffer para balancear entre latência e estabilidade.

### Implementação

```c
typedef struct {
    // Buffer circular
    int16_t* buffer;
    uint32_t buffer_size;
    uint32_t read_pos;
    uint32_t write_pos;

    // Estado
    uint32_t fill_level;
    uint32_t underruns;
    uint32_t overruns;

    // Sincronização
    SDL_mutex* mutex;
} audio_buffer_t;
```

## Integração com o Fluxo Principal de Execução

O sistema de áudio se integra ao fluxo principal de execução de diversas maneiras:

### 1. Sincronização por Ciclos

Cada chip de áudio é atualizado a cada N ciclos de CPU:

```c
// Exemplo: Atualização do áudio durante emulação
void platform_execute_frame(platform_context_t* context) {
    int cycles_executed = 0;

    while (cycles_executed < CYCLES_PER_FRAME) {
        // Executar CPU principal
        int cpu_cycles = cpu_execute(context->cpu, CPU_CHUNK_SIZE);

        // Atualizar subsistemas
        video_update(context->video, cpu_cycles);

        // Atualizar áudio - converte ciclos de CPU em amostras
        audio_update(context->audio, cpu_cycles);

        // Atualizar outros subsistemas
        cycles_executed += cpu_cycles;
    }
}
```

### 2. Sistema de Eventos

Para interações complexas como interrupções e comunicação entre chips:

```c
// Exemplo: Temporizador gerando interrupção de áudio
void audio_timer_callback(platform_context_t* context) {
    // Notificar chip de áudio sobre evento de timer
    ym2612_timer_event(context->audio->ym2612);

    // Verificar se interrupção deve ser gerada
    if (ym2612_check_interrupt(context->audio->ym2612)) {
        // Gerar interrupção na CPU
        cpu_set_irq(context->cpu, AUDIO_IRQ_LEVEL);
    }
}
```

### 3. Callbacks de Áudio

Para entregar o áudio processado ao sistema de saída:

```c
// Callback chamado pelo driver de áudio (ex: SDL)
void audio_callback(void* userdata, uint8_t* stream, int len) {
    platform_context_t* context = (platform_context_t*)userdata;
    int16_t* output = (int16_t*)stream;
    int samples = len / (2 * sizeof(int16_t)); // Saída estéreo de 16 bits

    // Obter amostras mixadas do buffer
    audio_buffer_get_samples(context->audio->buffer, output, samples);
}
```

## Interface com o Frontend

A interface de áudio com o frontend (SDL na maioria dos casos) é gerenciada através de uma camada de abstração:

```c
// Inicialização do áudio
bool audio_init(audio_context_t* audio, uint32_t sample_rate, uint32_t buffer_size) {
    // Inicializar SDL Audio
    SDL_AudioSpec want, have;

    // Configurar especificação de áudio
    SDL_memset(&want, 0, sizeof(want));
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = buffer_size;
    want.callback = audio_callback;
    want.userdata = audio->platform_context;

    // Abrir dispositivo de áudio
    audio->device_id = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio->device_id == 0) {
        return false;
    }

    // Inicializar sistema de mixagem com parâmetros reais
    audio_mixer_init(audio->mixer, have.freq, have.samples);

    // Iniciar reprodução
    SDL_PauseAudioDevice(audio->device_id, 0);

    return true;
}
```

## Especificidades por Plataforma

### Mega Drive

No Mega Drive, a emulação de áudio envolve:

1. YM2612 para síntese FM
2. SN76489 para ondas quadradas e ruído
3. Comunicação com o Z80 que atua como coprocessador de áudio
4. Temporizadores do YM2612 que podem gerar interrupções

### Master System

No Master System:

1. SN76489 como único chip de som
2. Sem coprocessador de áudio

### NES

No NES:

1. APU integrado ao chip 2A03/2A07
2. Dificuldades de timing devido à sincronização com CPU e PPU
3. Canal DMC com acesso direto à memória

### SNES

No SNES:

1. Processador SPC700 dedicado com memória independente
2. DSP integrado para processamento de samples
3. Comunicação com CPU principal via portas dedicadas

## Considerações Especiais

### Precisão vs. Desempenho

O sistema de áudio oferece diferentes modos de emulação:

1. **Modo de Alta Precisão**: Emulação ciclo-a-ciclo de todos os detalhes dos chips, com maior consumo de CPU.
2. **Modo de Desempenho**: Emulação simplificada com boa qualidade de áudio mas menor precisão, para hardware mais limitado.

### Emulação de Hardware Analógico

Para emular características analógicas dos chips de áudio originais:

1. **Filtros**: Emulação de filtros RC encontrados em hardware real
2. **Ruído**: Emulação de ruído e distorção característicos de chips analógicos
3. **Artefatos**: Emulação de artefatos como distorção inerente ao hardware original

## Estado de Implementação

### Completo

- Estrutura base do sistema de áudio
- Integração com SDL
- Implementação do YM2612 (Mega Drive)
- Implementação do SN76489 (Mega Drive, Master System)
- Sistema básico de mixagem
- Buffer de áudio com proteção contra sub/overruns

### Em Progresso

- Implementação do NES APU
- Implementação do SPC700 (SNES)
- Melhorias no sistema de mixagem (efeitos, equalização)
- Otimizações de performance

### Pendente

- Implementação do APU do Game Boy
- Implementação do POKEY (Atari)
- Implementação do SPU (PlayStation)
- Implementação do SCSP (Saturn)
- Implementação do AICA (Dreamcast)
- Sistema avançado de filtros para emulação analógica precisa

## Referências

1. [YM2612 Application Manual](https://plutiedev.com/ym2612-registers)
2. [SN76489 Data Sheet](https://segaretro.org/images/4/47/SN76489AN_datasheet.pdf)
3. [NES APU Sound Hardware Reference](https://www.nesdev.org/wiki/APU)
4. [SNES SPC700 Reference](https://sneslab.net/wiki/SPC700_reference)
5. [Game Boy Sound Hardware](https://gbdev.io/pandocs/Sound_Controller.html)
6. [PlayStation SPU Documentation](https://psx-spx.consoledev.net/soundprocessingunitspu/)
