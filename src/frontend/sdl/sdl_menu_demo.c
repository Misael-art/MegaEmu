/**
 * @file sdl_menu_demo.c
 * @brief Demonstração do sistema de menu SDL
 */
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend/sdl/sdl_game_renderer.h"
#include "frontend/sdl/sdl_menu.h"

#define DEMO_WIDTH 800
#define DEMO_HEIGHT 600
#define GAME_WIDTH 256
#define GAME_HEIGHT 240

// Estado global da demo
typedef struct {
  bool running;
  bool menu_visible;
  SDL_Window *window;
  SDL_Renderer *renderer;
  sdl_game_renderer_t game_renderer;
  sdl_menu_context_t menu_context;
  uint32_t *framebuffer;

  // Configurações para demonstração
  bool fullscreen;
  bool scanlines;
  bool crt_effect;
  bool smooth_scaling;
  int scale_factor;
  int volume;

  // Menus
  sdl_menu_t *main_menu;
  sdl_menu_t *video_menu;
  sdl_menu_t *audio_menu;
} demo_state_t;

static demo_state_t g_demo;

// Callbacks para itens de menu
static void toggle_fullscreen_callback(bool value, void *userdata) {
  demo_state_t *demo = (demo_state_t *)userdata;
  demo->fullscreen = value;
  sdl_game_renderer_toggle_fullscreen(&demo->game_renderer);
  printf("Fullscreen: %s\n", value ? "ON" : "OFF");
}

static void toggle_scanlines_callback(bool value, void *userdata) {
  demo_state_t *demo = (demo_state_t *)userdata;
  demo->scanlines = value;
  sdl_game_renderer_set_scanlines(&demo->game_renderer, value);
  printf("Scanlines: %s\n", value ? "ON" : "OFF");
}

static void toggle_crt_callback(bool value, void *userdata) {
  demo_state_t *demo = (demo_state_t *)userdata;
  demo->crt_effect = value;
  sdl_game_renderer_set_crt_effect(&demo->game_renderer, value);
  printf("CRT Effect: %s\n", value ? "ON" : "OFF");
}

static void toggle_smooth_scaling_callback(bool value, void *userdata) {
  demo_state_t *demo = (demo_state_t *)userdata;
  demo->smooth_scaling = value;
  sdl_game_renderer_set_smooth_scaling(&demo->game_renderer, value);
  printf("Smooth Scaling: %s\n", value ? "ON" : "OFF");
}

static void set_scale_factor_callback(int value, void *userdata) {
  demo_state_t *demo = (demo_state_t *)userdata;
  demo->scale_factor = value;
  float scale = (float)value / 10.0f;
  sdl_game_renderer_set_scale(&demo->game_renderer, scale);
  printf("Scale Factor: %f\n", scale);
}

static void set_volume_callback(int value, void *userdata) {
  demo_state_t *demo = (demo_state_t *)userdata;
  demo->volume = value;
  printf("Volume: %d%%\n", value);
}

static void exit_callback(void *userdata) {
  demo_state_t *demo = (demo_state_t *)userdata;
  demo->running = false;
  printf("Exiting demo\n");
}

// Gerar um framebuffer de exemplo para demonstração
static void generate_demo_frame(uint32_t *framebuffer, int width, int height,
                                uint32_t frame_count) {
  // Cores NES para demonstração
  static const uint32_t nes_palette[] = {
      0xFF808080, 0xFF0000BB, 0xFF3700BF, 0xFF8400A6, 0xFFBB006A, 0xFFB7001E,
      0xFF8A0700, 0xFF480D00, 0xFF001700, 0xFF001F00, 0xFF002100, 0xFF001E40,
      0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFBCBCBC, 0xFF0059FF,
      0xFF443CFF, 0xFF8C00FF, 0xFFBE007F, 0xFFD60040, 0xFFCB0000, 0xFF8B0000,
      0xFF003F00, 0xFF005800, 0xFF006B00, 0xFF006000, 0xFF000000, 0xFF000000,
      0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF38BFFF, 0xFF5883FF, 0xFFA057FF,
      0xFFD841FF, 0xFFFF4FC3, 0xFFFF6D3F, 0xFFCB8000, 0xFF8CBF00, 0xFF50DC00,
      0xFF40DF4F, 0xFF48C4C4, 0xFF555555, 0xFF000000, 0xFF000000, 0xFF000000,
      0xFFFFFFFF, 0xFFA8E4FF, 0xFFC3C9FF, 0xFFD5B8FF, 0xFFEEA9FF, 0xFFFFC0E0,
      0xFFFFD1AB, 0xFFFFE299, 0xFFDCF293, 0xFFD0FF9D, 0xFFCCFFCE, 0xFFBEFFE2,
      0xFFBEEEEE, 0xFF000000, 0xFF000000, 0xFF000000};

  // Calcular deslocamento para animar
  int offset_x = (frame_count / 2) % width;
  int offset_y = (frame_count / 3) % height;

  // Desenhar padrão de teste
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int index = y * width + x;

      // Fundo quadriculado
      if ((x / 8 + y / 8) % 2 == 0) {
        framebuffer[index] = 0xFF202020;
      } else {
        framebuffer[index] = 0xFF101010;
      }

      // Borda da tela
      if (x < 8 || x >= width - 8 || y < 8 || y >= height - 8) {
        framebuffer[index] = nes_palette[0x10]; // Cinza médio
      }

      // Grade de cores NES para demonstração
      int px = (x + offset_x) % width;
      int py = (y + offset_y) % height;

      if (px >= 32 && px < 32 + 16 * 8 && py >= 32 && py < 32 + 4 * 8) {
        int color_x = (px - 32) / 8;
        int color_y = (py - 32) / 8;
        int color_index = color_y * 16 + color_x;

        if (color_index < 64) {
          framebuffer[index] = nes_palette[color_index];
        }
      }

      // Texto "MENU DEMO"
      if (py >= height / 2 - 16 && py < height / 2 + 16) {
        // Texto simples
        const char *text = "MENU DEMO - F12";
        int text_len = strlen(text);
        int text_x = width / 2 - text_len * 4;

        if (px >= text_x && px < text_x + text_len * 8) {
          int char_index = (px - text_x) / 8;
          if (char_index < text_len) {
            char c = text[char_index];
            int bit_x = (px - text_x) % 8;
            int bit_y = (py - (height / 2 - 16)) % 16;

            // Desenho simples de fonte
            if (bit_y >= 2 && bit_y < 14 && bit_x >= 1 && bit_x < 7) {
              framebuffer[index] = 0xFFFFFFFF; // Branco
            }
          }
        }
      }
    }
  }
}

// Inicializar menus
static bool init_menus(void) {
  // Inicializar contexto de menu
  if (!sdl_menu_init(&g_demo.menu_context, &g_demo.game_renderer)) {
    printf("Erro ao inicializar contexto de menu\n");
    return false;
  }

  // Definir userdata
  g_demo.menu_context.userdata = &g_demo;

  // Menu principal
  g_demo.main_menu = sdl_menu_create("Menu Demo", NULL);
  if (!g_demo.main_menu) {
    printf("Erro ao criar menu principal\n");
    return false;
  }

  // Menu de vídeo
  g_demo.video_menu =
      sdl_menu_create("Configurações de Vídeo", g_demo.main_menu);
  if (!g_demo.video_menu) {
    printf("Erro ao criar menu de vídeo\n");
    sdl_menu_destroy(g_demo.main_menu);
    return false;
  }

  // Menu de áudio
  g_demo.audio_menu =
      sdl_menu_create("Configurações de Áudio", g_demo.main_menu);
  if (!g_demo.audio_menu) {
    printf("Erro ao criar menu de áudio\n");
    sdl_menu_destroy(g_demo.video_menu);
    sdl_menu_destroy(g_demo.main_menu);
    return false;
  }

  // Adicionar itens ao menu principal
  sdl_menu_add_submenu(g_demo.main_menu, "video_menu", "Vídeo",
                       g_demo.video_menu);
  sdl_menu_add_submenu(g_demo.main_menu, "audio_menu", "Áudio",
                       g_demo.audio_menu);
  sdl_menu_add_separator(g_demo.main_menu);
  sdl_menu_add_action(g_demo.main_menu, "exit", "Sair", exit_callback);

  // Adicionar itens ao menu de vídeo
  sdl_menu_add_toggle(g_demo.video_menu, "fullscreen", "Tela Cheia",
                      g_demo.fullscreen, toggle_fullscreen_callback);
  sdl_menu_add_toggle(g_demo.video_menu, "scanlines", "Scanlines",
                      g_demo.scanlines, toggle_scanlines_callback);
  sdl_menu_add_toggle(g_demo.video_menu, "crt", "Efeito CRT", g_demo.crt_effect,
                      toggle_crt_callback);
  sdl_menu_add_toggle(g_demo.video_menu, "smooth", "Escala Suave",
                      g_demo.smooth_scaling, toggle_smooth_scaling_callback);
  sdl_menu_add_slider(g_demo.video_menu, "scale", "Fator de Escala", 10, 50,
                      g_demo.scale_factor, 5, set_scale_factor_callback);

  // Adicionar itens ao menu de áudio
  sdl_menu_add_slider(g_demo.audio_menu, "volume", "Volume", 0, 100,
                      g_demo.volume, 5, set_volume_callback);

  // Criar opções para demonstração
  sdl_menu_choice_option_t quality_options[3] = {
      {"Baixa", 0}, {"Média", 1}, {"Alta", 2}};

  sdl_menu_add_choice(g_demo.audio_menu, "quality", "Qualidade",
                      quality_options, 3, 1, NULL);

  // Ativar menu principal
  sdl_menu_navigate_to(&g_demo.menu_context, g_demo.main_menu);

  return true;
}

// Finalizar menus
static void shutdown_menus(void) {
  if (g_demo.audio_menu) {
    sdl_menu_destroy(g_demo.audio_menu);
    g_demo.audio_menu = NULL;
  }

  if (g_demo.video_menu) {
    sdl_menu_destroy(g_demo.video_menu);
    g_demo.video_menu = NULL;
  }

  if (g_demo.main_menu) {
    sdl_menu_destroy(g_demo.main_menu);
    g_demo.main_menu = NULL;
  }

  sdl_menu_shutdown(&g_demo.menu_context);
}

// Inicializar demonstração
static bool init_demo(void) {
  // Limpar estado
  memset(&g_demo, 0, sizeof(demo_state_t));

  // Inicializar valores padrão
  g_demo.running = true;
  g_demo.menu_visible = false;
  g_demo.scale_factor = 20; // 2.0
  g_demo.volume = 80;       // 80%

  // Inicializar SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
    return false;
  }

  // Configurar renderizador de jogo
  sdl_renderer_config_t config = {.window_width = DEMO_WIDTH,
                                  .window_height = DEMO_HEIGHT,
                                  .game_width = GAME_WIDTH,
                                  .game_height = GAME_HEIGHT,
                                  .scale_factor = 2.0f,
                                  .vsync_enabled = true,
                                  .fullscreen = false,
                                  .smooth_scaling = false,
                                  .integer_scaling = true,
                                  .scanlines_enabled = false,
                                  .crt_effect = false,
                                  .system_name = "MENU_DEMO"};

  if (!sdl_game_renderer_init(&g_demo.game_renderer, &config)) {
    printf("Erro ao inicializar renderizador de jogo\n");
    SDL_Quit();
    return false;
  }

  // Obter janela e renderizador
  g_demo.window = SDL_RenderGetWindow(g_demo.game_renderer.renderer);
  g_demo.renderer = g_demo.game_renderer.renderer;

  // Alocar framebuffer para demonstração
  g_demo.framebuffer =
      (uint32_t *)malloc(GAME_WIDTH * GAME_HEIGHT * sizeof(uint32_t));
  if (!g_demo.framebuffer) {
    printf("Erro ao alocar framebuffer\n");
    sdl_game_renderer_shutdown(&g_demo.game_renderer);
    SDL_Quit();
    return false;
  }

  // Inicializar menus
  if (!init_menus()) {
    printf("Erro ao inicializar menus\n");
    free(g_demo.framebuffer);
    sdl_game_renderer_shutdown(&g_demo.game_renderer);
    SDL_Quit();
    return false;
  }

  return true;
}

// Finalizar demonstração
static void shutdown_demo(void) {
  shutdown_menus();

  if (g_demo.framebuffer) {
    free(g_demo.framebuffer);
    g_demo.framebuffer = NULL;
  }

  sdl_game_renderer_shutdown(&g_demo.game_renderer);
  SDL_Quit();
}

// Loop principal
static void run_demo(void) {
  uint32_t frame_count = 0;

  while (g_demo.running) {
    // Processar eventos
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      // Processar eventos do menu se estiver visível
      if (g_demo.menu_visible) {
        if (sdl_menu_process_event(&g_demo.menu_context, &event)) {
          continue;
        }
      }

      switch (event.type) {
      case SDL_QUIT:
        g_demo.running = false;
        break;

      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          if (g_demo.menu_visible) {
            g_demo.menu_visible = false;
          } else {
            g_demo.running = false;
          }
        } else if (event.key.keysym.sym == SDLK_F12) {
          g_demo.menu_visible = !g_demo.menu_visible;
          sdl_menu_set_visible(&g_demo.menu_context, g_demo.menu_visible);
        }
        break;
      }
    }

    // Gerar frame de demonstração
    generate_demo_frame(g_demo.framebuffer, GAME_WIDTH, GAME_HEIGHT,
                        frame_count);

    // Renderizar frame
    sdl_game_renderer_begin_frame(&g_demo.game_renderer);
    sdl_game_renderer_update_game_texture(&g_demo.game_renderer,
                                          g_demo.framebuffer);
    sdl_game_renderer_draw_frame(&g_demo.game_renderer);

    // Renderizar menu se estiver visível
    if (g_demo.menu_visible) {
      sdl_menu_render(&g_demo.menu_context);
    }

    sdl_game_renderer_end_frame(&g_demo.game_renderer);

    // Controlar taxa de frames
    SDL_Delay(16); // Aproximadamente 60 FPS
    frame_count++;
  }
}

// Função principal
int main(int argc, char *argv[]) {
  printf("Demo do Sistema de Menu SDL\n");
  printf("Pressione F12 para abrir/fechar o menu\n");
  printf("Pressione ESC para sair\n\n");

  if (!init_demo()) {
    printf("Erro ao inicializar demo\n");
    return 1;
  }

  run_demo();

  shutdown_demo();

  printf("Demo finalizada\n");
  return 0;
}
