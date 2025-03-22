/**
 * @file sms_io.c
 * @brief Implementação do sistema de entrada do Master System
 */

#include "sms_io.h"
#include "../../../utils/enhanced_log.h"
#include "../../../utils/log_categories.h"
#include <stdlib.h>
#include <string.h>

// Definir a categoria de log para o sistema de entrada do Master System
#define EMU_LOG_CAT_IO EMU_LOG_CAT_MASTERSYSTEM

// Macros de log específicas para o sistema de entrada do Master System
#define SMS_IO_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_IO, __VA_ARGS__)
#define SMS_IO_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_IO, __VA_ARGS__)
#define SMS_IO_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_IO, __VA_ARGS__)
#define SMS_IO_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_IO, __VA_ARGS__)
#define SMS_IO_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_IO, __VA_ARGS__)

/**
 * @brief Portas de I/O do Master System
 */
#define SMS_IO_PORT_A 0xDC        // Porta A (controle 1)
#define SMS_IO_PORT_B 0xDD        // Porta B (controle 2)
#define SMS_IO_CONTROL_PORT 0x3F  // Porta de controle

/**
 * @brief Estrutura interna do sistema de entrada
 */
struct sms_input_t {
    uint8_t port_a;               // Estado da porta A (controle 1)
    uint8_t port_b;               // Estado da porta B (controle 2)
    uint8_t control_port;         // Estado da porta de controle
    
    // Estado dos controles
    sms_controller_state_t controllers[SMS_MAX_CONTROLLERS];
    
    // Estado do teclado (para computadores compatíveis)
    uint8_t keyboard[16];
    
    // Flags de configuração
    uint8_t is_gg;                // Flag indicando se é Game Gear
    uint8_t is_sg1000;            // Flag indicando se é SG-1000
    uint8_t region;               // Região do console (0 = Japão, 1 = EUA, 2 = Europa)
};

/**
 * @brief Cria uma nova instância do sistema de entrada
 * 
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
sms_input_t* sms_input_create(void) {
    sms_input_t *input = (sms_input_t*)malloc(sizeof(sms_input_t));
    if (!input) {
        SMS_IO_LOG_ERROR("Falha ao alocar memória para o sistema de entrada");
        return NULL;
    }
    
    // Inicializa a estrutura
    memset(input, 0, sizeof(sms_input_t));
    
    // Inicializa as portas com valores padrão
    // No Master System, os bits são ativos em baixo (0 = pressionado, 1 = liberado)
    input->port_a = 0xFF;
    input->port_b = 0xFF;
    input->control_port = 0xFF;
    
    // Inicializa os controles
    for (int i = 0; i < SMS_MAX_CONTROLLERS; i++) {
        memset(&input->controllers[i], 0, sizeof(sms_controller_state_t));
    }
    
    // Inicializa o teclado
    memset(input->keyboard, 0, sizeof(input->keyboard));
    
    SMS_IO_LOG_INFO("Sistema de entrada criado com sucesso");
    
    return input;
}

/**
 * @brief Destrói uma instância do sistema de entrada e libera recursos
 * 
 * @param input Ponteiro para a instância
 */
void sms_input_destroy(sms_input_t *input) {
    if (!input) {
        return;
    }
    
    // Libera a estrutura principal
    free(input);
    
    SMS_IO_LOG_INFO("Sistema de entrada destruído");
}

/**
 * @brief Reseta o sistema de entrada para o estado inicial
 * 
 * @param input Ponteiro para a instância
 */
void sms_input_reset(sms_input_t *input) {
    if (!input) {
        return;
    }
    
    // Reseta as portas para os valores padrão
    input->port_a = 0xFF;
    input->port_b = 0xFF;
    input->control_port = 0xFF;
    
    // Reseta os controles
    for (int i = 0; i < SMS_MAX_CONTROLLERS; i++) {
        memset(&input->controllers[i], 0, sizeof(sms_controller_state_t));
    }
    
    // Reseta o teclado
    memset(input->keyboard, 0, sizeof(input->keyboard));
    
    SMS_IO_LOG_INFO("Sistema de entrada resetado");
}

/**
 * @brief Configura o sistema de entrada para o modo Game Gear
 * 
 * @param input Ponteiro para a instância
 * @param is_gg Flag indicando se é Game Gear
 */
void sms_input_set_game_gear(sms_input_t *input, uint8_t is_gg) {
    if (!input) {
        return;
    }
    
    input->is_gg = is_gg ? 1 : 0;
    
    SMS_IO_LOG_INFO("Modo Game Gear %s", is_gg ? "ativado" : "desativado");
}

/**
 * @brief Configura o sistema de entrada para o modo SG-1000
 * 
 * @param input Ponteiro para a instância
 * @param is_sg1000 Flag indicando se é SG-1000
 */
void sms_input_set_sg1000(sms_input_t *input, uint8_t is_sg1000) {
    if (!input) {
        return;
    }
    
    input->is_sg1000 = is_sg1000 ? 1 : 0;
    
    SMS_IO_LOG_INFO("Modo SG-1000 %s", is_sg1000 ? "ativado" : "desativado");
}

/**
 * @brief Configura a região do console
 * 
 * @param input Ponteiro para a instância
 * @param region Região (0 = Japão, 1 = EUA, 2 = Europa)
 */
void sms_input_set_region(sms_input_t *input, uint8_t region) {
    if (!input) {
        return;
    }
    
    input->region = region;
    
    const char *region_str = "Desconhecida";
    switch (region) {
        case 0: region_str = "Japão"; break;
        case 1: region_str = "EUA"; break;
        case 2: region_str = "Europa"; break;
    }
    
    SMS_IO_LOG_INFO("Região configurada: %s", region_str);
}

/**
 * @brief Atualiza o estado de um botão do controle
 * 
 * @param input Ponteiro para a instância
 * @param controller_id ID do controle (0 ou 1)
 * @param button Botão a ser atualizado
 * @param pressed Estado do botão (1 = pressionado, 0 = liberado)
 */
void sms_input_set_button(sms_input_t *input, uint8_t controller_id, sms_button_t button, uint8_t pressed) {
    if (!input || controller_id >= SMS_MAX_CONTROLLERS) {
        return;
    }
    
    // Atualiza o estado do botão
    if (pressed) {
        input->controllers[controller_id].buttons |= button;
    } else {
        input->controllers[controller_id].buttons &= ~button;
    }
    
    // Atualiza as portas de acordo com o estado dos botões
    // No Master System, os bits são ativos em baixo (0 = pressionado, 1 = liberado)
    
    // Controle 1 (Porta A)
    if (controller_id == 0) {
        // Botões direcionais
        if (input->controllers[0].buttons & SMS_BUTTON_UP) {
            input->port_a &= ~0x01;
        } else {
            input->port_a |= 0x01;
        }
        
        if (input->controllers[0].buttons & SMS_BUTTON_DOWN) {
            input->port_a &= ~0x02;
        } else {
            input->port_a |= 0x02;
        }
        
        if (input->controllers[0].buttons & SMS_BUTTON_LEFT) {
            input->port_a &= ~0x04;
        } else {
            input->port_a |= 0x04;
        }
        
        if (input->controllers[0].buttons & SMS_BUTTON_RIGHT) {
            input->port_a &= ~0x08;
        } else {
            input->port_a |= 0x08;
        }
        
        // Botões de ação
        if (input->controllers[0].buttons & SMS_BUTTON_1) {
            input->port_a &= ~0x10;
        } else {
            input->port_a |= 0x10;
        }
        
        if (input->controllers[0].buttons & SMS_BUTTON_2) {
            input->port_a &= ~0x20;
        } else {
            input->port_a |= 0x20;
        }
    }
    
    // Controle 2 (Porta B)
    if (controller_id == 1) {
        // Botões direcionais
        if (input->controllers[1].buttons & SMS_BUTTON_UP) {
            input->port_b &= ~0x01;
        } else {
            input->port_b |= 0x01;
        }
        
        if (input->controllers[1].buttons & SMS_BUTTON_DOWN) {
            input->port_b &= ~0x02;
        } else {
            input->port_b |= 0x02;
        }
        
        if (input->controllers[1].buttons & SMS_BUTTON_LEFT) {
            input->port_b &= ~0x04;
        } else {
            input->port_b |= 0x04;
        }
        
        if (input->controllers[1].buttons & SMS_BUTTON_RIGHT) {
            input->port_b &= ~0x08;
        } else {
            input->port_b |= 0x08;
        }
        
        // Botões de ação
        if (input->controllers[1].buttons & SMS_BUTTON_1) {
            input->port_b &= ~0x10;
        } else {
            input->port_b |= 0x10;
        }
        
        if (input->controllers[1].buttons & SMS_BUTTON_2) {
            input->port_b &= ~0x20;
        } else {
            input->port_b |= 0x20;
        }
    }
    
    // Botão Start/Pause (compartilhado)
    // No Master System, o botão Pause é conectado à linha de interrupção não-mascarável (NMI)
    // No Game Gear, o botão Start é mapeado para a porta de controle
    if (input->is_gg) {
        if ((input->controllers[0].buttons & SMS_BUTTON_START) || 
            (input->controllers[1].buttons & SMS_BUTTON_START)) {
            input->control_port &= ~0x80;
        } else {
            input->control_port |= 0x80;
        }
    }
    
    SMS_IO_LOG_TRACE("Botão %d do controle %d %s", button, controller_id, pressed ? "pressionado" : "liberado");
}

/**
 * @brief Obtém o estado atual de um controle
 * 
 * @param input Ponteiro para a instância
 * @param controller_id ID do controle (0 ou 1)
 * @param state Ponteiro para a estrutura que receberá o estado
 */
void sms_input_get_controller_state(sms_input_t *input, uint8_t controller_id, sms_controller_state_t *state) {
    if (!input || !state || controller_id >= SMS_MAX_CONTROLLERS) {
        return;
    }
    
    // Copia o estado do controle
    *state = input->controllers[controller_id];
}

/**
 * @brief Lê o valor de uma porta de I/O
 * 
 * @param input Ponteiro para a instância
 * @param port Porta a ser lida
 * @return Valor lido
 */
uint8_t sms_input_read_port(sms_input_t *input, uint8_t port) {
    if (!input) {
        return 0xFF;
    }
    
    // Leitura das portas de I/O
    switch (port) {
        case SMS_IO_PORT_A:
            // Porta A (controle 1)
            return input->port_a;
            
        case SMS_IO_PORT_B:
            // Porta B (controle 2)
            return input->port_b;
            
        case SMS_IO_CONTROL_PORT:
            // Porta de controle
            return input->control_port;
            
        default:
            // Porta desconhecida
            SMS_IO_LOG_TRACE("Leitura de porta desconhecida: 0x%02X", port);
            return 0xFF;
    }
}

/**
 * @brief Escreve um valor em uma porta de I/O
 * 
 * @param input Ponteiro para a instância
 * @param port Porta a ser escrita
 * @param value Valor a ser escrito
 */
void sms_input_write_port(sms_input_t *input, uint8_t port, uint8_t value) {
    if (!input) {
        return;
    }
    
    // Escrita nas portas de I/O
    switch (port) {
        case SMS_IO_CONTROL_PORT:
            // Porta de controle
            input->control_port = value;
            break;
            
        default:
            // Porta desconhecida
            SMS_IO_LOG_TRACE("Escrita em porta desconhecida: 0x%02X = 0x%02X", port, value);
            break;
    }
}

/**
 * @brief Registra o sistema de entrada no sistema de save state
 * 
 * @param input Ponteiro para a instância
 * @param state Ponteiro para o contexto de save state
 * @return Código de erro (0 para sucesso)
 */
int sms_input_register_save_state(sms_input_t *input, save_state_t *state) {
    if (!input || !state) {
        return -1;
    }
    
    // Registra as portas
    save_state_register_field(state, "sms_port_a", &input->port_a, 1);
    save_state_register_field(state, "sms_port_b", &input->port_b, 1);
    save_state_register_field(state, "sms_control_port", &input->control_port, 1);
    
    // Registra os controles
    save_state_register_field(state, "sms_controllers", input->controllers, sizeof(input->controllers));
    
    // Registra o teclado
    save_state_register_field(state, "sms_keyboard", input->keyboard, sizeof(input->keyboard));
    
    // Registra as flags de configuração
    save_state_register_field(state, "sms_is_gg", &input->is_gg, 1);
    save_state_register_field(state, "sms_is_sg1000", &input->is_sg1000, 1);
    save_state_register_field(state, "sms_region", &input->region, 1);
    
    SMS_IO_LOG_DEBUG("Sistema de entrada registrado no sistema de save state");
    
    return 0;
}
