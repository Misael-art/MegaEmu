/**
 * @file waveform_cli.c
 * @brief Implementação da interface de linha de comando
 */

#include "waveform_cli.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constantes
#define MAX_ARGS 16
#define PROMPT "> "

// Estrutura para mapear comandos a funções
typedef bool (*command_func)(waveform_cli_t *, int, char **);

typedef struct {
  const char *name;
  command_func func;
  const char *help;
} command_entry_t;

// Protótipos das funções de comando
static bool cmd_help(waveform_cli_t *cli, int argc, char **argv);
static bool cmd_quit(waveform_cli_t *cli, int argc, char **argv);
static bool cmd_echo(waveform_cli_t *cli, int argc, char **argv);
static bool cmd_zoom(waveform_cli_t *cli, int argc, char **argv);
static bool cmd_scroll(waveform_cli_t *cli, int argc, char **argv);
static bool cmd_channel(waveform_cli_t *cli, int argc, char **argv);
static bool cmd_grid(waveform_cli_t *cli, int argc, char **argv);
static bool cmd_peaks(waveform_cli_t *cli, int argc, char **argv);

// Tabela de comandos
static const command_entry_t commands[] = {
    {"help", cmd_help, "Mostra esta ajuda"},
    {"quit", cmd_quit, "Sai do programa"},
    {"echo", cmd_echo, "Habilita/desabilita eco de comandos"},
    {"zoom", cmd_zoom, "Define o nível de zoom (1-10)"},
    {"scroll", cmd_scroll, "Habilita/desabilita rolagem automática"},
    {"channel", cmd_channel,
     "Configura canal (pulse1|pulse2|triangle|noise|dmc) (on|off)"},
    {"grid", cmd_grid, "Habilita/desabilita grade"},
    {"peaks", cmd_peaks, "Habilita/desabilita indicadores de pico"},
    {NULL, NULL, NULL}};

// Funções auxiliares
static void split_command(char *cmd, int *argc, char **argv) {
  *argc = 0;
  char *token = strtok(cmd, " \t\n");
  while (token && *argc < MAX_ARGS) {
    argv[(*argc)++] = token;
    token = strtok(NULL, " \t\n");
  }
}

static command_func find_command(const char *name) {
  for (const command_entry_t *cmd = commands; cmd->name; cmd++) {
    if (strcmp(cmd->name, name) == 0) {
      return cmd->func;
    }
  }
  return NULL;
}

// Implementação das funções públicas
waveform_cli_t *waveform_cli_create(waveform_viewer_t *viewer) {
  if (!viewer)
    return NULL;

  waveform_cli_t *cli = calloc(1, sizeof(waveform_cli_t));
  if (!cli)
    return NULL;

  cli->viewer = viewer;
  cli->running = true;
  cli->echo_enabled = true;
  cli->buffer_pos = 0;
  cli->command_buffer[0] = '\0';

  printf("Digite 'help' para ver os comandos disponíveis.\n");
  printf(PROMPT);
  fflush(stdout);

  return cli;
}

void waveform_cli_destroy(waveform_cli_t *cli) { free(cli); }

bool waveform_cli_process_char(waveform_cli_t *cli, char c) {
  if (!cli)
    return false;

  // Processa caracteres especiais
  switch (c) {
  case '\n':
  case '\r':
    printf("\n");
    if (cli->buffer_pos > 0) {
      cli->command_buffer[cli->buffer_pos] = '\0';
      bool result = waveform_cli_execute_command(cli, cli->command_buffer);
      cli->buffer_pos = 0;
      if (cli->running) {
        printf(PROMPT);
        fflush(stdout);
      }
      return result;
    }
    printf(PROMPT);
    fflush(stdout);
    return true;

  case '\b':
  case 127: // DEL
    if (cli->buffer_pos > 0) {
      cli->buffer_pos--;
      if (cli->echo_enabled) {
        printf("\b \b");
        fflush(stdout);
      }
    }
    return true;

  default:
    if (cli->buffer_pos < sizeof(cli->command_buffer) - 1 && isprint(c)) {
      cli->command_buffer[cli->buffer_pos++] = c;
      if (cli->echo_enabled) {
        putchar(c);
        fflush(stdout);
      }
    }
    return true;
  }
}

bool waveform_cli_execute_command(waveform_cli_t *cli, const char *command) {
  if (!cli || !command)
    return false;

  char cmd_copy[256];
  strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
  cmd_copy[sizeof(cmd_copy) - 1] = '\0';

  int argc = 0;
  char *argv[MAX_ARGS];
  split_command(cmd_copy, &argc, argv);

  if (argc == 0)
    return true;

  command_func func = find_command(argv[0]);
  if (func) {
    return func(cli, argc, argv);
  } else {
    printf("Comando desconhecido: %s\n", argv[0]);
    return true;
  }
}

void waveform_cli_show_help(waveform_cli_t *cli) {
  printf("Comandos disponíveis:\n");
  for (const command_entry_t *cmd = commands; cmd->name; cmd++) {
    printf("  %-10s %s\n", cmd->name, cmd->help);
  }
}

void waveform_cli_set_echo(waveform_cli_t *cli, bool enabled) {
  if (cli) {
    cli->echo_enabled = enabled;
  }
}

void waveform_cli_clear_buffer(waveform_cli_t *cli) {
  if (cli) {
    cli->buffer_pos = 0;
    cli->command_buffer[0] = '\0';
  }
}

// Implementação dos comandos
static bool cmd_help(waveform_cli_t *cli, int argc, char **argv) {
  waveform_cli_show_help(cli);
  return true;
}

static bool cmd_quit(waveform_cli_t *cli, int argc, char **argv) {
  cli->running = false;
  return false;
}

static bool cmd_echo(waveform_cli_t *cli, int argc, char **argv) {
  if (argc != 2) {
    printf("Uso: echo on|off\n");
    return true;
  }
  cli->echo_enabled = (strcmp(argv[1], "on") == 0);
  return true;
}

static bool cmd_zoom(waveform_cli_t *cli, int argc, char **argv) {
  if (argc != 2) {
    printf("Uso: zoom 1-10\n");
    return true;
  }
  int zoom = atoi(argv[1]);
  if (zoom < 1 || zoom > 10) {
    printf("Nível de zoom deve estar entre 1 e 10\n");
    return true;
  }
  waveform_set_zoom(cli->viewer, zoom);
  return true;
}

static bool cmd_scroll(waveform_cli_t *cli, int argc, char **argv) {
  if (argc != 2) {
    printf("Uso: scroll on|off\n");
    return true;
  }
  waveform_set_auto_scroll(cli->viewer, strcmp(argv[1], "on") == 0);
  return true;
}

static bool cmd_channel(waveform_cli_t *cli, int argc, char **argv) {
  if (argc != 3) {
    printf("Uso: channel <nome> on|off\n");
    printf("Canais: pulse1, pulse2, triangle, noise, dmc\n");
    return true;
  }

  waveform_channel_t channel;
  if (strcmp(argv[1], "pulse1") == 0)
    channel = WAVEFORM_CHANNEL_PULSE1;
  else if (strcmp(argv[1], "pulse2") == 0)
    channel = WAVEFORM_CHANNEL_PULSE2;
  else if (strcmp(argv[1], "triangle") == 0)
    channel = WAVEFORM_CHANNEL_TRIANGLE;
  else if (strcmp(argv[1], "noise") == 0)
    channel = WAVEFORM_CHANNEL_NOISE;
  else if (strcmp(argv[1], "dmc") == 0)
    channel = WAVEFORM_CHANNEL_DMC;
  else {
    printf("Canal inválido: %s\n", argv[1]);
    return true;
  }

  bool enabled = (strcmp(argv[2], "on") == 0);
  waveform_config_t config = waveform_get_config(cli->viewer);
  config.channel_enabled[channel] = enabled;
  waveform_set_config(cli->viewer, &config);
  return true;
}

static bool cmd_grid(waveform_cli_t *cli, int argc, char **argv) {
  if (argc != 2) {
    printf("Uso: grid on|off\n");
    return true;
  }
  waveform_config_t config = waveform_get_config(cli->viewer);
  config.show_grid = (strcmp(argv[1], "on") == 0);
  waveform_set_config(cli->viewer, &config);
  return true;
}

static bool cmd_peaks(waveform_cli_t *cli, int argc, char **argv) {
  if (argc != 2) {
    printf("Uso: peaks on|off\n");
    return true;
  }
  waveform_config_t config = waveform_get_config(cli->viewer);
  config.show_peaks = (strcmp(argv[1], "on") == 0);
  waveform_set_config(cli->viewer, &config);
  return true;
}
