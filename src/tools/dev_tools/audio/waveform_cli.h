/**
 * @file waveform_cli.h
 * @brief Interface de linha de comando para o visualizador de forma de onda
 */

#ifndef WAVEFORM_CLI_H
#define WAVEFORM_CLI_H

#include "waveform_viewer.h"
#include <stdbool.h>

/**
 * @brief Estrutura que mantém o estado da interface de linha de comando
 */
typedef struct {
  bool running;              // Se a CLI está em execução
  bool echo_enabled;         // Se deve mostrar os comandos digitados
  char command_buffer[256];  // Buffer para o comando atual
  size_t buffer_pos;         // Posição atual no buffer
  waveform_viewer_t *viewer; // Ponteiro para o visualizador
} waveform_cli_t;

/**
 * @brief Cria uma nova interface de linha de comando
 * @param viewer Ponteiro para o visualizador associado
 * @return Ponteiro para a CLI criada ou NULL em caso de erro
 */
waveform_cli_t *waveform_cli_create(waveform_viewer_t *viewer);

/**
 * @brief Destrói uma interface de linha de comando
 * @param cli Ponteiro para a CLI
 */
void waveform_cli_destroy(waveform_cli_t *cli);

/**
 * @brief Processa um caractere de entrada
 * @param cli Ponteiro para a CLI
 * @param c Caractere a ser processado
 * @return true se deve continuar executando, false para sair
 */
bool waveform_cli_process_char(waveform_cli_t *cli, char c);

/**
 * @brief Executa um comando
 * @param cli Ponteiro para a CLI
 * @param command String contendo o comando
 * @return true se deve continuar executando, false para sair
 */
bool waveform_cli_execute_command(waveform_cli_t *cli, const char *command);

/**
 * @brief Mostra a ajuda com os comandos disponíveis
 * @param cli Ponteiro para a CLI
 */
void waveform_cli_show_help(waveform_cli_t *cli);

/**
 * @brief Habilita ou desabilita o eco de comandos
 * @param cli Ponteiro para a CLI
 * @param enabled true para habilitar, false para desabilitar
 */
void waveform_cli_set_echo(waveform_cli_t *cli, bool enabled);

/**
 * @brief Limpa o buffer de comando atual
 * @param cli Ponteiro para a CLI
 */
void waveform_cli_clear_buffer(waveform_cli_t *cli);

#endif // WAVEFORM_CLI_H
