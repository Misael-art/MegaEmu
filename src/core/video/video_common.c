/**
 * @file video_common.c
 * @brief Interface comum para diferentes implementações de PPU
 *
 * Este arquivo fornece funcionalidades comuns e uma camada de abstração
 * para diferentes implementações de PPUs.
 */

#include "ppu/ppu.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Tipo de video/PPU disponível
 */
typedef enum {
  VIDEO_TYPE_NES = 0,
  VIDEO_TYPE_SNES,
  VIDEO_TYPE_SMS_GG,
  VIDEO_TYPE_GENESIS,
  VIDEO_TYPE_GB,
  VIDEO_TYPE_GBA,
  VIDEO_TYPE_PSX,
  VIDEO_TYPE_N64,
  VIDEO_TYPE_CUSTOM
} video_type_t;

/**
 * @brief Estrutura de contexto para um subsistema de vídeo genérico
 */
typedef struct {
  video_type_t type;   /**< Tipo de vídeo */
  void *video_context; /**< Ponteiro para o contexto específico da PPU */

  /* Funções de operação */
  void (*destroy)(void *);      /**< Função para destruir a PPU */
  void (*reset)(void *);        /**< Função para resetar a PPU */
  int (*execute)(void *, int);  /**< Função para executar ciclos */
  int (*execute_frame)(void *); /**< Função para executar um frame completo */
  uint8_t (*read_register)(void *,
                           uint16_t); /**< Função para ler registrador */
  void (*write_register)(void *, uint16_t,
                         uint8_t); /**< Função para escrever registrador */
} video_context_t;

/**
 * @brief Cria uma instância de vídeo do tipo especificado
 *
 * @param type Tipo de vídeo a ser criado
 * @param config Configuração específica para o tipo de vídeo
 * @return Contexto genérico do vídeo ou NULL em caso de erro
 */
video_context_t *video_create(video_type_t type, void *config) {
  video_context_t *context = (video_context_t *)malloc(sizeof(video_context_t));
  if (!context) {
    return NULL;
  }

  /* Inicializar campos */
  memset(context, 0, sizeof(video_context_t));
  context->type = type;

  /* Converter type para ppu_type_t */
  ppu_type_t ppu_type;
  switch (type) {
  case VIDEO_TYPE_NES:
    ppu_type = PPU_TYPE_NES;
    break;

  case VIDEO_TYPE_SNES:
    ppu_type = PPU_TYPE_SNES;
    break;

  case VIDEO_TYPE_SMS_GG:
    ppu_type = PPU_TYPE_SMS_GG;
    break;

  case VIDEO_TYPE_GENESIS:
    ppu_type = PPU_TYPE_GENESIS;
    break;

  case VIDEO_TYPE_GB:
    ppu_type = PPU_TYPE_GB;
    break;

  case VIDEO_TYPE_GBA:
    ppu_type = PPU_TYPE_GBA;
    break;

  default:
    ppu_type = PPU_TYPE_CUSTOM;
    break;
  }

  /* Atualizar o tipo na configuração */
  ppu_full_config_t *ppu_config = (ppu_full_config_t *)config;
  if (ppu_config) {
    ppu_config->config.type = ppu_type;
  }

  /* Criar a PPU específica */
  context->video_context = ppu_create(ppu_config);
  if (!context->video_context) {
    free(context);
    return NULL;
  }

  /* Configurar funções de operação */
  context->destroy = (void (*)(void *))ppu_destroy;
  context->reset = (void (*)(void *))ppu_reset;
  context->execute = (int (*)(void *, int))ppu_execute;
  context->execute_frame = (int (*)(void *))ppu_execute_frame;
  context->read_register = (uint8_t (*)(void *, uint16_t))ppu_read_register;
  context->write_register =
      (void (*)(void *, uint16_t, uint8_t))ppu_write_register;

  return context;
}

/**
 * @brief Destrói uma instância de vídeo
 *
 * @param context Contexto do vídeo a ser destruído
 */
void video_destroy(video_context_t *context) {
  if (!context) {
    return;
  }

  /* Destruir a PPU específica */
  if (context->video_context && context->destroy) {
    context->destroy(context->video_context);
  }

  /* Liberar o contexto */
  free(context);
}

/**
 * @brief Reseta o subsistema de vídeo para o estado inicial
 *
 * @param context Contexto do vídeo
 */
void video_reset(video_context_t *context) {
  if (!context || !context->video_context || !context->reset) {
    return;
  }

  context->reset(context->video_context);
}

/**
 * @brief Executa o subsistema de vídeo por um número específico de ciclos
 *
 * @param context Contexto do vídeo
 * @param cycles Número de ciclos a executar
 * @return Número de ciclos executados
 */
int video_execute(video_context_t *context, int cycles) {
  if (!context || !context->video_context || !context->execute) {
    return 0;
  }

  return context->execute(context->video_context, cycles);
}

/**
 * @brief Executa o subsistema de vídeo por um frame completo
 *
 * @param context Contexto do vídeo
 * @return Número de ciclos executados
 */
int video_execute_frame(video_context_t *context) {
  if (!context || !context->video_context || !context->execute_frame) {
    return 0;
  }

  return context->execute_frame(context->video_context);
}

/**
 * @brief Lê um registrador do subsistema de vídeo
 *
 * @param context Contexto do vídeo
 * @param reg_id ID do registrador
 * @return Valor do registrador
 */
uint8_t video_read_register(video_context_t *context, uint16_t reg_id) {
  if (!context || !context->video_context || !context->read_register) {
    return 0xFF;
  }

  return context->read_register(context->video_context, reg_id);
}

/**
 * @brief Escreve em um registrador do subsistema de vídeo
 *
 * @param context Contexto do vídeo
 * @param reg_id ID do registrador
 * @param value Valor a escrever
 */
void video_write_register(video_context_t *context, uint16_t reg_id,
                          uint8_t value) {
  if (!context || !context->video_context || !context->write_register) {
    return;
  }

  context->write_register(context->video_context, reg_id, value);
}

/**
 * @brief Obtém o tipo de vídeo
 *
 * @param context Contexto do vídeo
 * @return Tipo de vídeo
 */
video_type_t video_get_type(const video_context_t *context) {
  return context ? context->type : VIDEO_TYPE_CUSTOM;
}

/**
 * @brief Obtém o contexto interno da PPU
 *
 * @param context Contexto do vídeo
 * @return Ponteiro para o contexto interno específico do tipo de PPU
 */
void *video_get_specific_context(const video_context_t *context) {
  return context ? context->video_context : NULL;
}
