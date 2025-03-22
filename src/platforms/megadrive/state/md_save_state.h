/**
 * @file md_save_state.h
 * @brief Sistema de save state para o Mega Drive
 *
 * Este arquivo contém as definições e estruturas para o sistema de save state
 * específico para o emulador do Mega Drive.
 *
 * @version 1.3.0
 * @date 2025-04-01
 */

#ifndef MD_SAVE_STATE_H
#define MD_SAVE_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "core/save_state.h"

/**
 * @brief Códigos de erro específicos para o sistema de save state do Mega Drive
 */
#define MD_SAVE_STATE_ERROR_NONE 0        /**< Nenhum erro */
#define MD_SAVE_STATE_ERROR_INIT -1       /**< Erro de inicialização */
#define MD_SAVE_STATE_ERROR_INVALID -2    /**< Parâmetro inválido */
#define MD_SAVE_STATE_ERROR_ROM -3        /**< Erro relacionado à ROM */
#define MD_SAVE_STATE_ERROR_VERSION -4    /**< Versão incompatível */
#define MD_SAVE_STATE_ERROR_FILE -5       /**< Erro de leitura/escrita de arquivo */
#define MD_SAVE_STATE_ERROR_MEMORY -6     /**< Erro de alocação de memória */
#define MD_SAVE_STATE_ERROR_COMPRESS -7   /**< Erro de compressão */
#define MD_SAVE_STATE_ERROR_DECOMPRESS -8 /**< Erro de descompressão */
#define MD_SAVE_STATE_ERROR_FORMAT -9     /**< Formato de arquivo inválido */
#define MD_SAVE_STATE_ERROR_CHECKSUM -10  /**< Falha na verificação de checksum */

/**
 * @brief Estrutura para a configuração do sistema de rewind
 */
typedef struct
{
    uint32_t capacity;            /**< Capacidade total do buffer em snapshots */
    uint32_t frames_per_snapshot; /**< Número de frames entre cada snapshot */
    uint32_t head;                /**< Posição atual de gravação no buffer */
    uint32_t tail;                /**< Posição mais antiga no buffer */
    uint32_t count;               /**< Número atual de snapshots no buffer */
    save_state_t *snapshots;      /**< Buffer de snapshots */
    bool enabled;                 /**< Sistema de rewind habilitado? */
} md_rewind_state_t;

/**
 * @brief Inicializa o sistema de save state do Mega Drive
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 * @return true se inicializado com sucesso, false caso contrário
 */
bool md_save_state_init(void *platform);

/**
 * @brief Finaliza o sistema de save state do Mega Drive
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 */
void md_save_state_shutdown(void *platform);

/**
 * @brief Salva o estado atual em um arquivo
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 * @param filename Nome do arquivo para salvar
 * @param screenshot_data Dados da captura de tela (pode ser NULL)
 * @param width Largura da captura de tela
 * @param height Altura da captura de tela
 * @param stride Stride da captura de tela
 * @return true se salvo com sucesso, false caso contrário
 */
bool md_save_state_save_file(void *platform, const char *filename,
                             const uint8_t *screenshot_data, uint32_t width,
                             uint32_t height, uint32_t stride);

/**
 * @brief Carrega um estado a partir de um arquivo
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 * @param filename Nome do arquivo para carregar
 * @return true se carregado com sucesso, false caso contrário
 */
bool md_save_state_load_file(void *platform, const char *filename);

/**
 * @brief Configura o sistema de rewind
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 * @param capacity Capacidade em número de snapshots
 * @param frames_per_snapshot Número de frames entre cada snapshot
 * @return true se configurado com sucesso, false caso contrário
 */
bool md_save_state_config_rewind(void *platform, uint32_t capacity,
                                 uint32_t frames_per_snapshot);

/**
 * @brief Captura um snapshot para o sistema de rewind
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 * @return true se capturado com sucesso, false caso contrário
 */
bool md_save_state_capture_rewind(void *platform);

/**
 * @brief Aplica rewind de um estado (retrocede o emulador)
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 * @param steps Número de passos para retroceder (negativo) ou avançar (positivo)
 * @return true se aplicado com sucesso, false caso contrário
 */
bool md_save_state_apply_rewind(void *platform, int32_t steps);

/**
 * @brief Habilita ou desabilita o sistema de rewind
 *
 * @param platform Ponteiro para a plataforma Mega Drive
 * @param enabled true para habilitar, false para desabilitar
 * @return true se alterado com sucesso, false caso contrário
 */
bool md_save_state_enable_rewind(void *platform, bool enabled);

/**
 * @brief Registra o CPU M68K no sistema de save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param m68k Ponteiro para a estrutura do M68K
 * @return Código de erro
 */
int32_t m68k_register_save_state(save_state_t *state, void *m68k);

/**
 * @brief Restaura o CPU M68K a partir de um save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param m68k Ponteiro para a estrutura do M68K
 * @return Código de erro
 */
int32_t m68k_restore_save_state(save_state_t *state, void *m68k);

/**
 * @brief Registra o CPU Z80 no sistema de save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param z80 Ponteiro para a estrutura do Z80
 * @return Código de erro
 */
int32_t z80_register_save_state(save_state_t *state, void *z80);

/**
 * @brief Restaura o CPU Z80 a partir de um save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param z80 Ponteiro para a estrutura do Z80
 * @return Código de erro
 */
int32_t z80_restore_save_state(save_state_t *state, void *z80);

/**
 * @brief Registra o VDP no sistema de save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param vdp Ponteiro para a estrutura do VDP
 * @return Código de erro
 */
int32_t vdp_register_save_state(save_state_t *state, void *vdp);

/**
 * @brief Restaura o VDP a partir de um save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param vdp Ponteiro para a estrutura do VDP
 * @return Código de erro
 */
int32_t vdp_restore_save_state(save_state_t *state, void *vdp);

/**
 * @brief Aplica efeito de escala de cinza ao framebuffer do VDP
 *
 * @param vdp Ponteiro para a estrutura do VDP
 * @return Código de erro
 */
int32_t vdp_apply_grayscale_effect(void *vdp);

/**
 * @brief Registra o PSG no sistema de save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param psg Ponteiro para a estrutura do PSG
 * @return Código de erro
 */
int32_t psg_register_save_state(save_state_t *state, void *psg);

/**
 * @brief Restaura o PSG a partir de um save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param psg Ponteiro para a estrutura do PSG
 * @return Código de erro
 */
int32_t psg_restore_save_state(save_state_t *state, void *psg);

/**
 * @brief Registra o YM2612 (FM) no sistema de save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param ym2612 Ponteiro para a estrutura do YM2612
 * @return Código de erro
 */
int32_t ym2612_register_save_state(save_state_t *state, void *ym2612);

/**
 * @brief Restaura o YM2612 (FM) a partir de um save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param ym2612 Ponteiro para a estrutura do YM2612
 * @return Código de erro
 */
int32_t ym2612_restore_save_state(save_state_t *state, void *ym2612);

/**
 * @brief Registra o sistema de IO no sistema de save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param io Ponteiro para a estrutura do IO
 * @return Código de erro
 */
int32_t io_register_save_state(save_state_t *state, void *io);

/**
 * @brief Restaura o sistema de IO a partir de um save state
 *
 * @param state Ponteiro para a estrutura de save state
 * @param io Ponteiro para a estrutura do IO
 * @return Código de erro
 */
int32_t io_restore_save_state(save_state_t *state, void *io);

#endif /* MD_SAVE_STATE_H */
