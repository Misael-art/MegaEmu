/**
 * @file vdp_adapter.h
 * @brief Adaptador de vídeo VDP para o Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#ifndef MEGADRIVE_VDP_ADAPTER_H
#define MEGADRIVE_VDP_ADAPTER_H

#include "core/interfaces/video_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Constantes do VDP
 */
#define MD_VDP_VRAM_SIZE 0x10000 // 64KB VRAM
#define MD_VDP_CRAM_SIZE 0x40    // 64B CRAM (32 cores)
#define MD_VDP_VSRAM_SIZE 0x40   // 64B VSRAM
#define MD_VDP_REG_COUNT 0x18    // 24 registradores
#define MD_VDP_SPRITE_COUNT 0x50 // 80 sprites
#define MD_VDP_SPRITE_SIZE 8     // 8 bytes por sprite

/**
 * @brief Modos de vídeo do VDP
 */
typedef enum {
  MD_VDP_MODE_H32_V28 = 0, // 256x224
  MD_VDP_MODE_H32_V30,     // 256x240
  MD_VDP_MODE_H40_V28,     // 320x224
  MD_VDP_MODE_H40_V30      // 320x240
} md_vdp_mode_t;

/**
 * @brief Modos de acesso ao VDP
 */
typedef enum {
  MD_VDP_ACCESS_VRAM_READ = 0,
  MD_VDP_ACCESS_VRAM_WRITE,
  MD_VDP_ACCESS_CRAM_WRITE,
  MD_VDP_ACCESS_VSRAM_WRITE,
  MD_VDP_ACCESS_CRAM_READ,
  MD_VDP_ACCESS_VSRAM_READ
} md_vdp_access_t;

/**
 * @brief Estrutura de sprite
 */
typedef struct {
  uint16_t y;       // Posição Y
  uint8_t size;     // Tamanho (altura/largura)
  uint16_t link;    // Link para próximo sprite
  uint16_t attr;    // Atributos (flip, paleta, prioridade)
  uint16_t x;       // Posição X
  uint16_t pattern; // Padrão do sprite
} md_vdp_sprite_t;

/**
 * @brief Contexto específico do adaptador VDP
 */
typedef struct {
  // Memórias
  uint8_t vram[MD_VDP_VRAM_SIZE];        // Video RAM
  uint16_t cram[MD_VDP_CRAM_SIZE / 2];   // Color RAM
  uint16_t vsram[MD_VDP_VSRAM_SIZE / 2]; // Vertical Scroll RAM
  uint8_t regs[MD_VDP_REG_COUNT];        // Registradores

  // Estado do VDP
  md_vdp_mode_t mode;          // Modo de vídeo atual
  md_vdp_access_t access_mode; // Modo de acesso
  uint16_t status;             // Registro de status
  uint32_t address;            // Endereço de acesso
  uint16_t code;               // Código de controle
  bool first_byte;             // Flag de primeiro byte

  // Contadores
  uint16_t hcounter;    // Contador horizontal
  uint16_t vcounter;    // Contador vertical
  uint32_t frame_count; // Contador de frames

  // DMA
  bool dma_enabled;    // DMA habilitado
  uint32_t dma_source; // Endereço fonte DMA
  uint16_t dma_length; // Comprimento DMA
  uint8_t dma_type;    // Tipo de DMA

  // Sprites
  uint8_t sprite_count;  // Número de sprites
  bool sprite_collision; // Colisão detectada
  bool sprite_overflow;  // Overflow detectado

  // Interrupções
  bool vint_pending;    // Interrupção vertical pendente
  bool hint_pending;    // Interrupção horizontal pendente
  uint8_t hint_counter; // Contador de interrupção H
  uint8_t hint_value;   // Valor de comparação H

  // Callbacks
  void (*vint_callback)(void *); // Callback de VINT
  void (*hint_callback)(void *); // Callback de HINT
  void *callback_data;           // Dados para callbacks

  // Dados específicos
  void *user_data; // Dados da implementação
} megadrive_vdp_context_t;

/**
 * @brief Cria uma nova instância do adaptador VDP
 * @return Ponteiro para a interface de vídeo ou NULL em caso de erro
 */
emu_video_interface_t *megadrive_vdp_adapter_create(void);

/**
 * @brief Destrói uma instância do adaptador VDP
 * @param video Ponteiro para a interface de vídeo
 */
void megadrive_vdp_adapter_destroy(emu_video_interface_t *video);

/**
 * @brief Obtém o contexto específico do adaptador VDP
 * @param video Ponteiro para a interface de vídeo
 * @return Ponteiro para o contexto ou NULL em caso de erro
 */
megadrive_vdp_context_t *
megadrive_vdp_get_context(emu_video_interface_t *video);

/**
 * @brief Define o contexto específico do adaptador VDP
 * @param video Ponteiro para a interface de vídeo
 * @param context Ponteiro para o novo contexto
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int megadrive_vdp_set_context(emu_video_interface_t *video,
                              const megadrive_vdp_context_t *context);

/**
 * @brief Registra callbacks para interrupções
 * @param video Ponteiro para a interface de vídeo
 * @param vint_cb Callback para interrupção vertical
 * @param hint_cb Callback para interrupção horizontal
 * @param user_data Dados do usuário para os callbacks
 */
void megadrive_vdp_set_interrupt_callbacks(emu_video_interface_t *video,
                                           void (*vint_cb)(void *user_data),
                                           void (*hint_cb)(void *user_data),
                                           void *user_data);

// Funções de renderização
void vdp_render_line(megadrive_vdp_context_t *ctx, int line, uint8_t *output);

// Funções de DMA
void vdp_dma_execute(megadrive_vdp_context_t *ctx);
void vdp_dma_start(megadrive_vdp_context_t *ctx, uint8_t code, uint16_t address,
                   uint32_t source, uint16_t length);
bool vdp_dma_is_active(const megadrive_vdp_context_t *ctx);
void vdp_dma_set_memory_callback(megadrive_vdp_context_t *ctx,
                                 uint8_t (*read_cb)(uint32_t addr,
                                                    void *user_data),
                                 void *user_data);

// Funções de processamento de registradores
void vdp_write_register(megadrive_vdp_context_t *ctx, uint8_t reg,
                        uint8_t value);
uint8_t vdp_read_register(const megadrive_vdp_context_t *ctx, uint8_t reg);
void vdp_get_plane_info(const megadrive_vdp_context_t *ctx, uint8_t plane,
                        uint16_t *base_addr, uint8_t *width, uint8_t *height);
void vdp_get_sprite_info(const megadrive_vdp_context_t *ctx,
                         uint16_t *table_addr, uint8_t *max_sprites);
void vdp_get_scroll_info(const megadrive_vdp_context_t *ctx,
                         uint16_t *hscroll_addr, uint8_t *hscroll_mode,
                         uint8_t *vscroll_mode);

#ifdef __cplusplus
}
#endif

#endif // MEGADRIVE_VDP_ADAPTER_H
