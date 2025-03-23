# API do Emulador Mega_Emu

Este documento descreve a API do Mega_Emu que permite a comunicação entre o frontend React e o backend em C/C++.

## Visão Geral

A API do emulador é dividida em duas camadas principais:

1. **Camada de Comunicação**: WebSocket e REST API para comunicação entre frontend e backend
2. **Camada de Funções**: Interfaces de programação que expõem as funcionalidades do emulador

## Módulos Principais

### 1. Core do Emulador

#### 1.1. Gerenciamento do Ciclo de Vida

```c
/**
 * @brief Inicializa o emulador
 * @param config Ponteiro para estrutura de configuração
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_initialize(const emu_config_t* config);

/**
 * @brief Finaliza o emulador e libera recursos
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_shutdown();

/**
 * @brief Carrega uma ROM
 * @param path Caminho para o arquivo da ROM
 * @param auto_detect_platform Se verdadeiro, detecta automaticamente a plataforma
 * @param platform Plataforma a ser usada (ignorado se auto_detect_platform for verdadeiro)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_load_rom(const char* path, bool auto_detect_platform, emu_platform_t platform);

/**
 * @brief Inicia a emulação
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_start();

/**
 * @brief Pausa a emulação
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_pause();

/**
 * @brief Retoma a emulação após pausa
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_resume();

/**
 * @brief Reseta o sistema emulado
 * @param hard Se verdadeiro, realiza reset hard; caso contrário, reset soft
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_reset(bool hard);

/**
 * @brief Obtém o status atual do emulador
 * @param status Ponteiro para estrutura que receberá o status
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_get_status(emu_status_t* status);
```

#### 1.2. Configuração

```c
/**
 * @brief Define configuração do emulador
 * @param config Ponteiro para estrutura de configuração
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_set_config(const emu_config_t* config);

/**
 * @brief Obtém configuração atual do emulador
 * @param config Ponteiro para estrutura que receberá a configuração
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_get_config(emu_config_t* config);

/**
 * @brief Define opção de configuração específica
 * @param key Chave da opção
 * @param value Valor da opção
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_set_option(const char* key, const char* value);

/**
 * @brief Obtém opção de configuração específica
 * @param key Chave da opção
 * @param value Buffer para receber o valor
 * @param size Tamanho do buffer
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_get_option(const char* key, char* value, size_t size);
```

### 2. Vídeo e Rendering

```c
/**
 * @brief Obtém o próximo frame renderizado
 * @param frame_buffer Buffer para receber os dados do frame
 * @param width Ponteiro para variável que receberá a largura do frame
 * @param height Ponteiro para variável que receberá a altura do frame
 * @param format Formato de pixel desejado
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_video_get_frame(uint8_t* frame_buffer, int* width, int* height, emu_pixel_format_t format);

/**
 * @brief Configura parâmetros de vídeo
 * @param config Ponteiro para estrutura de configuração de vídeo
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_video_configure(const emu_video_config_t* config);

/**
 * @brief Captura screenshot
 * @param path Caminho onde salvar a imagem
 * @param format Formato da imagem (PNG, BMP, etc.)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_video_capture_screenshot(const char* path, emu_image_format_t format);

/**
 * @brief Obtém informações sobre o estado atual do vídeo
 * @param info Ponteiro para estrutura que receberá as informações
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_video_get_info(emu_video_info_t* info);
```

### 3. Áudio

```c
/**
 * @brief Configura parâmetros de áudio
 * @param config Ponteiro para estrutura de configuração de áudio
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_audio_configure(const emu_audio_config_t* config);

/**
 * @brief Obtém amostras de áudio
 * @param buffer Buffer para receber amostras de áudio
 * @param num_samples Número de amostras solicitadas
 * @param samples_read Ponteiro para variável que receberá o número real de amostras lidas
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_audio_get_samples(int16_t* buffer, size_t num_samples, size_t* samples_read);

/**
 * @brief Define volume de áudio
 * @param volume Valor do volume (0.0 - 1.0)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_audio_set_volume(float volume);

/**
 * @brief Ativa/desativa mudo
 * @param mute Se verdadeiro, ativa mudo; caso contrário, desativa
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_audio_set_mute(bool mute);
```

### 4. Entrada

```c
/**
 * @brief Define estado dos botões para um controlador
 * @param controller_id ID do controlador (0-7)
 * @param button_state Estado dos botões (máscara de bits)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_input_set_controller_state(int controller_id, uint32_t button_state);

/**
 * @brief Define estado de um botão específico
 * @param controller_id ID do controlador (0-7)
 * @param button Código do botão
 * @param pressed Se verdadeiro, botão pressionado; caso contrário, liberado
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_input_set_button_state(int controller_id, emu_button_t button, bool pressed);

/**
 * @brief Configura mapeamento de botões
 * @param mapping Ponteiro para estrutura de mapeamento
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_input_set_mapping(const emu_input_mapping_t* mapping);

/**
 * @brief Obtém mapeamento atual de botões
 * @param mapping Ponteiro para estrutura que receberá o mapeamento
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_input_get_mapping(emu_input_mapping_t* mapping);
```

### 5. Estados Salvos

```c
/**
 * @brief Salva o estado atual do emulador
 * @param slot Slot para salvar (0-9, -1 para auto)
 * @param description Descrição opcional do estado
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_state_save(int slot, const char* description);

/**
 * @brief Carrega um estado salvo
 * @param slot Slot para carregar (0-9)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_state_load(int slot);

/**
 * @brief Lista estados salvos disponíveis
 * @param states Array de estruturas para receber informações dos estados
 * @param max_states Tamanho máximo do array
 * @param num_states Ponteiro para variável que receberá o número real de estados
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_state_list(emu_state_info_t* states, int max_states, int* num_states);

/**
 * @brief Exporta um estado salvo para arquivo
 * @param slot Slot do estado a exportar
 * @param path Caminho do arquivo de destino
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_state_export(int slot, const char* path);

/**
 * @brief Importa um estado salvo de arquivo
 * @param path Caminho do arquivo de origem
 * @param slot Slot onde salvar o estado importado (0-9, -1 para auto)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_state_import(const char* path, int slot);
```

### 6. Depuração e Desenvolvimento

#### 6.1. Memória

```c
/**
 * @brief Lê dados da memória
 * @param address Endereço inicial
 * @param buffer Buffer para receber os dados
 * @param size Número de bytes a ler
 * @param domain Domínio de memória (CPU, VRAM, etc.)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_read_memory(uint32_t address, uint8_t* buffer, size_t size, emu_memory_domain_t domain);

/**
 * @brief Escreve dados na memória
 * @param address Endereço inicial
 * @param buffer Dados a escrever
 * @param size Número de bytes a escrever
 * @param domain Domínio de memória (CPU, VRAM, etc.)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_write_memory(uint32_t address, const uint8_t* buffer, size_t size, emu_memory_domain_t domain);

/**
 * @brief Adiciona um watchpoint na memória
 * @param watchpoint Ponteiro para estrutura que define o watchpoint
 * @param id Ponteiro para variável que receberá o ID do watchpoint
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_add_watchpoint(const emu_watchpoint_t* watchpoint, int* id);

/**
 * @brief Remove um watchpoint
 * @param id ID do watchpoint a remover
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_remove_watchpoint(int id);
```

#### 6.2. CPU

```c
/**
 * @brief Obtém estado atual da CPU
 * @param state Ponteiro para estrutura que receberá o estado
 * @param cpu_type Tipo de CPU
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_get_cpu_state(emu_cpu_state_t* state, emu_cpu_type_t cpu_type);

/**
 * @brief Define valor de registrador da CPU
 * @param cpu_type Tipo de CPU
 * @param reg_name Nome do registrador
 * @param value Valor a definir
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_set_register(emu_cpu_type_t cpu_type, const char* reg_name, uint32_t value);

/**
 * @brief Executa um passo da CPU
 * @param cpu_type Tipo de CPU
 * @param step_type Tipo de passo (instrução, ciclo, etc.)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_step_cpu(emu_cpu_type_t cpu_type, emu_step_type_t step_type);

/**
 * @brief Adiciona um breakpoint
 * @param breakpoint Ponteiro para estrutura que define o breakpoint
 * @param id Ponteiro para variável que receberá o ID do breakpoint
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_add_breakpoint(const emu_breakpoint_t* breakpoint, int* id);

/**
 * @brief Remove um breakpoint
 * @param id ID do breakpoint a remover
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_remove_breakpoint(int id);
```

#### 6.3. Sprites e Gráficos

```c
/**
 * @brief Obtém informações sobre sprites ativos
 * @param sprites Array de estruturas para receber informações dos sprites
 * @param max_sprites Tamanho máximo do array
 * @param num_sprites Ponteiro para variável que receberá o número real de sprites
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_get_sprites(emu_sprite_info_t* sprites, int max_sprites, int* num_sprites);

/**
 * @brief Obtém paletas de cores
 * @param palettes Array de estruturas para receber as paletas
 * @param max_palettes Tamanho máximo do array
 * @param num_palettes Ponteiro para variável que receberá o número real de paletas
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_get_palettes(emu_palette_t* palettes, int max_palettes, int* num_palettes);

/**
 * @brief Exporta sprite como imagem
 * @param sprite_id ID do sprite
 * @param path Caminho onde salvar a imagem
 * @param format Formato da imagem
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_export_sprite(int sprite_id, const char* path, emu_image_format_t format);
```

#### 6.4. Disassembly

```c
/**
 * @brief Descompila região de memória
 * @param address Endereço inicial
 * @param count Número de instruções a descompilar
 * @param instructions Array para receber instruções descompiladas
 * @param cpu_type Tipo de CPU
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_disassemble(uint32_t address, int count, emu_instruction_t* instructions, emu_cpu_type_t cpu_type);

/**
 * @brief Traduz endereço para nome simbólico, se disponível
 * @param address Endereço
 * @param symbol Buffer para receber o nome simbólico
 * @param size Tamanho do buffer
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int emu_debug_get_symbol(uint32_t address, char* symbol, size_t size);
```

## Estruturas de Dados Principais

### Configuração

```c
typedef struct {
    bool auto_frameskip;        // Se verdadeiro, ajusta frameskip automaticamente
    int frameskip;              // Número de frames a pular (0 = nenhum)
    bool vsync;                // Se verdadeiro, sincroniza com vsync do monitor
    bool throttle;             // Se verdadeiro, limita velocidade à original
    float speed_factor;        // Fator de velocidade (1.0 = normal)
    char rom_directory[256];   // Diretório padrão de ROMs
    char save_directory[256];  // Diretório para estados salvos
    bool rewind_enabled;       // Se verdadeiro, permite retroceder a emulação
    int rewind_buffer_size;    // Tamanho do buffer de rewind em MB
    emu_video_config_t video;  // Configuração de vídeo
    emu_audio_config_t audio;  // Configuração de áudio
    emu_input_config_t input;  // Configuração de entrada
} emu_config_t;
```

### Estado do Emulador

```c
typedef enum {
    EMU_STATUS_IDLE,      // Emulador ocioso
    EMU_STATUS_RUNNING,   // Emulador em execução
    EMU_STATUS_PAUSED,    // Emulador pausado
    EMU_STATUS_ERROR      // Erro no emulador
} emu_status_code_t;

typedef struct {
    emu_status_code_t code;         // Código de status
    char error_message[256];        // Mensagem de erro (se houver)
    emu_platform_t platform;        // Plataforma atual
    char rom_name[256];             // Nome da ROM carregada
    double fps;                     // FPS atual
    uint64_t frame_count;           // Contador de frames
    bool rom_loaded;                // Se ROM está carregada
    int current_save_slot;          // Slot de save atual
} emu_status_t;
```

### Vídeo

```c
typedef enum {
    EMU_PIXEL_FORMAT_RGB565,    // 16 bits por pixel, R5G6B5
    EMU_PIXEL_FORMAT_RGBA8888,  // 32 bits por pixel, R8G8B8A8
    EMU_PIXEL_FORMAT_RGB888     // 24 bits por pixel, R8G8B8
} emu_pixel_format_t;

typedef enum {
    EMU_SCALE_NEAREST,          // Escala nearest-neighbor (pixelado)
    EMU_SCALE_LINEAR,           // Escala linear (suavizado)
    EMU_SCALE_HQ2X,             // Algoritmo HQ2X
    EMU_SCALE_HQ3X,             // Algoritmo HQ3X
    EMU_SCALE_HQ4X              // Algoritmo HQ4X
} emu_scale_method_t;

typedef struct {
    int width;                  // Largura da janela
    int height;                 // Altura da janela
    bool fullscreen;            // Se verdadeiro, modo tela cheia
    emu_pixel_format_t format;  // Formato de pixel
    emu_scale_method_t scaling; // Método de escala
    bool bilinear;              // Se verdadeiro, usa filtragem bilinear
    bool aspect_correction;     // Se verdadeiro, mantém proporção original
    bool integer_scaling;       // Se verdadeiro, usa apenas multiplicadores inteiros
    int scanlines;              // Intensidade de scanlines (0-100, 0 = desativado)
    bool show_fps;              // Se verdadeiro, mostra FPS
} emu_video_config_t;
```

### Áudio

```c
typedef struct {
    int sample_rate;            // Taxa de amostragem (Hz)
    bool stereo;                // Se verdadeiro, áudio estéreo; caso contrário, mono
    int buffer_size;            // Tamanho do buffer em frames
    float volume;               // Volume (0.0 - 1.0)
    bool mute;                  // Se verdadeiro, sem áudio
    bool enable_effects;        // Se verdadeiro, habilita efeitos de áudio
} emu_audio_config_t;
```

### Entrada

```c
typedef enum {
    EMU_BUTTON_UP,
    EMU_BUTTON_DOWN,
    EMU_BUTTON_LEFT,
    EMU_BUTTON_RIGHT,
    EMU_BUTTON_A,
    EMU_BUTTON_B,
    EMU_BUTTON_C,
    EMU_BUTTON_X,
    EMU_BUTTON_Y,
    EMU_BUTTON_Z,
    EMU_BUTTON_START,
    EMU_BUTTON_SELECT,
    EMU_BUTTON_L,
    EMU_BUTTON_R,
    EMU_BUTTON_MODE,
    EMU_BUTTON_MAX
} emu_button_t;

typedef struct {
    char keyboard_key[32];      // Tecla de teclado mapeada
    int gamepad_button;         // Botão de gamepad mapeado
} emu_button_mapping_t;

typedef struct {
    emu_button_mapping_t button_mappings[EMU_BUTTON_MAX];
    bool auto_detect_gamepad;   // Se verdadeiro, detecta gamepads automaticamente
    int gamepad_deadzone;       // Zona morta para sticks analógicos (0-100)
} emu_input_config_t;
```

### Debugging

```c
typedef enum {
    EMU_MEM_CPU,    // Memória principal da CPU
    EMU_MEM_ROM,    // ROM
    EMU_MEM_VRAM,   // Memória de vídeo
    EMU_MEM_WRAM,   // Work RAM
    EMU_MEM_OAM,    // Object Attribute Memory
    EMU_MEM_IO,     // Registradores de I/O
    EMU_MEM_CART    // Memória do cartucho (ex: SRAM)
} emu_memory_domain_t;

typedef enum {
    EMU_CPU_6502,   // 6502/2A03 (NES)
    EMU_CPU_Z80,    // Z80 (Master System, Game Boy, Mega Drive)
    EMU_CPU_68000,  // 68000 (Mega Drive)
    EMU_CPU_65816,  // 65816 (SNES)
    EMU_CPU_ARM7    // ARM7TDMI (Game Boy Advance)
} emu_cpu_type_t;

typedef struct {
    emu_cpu_type_t cpu_type;                // Tipo de CPU
    char registers[32][16];                 // Valores de registradores como strings
    uint32_t register_values[32];           // Valores de registradores como inteiros
    uint32_t pc;                            // Program Counter
    uint32_t sp;                            // Stack Pointer
    uint32_t flags;                         // Flags combinadas
    char current_instruction[64];           // Instrução atual descompilada
    uint8_t current_opcode;                 // Opcode atual
    uint64_t cycle_count;                   // Contador de ciclos
    uint32_t next_pc;                       // Próximo valor de PC
} emu_cpu_state_t;

typedef struct {
    uint32_t address;                       // Endereço do breakpoint
    emu_cpu_type_t cpu_type;                // Tipo de CPU (para sistemas múltiplos)
    bool enabled;                           // Se habilitado
    bool temporary;                         // Se temporário (one-shot)
    char condition[256];                    // Condição de ativação (expressão)
    int hit_count;                          // Contador de hits
    int ignore_count;                       // Número de hits a ignorar
} emu_breakpoint_t;

typedef struct {
    uint32_t address;                       // Endereço do watchpoint
    size_t size;                            // Tamanho da região monitorada
    emu_memory_domain_t domain;             // Domínio de memória
    bool read;                              // Monitorar leituras
    bool write;                             // Monitorar escritas
    char condition[256];                    // Condição de ativação (expressão)
    bool enabled;                           // Se habilitado
} emu_watchpoint_t;

typedef struct {
    char disassembly[64];                   // Texto descompilado
    uint32_t address;                       // Endereço da instrução
    uint8_t bytes[16];                      // Bytes da instrução
    int byte_count;                         // Número de bytes na instrução
    uint32_t target_address;                // Endereço alvo (jumps/calls)
} emu_instruction_t;

typedef struct {
    int id;                                 // ID do sprite
    int x;                                  // Posição X
    int y;                                  // Posição Y
    int width;                              // Largura
    int height;                             // Altura
    int tile_index;                         // Índice do tile/padrão
    int palette_index;                      // Índice da paleta
    uint32_t attributes;                    // Atributos (depende da plataforma)
    bool visible;                           // Se visível
    bool priority;                          // Prioridade (depende da plataforma)
    bool flip_h;                            // Flip horizontal
    bool flip_v;                            // Flip vertical
} emu_sprite_info_t;
```

## Camada de Interface com WebSocket/REST

A camada de interface expõe a API C/C++ para comunicação WebSocket e REST.

### WebSocket Server

```c
/**
 * @brief Inicializa o servidor WebSocket
 * @param port Porta para o servidor
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int ws_server_init(uint16_t port);

/**
 * @brief Finaliza o servidor WebSocket
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int ws_server_shutdown();

/**
 * @brief Processa mensagens pendentes (chamado periodicamente)
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int ws_server_update();

/**
 * @brief Envia mensagem para cliente específico
 * @param client_id ID do cliente
 * @param message_type Tipo da mensagem
 * @param payload Dados da mensagem em formato JSON
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int ws_send_message(uint32_t client_id, const char* message_type, const char* payload);

/**
 * @brief Envia mensagem para todos os clientes conectados
 * @param message_type Tipo da mensagem
 * @param payload Dados da mensagem em formato JSON
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int ws_broadcast(const char* message_type, const char* payload);

/**
 * @brief Registra callback para tipo de mensagem
 * @param message_type Tipo da mensagem
 * @param callback Função callback
 * @param user_data Dados do usuário passados para o callback
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int ws_register_handler(const char* message_type, ws_message_handler_t callback, void* user_data);
```

### HTTP/REST Server

```c
/**
 * @brief Inicializa o servidor HTTP/REST
 * @param port Porta para o servidor
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int http_server_init(uint16_t port);

/**
 * @brief Finaliza o servidor HTTP/REST
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int http_server_shutdown();

/**
 * @brief Registra rota para método GET
 * @param path Caminho da rota
 * @param handler Função handler
 * @param user_data Dados do usuário passados para o handler
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int http_register_get(const char* path, http_handler_t handler, void* user_data);

/**
 * @brief Registra rota para método POST
 * @param path Caminho da rota
 * @param handler Função handler
 * @param user_data Dados do usuário passados para o handler
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int http_register_post(const char* path, http_handler_t handler, void* user_data);

/**
 * @brief Registra rota para método PUT
 * @param path Caminho da rota
 * @param handler Função handler
 * @param user_data Dados do usuário passados para o handler
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int http_register_put(const char* path, http_handler_t handler, void* user_data);

/**
 * @brief Registra rota para método DELETE
 * @param path Caminho da rota
 * @param handler Função handler
 * @param user_data Dados do usuário passados para o handler
 * @return 0 em caso de sucesso, código de erro em caso de falha
 */
int http_register_delete(const char* path, http_handler_t handler, void* user_data);
```

## Integração Frontend-Backend

### Exemplos de Integração

#### 1. Inicialização e Carregamento de ROM

```c
// Backend (C)
int handle_rom_load(http_request_t* req, http_response_t* res, void* user_data) {
    const char* rom_path = http_get_param(req, "path");
    if (!rom_path) {
        http_send_error(res, 400, "Missing path parameter");
        return -1;
    }

    int result = emu_load_rom(rom_path, true, EMU_PLATFORM_AUTO);
    if (result != 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Failed to load ROM: %s", emu_get_error_message(result));
        http_send_error(res, 500, error_msg);
        return -1;
    }

    // Obter informações da ROM
    emu_rom_info_t rom_info;
    emu_get_rom_info(&rom_info);

    // Criar resposta JSON
    json_object* response = json_object_new_object();
    json_object_object_add(response, "success", json_object_new_boolean(true));
    json_object_object_add(response, "name", json_object_new_string(rom_info.title));
    json_object_object_add(response, "platform", json_object_new_string(rom_info.platform_name));
    json_object_object_add(response, "size", json_object_new_int(rom_info.size));

    http_send_json(res, response);
    json_object_put(response);

    return 0;
}
```

```typescript
// Frontend (TypeScript)
export async function loadRom(path: string): Promise<RomInfo> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/rom/load?path=${encodeURIComponent(path)}`);

    if (!response.ok) {
      throw new Error(`Failed to load ROM: ${response.statusText}`);
    }

    const data = await response.json();

    if (!data.success) {
      throw new Error(data.error || 'Unknown error');
    }

    return {
      name: data.name,
      platform: data.platform,
      size: data.size
    };
  } catch (error) {
    console.error('Error loading ROM:', error);
    throw error;
  }
}
```

#### 2. Streaming de Frames via WebSocket

```c
// Backend (C)
void emulator_thread() {
    while (emulator_running) {
        // Executa um frame
        emu_run_frame();

        // Obtem o frame renderizado
        uint8_t frame_buffer[MAX_FRAME_SIZE];
        int width, height;
        emu_video_get_frame(frame_buffer, &width, &height, EMU_PIXEL_FORMAT_RGBA8888);

        // Codifica o frame para base64
        char* encoded_frame = base64_encode(frame_buffer, width * height * 4);

        // Cria payload JSON
        json_object* payload = json_object_new_object();
        json_object_object_add(payload, "width", json_object_new_int(width));
        json_object_object_add(payload, "height", json_object_new_int(height));
        json_object_object_add(payload, "format", json_object_new_string("rgba8888"));
        json_object_object_add(payload, "data", json_object_new_string(encoded_frame));
        json_object_object_add(payload, "frameNumber", json_object_new_int(frame_counter++));

        // Envia para todos os clientes
        ws_broadcast("VIDEO_FRAME", json_object_to_json_string(payload));

        // Limpa recursos
        json_object_put(payload);
        free(encoded_frame);

        // Limita taxa de frames se necessário
        if (throttle) {
            sleep_until_next_frame();
        }
    }
}
```

```typescript
// Frontend (TypeScript)
function setupFrameStreaming() {
  const canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
  const ctx = canvas.getContext('2d');

  if (!ctx) {
    console.error('Failed to get 2D context');
    return;
  }

  const socket = new WebSocket(`ws://${API_HOST}:${API_PORT}/ws`);

  socket.onopen = () => {
    console.log('WebSocket connected');

    // Enviar handshake
    socket.send(JSON.stringify({
      type: 'HANDSHAKE',
      payload: {
        clientId: generateClientId(),
        clientType: 'web-frontend',
        capabilities: ['video', 'input']
      }
    }));
  };

  socket.onmessage = (event) => {
    const message = JSON.parse(event.data);

    if (message.type === 'VIDEO_FRAME') {
      const { width, height, data, frameNumber } = message.payload;

      // Decodificar base64
      const binaryData = atob(data);
      const byteArray = new Uint8Array(binaryData.length);

      for (let i = 0; i < binaryData.length; i++) {
        byteArray[i] = binaryData.charCodeAt(i);
      }

      // Criar ImageData e renderizar no canvas
      const imageData = new ImageData(
        new Uint8ClampedArray(byteArray.buffer),
        width,
        height
      );

      ctx.putImageData(imageData, 0, 0);
    }
  };

  socket.onerror = (error) => {
    console.error('WebSocket error:', error);
  };

  socket.onclose = () => {
    console.log('WebSocket closed');
  };

  return socket;
}
```

#### 3. Enviando Comandos de Entrada

```typescript
// Frontend (TypeScript)
function setupInputHandling(socket: WebSocket) {
  const keyMap = {
    'ArrowUp': 'UP',
    'ArrowDown': 'DOWN',
    'ArrowLeft': 'LEFT',
    'ArrowRight': 'RIGHT',
    'z': 'A',
    'x': 'B',
    'c': 'C',
    'a': 'X',
    's': 'Y',
    'd': 'Z',
    'Enter': 'START',
    'ShiftRight': 'SELECT'
  };

  window.addEventListener('keydown', (event) => {
    const button = keyMap[event.key];
    if (button) {
      socket.send(JSON.stringify({
        type: 'INPUT_EVENT',
        payload: {
          controller: 0,
          button,
          pressed: true
        }
      }));

      event.preventDefault();
    }
  });

  window.addEventListener('keyup', (event) => {
    const button = keyMap[event.key];
    if (button) {
      socket.send(JSON.stringify({
        type: 'INPUT_EVENT',
        payload: {
          controller: 0,
          button,
          pressed: false
        }
      }));

      event.preventDefault();
    }
  });
}
```

```c
// Backend (C)
// Handler para eventos de input
int handle_input_event(uint32_t client_id, json_object* payload, void* user_data) {
    // Extrair dados do payload
    json_object* controller_obj;
    json_object* button_obj;
    json_object* pressed_obj;

    if (!json_object_object_get_ex(payload, "controller", &controller_obj) ||
        !json_object_object_get_ex(payload, "button", &button_obj) ||
        !json_object_object_get_ex(payload, "pressed", &pressed_obj)) {
        return -1; // Payload inválido
    }

    int controller_id = json_object_get_int(controller_obj);
    const char* button_str = json_object_get_string(button_obj);
    bool pressed = json_object_get_boolean(pressed_obj);

    // Mapear string do botão para enum
    emu_button_t button = str_to_button(button_str);

    // Definir estado do botão
    emu_input_set_button_state(controller_id, button, pressed);

    return 0;
}
```

## Códigos de Erro

| Código | Nome | Descrição |
|--------|------|-----------|
| 0 | EMU_ERROR_NONE | Sem erro |
| -1 | EMU_ERROR_GENERIC | Erro genérico |
| -2 | EMU_ERROR_NOT_INITIALIZED | Emulador não inicializado |
| -3 | EMU_ERROR_ALREADY_INITIALIZED | Emulador já inicializado |
| -4 | EMU_ERROR_ROM_LOAD | Erro ao carregar ROM |
| -5 | EMU_ERROR_ROM_NOT_FOUND | ROM não encontrada |
| -6 | EMU_ERROR_ROM_INVALID | ROM inválida |
| -7 | EMU_ERROR_ROM_UNSUPPORTED | ROM não suportada |
| -8 | EMU_ERROR_MEMORY_ALLOC | Falha na alocação de memória |
| -9 | EMU_ERROR_INVALID_PARAMETER | Parâmetro inválido |
| -10 | EMU_ERROR_STATE_SAVE | Erro ao salvar estado |
| -11 | EMU_ERROR_STATE_LOAD | Erro ao carregar estado |
| -12 | EMU_ERROR_FEATURE_UNSUPPORTED | Funcionalidade não suportada |
| -13 | EMU_ERROR_TIMEOUT | Operação expirou |
| -14 | EMU_ERROR_PERMISSION | Permissão negada |
| -15 | EMU_ERROR_NETWORK | Erro de rede |
| -16 | EMU_ERROR_INTERNAL | Erro interno |

## Requisitos e Considerações

### Desempenho

- A API é projetada para ser eficiente em termos de CPU e memória
- Otimizações específicas para streaming de vídeo e áudio
- Suporte a multi-threading para melhor desempenho
- Throttling adaptativo para equilibrar desempenho e carga do sistema

### Compatibilidade

- Suporte para Windows, Linux e macOS
- Interface consistente entre plataformas
- Suporte a diferentes arquiteturas (x86, ARM)
- C ABI estável para facilitar ligação com diferentes linguagens

### Segurança

- Validação rigorosa de entrada
- Sandboxing das operações do emulador
- Rate limiting para proteger contra abuso
- Verificação de limites de memória e recursos

## Recursos Adicionais

### Documentação e Exemplos

- Documentação API completa (Doxygen): `/docs/api/doxygen/`
- Exemplos de código: `/examples/`
- Tutoriais: `/docs/tutorials/`
- Referência de protocolo WebSocket: `/docs/api/websocket-protocol.md`

### Ferramentas de Diagnóstico

- Testes de unidade da API: `/tests/api/`
- Ferramenta de benchmark: `/tools/benchmark/`
- Ferramentas de depuração: `/tools/debug/`

## Apêndice: Suporte Multi-plataforma

### Considerações Específicas por Plataforma

#### NES (Nintendo Entertainment System)

- CPU: 6502 (2A03)
- Memória: 2KB RAM, 2KB VRAM
- Sprites: 64 sprites 8x8 ou 8x16
- PPU: 256x240 pixels, 52 cores

#### Mega Drive / Genesis

- CPUs: Motorola 68000 (principal) e Z80 (som)
- Memória: 64KB RAM, 64KB VRAM, 8KB Z80 RAM
- VDP: 320x224/240 pixels, 512 cores
- Sprites: 80 sprites 8x8 a 32x32

#### Master System

- CPU: Zilog Z80
- Memória: 8KB RAM, 16KB VRAM
- VDP: 256x192/224/240 pixels
- Sprites: 64 sprites 8x8 ou 8x16

#### SNES (Super Nintendo)

- CPU: 65C816
- Memória: 128KB RAM, 64KB VRAM
- PPU: 256x224/239 pixels, 32768 cores
- Sprites: 128 sprites 8x8 a 64x64
