/**
 * @file m68k.h
 * @brief API principal para o processador Motorola 68000 (M68K)
 */

#ifndef MEGA_EMU_M68K_H
#define MEGA_EMU_M68K_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Estrutura principal do processador M68K
     */
    typedef struct m68k_s
    {
        /* Registradores */
        uint32_t d[8]; /* Registradores de dados D0-D7 */
        uint32_t a[8]; /* Registradores de endereço A0-A7 */
        uint32_t pc;   /* Program Counter */
        uint16_t sr;   /* Status Register */

        /* Estado interno */
        bool stopped;          /* Estado STOP */
        int pending_interrupt; /* Interrupção pendente */
        int interrupt_level;   /* Nível de interrupção atual */
        int cycles_remaining;  /* Ciclos restantes na execução atual */
        int cycles_executed;   /* Ciclos executados na última operação */

        /* Callbacks de acesso à memória */
        uint8_t (*read_byte)(void *context, uint32_t address);
        uint16_t (*read_word)(void *context, uint32_t address);
        uint32_t (*read_long)(void *context, uint32_t address);
        void (*write_byte)(void *context, uint32_t address, uint8_t value);
        void (*write_word)(void *context, uint32_t address, uint16_t value);
        void (*write_long)(void *context, uint32_t address, uint32_t value);

        /* Contexto do sistema */
        void *context;
    } m68k_t;

    /**
     * @brief Cria uma nova instância do processador M68K
     * @return Ponteiro para a instância do M68K ou NULL em caso de erro
     */
    m68k_t *m68k_create(void);

    /**
     * @brief Destrói uma instância do processador M68K
     * @param cpu Ponteiro para a instância do M68K
     */
    void m68k_destroy(m68k_t *cpu);

    /**
     * @brief Inicializa o processador M68K
     * @param cpu Ponteiro para a instância do M68K
     */
    void m68k_init(m68k_t *cpu);

    /**
     * @brief Reseta o processador M68K
     * @param cpu Ponteiro para a instância do M68K
     */
    void m68k_reset(m68k_t *cpu);

    /**
     * @brief Executa um número específico de ciclos no processador M68K
     * @param cpu Ponteiro para a instância do M68K
     * @param cycles Número de ciclos a executar
     * @return Número de ciclos executados
     */
    int m68k_execute_cycles(m68k_t *cpu, int cycles);

    /**
     * @brief Define o nível de interrupção
     * @param cpu Ponteiro para a instância do M68K
     * @param level Nível de interrupção (0-7)
     */
    void m68k_set_irq(m68k_t *cpu, int level);

    /**
     * @brief Obtém o valor de um registrador
     * @param cpu Ponteiro para a instância do M68K
     * @param reg Índice do registrador
     * @return Valor do registrador
     */
    uint32_t m68k_get_register(m68k_t *cpu, int reg);

    /**
     * @brief Define o valor de um registrador
     * @param cpu Ponteiro para a instância do M68K
     * @param reg Índice do registrador
     * @param value Valor a definir
     */
    void m68k_set_register(m68k_t *cpu, int reg, uint32_t value);

/* Constantes para índices de registradores */
#define M68K_REG_D0 0
#define M68K_REG_D1 1
#define M68K_REG_D2 2
#define M68K_REG_D3 3
#define M68K_REG_D4 4
#define M68K_REG_D5 5
#define M68K_REG_D6 6
#define M68K_REG_D7 7
#define M68K_REG_A0 8
#define M68K_REG_A1 9
#define M68K_REG_A2 10
#define M68K_REG_A3 11
#define M68K_REG_A4 12
#define M68K_REG_A5 13
#define M68K_REG_A6 14
#define M68K_REG_A7 15
#define M68K_REG_PC 16
#define M68K_REG_SR 17

/* Bits do Status Register */
#define M68K_SR_C 0x0001  /* Carry */
#define M68K_SR_V 0x0002  /* Overflow */
#define M68K_SR_Z 0x0004  /* Zero */
#define M68K_SR_N 0x0008  /* Negative */
#define M68K_SR_X 0x0010  /* Extend */
#define M68K_SR_I0 0x0100 /* Interrupt mask level bit 0 */
#define M68K_SR_I1 0x0200 /* Interrupt mask level bit 1 */
#define M68K_SR_I2 0x0400 /* Interrupt mask level bit 2 */
#define M68K_SR_S 0x2000  /* Supervisor mode */
#define M68K_SR_T 0x8000  /* Trace mode */

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_M68K_H */
