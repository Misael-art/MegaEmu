/**
 * @file z80.h
 * @brief Interface C para a CPU Z80
 *
 * Este arquivo define a interface pública da implementação do processador Z80,
 * seguindo o padrão de arquitetura híbrida para permitir compatibilidade entre
 * C e C++.
 */

#ifndef EMU_Z80_H
#define EMU_Z80_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flags de status do Z80
 */
typedef enum {
    Z80_FLAG_CARRY     = 0x01,
    Z80_FLAG_SUBTRACT  = 0x02,
    Z80_FLAG_PARITY    = 0x04,
    Z80_FLAG_OVERFLOW  = 0x04, // Alias para PARITY
    Z80_FLAG_HALF_CARRY = 0x10,
    Z80_FLAG_ZERO      = 0x40,
    Z80_FLAG_SIGN      = 0x80
} z80_flag_t;

/**
 * @brief Registradores do Z80
 */
typedef enum {
    Z80_REG_A = 0,
    Z80_REG_F,
    Z80_REG_B,
    Z80_REG_C,
    Z80_REG_D,
    Z80_REG_E,
    Z80_REG_H,
    Z80_REG_L,
    Z80_REG_A_PRIME,
    Z80_REG_F_PRIME,
    Z80_REG_B_PRIME,
    Z80_REG_C_PRIME,
    Z80_REG_D_PRIME,
    Z80_REG_E_PRIME,
    Z80_REG_H_PRIME,
    Z80_REG_L_PRIME,
    Z80_REG_IX_HIGH,
    Z80_REG_IX_LOW,
    Z80_REG_IY_HIGH,
    Z80_REG_IY_LOW,
    Z80_REG_SP_HIGH,
    Z80_REG_SP_LOW,
    Z80_REG_PC_HIGH,
    Z80_REG_PC_LOW,
    Z80_REG_I,
    Z80_REG_R,
    Z80_REG_COUNT
} z80_register_t;

/**
 * @brief Modos de interrupção do Z80
 */
typedef enum {
    Z80_INTERRUPT_MODE_0 = 0,
    Z80_INTERRUPT_MODE_1,
    Z80_INTERRUPT_MODE_2
} z80_interrupt_mode_t;

/**
 * @brief Tipo opaco para o contexto do Z80
 */
typedef struct z80_context_s z80_t;

/**
 * @brief Callback para leitura de memória
 *
 * @param context Contexto do usuário
 * @param address Endereço a ler
 * @return Byte lido
 */
typedef uint8_t (*z80_read_callback_t)(void* context, uint16_t address);

/**
 * @brief Callback para escrita em memória
 *
 * @param context Contexto do usuário
 * @param address Endereço para escrita
 * @param value Valor a escrever
 */
typedef void (*z80_write_callback_t)(void* context, uint16_t address, uint8_t value);

/**
 * @brief Callback para leitura de porta I/O
 *
 * @param context Contexto do usuário
 * @param port Porta a ler
 * @return Byte lido
 */
typedef uint8_t (*z80_port_read_callback_t)(void* context, uint16_t port);

/**
 * @brief Callback para escrita em porta I/O
 *
 * @param context Contexto do usuário
 * @param port Porta para escrita
 * @param value Valor a escrever
 */
typedef void (*z80_port_write_callback_t)(void* context, uint16_t port, uint8_t value);

/**
 * @brief Configura a CPU Z80
 */
typedef struct {
    z80_read_callback_t read_memory;           /**< Callback para leitura de memória */
    z80_write_callback_t write_memory;         /**< Callback para escrita em memória */
    z80_port_read_callback_t read_io;          /**< Callback para leitura de porta I/O */
    z80_port_write_callback_t write_io;        /**< Callback para escrita em porta I/O */
    void* context;                            /**< Contexto do usuário para callbacks */
} z80_config_t;

/**
 * @brief Cria uma nova instância do Z80
 *
 * @param config Configuração do Z80
 * @return Instância do Z80 ou NULL em caso de erro
 */
z80_t* z80_create(const z80_config_t* config);

/**
 * @brief Destrói uma instância do Z80 e libera recursos
 *
 * @param cpu Instância do Z80
 */
void z80_destroy(z80_t* cpu);

/**
 * @brief Reseta a CPU para estado inicial
 *
 * @param cpu Instância do Z80
 */
void z80_reset(z80_t* cpu);

/**
 * @brief Executa um número específico de ciclos
 *
 * @param cpu Instância do Z80
 * @param cycles Número de ciclos a executar (ou 0 para uma instrução completa)
 * @return Número de ciclos realmente executados
 */
int z80_execute(z80_t* cpu, int cycles);

/**
 * @brief Executa uma instrução completa
 *
 * @param cpu Instância do Z80
 * @return Número de ciclos consumidos
 */
int z80_step(z80_t* cpu);

/**
 * @brief Dispara uma interrupção maskable (INT)
 *
 * @param cpu Instância do Z80
 * @param data Dado no barramento (usado no modo 0)
 * @return Número de ciclos consumidos (0 se interrupção ignorada)
 */
int z80_interrupt(z80_t* cpu, uint8_t data);

/**
 * @brief Dispara uma interrupção não-maskable (NMI)
 *
 * @param cpu Instância do Z80
 * @return Número de ciclos consumidos
 */
int z80_nmi(z80_t* cpu);

/**
 * @brief Obtém o valor de um registrador
 *
 * @param cpu Instância do Z80
 * @param reg Registrador a consultar
 * @return Valor do registrador
 */
uint8_t z80_get_register(const z80_t* cpu, z80_register_t reg);

/**
 * @brief Define o valor de um registrador
 *
 * @param cpu Instância do Z80
 * @param reg Registrador a modificar
 * @param value Novo valor
 */
void z80_set_register(z80_t* cpu, z80_register_t reg, uint8_t value);

/**
 * @brief Obtém o valor de um registrador de 16 bits
 *
 * @param cpu Instância do Z80
 * @param reg_high Registrador alto (MSB)
 * @param reg_low Registrador baixo (LSB)
 * @return Valor de 16 bits
 */
uint16_t z80_get_register_pair(const z80_t* cpu, z80_register_t reg_high, z80_register_t reg_low);

/**
 * @brief Define o valor de um registrador de 16 bits
 *
 * @param cpu Instância do Z80
 * @param reg_high Registrador alto (MSB)
 * @param reg_low Registrador baixo (LSB)
 * @param value Valor de 16 bits
 */
void z80_set_register_pair(z80_t* cpu, z80_register_t reg_high, z80_register_t reg_low, uint16_t value);

/**
 * @brief Obtém o valor do registrador AF
 *
 * @param cpu Instância do Z80
 * @return Valor de AF
 */
uint16_t z80_get_af(const z80_t* cpu);

/**
 * @brief Define o valor do registrador AF
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_af(z80_t* cpu, uint16_t value);

/**
 * @brief Obtém o valor do registrador BC
 *
 * @param cpu Instância do Z80
 * @return Valor de BC
 */
uint16_t z80_get_bc(const z80_t* cpu);

/**
 * @brief Define o valor do registrador BC
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_bc(z80_t* cpu, uint16_t value);

/**
 * @brief Obtém o valor do registrador DE
 *
 * @param cpu Instância do Z80
 * @return Valor de DE
 */
uint16_t z80_get_de(const z80_t* cpu);

/**
 * @brief Define o valor do registrador DE
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_de(z80_t* cpu, uint16_t value);

/**
 * @brief Obtém o valor do registrador HL
 *
 * @param cpu Instância do Z80
 * @return Valor de HL
 */
uint16_t z80_get_hl(const z80_t* cpu);

/**
 * @brief Define o valor do registrador HL
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_hl(z80_t* cpu, uint16_t value);

/**
 * @brief Obtém o valor do registrador IX
 *
 * @param cpu Instância do Z80
 * @return Valor de IX
 */
uint16_t z80_get_ix(const z80_t* cpu);

/**
 * @brief Define o valor do registrador IX
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_ix(z80_t* cpu, uint16_t value);

/**
 * @brief Obtém o valor do registrador IY
 *
 * @param cpu Instância do Z80
 * @return Valor de IY
 */
uint16_t z80_get_iy(const z80_t* cpu);

/**
 * @brief Define o valor do registrador IY
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_iy(z80_t* cpu, uint16_t value);

/**
 * @brief Obtém o valor do registrador SP (stack pointer)
 *
 * @param cpu Instância do Z80
 * @return Valor de SP
 */
uint16_t z80_get_sp(const z80_t* cpu);

/**
 * @brief Define o valor do registrador SP (stack pointer)
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_sp(z80_t* cpu, uint16_t value);

/**
 * @brief Obtém o valor do registrador PC (program counter)
 *
 * @param cpu Instância do Z80
 * @return Valor de PC
 */
uint16_t z80_get_pc(const z80_t* cpu);

/**
 * @brief Define o valor do registrador PC (program counter)
 *
 * @param cpu Instância do Z80
 * @param value Novo valor
 */
void z80_set_pc(z80_t* cpu, uint16_t value);

/**
 * @brief Verifica se um flag específico está ativo
 *
 * @param cpu Instância do Z80
 * @param flag Flag a verificar
 * @return true se ativo, false caso contrário
 */
bool z80_check_flag(const z80_t* cpu, z80_flag_t flag);

/**
 * @brief Define o estado de um flag
 *
 * @param cpu Instância do Z80
 * @param flag Flag a modificar
 * @param state Novo estado (true=ativo, false=inativo)
 */
void z80_set_flag(z80_t* cpu, z80_flag_t flag, bool state);

/**
 * @brief Obtém o modo de interrupção atual
 *
 * @param cpu Instância do Z80
 * @return Modo de interrupção
 */
z80_interrupt_mode_t z80_get_interrupt_mode(const z80_t* cpu);

/**
 * @brief Define o modo de interrupção
 *
 * @param cpu Instância do Z80
 * @param mode Novo modo de interrupção
 */
void z80_set_interrupt_mode(z80_t* cpu, z80_interrupt_mode_t mode);

/**
 * @brief Verifica se interrupções estão habilitadas (IFF1)
 *
 * @param cpu Instância do Z80
 * @return true se habilitadas, false caso contrário
 */
bool z80_interrupts_enabled(const z80_t* cpu);

/**
 * @brief Define o estado de habilitação de interrupções
 *
 * @param cpu Instância do Z80
 * @param enabled Novo estado
 */
void z80_set_interrupts_enabled(z80_t* cpu, bool enabled);

/**
 * @brief Salva o estado da CPU em um buffer
 *
 * @param cpu Instância do Z80
 * @param buffer Buffer para armazenar estado (deve ter tamanho suficiente)
 * @param buffer_size Tamanho do buffer
 * @return Número de bytes escritos ou -1 em caso de erro
 */
int z80_save_state(const z80_t* cpu, uint8_t* buffer, size_t buffer_size);

/**
 * @brief Carrega o estado da CPU de um buffer
 *
 * @param cpu Instância do Z80
 * @param buffer Buffer contendo estado salvo
 * @param buffer_size Tamanho do buffer
 * @return 0 em caso de sucesso ou -1 em caso de erro
 */
int z80_load_state(z80_t* cpu, const uint8_t* buffer, size_t buffer_size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* EMU_Z80_H */
