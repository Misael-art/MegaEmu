/** * @file nes_save_state.h * @brief Sistema de save states para o emulador NES */ #ifndef NES_SAVE_STATE_H #define NES_SAVE_STATE_H #include<stdint.h> #include<stdbool.h> #include "../cpu/nes_cpu.h" #include "../ppu/nes_ppu.h" #include "../apu/nes_apu.h" #include "../memory/nes_memory.h" #include "../../../core/save_state.h" #include "../../../core/rewind_buffer.h" #ifdef __cplusplusextern "C" {#endif /** * @brief Versão atual do formato de save state */ #define NES_SAVE_STATE_VERSION 2 /** * @brief Códigos de erro para operações de save state */ #define NES_SAVE_STATE_ERROR_NONE 0 #define NES_SAVE_STATE_ERROR_FILE - 1 #define NES_SAVE_STATE_ERROR_VERSION - 2 #define NES_SAVE_STATE_ERROR_INVALID - 3 #define NES_SAVE_STATE_ERROR_MEMORY - 4 #define NES_SAVE_STATE_ERROR_COMPRESSION - 5 #define NES_SAVE_STATE_ERROR_THUMBNAIL - 6 #define NES_SAVE_STATE_ERROR_REWIND - 7 /** * @brief Estrutura para metadados expandidos do save state */ typedef struct {char game_title[128]; /**< Título do jogo */ char game_region[16]; /**< Região do jogo (NTSC/PAL) */ char emulator_version[32]; /**< Versão do emulador */ char description[256]; /**< Descrição do save state */ char tags[128]; /**< Tags para categorização */ char user_notes[512]; /**< Notas do usuário */ uint32_t game_time_seconds; /**< Tempo de jogo em segundos */ uint32_t save_count; /**< Número de saves realizados */ uint32_t load_count; /**< Número de loads realizados */} nes_save_state_metadata_t; /** * @brief Estrutura que contém o estado completo do emulador */ typedef struct {uint32_t version; /**< Versão do formato do save state */ uint32_t timestamp; /**< Timestamp da criação do save state */ char rom_hash[33]; /**< Hash MD5 da ROM para validação */
// Metadados expandidos
nes_save_state_metadata_t metadata;

// Estados dos componentes
struct
{
    uint8_t a, x, y, s, p; /**< Registradores do CPU */
    uint16_t pc;           /**< Program Counter */
    uint32_t cycles;       /**< Ciclos totais */
    uint8_t irq_pending;   /**< Estado de interrupções */
    uint8_t nmi_pending;
} cpu_state;
struct
{
    uint8_t control; /**< Registradores da PPU */
    uint8_t mask;
    uint8_t status;
    uint8_t oam_addr;
    uint16_t v;               /**< Registrador interno V */
    uint16_t t;               /**< Registrador interno T */
    uint8_t x;                /**< Fine X scroll */
    uint8_t w;                /**< Write toggle */
    uint8_t buffer;           /**< Read buffer */
    uint8_t nametables[2048]; /**< Dados das nametables */
    uint8_t palette[32];      /**< Dados da paleta */
    uint8_t oam[256];         /**< Object Attribute Memory */
} ppu_state;
struct
{
    uint8_t pulse1_regs[4]; /**< Registradores dos canais */
    uint8_t pulse2_regs[4];
    uint8_t triangle_regs[4];
    uint8_t noise_regs[4];
    uint8_t dmc_regs[4];
    uint8_t status;        /**< Status do APU */
    uint8_t frame_counter; /**< Frame counter */
} apu_state;
struct
{
    uint8_t ram[2048];  /**< RAM interna */
    uint8_t sram[8192]; /**< SRAM do cartucho */
} memory_state;         // Informações do cartucho        struct        {            uint8_t mapper_number;     /**< Número do mapper */            uint8_t mapper_state[256]; /**< Estado do mapper (aumentado para suportar mappers complexos) */        } cart_state;

// Thumbnail
struct
{
    bool has_thumbnail; /**< Indica se há thumbnail */
    uint32_t width;     /**< Largura da thumbnail */
    uint32_t height;    /**< Altura da thumbnail */
    uint32_t data_size; /**< Tamanho dos dados comprimidos */
    uint8_t data[0];    /**< Dados da thumbnail (tamanho variável) */
} thumbnail;
}
nes_save_state_t; /** * @brief Configuração para o sistema de save states do NES */
typedef struct
{
    bool use_compression;                                                                     /**< Usar compressão delta */
    bool include_thumbnail;                                                                   /**< Incluir thumbnail nos saves */
    uint32_t thumbnail_quality;                                                               /**< Qualidade da thumbnail (0-100) */
    bool enable_rewind;                                                                       /**< Habilitar sistema de rewind */
    uint32_t rewind_frames;                                                                   /**< Número de frames para rewind */
    uint32_t frames_per_snapshot;                                                             /**< Frames entre cada snapshot de rewind */
    bool autosave_enabled;                                                                    /**< Habilitar autosave */
    uint32_t autosave_interval;                                                               /**< Intervalo de autosave em minutos */
    char autosave_path[256];                                                                  /**< Caminho para salvar autosaves */
} nes_save_state_config_t;                                                                    /** * @brief Inicializa o sistema de save states */
int32_t nes_save_state_init(void);                                                            /** * @brief Finaliza o sistema de save states */
void nes_save_state_shutdown(void);                                                           /** * @brief Salva o estado atual do emulador em um arquivo */
int32_t nes_save_state_save(const char *filename, const char *description, const char *tags); /** * @brief Carrega um estado salvo de um arquivo */
int32_t nes_save_state_load(const char *filename);                                            /** * @brief Verifica se um arquivo de save state é válido */
bool nes_save_state_validate(const char *filename);                                           /** * @brief Define a configuração do sistema de save states */
int32_t nes_save_state_set_config(const nes_save_state_config_t *config);                     /** * @brief Obtém a configuração atual do sistema de save states */
int32_t nes_save_state_get_config(nes_save_state_config_t *config);                           /** * @brief Captura um snapshot para o sistema de rewind */
int32_t nes_save_state_capture_rewind(void);                                                  /** * @brief Aplica um passo de rewind */
int32_t nes_save_state_rewind_step(void);                                                     /** * @brief Inicia o efeito visual de rewind */
int32_t nes_save_state_start_rewind_effect(uint32_t seconds_back);                            /** * @brief Exporta um save state para outro formato */
int32_t nes_save_state_export(const char *source_filename, const char *target_filename);      /** * @brief Importa um save state de outro formato */
int32_t nes_save_state_import(const char *source_filename, const char *target_filename);      /** * @brief Obtém a thumbnail de um save state */
int32_t nes_save_state_get_thumbnail(const char *filename, uint8_t **data,
                                     uint32_t *size, uint32_t *width, uint32_t *height); /** * @brief Cria um ponto de auto-save */
int32_t nes_save_state_create_autosave(void);                                            /** * @brief Registra o estado de um mapper específico */
int32_t nes_save_state_register_mapper(uint8_t mapper_id, void *state, uint32_t state_size);
#ifdef __cplusplus
}
#endif #endif // NES_SAVE_STATE_H
