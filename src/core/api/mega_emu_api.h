/**
 * @file mega_emu_api.h
 * @brief API pública do emulador Mega_Emu
 */

#ifndef MEGA_EMU_API_H
#define MEGA_EMU_API_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tipos de plataformas/consoles suportados
 */
typedef enum {
  MEGA_EMU_PLATFORM_UNKNOWN = 0,
  MEGA_EMU_PLATFORM_MEGADRIVE,     // Mega Drive / Genesis
  MEGA_EMU_PLATFORM_MASTERSYSTEM,  // Master System
  MEGA_EMU_PLATFORM_GAMEGEAR,      // Game Gear
  MEGA_EMU_PLATFORM_NES,           // Nintendo Entertainment System
  MEGA_EMU_PLATFORM_SNES,          // Super Nintendo Entertainment System
  MEGA_EMU_PLATFORM_GAMEBOY,       // Game Boy
  MEGA_EMU_PLATFORM_GAMEBOY_COLOR, // Game Boy Color
  MEGA_EMU_PLATFORM_32X,           // Sega 32X
  MEGA_EMU_PLATFORM_SEGACD,        // Sega CD / Mega CD
  MEGA_EMU_PLATFORM_COUNT          // Número total de plataformas
} mega_emu_platform_t;

/**
 * @brief Tipos de regiões de console
 */
typedef enum {
  MEGA_EMU_REGION_AUTO = 0, // Detecção automática
  MEGA_EMU_REGION_NTSC_US,  // NTSC (EUA/Canadá) - 60Hz
  MEGA_EMU_REGION_NTSC_JPN, // NTSC (Japão) - 60Hz
  MEGA_EMU_REGION_PAL,      // PAL (Europa, Austrália) - 50Hz
  MEGA_EMU_REGION_PAL_BR,   // PAL-M (Brasil) - 60Hz
  MEGA_EMU_REGION_NTSC_KOR, // NTSC (Coreia) - 60Hz
  MEGA_EMU_REGION_COUNT     // Número total de regiões
} mega_emu_region_t;

/**
 * @brief Modos de exibição de vídeo
 */
typedef enum {
  MEGA_EMU_VIDEO_MODE_ORIGINAL = 0,  // Resolução original/nativa
  MEGA_EMU_VIDEO_MODE_UPSCALED,      // Upscaling por múltiplos inteiros
  MEGA_EMU_VIDEO_MODE_FULLSCREEN,    // Tela cheia com proporção mantida
  MEGA_EMU_VIDEO_MODE_STRETCHED,     // Esticado para preencher a tela
  MEGA_EMU_VIDEO_MODE_PIXEL_PERFECT, // Pixels perfeitos com upscaling
  MEGA_EMU_VIDEO_MODE_COUNT          // Número total de modos
} mega_emu_video_mode_t;

/**
 * @brief Estados de emulação
 */
typedef enum {
  MEGA_EMU_STATE_STOPPED = 0, // Emulação parada
  MEGA_EMU_STATE_RUNNING,     // Emulação em execução
  MEGA_EMU_STATE_PAUSED,      // Emulação pausada
  MEGA_EMU_STATE_DEBUG,       // Modo de depuração
  MEGA_EMU_STATE_ERROR,       // Estado de erro
  MEGA_EMU_STATE_COUNT        // Número total de estados
} mega_emu_state_t;

/**
 * @brief Configurações de áudio
 */
typedef struct {
  uint32_t sample_rate;    // Taxa de amostragem (Hz)
  uint32_t buffer_size;    // Tamanho do buffer (amostras)
  uint8_t channels;        // Número de canais (1=mono, 2=estéreo)
  uint8_t bits_per_sample; // Bits por amostra (8, 16)
  bool enable_lowpass;     // Habilitar filtro passa-baixa
  float lowpass_cutoff;    // Frequência de corte do filtro (Hz)
  uint8_t volume;          // Volume (0-100)
  bool enable_resampling;  // Habilitar resampling de alta qualidade
} mega_emu_audio_config_t;

/**
 * @brief Configurações de vídeo
 */
typedef struct {
  uint32_t width;               // Largura da janela (pixels)
  uint32_t height;              // Altura da janela (pixels)
  mega_emu_video_mode_t mode;   // Modo de exibição
  bool vsync;                   // Sincronização vertical
  bool fullscreen;              // Modo tela cheia
  bool bilinear_filter;         // Filtro bilinear
  bool enable_scanlines;        // Habilitar scanlines
  uint8_t scanline_intensity;   // Intensidade das scanlines (0-100)
  bool enable_shader;           // Habilitar shader personalizado
  char shader_path[256];        // Caminho para o shader
  bool maintain_aspect_ratio;   // Manter proporção da tela
  bool integer_scaling;         // Escala apenas por valores inteiros
  bool enable_crt_effect;       // Habilitar efeito CRT
  bool enable_ghosting;         // Habilitar efeito de ghosting
  bool enable_color_correction; // Habilitar correção de cores
} mega_emu_video_config_t;

/**
 * @brief Tipos de dispositivos de entrada
 */
typedef enum {
  MEGA_EMU_INPUT_DEVICE_NONE = 0,    // Nenhum dispositivo
  MEGA_EMU_INPUT_DEVICE_GAMEPAD,     // Gamepad/Controle padrão
  MEGA_EMU_INPUT_DEVICE_ZAPPER,      // NES Zapper / Light Gun
  MEGA_EMU_INPUT_DEVICE_KEYBOARD,    // Teclado
  MEGA_EMU_INPUT_DEVICE_MOUSE,       // Mouse
  MEGA_EMU_INPUT_DEVICE_LIGHTPHASER, // Light Phaser (SMS)
  MEGA_EMU_INPUT_DEVICE_PADDLE,      // Paddle/Dial
  MEGA_EMU_INPUT_DEVICE_MULTITAP,    // Multitap (4+ jogadores)
  MEGA_EMU_INPUT_DEVICE_COUNT        // Número total de dispositivos
} mega_emu_input_device_t;

/**
 * @brief Configuração do emulador
 */
typedef struct {
  mega_emu_platform_t platform;             // Plataforma emulada
  mega_emu_region_t region;                 // Região do console
  mega_emu_audio_config_t audio;            // Configurações de áudio
  mega_emu_video_config_t video;            // Configurações de vídeo
  mega_emu_input_device_t input_devices[4]; // Dispositivos por porta
  char rom_path[512];                       // Caminho da ROM
  char save_path[512];                      // Diretório para saves
  char screenshot_path[512];                // Diretório para capturas de tela
  bool enable_rewind;                       // Habilitar recurso de rebobinar
  uint32_t rewind_buffer_size;   // Tamanho do buffer de rebobinagem (MB)
  bool enable_cheats;            // Habilitar sistema de cheats
  bool enable_turbo;             // Habilitar sistema de turbo
  bool auto_save_sram;           // Salvar SRAM automaticamente
  bool enable_rom_database;      // Habilitar banco de dados de ROMs
  bool enable_savestates;        // Habilitar gravação de estados
  char rom_database_path[512];   // Caminho para o banco de dados de ROMs
  bool stretch_to_fit;           // Esticar para ajustar à tela
  uint8_t fast_forward_speed;    // Multiplicador de avanço rápido
  bool enable_audio_channels[8]; // Habilitar/desabilitar canais de áudio
} mega_emu_config_t;

/**
 * @brief Informações sobre o emulador
 */
typedef struct {
  char version[32];              // Versão do emulador
  char build_date[32];           // Data da compilação
  char build_commit[64];         // Commit/hash da versão
  char supported_platforms[256]; // Plataformas suportadas
  uint32_t features;             // Flags de recursos (bitfield)
  char cpu_features[128];        // Recursos da CPU (SSE, AVX, etc.)
  char gpu_renderer[64];         // Tipo de renderização gráfica
  uint32_t max_rewind_frames;    // Número máximo de frames para rewind
  uint8_t api_version_major;     // Versão maior da API
  uint8_t api_version_minor;     // Versão menor da API
  uint8_t api_version_patch;     // Versão de patch da API
} mega_emu_info_t;

/**
 * @brief Callback para áudio
 *
 * @param buffer Buffer de áudio a ser preenchido
 * @param frames Número de frames a processar
 * @param user_data Dados do usuário
 */
typedef void (*mega_emu_audio_callback_t)(int16_t *buffer, uint32_t frames,
                                          void *user_data);

/**
 * @brief Callback para vídeo
 *
 * @param buffer Buffer de vídeo a ser desenhado
 * @param width Largura do frame
 * @param height Altura do frame
 * @param pitch Pitch do buffer (bytes por linha)
 * @param user_data Dados do usuário
 */
typedef void (*mega_emu_video_callback_t)(const void *buffer, uint32_t width,
                                          uint32_t height, uint32_t pitch,
                                          void *user_data);

/**
 * @brief Callback para input
 *
 * @param port Número da porta (0-3)
 * @param device Tipo de dispositivo
 * @param state Ponteiro para receber o estado dos botões
 * @param user_data Dados do usuário
 */
typedef void (*mega_emu_input_callback_t)(uint8_t port,
                                          mega_emu_input_device_t device,
                                          uint32_t *state, void *user_data);

/**
 * @brief Callback para eventos de log
 *
 * @param level Nível de log (0=debug, 1=info, 2=warning, 3=error)
 * @param message Mensagem de log
 * @param user_data Dados do usuário
 */
typedef void (*mega_emu_log_callback_t)(uint8_t level, const char *message,
                                        void *user_data);

/**
 * @brief Callback para eventos do emulador
 *
 * @param event_type Tipo de evento
 * @param param1 Primeiro parâmetro (específico do evento)
 * @param param2 Segundo parâmetro (específico do evento)
 * @param user_data Dados do usuário
 */
typedef void (*mega_emu_event_callback_t)(uint32_t event_type, uint32_t param1,
                                          uint32_t param2, void *user_data);

/**
 * @brief Callback de progresso
 *
 * @param current Valor atual
 * @param total Valor total
 * @param description Descrição da operação
 * @param user_data Dados do usuário
 * @return bool true para continuar, false para cancelar
 */
typedef bool (*mega_emu_progress_callback_t)(uint32_t current, uint32_t total,
                                             const char *description,
                                             void *user_data);

/**
 * @brief Inicializa o emulador
 *
 * @param config Configuração do emulador
 * @return bool true se inicializado com sucesso, false caso contrário
 */
bool mega_emu_init(const mega_emu_config_t *config);

/**
 * @brief Finaliza o emulador e libera recursos
 */
void mega_emu_shutdown(void);

/**
 * @brief Carrega uma ROM
 *
 * @param rom_path Caminho para o arquivo de ROM
 * @param auto_detect_platform Detectar automaticamente a plataforma
 * @param out_platform Ponteiro para receber a plataforma detectada
 * @return bool true se carregado com sucesso, false caso contrário
 */
bool mega_emu_load_rom(const char *rom_path, bool auto_detect_platform,
                       mega_emu_platform_t *out_platform);

/**
 * @brief Descarrega a ROM atual
 */
void mega_emu_unload_rom(void);

/**
 * @brief Executa um frame de emulação
 *
 * @return bool true se executado com sucesso, false caso contrário
 */
bool mega_emu_run_frame(void);

/**
 * @brief Pausa/retoma a emulação
 *
 * @param paused true para pausar, false para retomar
 */
void mega_emu_set_pause(bool paused);

/**
 * @brief Verifica se a emulação está pausada
 *
 * @return bool true se pausado, false caso contrário
 */
bool mega_emu_is_paused(void);

/**
 * @brief Reseta o sistema emulado
 *
 * @param hard_reset true para reset completo, false para reset suave
 * @return bool true se resetado com sucesso, false caso contrário
 */
bool mega_emu_reset(bool hard_reset);

/**
 * @brief Obtém o estado atual do emulador
 *
 * @return mega_emu_state_t Estado atual
 */
mega_emu_state_t mega_emu_get_state(void);

/**
 * @brief Registra callback para áudio
 *
 * @param callback Função de callback
 * @param user_data Dados do usuário
 * @return int ID do callback ou -1 em caso de erro
 */
int mega_emu_register_audio_callback(mega_emu_audio_callback_t callback,
                                     void *user_data);

/**
 * @brief Registra callback para vídeo
 *
 * @param callback Função de callback
 * @param user_data Dados do usuário
 * @return int ID do callback ou -1 em caso de erro
 */
int mega_emu_register_video_callback(mega_emu_video_callback_t callback,
                                     void *user_data);

/**
 * @brief Registra callback para input
 *
 * @param callback Função de callback
 * @param user_data Dados do usuário
 * @return int ID do callback ou -1 em caso de erro
 */
int mega_emu_register_input_callback(mega_emu_input_callback_t callback,
                                     void *user_data);

/**
 * @brief Registra callback para eventos de log
 *
 * @param callback Função de callback
 * @param level_mask Máscara de níveis (bits 0-3 para debug, info, warning,
 * error)
 * @param user_data Dados do usuário
 * @return int ID do callback ou -1 em caso de erro
 */
int mega_emu_register_log_callback(mega_emu_log_callback_t callback,
                                   uint8_t level_mask, void *user_data);

/**
 * @brief Registra callback para eventos do emulador
 *
 * @param callback Função de callback
 * @param event_mask Máscara de eventos
 * @param user_data Dados do usuário
 * @return int ID do callback ou -1 em caso de erro
 */
int mega_emu_register_event_callback(mega_emu_event_callback_t callback,
                                     uint32_t event_mask, void *user_data);

/**
 * @brief Remove um callback registrado
 *
 * @param callback_type Tipo de callback (0=áudio, 1=vídeo, 2=input, 3=log,
 * 4=evento)
 * @param callback_id ID do callback
 * @return bool true se removido com sucesso, false caso contrário
 */
bool mega_emu_unregister_callback(uint8_t callback_type, int callback_id);

/**
 * @brief Salva estado da emulação
 *
 * @param slot Slot para salvar (0-9, -1 para automático)
 * @param description Descrição do save state
 * @return bool true se salvo com sucesso, false caso contrário
 */
bool mega_emu_save_state(int slot, const char *description);

/**
 * @brief Carrega estado da emulação
 *
 * @param slot Slot para carregar (0-9)
 * @return bool true se carregado com sucesso, false caso contrário
 */
bool mega_emu_load_state(int slot);

/**
 * @brief Salva estado da emulação em um arquivo
 *
 * @param filename Caminho do arquivo
 * @param description Descrição do save state
 * @param include_thumbnail Incluir thumbnail
 * @param compress Comprimir o arquivo
 * @return bool true se salvo com sucesso, false caso contrário
 */
bool mega_emu_save_state_file(const char *filename, const char *description,
                              bool include_thumbnail, bool compress);

/**
 * @brief Carrega estado da emulação de um arquivo
 *
 * @param filename Caminho do arquivo
 * @return bool true se carregado com sucesso, false caso contrário
 */
bool mega_emu_load_state_file(const char *filename);

/**
 * @brief Rebobina a emulação
 *
 * @param frames Número de frames para rebobinar
 * @return bool true se rebobinado com sucesso, false caso contrário
 */
bool mega_emu_rewind(uint32_t frames);

/**
 * @brief Captura a tela atual
 *
 * @param filename Caminho do arquivo ou NULL para usar o padrão
 * @param format Formato (0=PNG, 1=JPEG, 2=BMP)
 * @return bool true se capturado com sucesso, false caso contrário
 */
bool mega_emu_capture_screenshot(const char *filename, uint8_t format);

/**
 * @brief Obtém informações sobre o emulador
 *
 * @param info Ponteiro para receber as informações
 * @return bool true se obtido com sucesso, false caso contrário
 */
bool mega_emu_get_info(mega_emu_info_t *info);

/**
 * @brief Configura o nível de volume
 *
 * @param volume Nível de volume (0-100)
 */
void mega_emu_set_volume(uint8_t volume);

/**
 * @brief Habilita/desabilita canal de áudio
 *
 * @param channel Número do canal
 * @param enabled true para habilitar, false para desabilitar
 * @return bool true se configurado com sucesso, false caso contrário
 */
bool mega_emu_set_audio_channel_enabled(uint8_t channel, bool enabled);

/**
 * @brief Define a velocidade de avanço rápido
 *
 * @param multiplier Multiplicador de velocidade (2-10)
 */
void mega_emu_set_fast_forward_speed(uint8_t multiplier);

/**
 * @brief Ativa/desativa o modo de avanço rápido
 *
 * @param enabled true para ativar, false para desativar
 */
void mega_emu_set_fast_forward(bool enabled);

/**
 * @brief Aplica configurações de vídeo
 *
 * @param config Novas configurações
 * @return bool true se aplicado com sucesso, false caso contrário
 */
bool mega_emu_apply_video_config(const mega_emu_video_config_t *config);

/**
 * @brief Aplica configurações de áudio
 *
 * @param config Novas configurações
 * @return bool true se aplicado com sucesso, false caso contrário
 */
bool mega_emu_apply_audio_config(const mega_emu_audio_config_t *config);

/**
 * @brief Define configuração para uma porta de controle
 *
 * @param port Número da porta (0-3)
 * @param device Tipo de dispositivo
 * @return bool true se configurado com sucesso, false caso contrário
 */
bool mega_emu_set_input_device(uint8_t port, mega_emu_input_device_t device);

/**
 * @brief Define o estado dos botões para uma porta
 *
 * @param port Número da porta (0-3)
 * @param state Estado dos botões (bitfield)
 * @return bool true se definido com sucesso, false caso contrário
 */
bool mega_emu_set_input_state(uint8_t port, uint32_t state);

/**
 * @brief Obtém o estado dos botões para uma porta
 *
 * @param port Número da porta (0-3)
 * @param state Ponteiro para receber o estado
 * @return bool true se obtido com sucesso, false caso contrário
 */
bool mega_emu_get_input_state(uint8_t port, uint32_t *state);

/**
 * @brief Carrega um cheat
 *
 * @param cheat_code Código do cheat
 * @param description Descrição do cheat
 * @param enabled Estado inicial (habilitado/desabilitado)
 * @return int ID do cheat ou -1 em caso de erro
 */
int mega_emu_add_cheat(const char *cheat_code, const char *description,
                       bool enabled);

/**
 * @brief Habilita/desabilita um cheat
 *
 * @param cheat_id ID do cheat
 * @param enabled Estado desejado
 * @return bool true se configurado com sucesso, false caso contrário
 */
bool mega_emu_set_cheat_enabled(int cheat_id, bool enabled);

/**
 * @brief Remove um cheat
 *
 * @param cheat_id ID do cheat
 * @return bool true se removido com sucesso, false caso contrário
 */
bool mega_emu_remove_cheat(int cheat_id);

/**
 * @brief Remove todos os cheats
 */
void mega_emu_clear_cheats(void);

/**
 * @brief Pesquisa no banco de dados de ROMs
 *
 * @param query String de pesquisa
 * @param platform Plataforma específica ou -1 para todas
 * @param results Buffer para receber resultados
 * @param max_results Tamanho máximo do buffer
 * @param actual_results Ponteiro para receber o número de resultados
 * encontrados
 * @return bool true se pesquisado com sucesso, false caso contrário
 */
bool mega_emu_search_rom_database(const char *query, int platform,
                                  void *results, uint32_t max_results,
                                  uint32_t *actual_results);

/**
 * @brief Obtém informações de uma ROM pelo hash
 *
 * @param hash_type Tipo de hash (0=CRC32, 1=MD5, 2=SHA1)
 * @param hash String do hash
 * @param info Buffer para receber informações
 * @return bool true se encontrado, false caso contrário
 */
bool mega_emu_get_rom_info_by_hash(uint8_t hash_type, const char *hash,
                                   void *info);

/**
 * @brief Calcula hashes para um arquivo de ROM
 *
 * @param rom_path Caminho para o arquivo
 * @param crc32 Buffer para receber CRC32 (8 caracteres + nulo)
 * @param md5 Buffer para receber MD5 (32 caracteres + nulo)
 * @param sha1 Buffer para receber SHA1 (40 caracteres + nulo)
 * @param callback Callback de progresso (opcional)
 * @param user_data Dados do usuário para o callback
 * @return bool true se calculado com sucesso, false caso contrário
 */
bool mega_emu_calculate_rom_hashes(const char *rom_path, char *crc32, char *md5,
                                   char *sha1,
                                   mega_emu_progress_callback_t callback,
                                   void *user_data);

/**
 * @brief Configura turbo para um botão
 *
 * @param port Porta do controle (0-3)
 * @param button_mask Máscara de bit do botão
 * @param enabled Estado (ativo/inativo)
 * @param frequency Frequência em Hz (5-30)
 * @return bool true se configurado com sucesso, false caso contrário
 */
bool mega_emu_set_turbo_button(uint8_t port, uint32_t button_mask, bool enabled,
                               uint8_t frequency);

/**
 * @brief Verifica se um botão tem turbo configurado
 *
 * @param port Porta do controle (0-3)
 * @param button_mask Máscara de bit do botão
 * @param frequency Ponteiro para receber a frequência
 * @return bool true se o botão tem turbo, false caso contrário
 */
bool mega_emu_is_turbo_button(uint8_t port, uint32_t button_mask,
                              uint8_t *frequency);

/**
 * @brief Remove configuração de turbo para um botão
 *
 * @param port Porta do controle (0-3)
 * @param button_mask Máscara de bit do botão
 * @return bool true se removido com sucesso, false caso contrário
 */
bool mega_emu_remove_turbo_button(uint8_t port, uint32_t button_mask);

/**
 * @brief Limpa todas as configurações de turbo
 */
void mega_emu_clear_turbo_buttons(void);

/**
 * @brief Define o modo de turbo para um botão
 *
 * @param port Porta do controle (0-3)
 * @param button_mask Máscara de bit do botão
 * @param mode Modo (0=Toggle, 1=Pulse, 2=Hold)
 * @return bool true se configurado com sucesso, false caso contrário
 */
bool mega_emu_set_turbo_mode(uint8_t port, uint32_t button_mask, uint8_t mode);

/**
 * @brief Define o ciclo de trabalho (duty cycle) para um botão com turbo
 *
 * @param port Porta do controle (0-3)
 * @param button_mask Máscara de bit do botão
 * @param duty_cycle Ciclo de trabalho (1-99%)
 * @return bool true se configurado com sucesso, false caso contrário
 */
bool mega_emu_set_turbo_duty_cycle(uint8_t port, uint32_t button_mask,
                                   uint8_t duty_cycle);

/**
 * @brief Salva configurações de turbo em um arquivo
 *
 * @param filename Caminho do arquivo
 * @return bool true se salvo com sucesso, false caso contrário
 */
bool mega_emu_save_turbo_config(const char *filename);

/**
 * @brief Carrega configurações de turbo de um arquivo
 *
 * @param filename Caminho do arquivo
 * @return bool true se carregado com sucesso, false caso contrário
 */
bool mega_emu_load_turbo_config(const char *filename);

#ifdef __cplusplus
}
#endif

#endif // MEGA_EMU_API_H
