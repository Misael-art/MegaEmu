/**
 * @file sms_vdp.c
 * @brief Implementação do VDP do Master System com suporte a extensões
 */

#include "sms_vdp.h"
#include "../../../core/save_state.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir categoria de log
#define EMU_LOG_CAT_SMS_VDP EMU_LOG_CAT_VIDEO

// Macros de log
#define SMS_VDP_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)

// Definições de registradores do VDP
#define VDP_REG0_MASK 0xFF  // Modo de vídeo 1
#define VDP_REG1_MASK 0xFF  // Modo de vídeo 2
#define VDP_REG2_MASK 0xFF  // Base de nomes de padrão
#define VDP_REG3_MASK 0xFF  // Base de cores
#define VDP_REG4_MASK 0xFF  // Base de padrões
#define VDP_REG5_MASK 0xFF  // Base de sprites
#define VDP_REG6_MASK 0xFF  // Base de cores de sprites
#define VDP_REG7_MASK 0xFF  // Cor de borda/background
#define VDP_REG8_MASK 0xFF  // Scroll X
#define VDP_REG9_MASK 0xFF  // Scroll Y
#define VDP_REG10_MASK 0xFF // Contador de linhas

// Flags do registrador de status
#define VDP_STATUS_VBLANK 0x80      // Frame concluído
#define VDP_STATUS_SPRITE_COLL 0x20 // Colisão de sprites
#define VDP_STATUS_SPRITE_OVER 0x40 // Overflow de sprites

// Tamanho da VRAM (16KB)
#define VDP_VRAM_SIZE 0x4000

// Tamanho da CRAM (32 bytes para SMS, 64 para GG)
#define VDP_CRAM_SIZE 0x40

// Adicionar antes da estrutura do VDP
// Estrutura para scroll por linha
typedef struct {
  uint8_t scroll_x[SMS_SCREEN_HEIGHT]; // Valor de scroll X para cada linha
  uint8_t scroll_y[SMS_SCREEN_HEIGHT]; // Valor de scroll Y para cada linha
  bool enabled; // Indica se o scroll por linha está ativado
} sms_line_scroll_t;

// Estrutura para controle de timing
typedef struct {
  uint8_t h_counter;      // Contador horizontal
  uint8_t v_counter;      // Contador vertical
  uint8_t line_cycles;    // Ciclos acumulados na linha atual
  uint16_t frame_cycles;  // Ciclos totais do frame
  uint8_t interrupt_line; // Linha de interrupção
  bool h_interrupt;       // Flag de interrupção horizontal
  bool v_interrupt;       // Flag de interrupção vertical
} sms_vdp_timing_t;

/**
 * @brief Estrutura do VDP do Master System
 */
struct sms_vdp_t {
  // Memórias
  uint8_t *vram;    // Video RAM
  uint8_t *cram;    // Color RAM
  uint8_t regs[16]; // Registradores

  // Estado do VDP
  uint8_t status;  // Registrador de status
  uint16_t addr;   // Endereço atual
  uint8_t code;    // Código de operação
  bool first_byte; // Flag para primeiro byte
  uint8_t buffer;  // Buffer de leitura
  uint8_t line;    // Linha atual

  // Buffer de tela
  uint32_t *screen_buffer;

  // Extensão
  const sms_vdp_ext_t *ext; // Interface de extensão
  void *ext_data;           // Dados da extensão

  // Ponteiros para outros componentes
  void *cpu; // Ponteiro para a CPU

  // Efeitos especiais e timing avançados
  uint16_t special_effects;      // Flags de efeitos especiais
  bool interlace_enabled;        // Modo interlace ativado
  bool odd_frame;                // Flag de quadro ímpar (para interlace)
  sms_line_scroll_t line_scroll; // Sistema de scroll por linha
  sms_vdp_timing_t timing;       // Sistema de timing preciso

  // Cache para otimizações
  uint8_t *pattern_cache;  // Cache de padrões decodificados
  bool *pattern_is_cached; // Flags indicando quais padrões estão em cache
  bool cache_dirty;        // Indica se o cache precisa ser atualizado
};

// Forward declarations de funções internas
static void sms_vdp_reset_registers(sms_vdp_t *vdp);
static void sms_vdp_control_write(sms_vdp_t *vdp, uint8_t value);
static void sms_vdp_render_line(sms_vdp_t *vdp, uint16_t line);
static void sms_vdp_update_status(sms_vdp_t *vdp);
static void sms_vdp_update_palette(sms_vdp_t *vdp);
static void sms_vdp_process_command(sms_vdp_t *vdp, uint8_t value);

/**
 * @brief Cria uma nova instância do VDP
 * @param ext Ponteiro para interface de extensão (opcional)
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_vdp_t *sms_vdp_create(const sms_vdp_ext_t *ext) {
  sms_vdp_t *vdp = (sms_vdp_t *)malloc(sizeof(sms_vdp_t));
  if (!vdp) {
    SMS_VDP_LOG_ERROR("Falha ao alocar memória para VDP");
    return NULL;
  }

  // Inicializa estrutura
  memset(vdp, 0, sizeof(sms_vdp_t));

  // Inicializa extensão se fornecida
  vdp->ext = ext;
  if (ext) {
    vdp->ext_data = ext->init();
    if (!vdp->ext_data) {
      SMS_VDP_LOG_ERROR("Falha ao inicializar extensão do VDP");
      free(vdp);
      return NULL;
    }
  }

  // Aloca a VRAM
  vdp->vram = (uint8_t *)malloc(VDP_VRAM_SIZE);
  if (!vdp->vram) {
    SMS_VDP_LOG_ERROR("Falha ao alocar memória para a VRAM");
    if (vdp->ext && vdp->ext_data) {
      vdp->ext->shutdown(vdp->ext_data);
    }
    free(vdp);
    return NULL;
  }

  // Aloca a CRAM
  vdp->cram = (uint8_t *)malloc(VDP_CRAM_SIZE);
  if (!vdp->cram) {
    SMS_VDP_LOG_ERROR("Falha ao alocar memória para a CRAM");
    free(vdp->vram);
    if (vdp->ext && vdp->ext_data) {
      vdp->ext->shutdown(vdp->ext_data);
    }
    free(vdp);
    return NULL;
  }

  // Aloca o buffer de tela
  vdp->screen_buffer = (uint32_t *)malloc(SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT *
                                          sizeof(uint32_t));
  if (!vdp->screen_buffer) {
    SMS_VDP_LOG_ERROR("Falha ao alocar memória para o buffer de tela");
    free(vdp->cram);
    free(vdp->vram);
    if (vdp->ext && vdp->ext_data) {
      vdp->ext->shutdown(vdp->ext_data);
    }
    free(vdp);
    return NULL;
  }

  // Aloca o cache de padrões para otimização
  vdp->pattern_cache =
      (uint8_t *)malloc(256 * 8 * 8); // 256 padrões de 8x8 pixels
  if (!vdp->pattern_cache) {
    SMS_VDP_LOG_ERROR("Falha ao alocar memória para o cache de padrões");
    free(vdp->screen_buffer);
    free(vdp->cram);
    free(vdp->vram);
    if (vdp->ext && vdp->ext_data) {
      vdp->ext->shutdown(vdp->ext_data);
    }
    free(vdp);
    return NULL;
  }

  // Aloca os flags de cache
  vdp->pattern_is_cached = (bool *)malloc(256 * sizeof(bool));
  if (!vdp->pattern_is_cached) {
    SMS_VDP_LOG_ERROR("Falha ao alocar memória para os flags de cache");
    free(vdp->pattern_cache);
    free(vdp->screen_buffer);
    free(vdp->cram);
    free(vdp->vram);
    if (vdp->ext && vdp->ext_data) {
      vdp->ext->shutdown(vdp->ext_data);
    }
    free(vdp);
    return NULL;
  }

  // Inicializa a memória
  memset(vdp->vram, 0, VDP_VRAM_SIZE);
  memset(vdp->cram, 0, VDP_CRAM_SIZE);
  memset(vdp->screen_buffer, 0,
         SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT * sizeof(uint32_t));
  memset(vdp->pattern_cache, 0, 256 * 8 * 8);
  memset(vdp->pattern_is_cached, 0, 256 * sizeof(bool));

  // Inicializa os registradores
  sms_vdp_reset_registers(vdp);

  // Inicializa os sistemas de efeitos especiais
  vdp->special_effects = 0;
  vdp->interlace_enabled = false;
  vdp->odd_frame = false;
  sms_vdp_init_line_scroll(vdp);
  sms_vdp_init_timing(vdp);
  vdp->cache_dirty = true;

  SMS_VDP_LOG_INFO("VDP do Master System criado%s", ext ? " com extensão" : "");
  return vdp;
}

/**
 * @brief Destrói uma instância do VDP
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_destroy(sms_vdp_t *vdp) {
  if (!vdp)
    return;

  // Finaliza extensão se presente
  if (vdp->ext && vdp->ext_data) {
    vdp->ext->shutdown(vdp->ext_data);
  }

  // Libera recursos
  if (vdp->vram) {
    free(vdp->vram);
  }

  if (vdp->cram) {
    free(vdp->cram);
  }

  if (vdp->screen_buffer) {
    free(vdp->screen_buffer);
  }

  if (vdp->pattern_cache) {
    free(vdp->pattern_cache);
  }

  if (vdp->pattern_is_cached) {
    free(vdp->pattern_is_cached);
  }

  free(vdp);
  SMS_VDP_LOG_INFO("VDP do Master System destruído");
}

/**
 * @brief Reseta o VDP
 * @param vdp Ponteiro para a instância
 */
void sms_vdp_reset(sms_vdp_t *vdp) {
  if (!vdp)
    return;

  // Limpa memórias e registradores
  memset(vdp->vram, 0, VDP_VRAM_SIZE);
  memset(vdp->cram, 0, VDP_CRAM_SIZE);
  memset(vdp->regs, 0, sizeof(vdp->regs));
  memset(vdp->screen_buffer, 0,
         SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT * sizeof(uint32_t));
  memset(vdp->pattern_cache, 0, 256 * 8 * 8);
  memset(vdp->pattern_is_cached, 0, 256 * sizeof(bool));

  // Reseta estado
  vdp->status = 0;
  vdp->addr = 0;
  vdp->code = 0;
  vdp->first_byte = true;
  vdp->buffer = 0;
  vdp->line = 0;
  vdp->cache_dirty = true;

  // Reseta efeitos especiais
  vdp->special_effects = 0;
  vdp->interlace_enabled = false;
  vdp->odd_frame = false;
  sms_vdp_init_line_scroll(vdp);
  sms_vdp_init_timing(vdp);

  // Reseta extensão se presente
  if (vdp->ext && vdp->ext_data) {
    vdp->ext->reset(vdp->ext_data);
  }

  SMS_VDP_LOG_INFO("VDP do Master System resetado");
}

/**
 * @brief Escreve um valor em um registrador do VDP
 * @param vdp Ponteiro para a instância
 * @param reg Número do registrador
 * @param value Valor a ser escrito
 */
void sms_vdp_write_register(sms_vdp_t *vdp, uint8_t reg, uint8_t value) {
  if (!vdp || reg >= 16)
    return;

  vdp->regs[reg] = value;
  SMS_VDP_LOG_TRACE("Registrador %02X = %02X", reg, value);
}

/**
 * @brief Lê um registrador do VDP
 * @param vdp Ponteiro para a instância
 * @param reg Número do registrador
 * @return Valor do registrador
 */
uint8_t sms_vdp_read_register(sms_vdp_t *vdp, uint8_t reg) {
  if (!vdp || reg >= 16)
    return 0;
  return vdp->regs[reg];
}

/**
 * @brief Escreve um valor na porta de dados do VDP
 * @param vdp Ponteiro para a instância
 * @param value Valor a ser escrito
 */
void sms_vdp_write_data(sms_vdp_t *vdp, uint8_t value) {
  if (!vdp)
    return;

  switch (vdp->code) {
  case SMS_VDP_CODE_VRAM:
    vdp->vram[vdp->addr & 0x3FFF] = value;
    break;

  case SMS_VDP_CODE_CRAM:
    vdp->cram[vdp->addr & 0x1F] = value;
    // Notifica extensão sobre escrita na CRAM
    if (vdp->ext && vdp->ext_data) {
      vdp->ext->write_cram(vdp->ext_data, value);
    }
    break;

  case SMS_VDP_CODE_REGISTER:
    sms_vdp_write_register(vdp, vdp->addr & 0x0F, value);
    break;
  }

  vdp->addr = (vdp->addr + 1) & 0x3FFF;
  SMS_VDP_LOG_TRACE("Escrita de dados: %02X", value);
}

/**
 * @brief Lê um valor da porta de dados do VDP
 * @param vdp Ponteiro para a instância
 * @return Valor lido
 */
uint8_t sms_vdp_read_data(sms_vdp_t *vdp) {
  if (!vdp)
    return 0;

  uint8_t value = vdp->buffer;
  vdp->buffer = vdp->vram[vdp->addr & 0x3FFF];
  vdp->addr = (vdp->addr + 1) & 0x3FFF;

  SMS_VDP_LOG_TRACE("Leitura de dados: %02X", value);
  return value;
}

/**
 * @brief Escreve um valor na porta de controle do VDP
 * @param vdp Ponteiro para a instância
 * @param value Valor a ser escrito
 */
void sms_vdp_write_control(sms_vdp_t *vdp, uint8_t value) {
  if (!vdp)
    return;

  if (vdp->first_byte) {
    vdp->addr = (vdp->addr & 0xFF00) | value;
    vdp->first_byte = false;
  } else {
    vdp->addr = (vdp->addr & 0x00FF) | ((value & 0x3F) << 8);
    vdp->code = value & 0xC0;
    vdp->first_byte = true;
  }

  SMS_VDP_LOG_TRACE("Escrita de controle: %02X", value);
}

/**
 * @brief Lê o registrador de status do VDP
 * @param vdp Ponteiro para a instância
 * @return Valor do registrador de status
 */
uint8_t sms_vdp_read_status(sms_vdp_t *vdp) {
  if (!vdp)
    return 0;

  uint8_t value = vdp->status;
  vdp->status &=
      ~(VDP_STATUS_SPRITE_OVER | VDP_STATUS_SPRITE_COLL | VDP_STATUS_VBLANK);
  vdp->first_byte = true;

  SMS_VDP_LOG_TRACE("Leitura de status: %02X", value);
  return value;
}

/**
 * @brief Executa um frame do VDP
 * @param vdp Ponteiro para a instância
 * @return true se uma interrupção vertical foi gerada
 */
bool sms_vdp_run_frame(sms_vdp_t *vdp) {
  if (!vdp)
    return false;

  // Limpa flags de interrupção vertical anterior
  vdp->timing.v_interrupt = false;

  // Verifica se o display está ativado
  bool display_enabled = (vdp->regs[1] & 0x40) != 0;

  // No modo interlace, renderizamos apenas campos alternados
  bool should_render = true;
  if (vdp->interlace_enabled) {
    // No modo interlace, renderizamos linhas pares em um campo e ímpares no
    // outro
    if (vdp->odd_frame) {
      // Campo ímpar - renderiza apenas linhas ímpares
      should_render = (vdp->regs[1] & 0x20) !=
                      0; // Verifica se o display de linhas ímpares está ativado
    } else {
      // Campo par - renderiza apenas linhas pares
      should_render =
          (vdp->regs[1] & 0x40) != 0; // Verifica se o display está ativado
    }
  }

  // Renderiza cada linha visível
  if (display_enabled && should_render) {
    uint16_t start_line = vdp->interlace_enabled ? (vdp->odd_frame ? 1 : 0) : 0;
    uint16_t line_increment = vdp->interlace_enabled ? 2 : 1;

    for (uint16_t line = start_line; line < SMS_SCREEN_HEIGHT;
         line += line_increment) {
      // Se o scroll por linha estiver ativado, usa a função especializada
      if (vdp->line_scroll.enabled) {
        sms_vdp_render_line_with_scroll(vdp, line);
      } else {
        sms_vdp_render_line(vdp, line);
      }
    }
  } else {
    // Se o display estiver desativado, preenche a tela com a cor de fundo
    uint8_t bg_color_index = vdp->regs[7] & 0x0F;
    uint32_t bg_color;

    // Obtém a cor de fundo da CRAM
    if (vdp->ext && vdp->ext_data && vdp->ext->read_cram) {
      uint16_t color16 = vdp->ext->read_cram(vdp->ext_data, bg_color_index);
      bg_color = ((color16 & 0x000F) << 20) | // R
                 ((color16 & 0x00F0) << 8) |  // G
                 ((color16 & 0x0F00) >> 4) |  // B
                 0xFF000000;                  // A
    } else {
      // Converte o valor de 6-bit para RGB32
      uint8_t r = ((bg_color_index >> 0) & 0x03) * 85;
      uint8_t g = ((bg_color_index >> 2) & 0x03) * 85;
      uint8_t b = ((bg_color_index >> 4) & 0x03) * 85;
      bg_color = (r << 16) | (g << 8) | b | 0xFF000000;
    }

    // Preenche o buffer com a cor de fundo
    for (int i = 0; i < SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT; i++) {
      vdp->screen_buffer[i] = bg_color;
    }
  }

  // Simula o ciclo de varredura completo
  for (uint16_t line = 0; line < 262; line++) {
    // Calcula o número de ciclos para esta linha
    uint8_t cycles_in_line = 228;

    // Se estivermos na última linha do frame e o modo interlace estiver
    // ativado, adicionamos ciclos extras para sincronização
    if (line == 261 && vdp->interlace_enabled) {
      cycles_in_line += vdp->odd_frame ? 1 : 2;
    }

    // Atualiza o timing para esta linha
    for (uint8_t c = 0; c < cycles_in_line; c += 4) {
      uint8_t cycles_to_next = sms_vdp_update_timing(vdp, 4);
      if (cycles_to_next == 0) {
        // Uma interrupção ocorreu, podemos processá-la aqui se necessário
      }
    }
  }

  // Alternamos o campo para o próximo frame
  vdp->odd_frame = !vdp->odd_frame;

  // Notifica extensão sobre atualização do buffer
  if (vdp->ext && vdp->ext_data && vdp->ext->convert_buffer) {
    vdp->ext->convert_buffer(vdp->ext_data,
                             (const uint8_t *)vdp->screen_buffer);
  }

  // Atualiza status
  vdp->status |= VDP_STATUS_VBLANK;

  // Retorna se uma interrupção vertical foi gerada
  return (vdp->regs[1] & 0x20) != 0;
}

/**
 * @brief Obtém o buffer de tela do VDP
 * @param vdp Ponteiro para a instância
 * @return Ponteiro para o buffer de tela
 */
const uint16_t *sms_vdp_get_screen_buffer(sms_vdp_t *vdp) {
  if (!vdp)
    return NULL;

  // Se houver extensão, retorna o buffer da extensão
  if (vdp->ext && vdp->ext_data) {
    return vdp->ext->get_screen_buffer(vdp->ext_data);
  }

  // TODO: Implementar conversão do buffer para RGB565
  return NULL;
}

/**
 * @brief Registra campos do VDP no sistema de save state
 * @param vdp Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int sms_vdp_register_save_state(sms_vdp_t *vdp, save_state_t *state) {
  if (!vdp || !state)
    return -1;

  // Registra campos do VDP
  save_state_register_field(state, "sms_vdp_vram", vdp->vram,
                            sizeof(vdp->vram));
  save_state_register_field(state, "sms_vdp_cram", vdp->cram,
                            sizeof(vdp->cram));
  save_state_register_field(state, "sms_vdp_regs", vdp->regs,
                            sizeof(vdp->regs));
  save_state_register_field(state, "sms_vdp_status", &vdp->status,
                            sizeof(vdp->status));
  save_state_register_field(state, "sms_vdp_addr", &vdp->addr,
                            sizeof(vdp->addr));
  save_state_register_field(state, "sms_vdp_code", &vdp->code,
                            sizeof(vdp->code));
  save_state_register_field(state, "sms_vdp_first_byte", &vdp->first_byte,
                            sizeof(vdp->first_byte));
  save_state_register_field(state, "sms_vdp_buffer", &vdp->buffer,
                            sizeof(vdp->buffer));
  save_state_register_field(state, "sms_vdp_line", &vdp->line,
                            sizeof(vdp->line));

  // Registra campos da extensão se presente
  if (vdp->ext && vdp->ext_data) {
    if (vdp->ext->register_save_state(vdp->ext_data, state) != 0) {
      return -1;
    }
  }

  return 0;
}

/**
 * @brief Inicializa o sistema de scroll por linha
 * @param vdp Ponteiro para a instância
 */
static void sms_vdp_init_line_scroll(sms_vdp_t *vdp) {
  if (!vdp)
    return;

  // Inicializa a estrutura
  memset(vdp->line_scroll.scroll_x, 0, sizeof(vdp->line_scroll.scroll_x));
  memset(vdp->line_scroll.scroll_y, 0, sizeof(vdp->line_scroll.scroll_y));
  vdp->line_scroll.enabled = false;
}

/**
 * @brief Inicializa o sistema de timing preciso
 * @param vdp Ponteiro para a instância
 */
static void sms_vdp_init_timing(sms_vdp_t *vdp) {
  if (!vdp)
    return;

  // Inicializa a estrutura
  memset(&vdp->timing, 0, sizeof(vdp->timing));
  vdp->timing.interrupt_line = 0;
  vdp->timing.h_interrupt = false;
  vdp->timing.v_interrupt = false;
}

/**
 * @brief Configura o modo interlace
 * @param vdp Ponteiro para a instância
 * @param enabled true para ativar, false para desativar
 */
void sms_vdp_set_interlace_mode(sms_vdp_t *vdp, bool enabled) {
  if (!vdp)
    return;

  vdp->interlace_enabled = enabled;
  vdp->special_effects = (vdp->special_effects & ~SMS_VDP_EXT_FLAG_INTERLACE) |
                         (enabled ? SMS_VDP_EXT_FLAG_INTERLACE : 0);

  // Notifica a extensão se existir
  if (vdp->ext && vdp->ext_data && vdp->ext->set_interlace_mode) {
    vdp->ext->set_interlace_mode(vdp->ext_data, enabled);
  }

  SMS_VDP_LOG_INFO("Modo interlace %s", enabled ? "ativado" : "desativado");
}

/**
 * @brief Configura o scroll para uma linha específica
 * @param vdp Ponteiro para a instância
 * @param line Número da linha (0-191)
 * @param scroll_x Valor do scroll horizontal
 * @param scroll_y Valor do scroll vertical (se suportado)
 */
void sms_vdp_set_line_scroll(sms_vdp_t *vdp, uint8_t line, uint8_t scroll_x,
                             uint8_t scroll_y) {
  if (!vdp || line >= SMS_SCREEN_HEIGHT)
    return;

  // Ativa o scroll por linha se ainda não estiver ativado
  if (!vdp->line_scroll.enabled) {
    vdp->line_scroll.enabled = true;
    vdp->special_effects |= SMS_VDP_EXT_FLAG_LINE_SCROLL;
  }

  // Define os valores de scroll
  vdp->line_scroll.scroll_x[line] = scroll_x;
  vdp->line_scroll.scroll_y[line] = scroll_y;

  // Notifica a extensão se existir
  if (vdp->ext && vdp->ext_data && vdp->ext->set_line_scroll) {
    vdp->ext->set_line_scroll(vdp->ext_data, line, scroll_x, scroll_y);
  }

  SMS_VDP_LOG_TRACE("Scroll de linha %d configurado: X=%d, Y=%d", line,
                    scroll_x, scroll_y);
}

/**
 * @brief Configura flags de efeitos especiais
 * @param vdp Ponteiro para a instância
 * @param flags Bits de flag conforme definidos por SMS_VDP_EXT_FLAG_*
 */
void sms_vdp_set_special_effects(sms_vdp_t *vdp, uint16_t flags) {
  if (!vdp)
    return;

  vdp->special_effects = flags;

  // Atualiza flags internas baseadas nas flags especiais
  vdp->interlace_enabled = (flags & SMS_VDP_EXT_FLAG_INTERLACE) != 0;
  vdp->line_scroll.enabled = (flags & SMS_VDP_EXT_FLAG_LINE_SCROLL) != 0;

  // Notifica a extensão se existir
  if (vdp->ext && vdp->ext_data && vdp->ext->set_special_effects) {
    vdp->ext->set_special_effects(vdp->ext_data, flags);
  }

  SMS_VDP_LOG_INFO("Efeitos especiais configurados: 0x%04X", flags);
}

/**
 * @brief Atualiza o timing do VDP de forma precisa
 * @param vdp Ponteiro para a instância
 * @param cycles Número de ciclos a avançar
 * @return Número de ciclos até o próximo evento
 */
uint8_t sms_vdp_update_timing(sms_vdp_t *vdp, uint8_t cycles) {
  if (!vdp)
    return 0;

  // Atualiza contadores
  vdp->timing.line_cycles += cycles;
  vdp->timing.frame_cycles += cycles;

  // Verifica se completou uma linha
  if (vdp->timing.line_cycles >= 228) { // 228 ciclos = duração de uma linha
    vdp->timing.line_cycles -= 228;
    vdp->timing.v_counter++;

    // Reinicia o contador horizontal
    vdp->timing.h_counter = 0;

    // Verifica interrupção horizontal
    if ((vdp->regs[0] & 0x10) && (vdp->timing.v_counter <= 192)) {
      if ((vdp->timing.v_counter % vdp->regs[10]) == 0) {
        vdp->timing.h_interrupt = true;
      }
    }

    // Verifica início do VBlank
    if (vdp->timing.v_counter == 192) {
      vdp->status |= VDP_STATUS_VBLANK;

      // Verifica interrupção vertical
      if (vdp->regs[1] & 0x20) {
        vdp->timing.v_interrupt = true;
      }
    }

    // Verifica fim do frame (NTSC = 262 linhas, PAL = 313 linhas)
    bool is_pal = (vdp->regs[0] & 0x02) != 0;
    uint16_t total_lines = is_pal ? 313 : 262;

    if (vdp->timing.v_counter >= total_lines) {
      vdp->timing.v_counter = 0;
      vdp->timing.frame_cycles = 0;
      vdp->odd_frame = !vdp->odd_frame; // Alterna quadros para interlace
    }
  } else {
    // Atualiza o contador horizontal (aproximadamente)
    vdp->timing.h_counter = (vdp->timing.line_cycles * 256) / 228;
  }

  // Verifica se há uma extensão para ajustar o timing
  if (vdp->ext && vdp->ext_data && vdp->ext->adjust_timing) {
    return vdp->ext->adjust_timing(vdp->ext_data, vdp->timing.h_counter,
                                   vdp->timing.v_counter, cycles);
  }

  // Retorna ciclos até a próxima interrupção ou próxima linha
  if (vdp->timing.h_interrupt || vdp->timing.v_interrupt) {
    return 0; // Interrupção pendente
  }

  return 228 - vdp->timing.line_cycles; // Ciclos até completar a linha
}

/**
 * @brief Verifica se há interrupção pendente do VDP
 * @param vdp Ponteiro para a instância
 * @return Valor diferente de zero se há interrupção
 */
uint8_t sms_vdp_check_interrupt(sms_vdp_t *vdp) {
  if (!vdp)
    return 0;

  // Verifica interrupções
  if (vdp->timing.v_interrupt ||
      (vdp->timing.h_interrupt && (vdp->regs[0] & 0x10))) {

    // Limpa flags de interrupção
    vdp->timing.h_interrupt = false;
    vdp->timing.v_interrupt = false;

    return 1;
  }

  return 0;
}

/**
 * @brief Renderiza os tiles com scroll por linha, se ativado
 * @param vdp Ponteiro para a instância
 * @param line Linha a ser renderizada
 */
static void sms_vdp_render_line_with_scroll(sms_vdp_t *vdp, uint16_t line) {
  if (!vdp || line >= SMS_SCREEN_HEIGHT)
    return;

  // Se o scroll por linha não estiver ativado, usa a função padrão
  if (!vdp->line_scroll.enabled) {
    sms_vdp_render_line(vdp, line);
    return;
  }

  // Obtém os valores de scroll para a linha
  uint8_t scroll_x = vdp->line_scroll.scroll_x[line];
  uint8_t scroll_y = vdp->line_scroll.scroll_y[line];

  // Implementa renderização com scroll por linha (simplificada)
  // Esta implementação deve ser expandida para renderizar corretamente

  // Todo: Implementar renderização baseada no scroll específico da linha

  // Por enquanto, ajustamos os registradores temporariamente para a linha
  uint8_t original_scroll_x = vdp->regs[8];
  uint8_t original_scroll_y = vdp->regs[9];

  vdp->regs[8] = scroll_x;
  vdp->regs[9] = scroll_y;

  // Renderiza a linha com os novos valores de scroll
  sms_vdp_render_line(vdp, line);

  // Restaura os registradores originais
  vdp->regs[8] = original_scroll_x;
  vdp->regs[9] = original_scroll_y;
}
