/**
 * @file sdl_video_recorder.h
 * @brief Sistema de gravação de vídeo para o frontend SDL
 */
#ifndef SDL_VIDEO_RECORDER_H
#define SDL_VIDEO_RECORDER_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "sdl_game_renderer.h"

/**
 * @brief Tamanho máximo do caminho de arquivo
 */
#define SDL_VIDEO_RECORDER_MAX_PATH 256

/**
 * @brief Formatos de vídeo suportados
 */
typedef enum {
    SDL_VIDEO_FORMAT_MP4,   ///< Formato MP4 (H.264)
    SDL_VIDEO_FORMAT_AVI,   ///< Formato AVI (não comprimido)
    SDL_VIDEO_FORMAT_GIF    ///< Formato GIF animado (sem áudio)
} sdl_video_format_t;

/**
 * @brief Qualidade de gravação de vídeo
 */
typedef enum {
    SDL_VIDEO_QUALITY_LOW,     ///< Baixa qualidade (menor arquivo)
    SDL_VIDEO_QUALITY_MEDIUM,  ///< Qualidade média
    SDL_VIDEO_QUALITY_HIGH     ///< Alta qualidade (maior arquivo)
} sdl_video_quality_t;

/**
 * @brief Configuração do sistema de gravação de vídeo
 */
typedef struct {
    char output_dir[SDL_VIDEO_RECORDER_MAX_PATH];  ///< Diretório de saída para vídeos
    sdl_video_format_t format;                     ///< Formato do vídeo
    sdl_video_quality_t quality;                   ///< Qualidade do vídeo
    uint32_t fps;                                  ///< Frames por segundo do vídeo (tipicamente 30 ou 60)
    bool include_audio;                            ///< Incluir áudio na gravação
    bool include_timestamp;                        ///< Incluir timestamp no nome do arquivo
    bool include_game_name;                        ///< Incluir nome do jogo no nome do arquivo
    bool show_notification;                        ///< Mostrar notificação durante gravação
    bool show_indicator;                           ///< Mostrar indicador de gravação na tela
    uint32_t max_duration_seconds;                 ///< Duração máxima de gravação em segundos (0 = sem limite)
    uint32_t max_file_size_mb;                     ///< Tamanho máximo do arquivo em MB (0 = sem limite)
    int bitrate_kbps;                              ///< Taxa de bits em kbps (0 = automático)
} sdl_video_recorder_config_t;

/**
 * @brief Estrutura para notificação de gravação
 */
typedef struct {
    bool visible;                     ///< Se a notificação está visível
    Uint32 start_time;                ///< Tempo de início da notificação
    Uint32 duration;                  ///< Duração da notificação em ms
    SDL_Texture *message;             ///< Textura da mensagem
    SDL_Texture *icon;                ///< Textura do ícone
    SDL_Rect position;                ///< Posição e tamanho da notificação
} sdl_video_recorder_notification_t;

/**
 * @brief Estrutura para indicador de gravação
 */
typedef struct {
    bool visible;                     ///< Se o indicador está visível
    SDL_Texture *icon;                ///< Textura do ícone
    SDL_Rect position;                ///< Posição e tamanho do indicador
    Uint32 blink_interval;            ///< Intervalo do piscar em ms
    Uint32 last_blink_time;           ///< Tempo do último piscar
    bool blink_state;                 ///< Estado atual do piscar (visible/hidden)
    Uint32 recording_duration;        ///< Duração da gravação atual em segundos
    SDL_Texture *duration_text;       ///< Textura para mostrar a duração
    SDL_Rect duration_position;       ///< Posição do texto de duração
} sdl_video_recorder_indicator_t;

/**
 * @brief Estatísticas de gravação
 */
typedef struct {
    uint32_t total_frames;            ///< Número total de frames gravados
    uint32_t dropped_frames;          ///< Número de frames descartados
    uint32_t current_bitrate;         ///< Taxa de bits atual em kbps
    uint32_t file_size_kb;            ///< Tamanho atual do arquivo em KB
    uint32_t duration_seconds;        ///< Duração atual em segundos
} sdl_video_recorder_stats_t;

/**
 * @brief Estado interno de gravação
 */
typedef struct {
    void *ffmpeg_context;             ///< Contexto de codificação (específico da implementação)
    void *audio_context;              ///< Contexto de processamento de áudio
    uint8_t *frame_buffer;            ///< Buffer temporário para processamento de frames
    int frame_buffer_size;            ///< Tamanho do buffer de frames
    SDL_Thread *encoder_thread;       ///< Thread dedicada para codificação
    SDL_mutex *mutex;                 ///< Mutex para sincronização
    SDL_cond *frame_ready_cond;       ///< Condição para sinalizar frame pronto
    bool encoder_running;             ///< Flag indicando se o codificador está rodando
    bool frame_ready;                 ///< Flag indicando se há frame pronto para codificar
    uint32_t start_time;              ///< Tempo de início da gravação em ms
    uint32_t last_frame_time;         ///< Tempo do último frame capturado
    char output_file[SDL_VIDEO_RECORDER_MAX_PATH]; ///< Arquivo de saída atual
} sdl_video_recorder_internal_t;

/**
 * @brief Estrutura principal do sistema de gravação de vídeo
 */
typedef struct {
    bool initialized;                                  ///< Se o sistema está inicializado
    bool recording;                                    ///< Se está gravando atualmente
    sdl_game_renderer_t *renderer;                     ///< Renderizador do jogo
    char current_game_name[SDL_VIDEO_RECORDER_MAX_PATH]; ///< Nome do jogo atual
    time_t last_recording_time;                        ///< Timestamp da última gravação
    char last_recording_path[SDL_VIDEO_RECORDER_MAX_PATH]; ///< Caminho do último vídeo

    sdl_video_recorder_config_t config;                ///< Configuração atual
    sdl_video_recorder_notification_t notification;    ///< Notificação na tela
    sdl_video_recorder_indicator_t indicator;          ///< Indicador de gravação
    sdl_video_recorder_stats_t stats;                  ///< Estatísticas da gravação atual
    sdl_video_recorder_internal_t internal;            ///< Estado interno da codificação
} sdl_video_recorder_t;

/**
 * @brief Inicializa o sistema de gravação de vídeo
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param renderer Ponteiro para o renderizador do jogo
 * @return true Se inicializado com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_init(sdl_video_recorder_t *recorder, sdl_game_renderer_t *renderer);

/**
 * @brief Finaliza o sistema de gravação de vídeo
 *
 * @param recorder Ponteiro para a estrutura de gravação
 */
void sdl_video_recorder_shutdown(sdl_video_recorder_t *recorder);

/**
 * @brief Inicia a gravação de vídeo
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @return true Se iniciou com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_start(sdl_video_recorder_t *recorder);

/**
 * @brief Inicia a gravação com um nome de arquivo específico
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param filepath Caminho completo para o arquivo
 * @return true Se iniciou com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_start_to_file(sdl_video_recorder_t *recorder, const char *filepath);

/**
 * @brief Para a gravação atual
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @return true Se parou com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_stop(sdl_video_recorder_t *recorder);

/**
 * @brief Pausa ou retoma a gravação atual
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param paused true para pausar, false para retomar
 * @return true Se mudou o estado com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_set_paused(sdl_video_recorder_t *recorder, bool paused);

/**
 * @brief Captura um frame para a gravação
 *
 * Deve ser chamado dentro do loop principal de renderização
 * após a renderização do quadro atual, mas antes de SDL_RenderPresent
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @return true Se capturou com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_capture_frame(sdl_video_recorder_t *recorder);

/**
 * @brief Captura um framebuffer específico para a gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param framebuffer Buffer de pixels a ser capturado (RGBA)
 * @param width Largura da imagem
 * @param height Altura da imagem
 * @return true Se capturou com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_capture_framebuffer(sdl_video_recorder_t *recorder,
                                          const uint32_t *framebuffer,
                                          int width, int height);

/**
 * @brief Fornece áudio para a gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param audio_data Dados de áudio
 * @param length Tamanho dos dados em bytes
 * @return true Se capturou com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_capture_audio(sdl_video_recorder_t *recorder,
                                    const void *audio_data,
                                    int length);

/**
 * @brief Define a configuração do sistema de gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param config Nova configuração
 */
void sdl_video_recorder_set_config(sdl_video_recorder_t *recorder,
                                 const sdl_video_recorder_config_t *config);

/**
 * @brief Obtém a configuração atual do sistema de gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param config Ponteiro para armazenar a configuração
 */
void sdl_video_recorder_get_config(const sdl_video_recorder_t *recorder,
                                 sdl_video_recorder_config_t *config);

/**
 * @brief Restaura a configuração padrão
 *
 * @param recorder Ponteiro para a estrutura de gravação
 */
void sdl_video_recorder_set_default_config(sdl_video_recorder_t *recorder);

/**
 * @brief Salva a configuração atual para um arquivo
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param filepath Caminho do arquivo
 * @return true Se salvo com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_save_config(const sdl_video_recorder_t *recorder, const char *filepath);

/**
 * @brief Carrega a configuração de um arquivo
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param filepath Caminho do arquivo
 * @return true Se carregado com sucesso
 * @return false Se falhou
 */
bool sdl_video_recorder_load_config(sdl_video_recorder_t *recorder, const char *filepath);

/**
 * @brief Define o nome do jogo atual
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param game_name Nome do jogo
 */
void sdl_video_recorder_set_game_name(sdl_video_recorder_t *recorder, const char *game_name);

/**
 * @brief Obtém o caminho da última gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @return const char* Caminho da última gravação, ou NULL se não houver
 */
const char* sdl_video_recorder_get_last_path(const sdl_video_recorder_t *recorder);

/**
 * @brief Obtém o timestamp da última gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @return time_t Timestamp da última gravação, ou 0 se não houver
 */
time_t sdl_video_recorder_get_last_time(const sdl_video_recorder_t *recorder);

/**
 * @brief Obtém as estatísticas da gravação atual
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param stats Ponteiro para armazenar as estatísticas
 */
void sdl_video_recorder_get_stats(const sdl_video_recorder_t *recorder,
                                sdl_video_recorder_stats_t *stats);

/**
 * @brief Atualiza o estado do indicador e notificação de gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 */
void sdl_video_recorder_update_ui(sdl_video_recorder_t *recorder);

/**
 * @brief Renderiza o indicador e notificação de gravação na tela
 *
 * @param recorder Ponteiro para a estrutura de gravação
 */
void sdl_video_recorder_render_ui(sdl_video_recorder_t *recorder);

/**
 * @brief Verifica se o sistema de gravação está gravando atualmente
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @return true Se está gravando
 * @return false Se não está gravando
 */
bool sdl_video_recorder_is_recording(const sdl_video_recorder_t *recorder);

/**
 * @brief Verifica se o sistema de gravação está pausado
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @return true Se está pausado
 * @return false Se não está pausado
 */
bool sdl_video_recorder_is_paused(const sdl_video_recorder_t *recorder);

/**
 * @brief Define a duração da notificação em milissegundos
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param duration_ms Duração em milissegundos
 */
void sdl_video_recorder_set_notification_duration(sdl_video_recorder_t *recorder,
                                               Uint32 duration_ms);

/**
 * @brief Define a posição do indicador de gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param x Posição X
 * @param y Posição Y
 * @param width Largura
 * @param height Altura
 */
void sdl_video_recorder_set_indicator_position(sdl_video_recorder_t *recorder,
                                             int x, int y, int width, int height);

/**
 * @brief Define o texto personalizado para o indicador de gravação
 *
 * @param recorder Ponteiro para a estrutura de gravação
 * @param text Texto a ser exibido (NULL para usar texto padrão)
 */
void sdl_video_recorder_set_indicator_text(sdl_video_recorder_t *recorder, const char *text);

#endif /* SDL_VIDEO_RECORDER_H */
