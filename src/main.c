/** * @file main.c * @brief Ponto de entrada principal do emulador Mega_Emu */           \
#include<stdio.h> #include<stdlib.h> #include<string.h> #include<SDL2 / SDL.h> #include< \
    lua.h> #include<lauxlib                                                              \
                        .h> #include<lualib                                              \
                                         .h> #include<stdbool                            \
                                                          .h> // Definir
                                                              // SDL_MAIN_HANDLED
                                                              // antes de
                                                              // incluir
                                                              // SDL.h#define
                                                              // SDL_MAIN_HANDLED#include
                                                              // "utils/error_handling.h"#include
                                                              // "utils/enhanced_log.h"#include
                                                              // "frontend/sdl/nes_window.h"#define
                                                              // MAX_PATH_LENGTH
                                                              // 512// Estrutura
                                                              // principal do
                                                              // emuladortypedef
                                                              // struct {    //
                                                              // CPUs    m68k_t
                                                              // m68k;    z80_t
                                                              // z80;    r6502_t
                                                              // r6502;     //
                                                              // Vídeo vdp_md_t
                                                              // vdp_md;
                                                              // vdp_sms_t
                                                              // vdp_sms;
                                                              // ppu_nes_t
                                                              // ppu_nes;     //
                                                              // Áudio ym2612_t
                                                              // ym2612;
                                                              // sn76489_t
                                                              // sn76489;
                                                              // apu_nes_t
                                                              // apu_nes;     //
                                                              // Memória
                                                              // memory_t
                                                              // memory;     //
                                                              // Sistema de
                                                              // scripts
                                                              // lua_State* lua;
                                                              // // SDL
                                                              // SDL_Window*
                                                              // window;
                                                              // SDL_Renderer*
                                                              // renderer;
                                                              // SDL_Texture*
                                                              // texture;
                                                              // SDL_AudioDeviceID
                                                              // audio_device;
                                                              // // Estado bool
                                                              // running; bool
                                                              // paused;    int
                                                              // system;  // 0 =
                                                              // MD, 1 = SMS, 2
                                                              // = NES}
                                                              // emulator_t;

#include "platforms/megadrive/megadrive.h"
#include "frontend/sdl/sdl_frontend.h"

#define WINDOW_SCALE 2
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

// Funções de inicialização
static bool init_sdl(emulator_t *emu) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
    return false;
  }

  emu->window = SDL_CreateWindow("Mega_Emu", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, 640, 480,
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!emu->window) {
    printf("Erro ao criar janela: %s\n", SDL_GetError());
    return false;
  }

  emu->renderer = SDL_CreateRenderer(
      emu->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!emu->renderer) {
    printf("Erro ao criar renderer: %s\n", SDL_GetError());
    return false;
  }

  emu->texture = SDL_CreateTexture(emu->renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, 320, 240);
  if (!emu->texture) {
    printf("Erro ao criar textura: %s\n", SDL_GetError());
    return false;
  }

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = 44100;
  want.format = AUDIO_S16SYS;
  want.channels = 2;
  want.samples = 2048;
  want.callback = NULL;

  emu->audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (!emu->audio_device) {
    printf("Erro ao abrir dispositivo de áudio: %s\n", SDL_GetError());
    return false;
  }

  SDL_PauseAudioDevice(emu->audio_device, 0);
  return true;
}

static bool init_lua(emulator_t *emu) {
  emu->lua = luaL_newstate();
  if (!emu->lua) {
    printf("Erro ao criar estado Lua\n");
    return false;
  }

  luaL_openlibs(emu->lua);
  script_system_register(emu->lua, emu);
  return true;
}

static void cleanup(emulator_t *emu) {
  if (emu->lua)
    lua_close(emu->lua);
  if (emu->audio_device)
    SDL_CloseAudioDevice(emu->audio_device);
  if (emu->texture)
    SDL_DestroyTexture(emu->texture);
  if (emu->renderer)
    SDL_DestroyRenderer(emu->renderer);
  if (emu->window)
    SDL_DestroyWindow(emu->window);
  SDL_Quit();
}

static void handle_events(emulator_t *emu) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      emu->running = false;
      break;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        emu->running = false;
        break;
      case SDLK_SPACE:
        emu->paused = !emu->paused;
        break;
      }
      break;
    }
  }
}

static void update_audio(emulator_t *emu) {
  static int16_t buffer[4096];

  switch (emu->system) {
  case 0: // Mega Drive
    ym2612_update(&emu->ym2612, buffer, 2048);
    sn76489_mix_output(&emu->sn76489, buffer, 2048);
    break;
  case 1: // Master System
    sn76489_update(&emu->sn76489, buffer, 2048);
    break;
  case 2: // NES
    apu_nes_mix_output(&emu->apu_nes, buffer, 2048);
    break;
  }

  SDL_QueueAudio(emu->audio_device, buffer, 4096);
}

static void update_video(emulator_t *emu) {
  void *pixels;
  int pitch;
  SDL_LockTexture(emu->texture, NULL, &pixels, &pitch);

  switch (emu->system) {
  case 0: // Mega Drive
    memcpy(pixels, emu->vdp_md.framebuffer, 320 * 240 * 4);
    break;
  case 1: // Master System
    memcpy(pixels, emu->vdp_sms.framebuffer, 256 * 192 * 4);
    break;
  case 2: // NES
    memcpy(pixels, emu->ppu_nes.framebuffer, 256 * 240 * 4);
    break;
  }

  SDL_UnlockTexture(emu->texture);
  SDL_RenderClear(emu->renderer);
  SDL_RenderCopy(emu->renderer, emu->texture, NULL, NULL);
  SDL_RenderPresent(emu->renderer);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Uso: %s <arquivo_rom>\n", argv[0]);
    return 1;
  }

  // Inicializa plataforma
  MegaDrive* md = (MegaDrive*)malloc(sizeof(MegaDrive));
  if (!megadrive_init(md)) {
    printf("Erro ao inicializar Mega Drive\n");
    return 1;
  }

  // Carrega ROM
  if (!megadrive_load_rom(md, argv[1])) {
    printf("Erro ao carregar ROM: %s\n", argv[1]);
    megadrive_destroy(md);
    return 1;
  }

  // Inicializa frontend SDL
  SDLFrontend frontend;
  if (!sdl_frontend_init(&frontend, "Mega Drive Emulator", WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_SCALE)) {
    printf("Erro ao inicializar SDL frontend\n");
    megadrive_destroy(md);
    return 1;
  }

  // Loop principal
  while (sdl_frontend_handle_events(&frontend)) {
    // Executa um frame do emulador
    megadrive_run_frame(md);

    // Atualiza a tela
    sdl_frontend_update(&frontend, md->ppu->framebuffer, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Limita a taxa de frames
    SDL_Delay(16); // ~60 FPS
  }

  // Cleanup
  sdl_frontend_destroy(&frontend);
  megadrive_destroy(md);
  free(md);

  return 0;
}
