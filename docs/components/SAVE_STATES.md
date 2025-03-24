# Sistema de Save State Unificado - Mega_Emu

## Visão Geral

O sistema de Save State Unificado do Mega_Emu permite salvar e restaurar o estado completo do emulador em qualquer momento, oferecendo uma API consistente para todas as plataformas emuladas (Mega Drive, NES, Master System, SNES). A implementação prioriza confiabilidade, segurança e performance, fornecendo recursos avançados como compressão delta, thumbnails e suporte a nuvem.

## Arquitetura

O sistema é estruturado em camadas para separar as responsabilidades:

```
┌─────────────────────────────────────────────────────────┐
│                 INTERFACE PÚBLICA                        │
├─────────────────────────────────────────────────────────┤
│ emu_save_state_save()    emu_save_state_load()          │
│ emu_save_state_get_info()    emu_save_state_set_meta()  │
│ emu_save_state_capture_thumbnail()    emu_rewind()      │
├─────────┬───────────────┬────────────────┬─────────────┤
│VALIDAÇÃO│  SEGURANÇA    │  COMPRESSÃO    │ THUMBNAILS  │
├─────────┴───────────────┴────────────────┴─────────────┤
│              CAMADA DE ABSTRAÇÃO                        │
├─────────┬───────────────┬────────────────┬─────────────┤
│ MEGADRIVE│     NES       │ MASTER SYSTEM  │    SNES     │
└─────────┴───────────────┴────────────────┴─────────────┘
```

- **Interface Pública**: API consistente para todas as plataformas
- **Serviços Intermediários**: Validação, segurança, compressão e thumbnails
- **Camada de Abstração**: Traduz chamadas genéricas para específicas de cada plataforma
- **Implementações de Plataforma**: Código específico para cada console emulado

## Formato do Save State

Cada arquivo de save state contém:

1. **Cabeçalho**: Metadados e informações de validação
2. **Metadados Expandidos**: Informações detalhadas sobre o jogo e estado
3. **Thumbnail**: Imagem WebP comprimida do estado visual
4. **Dados por Componente**: Estado salvo de cada componente do sistema
5. **Checksum**: Hash SHA-256 para validação de integridade

## API Pública

### Funções Principais

```c
// Inicialização e finalização
emu_save_state_t* emu_save_state_init(uint8_t platform_id, const void* rom_data, size_t rom_size);
void emu_save_state_shutdown(emu_save_state_t* state);

// Operações de save/load
int32_t emu_save_state_save(emu_save_state_t* state, const char* filename, const emu_save_options_t* options);
int32_t emu_save_state_load(emu_save_state_t* state, const char* filename, const emu_save_options_t* options);

// Registro de regiões de memória
int32_t emu_save_state_register_region(emu_save_state_t* state, const char* name, void* data, size_t size);

// Metadados
int32_t emu_save_state_set_metadata(emu_save_state_t* state, const char* key, const char* value);
int32_t emu_save_state_get_metadata(emu_save_state_t* state, const char* key, char* value, size_t size);

// Thumbnails
int32_t emu_save_state_capture_thumbnail(emu_save_state_t* state, const void* framebuffer,
                                        uint32_t width, uint32_t height, uint32_t format);

// Rewind
int32_t emu_save_state_config_rewind(emu_save_state_t* state, uint32_t frames, uint32_t interval);
int32_t emu_save_state_rewind_capture(emu_save_state_t* state);
int32_t emu_save_state_rewind_step(emu_save_state_t* state, int32_t steps);

// Segurança
int32_t emu_save_state_set_encryption(emu_save_state_t* state, bool enable,
                                     const uint8_t* key, size_t key_size);
```

### Estruturas Principais

```c
// Opções para salvamento
typedef struct {
    bool include_thumbnail;     // Incluir thumbnail no arquivo
    uint8_t compression_level;  // Nível de compressão (0-9)
    bool use_encryption;        // Usar criptografia
    char description[256];      // Descrição do save state
    char tags[128];             // Tags para categorização
} emu_save_options_t;

// Informações sobre um save state
typedef struct {
    uint32_t version;           // Versão do formato
    uint8_t platform_id;        // ID da plataforma
    char rom_name[128];         // Nome da ROM
    char rom_hash[65];          // Hash da ROM (SHA-256)
    uint32_t timestamp;         // Timestamp de criação
    uint32_t size;              // Tamanho do arquivo em bytes
    bool has_thumbnail;         // Contém thumbnail?
    bool is_encrypted;          // Está criptografado?
    bool is_compressed;         // Está comprimido?
    char description[256];      // Descrição do save state
    char tags[128];             // Tags do save state
} emu_save_info_t;
```

## Implementação por Plataforma

Cada plataforma emulada implementa sua própria versão dos hooks de registro:

```c
// Mega Drive
int32_t md_save_state_register(emu_save_state_t* state);

// NES
int32_t nes_save_state_register(emu_save_state_t* state);

// Master System
int32_t sms_save_state_register(emu_save_state_t* state);

// SNES
int32_t snes_save_state_register(emu_save_state_t* state);
```

## Recursos Avançados

### Compressão Delta

O sistema utiliza compressão delta para reduzir o tamanho dos arquivos, armazenando apenas as diferenças entre estados consecutivos.

### Thumbnails WebP

Cada save state inclui uma thumbnail comprimida em formato WebP, permitindo visualização rápida do estado do jogo sem carregar o estado completo.

### Rewind

O sistema permite "rebobinar" o jogo mantendo um buffer circular de estados recentes:

1. Captura periódica de estados
2. Armazenamento otimizado em memória
3. Restauração rápida para pontos anteriores

### Validação de Integridade

Cada save state inclui:

1. Hash SHA-256 para verificação de integridade
2. Validação de assinatura e versão
3. Verificação de compatibilidade com a ROM atual

### Integração com Nuvem

Suporte para sincronização com serviços de nuvem:

1. Sincronização automática de saves
2. Resolução de conflitos
3. Gerenciamento de versões

## Exemplo de Uso

```c
// Inicializar o sistema para a plataforma Mega Drive
emu_save_state_t* state = emu_save_state_init(PLATFORM_MEGA_DRIVE, rom_data, rom_size);

// Registrar componentes do emulador
emu_save_state_register_region(state, "md_m68k", &md_state.m68k, sizeof(m68k_t));
emu_save_state_register_region(state, "md_z80", &md_state.z80, sizeof(z80_t));
emu_save_state_register_region(state, "md_vdp", &md_state.vdp, sizeof(vdp_t));
emu_save_state_register_region(state, "md_ram", md_state.ram, MD_RAM_SIZE);

// Configurar rewind
emu_save_state_config_rewind(state, 60, 10); // 60 estados, capturados a cada 10 frames

// Salvar estado
emu_save_options_t options = {
    .include_thumbnail = true,
    .compression_level = 5,
    .use_encryption = true,
    .description = "Antes do chefe final"
};
emu_save_state_save(state, "saves/sonic_final_boss.state", &options);

// Carregar estado
emu_save_state_load(state, "saves/sonic_final_boss.state", NULL);

// Finalizar
emu_save_state_shutdown(state);
```

## Compatibilidade com Formatos Antigos

O sistema mantém compatibilidade com formatos de save state antigos:

1. Detecção automática do formato
2. Conversão transparente para o novo formato
3. Suporte legado para versões anteriores do emulador

## Status de Implementação

| Plataforma | Status | Versão | Observações |
|------------|--------|--------|------------|
| Mega Drive | 85% | 1.3.0 | Delta compression implementada |
| Master System | 95% | 1.0.0 | Totalmente funcional |
| NES | 90% | 2.0.0 | Rewind implementado |
| SNES | 40% | 0.5.0 | Em desenvolvimento |

## Próximos Passos

1. **Unificação API**: Padronizar interface entre todas as plataformas
2. **Cloud Integration**: Implementar sincronização com serviços de nuvem
3. **Security**: Adicionar criptografia AES-256 e verificações de integridade
4. **Portabilidade**: Garantir compatibilidade entre diferentes sistemas operacionais
5. **UI Integration**: Melhorar integração com o frontend React

## Referências

- [Documentação Técnica Completa](../api/save-states-api.md)
- [Formato do Arquivo de Save State](../specs/save-state-format.md)
- [Guia de Migração](../guidelines/save-state-migration.md)

## Autores

Equipe de Desenvolvimento do Mega_Emu
Versão: 1.0.0 (Março 2025)
