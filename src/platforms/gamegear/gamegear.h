/**
 * @file gamegear.h
 * @brief Interface principal do emulador Game Gear
 */

#ifndef EMU_GAMEGEAR_H
#define EMU_GAMEGEAR_H

#include "../../core/config.h"
#include "../../core/save_state.h"
#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct gamegear_t gamegear_t;
typedef struct sms_vdp_t sms_vdp_t;
typedef struct sms_psg_t sms_psg_t;
typedef struct sms_memory_t sms_memory_t;
typedef struct sms_z80_adapter_t sms_z80_adapter_t;

// Portas de I/O específicas do Game Gear
#define GG_PORT_LCD_CONTROL      0x00  // Controle do LCD
#define GG_PORT_STEREO_CONTROL   0x06  // Controle de som estéreo
#define GG_PORT_LCD_CONTRAST     0x10  // Controle de contraste do LCD
#define GG_PORT_POWER_SAVE       0x11  // Controle de economia de energia
#define GG_PORT_START_BUTTON     0x80  // Estado do botão Start

// Flags de controle do LCD
#define GG_LCD_BACKLIGHT_ON      0x01  // Liga retroiluminação
#define GG_LCD_ENABLE            0x02  // Habilita LCD
#define GG_LCD_NORMAL_MODE       0x04  // Modo normal (vs. sleep)

// Flags de controle de economia de energia
#define GG_POWER_LCD_OFF         0x01  // Desliga LCD para poupar energia
#define GG_POWER_PSG_OFF         0x02  // Desliga PSG para poupar energia
#define GG_POWER_Z80_SLOW        0x04  // Reduz clock do Z80 para poupar energia
#define GG_POWER_DEEP_SLEEP      0x08  // Modo de sono profundo

/**
 * @brief Estrutura para o adaptador de Master System
 */
typedef struct {
    bool enabled;               // Adaptador habilitado
    bool force_sms_mode;        // Força modo Master System
    bool stretch_display;       // Estender display para tela inteira
    bool apply_palette_filter;  // Aplicar filtro de paleta para jogos SMS
} gg_sms_adapter_t;

/**
 * @brief Cria uma nova instância do emulador Game Gear
 * @param config Configuração do emulador
 * @return Ponteiro para a instância ou NULL em caso de erro
 */
gamegear_t *gamegear_create(config_t *config);

/**
 * @brief Destrói uma instância do emulador Game Gear
 * @param gg Ponteiro para a instância
 */
void gamegear_destroy(gamegear_t *gg);

/**
 * @brief Reseta o emulador Game Gear
 * @param gg Ponteiro para a instância
 */
void gamegear_reset(gamegear_t *gg);

/**
 * @brief Carrega uma ROM no Game Gear
 * @param gg Ponteiro para a instância
 * @param rom_data Dados da ROM
 * @param rom_size Tamanho da ROM em bytes
 * @return true se sucesso, false caso contrário
 */
bool gamegear_load_rom(gamegear_t *gg, const uint8_t *rom_data,
                       size_t rom_size);

/**
 * @brief Executa um frame do Game Gear
 * @param gg Ponteiro para a instância
 */
void gamegear_run_frame(gamegear_t *gg);

/**
 * @brief Obtém o buffer de vídeo atual
 * @param gg Ponteiro para a instância
 * @return Ponteiro para o buffer de vídeo
 */
const uint16_t *gamegear_get_video_buffer(gamegear_t *gg);

/**
 * @brief Obtém o buffer de áudio atual
 * @param gg Ponteiro para a instância
 * @param size Ponteiro para armazenar o tamanho do buffer
 * @return Ponteiro para o buffer de áudio
 */
const int16_t *gamegear_get_audio_buffer(gamegear_t *gg, size_t *size);

/**
 * @brief Processa input do Game Gear
 * @param gg Ponteiro para a instância
 * @param port Porta de input (1 ou 2)
 * @param value Valor do input
 */
void gamegear_set_input(gamegear_t *gg, int port, uint8_t value);

/**
 * @brief Registra o estado para save state
 * @param gg Ponteiro para a instância
 * @param state Ponteiro para o save state
 * @return 0 se sucesso, -1 caso contrário
 */
int gamegear_register_save_state(gamegear_t *gg, save_state_t *state);

/**
 * @brief Atualiza o estado após carregar um save state
 * @param gg Ponteiro para a instância
 */
void gamegear_update_state(gamegear_t *gg);

/**
 * @brief Pausa ou retoma a emulação
 * @param gg Ponteiro para a instância
 * @param paused true para pausar, false para retomar
 */
void gamegear_set_paused(gamegear_t *gg, bool paused);

/**
 * @brief Verifica se o emulador está pausado
 * @param gg Ponteiro para a instância
 * @return true se pausado, false caso contrário
 */
bool gamegear_is_paused(gamegear_t *gg);

/**
 * @brief Configura o nível de contraste do LCD
 * @param gg Ponteiro para a instância
 * @param level Nível de contraste (0-31)
 */
void gamegear_set_lcd_contrast(gamegear_t *gg, uint8_t level);

/**
 * @brief Configura o modo de economia de energia
 * @param gg Ponteiro para a instância
 * @param mode Combinação de flags GG_POWER_*
 */
void gamegear_set_power_mode(gamegear_t *gg, uint8_t mode);

/**
 * @brief Configura o adaptador de Master System
 * @param gg Ponteiro para a instância
 * @param adapter Configuração do adaptador
 */
void gamegear_set_sms_adapter(gamegear_t *gg, const gg_sms_adapter_t *adapter);

#endif // EMU_GAMEGEAR_H
