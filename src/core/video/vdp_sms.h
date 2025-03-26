#ifndef VDP_SMS_H
#define VDP_SMS_H

#include <stdbool.h>
#include <stdint.h>

// Constantes do VDP
#define VDP_VRAM_SIZE 0x4000 // 16KB VRAM
#define VDP_CRAM_SIZE 0x20   // 32 bytes CRAM (32 cores)
#define VDP_SCREEN_WIDTH 256
#define VDP_SCREEN_HEIGHT 192

// Estrutura de registradores do VDP
typedef struct {
  uint8_t regs[16];  // Registradores de controle
  uint8_t status;    // Registrador de status
  uint8_t data_port; // Porta de dados
  uint8_t ctrl_port; // Porta de controle
  uint16_t addr;     // Endereço atual
  bool first_byte;   // Primeiro byte do comando
  uint8_t latch;     // Latch de dados
} vdp_registers_t;

// Estrutura do VDP
typedef struct {
  vdp_registers_t regs;
  uint8_t *vram;
  uint8_t *cram;
  uint32_t *framebuffer;
  bool vblank;
  bool hblank;
  int scanline;
  int cycle;
  bool mode_4;   // Modo 4 (SMS) vs Modo TMS9918 (SG-1000)
  bool pal_mode; // PAL vs NTSC
} vdp_sms_t;

// Funções de inicialização e controle
void vdp_sms_init(vdp_sms_t *vdp);
void vdp_sms_reset(vdp_sms_t *vdp);
void vdp_sms_run(vdp_sms_t *vdp, int cycles);

// Funções de acesso aos registradores
uint8_t vdp_sms_read_reg(vdp_sms_t *vdp, int reg);
void vdp_sms_write_reg(vdp_sms_t *vdp, int reg, uint8_t value);
uint8_t vdp_sms_read_status(vdp_sms_t *vdp);
uint8_t vdp_sms_read_data(vdp_sms_t *vdp);
void vdp_sms_write_data(vdp_sms_t *vdp, uint8_t value);
void vdp_sms_write_ctrl(vdp_sms_t *vdp, uint8_t value);

// Funções de acesso à memória
uint8_t vdp_sms_read_vram(vdp_sms_t *vdp, uint16_t addr);
void vdp_sms_write_vram(vdp_sms_t *vdp, uint16_t addr, uint8_t value);
uint8_t vdp_sms_read_cram(vdp_sms_t *vdp, uint8_t addr);
void vdp_sms_write_cram(vdp_sms_t *vdp, uint8_t addr, uint8_t value);

// Funções de renderização
void vdp_sms_render_line(vdp_sms_t *vdp);
void vdp_sms_render_sprites(vdp_sms_t *vdp);
void vdp_sms_update_framebuffer(vdp_sms_t *vdp);

#endif // VDP_SMS_H
