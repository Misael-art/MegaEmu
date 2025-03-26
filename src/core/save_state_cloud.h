/**
 * @file save_state_cloud.h
 * @brief API para integração de save states com serviços de nuvem
 * @version 1.0
 * @date 2025-03-18
 *
 * Este arquivo define a API pública para a integração de save states
 * com serviços de nuvem, permitindo sincronização, backup e restauração
 * automática de save states entre diferentes dispositivos.
 */

#ifndef EMU_SAVE_STATE_CLOUD_H
#define EMU_SAVE_STATE_CLOUD_H

#include <stdint.h>
#include <stdbool.h>
#include "save_state.h"

/**
 * @brief Provedores de nuvem suportados
 */
typedef enum {
    EMU_CLOUD_NONE = 0,      /**< Sem provedor de nuvem */
    EMU_CLOUD_GOOGLE_DRIVE,  /**< Google Drive */
    EMU_CLOUD_ONEDRIVE,      /**< Microsoft OneDrive */
    EMU_CLOUD_DROPBOX,       /**< Dropbox */
    EMU_CLOUD_CUSTOM         /**< Implementação personalizada */
} emu_cloud_provider_t;

/**
 * @brief Estratégias de resolução de conflitos de sincronização
 */
typedef enum {
    EMU_CLOUD_CONFLICT_ASK = 0,  /**< Perguntar ao usuário */
    EMU_CLOUD_CONFLICT_LOCAL,    /**< Usar versão local */
    EMU_CLOUD_CONFLICT_REMOTE,   /**< Usar versão remota */
    EMU_CLOUD_CONFLICT_NEWER,    /**< Usar versão mais recente */
    EMU_CLOUD_CONFLICT_MERGE     /**< Tentar mesclar (quando possível) */
} emu_cloud_conflict_t;

/**
 * @brief Status de sincronização
 */
typedef enum {
    EMU_CLOUD_SYNC_UNKNOWN = 0,  /**< Status desconhecido */
    EMU_CLOUD_SYNC_SYNCED,       /**< Sincronizado */
    EMU_CLOUD_SYNC_LOCAL_NEWER,  /**< Versão local mais recente */
    EMU_CLOUD_SYNC_REMOTE_NEWER, /**< Versão remota mais recente */
    EMU_CLOUD_SYNC_CONFLICT,     /**< Conflito detectado */
    EMU_CLOUD_SYNC_ERROR         /**< Erro na sincronização */
} emu_cloud_sync_status_t;

/**
 * @brief Informações sobre arquivo na nuvem
 */
typedef struct {
    char filename[256];       /**< Nome do arquivo */
    char remote_path[512];    /**< Caminho remoto */
    uint64_t size;            /**< Tamanho em bytes */
    uint64_t timestamp;       /**< Timestamp de modificação */
    uint32_t platform_id;     /**< ID da plataforma */
    char game_title[128];     /**< Título do jogo */
    bool has_thumbnail;       /**< Indica se possui thumbnail */
    bool is_encrypted;        /**< Indica se está criptografado */
} emu_cloud_file_info_t;

/**
 * @brief Callbacks para implementação personalizada de provedor de nuvem
 */
typedef bool (*emu_cloud_upload_callback_t)(const char* local_path, const char* remote_path, void* user_data);
typedef bool (*emu_cloud_download_callback_t)(const char* remote_path, const char* local_path, void* user_data);
typedef bool (*emu_cloud_list_callback_t)(const char* remote_path, void* user_data, char* results, size_t max_results);
typedef bool (*emu_cloud_timestamp_callback_t)(const char* remote_path, void* user_data, uint64_t* timestamp);
typedef bool (*emu_cloud_conflict_callback_t)(const char* local_path, const char* remote_path, void* user_data, emu_cloud_conflict_t* resolution);

/**
 * @brief Configuração de integração com nuvem
 */
typedef struct {
    emu_cloud_provider_t provider;     /**< Provedor de nuvem */
    char auth_token[512];              /**< Token de autorização */
    char refresh_token[512];           /**< Token de atualização */
    uint64_t token_expiry;             /**< Timestamp de expiração do token */
    char folder_path[256];             /**< Caminho da pasta remota */
    bool auto_sync;                    /**< Sincronização automática */
    uint32_t auto_sync_interval;       /**< Intervalo de sincronização (em segundos) */
    emu_cloud_conflict_t conflict_strategy; /**< Estratégia de resolução de conflitos */

    /* Callbacks para implementação personalizada */
    emu_cloud_upload_callback_t custom_upload;       /**< Callback de upload */
    emu_cloud_download_callback_t custom_download;   /**< Callback de download */
    emu_cloud_list_callback_t custom_list;           /**< Callback de listagem */
    emu_cloud_timestamp_callback_t custom_timestamp; /**< Callback de timestamp */
    emu_cloud_conflict_callback_t custom_conflict;   /**< Callback de conflito */
    void* user_data;                                 /**< Dados do usuário para callbacks */

    /* Opções avançadas */
    bool encrypt_cloud;       /**< Criptografar antes de enviar para nuvem */
    bool log_operations;      /**< Registrar operações em log */
    char cache_dir[256];      /**< Diretório de cache */
    uint32_t max_cache_size;  /**< Tamanho máximo de cache (MB) */
} emu_cloud_config_t;

/**
 * @brief Inicializa subsistema de nuvem
 *
 * @return true se inicializado com sucesso, false caso contrário
 */
bool emu_cloud_init(void);

/**
 * @brief Finaliza subsistema de nuvem
 */
void emu_cloud_shutdown(void);

/**
 * @brief Configura integração com nuvem para um contexto de save state
 *
 * @param state Contexto de save state
 * @param config Configuração de nuvem
 * @return true se configurado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_configure(emu_save_state_t* state, const emu_cloud_config_t* config);

/**
 * @brief Obtém configuração atual de nuvem
 *
 * @param state Contexto de save state
 * @param config Buffer para receber configuração
 * @return true se obtida com sucesso, false caso contrário
 */
bool emu_save_state_cloud_get_config(emu_save_state_t* state, emu_cloud_config_t* config);

/**
 * @brief Sincroniza um save state com a nuvem
 *
 * @param state Contexto de save state
 * @param local_path Caminho local do arquivo
 * @param force_upload Força upload mesmo se versão local for mais antiga
 * @return true se sincronizado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_sync(emu_save_state_t* state, const char* local_path, bool force_upload);

/**
 * @brief Verifica status de sincronização de um save state
 *
 * @param state Contexto de save state
 * @param local_path Caminho local do arquivo
 * @param status Buffer para receber status
 * @return true se verificado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_get_status(emu_save_state_t* state, const char* local_path, emu_cloud_sync_status_t* status);

/**
 * @brief Lista arquivos de save state na nuvem
 *
 * @param state Contexto de save state
 * @param files Buffer para receber informações dos arquivos
 * @param max_files Número máximo de arquivos a listar
 * @param num_files Número de arquivos encontrados
 * @return true se listado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_list(emu_save_state_t* state, emu_cloud_file_info_t* files, size_t max_files, size_t* num_files);

/**
 * @brief Verifica atualizações na nuvem para saves locais
 *
 * @param state Contexto de save state
 * @param local_dir Diretório local a verificar
 * @param num_changes Número de mudanças detectadas
 * @return true se verificado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_check_updates(emu_save_state_t* state, const char* local_dir, size_t* num_changes);

/**
 * @brief Resolve um conflito de sincronização
 *
 * @param state Contexto de save state
 * @param local_path Caminho local do arquivo
 * @param resolution Estratégia de resolução
 * @return true se resolvido com sucesso, false caso contrário
 */
bool emu_save_state_cloud_resolve_conflict(emu_save_state_t* state, const char* local_path, emu_cloud_conflict_t resolution);

/**
 * @brief Configura backup automático para nuvem
 *
 * @param state Contexto de save state
 * @param enable Habilitar ou desabilitar
 * @param interval Intervalo em segundos (0 para padrão)
 * @return true se configurado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_configure_auto_backup(emu_save_state_t* state, bool enable, uint32_t interval);

/**
 * @brief Verifica se há operação de nuvem em andamento
 *
 * @param state Contexto de save state
 * @param progress Progresso atual (0-100), pode ser NULL
 * @return true se ocupado, false se ocioso
 */
bool emu_save_state_cloud_is_busy(emu_save_state_t* state, int* progress);

/**
 * @brief Cancela operação de nuvem em andamento
 *
 * @param state Contexto de save state
 * @return true se cancelado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_cancel_operation(emu_save_state_t* state);

/**
 * @brief Obtém URL de autenticação para um provedor
 *
 * @param provider Provedor de nuvem
 * @param url_buffer Buffer para receber URL
 * @param buffer_size Tamanho do buffer
 * @return true se obtido com sucesso, false caso contrário
 */
bool emu_cloud_get_auth_url(emu_cloud_provider_t provider, char* url_buffer, size_t buffer_size);

/**
 * @brief Autentica com um provedor usando código OAuth
 *
 * @param provider Provedor de nuvem
 * @param auth_code Código de autorização
 * @param tokens_out Buffer para receber tokens (pode ser NULL)
 * @return true se autenticado com sucesso, false caso contrário
 */
bool emu_cloud_authenticate(emu_cloud_provider_t provider, const char* auth_code, void* tokens_out);

/**
 * @brief Atualiza token de autorização expirado
 *
 * @param provider Provedor de nuvem
 * @param refresh_token Token de atualização
 * @param tokens_out Buffer para receber novos tokens (pode ser NULL)
 * @return true se atualizado com sucesso, false caso contrário
 */
bool emu_cloud_refresh_auth(emu_cloud_provider_t provider, const char* refresh_token, void* tokens_out);

/**
 * @brief Obtém informações detalhadas sobre um arquivo na nuvem
 *
 * @param state Contexto de save state
 * @param remote_path Caminho remoto do arquivo
 * @param info Buffer para receber informações
 * @return true se obtido com sucesso, false caso contrário
 */
bool emu_save_state_cloud_get_file_info(emu_save_state_t* state, const char* remote_path, emu_cloud_file_info_t* info);

/**
 * @brief Baixa thumbnail de um save state da nuvem
 *
 * @param state Contexto de save state
 * @param remote_path Caminho remoto do arquivo
 * @param buffer Buffer para receber dados da thumbnail
 * @param buffer_size Tamanho do buffer
 * @param bytes_written Bytes escritos no buffer
 * @return true se baixado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_download_thumbnail(emu_save_state_t* state, const char* remote_path,
                                          void* buffer, size_t buffer_size, size_t* bytes_written);

/**
 * @brief Define callback de progresso para operações de nuvem
 *
 * @param state Contexto de save state
 * @param callback Função de callback para progresso (NULL para desabilitar)
 * @param user_data Dados passados para o callback
 * @return true se configurado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_set_progress_callback(emu_save_state_t* state,
                                             void (*callback)(int progress, const char* operation, void* user_data),
                                             void* user_data);

/**
 * @brief Limpa cache local de arquivos da nuvem
 *
 * @param state Contexto de save state
 * @return true se limpado com sucesso, false caso contrário
 */
bool emu_save_state_cloud_clear_cache(emu_save_state_t* state);

#endif /* EMU_SAVE_STATE_CLOUD_H */
