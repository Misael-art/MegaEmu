/**
 * @file sdl_input_config.h
 * @brief Sistema de configuração de entrada para o frontend SDL
 */
#ifndef SDL_INPUT_CONFIG_H
#define SDL_INPUT_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include "sdl_game_renderer.h"

/**
 * @brief Tamanho máximo para nomes de dispositivos, botões, etc.
 */
#define SDL_INPUT_MAX_NAME_LENGTH 64

/**
 * @brief Número máximo de botões configuráveis por jogador
 */
#define SDL_INPUT_MAX_BUTTONS 16

/**
 * @brief Número máximo de jogadores suportados
 */
#define SDL_INPUT_MAX_PLAYERS 4

/**
 * @brief Tipos de dispositivos de entrada suportados
 */
typedef enum {
    SDL_INPUT_DEVICE_KEYBOARD,    ///< Teclado
    SDL_INPUT_DEVICE_GAMEPAD,     ///< Controle de jogo (Gamepad/Joystick)
    SDL_INPUT_DEVICE_MOUSE,       ///< Mouse
    SDL_INPUT_DEVICE_TOUCH        ///< Toque na tela (dispositivos touch)
} sdl_input_device_type_t;

/**
 * @brief Tipos de botões padrão em sistemas de jogos
 */
typedef enum {
    // Botões digitais padrão
    SDL_INPUT_BUTTON_UP,          ///< D-Pad para cima
    SDL_INPUT_BUTTON_DOWN,        ///< D-Pad para baixo
    SDL_INPUT_BUTTON_LEFT,        ///< D-Pad para esquerda
    SDL_INPUT_BUTTON_RIGHT,       ///< D-Pad para direita
    SDL_INPUT_BUTTON_A,           ///< Botão A (primary)
    SDL_INPUT_BUTTON_B,           ///< Botão B (secondary)
    SDL_INPUT_BUTTON_X,           ///< Botão X (secondary)
    SDL_INPUT_BUTTON_Y,           ///< Botão Y (secondary)
    SDL_INPUT_BUTTON_L,           ///< Botão L (left shoulder)
    SDL_INPUT_BUTTON_R,           ///< Botão R (right shoulder)
    SDL_INPUT_BUTTON_L2,          ///< Botão L2 (left trigger)
    SDL_INPUT_BUTTON_R2,          ///< Botão R2 (right trigger)
    SDL_INPUT_BUTTON_L3,          ///< Botão L3 (left stick press)
    SDL_INPUT_BUTTON_R3,          ///< Botão R3 (right stick press)
    SDL_INPUT_BUTTON_SELECT,      ///< Botão Select/Back
    SDL_INPUT_BUTTON_START,       ///< Botão Start

    // Meta-botões para sistema
    SDL_INPUT_BUTTON_HOME,        ///< Botão Home/Guide
    SDL_INPUT_BUTTON_MENU,        ///< Botão de Menu do sistema
    SDL_INPUT_BUTTON_CAPTURE,     ///< Botão de captura de tela

    // Direções analógicas (sticks)
    SDL_INPUT_ANALOG_LEFT_X,      ///< Eixo X analógico esquerdo
    SDL_INPUT_ANALOG_LEFT_Y,      ///< Eixo Y analógico esquerdo
    SDL_INPUT_ANALOG_RIGHT_X,     ///< Eixo X analógico direito
    SDL_INPUT_ANALOG_RIGHT_Y,     ///< Eixo Y analógico direito

    // Total de botões
    SDL_INPUT_BUTTON_COUNT
} sdl_input_button_type_t;

/**
 * @brief Mapeamento de um botão físico para um botão emulado
 */
typedef struct {
    sdl_input_device_type_t device_type;                      ///< Tipo de dispositivo
    int device_id;                                           ///< ID do dispositivo (para gamepads)

    union {
        struct {
            SDL_Scancode scancode;                           ///< Código da tecla para teclado
        } keyboard;

        struct {
            int button;                                      ///< Número do botão no gamepad
        } gamepad_button;

        struct {
            int axis;                                        ///< Número do eixo no gamepad
            int direction;                                   ///< 1 para positivo, -1 para negativo
            float threshold;                                 ///< Limiar de ativação (0.0 a 1.0)
        } gamepad_axis;

        struct {
            int button;                                      ///< Botão do mouse (1=esquerdo, 2=meio, 3=direito)
        } mouse_button;

        struct {
            SDL_Rect region;                                 ///< Região da tela para toques
        } touch;
    };

    bool inverted;                                           ///< Se a entrada deve ser invertida
    float deadzone;                                          ///< Zona morta para entradas analógicas
    float sensitivity;                                       ///< Sensibilidade para entradas analógicas
} sdl_input_mapping_t;

/**
 * @brief Configuração de entrada para um jogador
 */
typedef struct {
    char profile_name[SDL_INPUT_MAX_NAME_LENGTH];            ///< Nome do perfil
    bool enabled;                                            ///< Se este perfil está ativo
    sdl_input_mapping_t mappings[SDL_INPUT_BUTTON_COUNT];    ///< Mapeamento para cada botão
} sdl_input_player_config_t;

/**
 * @brief Opções de visualização de configuração de entrada
 */
typedef struct {
    bool show_button_labels;                                 ///< Mostrar rótulos dos botões
    bool show_controller_image;                              ///< Mostrar imagem do controle
    bool show_input_history;                                 ///< Mostrar histórico de entradas
    bool highlight_active_buttons;                           ///< Destacar botões ativos
    int mapping_area_width;                                  ///< Largura da área de mapeamento
    int mapping_area_height;                                 ///< Altura da área de mapeamento
} sdl_input_config_display_options_t;

/**
 * @brief Informações sobre um dispositivo de entrada conectado
 */
typedef struct {
    sdl_input_device_type_t type;                            ///< Tipo de dispositivo
    int id;                                                  ///< ID do dispositivo
    char name[SDL_INPUT_MAX_NAME_LENGTH];                    ///< Nome do dispositivo
    bool connected;                                          ///< Se está conectado atualmente
    int num_buttons;                                         ///< Número de botões (para gamepad)
    int num_axes;                                            ///< Número de eixos (para gamepad)
    SDL_JoystickGUID guid;                                   ///< GUID do controle (para gamepad)
    SDL_JoystickID instance_id;                              ///< ID da instância do joystick

    // Dados específicos por tipo de dispositivo
    union {
        struct {
            SDL_Joystick *joystick;                          ///< Handle do joystick
            SDL_GameController *controller;                  ///< Handle do game controller
            bool is_game_controller;                         ///< Se é reconhecido como game controller
        } gamepad;
    };
} sdl_input_device_info_t;

/**
 * @brief Estado de um evento de mapeamento
 */
typedef struct {
    bool active;                                             ///< Se está aguardando entrada
    int player_index;                                        ///< Índice do jogador
    sdl_input_button_type_t button_type;                     ///< Tipo de botão sendo mapeado
    Uint32 start_time;                                       ///< Tempo de início do mapping
    Uint32 timeout;                                          ///< Timeout em ms
} sdl_input_mapping_event_t;

/**
 * @brief Histórico de entrada para visualização
 */
typedef struct {
    sdl_input_button_type_t button_type;                     ///< Tipo de botão pressionado
    int player_index;                                        ///< Índice do jogador
    Uint32 timestamp;                                        ///< Timestamp do evento
    bool is_pressed;                                         ///< Se é um evento de pressionar ou soltar
    float value;                                             ///< Valor (para entradas analógicas)
} sdl_input_history_entry_t;

/**
 * @brief Container para histórico de entradas
 */
typedef struct {
    sdl_input_history_entry_t entries[64];                   ///< Histórico circular de entradas
    int count;                                               ///< Número de entradas válidas
    int next_index;                                          ///< Próximo índice a ser escrito
} sdl_input_history_t;

/**
 * @brief Estado dos botões para um jogador
 */
typedef struct {
    bool digital[SDL_INPUT_BUTTON_COUNT];                    ///< Estado dos botões digitais
    float analog[SDL_INPUT_BUTTON_COUNT];                    ///< Valores dos controles analógicos
} sdl_input_player_state_t;

/**
 * @brief Estrutura principal para configuração de entrada
 */
typedef struct {
    bool initialized;                                        ///< Se o sistema está inicializado
    sdl_game_renderer_t *renderer;                           ///< Renderizador do jogo

    sdl_input_player_config_t player_configs[SDL_INPUT_MAX_PLAYERS]; ///< Configuração por jogador
    sdl_input_player_state_t player_states[SDL_INPUT_MAX_PLAYERS];   ///< Estado atual por jogador

    sdl_input_device_info_t *devices;                        ///< Lista de dispositivos conectados
    int num_devices;                                         ///< Número de dispositivos conectados
    int devices_capacity;                                    ///< Capacidade da lista de dispositivos

    sdl_input_mapping_event_t mapping_event;                 ///< Evento de mapeamento atual
    sdl_input_history_t input_history;                       ///< Histórico de entradas

    bool reconfigure_mode;                                   ///< Se está no modo de reconfiguração
    int active_player_config;                                ///< Índice do jogador sendo configurado

    SDL_Texture *controller_image;                           ///< Imagem de referência do controle
    SDL_Texture *button_labels[SDL_INPUT_BUTTON_COUNT];      ///< Texturas para rótulos dos botões

    sdl_input_config_display_options_t display_options;      ///< Opções de visualização

    // Callbacks para eventos de configuração
    void (*on_mapping_changed)(int player, sdl_input_button_type_t button);
    void (*on_config_saved)(void);
    void (*on_config_loaded)(void);
    void (*on_device_connected)(const sdl_input_device_info_t *device);
    void (*on_device_disconnected)(const sdl_input_device_info_t *device);
} sdl_input_config_t;

/**
 * @brief Inicializa o sistema de configuração de entrada
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param renderer Ponteiro para o renderizador do jogo
 * @return true Se inicializado com sucesso
 * @return false Se falhou
 */
bool sdl_input_config_init(sdl_input_config_t *input_config, sdl_game_renderer_t *renderer);

/**
 * @brief Finaliza o sistema de configuração de entrada
 *
 * @param input_config Ponteiro para a estrutura de configuração
 */
void sdl_input_config_shutdown(sdl_input_config_t *input_config);

/**
 * @brief Carrega configurações de um arquivo
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param filepath Caminho do arquivo
 * @return true Se carregado com sucesso
 * @return false Se falhou
 */
bool sdl_input_config_load(sdl_input_config_t *input_config, const char *filepath);

/**
 * @brief Salva configurações para um arquivo
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param filepath Caminho do arquivo
 * @return true Se salvo com sucesso
 * @return false Se falhou
 */
bool sdl_input_config_save(sdl_input_config_t *input_config, const char *filepath);

/**
 * @brief Carrega configurações padrões
 *
 * @param input_config Ponteiro para a estrutura de configuração
 */
void sdl_input_config_load_defaults(sdl_input_config_t *input_config);

/**
 * @brief Processa eventos SDL para atualizar o estado de entrada
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param event Evento SDL a ser processado
 * @return true Se o evento foi consumido
 * @return false Se o evento não foi consumido
 */
bool sdl_input_config_process_event(sdl_input_config_t *input_config, const SDL_Event *event);

/**
 * @brief Atualiza o estado de entrada
 *
 * Deve ser chamado uma vez por frame para atualizar estados analógicos
 * e processar repetições de botões.
 *
 * @param input_config Ponteiro para a estrutura de configuração
 */
void sdl_input_config_update(sdl_input_config_t *input_config);

/**
 * @brief Verifica se um botão está pressionado para um jogador
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador (0-3)
 * @param button_type Tipo do botão
 * @return true Se o botão está pressionado
 * @return false Se o botão não está pressionado
 */
bool sdl_input_config_is_button_pressed(sdl_input_config_t *input_config,
                                      int player_index,
                                      sdl_input_button_type_t button_type);

/**
 * @brief Obtém o valor analógico para um botão/eixo
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador (0-3)
 * @param button_type Tipo do botão ou eixo
 * @return float Valor normalizado (-1.0 a 1.0)
 */
float sdl_input_config_get_analog_value(sdl_input_config_t *input_config,
                                      int player_index,
                                      sdl_input_button_type_t button_type);

/**
 * @brief Inicia o processo de mapeamento de um botão
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador (0-3)
 * @param button_type Tipo do botão
 * @param timeout_ms Timeout em ms (0 para sem timeout)
 * @return true Se iniciou com sucesso
 * @return false Se falhou
 */
bool sdl_input_config_start_mapping(sdl_input_config_t *input_config,
                                  int player_index,
                                  sdl_input_button_type_t button_type,
                                  Uint32 timeout_ms);

/**
 * @brief Cancela o processo de mapeamento de botão atual
 *
 * @param input_config Ponteiro para a estrutura de configuração
 */
void sdl_input_config_cancel_mapping(sdl_input_config_t *input_config);

/**
 * @brief Reseta o mapeamento de um botão para o padrão
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador (0-3)
 * @param button_type Tipo do botão
 */
void sdl_input_config_reset_mapping(sdl_input_config_t *input_config,
                                  int player_index,
                                  sdl_input_button_type_t button_type);

/**
 * @brief Reseta todos os mapeamentos para o padrão
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador (0-3)
 */
void sdl_input_config_reset_all_mappings(sdl_input_config_t *input_config, int player_index);

/**
 * @brief Define manualmente um mapeamento de botão
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador (0-3)
 * @param button_type Tipo do botão
 * @param mapping Novo mapeamento
 */
void sdl_input_config_set_mapping(sdl_input_config_t *input_config,
                                int player_index,
                                sdl_input_button_type_t button_type,
                                const sdl_input_mapping_t *mapping);

/**
 * @brief Obtém o mapeamento atual de um botão
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador (0-3)
 * @param button_type Tipo do botão
 * @param mapping Ponteiro para armazenar o mapeamento
 */
void sdl_input_config_get_mapping(sdl_input_config_t *input_config,
                                int player_index,
                                sdl_input_button_type_t button_type,
                                sdl_input_mapping_t *mapping);

/**
 * @brief Renderiza a interface de configuração de controles
 *
 * @param input_config Ponteiro para a estrutura de configuração
 */
void sdl_input_config_render(sdl_input_config_t *input_config);

/**
 * @brief Ativa ou desativa o modo de reconfiguração
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param active Se o modo deve ser ativado
 * @param player_index Índice do jogador a ser configurado
 */
void sdl_input_config_set_reconfigure_mode(sdl_input_config_t *input_config,
                                         bool active,
                                         int player_index);

/**
 * @brief Verifica se o modo de reconfiguração está ativo
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @return true Se o modo de reconfiguração está ativo
 * @return false Se não está ativo
 */
bool sdl_input_config_is_reconfigure_mode(const sdl_input_config_t *input_config);

/**
 * @brief Atualiza a lista de dispositivos de entrada conectados
 *
 * @param input_config Ponteiro para a estrutura de configuração
 */
void sdl_input_config_refresh_devices(sdl_input_config_t *input_config);

/**
 * @brief Obtém informações sobre um dispositivo conectado
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param device_index Índice do dispositivo
 * @return const sdl_input_device_info_t* Ponteiro para informações do dispositivo ou NULL se inválido
 */
const sdl_input_device_info_t* sdl_input_config_get_device_info(
    const sdl_input_config_t *input_config,
    int device_index);

/**
 * @brief Obtém o número de dispositivos conectados
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @return int Número de dispositivos
 */
int sdl_input_config_get_device_count(const sdl_input_config_t *input_config);

/**
 * @brief Adiciona uma entrada ao histórico
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param player_index Índice do jogador
 * @param button_type Tipo do botão
 * @param is_pressed Se é um evento de pressionar ou soltar
 * @param value Valor (para entradas analógicas)
 */
void sdl_input_config_add_to_history(sdl_input_config_t *input_config,
                                   int player_index,
                                   sdl_input_button_type_t button_type,
                                   bool is_pressed,
                                   float value);

/**
 * @brief Limpa o histórico de entradas
 *
 * @param input_config Ponteiro para a estrutura de configuração
 */
void sdl_input_config_clear_history(sdl_input_config_t *input_config);

/**
 * @brief Obtém o nome de um botão para exibição
 *
 * @param button_type Tipo do botão
 * @return const char* Nome do botão
 */
const char* sdl_input_config_get_button_name(sdl_input_button_type_t button_type);

/**
 * @brief Obtém o nome de um dispositivo para exibição
 *
 * @param device_type Tipo do dispositivo
 * @return const char* Nome do dispositivo
 */
const char* sdl_input_config_get_device_type_name(sdl_input_device_type_t device_type);

/**
 * @brief Obtém uma descrição textual de um mapeamento
 *
 * @param mapping Mapeamento a ser descrito
 * @param buffer Buffer para a descrição
 * @param buffer_size Tamanho do buffer
 */
void sdl_input_config_get_mapping_description(const sdl_input_mapping_t *mapping,
                                            char *buffer,
                                            size_t buffer_size);

/**
 * @brief Exibe a notificação de dispositivo conectado/desconectado
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param device_info Informações do dispositivo
 * @param connected Se está conectando (true) ou desconectando (false)
 */
void sdl_input_config_show_device_notification(sdl_input_config_t *input_config,
                                             const sdl_input_device_info_t *device_info,
                                             bool connected);

/**
 * @brief Atualiza as opções de visualização
 *
 * @param input_config Ponteiro para a estrutura de configuração
 * @param options Novas opções
 */
void sdl_input_config_set_display_options(sdl_input_config_t *input_config,
                                        const sdl_input_config_display_options_t *options);

#endif /* SDL_INPUT_CONFIG_H */
