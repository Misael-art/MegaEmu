/**
 * @file events_enhanced.c
 * @brief Implementação da interface aprimorada para eventos com suporte a agendamento diferido e sincronização precisa
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "events_enhanced.h"
#include "../../utils/error_handling.h"

// Estrutura para rastreamento de eventos periódicos
typedef struct
{
    uint32_t id;                   // ID único do evento periódico
    emu_event_type_t type;         // Tipo do evento
    void *data;                    // Dados do evento
    size_t data_size;              // Tamanho dos dados
    emu_event_priority_t priority; // Prioridade do evento
    uint64_t interval_ms;          // Intervalo entre execuções
    uint64_t next_trigger;         // Próximo timestamp para disparar
    uint32_t count;                // Contagem restante (0 = infinito)
    bool active;                   // Se o evento está ativo
} periodic_event_t;

// Número máximo de eventos periódicos suportados
#define MAX_PERIODIC_EVENTS 64

// Array de eventos periódicos
static periodic_event_t g_periodic_events[MAX_PERIODIC_EVENTS];

// Contador para IDs de eventos periódicos
static uint32_t g_next_periodic_id = 1;

// Estrutura para armazenamento de callbacks
typedef struct callback_entry
{
    emu_event_type_t type;         // Tipo do evento
    emu_event_callback_t callback; // Função de callback
    void *userdata;                // Dados do usuário
    uint32_t id;                   // ID único do registro
    struct callback_entry *next;   // Próximo na lista
} callback_entry_t;

// Lista de callbacks por tipo de evento
static callback_entry_t *g_callbacks[EMU_EVENT_MAX] = {NULL};

// Próximo ID de registro de callback
static uint32_t g_next_callback_id = 1;

// Estatísticas de processamento
typedef struct
{
    uint32_t total_processed;    // Total de eventos processados
    uint32_t total_samples;      // Número de amostras para média
    uint64_t total_time;         // Tempo total de processamento em ms
    uint64_t last_second;        // Timestamp do último segundo
    uint32_t events_this_second; // Eventos processados no último segundo
} processing_stats_t;

// Estatísticas globais
static processing_stats_t g_stats = {0};

// Função para obter timestamp atual do sistema
static uint64_t _get_system_timestamp_ms(void)
{
    // Implementação real deve usar uma função de tempo do sistema
    // Esta é apenas uma função stub para referência
    static uint64_t timestamp = 0;
    return timestamp += 10; // Incremento arbitrário
}

// Inicialização do sistema de eventos aprimorado
bool emu_events_enhanced_init(emu_events_enhanced_t *events, uint32_t queue_capacity)
{
    if (!events || queue_capacity == 0)
    {
        return false;
    }

    // Inicializar a fila de prioridade
    events->queue = emu_event_queue_create(queue_capacity);
    if (!events->queue)
    {
        return false;
    }

    // Inicializar o estado do sistema de eventos
    events->paused = false;
    events->current_time = _get_system_timestamp_ms();
    events->system_load = 0.0f;
    events->events_per_second = 0;
    events->max_events_per_frame = 100; // Valor padrão

    // Inicializar eventos periódicos
    memset(g_periodic_events, 0, sizeof(g_periodic_events));
    g_next_periodic_id = 1;

    // Inicializar callbacks
    for (int i = 0; i < EMU_EVENT_MAX; i++)
    {
        g_callbacks[i] = NULL;
    }
    g_next_callback_id = 1;

    // Inicializar estatísticas
    memset(&g_stats, 0, sizeof(g_stats));
    g_stats.last_second = events->current_time;

    return true;
}

// Finalização do sistema de eventos
void emu_events_enhanced_shutdown(emu_events_enhanced_t *events)
{
    if (!events)
    {
        return;
    }

    // Destruir a fila de prioridade
    if (events->queue)
    {
        emu_event_queue_destroy(events->queue);
        events->queue = NULL;
    }

    // Liberar memória dos eventos periódicos
    for (int i = 0; i < MAX_PERIODIC_EVENTS; i++)
    {
        if (g_periodic_events[i].active && g_periodic_events[i].data)
        {
            free(g_periodic_events[i].data);
            g_periodic_events[i].data = NULL;
            g_periodic_events[i].active = false;
        }
    }

    // Liberar memória dos callbacks
    for (int i = 0; i < EMU_EVENT_MAX; i++)
    {
        callback_entry_t *current = g_callbacks[i];
        while (current)
        {
            callback_entry_t *next = current->next;
            free(current);
            current = next;
        }
        g_callbacks[i] = NULL;
    }
}

// Agendar evento
bool emu_events_enhanced_schedule(emu_events_enhanced_t *events,
                                  emu_event_type_t type,
                                  void *data,
                                  size_t data_size,
                                  emu_event_priority_t priority,
                                  uint64_t delay_ms)
{
    if (!events || !events->queue || type >= EMU_EVENT_MAX)
    {
        return false;
    }

    // Criar o evento
    emu_event_t event;
    event.type = type;
    event.timestamp = events->current_time;
    event.data = data;
    event.data_size = data_size;

    // Adicionar à fila de prioridade
    return emu_event_queue_push(events->queue, &event, priority, delay_ms);
}

// Agendar evento periódico
uint32_t emu_events_enhanced_schedule_periodic(emu_events_enhanced_t *events,
                                               emu_event_type_t type,
                                               void *data,
                                               size_t data_size,
                                               emu_event_priority_t priority,
                                               uint64_t interval_ms,
                                               uint32_t count)
{
    if (!events || !events->queue || type >= EMU_EVENT_MAX || interval_ms == 0)
    {
        return 0;
    }

    // Procurar slot livre
    int slot = -1;
    for (int i = 0; i < MAX_PERIODIC_EVENTS; i++)
    {
        if (!g_periodic_events[i].active)
        {
            slot = i;
            break;
        }
    }

    if (slot == -1)
    {
        return 0; // Não há slots disponíveis
    }

    // Criar cópia dos dados, se necessário
    void *data_copy = NULL;
    if (data && data_size > 0)
    {
        data_copy = malloc(data_size);
        if (!data_copy)
        {
            return 0;
        }
        memcpy(data_copy, data, data_size);
    }

    // Configurar o evento periódico
    g_periodic_events[slot].id = g_next_periodic_id++;
    g_periodic_events[slot].type = type;
    g_periodic_events[slot].data = data_copy;
    g_periodic_events[slot].data_size = data_size;
    g_periodic_events[slot].priority = priority;
    g_periodic_events[slot].interval_ms = interval_ms;
    g_periodic_events[slot].next_trigger = events->current_time + interval_ms;
    g_periodic_events[slot].count = count;
    g_periodic_events[slot].active = true;

    // Criar o primeiro evento
    emu_event_t event;
    event.type = type;
    event.timestamp = events->current_time;
    event.data = data_copy;
    event.data_size = data_size;

    // Adicionar à fila de prioridade
    emu_event_queue_push(events->queue, &event, priority, interval_ms);

    return g_periodic_events[slot].id;
}

// Cancelar evento
bool emu_events_enhanced_cancel(emu_events_enhanced_t *events,
                                emu_event_type_t type,
                                void *data)
{
    if (!events || !events->queue)
    {
        return false;
    }

    return emu_event_queue_cancel(events->queue, type, data);
}

// Cancelar evento periódico
bool emu_events_enhanced_cancel_periodic(emu_events_enhanced_t *events,
                                         uint32_t periodic_id)
{
    if (!events || periodic_id == 0)
    {
        return false;
    }

    // Procurar o evento periódico
    for (int i = 0; i < MAX_PERIODIC_EVENTS; i++)
    {
        if (g_periodic_events[i].active && g_periodic_events[i].id == periodic_id)
        {
            // Liberar memória dos dados
            if (g_periodic_events[i].data)
            {
                free(g_periodic_events[i].data);
                g_periodic_events[i].data = NULL;
            }

            g_periodic_events[i].active = false;

            // Cancelar qualquer instância pendente na fila
            emu_event_queue_cancel(events->queue, g_periodic_events[i].type, NULL);

            return true;
        }
    }

    return false;
}

// Processa callbacks para um evento
static void _process_callbacks(emu_event_t *event)
{
    if (!event || event->type >= EMU_EVENT_MAX)
    {
        return;
    }

    // Percorrer a lista de callbacks para este tipo de evento
    callback_entry_t *callback = g_callbacks[event->type];
    while (callback)
    {
        callback->callback(event, callback->userdata);
        callback = callback->next;
    }
}

// Atualiza eventos periódicos
static void _update_periodic_events(emu_events_enhanced_t *events)
{
    if (!events)
    {
        return;
    }

    uint64_t current_time = events->current_time;

    // Verificar cada evento periódico
    for (int i = 0; i < MAX_PERIODIC_EVENTS; i++)
    {
        if (g_periodic_events[i].active && current_time >= g_periodic_events[i].next_trigger)
        {
            // Programar uma nova instância do evento
            emu_event_t event;
            event.type = g_periodic_events[i].type;
            event.timestamp = current_time;
            event.data = g_periodic_events[i].data;
            event.data_size = g_periodic_events[i].data_size;

            emu_event_queue_push(events->queue, &event, g_periodic_events[i].priority, 0);

            // Atualizar o próximo tempo de disparo
            g_periodic_events[i].next_trigger += g_periodic_events[i].interval_ms;

            // Decrementar contagem, se necessário
            if (g_periodic_events[i].count > 0)
            {
                g_periodic_events[i].count--;

                // Desativar se atingiu a contagem zero
                if (g_periodic_events[i].count == 0)
                {
                    if (g_periodic_events[i].data)
                    {
                        free(g_periodic_events[i].data);
                        g_periodic_events[i].data = NULL;
                    }
                    g_periodic_events[i].active = false;
                }
            }
        }
    }
}

// Processamento de eventos
uint32_t emu_events_enhanced_process(emu_events_enhanced_t *events,
                                     uint64_t timestamp_ms,
                                     uint32_t max_events)
{
    if (!events || !events->queue || events->paused)
    {
        return 0;
    }

    // Atualizar o timestamp atual
    events->current_time = timestamp_ms;

    // Atualizar a fila de eventos
    emu_event_queue_update(events->queue, timestamp_ms);

    // Processar eventos periódicos
    _update_periodic_events(events);

    // Adaptar prioridades com base na carga do sistema
    emu_event_queue_adapt_priorities(events->queue, events->system_load);

    // Limitar o número de eventos a processar
    uint32_t events_to_process = max_events;
    if (events_to_process == 0 || events_to_process > events->max_events_per_frame)
    {
        events_to_process = events->max_events_per_frame;
    }

    // Processar eventos
    uint64_t start_time = _get_system_timestamp_ms();
    uint32_t processed_count = 0;

    for (uint32_t i = 0; i < events_to_process; i++)
    {
        emu_event_t event;
        if (emu_event_queue_pop(events->queue, &event))
        {
            // Processar o evento
            _process_callbacks(&event);
            processed_count++;

            // Liberar memória do evento, se necessário
            if (event.data)
            {
                // Verificar se este é um evento periódico
                bool is_periodic = false;
                for (int j = 0; j < MAX_PERIODIC_EVENTS; j++)
                {
                    if (g_periodic_events[j].active && g_periodic_events[j].data == event.data)
                    {
                        is_periodic = true;
                        break;
                    }
                }

                // Não liberar a memória de um evento periódico
                if (!is_periodic)
                {
                    free(event.data);
                    event.data = NULL;
                }
            }
        }
        else
        {
            // Não há mais eventos para processar
            break;
        }
    }

    // Atualizar estatísticas
    uint64_t end_time = _get_system_timestamp_ms();
    uint64_t process_time = end_time - start_time;

    g_stats.total_processed += processed_count;
    g_stats.total_time += process_time;
    g_stats.total_samples++;
    g_stats.events_this_second += processed_count;

    // Atualizar eventos por segundo
    if (timestamp_ms - g_stats.last_second >= 1000)
    {
        events->events_per_second = g_stats.events_this_second;
        g_stats.events_this_second = 0;
        g_stats.last_second = timestamp_ms;
    }

    return processed_count;
}

// Pausar o sistema de eventos
void emu_events_enhanced_pause(emu_events_enhanced_t *events)
{
    if (events)
    {
        events->paused = true;
    }
}

// Retomar o sistema de eventos
void emu_events_enhanced_resume(emu_events_enhanced_t *events)
{
    if (events)
    {
        events->paused = false;
    }
}

// Verificar se o sistema está pausado
bool emu_events_enhanced_is_paused(emu_events_enhanced_t *events)
{
    if (!events)
    {
        return true;
    }

    return events->paused;
}

// Registrar callback
uint32_t emu_events_enhanced_register_callback(emu_events_enhanced_t *events,
                                               emu_event_type_t type,
                                               emu_event_callback_t callback,
                                               void *userdata)
{
    if (!events || !callback || type >= EMU_EVENT_MAX)
    {
        return 0;
    }

    // Criar novo registro de callback
    callback_entry_t *entry = (callback_entry_t *)malloc(sizeof(callback_entry_t));
    if (!entry)
    {
        return 0;
    }

    // Configurar o registro
    entry->type = type;
    entry->callback = callback;
    entry->userdata = userdata;
    entry->id = g_next_callback_id++;

    // Adicionar à lista de callbacks para este tipo
    entry->next = g_callbacks[type];
    g_callbacks[type] = entry;

    return entry->id;
}

// Remover callback
bool emu_events_enhanced_unregister_callback(emu_events_enhanced_t *events,
                                             emu_event_type_t type,
                                             emu_event_callback_t callback)
{
    if (!events || !callback || type >= EMU_EVENT_MAX)
    {
        return false;
    }

    callback_entry_t *current = g_callbacks[type];
    callback_entry_t *prev = NULL;

    while (current)
    {
        if (current->callback == callback)
        {
            // Remover da lista
            if (prev)
            {
                prev->next = current->next;
            }
            else
            {
                g_callbacks[type] = current->next;
            }

            free(current);
            return true;
        }

        prev = current;
        current = current->next;
    }

    return false;
}

// Obter timestamp atual
emu_timestamp_t emu_events_enhanced_get_timestamp(emu_events_enhanced_t *events)
{
    if (!events)
    {
        return 0;
    }

    return events->current_time;
}

// Definir carga do sistema
void emu_events_enhanced_set_system_load(emu_events_enhanced_t *events, float system_load)
{
    if (!events)
    {
        return;
    }

    // Limitar ao intervalo [0.0, 1.0]
    if (system_load < 0.0f)
        system_load = 0.0f;
    if (system_load > 1.0f)
        system_load = 1.0f;

    events->system_load = system_load;
}

// Purgar eventos cancelados
uint32_t emu_events_enhanced_purge(emu_events_enhanced_t *events)
{
    if (!events || !events->queue)
    {
        return 0;
    }

    return emu_event_queue_purge(events->queue);
}

// Definir máximo de eventos por frame
void emu_events_enhanced_set_max_events_per_frame(emu_events_enhanced_t *events, uint32_t max_events)
{
    if (!events)
    {
        return;
    }

    events->max_events_per_frame = max_events > 0 ? max_events : 1;
}

// Obter estatísticas
void emu_events_enhanced_get_stats(emu_events_enhanced_t *events,
                                   uint32_t *events_in_queue,
                                   uint32_t *events_per_second,
                                   float *avg_processing_time)
{
    if (!events)
    {
        if (events_in_queue)
            *events_in_queue = 0;
        if (events_per_second)
            *events_per_second = 0;
        if (avg_processing_time)
            *avg_processing_time = 0.0f;
        return;
    }

    if (events_in_queue)
    {
        *events_in_queue = events->queue ? emu_event_queue_size(events->queue) : 0;
    }

    if (events_per_second)
    {
        *events_per_second = events->events_per_second;
    }

    if (avg_processing_time)
    {
        *avg_processing_time = (g_stats.total_samples > 0) ? ((float)g_stats.total_time / g_stats.total_samples) : 0.0f;
    }
}
