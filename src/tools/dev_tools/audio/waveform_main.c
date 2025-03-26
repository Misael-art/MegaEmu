/**
 * @file waveform_main.c
 * @brief Programa principal do visualizador de forma de onda
 */

#include "waveform_interface.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// Variáveis globais para tratamento de sinais
static volatile bool g_running = true;
static struct termios g_old_term;

// Handler para sinais de interrupção
static void signal_handler(int signum) { g_running = false; }

// Configura o terminal para leitura não-bloqueante
static void configure_terminal(void) {
  struct termios new_term;
  tcgetattr(STDIN_FILENO, &g_old_term);
  new_term = g_old_term;
  new_term.c_lflag &= ~(ICANON | ECHO);
  new_term.c_cc[VMIN] = 0;
  new_term.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

// Restaura a configuração original do terminal
static void restore_terminal(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &g_old_term);
}

// Dorme por um número específico de milissegundos
static void sleep_ms(uint32_t ms) {
  struct timespec ts = {.tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000};
  nanosleep(&ts, NULL);
}

int main(int argc, char *argv[]) {
  // Configura handler de sinais
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // Configura terminal
  configure_terminal();
  atexit(restore_terminal);

  // Cria a interface
  waveform_interface_t *interface =
      waveform_interface_create(800, 600, "Visualizador de Forma de Onda");

  if (!interface) {
    fprintf(stderr, "Erro ao criar interface\n");
    return EXIT_FAILURE;
  }

  // Loop principal
  char c;
  while (g_running) {
    // Processa entrada do usuário
    if (read(STDIN_FILENO, &c, 1) == 1) {
      waveform_interface_process_char(interface, c);
    }

    // Atualiza a interface
    if (!waveform_interface_update(interface)) {
      break;
    }

    // Limita a taxa de atualização
    sleep_ms(16); // ~60 FPS
  }

  // Limpa
  waveform_interface_destroy(interface);

  printf("\nVisualizador encerrado.\n");
  return EXIT_SUCCESS;
}
