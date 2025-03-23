/**
 * @file sms_vdp.c
 * @brief Implementação do Video Display Processor (VDP) do Master System
 */

#include "sms_vdp.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include "../../../core/save_state.h"
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o VDP do Master System
#define EMU_LOG_CAT_SMS_VDP EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para o VDP do Master System
#define SMS_VDP_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)
#define SMS_VDP_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_SMS_VDP, __VA_ARGS__)

// Definições de registradores do VDP
#define VDP_REG0_MASK 0xFF  // Modo de vídeo 1
#define VDP_REG1_MASK 0xFF  // Modo de vídeo 2
#define VDP_REG2_MASK 0xFF  // Base de nomes de padrão
#define VDP_REG3_MASK 0xFF  // Base de cores
#define VDP_REG4_MASK 0xFF  // Base de padrões
#define VDP_REG5_MASK 0xFF  // Base de sprites
#define VDP_REG6_MASK 0xFF  // Base de cores de sprites
#define VDP_REG7_MASK 0xFF  // Cor de borda/background
#define VDP_REG8_MASK 0xFF  // Scroll X
#define VDP_REG9_MASK 0xFF  // Scroll Y
#define VDP_REG10_MASK 0xFF // Contador de linhas

// Flags do registrador de status
#define VDP_STATUS_VBLANK 0x80      // Frame concluído
#define VDP_STATUS_SPRITE_COLL 0x20 // Colisão de sprites
#define VDP_STATUS_SPRITE_OVER 0x40 // Overflow de sprites

// Tamanho da VRAM (16KB)
#define VDP_VRAM_SIZE 0x4000

// Tamanho da CRAM (32 bytes para SMS, 64 para GG)
#define VDP_CRAM_SIZE 0x40

/**
 * @brief Estrutura do VDP do Master System
 */
struct sms_vdp_t
{
    uint8_t *vram;             // VRAM (16KB)
    uint8_t *cram;             // CRAM (paleta de cores)
    uint8_t regs[16];          // Registradores do VDP (0-10 são usados)
    uint8_t status;            // Registrador de status
    uint8_t control_latch;     // Buffer para o byte de controle
    bool second_control_byte;  // Flag para o segundo byte de controle
    uint16_t address_register; // Registrador de endereço interno
    uint8_t data_buffer;       // Buffer de leitura de dados
    bool data_read_pending;    // Flag para leitura de dados

    uint8_t mode;           // Modo do VDP (0-4)
    uint16_t line_counter;  // Contador de linhas
    uint16_t current_line;  // Linha atual sendo renderizada
    bool interrupt_pending; // Flag para interrupção pendente

    void *cpu; // Ponteiro para a CPU

    bool game_gear_mode; // Flag para modo Game Gear

    // Frame buffer interno
    uint32_t *frame_buffer;  // Ponteiro para o buffer de frame
    bool frame_complete;     // Flag de frame completo
    uint32_t cycles_counter; // Contador de ciclos
    uint32_t frame_cycles;   // Ciclos por frame
};

// Forward declarations de funções internas
static void sms_vdp_reset_registers(sms_vdp_t *vdp);
static void sms_vdp_control_write(sms_vdp_t *vdp, uint8_t value);
static void sms_vdp_render_line(sms_vdp_t *vdp, uint16_t line);
static void sms_vdp_update_status(sms_vdp_t *vdp);

/**
 * @brief Cria uma nova instância do VDP
 */
sms_vdp_t *sms_vdp_create(void)
{
    sms_vdp_t *vdp = (sms_vdp_t *)malloc(sizeof(sms_vdp_t));
    if (!vdp)
    {
        SMS_VDP_LOG_ERROR("Falha ao alocar memória para o VDP");
        return NULL;
    }

    // Inicializa a estrutura
    memset(vdp, 0, sizeof(sms_vdp_t));

    // Aloca a VRAM
    vdp->vram = (uint8_t *)malloc(VDP_VRAM_SIZE);
    if (!vdp->vram)
    {
        SMS_VDP_LOG_ERROR("Falha ao alocar memória para a VRAM");
        free(vdp);
        return NULL;
    }

    // Aloca a CRAM
    vdp->cram = (uint8_t *)malloc(VDP_CRAM_SIZE);
    if (!vdp->cram)
    {
        SMS_VDP_LOG_ERROR("Falha ao alocar memória para a CRAM");
        free(vdp->vram);
        free(vdp);
        return NULL;
    }

    // Inicializa a memória
    memset(vdp->vram, 0, VDP_VRAM_SIZE);
    memset(vdp->cram, 0, VDP_CRAM_SIZE);

    // Inicializa os registradores
    sms_vdp_reset_registers(vdp);

    SMS_VDP_LOG_INFO("VDP do Master System criado com sucesso");
    return vdp;
}

/**
 * @brief Destrói uma instância do VDP e libera recursos
 */
void sms_vdp_destroy(sms_vdp_t *vdp)
{
    if (!vdp)
    {
        return;
    }

    // Libera recursos
    if (vdp->vram)
    {
        free(vdp->vram);
    }

    if (vdp->cram)
    {
        free(vdp->cram);
    }

    free(vdp);
    SMS_VDP_LOG_INFO("VDP do Master System destruído");
}

/**
 * @brief Reseta o VDP para o estado inicial
 */
void sms_vdp_reset(sms_vdp_t *vdp)
{
    if (!vdp)
    {
        return;
    }

    // Reseta a VRAM e CRAM
    memset(vdp->vram, 0, VDP_VRAM_SIZE);
    memset(vdp->cram, 0, VDP_CRAM_SIZE);

    // Reseta registradores
    sms_vdp_reset_registers(vdp);

    // Reseta estados internos
    vdp->status = 0;
    vdp->control_latch = 0;
    vdp->second_control_byte = false;
    vdp->address_register = 0;
    vdp->data_buffer = 0;
    vdp->data_read_pending = false;
    vdp->current_line = 0;
    vdp->interrupt_pending = false;
    vdp->frame_complete = false;
    vdp->cycles_counter = 0;

    SMS_VDP_LOG_INFO("VDP do Master System resetado");
}

/**
 * @brief Reseta os registradores do VDP para valores padrão
 */
static void sms_vdp_reset_registers(sms_vdp_t *vdp)
{
    if (!vdp)
    {
        return;
    }

    // Inicializa registradores com valores padrão
    for (int i = 0; i < 16; i++)
    {
        vdp->regs[i] = 0;
    }

    // Alguns valores padrão específicos
    vdp->regs[2] = 0x0E; // Base de nomes em 0x3800
    vdp->regs[5] = 0x7E; // Base de sprites em 0x3F00

    vdp->mode = 0; // Modo inicial
}

/**
 * @brief Conecta o VDP à CPU
 */
void sms_vdp_connect_cpu(sms_vdp_t *vdp, void *cpu)
{
    if (!vdp)
    {
        return;
    }

    vdp->cpu = cpu;
    SMS_VDP_LOG_DEBUG("CPU conectada ao VDP");
}

/**
 * @brief Inicia um novo frame no VDP
 */
void sms_vdp_start_frame(sms_vdp_t *vdp)
{
    if (!vdp)
    {
        return;
    }

    vdp->current_line = 0;
    vdp->frame_complete = false;
    vdp->cycles_counter = 0;

    // Define o número de ciclos por frame com base no modo
    vdp->frame_cycles = vdp->game_gear_mode ? 228 * 160 : 228 * 262;

    SMS_VDP_LOG_TRACE("Iniciando novo frame");
}

/**
 * @brief Atualiza o estado do VDP com base nos ciclos executados
 */
void sms_vdp_update(sms_vdp_t *vdp, uint8_t cycles)
{
    if (!vdp || vdp->frame_complete)
    {
        return;
    }

    // Atualiza o contador de ciclos
    vdp->cycles_counter += cycles;

    // Calcula a linha atual com base nos ciclos
    uint16_t new_line = vdp->cycles_counter / 228;

    // Verifica se mudou de linha
    if (new_line > vdp->current_line)
    {
        // Renderiza as linhas puladas
        for (uint16_t line = vdp->current_line; line < new_line; line++)
        {
            // Renderiza apenas linhas visíveis
            if (line < (vdp->game_gear_mode ? SMS_GG_SCREEN_HEIGHT : SMS_SCREEN_HEIGHT))
            {
                sms_vdp_render_line(vdp, line);
            }

            // Atualiza contador de linhas
            vdp->line_counter--;
            if (vdp->line_counter == 0xFF)
            {
                // Reinicia contador de linhas e verifica interrupções
                vdp->line_counter = vdp->regs[10];

                // Gera interrupção se habilitado
                if (vdp->regs[0] & 0x10)
                {
                    vdp->interrupt_pending = true;
                }
            }
        }

        vdp->current_line = new_line;
    }

    // Verifica se o frame está completo
    if (vdp->current_line >= (vdp->game_gear_mode ? 144 : 262))
    {
        vdp->frame_complete = true;
        vdp->status |= VDP_STATUS_VBLANK;

        // Gera interrupção de frame se habilitado
        if (vdp->regs[1] & 0x20)
        {
            vdp->interrupt_pending = true;
        }
    }
}

/**
 * @brief Renderiza uma linha do VDP
 */
static void sms_vdp_render_line(sms_vdp_t *vdp, uint16_t line)
{
    // Implementação simplificada - seria altamente otimizada em uma implementação real

    // Se o frame buffer não estiver alocado, não renderiza
    if (!vdp->frame_buffer)
    {
        return;
    }

    // Verifica se o display está desativado
    if (!(vdp->regs[1] & 0x40))
    {
        // Preenche com cor de borda
        uint8_t border_color = vdp->regs[7] & 0x0F;
        uint32_t color = 0xFF000000; // Cor padrão (preto opaco)

        // Preencheria com a cor da borda em uma implementação completa
        uint32_t *line_buffer = vdp->frame_buffer + (line * (vdp->game_gear_mode ? SMS_GG_SCREEN_WIDTH : SMS_SCREEN_WIDTH));
        for (int x = 0; x < (vdp->game_gear_mode ? SMS_GG_SCREEN_WIDTH : SMS_SCREEN_WIDTH); x++)
        {
            line_buffer[x] = color;
        }
        return;
    }

    // Implementação real renderia aqui os sprites e tiles
    // ...
}

/**
 * @brief Finaliza o frame atual e renderiza para o buffer
 */
void sms_vdp_end_frame(sms_vdp_t *vdp, uint32_t *frame_buffer)
{
    if (!vdp || !frame_buffer)
    {
        return;
    }

    // Define o buffer de frame
    vdp->frame_buffer = frame_buffer;

    // Renderiza linhas restantes se necessário
    if (!vdp->frame_complete)
    {
        uint16_t max_lines = vdp->game_gear_mode ? SMS_GG_SCREEN_HEIGHT : SMS_SCREEN_HEIGHT;
        for (uint16_t line = vdp->current_line; line < max_lines; line++)
        {
            sms_vdp_render_line(vdp, line);
        }
    }

    // Reseta o estado para o próximo frame
    vdp->status |= VDP_STATUS_VBLANK;
    vdp->frame_complete = true;

    SMS_VDP_LOG_TRACE("Frame finalizado");
}

/**
 * @brief Verifica se o VDP está gerando uma interrupção
 */
uint8_t sms_vdp_check_interrupt(sms_vdp_t *vdp)
{
    if (!vdp)
    {
        return 0;
    }

    return vdp->interrupt_pending ? 1 : 0;
}

/**
 * @brief Lê um byte da porta de dados do VDP
 */
uint8_t sms_vdp_read_data_port(sms_vdp_t *vdp)
{
    if (!vdp)
    {
        return 0xFF;
    }

    // Reseta o estado de controle
    vdp->second_control_byte = false;

    // Obtém o valor do buffer de dados
    uint8_t value = vdp->data_buffer;

    // Lê próximo byte e incrementa o endereço
    vdp->data_buffer = vdp->vram[vdp->address_register & 0x3FFF];
    vdp->address_register = (vdp->address_register + 1) & 0x3FFF;

    SMS_VDP_LOG_TRACE("Leitura da porta de dados: 0x%02X, próximo endereço: 0x%04X", value, vdp->address_register);
    return value;
}

/**
 * @brief Lê um byte da porta de status do VDP
 */
uint8_t sms_vdp_read_status_port(sms_vdp_t *vdp)
{
    if (!vdp)
    {
        return 0xFF;
    }

    // Reseta o estado de controle
    vdp->second_control_byte = false;

    // Obtém valor do status
    uint8_t value = vdp->status;

    // Limpa os flags de status
    vdp->status = 0;

    // Reseta interrupção pendente
    vdp->interrupt_pending = false;

    SMS_VDP_LOG_TRACE("Leitura da porta de status: 0x%02X", value);
    return value;
}

/**
 * @brief Escreve um byte na porta de dados do VDP
 */
void sms_vdp_write_data_port(sms_vdp_t *vdp, uint8_t value)
{
    if (!vdp)
    {
        return;
    }

    // Reseta o estado de controle
    vdp->second_control_byte = false;

    // Escreve o valor na VRAM
    vdp->vram[vdp->address_register & 0x3FFF] = value;

    // Incrementa o endereço
    vdp->address_register = (vdp->address_register + 1) & 0x3FFF;

    SMS_VDP_LOG_TRACE("Escrita na porta de dados: 0x%02X, próximo endereço: 0x%04X", value, vdp->address_register);
}

/**
 * @brief Escreve um byte na porta de controle do VDP
 */
void sms_vdp_write_control_port(sms_vdp_t *vdp, uint8_t value)
{
    if (!vdp)
    {
        return;
    }

    if (!vdp->second_control_byte)
    {
        // Primeiro byte: parte baixa do endereço
        vdp->control_latch = value;
        vdp->address_register = (vdp->address_register & 0x3F00) | value;
        vdp->second_control_byte = true;

        SMS_VDP_LOG_TRACE("Escrita na porta de controle (1º byte): 0x%02X", value);
    }
    else
    {
        // Segundo byte: parte alta do endereço e código de operação
        vdp->second_control_byte = false;

        // Bits 6-7 determinam a operação
        uint8_t code = (value >> 6) & 0x03;

        // Atualiza o registrador de endereço
        vdp->address_register = ((value & 0x3F) << 8) | vdp->control_latch;

        switch (code)
        {
        case 0: // Leitura da VRAM
            // Pré-carrega o buffer de dados
            vdp->data_buffer = vdp->vram[vdp->address_register & 0x3FFF];
            vdp->address_register = (vdp->address_register + 1) & 0x3FFF;
            SMS_VDP_LOG_TRACE("Configuração para leitura VRAM, endereço: 0x%04X", vdp->address_register - 1);
            break;

        case 1: // Escrita na VRAM
            SMS_VDP_LOG_TRACE("Configuração para escrita VRAM, endereço: 0x%04X", vdp->address_register);
            break;

        case 2: // Escrita em registrador
        {
            uint8_t reg = value & 0x0F; // Apenas registradores 0-15
            vdp->regs[reg] = vdp->control_latch;

            // Atualiza o modo com base nos registradores
            if (reg <= 1)
            {
                vdp->mode = ((vdp->regs[1] & 0x10) >> 1) | ((vdp->regs[0] & 0x02) >> 1) | ((vdp->regs[1] & 0x08) >> 3);
            }

            SMS_VDP_LOG_TRACE("Escrita em registrador %d: 0x%02X", reg, vdp->control_latch);
        }
        break;

        case 3: // Escrita na CRAM
        {
            uint8_t cram_addr = vdp->address_register & (VDP_CRAM_SIZE - 1);
            vdp->cram[cram_addr] = vdp->control_latch;
            vdp->address_register = (vdp->address_register + 1) & 0x3FFF;
            SMS_VDP_LOG_TRACE("Escrita na CRAM, endereço: 0x%02X, valor: 0x%02X", cram_addr, vdp->control_latch);
        }
        break;
        }
    }
}

/**
 * @brief Registra o estado do VDP no sistema de save state
 */
int sms_vdp_register_save_state(sms_vdp_t *vdp, save_state_t *state)
{
    if (!vdp || !state)
    {
        return -1;
    }

    // Implementação simplificada - em uma implementação real, registraria todos os campos necessários
    save_state_register_section(state, "SMS_VDP");

    save_state_register_field(state, vdp->vram, VDP_VRAM_SIZE);
    save_state_register_field(state, vdp->cram, VDP_CRAM_SIZE);
    save_state_register_field(state, vdp->regs, sizeof(vdp->regs));
    save_state_register_field(state, &vdp->status, sizeof(vdp->status));
    save_state_register_field(state, &vdp->control_latch, sizeof(vdp->control_latch));
    save_state_register_field(state, &vdp->second_control_byte, sizeof(vdp->second_control_byte));
    save_state_register_field(state, &vdp->address_register, sizeof(vdp->address_register));
    save_state_register_field(state, &vdp->data_buffer, sizeof(vdp->data_buffer));
    save_state_register_field(state, &vdp->mode, sizeof(vdp->mode));
    save_state_register_field(state, &vdp->line_counter, sizeof(vdp->line_counter));
    save_state_register_field(state, &vdp->interrupt_pending, sizeof(vdp->interrupt_pending));
    save_state_register_field(state, &vdp->game_gear_mode, sizeof(vdp->game_gear_mode));

    save_state_end_section(state);

    SMS_VDP_LOG_INFO("Estado do VDP registrado no sistema de save state");
    return 0;
}
