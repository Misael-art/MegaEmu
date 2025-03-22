/**
 * @file z80_adapter.c
 * @brief Implementação do adaptador do Z80 para o Master System
 */

#include "z80_adapter.h"
#include "../../../core/cpu/z80/z80.h"
#include "../../../core/cpu/z80/z80_instructions.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../../../core/save_state.h"
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o Z80 Master System
#define EMU_LOG_CAT_SMS_Z80 EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para o Z80 Master System
#define SMS_Z80_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_SMS_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_SMS_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_SMS_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_SMS_Z80, __VA_ARGS__)
#define SMS_Z80_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_SMS_Z80, __VA_ARGS__)

// Portas de I/O do Master System
#define SMS_PORT_VDP_DATA      0xBE  // Porta de dados do VDP
#define SMS_PORT_VDP_CONTROL   0xBF  // Porta de controle do VDP
#define SMS_PORT_PSG           0x7F  // Porta do PSG

/**
 * @brief Estrutura interna do adaptador Z80 para o Master System
 */
struct sms_z80_adapter_s {
    z80_t *cpu;            // Instância base do Z80
    sms_memory_t *memory;  // Sistema de memória do Master System
    void *vdp;             // VDP (Video Display Processor)
    void *psg;             // PSG (Programmable Sound Generator)
    uint8_t vdp_status;    // Status do VDP
    uint8_t vdp_latch;     // Latch de endereço do VDP
    bool vdp_second_byte;  // Flag para o segundo byte do comando VDP
    bool nmi_pending;      // Flag para NMI pendente
    bool irq_pending;      // Flag para IRQ pendente
};

// Forward declarations de funções de callback
static uint8_t sms_z80_read_callback(void *context, uint16_t address);
static void sms_z80_write_callback(void *context, uint16_t address, uint8_t value);
static uint8_t sms_z80_io_read_callback(void *context, uint8_t port);
static void sms_z80_io_write_callback(void *context, uint8_t port, uint8_t value);

// Forward declarations de funções do VDP
static uint8_t sms_vdp_read_data(void *vdp);
static uint8_t sms_vdp_read_status(void *vdp);
static void sms_vdp_write_data(void *vdp, uint8_t value);
static void sms_vdp_write_control(void *vdp, uint8_t value);

// Forward declarations de funções do PSG
static void sms_psg_write(void *psg, uint8_t value);

/**
 * @brief Cria uma nova instância do adaptador Z80 para o Master System
 */
sms_z80_adapter_t* sms_z80_adapter_create(void) {
    sms_z80_adapter_t *adapter = (sms_z80_adapter_t*)malloc(sizeof(sms_z80_adapter_t));
    if (!adapter) {
        SMS_Z80_LOG_ERROR("Falha ao alocar memória para o adaptador Z80");
        return NULL;
    }

    // Inicializa a estrutura
    memset(adapter, 0, sizeof(sms_z80_adapter_t));

    // Cria instância base do Z80
    adapter->cpu = z80_create();
    if (!adapter->cpu) {
        SMS_Z80_LOG_ERROR("Falha ao criar instância base do Z80");
        free(adapter);
        return NULL;
    }

    // Configura callbacks para memória e I/O
    z80_set_memory_callbacks(adapter->cpu, sms_z80_read_callback, sms_z80_write_callback, adapter);
    z80_set_io_callbacks(adapter->cpu, sms_z80_io_read_callback, sms_z80_io_write_callback, adapter);

    // Inicializa tabelas de instruções
    if (!z80_instructions_init()) {
        SMS_Z80_LOG_ERROR("Falha ao inicializar instruções do Z80");
        z80_destroy(adapter->cpu);
        free(adapter);
        return NULL;
    }

    // Define modo de interrupção 1 (padrão do Master System)
    z80_set_interrupt_mode(adapter->cpu, 1);

    SMS_Z80_LOG_INFO("Adaptador Z80 para Master System criado com sucesso");
    return adapter;
}

/**
 * @brief Destrói uma instância do adaptador Z80 e libera recursos
 */
void sms_z80_adapter_destroy(sms_z80_adapter_t *z80) {
    if (!z80) {
        return;
    }

    // Libera recursos
    if (z80->cpu) {
        z80_destroy(z80->cpu);
    }

    free(z80);
    SMS_Z80_LOG_INFO("Adaptador Z80 para Master System destruído");
}

/**
 * @brief Reseta o Z80 para o estado inicial
 */
void sms_z80_adapter_reset(sms_z80_adapter_t *z80) {
    if (!z80 || !z80->cpu) {
        return;
    }

    // Reseta o Z80 base
    z80_reset(z80->cpu);

    // Reseta estados internos
    z80->vdp_status = 0;
    z80->vdp_latch = 0;
    z80->vdp_second_byte = false;
    z80->nmi_pending = false;
    z80->irq_pending = false;

    // Define modo de interrupção 1 (padrão do Master System)
    z80_set_interrupt_mode(z80->cpu, 1);

    SMS_Z80_LOG_INFO("Adaptador Z80 para Master System resetado");
}

/**
 * @brief Conecta o sistema de memória ao Z80
 */
bool sms_z80_adapter_connect_memory(sms_z80_adapter_t *z80, sms_memory_t *memory) {
    if (!z80) {
        return false;
    }

    z80->memory = memory;
    SMS_Z80_LOG_DEBUG("Sistema de memória conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Conecta o sistema de vídeo ao Z80
 */
bool sms_z80_adapter_connect_vdp(sms_z80_adapter_t *z80, void *vdp) {
    if (!z80) {
        return false;
    }

    z80->vdp = vdp;
    SMS_Z80_LOG_DEBUG("VDP conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Conecta o sistema de áudio ao Z80
 */
bool sms_z80_adapter_connect_psg(sms_z80_adapter_t *z80, void *psg) {
    if (!z80) {
        return false;
    }

    z80->psg = psg;
    SMS_Z80_LOG_DEBUG("PSG conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Executa um passo de instrução no Z80
 */
uint8_t sms_z80_adapter_step(sms_z80_adapter_t *z80) {
    if (!z80 || !z80->cpu) {
        return 0;
    }

    // Verifica interrupções pendentes
    if (z80->nmi_pending) {
        z80->nmi_pending = false;
        z80_interrupt(z80->cpu, true); // NMI
    } else if (z80->irq_pending) {
        // Apenas gera IRQ se interrupções estiverem habilitadas
        bool interrupts_enabled = z80_get_register(z80->cpu, Z80_REG_IFF1) != 0;
        if (interrupts_enabled) {
            z80->irq_pending = false;
            z80_interrupt(z80->cpu, false); // IRQ
        }
    }

    // Executa um passo no Z80 base
    return z80_step(z80->cpu);
}

/**
 * @brief Executa um número específico de ciclos no Z80
 */
uint32_t sms_z80_adapter_run(sms_z80_adapter_t *z80, uint32_t cycles) {
    if (!z80 || !z80->cpu) {
        return 0;
    }

    uint32_t executed_cycles = 0;

    // Executa ciclos no Z80 base
    while (executed_cycles < cycles) {
        executed_cycles += sms_z80_adapter_step(z80);
    }

    return executed_cycles;
}

/**
 * @brief Gera uma interrupção no Z80
 */
void sms_z80_adapter_interrupt(sms_z80_adapter_t *z80) {
    if (!z80) {
        return;
    }

    // Marca a interrupção como pendente (será processada no próximo passo)
    z80->irq_pending = true;
}

/**
 * @brief Obtém o valor do registrador PC atual
 */
uint16_t sms_z80_adapter_get_pc(sms_z80_adapter_t *z80) {
    if (!z80 || !z80->cpu) {
        return 0;
    }

    return z80_get_register(z80->cpu, Z80_REG_PC);
}

/**
 * @brief Registra o Z80 no sistema de save state
 */
int sms_z80_adapter_register_save_state(sms_z80_adapter_t *z80, save_state_t *state) {
    if (!z80 || !state) {
        return -1;
    }

    // Registra o Z80 base
    if (z80->cpu) {
        z80_register_save_state(z80->cpu, state);
    }

    // Registra estados internos
    save_state_register_field(state, "sms_z80_vdp_status", &z80->vdp_status, sizeof(uint8_t));
    save_state_register_field(state, "sms_z80_vdp_latch", &z80->vdp_latch, sizeof(uint8_t));
    save_state_register_field(state, "sms_z80_vdp_second_byte", &z80->vdp_second_byte, sizeof(bool));
    save_state_register_field(state, "sms_z80_nmi_pending", &z80->nmi_pending, sizeof(bool));
    save_state_register_field(state, "sms_z80_irq_pending", &z80->irq_pending, sizeof(bool));

    SMS_Z80_LOG_DEBUG("Adaptador Z80 registrado no sistema de save state");
    return 0;
}

/**
 * @brief Atualiza o Z80 após um carregamento de save state
 */
void sms_z80_adapter_update_after_state_load(sms_z80_adapter_t *z80) {
    if (!z80 || !z80->cpu) {
        return;
    }

    // Atualiza o Z80 base
    z80_update_after_state_load(z80->cpu);

    SMS_Z80_LOG_DEBUG("Adaptador Z80 atualizado após carregamento de estado");
}

// Implementações das funções de callback

/**
 * @brief Callback para leitura de memória do Z80
 */
static uint8_t sms_z80_read_callback(void *context, uint16_t address) {
    sms_z80_adapter_t *adapter = (sms_z80_adapter_t*)context;

    if (!adapter) {
        return 0xFF;
    }

    // Verifica se o sistema de memória está conectado
    if (adapter->memory) {
        return sms_memory_read_8(adapter->memory, address);
    }

    return 0xFF; // Valor padrão para endereços inválidos
}

/**
 * @brief Callback para escrita em memória do Z80
 */
static void sms_z80_write_callback(void *context, uint16_t address, uint8_t value) {
    sms_z80_adapter_t *adapter = (sms_z80_adapter_t*)context;

    if (!adapter) {
        return;
    }

    // Verifica se o sistema de memória está conectado
    if (adapter->memory) {
        sms_memory_write_8(adapter->memory, address, value);
    }
}

/**
 * @brief Callback para leitura de porta I/O do Z80
 */
static uint8_t sms_z80_io_read_callback(void *context, uint8_t port) {
    sms_z80_adapter_t *adapter = (sms_z80_adapter_t*)context;

    if (!adapter) {
        return 0xFF;
    }

    // Mascara o endereço para 8 bits (Master System ignora bits superiores)
    port &= 0xFF;

    // Verifica a porta acessada
    switch (port) {
        case SMS_PORT_VDP_DATA:
            // Leitura de dados do VDP
            if (adapter->vdp) {
                return sms_vdp_read_data(adapter->vdp);
            }
            break;

        case SMS_PORT_VDP_CONTROL:
            // Leitura de status do VDP
            if (adapter->vdp) {
                // Lê status e reseta flags
                uint8_t status = sms_vdp_read_status(adapter->vdp);
                adapter->vdp_second_byte = false;
                return status;
            }
            break;

        // Outras portas de I/O específicas do Master System
        // (portas de controle, cartuchos, expansões, etc.)

        default:
            // SMS_Z80_LOG_TRACE("Leitura de porta I/O não implementada: 0x%02X", port);
            break;
    }

    return 0xFF; // Valor padrão para portas não implementadas
}

/**
 * @brief Callback para escrita em porta I/O do Z80
 */
static void sms_z80_io_write_callback(void *context, uint8_t port, uint8_t value) {
    sms_z80_adapter_t *adapter = (sms_z80_adapter_t*)context;

    if (!adapter) {
        return;
    }

    // Mascara o endereço para 8 bits (Master System ignora bits superiores)
    port &= 0xFF;

    // Verifica a porta acessada
    switch (port) {
        case SMS_PORT_VDP_DATA:
            // Escrita de dados no VDP
            if (adapter->vdp) {
                sms_vdp_write_data(adapter->vdp, value);
                adapter->vdp_second_byte = false;
            }
            break;

        case SMS_PORT_VDP_CONTROL:
            // Escrita de controle no VDP
            if (adapter->vdp) {
                if (!adapter->vdp_second_byte) {
                    // Primeiro byte (normalmente dados)
                    adapter->vdp_latch = value;
                    adapter->vdp_second_byte = true;
                } else {
                    // Segundo byte (normalmente comando)
                    sms_vdp_write_control(adapter->vdp, (value << 8) | adapter->vdp_latch);
                    adapter->vdp_second_byte = false;
                }
            }
            break;

        case SMS_PORT_PSG:
            // Escrita no PSG
            if (adapter->psg) {
                sms_psg_write(adapter->psg, value);
            }
            break;

        // Outras portas de I/O específicas do Master System
        // (portas de controle, cartuchos, expansões, etc.)

        default:
            // SMS_Z80_LOG_TRACE("Escrita em porta I/O não implementada: 0x%02X = 0x%02X", port, value);
            break;
    }
}

// Implementações simplificadas das funções do VDP e PSG
// Estas funções seriam substituídas por chamadas reais para os componentes correspondentes

static uint8_t sms_vdp_read_data(void *vdp) {
    // Implementação temporária
    return 0xFF;
}

static uint8_t sms_vdp_read_status(void *vdp) {
    // Implementação temporária
    return 0x00;
}

static void sms_vdp_write_data(void *vdp, uint8_t value) {
    // Implementação temporária
}

static void sms_vdp_write_control(void *vdp, uint8_t value) {
    // Implementação temporária
}

static void sms_psg_write(void *psg, uint8_t value) {
    // Implementação temporária
}
