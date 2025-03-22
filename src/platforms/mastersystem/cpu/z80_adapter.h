/**
 * @file z80_adapter.h
 * @brief Adaptador do Z80 para o Master System
 *
 * Este arquivo contém a interface do adaptador do Z80 para o Master System,
 * que conecta a implementação base do Z80 com o sistema do Master System.
 */

#ifndef SMS_Z80_ADAPTER_H
#define SMS_Z80_ADAPTER_H

#include <stdint.h>
#include <stdbool.h>
#include "../memory/memory.h"
#include "../../../core/save_state.h"

/**
 * @brief Opaque handle para o adaptador Z80 do Master System
 */
typedef struct sms_z80_adapter_s sms_z80_adapter_t;

/**
 * @brief Cria uma nova instância do adaptador Z80 para o Master System
 *
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_z80_adapter_t* sms_z80_adapter_create(void);

/**
 * @brief Destrói uma instância do adaptador Z80 e libera recursos
 *
 * @param z80 Ponteiro para a instância
 */
void sms_z80_adapter_destroy(sms_z80_adapter_t *z80);

/**
 * @brief Reseta o Z80 para o estado inicial
 *
 * @param z80 Ponteiro para a instância
 */
void sms_z80_adapter_reset(sms_z80_adapter_t *z80);

/**
 * @brief Conecta o sistema de memória ao Z80
 *
 * @param z80 Ponteiro para a instância
 * @param memory Ponteiro para o sistema de memória
 * @return true se conectado com sucesso, false caso contrário
 */
bool sms_z80_adapter_connect_memory(sms_z80_adapter_t *z80, sms_memory_t *memory);

/**
 * @brief Conecta o sistema de vídeo ao Z80
 *
 * @param z80 Ponteiro para a instância
 * @param vdp Ponteiro para o sistema de vídeo
 * @return true se conectado com sucesso, false caso contrário
 */
bool sms_z80_adapter_connect_vdp(sms_z80_adapter_t *z80, void *vdp);

/**
 * @brief Conecta o sistema de áudio ao Z80
 *
 * @param z80 Ponteiro para a instância
 * @param psg Ponteiro para o chip de som PSG
 * @return true se conectado com sucesso, false caso contrário
 */
bool sms_z80_adapter_connect_psg(sms_z80_adapter_t *z80, void *psg);

/**
 * @brief Executa um passo de instrução no Z80
 *
 * @param z80 Ponteiro para a instância
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t sms_z80_adapter_step(sms_z80_adapter_t *z80);

/**
 * @brief Executa um número específico de ciclos no Z80
 *
 * @param z80 Ponteiro para a instância
 * @param cycles Número de ciclos a executar
 * @return Número real de ciclos executados
 */
uint32_t sms_z80_adapter_run(sms_z80_adapter_t *z80, uint32_t cycles);

/**
 * @brief Gera uma interrupção no Z80
 *
 * @param z80 Ponteiro para a instância
 */
void sms_z80_adapter_interrupt(sms_z80_adapter_t *z80);

/**
 * @brief Obtém o valor do registrador PC atual
 *
 * @param z80 Ponteiro para a instância
 * @return Valor atual do PC
 */
uint16_t sms_z80_adapter_get_pc(sms_z80_adapter_t *z80);

/**
 * @brief Registra o Z80 no sistema de save state
 *
 * @param z80 Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_z80_adapter_register_save_state(sms_z80_adapter_t *z80, save_state_t *state);

/**
 * @brief Atualiza o Z80 após um carregamento de save state
 *
 * @param z80 Ponteiro para a instância
 */
void sms_z80_adapter_update_after_state_load(sms_z80_adapter_t *z80);

#endif /* SMS_Z80_ADAPTER_H */
