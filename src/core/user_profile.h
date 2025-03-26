/**
 * @file user_profile.h
 * @brief Gerenciamento de perfis de usuário para o emulador
 */

#ifndef USER_PROFILE_H
#define USER_PROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "save_state.h"

// Códigos de erro específicos para perfis
#define PROFILE_ERROR_NONE 0
#define PROFILE_ERROR_DATABASE -1
#define PROFILE_ERROR_INVALID_PARAM -2
#define PROFILE_ERROR_USER_EXISTS -3
#define PROFILE_ERROR_USER_NOT_FOUND -4
#define PROFILE_ERROR_AUTH_FAILED -5
#define PROFILE_ERROR_INVALID_TOKEN -6
#define PROFILE_ERROR_NETWORK -7
#define PROFILE_ERROR_CLOUD_SYNC -8
#define PROFILE_ERROR_MEMORY -9

    // Configurações de privacidade
    typedef enum
    {
        PROFILE_PRIVACY_PUBLIC,  // Visível para todos
        PROFILE_PRIVACY_FRIENDS, // Visível apenas para amigos
        PROFILE_PRIVACY_PRIVATE  // Visível apenas para o usuário
    } profile_privacy_t;

    // Configurações de nuvem
    typedef enum
    {
        PROFILE_CLOUD_NONE,     // Sem integração com nuvem
        PROFILE_CLOUD_GOOGLE,   // Google Drive
        PROFILE_CLOUD_ONEDRIVE, // Microsoft OneDrive
        PROFILE_CLOUD_DROPBOX   // Dropbox
    } profile_cloud_service_t;

    // Estatísticas do perfil
    typedef struct
    {
        uint32_t total_play_time;       // Tempo total de jogo em segundos
        uint32_t games_played;          // Número de jogos diferentes jogados
        uint32_t save_states_created;   // Número de save states criados
        uint32_t save_states_loaded;    // Número de save states carregados
        uint32_t achievements_unlocked; // Número de conquistas desbloqueadas
        uint32_t total_launches;        // Número total de vezes que o emulador foi iniciado
        uint64_t last_active_timestamp; // Último timestamp de atividade
    } profile_stats_t;

    // Configurações de cloud sync
    typedef struct
    {
        profile_cloud_service_t service; // Serviço de nuvem a ser usado
        char auth_token[256];            // Token de autenticação
        char refresh_token[256];         // Token de refresh
        char folder_path[256];           // Caminho da pasta remota
        bool auto_sync;                  // Sincronização automática
        bool sync_on_exit;               // Sincronizar ao sair
        bool sync_screenshots;           // Sincronizar screenshots
        uint32_t sync_interval;          // Intervalo de sincronização em minutos (0 = manual)
    } profile_cloud_config_t;

    // Configurações de compartilhamento social
    typedef struct
    {
        bool share_enabled;                // Habilitar compartilhamento
        profile_privacy_t default_privacy; // Privacidade padrão para compartilhamentos
        bool auto_share_achievements;      // Compartilhar conquistas automaticamente
        char twitter_token[128];           // Token para Twitter
        char facebook_token[128];          // Token para Facebook
        char discord_webhook[256];         // Webhook para Discord
    } profile_social_config_t;

    // Estrutura completa de perfil de usuário
    typedef struct
    {
        char username[64];              // Nome de usuário
        char display_name[128];         // Nome de exibição
        char email[128];                // E-mail do usuário
        char password_hash[64];         // Hash da senha (SHA-256)
        char avatar_path[256];          // Caminho para o avatar
        uint64_t created_timestamp;     // Data de criação
        uint64_t last_login_timestamp;  // Último login
        profile_stats_t stats;          // Estatísticas do perfil
        profile_cloud_config_t cloud;   // Configurações de nuvem
        profile_social_config_t social; // Configurações sociais
        bool is_active;                 // Se é o perfil ativo
    } user_profile_t;

    /**
     * @brief Inicializa o sistema de perfis de usuário
     *
     * @param database_path Caminho para o banco de dados SQLite
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_init(const char *database_path);

    /**
     * @brief Finaliza o sistema de perfis de usuário
     */
    void profile_shutdown(void);

    /**
     * @brief Cria um novo perfil de usuário
     *
     * @param username Nome de usuário
     * @param display_name Nome de exibição
     * @param email E-mail do usuário
     * @param password Senha (não criptografada)
     * @param avatar_path Caminho para o avatar (opcional, pode ser NULL)
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_create(
        const char *username,
        const char *display_name,
        const char *email,
        const char *password,
        const char *avatar_path);

    /**
     * @brief Autentica um usuário
     *
     * @param username Nome de usuário
     * @param password Senha (não criptografada)
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_authenticate(
        const char *username,
        const char *password);

    /**
     * @brief Carrega informações do perfil ativo
     *
     * @param profile Ponteiro para receber as informações
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_get_active(user_profile_t *profile);

    /**
     * @brief Define o perfil ativo
     *
     * @param username Nome de usuário
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_set_active(const char *username);

    /**
     * @brief Atualiza as informações do perfil
     *
     * @param profile Informações atualizadas
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_update(const user_profile_t *profile);

    /**
     * @brief Remove um perfil
     *
     * @param username Nome de usuário
     * @param password Senha (para confirmar)
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_delete(
        const char *username,
        const char *password);

    /**
     * @brief Configura a integração com nuvem
     *
     * @param username Nome de usuário
     * @param config Configurações de nuvem
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_configure_cloud(
        const char *username,
        const profile_cloud_config_t *config);

    /**
     * @brief Inicia sincronização com a nuvem
     *
     * @param upload_only Se true, apenas faz upload dos arquivos locais
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_sync_with_cloud(bool upload_only);

    /**
     * @brief Configura compartilhamento social
     *
     * @param username Nome de usuário
     * @param config Configurações sociais
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_configure_social(
        const char *username,
        const profile_social_config_t *config);

    /**
     * @brief Compartilha um save state em redes sociais
     *
     * @param save_file Caminho para o arquivo de save state
     * @param message Mensagem a ser compartilhada
     * @param privacy Nível de privacidade
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_share_save_state(
        const char *save_file,
        const char *message,
        profile_privacy_t privacy);

    /**
     * @brief Atualiza as estatísticas do perfil
     *
     * @param stats Novas estatísticas
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_update_stats(const profile_stats_t *stats);

    /**
     * @brief Obtém a lista de perfis disponíveis
     *
     * @param profiles Array para receber os perfis
     * @param max_count Tamanho máximo do array
     * @param actual_count Ponteiro para receber o número real de perfis
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_get_all(
        user_profile_t *profiles,
        uint32_t max_count,
        uint32_t *actual_count);

    /**
     * @brief Verifica se um nome de usuário está disponível
     *
     * @param username Nome de usuário
     * @return bool true se disponível, false caso contrário
     */
    bool profile_is_username_available(const char *username);

    /**
     * @brief Altera a senha de um perfil
     *
     * @param username Nome de usuário
     * @param old_password Senha atual
     * @param new_password Nova senha
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_change_password(
        const char *username,
        const char *old_password,
        const char *new_password);

    /**
     * @brief Integra o perfil com o save state
     *
     * Adiciona informações do perfil ao save state e atualiza estatísticas.
     *
     * @param state Ponteiro para o contexto de save state
     * @return int32_t PROFILE_ERROR_NONE em caso de sucesso, código de erro caso contrário
     */
    int32_t profile_integrate_with_save_state(save_state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* USER_PROFILE_H */
