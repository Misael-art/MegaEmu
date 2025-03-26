/**
 * @file z80.c
 * @brief Implementação da CPU Z80
 *
 * Implementação do processador Z80 seguindo o padrão de arquitetura híbrida.
 */

#include "z80.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Contexto interno da CPU Z80
 */
struct z80_context_s {
  /* Registradores principais */
  uint8_t a, f; /* Acumulador e Flags */
  uint8_t b, c; /* Registrador BC */
  uint8_t d, e; /* Registrador DE */
  uint8_t h, l; /* Registrador HL */

  /* Registradores alternativos */
  uint8_t a_prime, f_prime;
  uint8_t b_prime, c_prime;
  uint8_t d_prime, e_prime;
  uint8_t h_prime, l_prime;

  /* Registradores especiais */
  uint8_t i; /* Registrador de interrupção */
  uint8_t r; /* Registrador de refresh de memória */

  /* Registradores de 16 bits */
  uint16_t ix; /* Registrador IX */
  uint16_t iy; /* Registrador IY */
  uint16_t sp; /* Stack Pointer */
  uint16_t pc; /* Program Counter */

  /* Estado de interrupção */
  bool iff1;               /* Interrupt Flip-Flop 1 */
  bool iff2;               /* Interrupt Flip-Flop 2 */
  z80_interrupt_mode_t im; /* Modo de interrupção */

  /* Estado de execução */
  int cycles;  /* Ciclos acumulados */
  bool halted; /* Estado de HALT */

  /* Callbacks para acesso à memória e I/O */
  z80_config_t config;
};

/* Tamanho do estado salvo em bytes */
#define Z80_STATE_SIZE 64

/**
 * @brief Cria uma nova instância do Z80
 */
z80_t *z80_create(const z80_config_t *config) {
  if (!config || !config->read_memory || !config->write_memory) {
    return NULL;
  }

  z80_t *cpu = (z80_t *)calloc(1, sizeof(z80_t));
  if (!cpu) {
    return NULL;
  }

  /* Copiar configuração */
  cpu->config = *config;

  /* Inicializar estado */
  z80_reset(cpu);

  return cpu;
}

/**
 * @brief Destrói uma instância do Z80 e libera recursos
 */
void z80_destroy(z80_t *cpu) {
  if (cpu) {
    free(cpu);
  }
}

/**
 * @brief Reseta a CPU para estado inicial
 */
void z80_reset(z80_t *cpu) {
  if (!cpu) {
    return;
  }

  /* Reset dos registradores */
  cpu->a = cpu->f = 0;
  cpu->b = cpu->c = 0;
  cpu->d = cpu->e = 0;
  cpu->h = cpu->l = 0;

  cpu->a_prime = cpu->f_prime = 0;
  cpu->b_prime = cpu->c_prime = 0;
  cpu->d_prime = cpu->e_prime = 0;
  cpu->h_prime = cpu->l_prime = 0;

  cpu->i = cpu->r = 0;

  cpu->ix = cpu->iy = 0;
  cpu->sp = 0xFFFF;
  cpu->pc = 0;

  /* Reset do estado de interrupção */
  cpu->iff1 = cpu->iff2 = false;
  cpu->im = Z80_INTERRUPT_MODE_0;

  /* Reset do estado de execução */
  cpu->cycles = 0;
  cpu->halted = false;
}

/**
 * @brief Lê um byte da memória
 */
static inline uint8_t read_memory(z80_t *cpu, uint16_t address) {
  return cpu->config.read_memory(cpu->config.context, address);
}

/**
 * @brief Escreve um byte na memória
 */
static inline void write_memory(z80_t *cpu, uint16_t address, uint8_t value) {
  cpu->config.write_memory(cpu->config.context, address, value);
}

/**
 * @brief Lê um byte de uma porta I/O
 */
static inline uint8_t read_io(z80_t *cpu, uint16_t port) {
  if (cpu->config.read_io) {
    return cpu->config.read_io(cpu->config.context, port);
  }
  return 0xFF;
}

/**
 * @brief Escreve um byte em uma porta I/O
 */
static inline void write_io(z80_t *cpu, uint16_t port, uint8_t value) {
  if (cpu->config.write_io) {
    cpu->config.write_io(cpu->config.context, port, value);
  }
}

/**
 * @brief Executa uma instrução - implementação básica com instrução NOP
 * Nota: Esta implementação é apenas um placeholder. O código real
 * incluiria o processamento completo do conjunto de instruções Z80.
 */
static int execute_instruction(z80_t *cpu) {
  /* Se estiver em HALT, continua em HALT até que uma interrupção ocorra */
  if (cpu->halted) {
    return 4; /* HALT consome 4 ciclos por "instrução" */
  }

  /* Buscar opcode */
  uint8_t opcode = read_memory(cpu, cpu->pc++);

  /* Placeholder: Instrução NOP */
  /* Em uma implementação real, aqui teríamos o decode completo */
  return 4; /* NOP consome 4 ciclos */
}

/**
 * @brief Executa um número específico de ciclos
 */
int z80_execute(z80_t *cpu, int cycles) {
  if (!cpu) {
    return 0;
  }

  /* Resetar contador de ciclos */
  cpu->cycles = 0;

  /* Se cycles for 0, executar uma única instrução completa */
  if (cycles == 0) {
    return execute_instruction(cpu);
  }

  /* Executar instruções até atingir ou ultrapassar o número de ciclos */
  while (cpu->cycles < cycles) {
    int instruction_cycles = execute_instruction(cpu);
    cpu->cycles += instruction_cycles;
  }

  return cpu->cycles;
}

/**
 * @brief Executa uma instrução completa
 */
int z80_step(z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return execute_instruction(cpu);
}

/**
 * @brief Dispara uma interrupção maskable (INT)
 */
int z80_interrupt(z80_t *cpu, uint8_t data) {
  if (!cpu || !cpu->iff1) {
    return 0; /* Interrupções desabilitadas ou CPU inválida */
  }

  /* Desativar HALT se estiver ativo */
  cpu->halted = false;

  /* Desabilitar interrupções */
  cpu->iff1 = cpu->iff2 = false;

  /* Processamento da interrupção depende do modo */
  switch (cpu->im) {
  case Z80_INTERRUPT_MODE_0:
    /* Modo 0: Executar instrução fornecida pelo dispositivo (normalmente RST)
     */
    /* Placeholder - a implementação real dependeria do conjunto de instruções
     */
    return 12; /* Ciclos aproximados para RST */

  case Z80_INTERRUPT_MODE_1:
    /* Modo 1: Executar RST 38h */
    /* Push PC na pilha */
    cpu->sp -= 2;
    write_memory(cpu, cpu->sp, cpu->pc & 0xFF);
    write_memory(cpu, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);

    /* Saltar para 0x0038 */
    cpu->pc = 0x0038;
    return 13; /* Ciclos para IM 1 */

  case Z80_INTERRUPT_MODE_2:
    /* Modo 2: Buscar endereço de tabela I*256+data */
    {
      uint16_t vector_addr = (cpu->i << 8) | data;
      uint16_t jump_addr = read_memory(cpu, vector_addr) |
                           (read_memory(cpu, vector_addr + 1) << 8);

      /* Push PC na pilha */
      cpu->sp -= 2;
      write_memory(cpu, cpu->sp, cpu->pc & 0xFF);
      write_memory(cpu, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);

      /* Saltar para o endereço obtido */
      cpu->pc = jump_addr;
      return 19; /* Ciclos para IM 2 */
    }
  }

  return 0; /* Nunca deve chegar aqui */
}

/**
 * @brief Dispara uma interrupção não-maskable (NMI)
 */
int z80_nmi(z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  /* Desativar HALT se estiver ativo */
  cpu->halted = false;

  /* Backup de IFF1 para IFF2 e desabilitar IFF1 */
  cpu->iff2 = cpu->iff1;
  cpu->iff1 = false;

  /* Push PC na pilha */
  cpu->sp -= 2;
  write_memory(cpu, cpu->sp, cpu->pc & 0xFF);
  write_memory(cpu, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);

  /* Saltar para 0x0066 */
  cpu->pc = 0x0066;

  return 11; /* Ciclos para NMI */
}

/**
 * @brief Obtém o valor de um registrador
 */
uint8_t z80_get_register(const z80_t *cpu, z80_register_t reg) {
  if (!cpu) {
    return 0;
  }

  switch (reg) {
  case Z80_REG_A:
    return cpu->a;
  case Z80_REG_F:
    return cpu->f;
  case Z80_REG_B:
    return cpu->b;
  case Z80_REG_C:
    return cpu->c;
  case Z80_REG_D:
    return cpu->d;
  case Z80_REG_E:
    return cpu->e;
  case Z80_REG_H:
    return cpu->h;
  case Z80_REG_L:
    return cpu->l;
  case Z80_REG_A_PRIME:
    return cpu->a_prime;
  case Z80_REG_F_PRIME:
    return cpu->f_prime;
  case Z80_REG_B_PRIME:
    return cpu->b_prime;
  case Z80_REG_C_PRIME:
    return cpu->c_prime;
  case Z80_REG_D_PRIME:
    return cpu->d_prime;
  case Z80_REG_E_PRIME:
    return cpu->e_prime;
  case Z80_REG_H_PRIME:
    return cpu->h_prime;
  case Z80_REG_L_PRIME:
    return cpu->l_prime;
  case Z80_REG_IX_HIGH:
    return (cpu->ix >> 8) & 0xFF;
  case Z80_REG_IX_LOW:
    return cpu->ix & 0xFF;
  case Z80_REG_IY_HIGH:
    return (cpu->iy >> 8) & 0xFF;
  case Z80_REG_IY_LOW:
    return cpu->iy & 0xFF;
  case Z80_REG_SP_HIGH:
    return (cpu->sp >> 8) & 0xFF;
  case Z80_REG_SP_LOW:
    return cpu->sp & 0xFF;
  case Z80_REG_PC_HIGH:
    return (cpu->pc >> 8) & 0xFF;
  case Z80_REG_PC_LOW:
    return cpu->pc & 0xFF;
  case Z80_REG_I:
    return cpu->i;
  case Z80_REG_R:
    return cpu->r;
  default:
    return 0;
  }
}

/**
 * @brief Define o valor de um registrador
 */
void z80_set_register(z80_t *cpu, z80_register_t reg, uint8_t value) {
  if (!cpu) {
    return;
  }

  switch (reg) {
  case Z80_REG_A:
    cpu->a = value;
    break;
  case Z80_REG_F:
    cpu->f = value;
    break;
  case Z80_REG_B:
    cpu->b = value;
    break;
  case Z80_REG_C:
    cpu->c = value;
    break;
  case Z80_REG_D:
    cpu->d = value;
    break;
  case Z80_REG_E:
    cpu->e = value;
    break;
  case Z80_REG_H:
    cpu->h = value;
    break;
  case Z80_REG_L:
    cpu->l = value;
    break;
  case Z80_REG_A_PRIME:
    cpu->a_prime = value;
    break;
  case Z80_REG_F_PRIME:
    cpu->f_prime = value;
    break;
  case Z80_REG_B_PRIME:
    cpu->b_prime = value;
    break;
  case Z80_REG_C_PRIME:
    cpu->c_prime = value;
    break;
  case Z80_REG_D_PRIME:
    cpu->d_prime = value;
    break;
  case Z80_REG_E_PRIME:
    cpu->e_prime = value;
    break;
  case Z80_REG_H_PRIME:
    cpu->h_prime = value;
    break;
  case Z80_REG_L_PRIME:
    cpu->l_prime = value;
    break;
  case Z80_REG_IX_HIGH:
    cpu->ix = (cpu->ix & 0x00FF) | (value << 8);
    break;
  case Z80_REG_IX_LOW:
    cpu->ix = (cpu->ix & 0xFF00) | value;
    break;
  case Z80_REG_IY_HIGH:
    cpu->iy = (cpu->iy & 0x00FF) | (value << 8);
    break;
  case Z80_REG_IY_LOW:
    cpu->iy = (cpu->iy & 0xFF00) | value;
    break;
  case Z80_REG_SP_HIGH:
    cpu->sp = (cpu->sp & 0x00FF) | (value << 8);
    break;
  case Z80_REG_SP_LOW:
    cpu->sp = (cpu->sp & 0xFF00) | value;
    break;
  case Z80_REG_PC_HIGH:
    cpu->pc = (cpu->pc & 0x00FF) | (value << 8);
    break;
  case Z80_REG_PC_LOW:
    cpu->pc = (cpu->pc & 0xFF00) | value;
    break;
  case Z80_REG_I:
    cpu->i = value;
    break;
  case Z80_REG_R:
    cpu->r = value;
    break;
  default:
    break;
  }
}

/**
 * @brief Obtém o valor de um registrador de 16 bits
 */
uint16_t z80_get_register_pair(const z80_t *cpu, z80_register_t reg_high,
                               z80_register_t reg_low) {
  if (!cpu) {
    return 0;
  }

  return (z80_get_register(cpu, reg_high) << 8) |
         z80_get_register(cpu, reg_low);
}

/**
 * @brief Define o valor de um registrador de 16 bits
 */
void z80_set_register_pair(z80_t *cpu, z80_register_t reg_high,
                           z80_register_t reg_low, uint16_t value) {
  if (!cpu) {
    return;
  }

  z80_set_register(cpu, reg_high, (value >> 8) & 0xFF);
  z80_set_register(cpu, reg_low, value & 0xFF);
}

/**
 * @brief Obtém o valor do registrador AF
 */
uint16_t z80_get_af(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return (cpu->a << 8) | cpu->f;
}

/**
 * @brief Define o valor do registrador AF
 */
void z80_set_af(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->a = (value >> 8) & 0xFF;
  cpu->f = value & 0xFF;
}

/**
 * @brief Obtém o valor do registrador BC
 */
uint16_t z80_get_bc(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return (cpu->b << 8) | cpu->c;
}

/**
 * @brief Define o valor do registrador BC
 */
void z80_set_bc(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->b = (value >> 8) & 0xFF;
  cpu->c = value & 0xFF;
}

/**
 * @brief Obtém o valor do registrador DE
 */
uint16_t z80_get_de(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return (cpu->d << 8) | cpu->e;
}

/**
 * @brief Define o valor do registrador DE
 */
void z80_set_de(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->d = (value >> 8) & 0xFF;
  cpu->e = value & 0xFF;
}

/**
 * @brief Obtém o valor do registrador HL
 */
uint16_t z80_get_hl(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return (cpu->h << 8) | cpu->l;
}

/**
 * @brief Define o valor do registrador HL
 */
void z80_set_hl(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->h = (value >> 8) & 0xFF;
  cpu->l = value & 0xFF;
}

/**
 * @brief Obtém o valor do registrador IX
 */
uint16_t z80_get_ix(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return cpu->ix;
}

/**
 * @brief Define o valor do registrador IX
 */
void z80_set_ix(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->ix = value;
}

/**
 * @brief Obtém o valor do registrador IY
 */
uint16_t z80_get_iy(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return cpu->iy;
}

/**
 * @brief Define o valor do registrador IY
 */
void z80_set_iy(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->iy = value;
}

/**
 * @brief Obtém o valor do registrador SP
 */
uint16_t z80_get_sp(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return cpu->sp;
}

/**
 * @brief Define o valor do registrador SP
 */
void z80_set_sp(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->sp = value;
}

/**
 * @brief Obtém o valor do registrador PC
 */
uint16_t z80_get_pc(const z80_t *cpu) {
  if (!cpu) {
    return 0;
  }

  return cpu->pc;
}

/**
 * @brief Define o valor do registrador PC
 */
void z80_set_pc(z80_t *cpu, uint16_t value) {
  if (!cpu) {
    return;
  }

  cpu->pc = value;
}

/**
 * @brief Verifica se um flag específico está ativo
 */
bool z80_check_flag(const z80_t *cpu, z80_flag_t flag) {
  if (!cpu) {
    return false;
  }

  return (cpu->f & flag) != 0;
}

/**
 * @brief Define o estado de um flag
 */
void z80_set_flag(z80_t *cpu, z80_flag_t flag, bool state) {
  if (!cpu) {
    return;
  }

  if (state) {
    cpu->f |= flag;
  } else {
    cpu->f &= ~flag;
  }
}

/**
 * @brief Obtém o modo de interrupção atual
 */
z80_interrupt_mode_t z80_get_interrupt_mode(const z80_t *cpu) {
  if (!cpu) {
    return Z80_INTERRUPT_MODE_0;
  }

  return cpu->im;
}

/**
 * @brief Define o modo de interrupção
 */
void z80_set_interrupt_mode(z80_t *cpu, z80_interrupt_mode_t mode) {
  if (!cpu) {
    return;
  }

  cpu->im = mode;
}

/**
 * @brief Verifica se interrupções estão habilitadas
 */
bool z80_interrupts_enabled(const z80_t *cpu) {
  if (!cpu) {
    return false;
  }

  return cpu->iff1;
}

/**
 * @brief Define o estado de habilitação de interrupções
 */
void z80_set_interrupts_enabled(z80_t *cpu, bool enabled) {
  if (!cpu) {
    return;
  }

  cpu->iff1 = cpu->iff2 = enabled;
}

/**
 * @brief Salva o estado da CPU em um buffer
 */
int z80_save_state(const z80_t *cpu, uint8_t *buffer, size_t buffer_size) {
  if (!cpu || !buffer || buffer_size < Z80_STATE_SIZE) {
    return -1;
  }

  uint8_t *ptr = buffer;

  /* Salvar registradores */
  *ptr++ = cpu->a;
  *ptr++ = cpu->f;
  *ptr++ = cpu->b;
  *ptr++ = cpu->c;
  *ptr++ = cpu->d;
  *ptr++ = cpu->e;
  *ptr++ = cpu->h;
  *ptr++ = cpu->l;

  *ptr++ = cpu->a_prime;
  *ptr++ = cpu->f_prime;
  *ptr++ = cpu->b_prime;
  *ptr++ = cpu->c_prime;
  *ptr++ = cpu->d_prime;
  *ptr++ = cpu->e_prime;
  *ptr++ = cpu->h_prime;
  *ptr++ = cpu->l_prime;

  *ptr++ = cpu->i;
  *ptr++ = cpu->r;

  /* Salvar registradores de 16 bits */
  *ptr++ = cpu->ix & 0xFF;
  *ptr++ = (cpu->ix >> 8) & 0xFF;
  *ptr++ = cpu->iy & 0xFF;
  *ptr++ = (cpu->iy >> 8) & 0xFF;
  *ptr++ = cpu->sp & 0xFF;
  *ptr++ = (cpu->sp >> 8) & 0xFF;
  *ptr++ = cpu->pc & 0xFF;
  *ptr++ = (cpu->pc >> 8) & 0xFF;

  /* Salvar estado de interrupção */
  *ptr++ = cpu->iff1 ? 1 : 0;
  *ptr++ = cpu->iff2 ? 1 : 0;
  *ptr++ = (uint8_t)cpu->im;

  /* Salvar estado de execução */
  *ptr++ = cpu->halted ? 1 : 0;

  /* Preencher o resto com zeros */
  while (ptr < buffer + Z80_STATE_SIZE) {
    *ptr++ = 0;
  }

  return Z80_STATE_SIZE;
}

/**
 * @brief Carrega o estado da CPU de um buffer
 */
int z80_load_state(z80_t *cpu, const uint8_t *buffer, size_t buffer_size) {
  if (!cpu || !buffer || buffer_size < Z80_STATE_SIZE) {
    return -1;
  }

  const uint8_t *ptr = buffer;

  /* Carregar registradores */
  cpu->a = *ptr++;
  cpu->f = *ptr++;
  cpu->b = *ptr++;
  cpu->c = *ptr++;
  cpu->d = *ptr++;
  cpu->e = *ptr++;
  cpu->h = *ptr++;
  cpu->l = *ptr++;

  cpu->a_prime = *ptr++;
  cpu->f_prime = *ptr++;
  cpu->b_prime = *ptr++;
  cpu->c_prime = *ptr++;
  cpu->d_prime = *ptr++;
  cpu->e_prime = *ptr++;
  cpu->h_prime = *ptr++;
  cpu->l_prime = *ptr++;

  cpu->i = *ptr++;
  cpu->r = *ptr++;

  /* Carregar registradores de 16 bits */
  cpu->ix = *ptr++;
  cpu->ix |= (*ptr++) << 8;
  cpu->iy = *ptr++;
  cpu->iy |= (*ptr++) << 8;
  cpu->sp = *ptr++;
  cpu->sp |= (*ptr++) << 8;
  cpu->pc = *ptr++;
  cpu->pc |= (*ptr++) << 8;

  /* Carregar estado de interrupção */
  cpu->iff1 = *ptr++ != 0;
  cpu->iff2 = *ptr++ != 0;
  cpu->im = (z80_interrupt_mode_t)*ptr++;

  /* Carregar estado de execução */
  cpu->halted = *ptr++ != 0;

  return 0;
}
