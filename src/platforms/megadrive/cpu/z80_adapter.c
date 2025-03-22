/**
 * @file z80_adapter.c
 * @brief Implementação do adaptador do Z80 para o Mega Drive
 */

#include "z80_adapter.h"
#include "../../../core/cpu/z80/z80.h"
#include "../../../core/cpu/z80/z80_instructions.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../../../core/save_state.h"
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o Z80 Mega Drive
#define EMU_LOG_CAT_MD_Z80 EMU_LOG_CAT_MEGADRIVE

// Macros de log específicas para o Z80 Mega Drive
#define MD_Z80_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_MD_Z80, __VA_ARGS__)
#define MD_Z80_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_MD_Z80, __VA_ARGS__)
#define MD_Z80_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_MD_Z80, __VA_ARGS__)
#define MD_Z80_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_MD_Z80, __VA_ARGS__)
#define MD_Z80_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_MD_Z80, __VA_ARGS__)

// Endereços do espaço Z80 no Mega Drive
#define MD_Z80_RAM_START  0x0000
#define MD_Z80_RAM_END    0x1FFF
#define MD_Z80_RAM_SIZE   0x2000  // 8KB
#define MD_Z80_YM2612     0x4000  // Registradores do YM2612
#define MD_Z80_BANK_REG   0x6000  // Registrador de banco
#define MD_Z80_PSG        0x7F11  // Porta do PSG
#define MD_Z80_BANK_START 0x8000  // Início do banco de ROM

/**
 * @brief Estrutura interna do adaptador Z80 para o Mega Drive
 */
struct md_z80_adapter_s {
    z80_t *cpu;                // Instância base do Z80
    md_memory_t *memory;       // Sistema de memória do Mega Drive
    md_audio_t *audio;         // Sistema de áudio do Mega Drive
    uint8_t *ram;              // RAM do Z80 (8KB)
    uint16_t bank_register;    // Registrador de banco
    bool reset_state;          // Estado de reset
    bool busreq_state;         // Estado de bus request
};

// Forward declarations de funções de callback
static uint8_t md_z80_read_callback(void *context, uint16_t address);
static void md_z80_write_callback(void *context, uint16_t address, uint8_t value);
static uint8_t md_z80_io_read_callback(void *context, uint8_t port);
static void md_z80_io_write_callback(void *context, uint8_t port, uint8_t value);

/**
 * @brief Cria uma nova instância do adaptador Z80 para o Mega Drive
 */
md_z80_adapter_t* md_z80_adapter_create(void) {
    md_z80_adapter_t *adapter = (md_z80_adapter_t*)malloc(sizeof(md_z80_adapter_t));
    if (!adapter) {
        MD_Z80_LOG_ERROR("Falha ao alocar memória para o adaptador Z80");
        return NULL;
    }

    // Inicializa a estrutura
    memset(adapter, 0, sizeof(md_z80_adapter_t));

    // Cria instância base do Z80
    adapter->cpu = z80_create();
    if (!adapter->cpu) {
        MD_Z80_LOG_ERROR("Falha ao criar instância base do Z80");
        free(adapter);
        return NULL;
    }

    // Aloca memória para a RAM do Z80
    adapter->ram = (uint8_t*)malloc(MD_Z80_RAM_SIZE);
    if (!adapter->ram) {
        MD_Z80_LOG_ERROR("Falha ao alocar memória para a RAM do Z80");
        z80_destroy(adapter->cpu);
        free(adapter);
        return NULL;
    }

    // Inicializa a RAM do Z80
    memset(adapter->ram, 0, MD_Z80_RAM_SIZE);

    // Inicializa registrador de banco
    adapter->bank_register = 0;

    // Define estados iniciais
    adapter->reset_state = true;   // Z80 começa em reset
    adapter->busreq_state = true;  // Barramento começa solicitado pelo 68000

    // Configura callbacks para memória e I/O
    z80_set_memory_callbacks(adapter->cpu, md_z80_read_callback, md_z80_write_callback, adapter);
    z80_set_io_callbacks(adapter->cpu, md_z80_io_read_callback, md_z80_io_write_callback, adapter);

    // Inicializa tabelas de instruções
    if (!z80_instructions_init()) {
        MD_Z80_LOG_ERROR("Falha ao inicializar instruções do Z80");
        free(adapter->ram);
        z80_destroy(adapter->cpu);
        free(adapter);
        return NULL;
    }

    MD_Z80_LOG_INFO("Adaptador Z80 para Mega Drive criado com sucesso");
    return adapter;
}

/**
 * @brief Destrói uma instância do adaptador Z80 e libera recursos
 */
void md_z80_adapter_destroy(md_z80_adapter_t *z80) {
    if (!z80) {
        return;
    }

    // Libera recursos
    if (z80->cpu) {
        z80_destroy(z80->cpu);
    }

    if (z80->ram) {
        free(z80->ram);
    }

    free(z80);
    MD_Z80_LOG_INFO("Adaptador Z80 para Mega Drive destruído");
}

/**
 * @brief Reseta o Z80 para o estado inicial
 */
void md_z80_adapter_reset(md_z80_adapter_t *z80) {
    if (!z80 || !z80->cpu) {
        return;
    }

    // Reseta o Z80 base
    z80_reset(z80->cpu);

    // Limpa a RAM
    if (z80->ram) {
        memset(z80->ram, 0, MD_Z80_RAM_SIZE);
    }

    // Reseta registrador de banco
    z80->bank_register = 0;

    // Define estados iniciais
    z80->reset_state = true;
    z80->busreq_state = true;

    MD_Z80_LOG_INFO("Adaptador Z80 para Mega Drive resetado");
}

/**
 * @brief Conecta o sistema de memória ao Z80
 */
bool md_z80_adapter_connect_memory(md_z80_adapter_t *z80, md_memory_t *memory) {
    if (!z80) {
        return false;
    }

    z80->memory = memory;
    MD_Z80_LOG_DEBUG("Sistema de memória conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Conecta o sistema de áudio ao Z80
 */
bool md_z80_adapter_connect_audio(md_z80_adapter_t *z80, md_audio_t *audio) {
    if (!z80) {
        return false;
    }

    z80->audio = audio;
    MD_Z80_LOG_DEBUG("Sistema de áudio conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Executa um passo de instrução no Z80
 */
uint8_t md_z80_adapter_step(md_z80_adapter_t *z80) {
    if (!z80 || !z80->cpu) {
        return 0;
    }

    // Se o Z80 estiver em reset ou busreq, não executa
    if (z80->reset_state || z80->busreq_state) {
        return 0;
    }

    // Executa um passo no Z80 base
    return z80_step(z80->cpu);
}

/**
 * @brief Executa um número específico de ciclos no Z80
 */
uint32_t md_z80_adapter_run(md_z80_adapter_t *z80, uint32_t cycles) {
    if (!z80 || !z80->cpu) {
        return 0;
    }

    // Se o Z80 estiver em reset ou busreq, não executa
    if (z80->reset_state || z80->busreq_state) {
        return 0;
    }

    // Executa ciclos no Z80 base
    return z80_execute_cycles(z80->cpu, cycles);
}

/**
 * @brief Gera uma interrupção no Z80
 */
void md_z80_adapter_interrupt(md_z80_adapter_t *z80) {
    if (!z80 || !z80->cpu) {
        return;
    }

    // Se o Z80 estiver em reset ou busreq, não processa a interrupção
    if (z80->reset_state || z80->busreq_state) {
        return;
    }

    // Gera interrupção no Z80 base
    z80_interrupt(z80->cpu, false);
}

/**
 * @brief Define o estado de reset do Z80
 */
void md_z80_adapter_set_reset(md_z80_adapter_t *z80, bool reset) {
    if (!z80) {
        return;
    }

    // Se o estado mudou
    if (z80->reset_state != reset) {
        z80->reset_state = reset;

        // Se saiu do reset, reseta o Z80
        if (!reset) {
            // Apenas reseta se não estiver em busreq
            if (!z80->busreq_state && z80->cpu) {
                z80_reset(z80->cpu);
            }
        }

        MD_Z80_LOG_DEBUG("Z80 %s", reset ? "resetado" : "liberado do reset");
    }
}

/**
 * @brief Define o estado de bus request do Z80
 */
void md_z80_adapter_set_busreq(md_z80_adapter_t *z80, bool request) {
    if (!z80) {
        return;
    }

    // Se o estado mudou
    if (z80->busreq_state != request) {
        z80->busreq_state = request;
        MD_Z80_LOG_DEBUG("Barramento do Z80 %s", request ? "solicitado" : "liberado");
    }
}

/**
 * @brief Obtém o estado atual de bus request do Z80
 */
bool md_z80_adapter_get_busreq(md_z80_adapter_t *z80) {
    if (!z80) {
        return true; // Valor padrão seguro
    }

    return z80->busreq_state;
}

/**
 * @brief Define o banco de memória para o Z80 acessar a memória principal
 */
void md_z80_adapter_set_bank(md_z80_adapter_t *z80, uint16_t bank) {
    if (!z80) {
        return;
    }

    z80->bank_register = bank & 0x1FF; // Limita a 9 bits (0-511)
    MD_Z80_LOG_DEBUG("Banco de memória do Z80 definido para 0x%04X", z80->bank_register);
}

/**
 * @brief Registra o Z80 no sistema de save state
 */
int md_z80_adapter_register_save_state(md_z80_adapter_t *z80, save_state_t *state) {
    if (!z80 || !state) {
        return -1;
    }

    // Registra o Z80 base
    if (z80->cpu) {
        z80_register_save_state(z80->cpu, state);
    }

    // Registra a RAM do Z80
    if (z80->ram) {
        save_state_register_field(state, "md_z80_ram", z80->ram, MD_Z80_RAM_SIZE);
    }

    // Registra outros campos
    save_state_register_field(state, "md_z80_bank_register", &z80->bank_register, sizeof(uint16_t));
    save_state_register_field(state, "md_z80_reset_state", &z80->reset_state, sizeof(bool));
    save_state_register_field(state, "md_z80_busreq_state", &z80->busreq_state, sizeof(bool));

    MD_Z80_LOG_DEBUG("Adaptador Z80 registrado no sistema de save state");
    return 0;
}

// Implementações das funções de callback

/**
 * @brief Callback para leitura de memória do Z80
 */
static uint8_t md_z80_read_callback(void *context, uint16_t address) {
    md_z80_adapter_t *adapter = (md_z80_adapter_t*)context;

    if (!adapter) {
        return 0xFF;
    }

    // Verifica se o endereço está na RAM do Z80
    if (address <= MD_Z80_RAM_END) {
        return adapter->ram[address];
    }

    // Registradores do YM2612
    if (address >= 0x4000 && address <= 0x4003) {
        if (adapter->audio) {
            // Implementar leitura dos registradores do YM2612
            // Esta é uma implementação básica, pode precisar de ajustes
            return md_audio_read_ym2612(adapter->audio, address & 0x03);
        }
        return 0xFF;
    }

    // Registrador de banco
    if (address == MD_Z80_BANK_REG || address == MD_Z80_BANK_REG + 1) {
        // Normalmente não é lido, mas retorna o último valor escrito
        return (address == MD_Z80_BANK_REG) ?
               (adapter->bank_register & 0xFF) :
               ((adapter->bank_register >> 8) & 0xFF);
    }

    // Porta do PSG
    if (address == MD_Z80_PSG) {
        // PSG tipicamente não é lido
        return 0xFF;
    }

    // Acesso ao banco mapeado da ROM
    if (address >= MD_Z80_BANK_START) {
        // Calcula o endereço na memória principal
        uint32_t main_address = (adapter->bank_register << 15) + (address - MD_Z80_BANK_START);

        // Lê da memória principal
        if (adapter->memory) {
            return md_memory_read_8(adapter->memory, main_address);
        }
    }

    return 0xFF; // Valor padrão para endereços inválidos
}

/**
 * @brief Callback para escrita em memória do Z80
 */
static void md_z80_write_callback(void *context, uint16_t address, uint8_t value) {
    md_z80_adapter_t *adapter = (md_z80_adapter_t*)context;

    if (!adapter) {
        return;
    }

    // Verifica se o endereço está na RAM do Z80
    if (address <= MD_Z80_RAM_END) {
        adapter->ram[address] = value;
        return;
    }

    // Registradores do YM2612
    if (address >= 0x4000 && address <= 0x4003) {
        if (adapter->audio) {
            // Implementar escrita nos registradores do YM2612
            md_audio_write_ym2612(adapter->audio, address & 0x03, value);
        }
        return;
    }

    // Registrador de banco
    if (address == MD_Z80_BANK_REG) {
        // Define o byte baixo do registrador de banco
        adapter->bank_register = (adapter->bank_register & 0xFF00) | value;
        return;
    } else if (address == MD_Z80_BANK_REG + 1) {
        // Define o byte alto do registrador de banco (normalmente apenas o primeiro bit é usado)
        adapter->bank_register = (adapter->bank_register & 0x00FF) | ((value & 0x01) << 8);
        return;
    }

    // Porta do PSG
    if (address == MD_Z80_PSG) {
        if (adapter->audio) {
            // Implementar escrita no PSG
            md_audio_write_psg(adapter->audio, value);
        }
        return;
    }

    // Acesso ao banco mapeado da ROM (escrita tipicamente não tem efeito)
    if (address >= MD_Z80_BANK_START) {
        // Ignora escritas na ROM
        return;
    }
}

/**
 * @brief Callback para leitura de porta I/O do Z80
 */
static uint8_t md_z80_io_read_callback(void *context, uint8_t port) {
    // O Mega Drive não usa as portas de I/O do Z80 de forma tradicional
    // Em vez disso, usa mapeamento de memória para acessar dispositivos
    return 0xFF;
}

/**
 * @brief Callback para escrita em porta I/O do Z80
 */
static void md_z80_io_write_callback(void *context, uint8_t port, uint8_t value) {
    // O Mega Drive não usa as portas de I/O do Z80 de forma tradicional
    // Em vez disso, usa mapeamento de memória para acessar dispositivos
}
