# Plano de Consolidação do NESCartridge e Mappers para C

## Fase 1: Análise de Compatibilidade e Manutenção

### Padrões Comuns entre Plataformas C (Mega Drive e Master System)

Após analisar o código existente do Mega Drive e Master System, identificamos os seguintes padrões comuns que podem ser aplicados ao NES:

1. **Estrutura de API**:
   - Ambos usam estruturas opacas com funções de criação/destruição (`sms_cartridge_create`/`sms_cartridge_destroy`)
   - Interfaces claras baseadas em funções C com prefixos específicos (ex: `md_mapper_*`, `sms_cartridge_*`)
   - Funções para leitura/escrita de memória bem definidas

2. **Gerenciamento de Memória**:
   - Alocação e liberação explícita de recursos
   - Estruturas de dados claramente definidas com documentação
   - Sem uso de STL ou containers C++

3. **Detecção e Registro de Mappers**:
   - No Mega Drive: `md_mapper_detect_type` para detecção automática
   - Funções de callback para operações específicas do mapper
   - Centralização da criação de mappers

4. **Integração com Save States**:
   - Funções específicas para registro no sistema de save state
   - Uso de ponteiros de função para operações de I/O específicas
   - Métodos para salvar/carregar SRAM e estados

### Análise de Manutenção

- A implementação atual do NES possui aproximadamente **11.036 linhas** em arquivos duplicados (C e C++)
- Os arquivos Mega Drive/Master System têm menor complexidade ciclomática por função (média de 6-8 vs 12-15 nas classes C++)
- A versão C do cartucho NES (`nes_cartridge.c`) tem melhor documentação, mas carece da implementação completa de alguns mappers

## Fase 2: Plano de Consolidação para C

### Fortalecimento da Implementação C

1. **Completar Documentação e Tratamento de Erros**:

```c
/**
 * @brief Carrega uma ROM no cartucho NES
 *
 * @param cart Ponteiro para o cartucho NES
 * @param rom_path Caminho para o arquivo ROM
 * @return int32_t 0 em caso de sucesso, código de erro em caso de falha
 */
int32_t nes_cartridge_load(nes_cartridge_t *cart, const char *rom_path)
{
    if (!cart || !rom_path) {
        LOG_ERROR(EMU_LOG_CAT_NES, "Parâmetros inválidos para carregamento de ROM");
        return NES_ERROR_INVALID_PARAMETER;
    }

    // Implementação existente...

    if (resultado_erro) {
        LOG_ERROR(EMU_LOG_CAT_NES, "Falha ao carregar ROM: %s (Erro: %d)", rom_path, codigo_erro);
        return codigo_erro;
    }

    LOG_INFO(EMU_LOG_CAT_NES, "ROM carregada com sucesso: %s", rom_path);
    return 0;
}
```

2. **Implementar Sistema de Registro de Mappers**:

```c
// Definição de função para inicialização de mappers
typedef int32_t (*nes_mapper_init_func)(nes_cartridge_t *cart);

// Array de funções de inicialização de mappers
static nes_mapper_init_func nes_mapper_init_funcs[256] = {0};

// Registro de mappers suportados
void nes_cartridge_register_mappers(void)
{
    nes_mapper_init_funcs[0] = nes_mapper_0_init;  // NROM
    nes_mapper_init_funcs[1] = nes_mapper_1_init;  // MMC1
    nes_mapper_init_funcs[2] = nes_mapper_2_init;  // UNROM
    nes_mapper_init_funcs[3] = nes_mapper_3_init;  // CNROM
    nes_mapper_init_funcs[4] = nes_mapper_4_init;  // MMC3
    nes_mapper_init_funcs[5] = nes_mapper_5_init;  // MMC5
    // etc.
}

// Inicialização de mapper baseada no número
int32_t nes_cartridge_create_mapper(nes_cartridge_t *cart)
{
    if (!cart) return NES_ERROR_INVALID_PARAMETER;

    // Verifica se o mapper é suportado
    if (cart->mapper_number >= 256 || !nes_mapper_init_funcs[cart->mapper_number]) {
        LOG_ERROR(EMU_LOG_CAT_NES, "Mapper não suportado: %d", cart->mapper_number);
        return NES_ERROR_UNSUPPORTED_MAPPER;
    }

    // Inicializa o mapper
    return nes_mapper_init_funcs[cart->mapper_number](cart);
}
```

## Fase 3: Implementação e Migração de Mappers

### Conversão dos Mappers C++ para C

Para cada mapper, vamos seguir um padrão consistente:

1. **Estrutura do Contexto do Mapper**:

```c
// Para Mapper 5 (MMC5)
typedef struct {
    nes_mapper_base_ctx_t base;  // Campos base

    // Campos específicos do MMC5
    uint8_t prg_mode;
    uint8_t chr_mode;
    uint8_t exram_mode;
    uint8_t nametable_mode;
    uint8_t prg_banks[5];
    uint8_t chr_banks[12];
    uint8_t exram[1024];
    uint8_t irq_scanline;
    uint8_t irq_enable;
    uint8_t irq_status;
    uint8_t mult_a;
    uint8_t mult_b;
    uint16_t mult_result;

    // etc.
} mapper5_ctx_t;
```

2. **Funções do Mapper**:

```c
// Protótipos
static uint8_t mapper5_cpu_read(void *ctx, uint16_t addr);
static void mapper5_cpu_write(void *ctx, uint16_t addr, uint8_t val);
static uint8_t mapper5_chr_read(void *ctx, uint16_t addr);
static void mapper5_chr_write(void *ctx, uint16_t addr, uint8_t val);
static void mapper5_scanline(void *ctx);
static void mapper5_reset(void *ctx);
static void mapper5_shutdown(void *ctx);

// Inicialização
int32_t nes_mapper_5_init(nes_cartridge_t *cart)
{
    // Alocação e inicialização do contexto
    mapper5_ctx_t *ctx = calloc(1, sizeof(mapper5_ctx_t));
    if (!ctx) return NES_ERROR_MEMORY_ALLOCATION;

    ctx->base.cart = cart;

    // Configuração dos callbacks
    cart->mapper->cpu_read = mapper5_cpu_read;
    cart->mapper->cpu_write = mapper5_cpu_write;
    cart->mapper->chr_read = mapper5_chr_read;
    cart->mapper->chr_write = mapper5_chr_write;
    cart->mapper->scanline = mapper5_scanline;
    cart->mapper->reset = mapper5_reset;
    cart->mapper->shutdown = mapper5_shutdown;
    cart->mapper->context = ctx;

    // Valores iniciais
    mapper5_reset(ctx);

    return 0;
}
```

## Fase 4: Adaptação e Compatibilidade

### Compatibilidade com C++

Para manter compatibilidade temporária com o código C++ existente, implementaremos wrappers:

```cpp
// No arquivo nes_cartridge_wrapper.cpp
extern "C" {
#include "nes_cartridge.h"
}

#include "nes_cartridge.hpp"

// Função de bridge
extern "C" void* nes_cpp_create_cartridge() {
    return new NESCartridge();
}

extern "C" void nes_cpp_destroy_cartridge(void* instance) {
    if (instance) {
        delete static_cast<NESCartridge*>(instance);
    }
}

extern "C" int32_t nes_cpp_cartridge_load_rom(void* instance, const char* filename) {
    if (!instance || !filename) return 0;
    try {
        NESCartridge* cart = static_cast<NESCartridge*>(instance);
        return cart->loadROM(filename) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}
```

## Fase 5: Medição e Validação

### Benchmarks de Compatibilidade

Um script de teste será desenvolvido para comparar o comportamento entre as implementações C e C++:

```c
// No arquivo mapper_compatibility_tests.c
void test_mapper_compatibility() {
    // Teste para mapper 5 (MMC5)
    nes_cartridge_t *cart_c = nes_cartridge_init();
    void *cart_cpp = nes_cpp_create_cartridge();

    // Carregar a mesma ROM em ambos
    nes_cartridge_load(cart_c, "test_roms/castlevania3.nes");
    nes_cpp_cartridge_load_rom(cart_cpp, "test_roms/castlevania3.nes");

    // Testar operações de leitura e escrita
    for (uint16_t addr = 0x8000; addr < 0xFFFF; addr++) {
        uint8_t val_c = nes_cartridge_cpu_read(cart_c, addr);
        uint8_t val_cpp = nes_cpp_cartridge_read(cart_cpp, addr);

        if (val_c != val_cpp) {
            printf("Diferença detectada em 0x%04X: C=%02X, C++=%02X\n",
                   addr, val_c, val_cpp);
        }
    }

    // Limpar
    nes_cartridge_shutdown(cart_c);
    nes_cpp_destroy_cartridge(cart_cpp);
}
```

## Fase 6: Cronograma de Implementação

### Semana 1: Preparação e Arquitetura

1. Documentar cada mapper existente (tanto C quanto C++)
2. Implementar sistema de registro de mappers
3. Criar estruturas base para todos os mappers

### Semana 2: Consolidação da Base C

1. Completar a implementação dos mappers 0, 1, 2, 3 (os mais simples)
2. Implementar funções auxiliares comuns a todos os mappers
3. Desenvolver testes de unidade para esses mappers

### Semana 3: Mappers Avançados

1. Consolidar implementações do mapper 4 (MMC3)
2. Consolidar implementações do mapper 5 (MMC5)
3. Implementar wrappers C++ temporários

### Semana 4: Validação e Integração

1. Executar benchmarks e testes de compatibilidade
2. Corrigir problemas detectados
3. Documentar API final

### Semana 5: Limpeza

1. Remover código C++ duplicado
2. Remover arquivos não utilizados
3. Finalizar documentação

## Benefícios Esperados

1. **Consistência de Código**: Todo o subsistema NES seguirá o mesmo padrão de código das outras plataformas
2. **Manutenção Simplificada**: Apenas uma implementação para manter
3. **Performance Potencialmente Melhor**: Eliminação do overhead de chamadas de métodos virtuais
4. **Maior Portabilidade**: Código C tem melhor suporte em plataformas embarcadas
