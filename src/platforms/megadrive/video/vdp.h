/**
 * @file vdp.h
 * @brief Definições e protótipos para o VDP do Mega Drive
 */

#ifndef EMU_VDP_H
#define EMU_VDP_H

#include <stdint.h>

// Tamanhos de memória
#define VDP_VRAM_SIZE 65536 // 64KB
#define VDP_CRAM_SIZE 128   // 64 words
#define VDP_VSRAM_SIZE 80   // 40 words

// Registradores do VDP
#define VDP_REG_MODE1 0x00      // Modo de controle 1
#define VDP_REG_MODE2 0x01      // Modo de controle 2
#define VDP_REG_PLANE_A 0x02    // Endereço base do plano A
#define VDP_REG_WINDOW 0x03     // Endereço base da janela
#define VDP_REG_PLANE_B 0x04    // Endereço base do plano B
#define VDP_REG_SPRITE 0x05     // Endereço base dos atributos de sprite
#define VDP_REG_BG_COLOR 0x07   // Cor de fundo (registro 7)
#define VDP_REG_HINT 0x0A       // Valor do contador de interrupção H
#define VDP_REG_MODE3 0x0B      // Modo de controle 3
#define VDP_REG_MODE4 0x0C      // Modo de controle 4
#define VDP_REG_HSCROLL 0x0D    // Endereço base do scroll horizontal
#define VDP_REG_BG_ADDR 0x0E    // Endereço base dos padrões de plano/sprite
#define VDP_REG_AUTO_INC 0x0F   // Valor de auto-incremento
#define VDP_REG_PLANE_SIZE 0x10 // Tamanho dos planos
#define VDP_REG_WIN_H 0x11      // Posição horizontal da janela
#define VDP_REG_WIN_V 0x12      // Posição vertical da janela

// Bits do registrador de status
#define VDP_STATUS_PAL 0x0001        // 0=NTSC, 1=PAL
#define VDP_STATUS_DMA 0x0002        // DMA em andamento
#define VDP_STATUS_HBLANK 0x0004     // Em H-blank
#define VDP_STATUS_VBLANK 0x0008     // Em V-blank
#define VDP_STATUS_ODD 0x0010        // Campo ímpar (modo entrelaçado)
#define VDP_STATUS_COLLISION 0x0020  // Colisão de sprites detectada
#define VDP_STATUS_SOVR 0x0040       // Overflow de sprites na linha
#define VDP_STATUS_VINT 0x0080       // Interrupção vertical pendente
#define VDP_STATUS_FIFO_FULL 0x0100  // FIFO de escrita cheia
#define VDP_STATUS_FIFO_EMPTY 0x0200 // FIFO de escrita vazia

// Modos de acesso à VRAM
typedef enum {
  VDP_ACCESS_VRAM_READ,   // Leitura da VRAM
  VDP_ACCESS_VRAM_WRITE,  // Escrita na VRAM
  VDP_ACCESS_CRAM_WRITE,  // Escrita na CRAM
  VDP_ACCESS_VSRAM_WRITE, // Escrita na VSRAM
  VDP_ACCESS_CRAM_READ,   // Leitura da CRAM
  VDP_ACCESS_VSRAM_READ   // Leitura da VSRAM
} vdp_access_mode_t;

// Estrutura para dados de padrão de tile
typedef struct {
  uint8_t data[32]; // 8x8 pixels, 4 bits por pixel
} vdp_pattern_t;

// Funções de acesso à memória
uint16_t vdp_read_vram(uint32_t addr);
void vdp_write_vram(uint32_t addr, uint16_t data);
uint16_t vdp_read_cram(uint16_t addr);
void vdp_write_cram(uint16_t addr, uint16_t data);
uint16_t vdp_read_vsram(uint16_t addr);
void vdp_write_vsram(uint16_t addr, uint16_t data);

// Funções de acesso a padrões
uint8_t *vdp_get_pattern_data(uint16_t pattern_index);
void vdp_write_pattern_data(uint16_t pattern_index, const uint8_t *data);

// Funções de renderização
void vdp_write_line_buffer(uint16_t line, const uint8_t *buffer);
void vdp_render_pattern(uint16_t pattern_index, int16_t x, int16_t y,
                        uint8_t palette, uint8_t priority, uint8_t flip_h,
                        uint8_t flip_v);

// Funções de controle
void vdp_init(void);
void vdp_reset(void);
void vdp_write_register(uint8_t reg, uint8_t value);
uint8_t vdp_read_register(uint8_t reg);
void vdp_write_control(uint16_t value);
uint16_t vdp_read_status(void);
void vdp_write_data(uint16_t value);
uint16_t vdp_read_data(void);

// Funções de processamento
void vdp_run_scanline(void);
void vdp_end_frame(void);

// Funções de consulta de estado
/**
 * @brief Verifica se o VDP está em período de VBLANK
 * @return 1 se estiver em VBLANK, 0 caso contrário
 */
uint8_t vdp_in_vblank(void);

/**
 * @brief Verifica se o VDP está em período de HBLANK
 * @return 1 se estiver em HBLANK, 0 caso contrário
 */
uint8_t vdp_in_hblank(void);

/**
 * @brief Obtém a linha atual sendo processada
 * @return Número da linha atual (0-262 NTSC, 0-312 PAL)
 */
uint16_t vdp_get_line(void);

/**
 * @brief Obtém o modo atual do VDP
 * @return Bits de modo:
 *         bit 0: H40 (1=40 células, 0=32 células)
 *         bit 1: Mode 5 (1=Mode 5, 0=Mode 4)
 *         bit 2: H80 (1=80 células)
 *         bit 3: Interlace (1=ativo)
 *         bit 4: Shadow/Highlight (1=ativo)
 */
uint8_t vdp_get_mode(void);

/**
 * @brief Verifica se o VDP está em modo PAL
 * @return 1 se PAL, 0 se NTSC
 */
uint8_t vdp_is_pal(void);

/**
 * @brief Verifica se o VDP está em Mode 5
 * @return 1 se Mode 5, 0 se Mode 4
 */
uint8_t vdp_is_mode5(void);

/**
 * @brief Obtém o modo de interlace atual
 * @return 0=Não interlaceado, 1=Interlace normal, 2=Interlace double resolution
 */
uint8_t vdp_get_interlace(void);

#endif // EMU_VDP_H
