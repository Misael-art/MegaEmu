/**
 * @file core.h
 * @brief Definições principais do núcleo do emulador
 * @author Mega_Emu Team
 * @version 1.1.0
 * @date 2024-03-12
 */

#ifndef EMU_CORE_H
#define EMU_CORE_H

#include "../utils/common_types.h"
#include "../utils/enhanced_log.h"
#include "../utils/error_handling.h"
#include "core_types.h"
#include "memory/memory_interface.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef EMU_CORE_INTERNAL
#define EMU_CORE_API
#else
#ifdef _WIN32
#ifdef EMU_CORE_EXPORT
#define EMU_CORE_API __declspec(dllexport)
#else
#define EMU_CORE_API __declspec(dllimport)
#endif
#else
#define EMU_CORE_API __attribute__((visibility("default")))
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct emu_video_instance;
struct emu_audio_instance;
struct emu_input_instance;
struct emu_platform_instance;
struct emu_core_instance;

typedef struct emu_video_instance emu_video_instance_t;
typedef struct emu_audio_instance emu_audio_instance_t;
typedef struct emu_input_instance emu_input_instance_t;
typedef struct emu_platform_instance emu_platform_instance_t;
typedef struct emu_core_instance emu_core_instance_t;

/* Estrutura de informações da plataforma */
typedef struct {
  const char *name;        /* Nome da plataforma */
  const char *description; /* Descrição da plataforma */
  const char *version;     /* Versão da plataforma */
  uint32_t flags;          /* Flags da plataforma */
} emu_platform_info_data_t;

/* Estrutura de funções da plataforma */
typedef struct {
  bool (*init)(emu_platform_instance_t *platform);
  void (*shutdown)(emu_platform_instance_t *platform);
  bool (*reset)(emu_platform_instance_t *platform);
  bool (*frame)(emu_platform_instance_t *platform);
  bool (*render)(emu_platform_instance_t *platform);
  bool (*input)(emu_platform_instance_t *platform, uint32_t input);
} emu_platform_functions_t;

/* Estrutura de instância da plataforma */
struct emu_platform_instance {
  emu_platform_info_data_t info;
  emu_platform_functions_t functions;
  emu_memory_instance_t *memory;
  emu_video_instance_t *video;
  emu_audio_instance_t *audio;
  emu_input_instance_t *input;
  void *user_data;
};

/* Estrutura de instância do core */
struct emu_core_instance {
  emu_platform_instance_t *platform;
  emu_memory_instance_t *memory;
  emu_video_instance_t *video;
  emu_audio_instance_t *audio;
  emu_input_instance_t *input;
  void *cpu;
  void *user_data;
};

/* Estrutura que representa o estado do core do emulador */
typedef struct {
  void *cpu;       // Interface genérica da CPU
  void *ppu;       // Interface genérica do PPU
  void *memory;    // Interface genérica da memória
  bool running;    // Flag que indica se o emulador está rodando
  uint64_t cycles; // Contador de ciclos
} emu_core_t;

/* Funções de interface */
EMU_CORE_API emu_core_t *emu_core_create(void);
EMU_CORE_API void emu_core_destroy(emu_core_t *core);
EMU_CORE_API bool emu_core_init(emu_core_t *core);
EMU_CORE_API void emu_core_shutdown(emu_core_t *core);
EMU_CORE_API void emu_core_reset(emu_core_t *core);

/* Funções de plataforma */
bool emu_core_load_platform(emu_core_instance_t *core,
                            const char *platform_name);
void emu_core_unload_platform(emu_core_instance_t *core);

/* Funções de execução */
bool emu_core_frame(emu_core_instance_t *core);
bool emu_core_render(emu_core_instance_t *core);
bool emu_core_input(emu_core_instance_t *core, uint32_t input);

/* Funções de configuração */
bool emu_core_set_video_mode(emu_core_instance_t *core, uint32_t width,
                             uint32_t height, uint32_t format);
bool emu_core_set_audio_mode(emu_core_instance_t *core, uint32_t sample_rate,
                             uint32_t channels);
bool emu_core_set_input_mode(emu_core_instance_t *core, uint32_t type);

/* Definições de tipos */
typedef struct {
  uint8_t r, g, b, a;
} emu_color_t;

typedef struct {
  int32_t x, y, width, height;
} emu_rect_t;

/* Códigos de erro são definidos em error_handling.h */
typedef emu_error_t core_error_t;

/* Tipos de plataforma suportados */
typedef enum {
  EMU_PLATFORM_MEGADRIVE = 0,
  EMU_PLATFORM_NES,
  EMU_PLATFORM_SNES,
  EMU_PLATFORM_MASTERSYSTEM,
  EMU_PLATFORM_COUNT
} emu_platform_type_t;

/* Funções principais do core */

/**
 * @brief Verifica se o núcleo foi inicializado
 * @return true se inicializado, false caso contrário
 */
bool emu_core_is_initialized(void);

/**
 * @brief Obtém a versão do emulador
 * @return String com a versão
 */
const char *emu_core_get_version(void);

/**
 * @brief Configura o nível de log do emulador
 * @param level Nível de log (0=desativado, 1=erros, 2=avisos, 3=info, 4=debug)
 */
void emu_core_set_log_level(int32_t level);

/**
 * @brief Seleciona a plataforma atual
 * @param platform_type Tipo de plataforma
 * @return EMU_SUCCESS em caso de sucesso ou código de erro
 */
emu_error_t emu_core_select_platform(emu_platform_type_t platform_type);

/**
 * @brief Obtém a plataforma atual selecionada
 * @return Tipo de plataforma atual
 */
emu_platform_type_t emu_core_get_current_platform(void);

/**
 * @brief Interface para carregar uma ROM
 * @param filename Caminho para o arquivo de ROM
 * @return EMU_SUCCESS em caso de sucesso ou código de erro
 */
emu_error_t emu_core_load_rom(const char *filename);

/**
 * @brief Interface para executar um quadro de emulação
 * @return EMU_SUCCESS em caso de sucesso ou código de erro
 */
emu_error_t emu_core_run_frame(void);

/**
 * @brief Interface para obter o buffer de vídeo atual
 * @return Ponteiro para o buffer de vídeo
 */
const uint32_t *emu_core_get_video_buffer(void);

/**
 * @brief Interface para obter as dimensões da tela
 * @param width Ponteiro para a largura da tela
 * @param height Ponteiro para a altura da tela
 */
void emu_core_get_screen_dimensions(int32_t *width, int32_t *height);

/**
 * @brief Interface para atualizar o estado dos controles
 * @param controller_data Dados do controle (específico da plataforma)
 * @param controller_index Índice do controle (0 para jogador 1)
 */
void emu_core_update_controller(const void *controller_data,
                                int32_t controller_index);

/* Funções de execução */
EMU_CORE_API void emu_core_run(emu_core_t *core);
EMU_CORE_API void emu_core_stop(emu_core_t *core);
EMU_CORE_API void emu_core_step(emu_core_t *core);

/* Funções de acesso aos componentes */
EMU_CORE_API void *emu_core_get_cpu(emu_core_t *core);
EMU_CORE_API void *emu_core_get_ppu(emu_core_t *core);
EMU_CORE_API void *emu_core_get_memory(emu_core_t *core);

/* Funções de estado */
EMU_CORE_API bool emu_core_is_running(emu_core_t *core);
EMU_CORE_API uint64_t emu_core_get_cycles(emu_core_t *core);

#ifdef __cplusplus
}
#endif

#endif /* EMU_CORE_H */
