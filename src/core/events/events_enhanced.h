/**
 * @file events_enhanced.h
 * @brief Interface aprimorada para eventos com suporte a agendamento diferido e sincronização precisa
 */

#ifndef EVENTS_ENHANCED_H
#define EVENTS_ENHANCED_H

#include "events_interface.h"
#include "priority_queue.h"

/**
 * @brief Tipo para timestamp unificado do sistema
 */
typedef uint64_t emu_timestamp_t;

/**
 * @brief Sistema de eventos aprimorado
 */
typedef struct
{
    emu_event_priority_queue_t *queue; /**< Fila de prioridade para eventos */
    bool paused;                       /**< Se o sistema de eventos está pausado */
    emu_timestamp_t current_time;      /**< Timestamp atual do sistema */
    float system_load;                 /**< Carga atual do sistema (0.0 - 1.0) */
    uint32_t events_per_second;        /**< Taxa de eventos por segundo */
    uint32_t max_events_per_frame;     /**< Número máximo de eventos processados por frame */
} emu_events_enhanced_t;

/**
 * @brief Inicializa o sistema de eventos aprimorado
 * @param events Ponteiro para o sistema de eventos
 * @param queue_capacity Capacidade inicial da fila de eventos
 * @return true se a inicialização foi bem-sucedida, false caso contrário
 */
bool emu_events_enhanced_init(emu_events_enhanced_t *events, uint32_t queue_capacity);

/**
 * @brief Finaliza o sistema de eventos aprimorado
 * @param events Ponteiro para o sistema de eventos
 */
void emu_events_enhanced_shutdown(emu_events_enhanced_t *events);

/**
 * @brief Agenda um evento para execução com prioridade e atraso especificados
 * @param events Ponteiro para o sistema de eventos
 * @param type Tipo do evento
 * @param data Dados do evento
 * @param data_size Tamanho dos dados
 * @param priority Prioridade do evento
 * @param delay_ms Atraso em milissegundos (0 para execução imediata)
 * @return true se o evento foi agendado com sucesso, false caso contrário
 */
bool emu_events_enhanced_schedule(emu_events_enhanced_t *events,
                                  emu_event_type_t type,
                                  void *data,
                                  size_t data_size,
                                  emu_event_priority_t priority,
                                  uint64_t delay_ms);

/**
 * @brief Agenda um evento periódico com intervalo especificado
 * @param events Ponteiro para o sistema de eventos
 * @param type Tipo do evento
 * @param data Dados do evento
 * @param data_size Tamanho dos dados
 * @param priority Prioridade do evento
 * @param interval_ms Intervalo entre execuções em milissegundos
 * @param count Número de execuções (0 para infinito)
 * @return ID do evento periódico ou 0 em caso de erro
 */
uint32_t emu_events_enhanced_schedule_periodic(emu_events_enhanced_t *events,
                                               emu_event_type_t type,
                                               void *data,
                                               size_t data_size,
                                               emu_event_priority_t priority,
                                               uint64_t interval_ms,
                                               uint32_t count);

/**
 * @brief Cancela um evento agendado
 * @param events Ponteiro para o sistema de eventos
 * @param type Tipo do evento a ser cancelado
 * @param data Dados específicos para correspondência (NULL para ignorar)
 * @return true se algum evento foi cancelado, false caso contrário
 */
bool emu_events_enhanced_cancel(emu_events_enhanced_t *events,
                                emu_event_type_t type,
                                void *data);

/**
 * @brief Cancela um evento periódico pelo ID
 * @param events Ponteiro para o sistema de eventos
 * @param periodic_id ID do evento periódico
 * @return true se o evento foi cancelado, false caso contrário
 */
bool emu_events_enhanced_cancel_periodic(emu_events_enhanced_t *events,
                                         uint32_t periodic_id);

/**
 * @brief Processa eventos pendentes
 * @param events Ponteiro para o sistema de eventos
 * @param timestamp_ms Timestamp atual em milissegundos
 * @param max_events Número máximo de eventos a processar (0 para processar todos)
 * @return Número de eventos processados
 */
uint32_t emu_events_enhanced_process(emu_events_enhanced_t *events,
                                     uint64_t timestamp_ms,
                                     uint32_t max_events);

/**
 * @brief Pausa o processamento de eventos
 * @param events Ponteiro para o sistema de eventos
 */
void emu_events_enhanced_pause(emu_events_enhanced_t *events);

/**
 * @brief Retoma o processamento de eventos
 * @param events Ponteiro para o sistema de eventos
 */
void emu_events_enhanced_resume(emu_events_enhanced_t *events);

/**
 * @brief Verifica se o sistema de eventos está pausado
 * @param events Ponteiro para o sistema de eventos
 * @return true se o sistema está pausado, false caso contrário
 */
bool emu_events_enhanced_is_paused(emu_events_enhanced_t *events);

/**
 * @brief Registra um callback para um tipo de evento
 * @param events Ponteiro para o sistema de eventos
 * @param type Tipo do evento
 * @param callback Função de callback
 * @param userdata Dados de usuário a serem passados para o callback
 * @return ID do registro ou 0 em caso de erro
 */
uint32_t emu_events_enhanced_register_callback(emu_events_enhanced_t *events,
                                               emu_event_type_t type,
                                               emu_event_callback_t callback,
                                               void *userdata);

/**
 * @brief Remove um callback registrado
 * @param events Ponteiro para o sistema de eventos
 * @param type Tipo do evento
 * @param callback Função de callback
 * @return true se o callback foi removido, false caso contrário
 */
bool emu_events_enhanced_unregister_callback(emu_events_enhanced_t *events,
                                             emu_event_type_t type,
                                             emu_event_callback_t callback);

/**
 * @brief Obtém o timestamp atual do sistema
 * @param events Ponteiro para o sistema de eventos
 * @return Timestamp atual em milissegundos
 */
emu_timestamp_t emu_events_enhanced_get_timestamp(emu_events_enhanced_t *events);

/**
 * @brief Define a carga do sistema para adaptar o comportamento de eventos
 * @param events Ponteiro para o sistema de eventos
 * @param system_load Valor da carga do sistema (0.0 - 1.0)
 */
void emu_events_enhanced_set_system_load(emu_events_enhanced_t *events, float system_load);

/**
 * @brief Purga eventos cancelados da fila
 * @param events Ponteiro para o sistema de eventos
 * @return Número de eventos purgados
 */
uint32_t emu_events_enhanced_purge(emu_events_enhanced_t *events);

/**
 * @brief Define o número máximo de eventos por frame
 * @param events Ponteiro para o sistema de eventos
 * @param max_events Número máximo de eventos
 */
void emu_events_enhanced_set_max_events_per_frame(emu_events_enhanced_t *events, uint32_t max_events);

/**
 * @brief Obtém estatísticas sobre o sistema de eventos
 * @param events Ponteiro para o sistema de eventos
 * @param events_in_queue Ponteiro para armazenar número de eventos na fila
 * @param events_per_second Ponteiro para armazenar taxa de eventos por segundo
 * @param avg_processing_time Ponteiro para armazenar tempo médio de processamento em ms
 */
void emu_events_enhanced_get_stats(emu_events_enhanced_t *events,
                                   uint32_t *events_in_queue,
                                   uint32_t *events_per_second,
                                   float *avg_processing_time);

#endif // EVENTS_ENHANCED_H
