/**
 * @file m68k_internal.h
 * @brief Definições internas para o processador Motorola 68000 (M68K)
 */

#ifndef MEGA_EMU_M68K_INTERNAL_H
#define MEGA_EMU_M68K_INTERNAL_H

#include "m68k.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Tamanho da tabela de instruções (0xFFFF + 1)
 */
#define M68K_INSTRUCTION_TABLE_SIZE 65536

    /**
     * @brief Tipo de função para manipulador de instrução
     */
    typedef int (*m68k_instruction_handler_t)(m68k_t *cpu, uint16_t opcode);

    /**
     * @brief Estrutura para uma instrução do M68K
     */
    typedef struct
    {
        m68k_instruction_handler_t handler; /* Manipulador da instrução */
        uint8_t cycles;                     /* Ciclos base da instrução */
        char mnemonic[8];                   /* Mnemônico para desassemblagem */
    } m68k_instruction_t;

    /**
     * @brief Tabela global de instruções do M68K
     */
    extern m68k_instruction_t m68k_instruction_table[M68K_INSTRUCTION_TABLE_SIZE];

    /**
     * @brief Inicializa a tabela de instruções do M68K
     */
    void m68k_init_instruction_table(void);

    /**
     * @brief Processa uma interrupção
     * @param cpu Ponteiro para a instância do M68K
     * @return Número de ciclos consumidos
     */
    int m68k_process_interrupt(m68k_t *cpu);

    /**
     * @brief Lê um byte da memória
     * @param cpu Ponteiro para a instância do M68K
     * @param address Endereço de memória
     * @return Valor lido
     */
    uint8_t m68k_read_byte_internal(m68k_t *cpu, uint32_t address);

    /**
     * @brief Lê uma palavra da memória
     * @param cpu Ponteiro para a instância do M68K
     * @param address Endereço de memória
     * @return Valor lido
     */
    uint16_t m68k_read_word_internal(m68k_t *cpu, uint32_t address);

    /**
     * @brief Lê uma palavra longa da memória
     * @param cpu Ponteiro para a instância do M68K
     * @param address Endereço de memória
     * @return Valor lido
     */
    uint32_t m68k_read_long_internal(m68k_t *cpu, uint32_t address);

    /**
     * @brief Escreve um byte na memória
     * @param cpu Ponteiro para a instância do M68K
     * @param address Endereço de memória
     * @param value Valor a escrever
     */
    void m68k_write_byte_internal(m68k_t *cpu, uint32_t address, uint8_t value);

    /**
     * @brief Escreve uma palavra na memória
     * @param cpu Ponteiro para a instância do M68K
     * @param address Endereço de memória
     * @param value Valor a escrever
     */
    void m68k_write_word_internal(m68k_t *cpu, uint32_t address, uint16_t value);

    /**
     * @brief Escreve uma palavra longa na memória
     * @param cpu Ponteiro para a instância do M68K
     * @param address Endereço de memória
     * @param value Valor a escrever
     */
    void m68k_write_long_internal(m68k_t *cpu, uint32_t address, uint32_t value);

    /**
     * @brief Calcula o valor efetivo para um modo de endereçamento
     * @param cpu Ponteiro para a instância do M68K
     * @param mode Modo de endereçamento
     * @param reg Registrador
     * @param size Tamanho do operando (1=byte, 2=word, 4=long)
     * @param address Ponteiro para armazenar o endereço calculado (para modos indiretos)
     * @return Valor efetivo calculado
     */
    uint32_t m68k_get_effective_address(m68k_t *cpu, uint8_t mode, uint8_t reg,
                                        uint8_t size, uint32_t *address);

    /**
     * @brief Verifica se uma condição é verdadeira com base no código de condição
     * @param cpu Ponteiro para a instância do M68K
     * @param condition Código de condição (0-15)
     * @return true se a condição for verdadeira, false caso contrário
     */
    bool m68k_test_condition(m68k_t *cpu, uint8_t condition);

/* Códigos de condição */
#define M68K_COND_TRUE 0  /* Sempre verdadeiro */
#define M68K_COND_FALSE 1 /* Sempre falso */
#define M68K_COND_HI 2    /* Maior que (não assinado) */
#define M68K_COND_LS 3    /* Menor ou igual (não assinado) */
#define M68K_COND_CC 4    /* Carry clear */
#define M68K_COND_CS 5    /* Carry set */
#define M68K_COND_NE 6    /* Não igual */
#define M68K_COND_EQ 7    /* Igual */
#define M68K_COND_VC 8    /* Overflow clear */
#define M68K_COND_VS 9    /* Overflow set */
#define M68K_COND_PL 10   /* Plus (positivo) */
#define M68K_COND_MI 11   /* Minus (negativo) */
#define M68K_COND_GE 12   /* Maior ou igual (assinado) */
#define M68K_COND_LT 13   /* Menor que (assinado) */
#define M68K_COND_GT 14   /* Maior que (assinado) */
#define M68K_COND_LE 15   /* Menor ou igual (assinado) */

/* Modos de endereçamento */
#define M68K_ADDR_MODE_REG_DIRECT 0    /* Registrador direto */
#define M68K_ADDR_MODE_ADDR_DIRECT 1   /* Endereço direto */
#define M68K_ADDR_MODE_ADDR_INDIRECT 2 /* Endereço indireto */
#define M68K_ADDR_MODE_ADDR_POSTINC 3  /* Endereço indireto com pós-incremento */
#define M68K_ADDR_MODE_ADDR_PREDEC 4   /* Endereço indireto com pré-decremento */
#define M68K_ADDR_MODE_ADDR_DISP 5     /* Endereço indireto com deslocamento */
#define M68K_ADDR_MODE_ADDR_INDEX 6    /* Endereço indireto indexado */
#define M68K_ADDR_MODE_PC_DISP 7       /* PC com deslocamento */
#define M68K_ADDR_MODE_PC_INDEX 8      /* PC indexado */
#define M68K_ADDR_MODE_ABS_SHORT 9     /* Endereço absoluto curto */
#define M68K_ADDR_MODE_ABS_LONG 10     /* Endereço absoluto longo */
#define M68K_ADDR_MODE_IMMEDIATE 11    /* Valor imediato */

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_M68K_INTERNAL_H */
