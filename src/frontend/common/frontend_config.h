/**
 * @file frontend_config.h
 * @brief Interface comum para configuração do frontend
 *
 * Este arquivo define a interface de configuração comum do frontend
 * que pode ser estendida por implementações específicas (SDL, Qt, etc.).
 */

#ifndef FRONTEND_CONFIG_H
#define FRONTEND_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Estrutura de configuração básica do frontend
 *
 * Esta estrutura contém configurações comuns a todos os frontends,
 * com a possibilidade de ser estendida para configurações específicas.
 */
typedef struct emu_frontend_config_s {
    // Vídeo
    int window_width;
    int window_height;
    int game_width;
    int game_height;
    float scale_factor;
    bool vsync_enabled;
    bool fullscreen;
    bool smooth_scaling;
    bool integer_scaling;

    // Áudio
    int audio_sample_rate;
    int audio_buffer_size;
    bool audio_enabled;

    // Entrada
    bool keyboard_enabled;
    bool gamepad_enabled;

    // Interface
    bool show_fps;
    bool debug_overlay;

    // Tema
    int theme_id;

    // Extensibilidade para frontends específicos
    void* frontend_specific;  // Ponteiro para dados específicos do frontend
} emu_frontend_config_t;

// Configuração padrão
extern const emu_frontend_config_t EMU_DEFAULT_FRONTEND_CONFIG;

/**
 * @brief Inicializa a configuração global do frontend
 */
void emu_frontend_config_init(void);

/**
 * @brief Define os valores padrão para uma configuração
 *
 * @param config Ponteiro para a estrutura de configuração a ser inicializada
 */
void emu_frontend_config_set_defaults(emu_frontend_config_t* config);

/**
 * @brief Obtém um ponteiro para a configuração global
 *
 * @return emu_frontend_config_t* Ponteiro para a configuração global
 */
emu_frontend_config_t* emu_frontend_config_get(void);

/**
 * @brief Carrega a configuração do frontend de um arquivo
 *
 * @param config_file Caminho para o arquivo de configuração
 * @param config Configuração a ser preenchida (se NULL, usa a global)
 * @return bool true se a configuração foi carregada com sucesso
 */
bool emu_frontend_config_load(const char* config_file, emu_frontend_config_t* config);

/**
 * @brief Salva a configuração do frontend em um arquivo
 *
 * @param config_file Caminho para o arquivo de configuração
 * @param config Configuração a ser salva (se NULL, usa a global)
 * @return bool true se a configuração foi salva com sucesso
 */
bool emu_frontend_config_save(const char* config_file, const emu_frontend_config_t* config);

/**
 * @brief API de extensão para ler opções específicas do frontend
 *
 * Esta função permite que implementações específicas processem suas próprias
 * opções de configuração durante o carregamento.
 *
 * @param key Chave da opção
 * @param value Valor da opção
 * @param config Ponteiro para a configuração
 * @return bool true se a opção foi processada
 */
bool emu_frontend_config_process_option(const char* key, const char* value, emu_frontend_config_t* config);

/**
 * @brief API de extensão para salvar opções específicas do frontend
 *
 * Esta função permite que implementações específicas salvem suas próprias
 * opções de configuração.
 *
 * @param file Ponteiro para o arquivo
 * @param config Ponteiro para a configuração
 * @return bool true se as opções foram salvas
 */
bool emu_frontend_config_write_specific_options(FILE* file, const emu_frontend_config_t* config);

#endif /* FRONTEND_CONFIG_H */
