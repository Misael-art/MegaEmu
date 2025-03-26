/**
 * @file turbo.h
 * @brief Interface principal do sistema de turbo/autofire
 */

#ifndef MEGA_EMU_TURBO_H
#define MEGA_EMU_TURBO_H

#include "../input/input_interface.h"
#include "turbo_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa o sistema de turbo/autofire
 *
 * @param input Interface de entrada do emulador
 * @param platform Plataforma alvo
 * @return true se inicializado com sucesso, false caso contrário
 */
bool mega_emu_turbo_init(void *input, mega_emu_turbo_platform_t platform);

/**
 * @brief Finaliza o sistema de turbo/autofire e libera recursos
 */
void mega_emu_turbo_shutdown(void);

/**
 * @brief Adiciona ou atualiza uma configuração de turbo para um botão
 *
 * @param config Configuração do botão com turbo
 * @return true se adicionado/atualizado com sucesso, false caso contrário
 */
bool mega_emu_turbo_set_config(const mega_emu_turbo_config_t *config);

/**
 * @brief Obtém a configuração de turbo atual para um botão
 *
 * @param button Botão a ser consultado
 * @param port Porta do controle
 * @param config Ponteiro para estrutura que receberá a configuração
 * @return true se obtido com sucesso, false caso contrário
 */
bool mega_emu_turbo_get_config(mega_emu_turbo_button_t button, uint8_t port,
                               mega_emu_turbo_config_t *config);

/**
 * @brief Remove a configuração de turbo para um botão
 *
 * @param button Botão a ser removido
 * @param port Porta do controle
 * @return true se removido com sucesso, false caso contrário
 */
bool mega_emu_turbo_remove_config(mega_emu_turbo_button_t button, uint8_t port);

/**
 * @brief Habilita ou desabilita o turbo para um botão
 *
 * @param button Botão a ser configurado
 * @param port Porta do controle
 * @param enabled Estado desejado (true para habilitado, false para
 * desabilitado)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_turbo_set_enabled(mega_emu_turbo_button_t button, uint8_t port,
                                bool enabled);

/**
 * @brief Verifica se o turbo está habilitado para um botão
 *
 * @param button Botão a ser consultado
 * @param port Porta do controle
 * @return true se o turbo está habilitado, false caso contrário
 */
bool mega_emu_turbo_is_enabled(mega_emu_turbo_button_t button, uint8_t port);

/**
 * @brief Define a velocidade do turbo para um botão
 *
 * @param button Botão a ser configurado
 * @param port Porta do controle
 * @param speed_preset Velocidade predefinida
 * @param custom_speed Velocidade personalizada (se speed_preset for
 * TURBO_SPEED_CUSTOM)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_turbo_set_speed(mega_emu_turbo_button_t button, uint8_t port,
                              mega_emu_turbo_speed_preset_t speed_preset,
                              uint8_t custom_speed);

/**
 * @brief Define o ciclo de trabalho do turbo para um botão
 *
 * @param button Botão a ser configurado
 * @param port Porta do controle
 * @param duty_cycle Ciclo de trabalho (0-100%)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_turbo_set_duty_cycle(mega_emu_turbo_button_t button, uint8_t port,
                                   uint8_t duty_cycle);

/**
 * @brief Define o modo de operação do turbo para um botão
 *
 * @param button Botão a ser configurado
 * @param port Porta do controle
 * @param mode Modo de operação
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_turbo_set_mode(mega_emu_turbo_button_t button, uint8_t port,
                             mega_emu_turbo_mode_t mode);

/**
 * @brief Obtém a lista de botões configurados com turbo
 *
 * @param buttons Array para armazenar os botões
 * @param ports Array para armazenar as portas dos controles
 * @param max_buttons Tamanho máximo dos arrays
 * @return Número de botões obtidos
 */
uint32_t mega_emu_turbo_get_configured_buttons(mega_emu_turbo_button_t *buttons,
                                               uint8_t *ports,
                                               uint32_t max_buttons);

/**
 * @brief Registra um callback para ser notificado sobre eventos do turbo
 *
 * @param callback Função de callback
 * @param user_data Dados do usuário passados para o callback
 * @return int ID do callback ou -1 em caso de erro
 */
int mega_emu_turbo_register_callback(mega_emu_turbo_callback_t callback,
                                     void *user_data);

/**
 * @brief Remove um callback registrado
 *
 * @param callback_id ID do callback
 * @return true se removido com sucesso, false caso contrário
 */
bool mega_emu_turbo_unregister_callback(int callback_id);

/**
 * @brief Processa os botões com turbo e atualiza estados
 *
 * Esta função deve ser chamada uma vez por frame para atualizar
 * os estados dos botões com turbo.
 *
 * @param frame_time Tempo decorrido desde o último frame em ms
 * @return Número de botões com turbo processados
 */
uint32_t mega_emu_turbo_process(float frame_time);

/**
 * @brief Reseta o estado de todos os botões com turbo
 *
 * @return true se resetado com sucesso, false caso contrário
 */
bool mega_emu_turbo_reset_all(void);

/**
 * @brief Verifica se um botão específico está em estado de turbo
 *
 * @param button Botão a ser consultado
 * @param port Porta do controle
 * @return true se o botão está em estado ativo de turbo, false caso contrário
 */
bool mega_emu_turbo_is_button_active(mega_emu_turbo_button_t button,
                                     uint8_t port);

/**
 * @brief Salva configurações de turbo em um arquivo
 *
 * @param filename Caminho para o arquivo
 * @return true se salvo com sucesso, false caso contrário
 */
bool mega_emu_turbo_save_config(const char *filename);

/**
 * @brief Carrega configurações de turbo a partir de um arquivo
 *
 * @param filename Caminho para o arquivo
 * @return true se carregado com sucesso, false caso contrário
 */
bool mega_emu_turbo_load_config(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_TURBO_H */
