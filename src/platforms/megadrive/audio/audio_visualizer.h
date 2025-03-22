/**
 * @file audio_visualizer.h
 * @brief Visualizador de áudio para o sistema de áudio do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#ifndef MEGADRIVE_AUDIO_VISUALIZER_H
#define MEGADRIVE_AUDIO_VISUALIZER_H

#include <stdint.h>
#include "../../../common/types.h"
#include "audio_system.h"

/**
 * @brief Modos de visualização disponíveis
 */
typedef enum {
    MD_AUDIO_VIS_MODE_WAVEFORM,    /**< Forma de onda no tempo */
    MD_AUDIO_VIS_MODE_SPECTRUM,    /**< Espectro de frequência */
    MD_AUDIO_VIS_MODE_CHANNEL,     /**< Visualização por canal */
    MD_AUDIO_VIS_MODE_ENVELOPE,    /**< Envelopes ADSR */
    MD_AUDIO_VIS_MODE_COUNT        /**< Número total de modos */
} md_audio_vis_mode_t;

/**
 * @brief Canais de visualização disponíveis
 */
typedef enum {
    MD_AUDIO_VIS_CHANNEL_MIXED,    /**< Saída mixada */
    MD_AUDIO_VIS_CHANNEL_YM2612,   /**< Apenas YM2612 */
    MD_AUDIO_VIS_CHANNEL_SN76489,  /**< Apenas SN76489 */
    MD_AUDIO_VIS_CHANNEL_COUNT     /**< Número total de canais */
} md_audio_vis_channel_t;

/**
 * @brief Estrutura do visualizador de áudio
 */
typedef struct {
    md_audio_system_t* audio;          /**< Referência para o sistema de áudio */
    md_audio_vis_mode_t mode;          /**< Modo de visualização atual */
    md_audio_vis_channel_t channel;    /**< Canal de visualização atual */
    
    uint32_t buffer_size;              /**< Tamanho do buffer de visualização */
    int16_t* buffer_left;              /**< Buffer para o canal esquerdo */
    int16_t* buffer_right;             /**< Buffer para o canal direito */
    
    float* spectrum_left;              /**< Espectro para o canal esquerdo */
    float* spectrum_right;             /**< Espectro para o canal direito */
    
    uint32_t window_width;             /**< Largura da janela de visualização */
    uint32_t window_height;            /**< Altura da janela de visualização */
    
    uint8_t enabled;                   /**< Flag de habilitação */
    uint8_t paused;                    /**< Flag de pausa */
    
    float scale;                       /**< Escala de visualização */
    float offset;                      /**< Deslocamento de visualização */
    
    uint32_t update_rate;              /**< Taxa de atualização (ms) */
    uint32_t last_update;              /**< Timestamp da última atualização */
    
    void* renderer;                    /**< Ponteiro para o renderizador (SDL_Renderer) */
    void* texture;                     /**< Ponteiro para a textura (SDL_Texture) */
    void* window;                      /**< Ponteiro para a janela (SDL_Window) */
    
    uint32_t background_color;         /**< Cor de fundo */
    uint32_t waveform_color;           /**< Cor da forma de onda */
    uint32_t grid_color;               /**< Cor da grade */
    uint32_t text_color;               /**< Cor do texto */
    
    char title[64];                    /**< Título da janela */
    
    // Dados específicos para cada modo de visualização
    union {
        struct {
            uint32_t zoom;             /**< Nível de zoom para forma de onda */
            uint32_t position;         /**< Posição de visualização */
        } waveform;
        
        struct {
            uint32_t fft_size;         /**< Tamanho da FFT */
            float* window_function;    /**< Função de janela para FFT */
            uint8_t log_scale;         /**< Flag para escala logarítmica */
        } spectrum;
        
        struct {
            uint8_t active_channels;   /**< Canais ativos para visualização */
            uint8_t show_labels;       /**< Flag para mostrar rótulos */
        } channel;
        
        struct {
            uint8_t active_operators;  /**< Operadores ativos para visualização */
            uint8_t show_rates;        /**< Flag para mostrar taxas */
        } envelope;
    };
} md_audio_visualizer_t;

/**
 * @brief Inicializa o visualizador de áudio
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param audio Ponteiro para o sistema de áudio
 * @param buffer_size Tamanho do buffer de visualização
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_init(md_audio_visualizer_t* vis, md_audio_system_t* audio, uint32_t buffer_size);

/**
 * @brief Desliga o visualizador de áudio e libera recursos
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_shutdown(md_audio_visualizer_t* vis);

/**
 * @brief Reseta o visualizador de áudio
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_reset(md_audio_visualizer_t* vis);

/**
 * @brief Cria a janela de visualização
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param width Largura da janela
 * @param height Altura da janela
 * @param title Título da janela
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_create_window(md_audio_visualizer_t* vis, uint32_t width, uint32_t height, const char* title);

/**
 * @brief Fecha a janela de visualização
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_close_window(md_audio_visualizer_t* vis);

/**
 * @brief Atualiza o visualizador com novas amostras
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param samples_left Amostras do canal esquerdo
 * @param samples_right Amostras do canal direito
 * @param num_samples Número de amostras
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_update(md_audio_visualizer_t* vis, const int16_t* samples_left, const int16_t* samples_right, uint32_t num_samples);

/**
 * @brief Renderiza o visualizador
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_render(md_audio_visualizer_t* vis);

/**
 * @brief Define o modo de visualização
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param mode Modo de visualização
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_set_mode(md_audio_visualizer_t* vis, md_audio_vis_mode_t mode);

/**
 * @brief Define o canal de visualização
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param channel Canal de visualização
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_set_channel(md_audio_visualizer_t* vis, md_audio_vis_channel_t channel);

/**
 * @brief Define a escala de visualização
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param scale Escala de visualização
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_set_scale(md_audio_visualizer_t* vis, float scale);

/**
 * @brief Define o deslocamento de visualização
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param offset Deslocamento de visualização
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_set_offset(md_audio_visualizer_t* vis, float offset);

/**
 * @brief Habilita ou desabilita o visualizador
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param enabled Flag de habilitação
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_set_enabled(md_audio_visualizer_t* vis, uint8_t enabled);

/**
 * @brief Pausa ou retoma o visualizador
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param paused Flag de pausa
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_set_paused(md_audio_visualizer_t* vis, uint8_t paused);

/**
 * @brief Define as cores do visualizador
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param background_color Cor de fundo
 * @param waveform_color Cor da forma de onda
 * @param grid_color Cor da grade
 * @param text_color Cor do texto
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_set_colors(md_audio_visualizer_t* vis, uint32_t background_color, uint32_t waveform_color, uint32_t grid_color, uint32_t text_color);

/**
 * @brief Processa eventos do visualizador
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_process_events(md_audio_visualizer_t* vis);

/**
 * @brief Captura uma imagem do visualizador
 * 
 * @param vis Ponteiro para a estrutura do visualizador
 * @param filename Nome do arquivo para salvar a imagem
 * @return emu_error_t Código de erro
 */
emu_error_t md_audio_vis_capture_screenshot(md_audio_visualizer_t* vis, const char* filename);

#endif /* MEGADRIVE_AUDIO_VISUALIZER_H */
