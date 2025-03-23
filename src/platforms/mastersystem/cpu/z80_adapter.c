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
#define SMS_PORT_VDP_DATA 0xBE    // Porta de dados do VDP
#define SMS_PORT_VDP_CONTROL 0xBF // Porta de controle do VDP
#define SMS_PORT_PSG 0x7F         // Porta do PSG

/**
 * @brief Estrutura interna do adaptador Z80 para o Master System
 */
struct sms_z80_adapter_s
{
    z80_t *cpu;           // Instância base do Z80
    sms_memory_t *memory; // Sistema de memória do Master System
    void *vdp;            // VDP (Video Display Processor)
    void *psg;            // PSG (Programmable Sound Generator)
    uint8_t vdp_status;   // Status do VDP
    uint8_t vdp_latch;    // Latch de endereço do VDP
    bool vdp_second_byte; // Flag para o segundo byte do comando VDP
    bool nmi_pending;     // Flag para NMI pendente
    bool irq_pending;     // Flag para IRQ pendente
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
sms_z80_adapter_t *sms_z80_adapter_create(void)
{
    sms_z80_adapter_t *adapter = (sms_z80_adapter_t *)malloc(sizeof(sms_z80_adapter_t));
    if (!adapter)
    {
        SMS_Z80_LOG_ERROR("Falha ao alocar memória para o adaptador Z80");
        return NULL;
    }

    // Inicializa a estrutura
    memset(adapter, 0, sizeof(sms_z80_adapter_t));

    // Cria instância base do Z80
    adapter->cpu = z80_create();
    if (!adapter->cpu)
    {
        SMS_Z80_LOG_ERROR("Falha ao criar instância base do Z80");
        free(adapter);
        return NULL;
    }

    // Configura callbacks para memória e I/O
    z80_set_memory_callbacks(adapter->cpu, sms_z80_read_callback, sms_z80_write_callback, adapter);
    z80_set_io_callbacks(adapter->cpu, sms_z80_io_read_callback, sms_z80_io_write_callback, adapter);

    // Inicializa tabelas de instruções
    if (!z80_instructions_init())
    {
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
void sms_z80_adapter_destroy(sms_z80_adapter_t *z80)
{
    if (!z80)
    {
        return;
    }

    // Libera recursos
    if (z80->cpu)
    {
        z80_destroy(z80->cpu);
    }

    free(z80);
    SMS_Z80_LOG_INFO("Adaptador Z80 para Master System destruído");
}

/**
 * @brief Reseta o Z80 para o estado inicial
 */
void sms_z80_adapter_reset(sms_z80_adapter_t *z80)
{
    if (!z80 || !z80->cpu)
    {
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
bool sms_z80_adapter_connect_memory(sms_z80_adapter_t *z80, sms_memory_t *memory)
{
    if (!z80)
    {
        return false;
    }

    z80->memory = memory;
    SMS_Z80_LOG_DEBUG("Sistema de memória conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Conecta o sistema de vídeo ao Z80
 */
bool sms_z80_adapter_connect_vdp(sms_z80_adapter_t *z80, void *vdp)
{
    if (!z80)
    {
        return false;
    }

    z80->vdp = vdp;
    SMS_Z80_LOG_DEBUG("VDP conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Conecta o sistema de áudio ao Z80
 */
bool sms_z80_adapter_connect_psg(sms_z80_adapter_t *z80, void *psg)
{
    if (!z80)
    {
        return false;
    }

    z80->psg = psg;
    SMS_Z80_LOG_DEBUG("PSG conectado ao adaptador Z80");
    return true;
}

/**
 * @brief Executa um passo de instrução no Z80
 */
uint8_t sms_z80_adapter_step(sms_z80_adapter_t *z80)
{
    if (!z80 || !z80->cpu)
    {
        return 0;
    }

    // Verifica interrupções pendentes
    if (z80->nmi_pending)
    {
        z80->nmi_pending = false;
        z80_interrupt(z80->cpu, true); // NMI
    }
    else if (z80->irq_pending)
    {
        // Apenas gera IRQ se interrupções estiverem habilitadas
        bool interrupts_enabled = z80_get_register(z80->cpu, Z80_REG_IFF1) != 0;
        if (interrupts_enabled)
        {
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
uint32_t sms_z80_adapter_run(sms_z80_adapter_t *z80, uint32_t cycles)
{
    if (!z80 || !z80->cpu)
    {
        return 0;
    }

    uint32_t executed_cycles = 0;

    // Executa ciclos no Z80 base
    while (executed_cycles < cycles)
    {
        executed_cycles += sms_z80_adapter_step(z80);
    }

    return executed_cycles;
}

/**
 * @brief Gera uma interrupção no Z80
 */
void sms_z80_adapter_interrupt(sms_z80_adapter_t *z80)
{
    if (!z80)
    {
        return;
    }

    // Marca a interrupção como pendente (será processada no próximo passo)
    z80->irq_pending = true;
}

/**
 * @brief Obtém o valor do registrador PC atual
 */
uint16_t sms_z80_adapter_get_pc(sms_z80_adapter_t *z80)
{
    if (!z80 || !z80->cpu)
    {
        return 0;
    }

    return z80_get_register(z80->cpu, Z80_REG_PC);
}

/**
 * @brief Registra o Z80 no sistema de save state
 */
int sms_z80_adapter_register_save_state(sms_z80_adapter_t *z80, save_state_t *state)
{
    if (!z80 || !state)
    {
        return -1;
    }

    // Registra os dados básicos do adaptador
    SAVE_STATE_REGISTER_SECTION(state, "SMS_Z80_ADAPTER");
    SAVE_STATE_REGISTER_FIELD(state, z80->vdp_status);
    SAVE_STATE_REGISTER_FIELD(state, z80->vdp_latch);
    SAVE_STATE_REGISTER_FIELD(state, z80->vdp_second_byte);
    SAVE_STATE_REGISTER_FIELD(state, z80->nmi_pending);
    SAVE_STATE_REGISTER_FIELD(state, z80->irq_pending);
    SAVE_STATE_END_SECTION(state);

    // Também precisa registrar o estado core do Z80
    // Isso deve ser implementado na biblioteca base
    extern int z80_register_save_state(z80_t * cpu, save_state_t * state);
    return z80_register_save_state(z80->cpu, state);
}

/**
 * @brief Atualiza o Z80 após um carregamento de save state
 */
void sms_z80_adapter_update_after_state_load(sms_z80_adapter_t *z80)
{
    if (!z80)
    {
        return;
    }

    // Atualiza o estado da CPU base se necessário
    extern void z80_update_after_state_load(z80_t * cpu);
    z80_update_after_state_load(z80->cpu);

    SMS_Z80_LOG_INFO("Estado do adaptador Z80 atualizado após carregamento de estado");
}

// Implementações das funções de callback

/**
 * @brief Callback para leitura de memória
 */
static uint8_t sms_z80_read_callback(void *context, uint16_t address)
{
    sms_z80_adapter_t *z80 = (sms_z80_adapter_t *)context;
    if (!z80 || !z80->memory)
    {
        return 0xFF;
    }

    // Delega a leitura para o sistema de memória do Master System
    extern uint8_t sms_memory_read(sms_memory_t * memory, uint16_t address);
    return sms_memory_read(z80->memory, address);
}

/**
 * @brief Callback para escrita na memória
 */
static void sms_z80_write_callback(void *context, uint16_t address, uint8_t value)
{
    sms_z80_adapter_t *z80 = (sms_z80_adapter_t *)context;
    if (!z80 || !z80->memory)
    {
        return;
    }

    // Delega a escrita para o sistema de memória do Master System
    extern void sms_memory_write(sms_memory_t * memory, uint16_t address, uint8_t value);
    sms_memory_write(z80->memory, address, value);
}

/**
 * @brief Callback para leitura de portas I/O
 */
static uint8_t sms_z80_io_read_callback(void *context, uint8_t port)
{
    sms_z80_adapter_t *z80 = (sms_z80_adapter_t *)context;
    if (!z80)
    {
        return 0xFF;
    }

    // No Master System, apenas os 8 bits inferiores são usados para endereçamento de I/O
    // Mas apenas certos bits são realmente considerados
    port &= 0xFF;

    // Leitura do VDP
    if ((port & 0xC1) == 0x40)
    { // 01xxxxxx para status do VDP (0x7E/0x7F no Game Gear)
        if (z80->vdp)
        {
            return sms_vdp_read_status(z80->vdp);
        }
    }
    else if ((port & 0xC1) == 0x80)
    { // 10xxxxxx para dados do VDP (0xBE no Master System)
        if (z80->vdp)
        {
            return sms_vdp_read_data(z80->vdp);
        }
    }

    // Leitura de controles e outras portas especiais
    if (port == 0xDC || port == 0xC0)
    { // Porta de controle 1
        extern uint8_t sms_input_read_port1(void);
        return sms_input_read_port1();
    }
    else if (port == 0xDD || port == 0xC1)
    { // Porta de controle 2
        extern uint8_t sms_input_read_port2(void);
        return sms_input_read_port2();
    }

    // Região de memória não mapeada
    SMS_Z80_LOG_TRACE("Leitura I/O não mapeada: 0x%02X", port);
    return 0xFF;
}

/**
 * @brief Callback para escrita em portas I/O
 */
static void sms_z80_io_write_callback(void *context, uint8_t port, uint8_t value)
{
    sms_z80_adapter_t *z80 = (sms_z80_adapter_t *)context;
    if (!z80)
    {
        return;
    }

    // No Master System, apenas os 8 bits inferiores são usados para endereçamento de I/O
    port &= 0xFF;

    // Escrita no VDP
    if ((port & 0xC1) == 0x80)
    { // 10xxxxxx para dados do VDP (0xBE no Master System)
        if (z80->vdp)
        {
            sms_vdp_write_data(z80->vdp, value);
        }
    }
    else if ((port & 0xC1) == 0x81)
    { // 10xxxxxx para controle do VDP (0xBF no Master System)
        if (z80->vdp)
        {
            sms_vdp_write_control(z80->vdp, value);
        }
    }

    // Escrita no PSG
    else if ((port & 0xC1) == 0x40)
    { // 01xxxxxx para PSG (0x7F no Master System)
        if (z80->psg)
        {
            sms_psg_write(z80->psg, value);
        }
    }

    // Controle de memória e mapeador
    else if (port == 0x3E || port == 0x3F)
    {
        // Registrador de controle de memória
        if (z80->memory)
        {
            sms_memory_control_write(z80->memory, value);
        }
    }
    else if (port >= 0xFC && port <= 0xFF)
    {
        // Registradores do mapeador de páginas
        if (z80->memory)
        {
            uint8_t reg = port & 0x03; // Mapeia 0xFC-0xFF para 0-3
            sms_memory_mapper_write(z80->memory, reg, value);
        }
    }
    else
    {
        SMS_Z80_LOG_TRACE("Escrita I/O não mapeada: porta=0x%02X, valor=0x%02X", port, value);
    }
}

// Implementações simplificadas das funções do VDP e PSG
// Estas funções seriam substituídas por chamadas reais para os componentes correspondentes

/**
 * @brief Lê dados do VDP
 *
 * Esta função lê dados da porta do VDP e reseta o latch de controle.
 */
static uint8_t sms_vdp_read_data(void *vdp)
{
    if (!vdp)
    {
        return 0xFF;
    }

    // Função externa do VDP para leitura de dados
    extern uint8_t sms_vdp_read_data_port(sms_vdp_t * vdp);
    uint8_t value = sms_vdp_read_data_port((sms_vdp_t *)vdp);

    SMS_Z80_LOG_TRACE("Leitura da porta de dados do VDP: 0x%02X", value);
    return value;
}

/**
 * @brief Lê status do VDP
 *
 * Esta função lê o registro de status do VDP e reseta o latch de controle.
 */
static uint8_t sms_vdp_read_status(void *vdp)
{
    if (!vdp)
    {
        return 0xFF;
    }

    // Função externa do VDP para leitura de status
    extern uint8_t sms_vdp_read_status_port(sms_vdp_t * vdp);
    uint8_t value = sms_vdp_read_status_port((sms_vdp_t *)vdp);

    SMS_Z80_LOG_TRACE("Leitura da porta de status do VDP: 0x%02X", value);
    return value;
}

/**
 * @brief Escreve dados no VDP
 *
 * Esta função escreve na porta de dados do VDP.
 */
static void sms_vdp_write_data(void *vdp, uint8_t value)
{
    if (!vdp)
    {
        return;
    }

    // Função externa do VDP para escrita de dados
    extern void sms_vdp_write_data_port(sms_vdp_t * vdp, uint8_t value);
    sms_vdp_write_data_port((sms_vdp_t *)vdp, value);

    SMS_Z80_LOG_TRACE("Escrita na porta de dados do VDP: 0x%02X", value);
}

/**
 * @brief Escreve no controle do VDP
 *
 * Esta função escreve na porta de controle do VDP, que pode ser um
 * endereço para operações subsequentes ou um valor para registrador.
 */
static void sms_vdp_write_control(void *vdp, uint8_t value)
{
    if (!vdp)
    {
        return;
    }

    // Função externa do VDP para escrita no controle
    extern void sms_vdp_write_control_port(sms_vdp_t * vdp, uint8_t value);
    sms_vdp_write_control_port((sms_vdp_t *)vdp, value);

    SMS_Z80_LOG_TRACE("Escrita na porta de controle do VDP: 0x%02X", value);
}

/**
 * @brief Escreve no PSG (Programmable Sound Generator)
 *
 * Esta função envia um comando para o PSG.
 */
static void sms_psg_write(void *psg, uint8_t value)
{
    if (!psg)
    {
        return;
    }

    // Função externa do PSG para escrita
    extern void sms_psg_write_port(sms_psg_t * psg, uint8_t value);
    sms_psg_write_port((sms_psg_t *)psg, value);

    SMS_Z80_LOG_TRACE("Escrita na porta do PSG: 0x%02X", value);
}
