# API do Sistema de Save States Unificado

Esta documentação detalha a API pública do sistema de Save States Unificado do Mega_Emu, incluindo as funções, estruturas e constantes disponíveis para desenvolvedores.

## Sumário

1. [Constantes e Definições](#constantes-e-definições)
2. [Estruturas de Dados](#estruturas-de-dados)
3. [Funções de Inicialização](#funções-de-inicialização)
4. [Operações de Save e Load](#operações-de-save-e-load)
5. [Gerenciamento de Regiões](#gerenciamento-de-regiões)
6. [Metadados](#metadados)
7. [Thumbnails](#thumbnails)
8. [Sistema de Rewind](#sistema-de-rewind)
9. [Segurança](#segurança)
10. [Integração com Nuvem](#integração-com-nuvem)
11. [Funções de Callback](#funções-de-callback)
12. [Códigos de Erro](#códigos-de-erro)
13. [Exemplos](#exemplos)

## Constantes e Definições

### Plataformas Suportadas

```c
// IDs de plataforma
#define EMU_PLATFORM_MEGA_DRIVE       0x01
#define EMU_PLATFORM_MASTER_SYSTEM    0x02
#define EMU_PLATFORM_NES              0x03
#define EMU_PLATFORM_SNES             0x04

// Versões de formato de save state
#define EMU_SAVE_FORMAT_VERSION       0x0300  // Versão atual (3.0)
#define EMU_SAVE_FORMAT_VERSION_2     0x0200  // Versão legada (2.0)
#define EMU_SAVE_FORMAT_VERSION_1     0x0100  // Versão legada (1.0)

// Flags de opções
#define EMU_SAVE_OPT_COMPRESS         0x0001  // Habilitar compressão
#define EMU_SAVE_OPT_ENCRYPT          0x0002  // Habilitar criptografia
#define EMU_SAVE_OPT_THUMBNAIL        0x0004  // Incluir thumbnail
#define EMU_SAVE_OPT_DELTA            0x0008  // Usar compressão delta
#define EMU_SAVE_OPT_VERIFY           0x0010  // Verificar após salvar
#define EMU_SAVE_OPT_CLOUD_SYNC       0x0020  // Sincronizar com nuvem
#define EMU_SAVE_OPT_AUTO_MIGRATE     0x0040  // Migrar automaticamente

// Formatos de thumbnail
#define EMU_THUMB_FORMAT_RGB565       0x01    // RGB565 (16-bit)
#define EMU_THUMB_FORMAT_RGB888       0x02    // RGB888 (24-bit)
#define EMU_THUMB_FORMAT_RGBA8888     0x03    // RGBA8888 (32-bit)
#define EMU_THUMB_FORMAT_WEBP         0x04    // WebP comprimido

// Limites
#define EMU_MAX_METADATA_SIZE         4096    // Tamanho máximo para metadados
#define EMU_MAX_REGIONS               256     // Máximo de regiões por save state
#define EMU_MAX_REGION_NAME_LEN       64      // Tamanho máximo para nomes de região
#define EMU_MAX_REWIND_FRAMES         600     // Máximo de frames para rewind

// Métodos de criptografia
#define EMU_CRYPT_NONE                0x00    // Sem criptografia
#define EMU_CRYPT_AES256_CBC          0x01    // AES-256 em modo CBC
#define EMU_CRYPT_AES256_GCM          0x02    // AES-256 em modo GCM (autenticado)

// Provedores de nuvem
#define EMU_CLOUD_NONE                0x00    // Sem integração com nuvem
#define EMU_CLOUD_DROPBOX             0x01    // Dropbox
#define EMU_CLOUD_GOOGLE_DRIVE        0x02    // Google Drive
#define EMU_CLOUD_ONEDRIVE            0x03    // Microsoft OneDrive
#define EMU_CLOUD_CUSTOM              0xFF    // Provedor personalizado
```

## Estruturas de Dados

### emu_save_state_t

Estrutura principal que representa um contexto de save state.

```c
typedef struct emu_save_state emu_save_state_t;
```

Esta é uma estrutura opaca cujos detalhes de implementação estão ocultos da API pública.

### emu_save_options_t

Opções para operações de save state.

```c
typedef struct {
    uint32_t flags;                  // Combinação de flags EMU_SAVE_OPT_*
    uint8_t compression_level;       // Nível de compressão (0-9)
    uint8_t encryption_method;       // Método de criptografia (EMU_CRYPT_*)
    uint8_t cloud_provider;          // Provedor de nuvem (EMU_CLOUD_*)
    uint8_t reserved;                // Reservado para alinhamento
    char description[256];           // Descrição do save state
    char tags[128];                  // Tags para categorização
    void* user_data;                 // Dados definidos pelo usuário
} emu_save_options_t;
```

### emu_save_info_t

Informações sobre um save state.

```c
typedef struct {
    uint32_t version;                // Versão do formato
    uint8_t platform_id;             // ID da plataforma
    char rom_name[128];              // Nome da ROM
    char rom_hash[65];               // Hash SHA-256 da ROM
    uint64_t timestamp;              // Timestamp de criação
    uint32_t size;                   // Tamanho total em bytes
    uint32_t flags;                  // Flags que indicam características
    uint8_t encryption_method;       // Método de criptografia usado
    uint8_t cloud_status;            // Status de sincronização com nuvem
    uint8_t reserved[2];             // Reservado para alinhamento
    char description[256];           // Descrição do save state
    char tags[128];                  // Tags para categorização
    uint16_t thumbnail_width;        // Largura da thumbnail
    uint16_t thumbnail_height;       // Altura da thumbnail
    uint8_t thumbnail_format;        // Formato da thumbnail
    uint8_t reserved2[3];            // Reservado para alinhamento
    char cloud_url[256];             // URL na nuvem (se sincronizado)
    uint64_t cloud_timestamp;        // Timestamp da última sincronização
} emu_save_info_t;
```

### emu_save_region_t

Representa uma região de memória registrada.

```c
typedef struct {
    char name[EMU_MAX_REGION_NAME_LEN]; // Nome da região
    void* data;                      // Ponteiro para dados
    size_t size;                     // Tamanho em bytes
    uint32_t flags;                  // Flags para esta região
    void (*pre_save)(void* data, size_t size, void* user_data);
    void (*post_load)(void* data, size_t size, void* user_data);
    void* user_data;                 // Dados de contexto para callbacks
} emu_save_region_t;
```

### emu_save_callbacks_t

Callbacks para personalização do comportamento do save state.

```c
typedef struct {
    void (*pre_save)(emu_save_state_t* state, void* user_data);
    void (*post_save)(emu_save_state_t* state, const char* filename, void* user_data);
    void (*pre_load)(emu_save_state_t* state, const char* filename, void* user_data);
    void (*post_load)(emu_save_state_t* state, void* user_data);
    void (*error)(emu_save_state_t* state, int32_t error_code, const char* message, void* user_data);
    void (*cloud_sync_progress)(emu_save_state_t* state, const char* operation, int32_t progress, int32_t total, void* user_data);
    void (*cloud_sync_complete)(emu_save_state_t* state, const char* cloud_url, int32_t status, void* user_data);
    void* user_data;
} emu_save_callbacks_t;
```

### emu_cloud_config_t

Configuração para integração com serviços de nuvem.

```c
typedef struct {
    uint8_t provider;                // Provedor de nuvem (EMU_CLOUD_*)
    char auth_token[256];            // Token de autenticação
    char folder_path[256];           // Caminho da pasta na nuvem
    bool auto_sync;                  // Sincronização automática
    uint32_t sync_interval;          // Intervalo de sincronização (segundos)
    bool conflict_resolution;        // Resolução automática de conflitos
    bool* cancel_flag;               // Ponteiro para flag de cancelamento
    void* user_data;                 // Dados definidos pelo usuário
    void (*custom_upload)(const char* local_path, const char* remote_path, void* user_data);
    void (*custom_download)(const char* remote_path, const char* local_path, void* user_data);
} emu_cloud_config_t;
```

### emu_encryption_config_t

Configuração para criptografia de save states.

```c
typedef struct {
    uint8_t method;                  // Método de criptografia (EMU_CRYPT_*)
    uint8_t key[32];                 // Chave de criptografia (256 bits)
    uint8_t iv[16];                  // Vetor de inicialização (se aplicável)
    bool derive_from_password;       // Derivar chave de senha?
    char password[128];              // Senha (se derive_from_password for true)
    uint32_t kdf_iterations;         // Iterações para derivação de chave
    uint8_t salt[16];                // Salt para derivação de chave
} emu_encryption_config_t;
```

## Funções de Inicialização

### emu_save_state_init

```c
emu_save_state_t* emu_save_state_init(
    uint8_t platform_id,
    const void* rom_data,
    size_t rom_size
);
```

Inicializa um novo contexto de save state para a plataforma especificada.

**Parâmetros:**

- `platform_id`: ID da plataforma (EMU_PLATFORM_*)
- `rom_data`: Ponteiro para os dados da ROM
- `rom_size`: Tamanho da ROM em bytes

**Retorno:**

- Ponteiro para o contexto inicializado ou NULL em caso de erro

### emu_save_state_init_ex

```c
emu_save_state_t* emu_save_state_init_ex(
    uint8_t platform_id,
    const void* rom_data,
    size_t rom_size,
    const emu_save_callbacks_t* callbacks
);
```

Inicializa um novo contexto de save state com callbacks personalizados.

**Parâmetros:**

- `platform_id`: ID da plataforma (EMU_PLATFORM_*)
- `rom_data`: Ponteiro para os dados da ROM
- `rom_size`: Tamanho da ROM em bytes
- `callbacks`: Estrutura com callbacks personalizados (pode ser NULL)

**Retorno:**

- Ponteiro para o contexto inicializado ou NULL em caso de erro

### emu_save_state_shutdown

```c
void emu_save_state_shutdown(emu_save_state_t* state);
```

Libera recursos associados a um contexto de save state.

**Parâmetros:**

- `state`: Ponteiro para o contexto a ser liberado

## Operações de Save e Load

### emu_save_state_save

```c
int32_t emu_save_state_save(
    emu_save_state_t* state,
    const char* filename,
    const emu_save_options_t* options
);
```

Salva o estado atual em um arquivo.

**Parâmetros:**

- `state`: Contexto de save state
- `filename`: Caminho do arquivo para salvar
- `options`: Opções para salvar (pode ser NULL para opções padrão)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_load

```c
int32_t emu_save_state_load(
    emu_save_state_t* state,
    const char* filename,
    const emu_save_options_t* options
);
```

Carrega um estado de um arquivo.

**Parâmetros:**

- `state`: Contexto de save state
- `filename`: Caminho do arquivo para carregar
- `options`: Opções para carregamento (pode ser NULL para opções padrão)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_save_to_memory

```c
int32_t emu_save_state_save_to_memory(
    emu_save_state_t* state,
    void** buffer,
    size_t* size,
    const emu_save_options_t* options
);
```

Salva o estado atual em um buffer de memória.

**Parâmetros:**

- `state`: Contexto de save state
- `buffer`: Ponteiro que receberá o buffer alocado
- `size`: Tamanho do buffer alocado
- `options`: Opções para salvar (pode ser NULL para opções padrão)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

**Nota:** O chamador é responsável por liberar o buffer usando `emu_save_state_free_buffer()`.

### emu_save_state_load_from_memory

```c
int32_t emu_save_state_load_from_memory(
    emu_save_state_t* state,
    const void* buffer,
    size_t size,
    const emu_save_options_t* options
);
```

Carrega um estado de um buffer de memória.

**Parâmetros:**

- `state`: Contexto de save state
- `buffer`: Buffer contendo o save state
- `size`: Tamanho do buffer
- `options`: Opções para carregamento (pode ser NULL para opções padrão)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_free_buffer

```c
void emu_save_state_free_buffer(void* buffer);
```

Libera um buffer alocado por `emu_save_state_save_to_memory()`.

**Parâmetros:**

- `buffer`: Buffer a ser liberado

### emu_save_state_get_info

```c
int32_t emu_save_state_get_info(
    const char* filename,
    emu_save_info_t* info
);
```

Obtém informações sobre um arquivo de save state sem carregá-lo completamente.

**Parâmetros:**

- `filename`: Caminho do arquivo
- `info`: Estrutura que receberá as informações

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

## Gerenciamento de Regiões

### emu_save_state_register_region

```c
int32_t emu_save_state_register_region(
    emu_save_state_t* state,
    const char* name,
    void* data,
    size_t size
);
```

Registra uma região de memória para ser salva/carregada.

**Parâmetros:**

- `state`: Contexto de save state
- `name`: Nome da região (deve ser único)
- `data`: Ponteiro para os dados
- `size`: Tamanho em bytes

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_register_region_ex

```c
int32_t emu_save_state_register_region_ex(
    emu_save_state_t* state,
    const char* name,
    void* data,
    size_t size,
    void (*pre_save)(void* data, size_t size, void* user_data),
    void (*post_load)(void* data, size_t size, void* user_data),
    void* user_data
);
```

Registra uma região de memória com callbacks personalizados.

**Parâmetros:**

- `state`: Contexto de save state
- `name`: Nome da região (deve ser único)
- `data`: Ponteiro para os dados
- `size`: Tamanho em bytes
- `pre_save`: Função chamada antes de salvar a região (pode ser NULL)
- `post_load`: Função chamada após carregar a região (pode ser NULL)
- `user_data`: Dados passados para os callbacks (pode ser NULL)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_unregister_region

```c
int32_t emu_save_state_unregister_region(
    emu_save_state_t* state,
    const char* name
);
```

Remove o registro de uma região previamente registrada.

**Parâmetros:**

- `state`: Contexto de save state
- `name`: Nome da região a ser removida

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_register_platform

```c
int32_t emu_save_state_register_platform(
    emu_save_state_t* state
);
```

Registra automaticamente todas as regiões específicas da plataforma.

**Parâmetros:**

- `state`: Contexto de save state

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

## Metadados

### emu_save_state_set_metadata

```c
int32_t emu_save_state_set_metadata(
    emu_save_state_t* state,
    const char* key,
    const char* value
);
```

Define um valor de metadado.

**Parâmetros:**

- `state`: Contexto de save state
- `key`: Nome da chave
- `value`: Valor associado

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_get_metadata

```c
int32_t emu_save_state_get_metadata(
    emu_save_state_t* state,
    const char* key,
    char* value,
    size_t size
);
```

Obtém um valor de metadado.

**Parâmetros:**

- `state`: Contexto de save state
- `key`: Nome da chave
- `value`: Buffer para receber o valor
- `size`: Tamanho do buffer

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_get_all_metadata

```c
int32_t emu_save_state_get_all_metadata(
    emu_save_state_t* state,
    char** buffer,
    size_t* size
);
```

Obtém todos os metadados como um objeto JSON.

**Parâmetros:**

- `state`: Contexto de save state
- `buffer`: Ponteiro que receberá o buffer alocado
- `size`: Tamanho do buffer alocado

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

**Nota:** O chamador é responsável por liberar o buffer usando `emu_save_state_free_buffer()`.

## Thumbnails

### emu_save_state_capture_thumbnail

```c
int32_t emu_save_state_capture_thumbnail(
    emu_save_state_t* state,
    const void* framebuffer,
    uint32_t width,
    uint32_t height,
    uint32_t format
);
```

Captura uma thumbnail do framebuffer atual.

**Parâmetros:**

- `state`: Contexto de save state
- `framebuffer`: Ponteiro para o framebuffer
- `width`: Largura em pixels
- `height`: Altura em pixels
- `format`: Formato do framebuffer (EMU_THUMB_FORMAT_*)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_get_thumbnail

```c
int32_t emu_save_state_get_thumbnail(
    emu_save_state_t* state,
    void** buffer,
    size_t* size,
    uint32_t* width,
    uint32_t* height,
    uint32_t* format
);
```

Obtém a thumbnail associada ao save state.

**Parâmetros:**

- `state`: Contexto de save state
- `buffer`: Ponteiro que receberá o buffer alocado
- `size`: Tamanho do buffer alocado
- `width`: Ponteiro para receber a largura
- `height`: Ponteiro para receber a altura
- `format`: Ponteiro para receber o formato

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

**Nota:** O chamador é responsável por liberar o buffer usando `emu_save_state_free_buffer()`.

### emu_save_state_get_thumbnail_from_file

```c
int32_t emu_save_state_get_thumbnail_from_file(
    const char* filename,
    void** buffer,
    size_t* size,
    uint32_t* width,
    uint32_t* height,
    uint32_t* format
);
```

Extrai a thumbnail de um arquivo de save state sem carregá-lo completamente.

**Parâmetros:**

- `filename`: Caminho do arquivo
- `buffer`: Ponteiro que receberá o buffer alocado
- `size`: Tamanho do buffer alocado
- `width`: Ponteiro para receber a largura
- `height`: Ponteiro para receber a altura
- `format`: Ponteiro para receber o formato

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

**Nota:** O chamador é responsável por liberar o buffer usando `emu_save_state_free_buffer()`.

## Sistema de Rewind

### emu_save_state_config_rewind

```c
int32_t emu_save_state_config_rewind(
    emu_save_state_t* state,
    uint32_t frames,
    uint32_t interval
);
```

Configura o sistema de rewind.

**Parâmetros:**

- `state`: Contexto de save state
- `frames`: Número máximo de frames a armazenar
- `interval`: Intervalo entre capturas (em frames)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_rewind_capture

```c
int32_t emu_save_state_rewind_capture(
    emu_save_state_t* state
);
```

Captura o estado atual para o buffer de rewind.

**Parâmetros:**

- `state`: Contexto de save state

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_rewind_step

```c
int32_t emu_save_state_rewind_step(
    emu_save_state_t* state,
    int32_t steps
);
```

Retrocede ou avança no buffer de rewind.

**Parâmetros:**

- `state`: Contexto de save state
- `steps`: Número de passos (negativo para retroceder, positivo para avançar)

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_rewind_get_info

```c
int32_t emu_save_state_rewind_get_info(
    emu_save_state_t* state,
    uint32_t* total_frames,
    uint32_t* current_position,
    uint32_t* memory_usage
);
```

Obtém informações sobre o estado atual do buffer de rewind.

**Parâmetros:**

- `state`: Contexto de save state
- `total_frames`: Ponteiro para receber o número total de frames armazenados
- `current_position`: Ponteiro para receber a posição atual no buffer
- `memory_usage`: Ponteiro para receber o uso de memória em bytes

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

## Segurança

### emu_save_state_set_encryption

```c
int32_t emu_save_state_set_encryption(
    emu_save_state_t* state,
    const emu_encryption_config_t* config
);
```

Configura a criptografia para o save state.

**Parâmetros:**

- `state`: Contexto de save state
- `config`: Configuração de criptografia

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_verify

```c
int32_t emu_save_state_verify(
    const char* filename,
    bool* valid,
    char* error_message,
    size_t error_message_size
);
```

Verifica a integridade de um arquivo de save state.

**Parâmetros:**

- `filename`: Caminho do arquivo
- `valid`: Ponteiro para receber o resultado da validação
- `error_message`: Buffer para receber mensagem de erro (se houver)
- `error_message_size`: Tamanho do buffer

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

## Integração com Nuvem

### emu_save_state_cloud_configure

```c
int32_t emu_save_state_cloud_configure(
    emu_save_state_t* state,
    const emu_cloud_config_t* config
);
```

Configura a integração com serviços de nuvem.

**Parâmetros:**

- `state`: Contexto de save state
- `config`: Configuração para a nuvem

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_cloud_sync

```c
int32_t emu_save_state_cloud_sync(
    emu_save_state_t* state,
    const char* filename,
    bool upload
);
```

Sincroniza um save state com a nuvem.

**Parâmetros:**

- `state`: Contexto de save state
- `filename`: Caminho do arquivo local
- `upload`: true para upload, false para download

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

### emu_save_state_cloud_list

```c
int32_t emu_save_state_cloud_list(
    emu_save_state_t* state,
    char** buffer,
    size_t* size
);
```

Lista todos os save states disponíveis na nuvem como um objeto JSON.

**Parâmetros:**

- `state`: Contexto de save state
- `buffer`: Ponteiro que receberá o buffer alocado
- `size`: Tamanho do buffer alocado

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

**Nota:** O chamador é responsável por liberar o buffer usando `emu_save_state_free_buffer()`.

## Funções de Callback

### emu_save_state_set_callbacks

```c
int32_t emu_save_state_set_callbacks(
    emu_save_state_t* state,
    const emu_save_callbacks_t* callbacks
);
```

Define callbacks para o save state.

**Parâmetros:**

- `state`: Contexto de save state
- `callbacks`: Estrutura com callbacks

**Retorno:**

- EMU_SUCCESS em caso de sucesso
- Código de erro negativo em caso de falha

## Códigos de Erro

```c
// Códigos de sucesso
#define EMU_SUCCESS                     0

// Códigos de erro gerais
#define EMU_ERROR_INVALID_PARAMETER    -1
#define EMU_ERROR_INVALID_STATE        -2
#define EMU_ERROR_OUT_OF_MEMORY        -3
#define EMU_ERROR_FILE_NOT_FOUND       -4
#define EMU_ERROR_FILE_READ            -5
#define EMU_ERROR_FILE_WRITE           -6
#define EMU_ERROR_FILE_INVALID         -7
#define EMU_ERROR_BUFFER_TOO_SMALL     -8

// Códigos de erro específicos de save state
#define EMU_ERROR_SAVE_INVALID_FORMAT  -100
#define EMU_ERROR_SAVE_VERSION         -101
#define EMU_ERROR_SAVE_WRONG_PLATFORM  -102
#define EMU_ERROR_SAVE_WRONG_ROM       -103
#define EMU_ERROR_SAVE_CHECKSUM        -104
#define EMU_ERROR_SAVE_CORRUPTED       -105
#define EMU_ERROR_SAVE_ENCRYPTED       -106
#define EMU_ERROR_SAVE_DECRYPTION      -107
#define EMU_ERROR_SAVE_COMPRESSION     -108
#define EMU_ERROR_SAVE_DECOMPRESSION   -109
#define EMU_ERROR_SAVE_REGION_MISSING  -110
#define EMU_ERROR_SAVE_REGION_SIZE     -111
#define EMU_ERROR_SAVE_MAX_REGIONS     -112

// Códigos de erro de nuvem
#define EMU_ERROR_CLOUD_NOT_CONFIGURED -200
#define EMU_ERROR_CLOUD_AUTH           -201
#define EMU_ERROR_CLOUD_NETWORK        -202
#define EMU_ERROR_CLOUD_CONFLICT       -203
#define EMU_ERROR_CLOUD_QUOTA          -204
#define EMU_ERROR_CLOUD_NOT_FOUND      -205
#define EMU_ERROR_CLOUD_TIMEOUT        -206
#define EMU_ERROR_CLOUD_CANCELED       -207
```

## Exemplos

### Exemplo Básico

```c
// Inicializar save state
emu_save_state_t* state = emu_save_state_init(EMU_PLATFORM_MEGA_DRIVE, rom_data, rom_size);

// Registrar componentes da plataforma automaticamente
emu_save_state_register_platform(state);

// Configurar opções
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL,
    .compression_level = 6,
    .description = "Level 5 - Boss Fight"
};

// Capturar thumbnail
emu_save_state_capture_thumbnail(state, framebuffer, 320, 240, EMU_THUMB_FORMAT_RGB565);

// Salvar
emu_save_state_save(state, "savegame.state", &options);

// Carregar
emu_save_state_load(state, "savegame.state", NULL);

// Liberar recursos
emu_save_state_shutdown(state);
```

### Exemplo de Rewind

```c
// Configurar rewind (manter 120 frames, capturar a cada 2 frames)
emu_save_state_config_rewind(state, 120, 2);

// Durante o loop principal do emulador
while (emulator_running) {
    // Executar um frame
    emulator_run_frame();

    // Capturar estado para rewind (a cada 2 frames)
    if (frame_counter % 2 == 0) {
        emu_save_state_rewind_capture(state);
    }

    // Processar input
    if (is_rewind_key_pressed) {
        // Retroceder 1 estado
        emu_save_state_rewind_step(state, -1);
    }

    frame_counter++;
}
```

### Exemplo com Criptografia

```c
// Configurar criptografia
emu_encryption_config_t crypto_config = {
    .method = EMU_CRYPT_AES256_CBC,
    .derive_from_password = true,
    .password = "senha_segura",
    .kdf_iterations = 10000
};

// Aplicar configuração
emu_save_state_set_encryption(state, &crypto_config);

// Opções com criptografia ativada
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_ENCRYPT | EMU_SAVE_OPT_COMPRESS,
    .description = "Dados sensíveis"
};

// Salvar com criptografia
emu_save_state_save(state, "encrypted_save.state", &options);
```

### Exemplo de Integração com Nuvem

```c
// Configurar integração com nuvem
emu_cloud_config_t cloud_config = {
    .provider = EMU_CLOUD_GOOGLE_DRIVE,
    .auth_token = "oauth_token_aqui",
    .folder_path = "/Mega_Emu_Saves",
    .auto_sync = true,
    .sync_interval = 300  // 5 minutos
};

// Aplicar configuração
emu_save_state_cloud_configure(state, &cloud_config);

// Opções com sincronização de nuvem
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_CLOUD_SYNC,
    .description = "Final Boss"
};

// Salvar e sincronizar com a nuvem
emu_save_state_save(state, "boss_fight.state", &options);

// Sincronizar manualmente um save state existente
emu_save_state_cloud_sync(state, "old_save.state", true);  // upload

// Listar saves disponíveis na nuvem
char* cloud_list;
size_t list_size;
emu_save_state_cloud_list(state, &cloud_list, &list_size);
printf("Saves na nuvem: %s\n", cloud_list);
emu_save_state_free_buffer(cloud_list);
```

### Exemplo de Callbacks Personalizados

```c
// Callbacks
void pre_save_callback(emu_save_state_t* state, void* user_data) {
    printf("Preparando para salvar...\n");
}

void post_load_callback(emu_save_state_t* state, void* user_data) {
    printf("Estado carregado com sucesso!\n");
    // Restaurar contexto adicional
    emulator_t* emu = (emulator_t*)user_data;
    emu_update_display(emu);
}

void error_callback(emu_save_state_t* state, int32_t error_code,
                   const char* message, void* user_data) {
    printf("Erro %d: %s\n", error_code, message);
}

// Configuração de callbacks
emu_save_callbacks_t callbacks = {
    .pre_save = pre_save_callback,
    .post_load = post_load_callback,
    .error = error_callback,
    .user_data = emulator_instance
};

// Inicializar com callbacks
emu_save_state_t* state = emu_save_state_init_ex(
    EMU_PLATFORM_NES, rom_data, rom_size, &callbacks);
```

## Compatibilidade da API

Esta API é compatível com as versões 2.0 e superiores do Mega_Emu. Para suporte a versões anteriores, consulte a documentação legada.
