/**
 * @file cpu_common.c
 * @brief Interface comum para diferentes implementações de CPU
 *
 * Este arquivo fornece funcionalidades comuns e uma camada de abstração
 * para diferentes implementações de CPUs.
 */

#include "z80/z80.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Tipo de CPU disponível
 */
typedef enum {
  CPU_TYPE_Z80 = 0,
  CPU_TYPE_6502,
  CPU_TYPE_68000,
  CPU_TYPE_65C816,
  CPU_TYPE_ARM7,
  CPU_TYPE_SH2,
  CPU_TYPE_SH4,
  CPU_TYPE_MIPS,
  CPU_TYPE_PPC,
  CPU_TYPE_CUSTOM
} cpu_type_t;

/**
 * @brief Estrutura de contexto para uma CPU genérica
 */
typedef struct {
  cpu_type_t type;   /**< Tipo de CPU */
  void *cpu_context; /**< Ponteiro para o contexto específico da CPU */

  /* Funções de operação */
  void (*destroy)(void *);           /**< Função para destruir a CPU */
  void (*reset)(void *);             /**< Função para resetar a CPU */
  int (*execute)(void *, int);       /**< Função para executar ciclos */
  int (*interrupt)(void *, uint8_t); /**< Função para sinalizar interrupção */
} cpu_context_t;

/**
 * @brief Cria uma instância de CPU do tipo especificado
 *
 * @param type Tipo de CPU a ser criada
 * @param config Configuração específica para o tipo de CPU
 * @return Contexto genérico da CPU ou NULL em caso de erro
 */
cpu_context_t *cpu_create(cpu_type_t type, void *config) {
  cpu_context_t *context = (cpu_context_t *)malloc(sizeof(cpu_context_t));
  if (!context) {
    return NULL;
  }

  /* Inicializar campos */
  memset(context, 0, sizeof(cpu_context_t));
  context->type = type;

  /* Criar a CPU específica */
  switch (type) {
  case CPU_TYPE_Z80:
    context->cpu_context = z80_create((z80_config_t *)config);
    if (!context->cpu_context) {
      free(context);
      return NULL;
    }

    /* Configurar funções de operação */
    context->destroy = (void (*)(void *))z80_destroy;
    context->reset = (void (*)(void *))z80_reset;
    context->execute = (int (*)(void *, int))z80_execute;
    context->interrupt = (int (*)(void *, uint8_t))z80_interrupt;
    break;

  /* Adicionar casos para outros tipos de CPU quando necessário */
  default:
    /* Tipo de CPU não suportado */
    free(context);
    return NULL;
  }

  return context;
}

/**
 * @brief Destrói uma instância de CPU
 *
 * @param context Contexto da CPU a ser destruída
 */
void cpu_destroy(cpu_context_t *context) {
  if (!context) {
    return;
  }

  /* Destruir a CPU específica */
  if (context->cpu_context && context->destroy) {
    context->destroy(context->cpu_context);
  }

  /* Liberar o contexto */
  free(context);
}

/**
 * @brief Reseta a CPU para o estado inicial
 *
 * @param context Contexto da CPU
 */
void cpu_reset(cpu_context_t *context) {
  if (!context || !context->cpu_context || !context->reset) {
    return;
  }

  context->reset(context->cpu_context);
}

/**
 * @brief Executa a CPU por um número específico de ciclos
 *
 * @param context Contexto da CPU
 * @param cycles Número de ciclos a executar (0 para um ciclo completo)
 * @return Número de ciclos executados
 */
int cpu_execute(cpu_context_t *context, int cycles) {
  if (!context || !context->cpu_context || !context->execute) {
    return 0;
  }

  return context->execute(context->cpu_context, cycles);
}

/**
 * @brief Sinaliza uma interrupção para a CPU
 *
 * @param context Contexto da CPU
 * @param data Dados da interrupção (depende do tipo de CPU)
 * @return Número de ciclos consumidos ou 0 se não tratada
 */
int cpu_interrupt(cpu_context_t *context, uint8_t data) {
  if (!context || !context->cpu_context || !context->interrupt) {
    return 0;
  }

  return context->interrupt(context->cpu_context, data);
}

/**
 * @brief Obtém o tipo de CPU
 *
 * @param context Contexto da CPU
 * @return Tipo de CPU
 */
cpu_type_t cpu_get_type(const cpu_context_t *context) {
  return context ? context->type : CPU_TYPE_CUSTOM;
}

/**
 * @brief Obtém o contexto interno da CPU
 *
 * @param context Contexto da CPU
 * @return Ponteiro para o contexto interno específico do tipo de CPU
 */
void *cpu_get_specific_context(const cpu_context_t *context) {
  return context ? context->cpu_context : NULL;
}
