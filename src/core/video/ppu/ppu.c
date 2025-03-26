/**
 * @file ppu.c
 * @brief Implementação da PPU (Picture Processing Unit)
 *
 * Implementação do processador de vídeo PPU seguindo o padrão de arquitetura
 * híbrida.
 */

#include "ppu.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Contexto interno da PPU
 */
struct ppu_context_s {
  /* Configuração */
  ppu_full_config_t config;

  /* Estado de renderização */
  int scanline;         /* Scanline atual */
  int cycle;            /* Ciclo dentro do scanline atual */
  uint32_t frame_count; /* Contador de frames */
  bool in_vblank;       /* Flag de vblank */
  bool frame_ready;     /* Flag de frame completo */

  /* Estado dos registradores - genérico para todas as PPUs */
  uint8_t registers[256]; /* Registradores mapeados em memória */

  /* Buffers internos */
  void *internal_framebuffer; /* Framebuffer interno se não fornecido
                                 externamente */
  size_t internal_fb_size;    /* Tamanho do framebuffer interno */

  /* Estado adicional específico para diferentes tipos de PPU */
  union {
    struct {
      /* Estado específico da PPU do NES */
      uint16_t v;           /* Registro V (deslocamento VRAM atual) */
      uint16_t t;           /* Registro T (deslocamento VRAM temporário) */
      uint8_t x;            /* Fine X scroll (3 bits) */
      uint8_t w;            /* Latch do primeiro/segundo write */
      bool nmi_occurred;    /* Flag NMI ocorreu */
      bool sprite_zero_hit; /* Flag de hit do sprite 0 */
      uint8_t oam_addr;     /* Endereço atual do OAM */
    } nes;

    struct {
      /* Estado específico da PPU do SNES */
      uint16_t vram_addr;  /* Endereço VRAM atual */
      uint8_t brightness;  /* Nível de brilho (0-15) */
      uint8_t mode;        /* Modo de vídeo (0-7) */
      bool mosaic_enabled; /* Mosaico habilitado */
      uint8_t mosaic_size; /* Tamanho do mosaico (1-16) */
    } snes;

    struct {
      /* Estado específico do VDP do Master System/Game Gear */
      uint8_t code_register;  /* Registro de código */
      uint8_t status;         /* Registro de status */
      uint16_t addr_register; /* Registro de endereço */
    } sms;

    struct {
      /* Estado específico do VDP do Mega Drive/Genesis */
      uint8_t code_register;  /* Registro de código */
      uint8_t status;         /* Registro de status */
      uint16_t addr_register; /* Registro de endereço */
      uint8_t dma_mode;       /* Modo de DMA */
      uint32_t dma_source;    /* Endereço fonte para DMA */
      uint16_t dma_length;    /* Comprimento da transferência DMA */
      bool dma_active;        /* Flag de DMA ativo */
    } genesis;

    struct {
      /* Estado específico da PPU do Game Boy */
      uint8_t lcdc;     /* Registro LCD Control */
      uint8_t stat;     /* Registro LCD Status */
      uint8_t scrollx;  /* Scroll X */
      uint8_t scrolly;  /* Scroll Y */
      uint8_t window_x; /* Posição X da janela */
      uint8_t window_y; /* Posição Y da janela */
      uint8_t ly;       /* Linha atual */
      uint8_t lyc;      /* Comparador de linha */
    } gb;
  } specific;
};

/* Tamanho do estado salvo em bytes - ajustar conforme necessário */
#define PPU_STATE_SIZE 1024

/**
 * @brief Cria uma nova instância da PPU
 */
ppu_t *ppu_create(const ppu_full_config_t *config) {
  if (!config) {
    return NULL;
  }

  ppu_t *ppu = (ppu_t *)calloc(1, sizeof(ppu_t));
  if (!ppu) {
    return NULL;
  }

  /* Copiar configuração */
  ppu->config = *config;

  /* Alocar framebuffer interno se não fornecido externamente */
  if (!ppu->config.config.framebuffer) {
    size_t pixel_size = 0;
    switch (ppu->config.config.pixel_format) {
    case PPU_PIXEL_FORMAT_RGB565:
      pixel_size = 2;
      break;

    case PPU_PIXEL_FORMAT_RGB888:
      pixel_size = 3;
      break;

    case PPU_PIXEL_FORMAT_RGBA8888:
      pixel_size = 4;
      break;

    default:
      pixel_size = 4; /* Padrão para RGBA8888 */
      break;
    }

    size_t buffer_size =
        ppu->config.config.width * ppu->config.config.height * pixel_size;

    ppu->internal_framebuffer = malloc(buffer_size);
    if (!ppu->internal_framebuffer) {
      free(ppu);
      return NULL;
    }

    ppu->internal_fb_size = buffer_size;
    ppu->config.config.framebuffer = ppu->internal_framebuffer;
    ppu->config.config.framebuffer_pitch =
        ppu->config.config.width * pixel_size;
  }

  /* Inicializar estado */
  ppu_reset(ppu);

  return ppu;
}

/**
 * @brief Destrói uma instância da PPU e libera recursos
 */
void ppu_destroy(ppu_t *ppu) {
  if (ppu) {
    /* Liberar framebuffer interno se foi alocado */
    if (ppu->internal_framebuffer) {
      free(ppu->internal_framebuffer);
    }

    free(ppu);
  }
}

/**
 * @brief Reseta a PPU para estado inicial
 */
void ppu_reset(ppu_t *ppu) {
  if (!ppu) {
    return;
  }

  /* Resetar estado de renderização */
  ppu->scanline = 0;
  ppu->cycle = 0;
  ppu->frame_count = 0;
  ppu->in_vblank = false;
  ppu->frame_ready = false;

  /* Resetar registradores */
  memset(ppu->registers, 0, sizeof(ppu->registers));

  /* Resetar estado específico ao tipo de PPU */
  switch (ppu->config.config.type) {
  case PPU_TYPE_NES:
    memset(&ppu->specific.nes, 0, sizeof(ppu->specific.nes));
    break;

  case PPU_TYPE_SNES:
    memset(&ppu->specific.snes, 0, sizeof(ppu->specific.snes));
    /* Valores iniciais específicos do SNES */
    ppu->specific.snes.brightness = 15; /* Brilho máximo */
    break;

  case PPU_TYPE_SMS_GG:
    memset(&ppu->specific.sms, 0, sizeof(ppu->specific.sms));
    break;

  case PPU_TYPE_GENESIS:
    memset(&ppu->specific.genesis, 0, sizeof(ppu->specific.genesis));
    break;

  case PPU_TYPE_GB:
    memset(&ppu->specific.gb, 0, sizeof(ppu->specific.gb));
    break;

  default:
    /* Nada específico para tipo personalizado */
    break;
  }

  /* Limpar framebuffer */
  if (ppu->config.config.framebuffer) {
    memset(ppu->config.config.framebuffer, 0,
           ppu->config.config.height * ppu->config.config.framebuffer_pitch);
  }
}

/**
 * @brief Define o framebuffer para renderização
 */
int ppu_set_framebuffer(ppu_t *ppu, void *framebuffer, size_t pitch) {
  if (!ppu) {
    return -1;
  }

  /* Se tínhamos um framebuffer interno e agora estamos usando externo */
  if (ppu->internal_framebuffer && framebuffer) {
    free(ppu->internal_framebuffer);
    ppu->internal_framebuffer = NULL;
    ppu->internal_fb_size = 0;
  }

  /* Se estamos removendo o framebuffer externo, criar um interno */
  if (!framebuffer && !ppu->internal_framebuffer) {
    size_t pixel_size = 0;
    switch (ppu->config.config.pixel_format) {
    case PPU_PIXEL_FORMAT_RGB565:
      pixel_size = 2;
      break;

    case PPU_PIXEL_FORMAT_RGB888:
      pixel_size = 3;
      break;

    case PPU_PIXEL_FORMAT_RGBA8888:
      pixel_size = 4;
      break;

    default:
      pixel_size = 4; /* Padrão para RGBA8888 */
      break;
    }

    size_t buffer_size =
        ppu->config.config.width * ppu->config.config.height * pixel_size;

    ppu->internal_framebuffer = malloc(buffer_size);
    if (!ppu->internal_framebuffer) {
      return -1;
    }

    ppu->internal_fb_size = buffer_size;
    ppu->config.config.framebuffer = ppu->internal_framebuffer;
    ppu->config.config.framebuffer_pitch =
        ppu->config.config.width * pixel_size;
  } else {
    /* Atualizar com o framebuffer externo */
    ppu->config.config.framebuffer = framebuffer;
    ppu->config.config.framebuffer_pitch = pitch;
  }

  return 0;
}

/**
 * @brief Função auxiliar para ler da VRAM
 */
static inline uint8_t read_vram(ppu_t *ppu, uint16_t address) {
  if (ppu->config.read_vram) {
    return ppu->config.read_vram(ppu->config.context, address);
  }
  return 0xFF;
}

/**
 * @brief Função auxiliar para escrever na VRAM
 */
static inline void write_vram(ppu_t *ppu, uint16_t address, uint8_t value) {
  if (ppu->config.write_vram) {
    ppu->config.write_vram(ppu->config.context, address, value);
  }
}

/**
 * @brief Função auxiliar para ler do OAM
 */
static inline uint8_t read_oam(ppu_t *ppu, uint16_t address) {
  if (ppu->config.read_oam) {
    return ppu->config.read_oam(ppu->config.context, address);
  }
  return 0xFF;
}

/**
 * @brief Função auxiliar para escrever no OAM
 */
static inline void write_oam(ppu_t *ppu, uint16_t address, uint8_t value) {
  if (ppu->config.write_oam) {
    ppu->config.write_oam(ppu->config.context, address, value);
  }
}

/**
 * @brief Função auxiliar para ler do CGRAM/paleta
 */
static inline uint8_t read_cgram(ppu_t *ppu, uint16_t address) {
  if (ppu->config.read_cgram) {
    return ppu->config.read_cgram(ppu->config.context, address);
  }
  return 0xFF;
}

/**
 * @brief Função auxiliar para escrever no CGRAM/paleta
 */
static inline void write_cgram(ppu_t *ppu, uint16_t address, uint8_t value) {
  if (ppu->config.write_cgram) {
    ppu->config.write_cgram(ppu->config.context, address, value);
  }
}

/**
 * @brief Avança o estado da PPU por um ciclo
 */
static void advance_ppu_state(ppu_t *ppu) {
  const int cycles_per_scanline =
      341; /* Para NES/SNES, ajustar conforme a plataforma */
  const int visible_scanlines = ppu->config.config.visible_height;
  const int vblank_scanlines = ppu->config.config.height - visible_scanlines;
  const int total_scanlines = ppu->config.config.height;

  /* Avançar ciclo */
  ppu->cycle++;

  /* Verificar fim do scanline */
  if (ppu->cycle >= cycles_per_scanline) {
    ppu->cycle = 0;
    ppu->scanline++;

    /* Callback de scanline se registrado */
    if (ppu->config.scanline_callback) {
      ppu->config.scanline_callback(ppu->config.context, ppu->scanline);
    }

    /* Verificar entrada em vblank */
    if (ppu->scanline == visible_scanlines) {
      ppu->in_vblank = true;
    }

    /* Verificar fim do frame */
    if (ppu->scanline >= total_scanlines) {
      ppu->scanline = 0;
      ppu->in_vblank = false;
      ppu->frame_count++;
      ppu->frame_ready = true;

      /* Callback de frame se registrado */
      if (ppu->config.frame_callback) {
        ppu->config.frame_callback(
            ppu->config.context, ppu->config.config.framebuffer,
            ppu->config.config.width, ppu->config.config.height,
            ppu->config.config.framebuffer_pitch);
      }
    }
  }
}

/**
 * @brief Executa uma função de renderização específica para o tipo de PPU
 * Observação: Esta é uma implementação mínima que deve ser estendida
 * para cada tipo específico de PPU.
 */
static void render_pixel(ppu_t *ppu) {
  /* Implementação de renderização mínima */
  int x = ppu->cycle - 1; /* Ajustar para coordenada de pixel */
  int y = ppu->scanline;

  /* Verificar se estamos dentro da área visível */
  if (x >= 0 && x < ppu->config.config.visible_width && y >= 0 &&
      y < ppu->config.config.visible_height) {

    /* Renderizar pixel */
    switch (ppu->config.config.pixel_format) {
    case PPU_PIXEL_FORMAT_RGB565: {
      uint16_t *fb = (uint16_t *)ppu->config.config.framebuffer;
      size_t pixel_offset = y * (ppu->config.config.framebuffer_pitch / 2) + x;
      /* Apenas para teste: Gradiente de cor simples */
      fb[pixel_offset] =
          ((x & 0x1F) << 11) | ((y & 0x3F) << 5) | ((x + y) & 0x1F);
      break;
    }

    case PPU_PIXEL_FORMAT_RGB888: {
      uint8_t *fb = (uint8_t *)ppu->config.config.framebuffer;
      size_t pixel_offset = y * ppu->config.config.framebuffer_pitch + x * 3;
      /* Apenas para teste: Gradiente de cor simples */
      fb[pixel_offset] = x & 0xFF;           /* R */
      fb[pixel_offset + 1] = y & 0xFF;       /* G */
      fb[pixel_offset + 2] = (x + y) & 0xFF; /* B */
      break;
    }

    case PPU_PIXEL_FORMAT_RGBA8888: {
      uint32_t *fb = (uint32_t *)ppu->config.config.framebuffer;
      size_t pixel_offset = y * (ppu->config.config.framebuffer_pitch / 4) + x;
      /* Apenas para teste: Gradiente de cor simples */
      fb[pixel_offset] = 0xFF000000 | ((x & 0xFF) << 16) | ((y & 0xFF) << 8) |
                         ((x + y) & 0xFF);
      break;
    }
    }
  }
}

/**
 * @brief Executa a PPU por um número específico de ciclos
 */
int ppu_execute(ppu_t *ppu, int cycles) {
  if (!ppu) {
    return 0;
  }

  int executed_cycles = 0;

  while (cycles > 0) {
    /* Renderizar pixel atual se estamos na área visível */
    if (ppu->cycle < ppu->config.config.visible_width &&
        ppu->scanline < ppu->config.config.visible_height) {
      render_pixel(ppu);
    }

    /* Avançar estado da PPU */
    advance_ppu_state(ppu);

    cycles--;
    executed_cycles++;
  }

  return executed_cycles;
}

/**
 * @brief Executa até completar um scanline
 */
int ppu_execute_scanline(ppu_t *ppu) {
  if (!ppu) {
    return 0;
  }

  const int cycles_per_scanline =
      341; /* Para NES/SNES, ajustar conforme a plataforma */
  int start_cycle = ppu->cycle;
  int cycles_to_execute = cycles_per_scanline - start_cycle;

  return ppu_execute(ppu, cycles_to_execute);
}

/**
 * @brief Executa até completar um frame
 */
int ppu_execute_frame(ppu_t *ppu) {
  if (!ppu) {
    return 0;
  }

  int start_scanline = ppu->scanline;
  int start_cycle = ppu->cycle;
  uint32_t start_frame = ppu->frame_count;
  int executed_cycles = 0;

  /* Executar até completar o frame atual */
  while (ppu->frame_count == start_frame) {
    executed_cycles += ppu_execute(ppu, 1);
  }

  return executed_cycles;
}

/**
 * @brief Obtém o scanline atual
 */
int ppu_get_scanline(const ppu_t *ppu) { return ppu ? ppu->scanline : 0; }

/**
 * @brief Obtém o ciclo atual dentro do scanline
 */
int ppu_get_cycle(const ppu_t *ppu) { return ppu ? ppu->cycle : 0; }

/**
 * @brief Obtém o contador de frames
 */
uint32_t ppu_get_frame_count(const ppu_t *ppu) {
  return ppu ? ppu->frame_count : 0;
}

/**
 * @brief Verifica se está ocorrendo vblank
 */
bool ppu_in_vblank(const ppu_t *ppu) { return ppu ? ppu->in_vblank : false; }

/**
 * @brief Lê um registrador da PPU
 */
uint8_t ppu_read_register(ppu_t *ppu, uint16_t reg_id) {
  if (!ppu || reg_id >= 256) {
    return 0xFF;
  }

  /* Valor padrão do registrador */
  uint8_t value = ppu->registers[reg_id];

  /* Processamento específico por tipo de PPU seria implementado aqui */

  return value;
}

/**
 * @brief Escreve em um registrador da PPU
 */
void ppu_write_register(ppu_t *ppu, uint16_t reg_id, uint8_t value) {
  if (!ppu || reg_id >= 256) {
    return;
  }

  /* Armazenar valor no registrador */
  ppu->registers[reg_id] = value;

  /* Processamento específico por tipo de PPU seria implementado aqui */
}

/**
 * @brief Salva o estado da PPU em um buffer
 */
int ppu_save_state(const ppu_t *ppu, uint8_t *buffer, size_t buffer_size) {
  if (!ppu || !buffer || buffer_size < PPU_STATE_SIZE) {
    return -1;
  }

  uint8_t *ptr = buffer;

  /* Salvar estado de renderização */
  memcpy(ptr, &ppu->scanline, sizeof(int));
  ptr += sizeof(int);

  memcpy(ptr, &ppu->cycle, sizeof(int));
  ptr += sizeof(int);

  memcpy(ptr, &ppu->frame_count, sizeof(uint32_t));
  ptr += sizeof(uint32_t);

  memcpy(ptr, &ppu->in_vblank, sizeof(bool));
  ptr += sizeof(bool);

  memcpy(ptr, &ppu->frame_ready, sizeof(bool));
  ptr += sizeof(bool);

  /* Salvar registradores */
  memcpy(ptr, ppu->registers, 256);
  ptr += 256;

  /* Salvar estado específico baseado no tipo de PPU */
  switch (ppu->config.config.type) {
  case PPU_TYPE_NES:
    memcpy(ptr, &ppu->specific.nes, sizeof(ppu->specific.nes));
    ptr += sizeof(ppu->specific.nes);
    break;

  case PPU_TYPE_SNES:
    memcpy(ptr, &ppu->specific.snes, sizeof(ppu->specific.snes));
    ptr += sizeof(ppu->specific.snes);
    break;

  case PPU_TYPE_SMS_GG:
    memcpy(ptr, &ppu->specific.sms, sizeof(ppu->specific.sms));
    ptr += sizeof(ppu->specific.sms);
    break;

  case PPU_TYPE_GENESIS:
    memcpy(ptr, &ppu->specific.genesis, sizeof(ppu->specific.genesis));
    ptr += sizeof(ppu->specific.genesis);
    break;

  case PPU_TYPE_GB:
    memcpy(ptr, &ppu->specific.gb, sizeof(ppu->specific.gb));
    ptr += sizeof(ppu->specific.gb);
    break;

  default:
    /* Nada específico para tipo personalizado */
    break;
  }

  /* Retornar número de bytes escritos */
  return (int)(ptr - buffer);
}

/**
 * @brief Carrega o estado da PPU de um buffer
 */
int ppu_load_state(ppu_t *ppu, const uint8_t *buffer, size_t buffer_size) {
  if (!ppu || !buffer || buffer_size < PPU_STATE_SIZE) {
    return -1;
  }

  const uint8_t *ptr = buffer;

  /* Carregar estado de renderização */
  memcpy(&ppu->scanline, ptr, sizeof(int));
  ptr += sizeof(int);

  memcpy(&ppu->cycle, ptr, sizeof(int));
  ptr += sizeof(int);

  memcpy(&ppu->frame_count, ptr, sizeof(uint32_t));
  ptr += sizeof(uint32_t);

  memcpy(&ppu->in_vblank, ptr, sizeof(bool));
  ptr += sizeof(bool);

  memcpy(&ppu->frame_ready, ptr, sizeof(bool));
  ptr += sizeof(bool);

  /* Carregar registradores */
  memcpy(ppu->registers, ptr, 256);
  ptr += 256;

  /* Carregar estado específico baseado no tipo de PPU */
  switch (ppu->config.config.type) {
  case PPU_TYPE_NES:
    memcpy(&ppu->specific.nes, ptr, sizeof(ppu->specific.nes));
    ptr += sizeof(ppu->specific.nes);
    break;

  case PPU_TYPE_SNES:
    memcpy(&ppu->specific.snes, ptr, sizeof(ppu->specific.snes));
    ptr += sizeof(ppu->specific.snes);
    break;

  case PPU_TYPE_SMS_GG:
    memcpy(&ppu->specific.sms, ptr, sizeof(ppu->specific.sms));
    ptr += sizeof(ppu->specific.sms);
    break;

  case PPU_TYPE_GENESIS:
    memcpy(&ppu->specific.genesis, ptr, sizeof(ppu->specific.genesis));
    ptr += sizeof(ppu->specific.genesis);
    break;

  case PPU_TYPE_GB:
    memcpy(&ppu->specific.gb, ptr, sizeof(ppu->specific.gb));
    ptr += sizeof(ppu->specific.gb);
    break;

  default:
    /* Nada específico para tipo personalizado */
    break;
  }

  return 0;
}
