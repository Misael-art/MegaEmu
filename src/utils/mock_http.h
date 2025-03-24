/**
 * @file mock_http.h
 * @brief Servidor HTTP mock para testes de funcionalidades de rede
 * @version 1.0
 * @date 2025-03-18
 *
 * Este arquivo define um servidor HTTP mock simples utilizado em testes
 * unitários para simular APIs REST, serviços de nuvem e outras funcionalidades
 * de rede sem depender de serviços externos.
 */

#ifndef EMU_MOCK_HTTP_H
#define EMU_MOCK_HTTP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Resposta mock configurada
 */
typedef struct
{
    char endpoint[256];       /**< Endpoint para responder (ex: "/api/v1/auth") */
    char response_data[8192]; /**< Dados de resposta */
    int status_code;          /**< Código de status HTTP */
    int delay_ms;             /**< Delay em ms antes de responder (para testar timeout) */
    bool used;                /**< Indica se a resposta foi usada */
} mock_http_response_t;

/**
 * @brief Estrutura de servidor HTTP mock
 */
typedef struct
{
    int port;      /**< Porta do servidor */
    bool running;  /**< Indica se o servidor está rodando */
    int thread_id; /**< ID da thread do servidor */

    mock_http_response_t responses[64]; /**< Respostas configuradas */
    int response_count;                 /**< Número de respostas configuradas */

    /* Log de requisições recebidas */
    struct
    {
        char method[32];    /**< Método HTTP (GET, POST, etc) */
        char endpoint[256]; /**< Endpoint da requisição */
        char body[8192];    /**< Corpo da requisição */
        char headers[4096]; /**< Cabeçalhos da requisição */
        uint64_t timestamp; /**< Timestamp de recebimento */
    } request_log[128];
    int request_count; /**< Número de requisições recebidas */

    void *server_data; /**< Dados internos do servidor */
} mock_http_server;

/**
 * @brief Inicializa um servidor HTTP mock na porta especificada
 *
 * @param server Ponteiro para estrutura de servidor
 * @param port Porta para escutar (ex: 8080)
 * @return true se inicializado com sucesso, false caso contrário
 */
bool mock_http_server_init(mock_http_server *server, int port);

/**
 * @brief Finaliza o servidor HTTP mock
 *
 * @param server Ponteiro para estrutura de servidor
 */
void mock_http_server_shutdown(mock_http_server *server);

/**
 * @brief Adiciona uma resposta para um endpoint específico
 *
 * @param server Ponteiro para estrutura de servidor
 * @param endpoint Endpoint para responder (ex: "/api/v1/auth")
 * @param response_data Dados de resposta
 * @param status_code Código de status HTTP
 * @return true se adicionado com sucesso, false caso contrário
 */
bool mock_http_server_add_response(mock_http_server *server,
                                   const char *endpoint,
                                   const char *response_data,
                                   int status_code);

/**
 * @brief Adiciona uma resposta com delay para simular operações longas
 *
 * @param server Ponteiro para estrutura de servidor
 * @param endpoint Endpoint para responder
 * @param response_data Dados de resposta
 * @param status_code Código de status HTTP
 * @param delay_ms Delay em millisegundos
 * @return true se adicionado com sucesso, false caso contrário
 */
bool mock_http_server_add_delayed_response(mock_http_server *server,
                                           const char *endpoint,
                                           const char *response_data,
                                           int status_code,
                                           int delay_ms);

/**
 * @brief Limpa todas as respostas configuradas
 *
 * @param server Ponteiro para estrutura de servidor
 */
void mock_http_server_clear_responses(mock_http_server *server);

/**
 * @brief Limpa o histórico de requisições recebidas
 *
 * @param server Ponteiro para estrutura de servidor
 */
void mock_http_server_clear_requests(mock_http_server *server);

/**
 * @brief Verifica se uma requisição para um endpoint específico foi recebida
 *
 * @param server Ponteiro para estrutura de servidor
 * @param endpoint Endpoint para verificar (NULL para qualquer endpoint)
 * @param method Método HTTP para verificar (NULL para qualquer método)
 * @return true se recebida, false caso contrário
 */
bool mock_http_server_received_request(mock_http_server *server,
                                       const char *endpoint,
                                       const char *method);

/**
 * @brief Obtém o corpo da última requisição para um endpoint específico
 *
 * @param server Ponteiro para estrutura de servidor
 * @param endpoint Endpoint para obter (NULL para qualquer endpoint)
 * @param method Método HTTP para obter (NULL para qualquer método)
 * @param body_out Buffer para receber o corpo
 * @param max_size Tamanho máximo do buffer
 * @return true se obtido com sucesso, false caso contrário
 */
bool mock_http_server_get_request_body(mock_http_server *server,
                                       const char *endpoint,
                                       const char *method,
                                       char *body_out,
                                       size_t max_size);

/**
 * @brief Configura um callback para processar requisições dinamicamente
 *
 * @param server Ponteiro para estrutura de servidor
 * @param callback Função de callback
 * @param user_data Dados de usuário para o callback
 * @return true se configurado com sucesso, false caso contrário
 */
bool mock_http_server_set_callback(mock_http_server *server,
                                   bool (*callback)(const char *method,
                                                    const char *endpoint,
                                                    const char *body,
                                                    char *response_out,
                                                    int *status_code_out,
                                                    void *user_data),
                                   void *user_data);

/**
 * @brief Configura autenticação básica para o servidor
 *
 * @param server Ponteiro para estrutura de servidor
 * @param username Nome de usuário
 * @param password Senha
 * @return true se configurado com sucesso, false caso contrário
 */
bool mock_http_server_set_basic_auth(mock_http_server *server,
                                     const char *username,
                                     const char *password);

/**
 * @brief Configura um certificado SSL para HTTPS
 *
 * @param server Ponteiro para estrutura de servidor
 * @param cert_file Caminho para arquivo de certificado
 * @param key_file Caminho para arquivo de chave privada
 * @return true se configurado com sucesso, false caso contrário
 */
bool mock_http_server_set_ssl(mock_http_server *server,
                              const char *cert_file,
                              const char *key_file);

/**
 * @brief Configura uma resposta de erro para requisições não mapeadas
 *
 * @param server Ponteiro para estrutura de servidor
 * @param response_data Dados de resposta
 * @param status_code Código de status HTTP
 * @return true se configurado com sucesso, false caso contrário
 */
bool mock_http_server_set_default_response(mock_http_server *server,
                                           const char *response_data,
                                           int status_code);

#endif /* EMU_MOCK_HTTP_H */
