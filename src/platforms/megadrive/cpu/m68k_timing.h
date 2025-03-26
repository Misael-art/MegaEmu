#ifndef MD_M68K_TIMING_H
#define MD_M68K_TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Ciclos base para instruções comuns
#define M68K_MOVE_BYTE_CYCLES       4
#define M68K_MOVE_WORD_CYCLES       4
#define M68K_MOVE_LONG_CYCLES       8
#define M68K_ALU_REG_CYCLES         2
#define M68K_ALU_MEM_CYCLES         6
#define M68K_BRANCH_TAKEN_CYCLES    10
#define M68K_BRANCH_NOT_TAKEN_CYCLES 8
#define M68K_JSR_CYCLES             16
#define M68K_RTS_CYCLES             16
#define M68K_MULU_CYCLES            70  // Ciclos base para multiplicação sem sinal
#define M68K_MULS_CYCLES            74  // Ciclos base para multiplicação com sinal
#define M68K_DIVU_CYCLES            140 // Ciclos base para divisão sem sinal
#define M68K_DIVS_CYCLES            158 // Ciclos base para divisão com sinal

// Ciclos adicionais para modos de endereçamento
typedef enum {
    M68K_EA_DATA_REG = 0,          // Dn
    M68K_EA_ADDR_REG = 0,          // An
    M68K_EA_ADDR_INDIRECT = 4,     // (An)
    M68K_EA_POSTINC = 4,           // (An)+
    M68K_EA_PREDEC = 6,            // -(An)
    M68K_EA_DISP16 = 8,           // (d16,An)
    M68K_EA_INDEX = 10,           // (d8,An,Xn)
    M68K_EA_ABS_SHORT = 8,        // (xxx).W
    M68K_EA_ABS_LONG = 12,        // (xxx).L
    M68K_EA_PC_DISP = 8,          // (d16,PC)
    M68K_EA_PC_INDEX = 10,        // (d8,PC,Xn)
    M68K_EA_IMMEDIATE = 4         // #imm
} md_m68k_ea_timing_t;

// Estrutura para timing preciso
typedef struct {
    uint32_t current_cycles;        // Ciclos acumulados
    uint32_t target_cycles;         // Ciclos alvo para sincronização
    uint8_t wait_states;           // Estados de espera para acessos à memória
    uint8_t prefetch_queue;        // Estado da fila de prefetch
    bool is_halted;               // CPU está parada

    // Contadores para profiling
    struct {
        uint32_t instruction_cycles;  // Ciclos gastos em instruções
        uint32_t memory_cycles;       // Ciclos gastos em acessos à memória
        uint32_t wait_cycles;         // Ciclos gastos esperando
        uint32_t total_instructions;  // Total de instruções executadas
    } stats;

    // Estado de sincronização
    struct {
        uint32_t last_sync_cycle;    // Último ciclo de sincronização
        uint8_t z80_sync_pending;    // Sincronização pendente com Z80
        uint8_t vdp_sync_pending;    // Sincronização pendente com VDP
    } sync;

} md_m68k_timing_t;

// Funções de timing
void md_m68k_init_timing(md_m68k_timing_t* timing);
void md_m68k_reset_timing(md_m68k_timing_t* timing);

// Cálculo de ciclos
uint8_t md_m68k_calculate_ea_timing(uint8_t mode, uint8_t reg, bool is_read);
uint8_t md_m68k_calculate_memory_timing(uint32_t address, bool is_write);
uint32_t md_m68k_get_instruction_timing(uint16_t opcode, uint8_t ea_mode, uint8_t ea_reg);

// Funções de sincronização
void md_m68k_sync_with_z80(md_m68k_timing_t* timing);
void md_m68k_wait_for_vdp(md_m68k_timing_t* timing);
void md_m68k_add_cycles(md_m68k_timing_t* timing, uint32_t cycles);
void md_m68k_sync_cycles(md_m68k_timing_t* timing);

// Funções de profiling
void md_m68k_get_timing_stats(const md_m68k_timing_t* timing,
                             uint32_t* instruction_cycles,
                             uint32_t* memory_cycles,
                             uint32_t* wait_cycles,
                             uint32_t* total_instructions);
void md_m68k_reset_timing_stats(md_m68k_timing_t* timing);

// Funções de controle
void md_m68k_set_wait_states(md_m68k_timing_t* timing, uint8_t states);
void md_m68k_request_z80_sync(md_m68k_timing_t* timing);
void md_m68k_request_vdp_sync(md_m68k_timing_t* timing);
bool md_m68k_is_sync_pending(const md_m68k_timing_t* timing);

#ifdef __cplusplus
}
#endif

#endif // MD_M68K_TIMING_H
