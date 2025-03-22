# Sistema de Memória do Mega_Emu

Este documento descreve o sistema de memória do emulador Mega_Emu, incluindo sua arquitetura, implementação e considerações de design.

## Visão Geral

O sistema de memória do Mega_Emu foi projetado para ser flexível, eficiente e fácil de depurar. Ele fornece uma camada de abstração sobre a memória física emulada dos diferentes sistemas, permitindo mapear regiões de memória, implementar espelhamento, e gerenciar bancos de memória de forma consistente.

## Arquitetura

### Componentes Principais

1. **Gerenciador de Memória**: Coordena todas as operações de memória e mantém o mapa de memória.
2. **Regiões de Memória**: Blocos contíguos de memória com propriedades específicas.
3. **Callbacks de Acesso**: Funções chamadas quando ocorrem leituras ou escritas em regiões específicas.
4. **Bancos de Memória**: Mecanismo para alternar entre diferentes mapeamentos de memória.
5. **Cache de Acesso**: Otimiza operações repetidas na mesma região de memória.

### Diagrama de Componentes

```
┌─────────────────────────────────────────────────────┐
│                  Processador Emulado                │
└───────────────────────┬─────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────┐
│                 Gerenciador de Memória              │
│                                                     │
│  ┌─────────────┐    ┌─────────────┐    ┌──────────┐ │
│  │  Região 1   │    │  Região 2   │    │ Região N │ │
│  │ (ex: ROM)   │    │ (ex: RAM)   │    │  (etc.)  │ │
│  └─────┬───────┘    └──────┬──────┘    └────┬─────┘ │
│        │                   │                 │      │
│  ┌─────┴───────┐    ┌──────┴──────┐    ┌────┴─────┐ │
│  │  Callbacks  │    │  Callbacks  │    │ Callbacks│ │
│  │  de Acesso  │    │  de Acesso  │    │ de Acesso│ │
│  └─────────────┘    └─────────────┘    └──────────┘ │
└─────────────────────────────────────────────────────┘
```

## Implementação

### Estruturas de Dados

```c
// Definição de uma região de memória
typedef struct {
    uint32_t start_address;    // Endereço inicial
    uint32_t end_address;      // Endereço final
    uint8_t *data;             // Ponteiro para os dados (NULL para regiões mapeadas por callbacks)
    uint8_t access_flags;      // Flags de acesso (READ, WRITE, EXECUTE)
    memory_read_callback_t read_callback;    // Callback para leitura
    memory_write_callback_t write_callback;  // Callback para escrita
    void *context;             // Contexto para callbacks
} memory_region_t;

// Sistema de memória completo
typedef struct {
    memory_region_t *regions;  // Array de regiões
    uint32_t region_count;     // Número de regiões
    uint32_t max_regions;      // Capacidade máxima de regiões
    memory_read_callback_t default_read;     // Callback padrão para leitura
    memory_write_callback_t default_write;   // Callback padrão para escrita
    void *default_context;     // Contexto padrão para callbacks
    memory_region_t *last_read_region;       // Cache para otimização
    memory_region_t *last_write_region;      // Cache para otimização
} memory_system_t;
```

### Funções Principais

```c
// Cria um novo sistema de memória
memory_system_t* memory_create(uint32_t max_regions);

// Destrói um sistema de memória
void memory_destroy(memory_system_t *memory);

// Adiciona uma região de memória
int memory_add_region(memory_system_t *memory, uint32_t start, uint32_t end,
                      uint8_t *data, uint8_t access_flags);

// Adiciona uma região mapeada por callbacks
int memory_add_mapped_region(memory_system_t *memory, uint32_t start, uint32_t end,
                             memory_read_callback_t read_cb,
                             memory_write_callback_t write_cb,
                             void *context, uint8_t access_flags);

// Define callbacks padrão para endereços não mapeados
void memory_set_default_callbacks(memory_system_t *memory,
                                  memory_read_callback_t read_cb,
                                  memory_write_callback_t write_cb,
                                  void *context);

// Funções de acesso à memória
uint8_t memory_read_8(memory_system_t *memory, uint32_t address);
uint16_t memory_read_16(memory_system_t *memory, uint32_t address);
uint32_t memory_read_32(memory_system_t *memory, uint32_t address);
void memory_write_8(memory_system_t *memory, uint32_t address, uint8_t value);
void memory_write_16(memory_system_t *memory, uint32_t address, uint16_t value);
void memory_write_32(memory_system_t *memory, uint32_t address, uint32_t value);
```

## Características Avançadas

### Sistema de Banking

O sistema de memória suporta banking (troca de bancos de memória), uma técnica comum em consoles antigos para superar limitações de espaço de endereçamento.

```c
// Define um banco de memória
typedef struct {
    uint32_t bank_address;     // Endereço base do banco
    uint32_t bank_size;        // Tamanho do banco
    uint8_t **bank_data;       // Array de ponteiros para dados dos bancos
    uint32_t bank_count;       // Número de bancos
    uint32_t current_bank;     // Banco atualmente selecionado
} memory_bank_t;

// Funções para gerenciamento de bancos
memory_bank_t* memory_bank_create(uint32_t bank_address, uint32_t bank_size, uint32_t bank_count);
void memory_bank_destroy(memory_bank_t *bank);
void memory_bank_set_data(memory_bank_t *bank, uint32_t bank_index, uint8_t *data);
void memory_bank_select(memory_bank_t *bank, uint32_t bank_index);
```

### Espelhamento de Memória

O espelhamento de memória é implementado através de callbacks especiais:

```c
// Callback para espelhamento de memória
uint8_t memory_mirror_read_callback(void *context, uint32_t address) {
    mirror_context_t *mirror = (mirror_context_t*)context;
    uint32_t mirrored_address = (address & mirror->mask) + mirror->offset;
    return memory_read_8(mirror->memory, mirrored_address);
}

void memory_mirror_write_callback(void *context, uint32_t address, uint8_t value) {
    mirror_context_t *mirror = (mirror_context_t*)context;
    uint32_t mirrored_address = (address & mirror->mask) + mirror->offset;
    memory_write_8(mirror->memory, mirrored_address, value);
}
```

### Proteção de Memória

Pode-se configurar regiões de memória com diferentes níveis de proteção:

- **READ_ONLY**: Apenas leitura, tentativas de escrita são ignoradas ou geram avisos
- **WRITE_ONLY**: Apenas escrita, leituras retornam um valor padrão
- **READ_WRITE**: Leitura e escrita permitidas
- **NO_ACCESS**: Nem leitura nem escrita são permitidas

### Cache de Otimização

Para melhorar o desempenho, o sistema mantém um cache das últimas regiões acessadas:

```c
// Exemplo de otimização para leitura
uint8_t memory_read_8(memory_system_t *memory, uint32_t address) {
    // Verificar se o endereço está na região cacheada anterior
    if (memory->last_read_region &&
        address >= memory->last_read_region->start_address &&
        address <= memory->last_read_region->end_address) {
        memory_region_t *region = memory->last_read_region;
        if (region->data) {
            return region->data[address - region->start_address];
        } else if (region->read_callback) {
            return region->read_callback(region->context, address);
        }
    }

    // Se não, buscar a região correta
    memory_region_t *region = find_region(memory, address);
    memory->last_read_region = region;

    // Continua com a leitura...
}
```

## Considerações de Design

### Endianness

O sistema de memória lida com sistemas de diferentes endianness:

```c
// Configuração de endianness
typedef enum {
    MEMORY_LITTLE_ENDIAN,
    MEMORY_BIG_ENDIAN
} memory_endianness_t;

// Definir endianness do sistema
void memory_set_endianness(memory_system_t *memory, memory_endianness_t endianness);

// Funções internas para conversão de endianness
uint16_t memory_convert_endian_16(memory_system_t *memory, uint16_t value);
uint32_t memory_convert_endian_32(memory_system_t *memory, uint32_t value);
```

### Alinhamento

Algumas plataformas têm requisitos específicos de alinhamento:

```c
// Configuração de requisitos de alinhamento
typedef enum {
    MEMORY_ALIGN_NONE,       // Sem restrições de alinhamento
    MEMORY_ALIGN_WORD,       // Alinhamento de 16 bits (endereços pares)
    MEMORY_ALIGN_DWORD       // Alinhamento de 32 bits (múltiplos de 4)
} memory_alignment_t;

// Definir requisitos de alinhamento
void memory_set_alignment(memory_system_t *memory, memory_alignment_t alignment);
```

### Depuração

O sistema inclui ferramentas de depuração para monitorar acessos à memória:

```c
// Tipo de callback para monitoramento de memória
typedef void (*memory_monitor_callback_t)(uint32_t address, uint8_t value,
                                          int is_write, void *context);

// Adicionar monitoramento a uma região
void memory_add_monitor(memory_system_t *memory, uint32_t start, uint32_t end,
                         memory_monitor_callback_t callback, void *context);

// Remover monitoramento
void memory_remove_monitor(memory_system_t *memory, uint32_t start, uint32_t end);
```

## Exemplos de Uso

### Configuração Básica do Sistema de Memória

```c
// Criar sistema de memória
memory_system_t *memory = memory_create(16);  // Suporta até 16 regiões

// Adicionar ROM
uint8_t *rom_data = load_rom("game.md");
memory_add_region(memory, 0x000000, 0x3FFFFF, rom_data, MEMORY_ACCESS_READ);

// Adicionar RAM
uint8_t *ram_data = malloc(64 * 1024);
memory_add_region(memory, 0xFF0000, 0xFFFFFF, ram_data, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

// Adicionar região mapeada para hardware
memory_add_mapped_region(memory, 0xA00000, 0xA0FFFF,
                         io_read_callback, io_write_callback,
                         io_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

// Configurar comportamento para endereços não mapeados
memory_set_default_callbacks(memory, unmapped_read, unmapped_write, NULL);
```

### Implementação de um Mapper NES

```c
// Callback para mapper
uint8_t mapper1_read(void *context, uint32_t address) {
    mapper1_t *mapper = (mapper1_t*)context;

    if (address < 0x2000) {
        // CHR ROM/RAM
        uint32_t bank = address / 0x1000;
        uint32_t offset = address % 0x1000;
        return mapper->chr_banks[bank][offset];
    } else if (address >= 0x8000) {
        // PRG ROM
        if (address < 0xC000) {
            // First bank (switchable)
            return mapper->prg_banks[mapper->prg_bank][address - 0x8000];
        } else {
            // Fixed bank (last bank)
            return mapper->prg_banks[mapper->prg_bank_count - 1][address - 0xC000];
        }
    }

    // Outros casos...
    return 0;
}

void mapper1_write(void *context, uint32_t address, uint8_t value) {
    mapper1_t *mapper = (mapper1_t*)context;

    if (address < 0x2000 && mapper->chr_ram) {
        // CHR RAM
        uint32_t bank = address / 0x1000;
        uint32_t offset = address % 0x1000;
        mapper->chr_banks[bank][offset] = value;
    } else if (address >= 0x8000) {
        // Registros de controle
        if (value & 0x80) {
            // Reset
            mapper->shift_register = 0;
            mapper->shift_count = 0;
            mapper->control = mapper->control | 0x0C;
        } else {
            // Carregar bit no registrador de deslocamento
            mapper->shift_register = ((mapper->shift_register >> 1) | ((value & 1) << 4));
            mapper->shift_count++;

            if (mapper->shift_count == 5) {
                // Processar comando completo
                uint8_t register_num = (address >> 13) & 0x03;
                process_mapper1_register(mapper, register_num, mapper->shift_register);

                mapper->shift_register = 0;
                mapper->shift_count = 0;
            }
        }
    }

    // Outros casos...
}
```

## Plataformas Específicas

### Mega Drive

No Mega Drive, o mapeamento de memória é o seguinte:

| Faixa de Endereço   | Tamanho | Descrição                     |
|---------------------|---------|-------------------------------|
| 0x000000 - 0x3FFFFF | 4MB     | Espaço para cartucho (ROM)    |
| 0x400000 - 0x7FFFFF | 4MB     | Reservado                     |
| 0x800000 - 0x9FFFFF | 2MB     | Reservado                     |
| 0xA00000 - 0xA0FFFF | 64KB    | Área de controle Z80          |
| 0xA10000 - 0xA1001F | 32 bytes | Portas de I/O e controle      |
| 0xA10020 - 0xBFFFFF | ~2MB    | Reservado                     |
| 0xC00000 - 0xDFFFFF | 2MB     | Espaço para VDP               |
| 0xE00000 - 0xFFFFFF | 2MB     | RAM (64KB espelhada)          |

```c
// Configuração da memória do Mega Drive
void setup_megadrive_memory(memory_system_t *memory, uint8_t *rom, size_t rom_size) {
    // ROM (até 4MB)
    memory_add_region(memory, 0x000000, 0x3FFFFF, rom, MEMORY_ACCESS_READ);

    // Z80 (64KB)
    memory_add_mapped_region(memory, 0xA00000, 0xA0FFFF,
                             z80_read_callback, z80_write_callback,
                             z80_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

    // I/O (32 bytes)
    memory_add_mapped_region(memory, 0xA10000, 0xA1001F,
                             io_read_callback, io_write_callback,
                             io_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

    // VDP (2MB)
    memory_add_mapped_region(memory, 0xC00000, 0xDFFFFF,
                             vdp_read_callback, vdp_write_callback,
                             vdp_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

    // RAM (64KB espelhada para 2MB)
    uint8_t *ram = malloc(64 * 1024);
    for (int i = 0; i < 32; i++) {
        uint32_t start = 0xE00000 + i * 0x10000;
        uint32_t end = start + 0xFFFF;
        memory_add_region(memory, start, end, ram,
                           MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);
    }
}
```

### NES

No NES, o mapeamento de memória é o seguinte:

| Faixa de Endereço | Tamanho | Descrição                           |
|-------------------|---------|-------------------------------------|
| 0x0000 - 0x07FF   | 2KB     | RAM interna                         |
| 0x0800 - 0x1FFF   | 6KB     | Espelhos da RAM (3 espelhos)        |
| 0x2000 - 0x2007   | 8 bytes  | Registradores do PPU                |
| 0x2008 - 0x3FFF   | ~8KB    | Espelhos dos registradores do PPU    |
| 0x4000 - 0x401F   | 32 bytes | Registradores de I/O e APU          |
| 0x4020 - 0x5FFF   | ~8KB    | Espaço para expansão                |
| 0x6000 - 0x7FFF   | 8KB     | SRAM para save                      |
| 0x8000 - 0xFFFF   | 32KB    | ROM do programa (PRG-ROM)           |

```c
// Configuração da memória do NES
void setup_nes_memory(memory_system_t *memory, nes_cartridge_t *cart) {
    // RAM interna (2KB)
    uint8_t *ram = malloc(2048);
    memory_add_region(memory, 0x0000, 0x07FF, ram, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

    // Espelhos da RAM
    for (int i = 0; i < 3; i++) {
        uint32_t start = 0x0800 + i * 0x0800;
        uint32_t end = start + 0x07FF;
        memory_add_mapped_region(memory, start, end,
                                  ram_mirror_read, ram_mirror_write,
                                  ram_mirror_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);
    }

    // Registradores do PPU
    memory_add_mapped_region(memory, 0x2000, 0x2007,
                             ppu_read_callback, ppu_write_callback,
                             ppu_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

    // Espelhos dos registradores do PPU
    for (int i = 0; i < 1023; i++) {
        uint32_t start = 0x2008 + i * 0x0008;
        uint32_t end = start + 0x0007;
        memory_add_mapped_region(memory, start, end,
                                  ppu_mirror_read, ppu_mirror_write,
                                  ppu_mirror_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);
    }

    // Registradores de I/O e APU
    memory_add_mapped_region(memory, 0x4000, 0x401F,
                             io_read_callback, io_write_callback,
                             io_context, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);

    // SRAM para save (se presente)
    if (cart->has_sram) {
        memory_add_region(memory, 0x6000, 0x7FFF, cart->sram,
                           MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);
    }

    // PRG-ROM e mapper
    memory_add_mapped_region(memory, 0x8000, 0xFFFF,
                             mapper_read_callback, mapper_write_callback,
                             cart->mapper, MEMORY_ACCESS_READ | MEMORY_ACCESS_WRITE);
}
```

## Considerações de Performance

### Acesso Direto vs. Callbacks

Para regiões de acesso frequente, como RAM, o acesso direto aos dados é mais eficiente que o uso de callbacks. O sistema de memória é projetado para otimizar automaticamente:

```c
uint8_t memory_read_8(memory_system_t *memory, uint32_t address) {
    memory_region_t *region = find_region(memory, address);

    if (!region) {
        // Região não encontrada, usar callback padrão
        return memory->default_read ? memory->default_read(memory->default_context, address) : 0xFF;
    }

    // Acesso direto é mais rápido se disponível
    if (region->data) {
        return region->data[address - region->start_address];
    } else if (region->read_callback) {
        return region->read_callback(region->context, address);
    }

    return 0xFF;
}
```

### Cache de Região

O sistema mantém um cache da última região acessada para leitura e para escrita, o que melhora significativamente o desempenho para acessos sequenciais ou localizados.

### Lookup Table

Para sistemas com espaço de endereçamento pequeno, como o NES, uma tabela de lookup pode ser mais eficiente:

```c
// Tabela de lookup para acesso rápido
void memory_build_lookup_table(memory_system_t *memory) {
    memory->read_table = malloc(65536 * sizeof(void*));
    memory->write_table = malloc(65536 * sizeof(void*));

    for (uint32_t addr = 0; addr <= 0xFFFF; addr++) {
        memory_region_t *region = find_region(memory, addr);

        if (region && region->data) {
            // Acesso direto
            memory->read_table[addr] = &region->data[addr - region->start_address];
            memory->write_table[addr] = &region->data[addr - region->start_address];
        } else {
            // Callback ou não mapeado
            memory->read_table[addr] = NULL;
            memory->write_table[addr] = NULL;
        }
    }
}

// Acesso de leitura otimizado com tabela
uint8_t memory_read_8_fast(memory_system_t *memory, uint16_t address) {
    if (memory->read_table[address]) {
        return *((uint8_t*)memory->read_table[address]);
    } else {
        return memory_read_8(memory, address);  // Fallback para método normal
    }
}
```

## Integração com Depuração

O sistema de memória integra-se com as ferramentas de depuração do emulador para fornecer recursos como:

1. **Logging de Acesso**: Rastreamento de leituras e escritas em regiões específicas
2. **Breakpoints**: Interrupção da emulação quando ocorrem acessos a endereços específicos
3. **Watchpoints**: Monitoramento de valores em endereços específicos
4. **Memory Dump**: Exportação do conteúdo da memória para análise
5. **Disassembly**: Decodificação de instruções da memória

```c
// Exemplo de integração com depuração
void memory_add_breakpoint(memory_system_t *memory, uint32_t address,
                            int break_on_read, int break_on_write) {
    breakpoint_t *bp = malloc(sizeof(breakpoint_t));
    bp->address = address;
    bp->break_on_read = break_on_read;
    bp->break_on_write = break_on_write;

    // Adicionar à lista de breakpoints
    add_to_breakpoint_list(memory->breakpoints, bp);

    // Adicionar monitor para acionar o breakpoint
    memory_add_monitor(memory, address, address, breakpoint_callback, bp);
}
```

## Conclusão

O sistema de memória do Mega_Emu fornece uma base sólida para a emulação de diversos sistemas. Sua flexibilidade permite acomodar as várias peculiaridades de diferentes consoles, enquanto seu design eficiente mantém o desempenho adequado para emulação em tempo real.

Futuras melhorias planejadas incluem:

1. Suporte a mapeamento baseado em páginas para sistemas complexos
2. JIT caching para acessos frequentes
3. Melhor integração com o sistema de savestates
4. Ferramentas visuais para análise de utilização de memória
5. Suporte para sistemas que manipulam memória virtual
