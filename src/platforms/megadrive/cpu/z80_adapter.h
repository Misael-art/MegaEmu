/**
 * @file z80_adapter.h
 * @brief Adaptador do Z80 para o Mega Drive
 *
 * Este arquivo contém a interface do adaptador do Z80 para o Mega Drive,
 * que conecta a implementação base do Z80 com o sistema do Mega Drive.
 */

#ifndef MD_Z80_ADAPTER_H
#define MD_Z80_ADAPTER_H

#include <stdint.h>
#include <stdbool.h>
#include "../memory/memory.h"
#include "../audio/audio.h"

/**
 * @brief Opaque handle para o adaptador Z80 do Mega Drive
 */
typedef struct md_z80_adapter_s md_z80_adapter_t;

/**
 * @brief Cria uma nova instância do adaptador Z80 para o Mega Drive
 *
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
md_z80_adapter_t* md_z80_adapter_create(void);

/**
 * @brief Destrói uma instância do adaptador Z80 e libera recursos
 *
 * @param z80 Ponteiro para a instância
 */
void md_z80_adapter_destroy(md_z80_adapter_t *z80);

/**
 * @brief Reseta o Z80 para o estado inicial
 *
 * @param z80 Ponteiro para a instância
 */
void md_z80_adapter_reset(md_z80_adapter_t *z80);

/**
 * @brief Conecta o sistema de memória ao Z80
 *
 * @param z80 Ponteiro para a instância
 * @param memory Ponteiro para o sistema de memória
 * @return true se conectado com sucesso, false caso contrário
 */
bool md_z80_adapter_connect_memory(md_z80_adapter_t *z80, md_memory_t *memory);

/**
 * @brief Conecta o sistema de áudio ao Z80
 *
 * @param z80 Ponteiro para a instância
 * @param audio Ponteiro para o sistema de áudio
 * @return true se conectado com sucesso, false caso contrário
 */
bool md_z80_adapter_connect_audio(md_z80_adapter_t *z80, md_audio_t *audio);

/**
 * @brief Executa um passo de instrução no Z80
 *
 * @param z80 Ponteiro para a instância
 * @return Número de ciclos consumidos pela instrução
 */
uint8_t md_z80_adapter_step(md_z80_adapter_t *z80);

/**
 * @brief Executa um número específico de ciclos no Z80
 *
 * @param z80 Ponteiro para a instância
 * @param cycles Número de ciclos a executar
 * @return Número real de ciclos executados
 */
uint32_t md_z80_adapter_run(md_z80_adapter_t *z80, uint32_t cycles);

/**
 * @brief Gera uma interrupção no Z80
 *
 * @param z80 Ponteiro para a instância
 */
void md_z80_adapter_interrupt(md_z80_adapter_t *z80);

/**
 * @brief Define o estado de reset do Z80
 *
 * No Mega Drive, o 68000 pode resetar o Z80
 *
 * @param z80 Ponteiro para a instância
 * @param reset true para resetar, false para liberar do reset
 */
void md_z80_adapter_set_reset(md_z80_adapter_t *z80, bool reset);

/**
 * @brief Define o estado de bus request do Z80
 *
 * No Mega Drive, o 68000 pode solicitar o barramento do Z80
 *
 * @param z80 Ponteiro para a instância
 * @param request true para solicitar, false para liberar
 */
void md_z80_adapter_set_busreq(md_z80_adapter_t *z80, bool request);

/**
 * @brief Obtém o estado atual de bus request do Z80
 *
 * @param z80 Ponteiro para a instância
 * @return true se o barramento está sendo solicitado, false caso contrário
 */
bool md_z80_adapter_get_busreq(md_z80_adapter_t *z80);

/**
 * @brief Define o banco de memória para o Z80 acessar a memória principal
 *
 * @param z80 Ponteiro para a instância
 * @param bank Banco de memória (0-511)
 */
void md_z80_adapter_set_bank(md_z80_adapter_t *z80, uint16_t bank);

/**
 * @brief Registra o Z80 no sistema de save state
 *
 * @param z80 Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int md_z80_adapter_register_save_state(md_z80_adapter_t *z80, save_state_t *state);

#endif /* MD_Z80_ADAPTER_H */
