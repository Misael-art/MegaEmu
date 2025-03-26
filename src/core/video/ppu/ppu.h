/**
 * @file ppu.h
 * @brief Interface C para a PPU (Picture Processing Unit)
 *
 * Este arquivo define a interface pública da implementação da PPU,
 * seguindo o padrão de arquitetura híbrida para permitir compatibilidade entre
 * C e C++.
 */

#ifndef EMU_PPU_H
#define EMU_PPU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tipo de PPU
 */
typedef enum {
  PPU_TYPE_NES = 0, /**< PPU do NES (Ricoh 2C02) */
  PPU_TYPE_SNES,    /**< PPU do SNES */
  PPU_TYPE_SMS_GG,  /**< PPU do Master System/Game Gear (VDP) */
  PPU_TYPE_GENESIS, /**< VDP do Mega Drive/Genesis */
  PPU_TYPE_GB,      /**< PPU do Game Boy */
  PPU_TYPE_GBA,     /**< PPU do Game Boy Advance */
  PPU_TYPE_CUSTOM   /**< PPU customizada */
} ppu_type_t;

/**
 * @brief Formato de pixel para o frame buffer
 */
typedef enum {
  PPU_PIXEL_FORMAT_RGB565 = 0, /**< RGB565 (16 bits por pixel) */
  PPU_PIXEL_FORMAT_RGB888,     /**< RGB888 (24 bits por pixel) */
  PPU_PIXEL_FORMAT_RGBA8888    /**< RGBA8888 (32 bits por pixel) */
} ppu_pixel_format_t;

/**
 * @brief Tipo de entrelace
 */
typedef enum {
  PPU_INTERLACE_NONE = 0, /**< Sem entrelace */
  PPU_INTERLACE_ODD,      /**< Linhas ímpares */
  PPU_INTERLACE_EVEN      /**< Linhas pares */
} ppu_interlace_mode_t;

/**
 * @brief Configuração da PPU
 */
typedef struct {
  ppu_type_t type;                 /**< Tipo de PPU */
  ppu_pixel_format_t pixel_format; /**< Formato de pixel do framebuffer */
  uint16_t width;                  /**< Largura da tela em pixels */
  uint16_t height;                 /**< Altura da tela em pixels */
  uint16_t visible_width;          /**< Largura visível em pixels */
  uint16_t visible_height;         /**< Altura visível em pixels */
  bool double_width;  /**< Largura dobrada (modo high-res do SNES) */
  bool double_height; /**< Altura dobrada (modo entrelace) */
  ppu_interlace_mode_t interlace; /**< Modo de entrelace */
  void *framebuffer;        /**< Buffer para renderização (pode ser NULL) */
  size_t framebuffer_pitch; /**< Pitch do framebuffer em bytes */
  bool enable_sprite_limit; /**< Habilitar limite de sprites por linha */
  bool enable_master_brightness; /**< Habilitar controle master de brilho */
} ppu_config_t;

/**
 * @brief Tipo opaco para o contexto da PPU
 */
typedef struct ppu_context_s ppu_t;

/**
 * @brief Callback para leitura de memória
 *
 * @param context Contexto do usuário
 * @param address Endereço a ler
 * @return Byte lido
 */
typedef uint8_t (*ppu_read_callback_t)(void *context, uint16_t address);

/**
 * @brief Callback para escrita em memória
 *
 * @param context Contexto do usuário
 * @param address Endereço para escrita
 * @param value Valor a escrever
 */
typedef void (*ppu_write_callback_t)(void *context, uint16_t address,
                                     uint8_t value);

/**
 * @brief Callback para notificação de scanline finalizada
 *
 * @param context Contexto do usuário
 * @param scanline Número da scanline
 */
typedef void (*ppu_scanline_callback_t)(void *context, int scanline);

/**
 * @brief Callback para notificação de frame finalizado
 *
 * @param context Contexto do usuário
 * @param framebuffer Ponteiro para o framebuffer atual
 * @param width Largura do framebuffer
 * @param height Altura do framebuffer
 * @param pitch Pitch do framebuffer em bytes
 */
typedef void (*ppu_frame_callback_t)(void *context, void *framebuffer,
                                     int width, int height, int pitch);

/**
 * @brief Estrutura estendida de configuração com callbacks
 */
typedef struct {
  ppu_config_t config;             /**< Configuração básica */
  ppu_read_callback_t read_vram;   /**< Callback para leitura de VRAM */
  ppu_write_callback_t write_vram; /**< Callback para escrita em VRAM */
  ppu_read_callback_t read_oam;    /**< Callback para leitura de OAM */
  ppu_write_callback_t write_oam;  /**< Callback para escrita em OAM */
  ppu_read_callback_t read_cgram;  /**< Callback para leitura de CGRAM/paleta */
  ppu_write_callback_t
      write_cgram; /**< Callback para escrita em CGRAM/paleta */
  ppu_scanline_callback_t
      scanline_callback;               /**< Callback para scanline completa */
  ppu_frame_callback_t frame_callback; /**< Callback para frame completo */
  void *context; /**< Contexto do usuário para callbacks */
} ppu_full_config_t;

/**
 * @brief Cria uma nova instância da PPU
 *
 * @param config Configuração da PPU
 * @return Instância da PPU ou NULL em caso de erro
 */
ppu_t *ppu_create(const ppu_full_config_t *config);

/**
 * @brief Destrói uma instância da PPU e libera recursos
 *
 * @param ppu Instância da PPU
 */
void ppu_destroy(ppu_t *ppu);

/**
 * @brief Reseta a PPU para estado inicial
 *
 * @param ppu Instância da PPU
 */
void ppu_reset(ppu_t *ppu);

/**
 * @brief Define o framebuffer para renderização
 *
 * @param ppu Instância da PPU
 * @param framebuffer Ponteiro para o framebuffer
 * @param pitch Pitch do framebuffer em bytes
 * @return 0 em caso de sucesso ou -1 em caso de erro
 */
int ppu_set_framebuffer(ppu_t *ppu, void *framebuffer, size_t pitch);

/**
 * @brief Executa a PPU por um número específico de ciclos
 *
 * @param ppu Instância da PPU
 * @param cycles Número de ciclos a executar
 * @return Número de ciclos realmente executados
 */
int ppu_execute(ppu_t *ppu, int cycles);

/**
 * @brief Executa até completar um scanline
 *
 * @param ppu Instância da PPU
 * @return Número de ciclos consumidos
 */
int ppu_execute_scanline(ppu_t *ppu);

/**
 * @brief Executa até completar um frame
 *
 * @param ppu Instância da PPU
 * @return Número de ciclos consumidos
 */
int ppu_execute_frame(ppu_t *ppu);

/**
 * @brief Obtém o scanline atual
 *
 * @param ppu Instância da PPU
 * @return Número do scanline atual
 */
int ppu_get_scanline(const ppu_t *ppu);

/**
 * @brief Obtém o ciclo atual dentro do scanline
 *
 * @param ppu Instância da PPU
 * @return Ciclo atual dentro do scanline
 */
int ppu_get_cycle(const ppu_t *ppu);

/**
 * @brief Obtém o contador de frames
 *
 * @param ppu Instância da PPU
 * @return Contador de frames
 */
uint32_t ppu_get_frame_count(const ppu_t *ppu);

/**
 * @brief Verifica se está ocorrendo vblank
 *
 * @param ppu Instância da PPU
 * @return true se em vblank, false caso contrário
 */
bool ppu_in_vblank(const ppu_t *ppu);

/**
 * @brief Lê um registrador da PPU
 *
 * @param ppu Instância da PPU
 * @param reg_id ID do registrador
 * @return Valor do registrador
 */
uint8_t ppu_read_register(ppu_t *ppu, uint16_t reg_id);

/**
 * @brief Escreve em um registrador da PPU
 *
 * @param ppu Instância da PPU
 * @param reg_id ID do registrador
 * @param value Valor a escrever
 */
void ppu_write_register(ppu_t *ppu, uint16_t reg_id, uint8_t value);

/**
 * @brief Salva o estado da PPU em um buffer
 *
 * @param ppu Instância da PPU
 * @param buffer Buffer para armazenar estado (deve ter tamanho suficiente)
 * @param buffer_size Tamanho do buffer
 * @return Número de bytes escritos ou -1 em caso de erro
 */
int ppu_save_state(const ppu_t *ppu, uint8_t *buffer, size_t buffer_size);

/**
 * @brief Carrega o estado da PPU de um buffer
 *
 * @param ppu Instância da PPU
 * @param buffer Buffer contendo estado salvo
 * @param buffer_size Tamanho do buffer
 * @return 0 em caso de sucesso ou -1 em caso de erro
 */
int ppu_load_state(ppu_t *ppu, const uint8_t *buffer, size_t buffer_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* EMU_PPU_H */
