/**
 * @file mastersystem.h
 * @brief Definições principais para a plataforma Master System/Game Gear
 */

#ifndef MASTERSYSTEM_H
#define MASTERSYSTEM_H

#include "core/core_types.h"
#include "utils/enhanced_log.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief Categorias de log específicas para o Master System 
 */
#define SMS_LOG_CAT_MAIN EMU_LOG_CAT_PLATFORM
#define SMS_LOG_CAT_CPU EMU_LOG_CAT_CPU
#define SMS_LOG_CAT_VDP EMU_LOG_CAT_VIDEO
#define SMS_LOG_CAT_PSG EMU_LOG_CAT_AUDIO
#define SMS_LOG_CAT_MEMORY EMU_LOG_CAT_MEMORY
#define SMS_LOG_CAT_INPUT EMU_LOG_CAT_INPUT
#define SMS_LOG_CAT_CARTRIDGE SMS_LOG_CAT_MAIN

/** 
 * @brief Códigos de erro para a plataforma Master System 
 */
#define SMS_ERROR_NONE 0                 /**< Sem erro */
#define SMS_ERROR_INVALID_PARAMETER -1   /**< Parâmetro inválido */
#define SMS_ERROR_MEMORY_ALLOCATION -2   /**< Erro de alocação de memória */
#define SMS_ERROR_FILE_NOT_FOUND -3      /**< Arquivo não encontrado */
#define SMS_ERROR_INVALID_ROM -4         /**< ROM inválida */
#define SMS_ERROR_UNSUPPORTED_MAPPER -5  /**< Mapper não suportado */
#define SMS_ERROR_INITIALIZATION -6      /**< Erro de inicialização */
#define SMS_ERROR_ALREADY_INITIALIZED -7 /**< Sistema já inicializado */
#define SMS_ERROR_NOT_INITIALIZED -8     /**< Sistema não inicializado */
#define SMS_ERROR_ROM_LOAD -9            /**< Erro ao carregar ROM */
#define SMS_ERROR_NOT_RUNNING -10        /**< Estado não em execução */

/**
 * @brief Tipos de mapper suportados pelo Master System
 */
typedef enum {
    SMS_MAPPER_SEGA = 0,    /**< Mapper padrão Sega */
    SMS_MAPPER_CODEMASTERS, /**< Mapper Codemasters */
    SMS_MAPPER_KOREAN,      /**< Mapper coreano */
    SMS_MAPPER_MSX          /**< Mapper MSX */
} sms_mapper_type_t;

/**
 * @brief Configuração do emulador Master System
 */
typedef struct {
    int32_t audio_enabled;     /**< 1 se o áudio está habilitado, 0 caso contrário */
    int32_t audio_sample_rate; /**< Taxa de amostragem do áudio (Hz) */
    int32_t video_scale;       /**< Fator de escala para o vídeo (1-4) */
    int32_t vsync_enabled;     /**< 1 se o vsync está habilitado, 0 caso contrário */
    int32_t fullscreen;        /**< 1 se em tela cheia, 0 caso contrário */
    int32_t filter_type;       /**< Tipo de filtro de vídeo */
    int32_t region;            /**< Região do console (NTSC/PAL) */
    int32_t ntsc_mode;         /**< 1 se modo NTSC, 0 se PAL */
    int32_t log_level;         /**< Nível de log (EMU_LOG_LEVEL_*) */
    const char *rom_path;      /**< Caminho para o arquivo ROM */
    int32_t system_type;       /**< 0 para Master System, 1 para Game Gear */
} sms_config_t;

/**
 * @brief Informações da ROM do Master System
 */
typedef struct {
    uint8_t *rom_data;         /**< Dados da ROM */
    size_t rom_size;           /**< Tamanho da ROM em bytes */
    sms_mapper_type_t mapper;  /**< Tipo de mapper */
    int32_t has_battery;       /**< 1 se tem bateria, 0 caso contrário */
    char title[32];            /**< Título da ROM */
    uint32_t checksum;         /**< Checksum da ROM */
    int32_t region;            /**< Região da ROM */
    int32_t system_type;       /**< 0 para Master System, 1 para Game Gear */
} sms_rom_info_t;

// Declarações de tipos para os componentes do Master System
typedef struct sms_z80_s sms_z80_t;
typedef struct sms_vdp_s sms_vdp_t;
typedef struct sms_psg_s sms_psg_t;
typedef struct sms_memory_s sms_memory_t;
typedef struct sms_input_s sms_input_t;
typedef struct sms_cartridge_s sms_cartridge_t;

/**
 * @brief Estado do sistema Master System
 */
typedef struct {
    // Informações gerais
    int32_t initialized;     /**< 1 se inicializado, 0 caso contrário */
    int32_t running;         /**< 1 se em execução, 0 caso contrário */
    int32_t frame_count;     /**< Contador de frames */
    uint64_t cycles_count;   /**< Contador de ciclos */
    sms_config_t config;     /**< Configuração atual */
    sms_rom_info_t rom_info; /**< Informações da ROM carregada */
    
    // Componentes do sistema
    sms_z80_t *cpu;             /**< Estado da CPU Z80 */
    sms_vdp_t *vdp;             /**< Estado do VDP */
    sms_psg_t *psg;             /**< Estado do PSG */
    sms_memory_t *memory;       /**< Estado da memória */
    sms_input_t *input;         /**< Estado dos controladores */
    sms_cartridge_t *cartridge; /**< Estado do cartucho */
} sms_state_t;

/**
 * @brief Estado global do sistema Master System
 * Declarado como externo para ser acessível por outros módulos
 */
extern sms_state_t g_sms_state;

/**
 * @brief Inicializa o sistema Master System
 *
 * @param config Configuração inicial (pode ser NULL para usar padrões)
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t sms_init(const sms_config_t *config);

/**
 * @brief Finaliza o sistema Master System e libera recursos
 */
void sms_shutdown(void);

/**
 * @brief Reseta o sistema Master System (similar a pressionar o botão RESET)
 *
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t sms_reset(void);

/**
 * @brief Carrega uma ROM no sistema Master System
 *
 * @param rom_path Caminho para o arquivo ROM
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t sms_load_rom(const char *rom_path);

/**
 * @brief Executa um único frame do sistema Master System
 *
 * @param frame_buffer Buffer para receber dados do frame renderizado
 * @param audio_buffer Buffer para receber dados de áudio
 * @param audio_buffer_size Tamanho do buffer de áudio
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t sms_run_frame(uint32_t *frame_buffer, int16_t *audio_buffer, int32_t audio_buffer_size);

/**
 * @brief Define o estado dos botões do controlador 1
 *
 * @param button_state Estado dos botões
 */
void sms_set_controller1(uint8_t button_state);

/**
 * @brief Define o estado dos botões do controlador 2
 *
 * @param button_state Estado dos botões
 */
void sms_set_controller2(uint8_t button_state);

/**
 * @brief Salva o estado atual do sistema Master System
 *
 * @param state_path Caminho para o arquivo de estado
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t sms_save_state(const char *state_path);

/**
 * @brief Carrega um estado salvo para o sistema Master System
 *
 * @param state_path Caminho para o arquivo de estado
 * @return int32_t Código de erro (0 para sucesso)
 */
int32_t sms_load_state(const char *state_path);

/**
 * @brief Obtém o estado atual do sistema Master System
 *
 * @return sms_state_t* Ponteiro para o estado (não modificar diretamente)
 */
const sms_state_t *sms_get_state(void);

// Constantes de ciclos por frame
#define SMS_NTSC_CYCLES_PER_FRAME 59736  // Z80 a 3.58MHz, 59.94Hz
#define SMS_PAL_CYCLES_PER_FRAME 70937   // Z80 a 3.55MHz, 50Hz

#ifdef __cplusplus
}
#endif

#endif /* MASTERSYSTEM_H */
