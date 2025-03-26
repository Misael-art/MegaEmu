/**
 * @file waveform_interface.h
 * @brief Interface principal do visualizador de forma de onda
 */

#ifndef WAVEFORM_INTERFACE_H
#define WAVEFORM_INTERFACE_H

#include "waveform_cli.h"
#include "waveform_display.h"
#include "waveform_viewer.h"
#include <stdbool.h>

/**
 * @brief Estrutura que mantém o estado da interface principal
 */
typedef struct {
  waveform_viewer_t *viewer;   // Visualizador
  waveform_display_t *display; // Interface gráfica
  waveform_cli_t *cli;         // Interface de linha de comando
  bool running;                // Se a interface está em execução
  bool paused;                 // Se a visualização está pausada
  uint32_t frame_count;        // Contador de frames
  uint32_t last_update_time;   // Timestamp da última atualização
} waveform_interface_t;

/**
 * @brief Cria uma nova interface principal
 * @param width Largura inicial da janela
 * @param height Altura inicial da janela
 * @param title Título da janela
 * @return Ponteiro para a interface criada ou NULL em caso de erro
 */
waveform_interface_t *waveform_interface_create(uint32_t width, uint32_t height,
                                                const char *title);

/**
 * @brief Destrói uma interface principal
 * @param interface Ponteiro para a interface
 */
void waveform_interface_destroy(waveform_interface_t *interface);

/**
 * @brief Executa um ciclo de atualização da interface
 * @param interface Ponteiro para a interface
 * @return true se deve continuar executando
 */
bool waveform_interface_update(waveform_interface_t *interface);

/**
 * @brief Processa entrada de caractere da linha de comando
 * @param interface Ponteiro para a interface
 * @param c Caractere a ser processado
 */
void waveform_interface_process_char(waveform_interface_t *interface, char c);

/**
 * @brief Pausa ou retoma a visualização
 * @param interface Ponteiro para a interface
 * @param paused true para pausar, false para retomar
 */
void waveform_interface_set_paused(waveform_interface_t *interface,
                                   bool paused);

/**
 * @brief Verifica se a visualização está pausada
 * @param interface Ponteiro para a interface
 * @return true se está pausada
 */
bool waveform_interface_is_paused(const waveform_interface_t *interface);

/**
 * @brief Obtém o contador de frames
 * @param interface Ponteiro para a interface
 * @return Número de frames desde o início
 */
uint32_t
waveform_interface_get_frame_count(const waveform_interface_t *interface);

/**
 * @brief Obtém o tempo desde a última atualização
 * @param interface Ponteiro para a interface
 * @return Tempo em milissegundos desde a última atualização
 */
uint32_t
waveform_interface_get_last_update_time(const waveform_interface_t *interface);

#endif // WAVEFORM_INTERFACE_H
