/**
 * @file m68k.h
 * @brief Definições e funções principais da CPU Motorola 68000
 */

#ifndef MD_M68K_H
#define MD_M68K_H

#include "../../../core/interfaces/cpu_interface.h"
#include "../../../core/memory/memory_interface.h"
#include "m68k_timing.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Estrutura que representa o estado da CPU M68K
 */
typedef struct {
  // Registradores de dados (D0-D7)
  uint32_t data_regs[8];

  // Registradores de endereço (A0-A7)
  uint32_t addr_regs[8];

  // Program Counter
  uint32_t pc;

  // Status Register
  uint16_t sr;

  // Sistema de timing preciso
  md_m68k_timing_t timing;

  // Cache de instruções
  struct {
    uint16_t prefetch_queue[4];  // Fila de prefetch
    uint8_t queue_size;          // Tamanho atual da fila
    uint32_t fetch_address;      // Endereço do próximo fetch
  } instruction_cache;

  // Estado da CPU
  uint8_t halted;
  uint8_t stopped;
  uint8_t supervisor_mode;
  uint8_t trace_mode;

  // Controle de interrupções
  struct {
    uint8_t pending_level;      // Nível de interrupção pendente
    uint8_t current_level;      // Nível de interrupção atual
    uint8_t mask;              // Máscara de interrupção
    bool auto_vector;          // Modo auto-vector ativo
  } interrupt;

} md_m68k_state_t;

/* Funções de acesso aos registradores */

/**
 * @brief Obtém o valor de um registrador de dados
 * @param reg Número do registrador (0-7)
 * @return Valor do registrador
 */
uint32_t md_m68k_get_data_reg(uint8_t reg);

/**
 * @brief Define o valor de um registrador de dados
 * @param reg Número do registrador (0-7)
 * @param value Valor a ser definido
 */
void md_m68k_set_data_reg(uint8_t reg, uint32_t value);

/**
 * @brief Obtém o valor de um registrador de endereço
 * @param reg Número do registrador (0-7)
 * @return Valor do registrador
 */
uint32_t md_m68k_get_addr_reg(uint8_t reg);

/**
 * @brief Define o valor de um registrador de endereço
 * @param reg Número do registrador (0-7)
 * @param value Valor a ser definido
 */
void md_m68k_set_addr_reg(uint8_t reg, uint32_t value);

/**
 * @brief Obtém o valor do registrador de status
 * @return Valor do SR
 */
uint16_t md_m68k_get_sr(void);

/**
 * @brief Define o valor do registrador de status
 * @param value Novo valor do SR
 */
void md_m68k_set_sr(uint16_t value);

/**
 * @brief Define o valor do contador de programa
 * @param value Novo valor do PC
 */
void md_m68k_set_pc(uint32_t value);

/* Funções de acesso à memória */

/**
 * @brief Lê um uint8_t da memória com timing preciso
 * @param address Endereço de memória
 * @return uint8_t lido
 */
uint8_t md_m68k_read_memory_8(uint32_t address);

/**
 * @brief Lê uma uint16_t da memória com timing preciso
 * @param address Endereço de memória
 * @return uint16_t lida
 */
uint16_t md_m68k_read_memory_16(uint32_t address);

/**
 * @brief Lê uma long uint16_t da memória com timing preciso
 * @param address Endereço de memória
 * @return Long uint16_t lida
 */
uint32_t md_m68k_read_memory_32(uint32_t address);

/**
 * @brief Escreve um uint8_t na memória com timing preciso
 * @param address Endereço de memória
 * @param value Valor a ser escrito
 */
void md_m68k_write_memory_8(uint32_t address, uint8_t value);

/**
 * @brief Escreve uma uint16_t na memória com timing preciso
 * @param address Endereço de memória
 * @param value Valor a ser escrito
 */
void md_m68k_write_memory_16(uint32_t address, uint16_t value);

/**
 * @brief Escreve uma long uint16_t na memória com timing preciso
 * @param address Endereço de memória
 * @param value Valor a ser escrito
 */
void md_m68k_write_memory_32(uint32_t address, uint32_t value);

/* Funções de controle da CPU */

/**
 * @brief Inicializa a CPU M68K
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int32_t md_m68k_init(void);

/**
 * @brief Reseta a CPU M68K
 */
void md_m68k_reset(void);

/**
 * @brief Executa um ciclo da CPU com timing preciso
 * @return Número de ciclos executados
 */
int32_t md_m68k_step(void);

/**
 * @brief Gera uma interrupção com timing preciso
 * @param level Nível da interrupção (1-7)
 * @param auto_vector Define se é uma interrupção auto-vetorizada
 */
void md_m68k_interrupt(uint8_t level, bool auto_vector);

/**
 * @brief Obtém a interface da CPU M68K
 * @return Ponteiro para a interface da CPU
 */
emu_cpu_t md_m68k_get_interface(void);

/* Funções de controle de timing */

/**
 * @brief Obtém o número atual de ciclos
 * @return Ciclos atuais
 */
uint32_t md_m68k_get_cycles(void);

/**
 * @brief Define o número de ciclos alvo para sincronização
 * @param cycles Número de ciclos
 */
void md_m68k_set_target_cycles(uint32_t cycles);

/**
 * @brief Sincroniza a CPU com outros componentes
 */
void md_m68k_sync(void);

/**
 * @brief Obtém estatísticas de timing da CPU
 * @param instruction_cycles Ciclos gastos em instruções
 * @param memory_cycles Ciclos gastos em acessos à memória
 * @param wait_cycles Ciclos gastos esperando
 * @param total_instructions Total de instruções executadas
 */
void md_m68k_get_timing_stats(uint32_t* instruction_cycles,
                             uint32_t* memory_cycles,
                             uint32_t* wait_cycles,
                             uint32_t* total_instructions);

#ifdef __cplusplus
}
#endif

#endif /* MD_M68K_H */
