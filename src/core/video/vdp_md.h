#ifndef VDP_MD_H
#define VDP_MD_H

#include <stdbool.h>
#include <stdint.h>

// Constantes do VDP
#define VDP_VRAM_SIZE 0x10000 // 64KB VRAM
#define VDP_CRAM_SIZE 0x80    // 128 bytes CRAM (64 cores)
#define VDP_VSRAM_SIZE 0x50   // 80 bytes VSRAM
#define VDP_SCREEN_WIDTH 320
#define VDP_SCREEN_HEIGHT 224

// Estrutura de registradores do VDP
typedef struct {
  uint8_t regs[24];   // Registradores de controle
  uint16_t status;    // Registrador de status
  uint16_t data_port; // Porta de dados
  uint16_t ctrl_port; // Porta de controle
  uint32_t addr;      // Endereço atual
  uint8_t code;       // Código de acesso
  bool pending;       // Operação pendente
} vdp_registers_t;

// Estrutura do VDP
typedef struct {
  vdp_registers_t regs;
  uint8_t *vram;
  uint8_t *cram;
  uint8_t *vsram;
  uint32_t *framebuffer;
  bool vblank;
  bool hblank;
  int scanline;
  int cycle;
  void *dma_ctx;
  void (*dma_callback)(void *ctx);
} vdp_md_t;

// Funções de inicialização e controle
void vdp_md_init(vdp_md_t *vdp);
void vdp_md_reset(vdp_md_t *vdp);
void vdp_md_run(vdp_md_t *vdp, int cycles);

// Funções de acesso aos registradores
uint8_t vdp_md_read_reg(vdp_md_t *vdp, int reg);
void vdp_md_write_reg(vdp_md_t *vdp, int reg, uint8_t value);
uint16_t vdp_md_read_status(vdp_md_t *vdp);
uint16_t vdp_md_read_data(vdp_md_t *vdp);
void vdp_md_write_data(vdp_md_t *vdp, uint16_t value);
void vdp_md_write_ctrl(vdp_md_t *vdp, uint16_t value);

// Funções de acesso à memória
uint8_t vdp_md_read_vram(vdp_md_t *vdp, uint16_t addr);
void vdp_md_write_vram(vdp_md_t *vdp, uint16_t addr, uint8_t value);
uint8_t vdp_md_read_cram(vdp_md_t *vdp, uint8_t addr);
void vdp_md_write_cram(vdp_md_t *vdp, uint8_t addr, uint8_t value);
uint8_t vdp_md_read_vsram(vdp_md_t *vdp, uint8_t addr);
void vdp_md_write_vsram(vdp_md_t *vdp, uint8_t addr, uint8_t value);

// Funções de renderização
void vdp_md_render_line(vdp_md_t *vdp);
void vdp_md_render_sprites(vdp_md_t *vdp);
void vdp_md_update_framebuffer(vdp_md_t *vdp);

#endif // VDP_MD_H
