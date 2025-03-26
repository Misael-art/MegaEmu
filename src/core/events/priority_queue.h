/**
 * @file priority_queue.h
 * @brief Fila de prioridade adaptativa para gerenciamento de eventos
 */

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "events_interface.h"

/**
 * @brief Níveis de prioridade para eventos
 */
typedef enum
{
    EMU_EVENT_PRIORITY_LOW = 0, /**< Prioridade baixa (processado por último) */
    EMU_EVENT_PRIORITY_NORMAL,  /**< Prioridade normal */
    EMU_EVENT_PRIORITY_HIGH,    /**< Prioridade alta */
    EMU_EVENT_PRIORITY_CRITICAL /**< Prioridade crítica (processado primeiro) */
} emu_event_priority_t;

/**
 * @brief Item na fila de prioridade de eventos
 */
typedef struct
{
    emu_event_t event;             /**< Evento */
    emu_event_priority_t priority; /**< Prioridade do evento */
    uint64_t scheduled_time;       /**< Timestamp programado para execução */
    bool is_canceled;              /**< Se o evento foi cancelado */
} emu_event_queue_item_t;

/**
 * @brief Fila de prioridade para eventos
 */
typedef struct
{
    emu_event_queue_item_t *items; /**< Array de itens */
    uint32_t capacity;             /**< Capacidade máxima da fila */
    uint32_t size;                 /**< Número atual de itens na fila */
    uint64_t current_time;         /**< Timestamp atual do sistema */
} emu_event_priority_queue_t;

/**
 * @brief Cria uma nova fila de prioridade para eventos
 * @param capacity Capacidade máxima da fila
 * @return Ponteiro para a fila criada ou NULL em caso de erro
 */
emu_event_priority_queue_t *emu_event_queue_create(uint32_t capacity);

/**
 * @brief Destrói uma fila de prioridade
 * @param queue Ponteiro para a fila a ser destruída
 */
void emu_event_queue_destroy(emu_event_priority_queue_t *queue);

/**
 * @brief Adiciona um evento à fila com prioridade específica
 * @param queue Ponteiro para a fila
 * @param event Evento a ser adicionado
 * @param priority Prioridade do evento
 * @param delay_ms Atraso em milissegundos para execução (0 para executar imediatamente)
 * @return true se o evento foi adicionado com sucesso, false caso contrário
 */
bool emu_event_queue_push(emu_event_priority_queue_t *queue, const emu_event_t *event,
                          emu_event_priority_t priority, uint64_t delay_ms);

/**
 * @brief Remove e retorna o próximo evento da fila
 * @param queue Ponteiro para a fila
 * @param event Ponteiro para armazenar o evento recuperado
 * @return true se um evento foi recuperado, false se a fila está vazia
 */
bool emu_event_queue_pop(emu_event_priority_queue_t *queue, emu_event_t *event);

/**
 * @brief Visualiza o próximo evento da fila sem removê-lo
 * @param queue Ponteiro para a fila
 * @param event Ponteiro para armazenar o evento visualizado
 * @return true se um evento foi visualizado, false se a fila está vazia
 */
bool emu_event_queue_peek(emu_event_priority_queue_t *queue, emu_event_t *event);

/**
 * @brief Retorna o número de eventos na fila
 * @param queue Ponteiro para a fila
 * @return Número de eventos na fila
 */
uint32_t emu_event_queue_size(emu_event_priority_queue_t *queue);

/**
 * @brief Atualiza o timestamp atual e processa eventos agendados
 * @param queue Ponteiro para a fila
 * @param current_time Timestamp atual do sistema
 */
void emu_event_queue_update(emu_event_priority_queue_t *queue, uint64_t current_time);

/**
 * @brief Cancela eventos pendentes de um tipo específico
 * @param queue Ponteiro para a fila
 * @param type Tipo do evento a ser cancelado
 * @param data Dados específicos para correspondência (NULL para ignorar)
 * @return true se algum evento foi cancelado, false caso contrário
 */
bool emu_event_queue_cancel(emu_event_priority_queue_t *queue, emu_event_type_t type, void *data);

/**
 * @brief Redefine dinamicamente a prioridade dos eventos baseado na carga do sistema
 * @param queue Ponteiro para a fila
 * @param system_load Carga do sistema (0.0 - 1.0)
 */
void emu_event_queue_adapt_priorities(emu_event_priority_queue_t *queue, float system_load);

/**
 * @brief Remove eventos inválidos ou cancelados da fila
 * @param queue Ponteiro para a fila
 * @return Número de eventos removidos
 */
uint32_t emu_event_queue_purge(emu_event_priority_queue_t *queue);

/**
 * @brief Limpa a fila, removendo todos os eventos
 * @param queue Ponteiro para a fila
 */
void emu_event_queue_clear(emu_event_priority_queue_t *queue);

#endif // PRIORITY_QUEUE_H
